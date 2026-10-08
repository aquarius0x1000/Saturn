#include "aquarius.h"
#include "deimos.h"

#ifdef _WIN32
  #include <windows.h>
#endif

#if !_WIN32
  #include <dlfcn.h>
#endif

struct DeimosFile_s { 
  AQDataStructureFlag flag;
  AQDestroyerLambda destroyer;
  AQAllocator allocator;
  DeimosFileModeFlag mode; 
  AQULong tab;
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
    AQULong size; 
    va_list args;
    va_list args2;
    va_start(args, format);
    if (file->backing == DeimosFileBackedFlag)
     result = vfprintf(file->file_struct,format,args);
    if (file->backing == DeimosStringBackedFlag) {
       start:   
        if ( file->index >= aqstring_get_size(file->file_buffer) )   
         aqstring_expand(file->file_buffer,file->index-aqstring_get_size(file->file_buffer));       
        if ( file->index > aqstring_get_size(file->file_buffer) ) result = EOF;
        if ( file->index < 0 ) result = EOF;
        if (result == EOF) goto end;
        size = aqstring_get_size_in_bytes(file->file_buffer)-file->index;
        va_copy(args2, args);
        result =  
         vsnprintf(&(aqstring_get_c_string(file->file_buffer)[file->index]),
          size,
           format,
            args2);
        va_end(args2);        
        if (result >= size) {
            aqstring_expand(file->file_buffer,(result-size)+1);
            goto start;    
        }   
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
        if (file->index >= aqstring_get_size(file->file_buffer)) return EOF;
        if (file->index < 0) return EOF;
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
        return byte;
    }
    return EOF;
}

static AQULong deimos_internal_fread(DeimosFile file, void* ptr, AQULong size, AQULong nmemb) {
    if (file == NULL || ptr == NULL) return 0;
    if (file->mode != DeimosReadModeFlag) return 0;
    if (file->backing == DeimosFileBackedFlag) 
     return (AQULong)fread(ptr, size, nmemb, file->file_struct);
    if (file->backing == DeimosStringBackedFlag) {
        AQULong total_to_read = size * nmemb;
        AQULong available = aqstring_get_size(file->file_buffer) - file->index;
        if (available == 0) return 0;
        AQULong actual_read = (total_to_read < available) ? total_to_read : available;
        memcpy(ptr, &(aqstring_get_c_string(file->file_buffer)[file->index]), actual_read);
        file->index += actual_read;
        return actual_read / size;
    }
    return 0;
}

static AQULong deimos_internal_fwrite(DeimosFile file, const void* ptr, AQULong size, AQULong nmemb) {
    if (file == NULL || ptr == NULL) return 0;
    if (file->mode != DeimosWriteModeFlag) return 0;
    if (file->backing == DeimosFileBackedFlag) 
        return (AQULong)fwrite(ptr, size, nmemb, file->file_struct);
    if (file->backing == DeimosStringBackedFlag) {
        AQULong total_to_write = size * nmemb;
        AQULong current_size = aqstring_get_size(file->file_buffer);
        if (file->index + total_to_write > current_size) {
            AQULong needed = (file->index + total_to_write) - current_size;
            if (aqstring_expand(file->file_buffer, needed) == NULL) return 0;
        }
        memcpy(&(aqstring_get_c_string(file->file_buffer)[file->index]), ptr, total_to_write);
        file->index += total_to_write;
        return nmemb;
    }
    return 0;
}

DeimosFile deimos_open_file_without_allocator(const AQChar* filepath, DeimosFileModeFlag mode) {
    return deimos_open_file_with_allocator(filepath,mode,aqmem_default_allocator());
}

DeimosFile deimos_open_file_with_allocator(const AQChar* filepath, 
     DeimosFileModeFlag mode, AQAllocator allocator) {
    DeimosFile file = aq_new(struct DeimosFile_s,allocator);
    if (file == NULL) return NULL;
    file->flag = AQDestroyableFlag;
    file->destroyer = (AQDestroyerLambda)deimos_close_file;
    file->allocator = allocator;
    if ( mode == DeimosReadModeFlag ) file->file_struct = fopen(filepath, "rb");
    if ( mode == DeimosWriteModeFlag ) file->file_struct = fopen(filepath, "wb");
    file->file_buffer = NULL;
    file->mode = mode;
    file->backing = DeimosFileBackedFlag;
    file->tab = 0;
    file->index = 0;
    return file;
}

DeimosFile deimos_get_file_from_string(AQString string, DeimosFileModeFlag mode) {
    DeimosFile file = aq_new(struct DeimosFile_s,aqstring_get_allocator(string));
    if (file == NULL) return NULL;
    file->flag = AQDestroyableFlag;
    file->destroyer = (AQDestroyerLambda)deimos_close_file;
    file->allocator = aqstring_get_allocator(string);
    file->file_buffer = string;
    file->file_struct = NULL;
    file->mode = mode;
    file->backing = DeimosStringBackedFlag;
    file->tab = 0;
    file->index = 0;
    return file;
}

