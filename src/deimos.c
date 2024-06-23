#include "deimos.h"

#ifdef _WIN32
  #include <windows.h>
#endif

#if !_WIN32
 #include <dlfcn.h>
#endif

struct DeimosFile_s { 
  AQDataStructureFlag flag;
  AQDestroyerFuncType destroyer;
  AQAllocator allocator;
  DeimosFileModeFlag mode; 
  AQInt tab;
  AQULong index;
  FILE* file_struct;
  AQString file_buffer;
  DeimosBackingFlag backing;
};

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

static AQInt deimos_internal_fprintf(DeimosFile file, AQChar* format,...) {
    if ( file->mode != DeimosWriteModeFlag ) return EOF;
    AQInt result;
    va_list args;
    va_start(args, format);
    if (file->backing == DeimosFileBackedFlag)
     result = vfprintf(file->file_struct,format,args);
    if (file->backing == DeimosStringBackedFlag) {
     if ( file->index+100 >= aqstring_get_size(file->file_buffer) )   
      aqstring_expand(file->file_buffer,100);       
     if ( file->index > aqstring_get_size(file->file_buffer) ) result = EOF;
     if ( file->index < 0 ) result = EOF;
     if (result == EOF) goto end;
     result = 
      vsnprintf(&(aqstring_get_c_string(file->file_buffer)[file->index]),
          aqstring_get_size_in_bytes(file->file_buffer)-file->index,
           format,
            args);
            
    if (result > 0) file->index += result;     
    }
end:        
    va_end(args);
    return result; 
}

static AQULong deimos_internal_ftell(DeimosFile file) {
    if (file->backing == DeimosFileBackedFlag)
     return deimos_ftell(file->file_struct);
    if (file->backing == DeimosStringBackedFlag)
     return file->index;
    return 1; 
}

static AQInt deimos_internal_fseek(DeimosFile file, AQULong index, AQInt flag) {
    if (file->backing == DeimosFileBackedFlag)
     return deimos_fseek(file->file_struct,index,flag);
    if (file->backing == DeimosStringBackedFlag) {
         file->index = index;
         return 0;
    }
    return 1;
}

static AQInt deimos_internal_fgetc(DeimosFile file) {
    if ( file->mode != DeimosReadModeFlag ) return EOF;
    AQInt result;
    if (file->backing == DeimosFileBackedFlag)
     return fgetc(file->file_struct);
    if (file->backing == DeimosStringBackedFlag)  {
        if ( file->index > aqstring_get_size(file->file_buffer) ) return EOF;
        if ( file->index < 0 ) return EOF;
        result = aqstring_get_byte(file->file_buffer,file->index);
        file->index++;
        return result;
    }
    return EOF;
}

static AQInt deimos_internal_fputc(AQInt byte, DeimosFile file) {
    if ( file->mode != DeimosWriteModeFlag ) return EOF;
    if (file->backing == DeimosFileBackedFlag)
     return fputc(byte,file->file_struct);
    if (file->backing == DeimosStringBackedFlag)  {
        if ( file->index >= aqstring_get_size(file->file_buffer) ) 
         aqstring_expand(file->file_buffer,1);
        if ( file->index > aqstring_get_size(file->file_buffer) ) return EOF;
        if ( file->index < 0 ) return EOF;
        aqstring_set_byte(file->file_buffer,file->index,(AQByte)byte);
        file->index++;
        return 0;
    }
    return EOF;
}

DeimosFile deimos_open_file_without_allocator(const AQChar* filepath, DeimosFileModeFlag mode) {
    return deimos_open_file_with_allocator(filepath,mode,aqmem_default_allocator());
}

DeimosFile deimos_open_file_with_allocator(const AQChar* filepath, 
     DeimosFileModeFlag mode, AQAllocator allocator) {
    DeimosFile file = aq_new(struct DeimosFile_s,allocator);
    file->flag = AQDestroyableFlag;
    file->destroyer = (AQDestroyerFuncType)deimos_close_file;
    file->allocator = allocator;
    if ( mode == DeimosReadModeFlag ) file->file_struct = fopen(filepath, "r");
    if ( mode == DeimosWriteModeFlag ) file->file_struct = fopen(filepath, "w");
    file->mode = mode;
    file->backing = DeimosFileBackedFlag;
    file->tab = 0;
    file->index = 0;
    return file;
}

