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
 #define deimos_internal_macro_ftell(file) _ftelli64(file)
#else
 #define deimos_internal_macro_ftell(file) ftell(file)
#endif
 
#ifdef _WIN32
 #define deimos_internal_macro_fseek(file,pos,origin) _fseeki64(file,pos,origin)
#else
 #define deimos_internal_macro_fseek(file,pos,origin) fseek(file,pos,origin)
#endif 

static AQInt deimos_internal_fprintf(DeimosFile file, AQChar* format,...) {
    if ( file->mode != DeimosWriteModeFlag ) return EOF;
    AQInt result;
    va_list args;
    va_start(args, format);
    if (file->backing == DeimosFileBackedFlag)
     result = vfprintf(file->file_struct,format,args);
    if (file->backing == DeimosStringBackedFlag) {
     if ( file->index+1000 >= aqstring_get_size(file->file_buffer) )   
      aqstring_expand(file->file_buffer,1000);       
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
     return deimos_internal_macro_ftell(file->file_struct);
    if (file->backing == DeimosStringBackedFlag)
     return file->index;
    return EOF; 
}

static AQInt deimos_internal_fseek(DeimosFile file, AQULong index, AQInt flag) {
    if (file->backing == DeimosFileBackedFlag)
     return deimos_internal_macro_fseek(file->file_struct,index,flag);
    if (file->backing == DeimosStringBackedFlag) {
         file->index = index;
         return 0;
    }
    return EOF;
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
        if (result == '\0') return EOF;
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
    file->file_buffer = NULL;
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
    file->file_struct = NULL;
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

AQAllocator deimos_get_allocator(DeimosFile file) {
    return file->allocator;  
}

FILE* deimos_get_file_struct(DeimosFile file) {
    return file->file_struct;
}

AQString deimos_get_file_string(DeimosFile file) {
    return file->file_buffer;
}

DeimosFileModeFlag deimos_get_file_mode(DeimosFile file) {
    return file->mode;
}

DeimosBackingFlag deimos_get_file_backing(DeimosFile file) {
    return file->backing;
}

AQULong deimos_get_file_position(DeimosFile file) {
    return deimos_internal_ftell(file);
}

AQInt deimos_set_file_position(DeimosFile file, AQULong position) {
    return deimos_internal_fseek(file,position,SEEK_SET);
}

AQInt deimos_advance_file_position(DeimosFile file, AQULong offset) {
    return deimos_set_file_position(file,
        deimos_get_file_position(file)+offset);
}

AQInt deimos_retreat_file_position(DeimosFile file, AQULong offset) {
    return deimos_set_file_position(file,
        deimos_get_file_position(file)-offset);
}

AQInt deimos_copy_file_to_file(DeimosFile file_to_copy, DeimosFile file_with_copied_data) {
    AQInt character = 0;
    while ((character = deimos_internal_fgetc(file_to_copy)) != EOF) {
        if (deimos_output_character(file_with_copied_data,character) == EOF) return EOF; 
    }
    return 0;
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
    if (string == NULL) return NULL;
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
    if (string == NULL) return NULL;
    AQString ret_string = aqstring_new_from_utf32((AQUInt*)string,num_of_characters);
    free(string);
    return ret_string;
}

static AQLong deimos_internal_get_signed(DeimosFile file) {
    if ( file->mode != DeimosReadModeFlag ) return EOF;
    AQString string = deimos_internal_get_number(file);
    if (string == NULL) return EOF;
    AQLong value = strtoimax(aqstring_get_c_string(string),NULL,10);
    aqstring_destroy(string);
    return value;
}

static AQULong deimos_internal_get_unsigned(DeimosFile file) {
    if ( file->mode != DeimosReadModeFlag ) return EOF;
    AQString string = deimos_internal_get_number(file);
    if (string == NULL) return EOF;
    AQULong value = strtoumax(aqstring_get_c_string(string),NULL,10);
    aqstring_destroy(string);
    return value;
}

static AQDouble deimos_internal_get_floating_point(DeimosFile file) {
    if ( file->mode != DeimosReadModeFlag ) return EOF;
    AQString string = deimos_internal_get_number(file);
    if (string == NULL) return EOF;
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

AQByte deimos_get_binary_byte(DeimosFile file) {
    return deimos_internal_fgetc(file);
}

AQInt deimos_output_binary_byte(DeimosFile file, AQByte byte) {
    return deimos_internal_fputc(byte,file);
}

/*
------------------------------------base32*----------------------------------------
-----------------------------------------------------------------------------------
Binary to text encoding, designed to be ideal for prometheus's serialization format.
Encodes data in 1-2 bytes, or ascii characters. Maps more frequently used characters 
in prometheus's serialization format, to be encoded in only one byte, that would
otherwise require two. Two sets of 32 printable ascii characters are used. First set
means no other byte is needed and the value can be determined, set 2 means its value
is the one's place and the next character which must be of set 1 is in the 32's place.

--set 1-- (+0)
0-9 -- 0-9
A-V -- 10 - 31
--set 2--(+1)
W-Z -- 0-3
a-z -- 4-29
& -- 30
$ -- 31
*/

static AQInt deimos_internal_b32s_mapping(AQInt value) {    
    if (value >= 0 && value <= 26)
     return value + 33;
    
    if (value >= 33 && value <= 59)
     return value - 33;
     
    if (value == 91) return 27; //[
    if (value == 27) return 91;
    
    if (value == 93) return 28; //]
    if (value == 28) return 93; 
    
    if (value == 123) return 29; //{
    if (value == 29) return 123;
    
    if (value == 125) return 30; //}
    if (value == 30) return 125;
    
    if (value == 64) return 31; //@
    if (value == 31) return 64;
    
    return value;
}

static AQInt deimos_internal_b32s_set_1_encode(AQInt value) {
    if (value >= 0 && value <= 9)
     value = value + 48;
     
    if (value >= 10 && value <= 31)
     value = value + 55;
    
    return value;
}

static AQInt deimos_internal_b32s_set_1_get_value(AQInt value) {
    if (value >= 48 && value <= 57)
     value = value - 48;
     
    if (value >= 65 && value <= 86)
     value = value - 55;
    
    return value;
}

static AQInt deimos_internal_is_b32s_set_1(AQInt value) {
    if (value >= 48 && value <= 57)
     return 1;
     
    if (value >= 65 && value <= 86)
     return 1;
     
    return 0;  
}

static AQInt deimos_internal_b32s_set_2_encode(AQInt value) {
    if (value >= 0 && value <= 3)
     value = value + 87;
     
    if (value >= 4 && value <= 29)
     value = value + 93; 
     
    if (value == 30) value = 36;
    if (value == 31) value = 38;
  
    return value;
}

static AQInt deimos_internal_b32s_set_2_get_value(AQInt value) {
    if (value >= 87 && value <= 91)
     value = value - 87;
     
    if (value >= 97 && value <= 122)
     value = value - 93; 
     
    if (value == 36) value = 30; 
    if (value == 38) value = 31;
  
    return value;
}

static AQInt deimos_internal_is_b32s_set_2(AQInt value) {
    if (value >= 87 && value <= 91)
     return 1;
     
    if (value >= 97 && value <= 122)
      return 1; 
     
    if (value == 36) return 1;
    if (value == 38) return 1;
   
    return 0;  
}

static void deimos_internal_b32s_encode_character(DeimosFile encoded_file, AQInt character) {
    AQDouble A, B, C = 0;
    if (character <= 31) {
        deimos_output_binary_byte(encoded_file,deimos_internal_b32s_set_1_encode(character));
    }
    if (character > 31) {
        A = character / 32.0;
        C = modf(A,&B);
        A = C * 32;
        deimos_output_binary_byte(encoded_file,deimos_internal_b32s_set_2_encode(A));
        deimos_output_binary_byte(encoded_file,deimos_internal_b32s_set_1_encode(B));
    }
}

AQInt deimos_get_base_32_star_encode(DeimosFile file_to_encode, DeimosFile encoded_file) {
    AQInt character = 0;
    while ((character = deimos_internal_fgetc(file_to_encode)) != EOF) {
        deimos_internal_b32s_encode_character(encoded_file,deimos_internal_b32s_mapping(character));
    }
    return 0;
}

static AQInt deimos_internal_b32s_decode_character(DeimosFile encoded_file, AQInt character) {
   if (deimos_internal_is_b32s_set_1(character))
    return deimos_internal_b32s_set_1_get_value(character);
   if (deimos_internal_is_b32s_set_2(character)) {
       AQInt next_character = deimos_internal_fgetc(encoded_file);
       if (next_character == EOF) return EOF;
       if (!deimos_internal_is_b32s_set_1(next_character)) return EOF;
       return deimos_internal_b32s_set_2_get_value(character) +
        deimos_internal_b32s_set_1_get_value(next_character) * 32;
   }
   return EOF;
}

AQInt deimos_get_base_32_star_decode(DeimosFile file_to_decode, DeimosFile decoded_file) {
    AQInt character = 0;
    AQInt decoded_character = 0;
    while ((character = deimos_internal_fgetc(file_to_decode)) != EOF) {
        decoded_character = deimos_internal_b32s_decode_character(file_to_decode,character);
        if (decoded_character == EOF) return EOF;
        deimos_output_binary_byte(decoded_file,deimos_internal_b32s_mapping(decoded_character)); 
    }
    return 0;
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
   if (character == EOF) return EOF;
   if (isspace(character)) goto skip;
   *offset = deimos_get_file_position(file) - file_position;
   deimos_set_file_position(file,file_position);
   return character;
}

AQInt deimos_peek_last_utf32_character(DeimosFile file, AQULong offset) {
   AQULong file_position = deimos_get_file_position(file);   
   AQULong last = file_position - (1+offset);
   deimos_set_file_position(file,last);
skip:   
   AQInt character = deimos_get_utf32_character(file);
   if (character == EOF) return EOF;
   if (isspace(character)) goto skip;
   deimos_set_file_position(file,file_position);
   return character;
}

AQInt deimos_output_utf32_character(DeimosFile file, AQInt character) {
    const AQInt* text = &character;
    AQString string = aqstring_new_from_utf32(text,1);
    AQChar* c_string = aqstring_convert_to_c_string(string);
    if (deimos_internal_fprintf(file,"%s",c_string) == EOF) 
     return EOF;
    free(c_string);
}
 
 #ifdef _WIN32
  #define deimos_internal_macro_dlopen(file) LoadLibrary(file)
  #define deimos_internal_macro_dlclose(library) FreeLibrary((HMODULE)library)
  #define deimos_internal_macro_dlsym(library, name) GetProcAddress((HMODULE)library,name)
 #else
  #define deimos_internal_macro_dlopen(file) dlopen(file, RTLD_LAZY | RTLD_LOCAL)
  #define deimos_internal_macro_dlclose(library) dlclose(library)
  #define deimos_internal_macro_dlsym(library, name) dlsym(library, name)
 #endif

AQAny deimos_load_library_file(const AQChar* filepath) {
    return deimos_internal_macro_dlopen(filepath);
}

AQInt deimos_free_library(AQAny library) {
    return deimos_internal_macro_dlclose(library) ;
}

AQAny deimos_get_function(AQAny library, const AQChar* name) {
     return deimos_internal_macro_dlsym(library, name);
}