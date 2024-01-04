#include "deimos.h"

#if  !_WIN32
 #include <dlfcn.h>
#endif

struct DeimosFile_s { 
  FILE* file;
  DeimosFileModeFlag mode; 
};

 #ifdef _WIN32
  #include <windows.h>
  #define deimos_dlopen(file) LoadLibrary(file)
  #define deimos_dlclose(library) FreeLibrary((HMODULE)library)
  #define deimos_dlsym(library, name) GetProcAddress((HMODULE)library,name)
 #else
  #define deimos_dlopen(file) dlopen(file, RTLD_LAZY | RTLD_LOCAL)
  #define deimos_dlclose(library) dlclose(library)
  #define deimos_dlsym(library, name) dlsym(library, name)
 #endif

#ifdef _WIN32
 #define deimos_ftell(file) _ftelli64(file)
#else
 #define deimos_ftell(file) ftell(file)
#endif
 
#ifdef _WIN32
 #define deimos_fseek(file,pos,origin) _fseeki64(file,pos,origin)
#else
 #define deimos_fseek(file,pos,origin) fseek(file,pos,origin)
#endif 
 
AQAny deimos_load_library_file(const AQChar* filepath) {
    return deimos_dlopen(filepath);
}

AQInt deimos_free_library(AQAny library) {
    return deimos_dlclose(library) ;
}

AQAny deimos_get_function(AQAny library, const AQChar* name) {
     return deimos_dlsym(library, name);
}

DeimosFile deimos_open_file(const AQChar* filepath, DeimosFileModeFlag mode) {
    DeimosFile file = aq_new(struct DeimosFile_s);
    if ( mode == DeimosReadModeFlag ) file->file = fopen(filepath, "r");
    if ( mode == DeimosWriteModeFlag ) file->file = fopen(filepath, "w");
    file->mode = mode;
    return file;
}

void deimos_close_file(DeimosFile file) {
    fclose(file->file);
    free(file);
}

FILE* deimos_get_file_struct(DeimosFile file) {
    return file->file;
}

AQULong deimos_get_file_position(DeimosFile file) {
    return deimos_ftell(file->file);
}

AQInt deimos_set_file_position(DeimosFile file, AQULong position) {
    return !deimos_fseek(file->file,position,SEEK_SET);
}

AQInt deimos_output_character(DeimosFile file, AQInt value) {
    return fputc(value,file->file);
}

AQInt deimos_output_string(DeimosFile file, AQChar* value) {
    return fprintf(file->file,"%s",value);
}

AQInt deimos_output_integer(DeimosFile file, AQInt value) {
    return fprintf(file->file,"%d",value);
}

AQInt deimos_output_float(DeimosFile file, AQFloat value) {
    return fprintf(file->file,"%f",value);
}

static AQUInt deimos_internal_get_32bit_int(FILE* file, AQInt* error) {
    static AQUInt table[] = {1,256,65536,16777216};
    AQInt i = 0;
    AQInt input = 0;
    AQInt errorval = 0;
    AQUInt numsum = 0;
    while (i < 4) {
        input = getc(file);
        if (input == EOF){
            errorval++;
            break;
        }
        numsum += (input * table[i]);
        i++;
    }
    *error = errorval;
    return numsum;
}

static AQInt deimos_internal_set_32bit_int(FILE* file, AQUInt num) {
    static AQUInt table[] = {1,256,65536,16777216};
    AQByte array[4];
    AQInt i = 0;
    AQInt j = 3;
    AQInt error = 0;
    AQUInt output = 0;
    while (j >= 0) {
        if ( num < table[j] ) {
         array[j] = 0;
        } else {
         output = num / table[j];
         array[j] = output;
         num -= (output * table[j]);
        }
        j-- ;
    }
    while ( i < 4 ) {
        error = putc(array[i],file);
        if (error == EOF) break;
        i++;
    }
    return (error < 0) ? 1 : 0;
}

static AQFloat deimos_internal_get_32bit_float(FILE* file, AQInt* error) {
    AQUInt i = deimos_internal_get_32bit_int(file, error);
    return *((AQFloat*)&i);
}

static AQInt deimos_internal_set_32bit_float(FILE* file, AQFloat num) {
    AQUInt i = *((AQUInt*)&num);
    return deimos_internal_set_32bit_int(file, i);
}


AQUInt deimos_read_32bit_int(DeimosFile file, AQInt* error) {
    if ( file->mode != DeimosReadModeFlag ) {
        *error = -1;
        return 0;
    }
    return deimos_internal_get_32bit_int(file->file, error);
}

AQInt deimos_write_32bit_int(DeimosFile file, AQInt num) {
    if ( file->mode != DeimosWriteModeFlag ) {
        return -1;
    }
   return deimos_internal_set_32bit_int(file->file, num);
}

AQFloat deimos_read_32bit_float(DeimosFile file, AQInt* error) {
    if ( file->mode != DeimosReadModeFlag ) {
        *error = -1;
        return 0;
    }
    return deimos_internal_get_32bit_float(file->file, error);
}

AQInt deimos_write_32bit_float(DeimosFile file, AQFloat num) {
    if ( file->mode != DeimosWriteModeFlag ) {
        return -1;
    }
    return deimos_internal_set_32bit_float(file->file, num) ;
}

AQInt deimos_get_utf32_character(DeimosFile file) {
    if ( file->mode != DeimosReadModeFlag ) {
        return -1;
    }
    AQInt n = 0;
    AQSByte byte = 0;
    AQUInt byte0 = 0;
    AQUInt byte1 = 0;
    AQUInt byte2 = 0;
    AQUInt byte3 = 0;
    AQUInt value = 0;
    AQSByte basebyte = 0;
loop:
    byte = fgetc(file->file);
    if ( byte == EOF ) return byte;
    basebyte = byte;
    if ( byte > 0 ) {
        n = 1;
    } else {
        n = 0;
        while (byte < 0) {
            byte = byte << 1;
            n++;
        }
        if ( n == 1 ) {
            goto loop;
        }
    }
    if ( n == 1 ) {
        byte0 = basebyte;
        value = byte0;
    }
    if ( n == 2 ) {
        byte0 = basebyte;
        byte0 = byte0 & 0x3F;
        byte0 = byte0 << 6;
        byte1 = fgetc(file->file);
        byte1 = byte1 & 0x7F;
        value = byte0 | byte1;
    }
    if ( n == 3 ) {
        byte0 = basebyte;
        byte0 = byte0 & 0x1F;
        byte0 = byte0 << 12;
        byte1 = fgetc(file->file);
        byte1 = byte1 & 0x7F;
        byte1 = byte1 << 6;
        byte2 = fgetc(file->file);
        byte2 = byte2 & 0x7F;
        value = byte0 | byte1 | byte2;
    }
    if ( n == 4 ) {
        byte0 = basebyte;
        byte0 = byte0 & 0xF;
        byte0 = byte0 << 18;
        byte1 = fgetc(file->file);
        byte1 = byte1 & 0x7F;
        byte1 = byte1 << 12;
        byte2 = fgetc(file->file);
        byte2 = byte2 & 0x7F;
        byte2 = byte2 << 6;
        byte3 = fgetc(file->file);
        byte3 = byte3 & 0x7F;
        value = byte0 | byte1 | byte2 | byte3;
    }
    return value;
}