DeimosFile deimos_get_file_from_string(AQString string, DeimosFileModeFlag mode) {
     DeimosFile file = aq_new(struct DeimosFile_s,aqstring_get_allocator(string));
    file->flag = AQDestroyableFlag;
    file->destroyer = (AQDestroyerFuncType)deimos_close_file;
    file->allocator = aqstring_get_allocator(string);
    file->file_buffer = string;
    file->mode = mode;
    file->backing = DeimosStringBackedFlag;
    file->tab = 0;
    file->index = 0;
    return file;
}

void deimos_close_file(DeimosFile file) {
    if (file->backing == DeimosFileBackedFlag) 
     fclose(file->file_struct);
    if (file->backing == DeimosStringBackedFlag) 
     aqstring_destroy(file->file_buffer);
    aq_free(file,file->allocator);
}

FILE* deimos_get_file_struct(DeimosFile file) {
    return file->file_struct;
}

AQString deimos_get_file_string(DeimosFile file) {
    return file->file_buffer;
}

AQULong deimos_get_file_position(DeimosFile file) {
    return deimos_internal_ftell(file);
}

AQInt deimos_set_file_position(DeimosFile file, AQULong position) {
    return !deimos_internal_fseek(file,position,SEEK_SET);
}

AQInt deimos_advance_file_position(DeimosFile file, AQULong offset) {
    return deimos_set_file_position(file,
        deimos_get_file_position(file)+offset);
}

AQInt deimos_retreat_file_position(DeimosFile file, AQULong offset) {
    return deimos_set_file_position(file,
        deimos_get_file_position(file)-offset);
}