DeimosStatus deimos_close_file(DeimosFile file) {
    if (file == NULL) return DeimosFailure;
    DeimosStatus status = DeimosSuccess;
    if (file->backing == DeimosFileBackedFlag) 
     if (fclose(file->file_struct) == EOF) 
      status = DeimosFailure;
    if (file->backing == DeimosStringBackedFlag) 
     if (aqstring_destroy(file->file_buffer) == AQFailureValue)
      status = DeimosFailure;
    if (aq_free(file,file->allocator) == AQFailureValue)
     status = DeimosFailure;
    return status;
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

DeimosStatus deimos_set_file_position(DeimosFile file, AQULong position) {
    return (deimos_internal_fseek(file,position,SEEK_SET) != 0) ? DeimosFailure : DeimosSuccess;
}

DeimosStatus deimos_advance_file_position(DeimosFile file, AQULong offset) {
    return (deimos_set_file_position(file,
     deimos_get_file_position(file)+offset) != 0) ? DeimosFailure : DeimosSuccess;
}

DeimosStatus deimos_retreat_file_position(DeimosFile file, AQULong offset) {
    return (deimos_set_file_position(file,
     deimos_get_file_position(file)-offset) != 0) ? DeimosFailure : DeimosSuccess;
}

DeimosStatus deimos_copy_file_to_file(DeimosFile file_to_copy, DeimosFile file_with_copied_data) {
    AQInt character = 0;
    while ((character = deimos_internal_fgetc(file_to_copy)) != EOF) {
        if (deimos_output_character(file_with_copied_data,character) == DeimosFailure) return DeimosFailure; 
    }
    return DeimosSuccess;
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
    while ((character = deimos_get_utf32_character(file)) != DeimosFailure) {
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
                num_of_characters-1, AQInt,1);
        string[num_of_characters-1] = character;     
    }
    AQString ret_string = NULL;
    if (string == NULL) ret_string = aqstr(""); //return empty string --add alloc--
    if (string != NULL) ret_string = aqstring_new_from_utf32((AQUInt*)string,num_of_characters);
    free(string);
    return ret_string;
}

