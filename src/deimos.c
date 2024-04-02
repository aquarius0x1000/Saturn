#include "deimos.h"

#if !_WIN32
 #include <dlfcn.h>
#endif

struct DeimosFile_s { 
  AQInt tab;  
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
    file->tab = 0;
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

AQInt deimos_advance_file_position(DeimosFile file, AQULong offset) {
    return deimos_set_file_position(file,
        deimos_get_file_position(file)+offset);
}

AQInt deimos_retreat_file_position(DeimosFile file, AQULong offset) {
    return deimos_set_file_position(file,
        deimos_get_file_position(file)-offset);
}

AQString deimos_get_string(DeimosFile file, AQInt start, AQInt end) {
    AQInt mode = 0;
    AQInt character = 0;
    AQInt has_started = 0;
    AQInt* string = NULL;
    AQULong num_of_characters = 0;
    if (start == end) mode++;
    while ((character = deimos_get_utf32_character(file)) != EOF) {
        if (mode) goto validate;
        if (character == start) has_started++;
        if (character == start) continue;
        if (!has_started) continue;
        if (character == end) break;
        goto add_character;
    validate:
        if (character == start) has_started++;
        if (character == start && has_started == 1) continue;
        if (!has_started) continue;
        if (has_started > 1) break;
    add_character:    
        num_of_characters++;
        if (string == NULL) string = aq_make_c_array(num_of_characters, AQInt);
        if (string != NULL) string = aq_realloc(string, num_of_characters,
                num_of_characters-1, AQInt, 1);
        string[num_of_characters-1] = character;     
    }
    AQString ret_string = aqstring_new_from_utf32((AQUInt*)string,num_of_characters);
    free(string);
    return ret_string;
}

static AQString deimos_get_number(DeimosFile file) {
    AQInt character = 0;
    AQInt found_numbers = 0;
    AQInt* string = NULL;
    AQULong num_of_characters = 0;
    while ((character = deimos_get_utf32_character(file)) != EOF) {
        if (isdigit(character)) found_numbers++;
        if (!found_numbers) continue;
        if (!isdigit(character) && character != '.') break;
        num_of_characters++;
        if (string == NULL) string = aq_make_c_array(num_of_characters, AQInt);
        if (string != NULL) string = aq_realloc(string, num_of_characters,
                num_of_characters-1, AQInt, 1);
        string[num_of_characters-1] = character;     
    }
    AQString ret_string = aqstring_new_from_utf32((AQUInt*)string,num_of_characters);
    free(string);
    return ret_string;
}

static AQLong deimos_get_signed(DeimosFile file) {
    AQString string = deimos_get_number(file);
    AQLong value = strtoimax(aqstring_get_c_string(string),NULL,10);
    aqstring_destroy(string);
    return value;
}

static AQULong deimos_get_unsigned(DeimosFile file) {
    AQString string = deimos_get_number(file);
    AQULong value = strtoumax(aqstring_get_c_string(string),NULL,10);
    aqstring_destroy(string);
    return value;
}

static AQDouble deimos_get_floating_point(DeimosFile file) {
    AQString string = deimos_get_number(file);
    AQDouble value = strtod(aqstring_get_c_string(string),NULL);
    aqstring_destroy(string);
    return value;
}

AQByte deimos_get_byte(DeimosFile file) {
    return deimos_get_unsigned(file);
}

AQSByte deimos_get_sbyte(DeimosFile file) {
    return deimos_get_signed(file);
}

AQShort deimos_get_short(DeimosFile file) {
    return deimos_get_signed(file);
}

AQUShort deimos_get_ushort(DeimosFile file) {
    return deimos_get_unsigned(file);
}

AQInt deimos_get_integer(DeimosFile file) {
    return deimos_get_signed(file);
}

AQUInt deimos_get_uinteger(DeimosFile file) {
    return deimos_get_unsigned(file);
}

AQLong deimos_get_long(DeimosFile file) {
    return deimos_get_signed(file);
}

AQULong deimos_get_ulong(DeimosFile file) {
    return deimos_get_unsigned(file);
}

AQFloat deimos_get_float(DeimosFile file) {
    return deimos_get_floating_point(file);
}

AQDouble deimos_get_double(DeimosFile file) {
    return deimos_get_floating_point(file);
}

AQInt deimos_output_add_to_tab(DeimosFile file, AQInt value) {
    return file->tab += value;
}

AQInt deimos_output_sub_from_tab(DeimosFile file, AQInt value) {
    return file->tab -= value;
}

AQInt deimos_output_tab(DeimosFile file) {
    AQInt i = file->tab;
    while (i > 0) {
        fputc('\t',file->file);
        i--;
    } 
  return 1;
}

AQInt deimos_output_character(DeimosFile file, AQInt value) {
    return fputc(value,file->file);
}

AQInt deimos_output_string(DeimosFile file, AQChar* value) {
    return fprintf(file->file,"%s",value);
}

AQInt deimos_output_byte(DeimosFile file, AQByte value) {
    return fprintf(file->file,"%hhu",value);
}

AQInt deimos_output_sbyte(DeimosFile file, AQSByte value) {
    return fprintf(file->file,"%hhd",value);
}

AQInt deimos_output_short(DeimosFile file, AQShort value) {
    return fprintf(file->file,"%hd",value);
}

AQInt deimos_output_ushort(DeimosFile file, AQUShort value) {
    return fprintf(file->file,"%hu",value);
}

AQInt deimos_output_integer(DeimosFile file, AQInt value) {
    return fprintf(file->file,"%d",value);
}

AQInt deimos_output_uinteger(DeimosFile file, AQUInt value) {
    return fprintf(file->file,"%u",value);
}

AQInt deimos_output_long(DeimosFile file, AQLong value) {
    return fprintf(file->file,"%ld",value);
}

AQInt deimos_output_ulong(DeimosFile file, AQULong value) {
    return fprintf(file->file,"%lu",value);
}

AQInt deimos_output_float(DeimosFile file, AQFloat value) {
    return fprintf(file->file,"%.*f",DECIMAL_DIG + 6,value);
}

AQInt deimos_output_double(DeimosFile file, AQDouble value) {
    return fprintf(file->file,"%.*lf",DECIMAL_DIG + 6,value);
}

AQInt deimos_get_character(DeimosFile file) {
    return deimos_get_utf32_character(file);
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

AQInt deimos_peek_utf32_character(DeimosFile file, AQULong* offset) {
   AQULong file_position = deimos_get_file_position(file);
skip: 
   AQInt character = deimos_get_utf32_character(file);
   if (isspace(character)) goto skip;
   *offset = deimos_get_file_position(file) - file_position;
   deimos_set_file_position(file,file_position);
   return character;
}

AQInt deimos_peek_last_utf32_character(DeimosFile file, AQULong offset) {
   AQULong file_position = deimos_get_file_position(file);
skip: 
   AQULong last = file_position - (1+offset);
   deimos_set_file_position(file,last);
   AQInt character = deimos_get_utf32_character(file);
   if (isspace(character)) goto skip;
   deimos_set_file_position(file,file_position);
   return character;
}