AQInt deimos_get_character(DeimosFile file) {
    return deimos_get_utf32_character(file);
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

static AQString deimos_internal_get_number(DeimosFile file) {
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

static AQLong deimos_internal_get_signed(DeimosFile file) {
    if ( file->mode != DeimosReadModeFlag ) return EOF;
    AQString string = deimos_internal_get_number(file);
    AQLong value = strtoimax(aqstring_get_c_string(string),NULL,10);
    aqstring_destroy(string);
    return value;
}

static AQULong deimos_internal_get_unsigned(DeimosFile file) {
    if ( file->mode != DeimosReadModeFlag ) return EOF;
    AQString string = deimos_internal_get_number(file);
    AQULong value = strtoumax(aqstring_get_c_string(string),NULL,10);
    aqstring_destroy(string);
    return value;
}

static AQDouble deimos_internal_get_floating_point(DeimosFile file) {
    if ( file->mode != DeimosReadModeFlag ) return EOF;
    AQString string = deimos_internal_get_number(file);
    AQDouble value = strtod(aqstring_get_c_string(string),NULL);
    aqstring_destroy(string);
    return value;
}

AQByte deimos_get_byte(DeimosFile file) {
    return deimos_internal_get_unsigned(file);
}

AQSByte deimos_get_sbyte(DeimosFile file) {
    return deimos_internal_get_signed(file);
}

AQShort deimos_get_short(DeimosFile file) {
    return deimos_internal_get_signed(file);
}

AQUShort deimos_get_ushort(DeimosFile file) {
    return deimos_internal_get_unsigned(file);
}

AQInt deimos_get_integer(DeimosFile file) {
    return deimos_internal_get_signed(file);
}

AQUInt deimos_get_uinteger(DeimosFile file) {
    return deimos_internal_get_unsigned(file);
}

AQLong deimos_get_long(DeimosFile file) {
    return deimos_internal_get_signed(file);
}

AQULong deimos_get_ulong(DeimosFile file) {
    return deimos_internal_get_unsigned(file);
}

AQFloat deimos_get_float(DeimosFile file) {
    return deimos_internal_get_floating_point(file);
}

AQDouble deimos_get_double(DeimosFile file) {
    return deimos_internal_get_floating_point(file);
}

AQInt deimos_output_add_to_tab(DeimosFile file, AQInt value) {
    return file->tab += value;
}

AQInt deimos_output_sub_from_tab(DeimosFile file, AQInt value) {
    return file->tab -= value;
}

AQInt deimos_output_tab(DeimosFile file) {
    AQInt i = file->tab;
    AQInt result;
    while (i > 0) {
        result = deimos_internal_fputc('\t',file);
        if (result == EOF) return EOF;
        i--;
    } 
  return 0;
}

AQInt deimos_output_character(DeimosFile file, AQInt value) {
    return deimos_internal_fputc(value,file);
}

AQInt deimos_output_string(DeimosFile file, AQChar* value) {
    return deimos_internal_fprintf(file,"%s",value);
}

AQInt deimos_output_byte(DeimosFile file, AQByte value) {
    return deimos_internal_fprintf(file,"%hhu",value);
}

AQInt deimos_output_sbyte(DeimosFile file, AQSByte value) {
    return deimos_internal_fprintf(file,"%hhd",value);
}

AQInt deimos_output_short(DeimosFile file, AQShort value) {
    return deimos_internal_fprintf(file,"%hd",value);
}

AQInt deimos_output_ushort(DeimosFile file, AQUShort value) {
    return deimos_internal_fprintf(file,"%hu",value);
}

AQInt deimos_output_integer(DeimosFile file, AQInt value) {
    return deimos_internal_fprintf(file,"%d",value);
}

AQInt deimos_output_uinteger(DeimosFile file, AQUInt value) {
    return deimos_internal_fprintf(file,"%u",value);
}

AQInt deimos_output_long(DeimosFile file, AQLong value) {
    #ifdef _WIN32
     return deimos_internal_fprintf(file,"%lld",value);
    #else
     return deimos_internal_fprintf(file,"%ld",value);
    #endif
}

AQInt deimos_output_ulong(DeimosFile file, AQULong value) {
    #ifdef _WIN32
     return deimos_internal_fprintf(file,"%llu",value);
    #else
     return deimos_internal_fprintf(file,"%lu",value);
    #endif
}

AQInt deimos_output_float(DeimosFile file, AQFloat value) {
    return deimos_internal_fprintf(file,"%.*f",DECIMAL_DIG + 6,value);
}

AQInt deimos_output_double(DeimosFile file, AQDouble value) {
    return deimos_internal_fprintf(file,"%.*lf",DECIMAL_DIG + 6,value);
}

AQInt deimos_get_utf32_character(DeimosFile file) {
    if ( file->mode != DeimosReadModeFlag ) return EOF;
    AQInt n = 0;
    AQSByte byte = 0;
    AQUInt byte0 = 0;
    AQUInt byte1 = 0;
    AQUInt byte2 = 0;
    AQUInt byte3 = 0;
    AQUInt value = 0;
    AQSByte basebyte = 0;
loop:
    byte = deimos_internal_fgetc(file);
    if ( byte == EOF ) return EOF;
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
        byte1 = deimos_internal_fgetc(file);
        byte1 = byte1 & 0x7F;
        value = byte0 | byte1;
    }
    if ( n == 3 ) {
        byte0 = basebyte;
        byte0 = byte0 & 0x1F;
        byte0 = byte0 << 12;
        byte1 = deimos_internal_fgetc(file);
        byte1 = byte1 & 0x7F;
        byte1 = byte1 << 6;
        byte2 = deimos_internal_fgetc(file);
        byte2 = byte2 & 0x7F;
        value = byte0 | byte1 | byte2;
    }
    if ( n == 4 ) {
        byte0 = basebyte;
        byte0 = byte0 & 0xF;
        byte0 = byte0 << 18;
        byte1 = deimos_internal_fgetc(file);
        byte1 = byte1 & 0x7F;
        byte1 = byte1 << 12;
        byte2 = deimos_internal_fgetc(file);
        byte2 = byte2 & 0x7F;
        byte2 = byte2 << 6;
        byte3 = deimos_internal_fgetc(file);
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

 #ifdef _WIN32
  #define deimos_dlopen(file) LoadLibrary(file)
  #define deimos_dlclose(library) FreeLibrary((HMODULE)library)
  #define deimos_dlsym(library, name) GetProcAddress((HMODULE)library,name)
 #else
  #define deimos_dlopen(file) dlopen(file, RTLD_LAZY | RTLD_LOCAL)
  #define deimos_dlclose(library) dlclose(library)
  #define deimos_dlsym(library, name) dlsym(library, name)
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