static AQString deimos_internal_get_number(DeimosFile file) {
    AQInt character = 0;
    AQInt found_numbers = 0;
    AQInt* string = NULL;
    AQULong num_of_characters = 0;
    while ((character = deimos_get_utf32_character(file)) != DeimosFailure) {
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
    if ( file->mode != DeimosReadModeFlag ) return DeimosFailure;
    AQString string = deimos_internal_get_number(file);
    if (string == NULL) return DeimosFailure;
    AQLong value = strtoimax(aqstring_get_c_string(string),NULL,10);
    aqstring_destroy(string);
    return value;
}

static AQULong deimos_internal_get_unsigned(DeimosFile file) {
    if ( file->mode != DeimosReadModeFlag ) return DeimosFailure;
    AQString string = deimos_internal_get_number(file);
    if (string == NULL) return DeimosFailure;
    AQULong value = strtoumax(aqstring_get_c_string(string),NULL,10);
    aqstring_destroy(string);
    return value;
}

static AQDouble deimos_internal_get_floating_point(DeimosFile file) {
    if ( file->mode != DeimosReadModeFlag ) return DeimosFailure;
    AQString string = deimos_internal_get_number(file);
    if (string == NULL) return DeimosFailure;
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

DeimosStatus deimos_output_tab(DeimosFile file) {
    AQInt i = file->tab;
    AQInt result;
    while (i > 0) {
        result = deimos_internal_fputc('\t',file);
        if (result == EOF) return DeimosFailure;
        i--;
    } 
  return DeimosSuccess;
}

DeimosStatus deimos_output_character(DeimosFile file, AQInt value) {
    if (deimos_internal_fputc(value,file) == EOF) return DeimosFailure;
    return DeimosSuccess;
}

DeimosStatus deimos_output_string(DeimosFile file, AQChar* value) {
    if (deimos_internal_fprintf(file,"%s",value) != 0) return DeimosFailure;
    return DeimosSuccess;
}

DeimosStatus deimos_output_byte(DeimosFile file, AQByte value) {
    if (deimos_internal_fprintf(file,"%hhu",value) != 0) return DeimosFailure;
    return DeimosSuccess;
}

DeimosStatus deimos_output_sbyte(DeimosFile file, AQSByte value) {
    if (deimos_internal_fprintf(file,"%hhd",value) != 0) return DeimosFailure;
    return DeimosSuccess;
}

DeimosStatus deimos_output_short(DeimosFile file, AQShort value) {
    if (deimos_internal_fprintf(file,"%hd",value) != 0) return DeimosFailure;
    return DeimosSuccess;
}

DeimosStatus deimos_output_ushort(DeimosFile file, AQUShort value) {
    if (deimos_internal_fprintf(file,"%hu",value) != 0) return DeimosFailure;
    return DeimosSuccess;
}

DeimosStatus deimos_output_integer(DeimosFile file, AQInt value) {
    if (deimos_internal_fprintf(file,"%d",value) != 0) return DeimosFailure;
    return DeimosSuccess;
}

DeimosStatus deimos_output_uinteger(DeimosFile file, AQUInt value) {
    if (deimos_internal_fprintf(file,"%u",value) != 0) return DeimosFailure;
    return DeimosSuccess;
}

DeimosStatus deimos_output_long(DeimosFile file, AQLong value) {
    #ifdef _WIN32
     if (deimos_internal_fprintf(file,"%lld",value) != 0) return DeimosFailure;
    #else
     if (deimos_internal_fprintf(file,"%ld",value) != 0) return DeimosFailure;
    #endif
    return DeimosSuccess;
}

DeimosStatus deimos_output_ulong(DeimosFile file, AQULong value) {
    #ifdef _WIN32
      if (deimos_internal_fprintf(file,"%llu",value) != 0) return DeimosFailure;
    #else
      if (deimos_internal_fprintf(file,"%lu",value) != 0) return DeimosFailure;
    #endif
    return DeimosSuccess;
}

DeimosStatus deimos_output_float(DeimosFile file, AQFloat value) {
    if (deimos_internal_fprintf(file,"%.*f",DECIMAL_DIG + 6,value) != 0) return DeimosFailure;
    return DeimosSuccess;     
}

DeimosStatus deimos_output_double(DeimosFile file, AQDouble value) {
    if (deimos_internal_fprintf(file,"%.*lf",DECIMAL_DIG + 6,value) != 0) return DeimosFailure;
    return DeimosSuccess;
}

static AQBool deimos_internal_is_little_endian(void) {
    AQUShort n = 0x1;
    return *(AQByte*)&n == 1;
}

static void deimos_internal_swap_bytes(AQByte* ptr, AQULong size) {
    AQULong i = 0;
    while (i < size / 2) {
        AQByte t = ptr[i];
        ptr[i] = ptr[size - 1 - i];
        ptr[size - 1 - i] = t;
        i++;
    }
}

static DeimosStatus deimos_internal_write_bytes(DeimosFile file, AQAny data, AQULong size) {
    AQByte buffer[16];
    memcpy(buffer, data, size);
    if (deimos_internal_is_little_endian()) {
        deimos_internal_swap_bytes(buffer, size);
    }
    if (deimos_internal_fwrite(file, buffer, size, 1) != 1) {
        return DeimosFailure;
    }
    return DeimosSuccess;
}

static DeimosStatus deimos_internal_read_bytes(DeimosFile file, AQAny data, AQULong size) {
    AQByte buffer[16];
    if (deimos_internal_fread(file, buffer, size, 1) != 1) {
        return DeimosFailure;
    }
    if (deimos_internal_is_little_endian()) {
        deimos_internal_swap_bytes(buffer, size);
    }
    memcpy(data, buffer, size);
    return DeimosSuccess;
}

AQByte deimos_get_binary_byte(DeimosFile file) {
    AQByte value;
    if (deimos_internal_read_bytes(file, &value, sizeof(AQByte)) == DeimosFailure) return 0;
    return value;
}

AQSByte deimos_get_binary_sbyte(DeimosFile file) {
    AQSByte value;
    if (deimos_internal_read_bytes(file, &value, sizeof(AQSByte)) == DeimosFailure) return 0;
    return value;
}

AQShort deimos_get_binary_short(DeimosFile file) {
    AQShort value;
    if (deimos_internal_read_bytes(file, &value, sizeof(AQShort)) == DeimosFailure) return 0;
    return value;
}

AQUShort deimos_get_binary_ushort(DeimosFile file) {
    AQUShort value;
    if (deimos_internal_read_bytes(file, &value, sizeof(AQUShort)) == DeimosFailure) return 0;
    return value;
}

AQInt deimos_get_binary_integer(DeimosFile file) {
    AQInt value;
    if (deimos_internal_read_bytes(file, &value, sizeof(AQInt)) == DeimosFailure) return 0;
    return value;
}

AQUInt deimos_get_binary_uinteger(DeimosFile file) {
    AQUInt value;
    if (deimos_internal_read_bytes(file, &value, sizeof(AQUInt)) == DeimosFailure) return 0;
    return value;
}

AQLong deimos_get_binary_long(DeimosFile file) {
    AQLong value;
    if (deimos_internal_read_bytes(file, &value, sizeof(AQLong)) == DeimosFailure) return 0;
    return value;
}

AQULong deimos_get_binary_ulong(DeimosFile file) {
    AQULong value;
    if (deimos_internal_read_bytes(file, &value, sizeof(AQULong)) == DeimosFailure) return 0;
    return value;
}

AQFloat deimos_get_binary_float(DeimosFile file) {
    AQFloat value;
    if (deimos_internal_read_bytes(file, &value, sizeof(AQFloat)) == DeimosFailure) return 0.0f;
    return value;
}

AQDouble deimos_get_binary_double(DeimosFile file) {
    AQDouble value;
    if (deimos_internal_read_bytes(file, &value, sizeof(AQDouble)) == DeimosFailure) return 0.0;
    return value;
}

DeimosStatus deimos_output_binary_byte(DeimosFile file, AQByte byte) {
    if (deimos_internal_fputc(byte,file) == EOF) return DeimosFailure;
    return DeimosSuccess;
}

DeimosStatus deimos_output_binary_sbyte(DeimosFile file, AQSByte value) {
    if (deimos_internal_fputc(value, file) == EOF) return DeimosFailure;
    return DeimosSuccess;
}

DeimosStatus deimos_output_binary_short(DeimosFile file, AQShort value) {
    return deimos_internal_write_bytes(file, &value, sizeof(AQShort));
}

DeimosStatus deimos_output_binary_ushort(DeimosFile file, AQUShort value) {
    return deimos_internal_write_bytes(file, &value, sizeof(AQUShort));
}

DeimosStatus deimos_output_binary_integer(DeimosFile file, AQInt value) {
    return deimos_internal_write_bytes(file, &value, sizeof(AQInt));
}

DeimosStatus deimos_output_binary_uinteger(DeimosFile file, AQUInt value) {
    return deimos_internal_write_bytes(file, &value, sizeof(AQUInt));
}

DeimosStatus deimos_output_binary_long(DeimosFile file, AQLong value) {
    return deimos_internal_write_bytes(file, &value, sizeof(AQLong));
}

DeimosStatus deimos_output_binary_ulong(DeimosFile file, AQULong value) {
    return deimos_internal_write_bytes(file, &value, sizeof(AQULong));
}

DeimosStatus deimos_output_binary_float(DeimosFile file, AQFloat value) {
    return deimos_internal_write_bytes(file, &value, sizeof(AQFloat));
}

DeimosStatus deimos_output_binary_double(DeimosFile file, AQDouble value) {
    return deimos_internal_write_bytes(file, &value, sizeof(AQDouble));
}

AQDataStructure deimos_get_binary_aqds(DeimosFile file) {
    if (file == NULL) return NULL;
    AQDataStructureFlag flag = deimos_get_binary_uinteger(file);
    deimos_retreat_file_position(file,sizeof(AQDataStructureFlag));
    switch (flag) {
        case AQMTAContainerFlag:
            return (AQDataStructure)deimos_get_binary_mta_container(file);
        case AQStringFlag:
            return (AQDataStructure)deimos_get_binary_string(file);
        case AQMultiTypeArrayFlag:
            return (AQDataStructure)deimos_get_binary_mta(file);
        case AQArrayFlag:
            return (AQDataStructure)deimos_get_binary_array(file);
        case AQListFlag:
            return (AQDataStructure)deimos_get_binary_list(file);
        case AQStoreFlag:
            return (AQDataStructure)deimos_get_binary_store(file);
        case AQArrayStoreFlag:
            return (AQDataStructure)deimos_get_binary_arraystore(file);
        default:
            return NULL;
    }
}

AQMTAContainer* deimos_get_binary_mta_container(DeimosFile file) {
    if (file == NULL) return NULL;
    AQDataStructureFlag ds_flag = (AQDataStructureFlag)deimos_get_binary_uinteger(file);
    if (ds_flag != AQMTAContainerFlag)
     return NULL;
    AQTypeFlag type_flag = (AQTypeFlag)deimos_get_binary_uinteger(file);
    AQMTAContainer* container = aq_new(AQMTAContainer, deimos_get_allocator(file));
    if (container == NULL) return NULL;
    container->flag = AQMTAContainerFlag;
    container->type = type_flag;
    switch (type_flag) {
        case AQByteFlag:
            container->AQByteVal = deimos_get_binary_byte(file);
            break;
        case AQSByteFlag:
            container->AQSByteVal = deimos_get_binary_sbyte(file);
            break;
        case AQShortFlag:
            container->AQShortVal = deimos_get_binary_short(file);
            break;
        case AQUShortFlag:
            container->AQUShortVal = deimos_get_binary_ushort(file);
            break;
        case AQIntFlag:
            container->AQIntVal = deimos_get_binary_integer(file);
            break;
        case AQUIntFlag:
            container->AQUIntVal = deimos_get_binary_uinteger(file);
            break;
        case AQLongFlag:
            container->AQLongVal = deimos_get_binary_long(file);
            break;
        case AQULongFlag:
            container->AQULongVal = deimos_get_binary_ulong(file);
            break;
        case AQFloatFlag:
            container->AQFloatVal = deimos_get_binary_float(file);
            break;
        case AQDoubleFlag:
            container->AQDoubleVal = deimos_get_binary_double(file);
            break;
        case AQAnyFlag:
        default:
            aq_free(container, deimos_get_allocator(file));
            return NULL;
    }
    return container;
}

AQString deimos_get_binary_string(DeimosFile file) {
    if (file == NULL) return NULL;
    AQDataStructureFlag flag = (AQDataStructureFlag)deimos_get_binary_uinteger(file);
    if (flag != (AQULong)AQStringFlag) return NULL;
    AQULong length = deimos_get_binary_ulong(file);
    if (length == 0)
     return aqstring_new_with_allocator(1, deimos_get_allocator(file));
    AQString string = aqstring_new_with_allocator(length + 1, deimos_get_allocator(file));
    if (string == NULL) return NULL;
    if (deimos_internal_fread(file, aqstring_get_c_string(string), 1, length) != length) {
        aqstring_destroy(string);
        return NULL;
    }
    return string;
}

AQMultiTypeArray deimos_get_binary_mta(DeimosFile file) {
    if (file == NULL) return NULL;
    AQDataStructureFlag ds_flag = (AQDataStructureFlag)deimos_get_binary_uinteger(file);
    if (ds_flag != AQMultiTypeArrayFlag)
     return NULL;
    AQUInt num_type_blocks = deimos_get_binary_uinteger(file);
    AQMultiTypeArray mta = aq_new_mta(deimos_get_allocator(file));
    if (mta == NULL) return NULL;
    AQUInt b = 0;
    while (b < num_type_blocks) {
        AQTypeFlag type_flag = (AQTypeFlag)deimos_get_binary_uinteger(file);
        AQULong length = deimos_get_binary_ulong(file);
        if (type_flag < AQByteFlag || type_flag > AQDoubleFlag) {
            aqmta_destroy(mta);
            return NULL;
        }
        switch(type_flag) {
            case AQByteFlag:    aq_mta_add_space(AQByte,mta,length); break;
            case AQSByteFlag:   aq_mta_add_space(AQSByte,mta,length); break;
            case AQShortFlag:   aq_mta_add_space(AQShort,mta,length); break;
            case AQUShortFlag:  aq_mta_add_space(AQUShort,mta,length); break;
            case AQIntFlag:     aq_mta_add_space(AQInt,mta,length); break;
            case AQUIntFlag:    aq_mta_add_space(AQUInt,mta,length); break;
            case AQLongFlag:    aq_mta_add_space(AQLong,mta,length); break;
            case AQULongFlag:   aq_mta_add_space(AQULong,mta,length); break;
            case AQFloatFlag:   aq_mta_add_space(AQFloat,mta,length); break;
            case AQDoubleFlag:  aq_mta_add_space(AQDouble,mta,length); break;
        }
        AQAny buffer = aqmta_get_buffer_for_type(mta,type_flag);
        AQULong element_size = 0;
        switch(type_flag) {
            case AQByteFlag:    element_size = sizeof(AQByte); break;
            case AQSByteFlag:   element_size = sizeof(AQSByte); break;
            case AQShortFlag:   element_size = sizeof(AQShort); break;
            case AQUShortFlag:  element_size = sizeof(AQUShort); break;
            case AQIntFlag:     element_size = sizeof(AQInt); break;
            case AQUIntFlag:    element_size = sizeof(AQUInt); break;
            case AQLongFlag:    element_size = sizeof(AQLong); break;
            case AQULongFlag:   element_size = sizeof(AQULong); break;
            case AQFloatFlag:   element_size = sizeof(AQFloat); break;
            case AQDoubleFlag:  element_size = sizeof(AQDouble); break;
        }
        AQByte* byte_ptr = (AQByte*)buffer;
        AQULong j = 0; 
        while (j < length) {
            if (deimos_internal_read_bytes(file, byte_ptr + (j * element_size), element_size) == DeimosFailure) {
                aqmta_destroy(mta);
                return NULL;
            }
            j++;
        }
        aqmta_set_num_of_items(mta,type_flag,length);
        b++;
    }
    return mta;
}

AQArray deimos_get_binary_array(DeimosFile file) {
    AQDataStructureFlag flag = (AQDataStructureFlag)deimos_get_binary_uinteger(file);
    if (flag != AQArrayFlag)
     return NULL;
    AQULong length = deimos_get_binary_ulong(file);
    AQArray array = aq_new_array(deimos_get_allocator(file));
    if (array == NULL) return NULL;
    AQULong i = 0;
    while (i < length) {
        AQDataStructure item = deimos_get_binary_aqds(file);
        if (item == NULL) goto destroy;
        if (aqarray_add_item(array,item) == AQFailureValue) {
           destroy: 
            aq_array_foreach(index,array) {
                AQDataStructure item = aqarray_get_item(array,index);
                if (aqds_get_flag(item) == AQMTAContainerFlag) {
                    aq_free(item,deimos_get_allocator(file));
                } else {
                    aq_destroy(item);
                }
            }
            aqarray_destroy(array);
            aq_destroy(item);
            return NULL;
        }
        i++;
    }
    return array;
}

AQList deimos_get_binary_list(DeimosFile file) {
    if (file == NULL) return NULL;
    AQDataStructureFlag flag = (AQDataStructureFlag)deimos_get_binary_uinteger(file);
    if (flag != AQListFlag)
     return NULL;
    AQULong length = deimos_get_binary_ulong(file);
    AQList list = aqlist_new_with_allocator(deimos_get_allocator(file));
    if (list == NULL) return NULL;
    AQULong i = 0;
    while ( i < length) {
        AQDataStructure item = deimos_get_binary_aqds(file);
        if (item == NULL) goto destroy;
        if (aqlist_add_item(list, item) == AQFailureValue) {   
       destroy:
            aq_list_foreach(node,list) {
                AQDataStructure item = aqlist_get_item(node);
                if (aqds_get_flag(item) == AQMTAContainerFlag) {
                    aq_free(item,deimos_get_allocator(file));
                } else {
                    aq_destroy(item);
                }
            }
            aqlist_destroy(list);
            aq_destroy(item);
            return NULL;     
        }
        i++;
    }
    return list;
}

AQStore deimos_get_binary_store(DeimosFile file) {
    if (file == NULL) return NULL;
    AQDataStructureFlag flag = (AQDataStructureFlag)deimos_get_binary_uinteger(file);
    if (flag != AQStoreFlag) return NULL;
    AQULong length = deimos_get_binary_ulong(file);
    AQStore store = aqstore_new_with_allocator(deimos_get_allocator(file));
    if (store == NULL) return NULL;
    AQULong i = 0;
    while ( i < length) {
        AQString label = deimos_get_binary_string(file);
        if (label == NULL) goto destroy;
        AQDataStructure item = deimos_get_binary_aqds(file);
        if (item == NULL) goto destroy;
        if (aqstore_add_item(store, item, aqstring_get_c_string(label)) == AQFailureValue) {
           destroy:
            aq_store_foreach(node,store) {
                AQDataStructure item = aqlist_get_item(node);
                if (aqds_get_flag(item) == AQMTAContainerFlag) {
                    aq_free(item,deimos_get_allocator(file));
                } else {
                    aq_destroy(item);
                }
            }
            aqstring_destroy(label);
            aq_destroy(item);
            aqstore_destroy(store);
            return NULL;
        }
        aqstring_destroy(label);
        i++;
    }
    return store;
}

AQArrayStore deimos_get_binary_arraystore(DeimosFile file) {
    if (file == NULL) return NULL;
    AQDataStructureFlag flag = deimos_get_binary_uinteger(file);
    if (flag != AQArrayStoreFlag) 
     return NULL;
    AQULong index = deimos_get_binary_ulong(file);
    AQStore store = deimos_get_binary_store(file); //if store is null, set_store will return fail
    AQArrayStore array_store = 
     aqarraystore_new_with_allocator(deimos_get_allocator(file));
    if (array_store == NULL) goto destroy;
    if (aqarraystore_set_index(array_store,index) == AQFailureValue) goto destroy;
    if (aqarraystore_set_store(array_store,store) == AQFailureValue) {
       destroy: 
        if (store != NULL) {
            aq_store_foreach(node,store) {
                AQDataStructure item = aqlist_get_item(node);
                if (aqds_get_flag(item) == AQMTAContainerFlag) {
                    aq_free(item,deimos_get_allocator(file));
                } else {
                    aq_destroy(item);
                }
            }
        }
        aqstore_destroy(store);
        aqarraystore_destroy(array_store);
        return NULL;
    }
    return array_store;
}

DeimosStatus deimos_output_binary_aqds(DeimosFile file, AQDataStructure ds) {
    if (file == NULL || ds == NULL) return DeimosFailure;
    switch (aqds_get_flag(ds)) {
        case AQMTAContainerFlag:
            return deimos_output_binary_mta_container(file, (AQMTAContainer*)ds);
        case AQStringFlag:
            return deimos_output_binary_string(file, (AQString)ds);
        case AQMultiTypeArrayFlag:
            return deimos_output_binary_mta(file, (AQMultiTypeArray)ds);
        case AQArrayFlag:
            return deimos_output_binary_array(file, (AQArray)ds);
        case AQListFlag:
            return deimos_output_binary_list(file, (AQList)ds);
        case AQStoreFlag:
            return deimos_output_binary_store(file, (AQStore)ds);
        case AQArrayStoreFlag:
            return deimos_output_binary_arraystore(file, (AQArrayStore)ds);
        default:
            return DeimosFailure; // Unsupported structure
    }
}

DeimosStatus deimos_output_binary_mta_container(DeimosFile file, AQMTAContainer* container) {
    if (file == NULL) return DeimosFailure;
    if (deimos_output_binary_uinteger(file, (AQUInt)AQMTAContainerFlag) == DeimosFailure)
     return DeimosFailure;
    if (deimos_output_binary_uinteger(file, (AQUInt)container->type) == DeimosFailure)
     return DeimosFailure;
    switch (container->type) {
        case AQByteFlag:
            if (deimos_output_binary_byte(file, container->AQByteVal) == DeimosFailure) return DeimosFailure;
            break;
        case AQSByteFlag:
            if (deimos_output_binary_sbyte(file, container->AQSByteVal) == DeimosFailure) return DeimosFailure;
            break;
        case AQShortFlag:
            if (deimos_output_binary_short(file, container->AQShortVal) == DeimosFailure) return DeimosFailure;
            break;
        case AQUShortFlag:
            if (deimos_output_binary_ushort(file, container->AQUShortVal) == DeimosFailure) return DeimosFailure;
            break;
        case AQIntFlag:
            if (deimos_output_binary_integer(file, container->AQIntVal) == DeimosFailure) return DeimosFailure;
            break;
        case AQUIntFlag:
            if (deimos_output_binary_uinteger(file, container->AQUIntVal) == DeimosFailure) return DeimosFailure;
            break;
        case AQLongFlag:
            if (deimos_output_binary_long(file, container->AQLongVal) == DeimosFailure) return DeimosFailure;
            break;
        case AQULongFlag:
            if (deimos_output_binary_ulong(file, container->AQULongVal) == DeimosFailure) return DeimosFailure;
            break;
        case AQFloatFlag:
            if (deimos_output_binary_float(file, container->AQFloatVal) == DeimosFailure) return DeimosFailure;
            break;
        case AQDoubleFlag:
            if (deimos_output_binary_double(file, container->AQDoubleVal) == DeimosFailure) return DeimosFailure;
            break;
        case AQAnyFlag:
        default:
            return DeimosFailure;
    }
    return DeimosSuccess;
}

DeimosStatus deimos_output_binary_string(DeimosFile file, AQString string) {
    if (file == NULL || string == NULL) return DeimosFailure;
    if (deimos_output_binary_uinteger(file, (AQUInt)AQStringFlag) == DeimosFailure)
     return DeimosFailure;
    AQULong length = aqstring_get_size(string);
    if (deimos_output_binary_ulong(file, length) == DeimosFailure)
     return DeimosFailure;    
    if (deimos_internal_fwrite(file, aqstring_get_c_string(string), 1, length) != length)
     return DeimosFailure;
    return DeimosSuccess;
}

DeimosStatus deimos_output_binary_mta(DeimosFile file, AQMultiTypeArray mta) {
    if (file == NULL || mta == NULL) return DeimosFailure;
    if (deimos_output_binary_uinteger(file, (AQUInt)AQMultiTypeArrayFlag) == DeimosFailure)
     return DeimosFailure;
    AQUInt num_type_blocks = 0;
    AQInt i = 0;
    while (i < 10) { 
        if (aqmta_get_num_of_items(mta,i+1) > 0) {
            num_type_blocks++;
        }
        i++;
    }
    if (deimos_output_binary_uinteger(file, num_type_blocks) == DeimosFailure)
     return DeimosFailure;
    i = 0; 
    while (i < 10) {
        AQULong count = aqmta_get_num_of_items(mta,i+1);
        if (count == 0) {
            i++;
            continue;
        }
        if (deimos_output_binary_uinteger(file, (AQUInt)(i + 1)) == DeimosFailure)
         return DeimosFailure;
        if (deimos_output_binary_ulong(file, count) == DeimosFailure)
         return DeimosFailure;
        AQAny buffer = aqmta_get_buffer_for_type(mta,i+1);
        AQULong element_size = 0;
        switch(i + 1) {
            case AQByteFlag:    element_size = sizeof(AQByte); break;
            case AQSByteFlag:   element_size = sizeof(AQSByte); break;
            case AQShortFlag:   element_size = sizeof(AQShort); break;
            case AQUShortFlag:  element_size = sizeof(AQUShort); break;
            case AQIntFlag:     element_size = sizeof(AQInt); break;
            case AQUIntFlag:    element_size = sizeof(AQUInt); break;
            case AQLongFlag:    element_size = sizeof(AQLong); break;
            case AQULongFlag:   element_size = sizeof(AQULong); break;
            case AQFloatFlag:   element_size = sizeof(AQFloat); break;
            case AQDoubleFlag:  element_size = sizeof(AQDouble); break;
        }
        AQByte* byte_ptr = (AQByte*)buffer;
        AQULong j = 0;
        while (j < count) {
            if (deimos_internal_write_bytes(file, byte_ptr + (j * element_size), element_size) == DeimosFailure)
             return DeimosFailure;
            j++; 
        }
        i++;
    }
    return DeimosSuccess;
}

DeimosStatus deimos_output_binary_array(DeimosFile file, AQArray array) {
    if (file == NULL || array == NULL) return DeimosFailure;
    if (deimos_output_binary_uinteger(file, (AQUInt)AQArrayFlag) == DeimosFailure) 
     return DeimosFailure;
    AQULong length = aqarray_get_num_of_items(array);
    if (deimos_output_binary_ulong(file, length) == DeimosFailure)
     return DeimosFailure;
    AQULong i = 0;
    while ( i < length) {
        AQAny item = aqarray_get_item(array, i);
        if (item == NULL) return DeimosFailure; //binary serialization requires no NULLs in arrays
        if (deimos_output_binary_aqds(file, (AQDataStructure)item) == DeimosFailure)
         return DeimosFailure;
        i++; 
    }
    return DeimosSuccess;
}

DeimosStatus deimos_output_binary_list(DeimosFile file, AQList list) {
    if (file == NULL || list == NULL) return DeimosFailure;
    if (deimos_output_binary_uinteger(file, (AQUInt)AQListFlag) == DeimosFailure)
     return DeimosFailure;
    AQULong length = aqlist_num_of_nodes(list);
    if (deimos_output_binary_ulong(file, length) == DeimosFailure)
     return DeimosFailure;
    aq_list_foreach(node,list) {
        AQAny item = aqlist_get_item(node);
        if (item == NULL) return DeimosFailure;
        if (deimos_output_binary_aqds(file, (AQDataStructure)item) == DeimosFailure)
         return DeimosFailure;
    }
    return DeimosSuccess;
}

DeimosStatus deimos_output_binary_store(DeimosFile file, AQStore store) {
    if (file == NULL || store == NULL) return DeimosFailure;
    if (deimos_output_binary_uinteger(file, (AQUInt)AQStoreFlag) == DeimosFailure)
     return DeimosFailure;
    AQULong length = aqstore_num_of_items(store);
    if (deimos_output_binary_ulong(file, length) == DeimosFailure)
     return DeimosFailure;
    aq_store_foreach(node, store) {
        AQString label = aqstore_label_from_list_node(node);
        if (deimos_output_binary_string(file, label) == DeimosFailure)
         return DeimosFailure;
        AQAny item = aqlist_get_item(node);
        if (item == NULL) return DeimosFailure;
        if (deimos_output_binary_aqds(file, (AQDataStructure)item) == DeimosFailure)
         return DeimosFailure;
    }
    return DeimosSuccess;
}

DeimosStatus deimos_output_binary_arraystore(DeimosFile file, AQArrayStore array_store) {
    if (file == NULL || array_store == NULL) return DeimosFailure;
    if (deimos_output_binary_uinteger(file, (AQUInt)AQArrayStoreFlag) == DeimosFailure) 
     return DeimosFailure;
    if (deimos_output_binary_ulong(file, aqarraystore_get_index(array_store)) == DeimosFailure) 
     return DeimosFailure;
    if (deimos_output_binary_store(file, aqarraystore_get_store(array_store)) == DeimosFailure) 
     return DeimosFailure;
    return DeimosSuccess;
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

static DeimosStatus deimos_internal_b32s_encode_character(DeimosFile encoded_file, AQInt character) {
    AQInt result = 0;
    AQDouble A, B, C = 0;
    if (character <= 31) {
        result = deimos_output_binary_byte(encoded_file,deimos_internal_b32s_set_1_encode(character));
    }
    if (character > 31) {
        A = character / 32.0;
        C = modf(A,&B);
        A = C * 32;
        deimos_output_binary_byte(encoded_file,deimos_internal_b32s_set_2_encode(A));
        result = deimos_output_binary_byte(encoded_file,deimos_internal_b32s_set_1_encode(B));
    }
    return (result == EOF) ? DeimosFailure : DeimosSuccess;
}

DeimosStatus deimos_get_base_32_star_encode(DeimosFile file_to_encode, DeimosFile encoded_file) {
    AQInt character = 0;
    while ((character = deimos_internal_fgetc(file_to_encode)) != EOF) {
        if (deimos_internal_b32s_encode_character(encoded_file,
         deimos_internal_b32s_mapping(character)) == DeimosFailure)
          return DeimosFailure;
    }
    return DeimosSuccess;
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

DeimosStatus deimos_get_base_32_star_decode(DeimosFile file_to_decode, DeimosFile decoded_file) {
    AQInt character = 0;
    AQInt decoded_character = 0;
    while ((character = deimos_internal_fgetc(file_to_decode)) != EOF) {
        decoded_character = deimos_internal_b32s_decode_character(file_to_decode,character);
        if (decoded_character == EOF) return DeimosFailure;
        deimos_output_binary_byte(decoded_file,deimos_internal_b32s_mapping(decoded_character)); 
    }
    return DeimosSuccess;
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
    if ( byte == EOF ) return DeimosFailure;
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
    if (character == EOF) return DeimosFailure;
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
    if (character == EOF) return DeimosFailure;
    if (isspace(character)) goto skip;
    deimos_set_file_position(file,file_position);
    return character;
}

DeimosStatus deimos_output_utf32_character(DeimosFile file, AQInt character) {
    const AQInt* text = &character;
    AQString string = aqstring_new_from_utf32(text,1);
    AQChar* c_string = aqstring_convert_to_c_string(string);
    if (deimos_internal_fprintf(file,"%s",c_string) != 0) {
       free(c_string);
       return DeimosFailure;
    }
    free(c_string);
    return DeimosSuccess;
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

DeimosStatus deimos_free_library(AQAny library) {
    return 
     (deimos_internal_macro_dlclose(library) != 0) ? DeimosFailure : DeimosSuccess;
}

AQAny deimos_get_function(AQAny library, const AQChar* name) {
    return deimos_internal_macro_dlsym(library, name);
}