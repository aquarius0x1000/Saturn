#include "aquarius.h"

struct AQDataStructure_s {
 AQ_DATA_STRUCTURE_BASE_CLASS
};

struct AQArray_s {
  AQ_DATA_STRUCTURE_BASE_CLASS
  AQAllocator allocator;  
  AQAny* items;
  AQULong num_of_items;
};

struct AQString_s {
  AQ_DATA_STRUCTURE_BASE_CLASS
  AQAllocator allocator;
  AQLong size_in_characters;
  AQULong size_in_bytes;
  AQChar* data;
 };

 struct AQListNode_s {
   AQ_DATA_STRUCTURE_BASE_CLASS   
   struct AQListNode_s* before;
   struct AQListNode_s* after;
   AQAny data;
   AQString string;
 };

 struct AQList_s {
   AQ_DATA_STRUCTURE_BASE_CLASS
   AQAllocator allocator;
   AQULong num_of_nodes;
   AQListNode first;
   AQListNode last;
};

struct AQStack_s {
  AQ_DATA_STRUCTURE_BASE_CLASS
  AQList list;
};

struct AQStackBuffer_s {
  AQ_DATA_STRUCTURE_BASE_CLASS
  AQArray buffer;
  AQULong index;
  AQULong count;
  AQULong rate;
};

struct AQMultiTypeArray_s {
  AQ_DATA_STRUCTURE_BASE_CLASS
  AQAllocator allocator;
  AQAny item_arrays[11];
  AQULong num_of_items[11];
};

struct AQMTAStackBuffer_s {
  AQ_DATA_STRUCTURE_BASE_CLASS
  AQMultiTypeArray data_buffer;
  AQTypeFlag* type_buffer;
  AQULong type_buffer_size;
  AQULong indexes[11];
  AQULong type_index;
  AQULong counts[11];
  AQULong type_count;
  AQULong rate;
};

typedef struct AQStoreBinaryNode_s {
  AQListNode node;
  struct AQStoreBinaryNode_s* next_binary_node;
} AQStoreBinaryNode;

struct AQStore_s {
  AQ_DATA_STRUCTURE_BASE_CLASS
  AQAllocator allocator;
  AQStoreBinaryNode* dictionary;
  AQList items;
};

AQInt aqprint_string(AQString value) {
    return aqstring_print(value);
}

AQInt aqprint_c_string(AQChar* value) {
    return printf("%s",value);
}

AQInt aqprint_byte(AQByte value) {
    return printf("%hhu",value);
}

AQInt aqprint_sbyte(AQSByte value) {
    return printf("%hhd",value);
}

AQInt aqprint_short(AQShort value) {
    return printf("%hd",value);
}

AQInt aqprint_ushort(AQUShort value) {
    return printf("%hu",value);
}

AQInt aqprint_int(AQInt value) {
    return printf("%d",value);
}

AQInt aqprint_uint(AQUInt value) {
    return printf("%u",value);
}

AQInt aqprint_long(AQLong value) {
    #ifdef _WIN32
     return printf("%lld",value);
    #else
     return printf("%ld",value);
    #endif 
}

AQInt aqprint_ulong(AQULong value) {
    #ifdef _WIN32
     return printf("%llu",value);
    #else
     return printf("%lu",value);
    #endif 
}

AQInt aqprint_float(AQFloat value) {
    return printf("%.*f",DECIMAL_DIG + 6,value);
}

AQInt aqprint_double(AQDouble value) {
    return printf("%.*lf",DECIMAL_DIG + 6,value);
}

static AQAny aqinternal_default_malloc(AQAny allocation_data, AQULong size_in_bytes) {
    return (AQAny)malloc(size_in_bytes);
}

static void aqinternal_default_free(AQAny allocation_data, AQAny data_to_be_freed) {
    free(data_to_be_freed);
}

AQAllocator aqmem_default_allocator(void) {
    static AQAllocatorStruct default_allocator = {
      .allocator_function = aqinternal_default_malloc,
      .free_function = aqinternal_default_free,
      .data = NULL
    };
    return &default_allocator;
}

AQAny aqmem_malloc(AQULong size_in_bytes) {
    return aqmem_malloc_with_allocator(size_in_bytes,aqmem_default_allocator());
}

AQAny aqmem_malloc_with_allocator(AQULong size_in_bytes, AQAllocator allocator) {
    return (AQAny)allocator->allocator_function(allocator->data,size_in_bytes);
}

void aqmem_free(AQAny data) {
    aqmem_free_with_allocator(data,aqmem_default_allocator());
}

void aqmem_free_with_allocator(AQAny data, AQAllocator allocator) {
    allocator->free_function(allocator->data,data);
}

AQAny aqmem_realloc(AQAny data, AQULong newsize,
  AQULong oldsize, AQInt NULLonError0No1Yes) {
    return aqmem_realloc_with_allocator(data,newsize,
      oldsize,NULLonError0No1Yes,aqmem_default_allocator());
}

AQAny aqmem_realloc_with_allocator(AQAny data, AQULong newsize,
  AQULong oldsize, AQInt NULLonError0No1Yes, AQAllocator allocator) {
    AQAny newdata = aq_alloc(newsize,allocator);
    if ( newdata != NULL ) {
        memcpy(newdata,data,oldsize);
        aq_free(data,allocator);
        return newdata;
    } else {
        if ( NULLonError0No1Yes ) {
            aq_free(data,allocator);
            return NULL;
        } else {
            return data;
        }
    }
}

void aqds_destroy(AQDataStructure ds) {
    ds->destroyer(ds);
}

AQDataStructureFlag aqds_get_flag(AQDataStructure ds) {
    return ds->flag;
}

AQArray aqarray_new(void) {
    return aqarray_new_with_allocator(aqmem_default_allocator());
}

AQArray aqarray_new_with_allocator(AQAllocator allocator) {
    AQArray array = aq_new(struct AQArray_s,allocator);
    array->flag = AQArrayFlag;
    array->destroyer = (AQDestroyerFuncType)aqarray_destroy;
    array->allocator = allocator;
    array->num_of_items = 0;
    array->items = NULL;
    return array;
}

AQArray aqarray_new_with_base_size(AQULong base_size) {
    return aqarray_new_with_base_size_with_allocator(base_size,aqmem_default_allocator());
}

AQArray aqarray_new_with_base_size_with_allocator(AQULong base_size, AQAllocator allocator) {
    AQArray array = aq_new(struct AQArray_s, allocator);
    array->flag = AQArrayFlag;
    array->destroyer = (AQDestroyerFuncType)aqarray_destroy;
    array->allocator = allocator;
    array->items = aq_make_c_array(base_size, AQAny, allocator);
    array->num_of_items = base_size;
    return array;
}

void aqarray_destroy(AQArray array) {
    if (array == NULL) return;
    aq_free(array->items,array->allocator);
    aq_free(array,array->allocator);
}

void aqarray_add_item(AQArray array, AQAny item) {
    array->num_of_items++;
    if ( array->items == NULL ) {
        array->items =
         aq_make_c_array(array->num_of_items, AQAny, array->allocator);
    } else {
        array->items = aq_realloc(
         array->items,
          array->num_of_items,
           array->num_of_items-1,
            AQAny,
             1,
              array->allocator);
    }
    array->items[array->num_of_items-1] = item;
}

void aqarray_remove_item(AQArray array) {
    array->num_of_items--;
    if ( array->items == NULL ) {
        return;
    } else {
        array->items = aq_realloc(
         array->items,
          array->num_of_items,
           array->num_of_items,
            AQAny,
             1,
              array->allocator);
    }
}

void aqarray_add_array(AQArray array, AQAny items[], AQULong num_of_items_to_add) {
    aqarray_add_space(array,num_of_items_to_add);
    memcpy(array->items+(array->num_of_items-num_of_items_to_add),items,array->num_of_items);
}

void aqarray_add_space(AQArray array, AQULong space_to_add) {
    if ( array->items == NULL ) {
        array->num_of_items = space_to_add;
        array->items =
         aq_make_c_array(array->num_of_items, AQAny, array->allocator);
    } else {
        array->num_of_items += space_to_add;
         array->items = aq_realloc(
          array->items,
           array->num_of_items,
            array->num_of_items-space_to_add,
             AQAny,
              1,
               array->allocator);
    }
}

void aqarray_remove_space(AQArray array, AQULong space_to_remove) {
    if ( array->items == NULL ) {
        return;
    } else {
        array->num_of_items -= space_to_remove;
         array->items = aq_realloc(
          array->items,
           array->num_of_items,
            array->num_of_items,
             AQAny,
              1,
               array->allocator);
 }
}

void* aqarray_get_item(AQArray array, AQULong index) {
    if ( index >= 0 && index < array->num_of_items ) {
        return array->items[index];
    }
    return NULL;
}

AQInt aqarray_set_item(AQArray array, AQULong index, AQAny item) {
    if ( index >= 0 && index < array->num_of_items ) {
        array->items[index] = item;
        return 1;
    }
    return 0;
}

AQULong aqarray_get_num_of_items(AQArray array) {
  return array->num_of_items;
}

void aqarray_iterate_with(AQIteratorFuncType iterator, AQArray array) {
    int i = 0;
    while ( i < array->num_of_items ) {
        iterator(array->items[i]);
        i++;
    }
}

AQString aqstring_new(AQULong size_in_bytes) {
    return aqstring_new_with_allocator(size_in_bytes,aqmem_default_allocator());
}

AQString aqstring_new_with_allocator(AQULong size_in_bytes, AQAllocator allocator) {
    AQString string = aq_new(struct AQString_s,allocator);
    string->flag = AQStringFlag;
    string->destroyer = (AQDestroyerFuncType)aqstring_destroy;
    string->size_in_bytes = size_in_bytes;
    string->data = aq_make_c_array(string->size_in_bytes, AQChar, allocator);
    string->data[string->size_in_bytes-1] = '\0';
    string->size_in_bytes -= 1; //not counting '\0'
    string->size_in_characters = -1;
    string->allocator = allocator;
    return string;
}

AQString aqstring_new_from_utf32(const AQUInt* text, AQULong num_of_characters) {
    return aqstring_new_from_utf32_with_allocator(text,num_of_characters,aqmem_default_allocator());
}

AQString aqstring_new_from_utf32_with_allocator(const AQUInt* text,
  AQULong num_of_characters, AQAllocator allocator) {
    AQInt i = 0;
    AQUInt character = 0;
    AQUInt value = 0;
    AQUInt value0 = 0;
    AQUInt value1 = 0;
    AQUInt value2 = 0;
    AQString string = NULL;
    AQULong utf8string_size = 1;
    AQChar* utf8string = aq_make_c_array(1, AQChar, allocator);
    utf8string[utf8string_size-1] = '\0';
    while ( i < num_of_characters ) {
        character = text[i];
        if ( character >= 0 && character <= 0x007F ) {
            utf8string =
              aq_realloc(utf8string, utf8string_size+1,
                 utf8string_size, AQChar, 1, allocator);
            utf8string_size++;
            utf8string[utf8string_size-2] = character;
            utf8string[utf8string_size-1] = '\0';
        }
        if ( character >= 0x0080 && character <= 0x07FF ) {
            value = character;
            value = value >> 6;
            value = value | 0xC0;
            value0 = character;
            value0 = value0 & 0x3F;
            value0 = value0 | 0x80;
            utf8string =
             aq_realloc(utf8string, utf8string_size+2,
                utf8string_size, AQChar, 1, allocator);
            utf8string_size+=2;
            utf8string[utf8string_size-3] = value;
            utf8string[utf8string_size-2] = value0;
            utf8string[utf8string_size-1] = '\0';
        }
        if ( character >= 0x0800 && character <= 0xFFFF ) {
            value = character;
            value = value >> 12;
            value = value | 0xE0;
            value0 = character;
            value0 = value0 >> 6;
            value0 = value0 & 0xBF;
            value0 = value0 | 0x80;
            value1 = character;
            value1 = value1 & 0x3F;
            value1 = value1 | 0x80;
            utf8string =
             aq_realloc(utf8string, utf8string_size+3,
                utf8string_size, AQChar, 1, allocator);
            utf8string_size+=3;
            utf8string[utf8string_size-4] = value;
            utf8string[utf8string_size-3] = value0;
            utf8string[utf8string_size-2] = value1;
            utf8string[utf8string_size-1] = '\0';
        }
        if ( character >= 0x10000 && character <= 0x10FFFF ) {
            value = character;
            value = value >> 18;
            value = value | 0xF0;
            value0 = character;
            value0 = value0 >> 12;
            value0 = value0 & 0xBF;
            value0 = value0 | 0x80;
            value1 = character;
            value1 = value1 >> 6;
            value1 = value1 & 0xBF;
            value1 = value1 | 0x80;
            value2 = character;
            value2 = value2 & 0x3F;
            value2 = value2 | 0x80;
            utf8string =
             aq_realloc(utf8string, utf8string_size+4,
                utf8string_size, AQChar, 1, allocator);
            utf8string_size+=4;
            utf8string[utf8string_size-5] = value;
            utf8string[utf8string_size-4] = value0;
            utf8string[utf8string_size-3] = value1;
            utf8string[utf8string_size-2] = value2;
            utf8string[utf8string_size-1] = '\0';
        }
        i++;
    }
    string = aqstring_new_from_c_string_with_allocator(utf8string,allocator);
    aq_free(utf8string,allocator);
    return string;
}

AQString aqstring_new_from_buffer(const AQChar* text, AQULong size_in_bytes) {
  return aqstring_new_from_buffer_with_allocator(text,size_in_bytes,aqmem_default_allocator());
}

AQString aqstring_new_from_buffer_with_allocator(const AQChar* text,
  AQULong size_in_bytes, AQAllocator allocator) {
    AQString string = aq_new(struct AQString_s, allocator);
    string->flag = AQStringFlag;
    string->destroyer = (AQDestroyerFuncType)aqstring_destroy;
    string->size_in_bytes = size_in_bytes;
    string->data = aq_make_c_array(string->size_in_bytes, AQChar, allocator);
    string->data[string->size_in_bytes-1] = '\0';
    int i = 0;
    while ( i < string->size_in_bytes-1 ) {
        string->data[i] = text[i];
        i++;
    }
    string->size_in_bytes -= 1; //not counting '\0'
    string->size_in_characters = -1;
    string->allocator = allocator;
    return string;
}

AQString aqstring_new_from_c_string(const AQChar* text) {
    return aqstring_new_from_c_string_with_allocator(text, aqmem_default_allocator());
}

AQString aqstring_new_from_c_string_with_allocator(const AQChar* text, AQAllocator allocator) {
    AQString string = aq_new(struct AQString_s, allocator);
    string->flag = AQStringFlag;
    string->destroyer = (AQDestroyerFuncType)aqstring_destroy;
    string->size_in_bytes = strlen(text);
    string->data =
     aq_make_c_array(string->size_in_bytes+1, AQChar, allocator);
    strcpy(string->data, text);
    string->size_in_characters = -1;
    string->allocator = allocator;
    return string;
}

AQString aqstring_new_from_two_strings(AQString a, AQString b) {
    return aqstring_new_from_two_strings_with_allocator(a,b,aqmem_default_allocator());
}

AQString aqstring_new_from_two_strings_with_allocator(AQString a,
  AQString b, AQAllocator allocator) {
    AQString string = aq_new(struct AQString_s, allocator);
    string->flag = AQStringFlag;
    string->destroyer = (AQDestroyerFuncType)aqstring_destroy;
    string->size_in_bytes = a->size_in_bytes + b->size_in_bytes;
    string->data = aq_make_c_array(string->size_in_bytes+1, AQChar, allocator);
    AQInt i = 0;
    AQInt j = 0;
    while ( j < a->size_in_bytes ) {
        string->data[i] = a->data[j];
        i++;
        j++;
    }
    j = 0;
    while ( j < b->size_in_bytes ) {
        string->data[i] = b->data[j];
        i++;
        j++;
    }
    string->data[i] = '\0';
    string->size_in_characters = -1;
    string->allocator = allocator;
    return string;
}

void aqstring_destroy(AQString string) {
    if ( string == NULL ) return;
    aq_free(string->data, string->allocator);
    aq_free(string, string->allocator);
}

AQAllocator aqstring_get_allocator(AQString string) {
    return string->allocator;
}

AQULong aqstring_get_size(AQString string) {
    return string->size_in_bytes;
}

AQULong aqstring_get_size_in_bytes(AQString string) {
    return string->size_in_bytes+1;
}

AQULong aqstring_get_length(AQString string) {
    AQInt i = 0;
    AQInt n = 0;
    AQSByte byte = 0;
    AQULong num_of_characters = 0;
    if ( string->size_in_characters == -1 ) {
        while ( i < string->size_in_bytes ) {
            byte = string->data[i];
            if ( byte >= 0 ) {
                if ( byte > 0 ) num_of_characters++;
            } else {
                n = 0;
                while (byte < 0) {
                    byte = byte << 1;
                    n++;
                }
                i += (n-1);
                num_of_characters++;
            }
            i++;
        }
        string->size_in_characters = num_of_characters;
    }
    return string->size_in_characters;
}

AQChar* aqstring_get_c_string(AQString string) {
    if ( string == NULL ) return NULL;
    return string->data;
}

AQUInt aqstring_get_character(AQString string, AQULong index, AQSByte* offset) {
    AQInt n = 0;
    AQSByte byte = 0;
    AQUInt byte0 = 0;
    AQUInt byte1 = 0;
    AQUInt byte2 = 0;
    AQUInt byte3 = 0;
    AQUInt value = 0;
loop:
    if ( index > string->size_in_bytes ) return 0;
    if ( index < 0 ) return 0;
    byte = string->data[index];
    if ( byte > 0 ) {
        n = 1;
    } else {
        n = 0;
        while (byte < 0) {
            byte = byte << 1;
            n++;
        }
        if ( n == 1 ) {
            index++;
            goto loop;
        }
       }
        if ( n == 1 ) {
            byte0 = string->data[index];
            value = byte0;
            *offset = 0;
        }
        if ( n == 2 ) {
            byte0 = string->data[index];
            byte0 = byte0 & 0x3F;
            byte0 = byte0 << 6;
            byte1 = string->data[index+1];
            byte1 = byte1 & 0x7F;
            value = byte0 | byte1;
            *offset = 1;
        }
        if ( n == 3 ) {
            byte0 = string->data[index];
            byte0 = byte0 & 0x1F;
            byte0 = byte0 << 12;
            byte1 = string->data[index+1];
            byte1 = byte1 & 0x7F;
            byte1 = byte1 << 6;
            byte2 = string->data[index+2];
            byte2 = byte2 & 0x7F;
            value = byte0 | byte1 | byte2;
            *offset = 2;
        }
        if ( n == 4 ) {
            byte0 = string->data[index];
            byte0 = byte0 & 0xF;
            byte0 = byte0 << 18;
            byte1 = string->data[index+1];
            byte1 = byte1 & 0x7F;
            byte1 = byte1 << 12;
            byte2 = string->data[index+2];
            byte2 = byte2 & 0x7F;
            byte2 = byte2 << 6;
            byte3 = string->data[index+3];
            byte3 = byte3 & 0x7F;
            value = byte0 | byte1 | byte2 | byte3;
            *offset = 3;
        }
    return value;
}

AQByte aqstring_get_byte(AQString string, AQULong index) {
    if ( index > string->size_in_bytes ) return 0;
    if ( index < 0 ) return 0;
    return (AQByte)string->data[index];
}

void aqstring_set_byte(AQString string, AQULong index, AQByte byte) {
    if ( index > string->size_in_bytes ) return;
    if ( index < 0 ) return;
    string->data[index] = (AQChar)byte;
}

AQString aqstring_append(AQString base_string, AQString appending_string) {
    AQString string =
     aq_add_strings(base_string, appending_string, base_string->allocator);
    aqstring_destroy(base_string);
    return string;
}

AQString aqstring_copy(AQString string) {
    if ( string == NULL ) return NULL;
    return
     aqstring_new_from_c_string_with_allocator(
       aqstring_get_c_string(string),string->allocator);
}

AQInt aqstring_are_equal(AQString a, AQString b) {
    return (strcmp(aqstring_get_c_string(a), aqstring_get_c_string(b)) == 0);
}

AQChar* aqstring_convert_to_c_string(AQString string) {
    if ( string == NULL ) return NULL;
    AQChar* str = string->data;
    aq_free(string,string->allocator);
    return str;
}

AQInt aqstring_print(AQString string) {
    return printf("%s", string->data);
}

AQUInt* aqstring_get_utf32_string(AQString string, AQULong* length) {
    AQInt i = 0;
    AQInt j = 0;
    AQSByte offset = 0;
    AQULong size_of_string = aqstring_get_size(string);
    AQUInt* utf32string =
     aq_make_c_array(size_of_string+1, AQUInt, string->allocator);
    while ( i < size_of_string ) {
        utf32string[j] = aqstring_get_character(string, i, &offset);
        i += offset;
        j++;
        i++;
    }
    utf32string[size_of_string] = '\0';
    *length = aqstring_get_length(string);
    return utf32string;
}

//For systems and environments that still don't support unicode
//mangles unicode characters into utf_<<bytes of the unicode character>>
AQString aqstring_get_string_for_ascii(AQString string) {
    AQInt i = 0;
    AQInt j = 0;
    AQInt x = 0;
    AQInt n = 0;
    AQULong str_size = 1;
    AQChar string_num[100];
    AQSByte byte = 0;
    AQByte ubyte = 0;
    AQString ascii_string = NULL;
    AQChar* str = aq_make_c_array(1, AQChar, string->allocator);
    str[0] = '\0';
    string_num[0] = '0';
    string_num[1] = '0';
    string_num[2] = '0';
    string_num[3] = '\0';
    while ( i < string->size_in_bytes  ) {
        if ( string->data[i] < 0 ) {
            str =
             aq_realloc(
               str, str_size+4, str_size, AQChar,
                1, string->allocator);
            str_size += 4;
            str[str_size-5] = 'u';
            str[str_size-4] = 't';
            str[str_size-3] = 'f';
            str[str_size-2] = '_';
            str[str_size-1] = '\0';
            byte = string->data[i];
            n = 0;
            while (byte < 0) {
                byte = byte << 1;
                n++;
            }
            ubyte = byte = string->data[i];
            x = 0;
            while ( x < n ) {
                snprintf(string_num, sizeof(string_num), "%u", ubyte);
                j = 0;
                while ( j < strlen(string_num) ) {
                 str =
                  aq_realloc(
                    str, str_size+1, str_size, AQChar,
                     1, string->allocator);
                 str_size++;
                 str[str_size-2] = string_num[j];
                 str[str_size-1] = '\0';
                 j++;
                }
                i++;
                if ( i >= string->size_in_bytes ) {
                    break;
                }
                ubyte = byte = string->data[i];
                x++;
            }
            i--;
        } else {
            str =
             aq_realloc(
               str, str_size+1, str_size, AQChar,
                1, string->allocator);
            str_size++;
            str[str_size-2] = string->data[i];
            str[str_size-1] = '\0';
        }
        i++;
    }
    ascii_string = aqstring_new_from_c_string_with_allocator(str,string->allocator);
    free(str);
    return ascii_string;
}

static int aqinternal_string_get_escape_character(int c) {
    switch (c) {
        case '\a':
            return 'a';
            break;
        case '\b':
            return 'b';
            break;
        case '\f':
            return 'f';
            break;
        case '\n':
            return 'n';
            break;
        case '\r':
            return 'r';
            break;
        case '\t':
            return 't';
            break;
        case '\v':
            return 'v';
            break;
        default:
            break;
    }
    return 'n';
}

AQString aqstring_swap_escape_sequences_with_characters(AQString string) {
    AQInt i = 0;
    AQInt j = 0;
    AQInt x = 0;
    AQInt c = 0;
    AQULong str_size = 0;
    AQULong str2_size = 0;
    AQString retstring = NULL;
    AQUInt* str = aqstring_get_utf32_string(string, &str_size);
    AQUInt* str2 = NULL;
    while ( i < str_size ) {
        c = str[i];
        if ( c == '\a' || c == '\b' || c == '\f' || c == '\n'
            || c == '\r' || c == '\t' || c == '\v' ) {
            j = 0;
            x = 0;
            str2_size = str_size+2;
            str2 = aq_make_c_array(str2_size, AQUInt, string->allocator);
            str2[str2_size-1] = '\0';
            while ( j < str2_size ) {
                if ( j != i && j != (i+1) && j != (str2_size-1) ) {
                    str2[j] = str[j+x];
                }

                if ( j == i ) {
                    str2[j] = '\\';
                }

                if ( j == i+1 ) {
                    str2[j] = aqinternal_string_get_escape_character(c);
                    x = -1;
                }

                if ( j == (str2_size-1) ) {
                    free(str);
                    str = str2;
                    str_size = str2_size;
                    i++;
                }
                j++;
            }
        }
        i++;
    }
    aqstring_destroy(string);
    retstring = aqstring_new_from_utf32_with_allocator(str, str_size, string->allocator);
    aq_free(str,string->allocator);
    return retstring;
}

AQString aqstring_expand(AQString string, AQULong expand_amount) {
    AQULong new_size = expand_amount + aqstring_get_size_in_bytes(string);
    string->data = aq_realloc(string->data,new_size,
        aqstring_get_size_in_bytes(string),AQChar,1,string->allocator);
    string->size_in_characters = -1;
    string->size_in_bytes = new_size-1;
    string->data[new_size-1] = '\0';
    if (string->data == NULL) return NULL;
    return string; 
}

void aqstring_iterate_bytes_with(AQByteIteratorFuncType iterator, AQString string) {
    AQULong i = 0;
    while ( i < string->size_in_bytes ) {
        iterator(string->data[i]);
        i++;
  }
} 
 
void aqstring_iterate_characters_with(AQCharacterIteratorFuncType iterator, AQString string) {
    AQULong i = 0;
    AQSByte offset = 0;
    aqstring_get_length(string);
    while ( i < string->size_in_characters ) {
        iterator(aqstring_get_character(string,i,&offset));
        i += offset;
        i++;
  }
}

AQList aqlist_new(void) {
    return aqlist_new_with_allocator(aqmem_default_allocator());
}

AQList aqlist_new_with_allocator(AQAllocator allocator) {
    AQList list = aq_new(struct AQList_s,allocator);
    list->flag = AQListFlag;
    list->destroyer = (AQDestroyerFuncType)aqlist_destroy;
    list->num_of_nodes = 0;
    list->first = NULL;
    list->last = NULL;
    list->allocator = allocator;
    return list;
}

AQList aqlist_new_from_array(AQAny array,
  AQGetDataFromArrayFuncType GetDataFromArrayFunc, AQULong size) {
    return aqlist_new_from_array_with_allocator(array,GetDataFromArrayFunc,size,aqmem_default_allocator());
}

AQList aqlist_new_from_array_with_allocator(AQAny array,
  AQGetDataFromArrayFuncType GetDataFromArrayFunc,
  AQULong size, AQAllocator allocator) {
    AQList list = aq_new_list(allocator);
    aqlist_copy_from_array(list, array, GetDataFromArrayFunc, size);
    return list;
}

AQListNode aqlist_new_node(AQList list, AQListNode before, AQListNode after, AQAny item) {
    AQListNode node = aq_new(struct AQListNode_s,list->allocator);
    node->flag = AQListNodeFlag;
    node->destroyer = (AQDestroyerFuncType)aqlist_destroy_node;
    node->before = before;
    node->after = after;
    node->data = item;
    node->string = NULL;
    return node;
}

void aqlist_destroy(AQList list) {
    if (list == NULL) return;
    while ( list->first != NULL ) {
        aqlist_destroy_node(list,list->first);
    }
    aq_free(list,list->allocator);
}

void aqlist_destroy_node(AQList list, AQListNode node) {
    if ( node == NULL ) return;
    if ( ( node->before == NULL ) || ( node->after == NULL ) ) {
        if ( ( node->before == NULL ) && ( node->after == NULL ) ) {
            list->first = NULL;
            list->last = NULL;
        }
        if ( ( node->before == NULL ) && ( node->after != NULL ) ) {
            list->first = node->after;
            list->first->before = NULL;
        }
        if ( ( node->before != NULL ) && ( node->after == NULL ) ) {
            list->last = node->before;
            list->last->after = NULL;
        }
    }
    if ( ( node->before != NULL ) && ( node->after != NULL ) ) {
        node->before->after = node->after;
        node->after->before = node->before;
    }
    list->num_of_nodes--;
    if ( node->string != NULL ) aqstring_destroy(node->string);
    aq_free(node,list->allocator);
}

void aqlist_destroy_node_with_index(AQList list, AQULong index) {
    AQListNode node = aqlist_get_node(list, index);
    aqlist_destroy_node(list, node);
}

AQListNode aqlist_destroy_node_get_next(AQList list, AQListNode node) {
    AQListNode next_node = aqlist_next_node(node);
    aqlist_destroy_node(list, node);
    return next_node;
}

void aqlist_destroy_all_nodes(AQList list) {
    while ( list->first != NULL ) {
        aqlist_destroy_node(list,list->first);
    }
}

AQListNode aqlist_add_item(AQList list, AQAny item) {
    if ( list->num_of_nodes == 0 ) {
        list->first =
         aq_new(struct AQListNode_s,list->allocator);
        list->first->before = NULL;
        list->first->after = NULL;
        list->last = list->first;
    } else {
        list->last->after =
         aq_new(struct AQListNode_s,list->allocator);
        list->last->after->before = list->last;
        list->last->after->after = NULL;
        list->last = list->last->after;
    }
    list->last->data = item;
    list->last->string = NULL;
    list->num_of_nodes++;
    return list->last;
}

AQListNode aqlist_add_node(AQList list, AQListNode node) {
    return aqlist_add_item(list, node->data);
}

void aqlist_node_swap(AQList list, AQListNode node_a, AQListNode node_b) {
    AQListNode before = (node_a->before == node_b) ? node_a : node_a->before;
    AQListNode after = (node_a->after == node_b) ? node_a : node_a->after;
    node_a->before = (node_b->before == node_a) ? node_b : node_b->before;
    node_a->after = (node_b->after == node_a) ? node_b : node_b->after;
    if ( node_a->before != NULL ) node_a->before->after = node_a;
    if ( node_a->after != NULL ) node_a->after->before = node_a;
    if ( node_a->before == NULL ) list->first = node_a;
    if ( node_a->after == NULL ) list->last = node_a;
    node_b->before = before;
    node_b->after = after;
    if ( node_b->before != NULL ) node_b->before->after = node_b;
    if ( node_b->after != NULL ) node_b->after->before = node_b;
    if ( node_b->before == NULL ) list->first = node_b;
    if ( node_b->after == NULL ) list->last = node_b;
}

void aqlist_item_swap(AQListNode node_a, AQListNode node_b) {
    AQAny item = node_a->data;
    node_a->data = node_b->data;
    node_b->data = item;
}

void aqlist_insert_a_to_b(AQList list, AQListNode node_a, AQListNode node_b) {
    if ( node_a->before != NULL ) node_a->before->after = node_a->after;
    if ( node_a->after != NULL ) node_a->after->before = node_a->before;
    if ( node_a->before == NULL ) list->first = node_a->after;
    if ( node_a->after == NULL ) list->last = node_a->before;
    if ( node_b->before == NULL ) list->first = node_a;
    if ( node_b->after == NULL ) list->last = node_a;
    node_a->before = node_b->before;
    node_a->after = node_b;
    if ( node_b->before != NULL ) node_b->before->after = node_a;
    node_b->before = node_a;
}

AQListNode aqlist_move_node(AQListNode node, AQList list_a, AQList list_b) {
    AQListNode new_node = aqlist_add_node(list_b, node);
    aqlist_destroy_node(list_a, node);
    return new_node;
}

void aqlist_copy(AQList list_a, AQList list_b) {
    AQListNode node = aqlist_first_node(list_a);
    while ( node != NULL ) {
        aqlist_add_node(list_b, node );
        node = aqlist_next_node(node);
    }
}

void aqlist_copy_from_array(AQList list, AQAny array,
  AQGetDataFromArrayFuncType GetDataFromArrayFunc, AQULong size) {
    AQAny data = NULL;
    int i = 0;
    while ( i < size ) {
        data = GetDataFromArrayFunc(array,i);
        aqlist_add_item(list, data);
        i++;
    }
}

void aqlist_set_item(AQListNode node, AQAny item) {
    node->data = item;
}

AQAny aqlist_get_item(AQListNode node) {
    if ( node == NULL ) return NULL;
    return node->data;
}

static void aqinternal_list_set_string(AQListNode node, AQString string) {
    if ( node == NULL ) return;
    node->string = string;
}

static AQString aqinternal_list_get_string(AQListNode node) {
    if ( node == NULL ) return NULL;
    return node->string;
}

AQListNode aqlist_next_node(AQListNode node) {
    if ( node == NULL ) return NULL;
    return node->after;
}

AQListNode aqlist_previous_node(AQListNode node) {
    if ( node == NULL ) return NULL;
    return node->before;
}

AQListNode aqlist_first_node(AQList list) {
    if ( list == NULL ) return NULL;
    return list->first;
}

AQListNode aqlist_last_node(AQList list) {
    if ( list == NULL ) return NULL;
    return list->last;
}

AQLong aqlist_num_of_nodes(AQList list) {
    if ( list == NULL ) return -1;
    return list->num_of_nodes;
}

AQListNode aqlist_get_node(AQList list, AQULong index) {
    AQULong i = 0;
    AQListNode node = list->first;
    while ( i < index ) {
        if ( node == NULL ) return NULL;
        node = node->after;
        i++;
    }
    return node;
}

AQULong aqlist_get_index(AQList list, AQListNode node) {
    AQULong i = 0;
    AQListNode node2 = list->first;
    while ( node2 != NULL ) {
        if ( node == node2 ) break;
        node2 = node2->after;
        i++;
    }
    return i;
}

AQListNode aqlist_next_node_after_n(AQListNode node, AQULong n) {
    AQULong i = 0;
    while ( i < n ) {
        if ( node == NULL ) return NULL;
        node = node->after;
        i++;
    }
    return node;
}

AQListNode aqlist_previous_node_after_n(AQListNode node, AQULong n) {
    AQULong i = 0;
    while ( i < n ) {
        if ( node == NULL ) return NULL;
        node = node->before;
        i++;
    }
    return node;
}

void aqlist_iterate_with(AQIteratorFuncType iterator, AQList list) {
    AQListNode node = list->first;
    AQAny item = NULL;
    while ( node != NULL ) {
        item = aqlist_get_item(node);
        iterator(item);
        node = aqlist_next_node(node);
    }
}

AQStack aqstack_new(void) {
    return aqstack_new_with_allocator(aqmem_default_allocator());
}

AQStack aqstack_new_with_allocator(AQAllocator allocator) {
    AQStack stack = aq_new(struct AQStack_s,allocator);
    stack->flag = AQStackFlag;
    stack->destroyer = (AQDestroyerFuncType)aqstack_destroy;
    stack->list = aq_new_list(allocator);
    return stack;
}

void aqstack_destroy(AQStack stack) {
    if (stack != NULL) {
      AQAllocator allocator = stack->list->allocator;
      aqlist_destroy(stack->list);
      aq_free(stack,allocator);
    }
}

void aqstack_push_item(AQStack stack, AQAny item) {
    aqlist_add_item(stack->list, item);
}

AQAny aqstack_pop_item(AQStack stack) {
    if ( aqstack_is_empty(stack) ) return NULL;
    AQAny item = aqlist_get_item(aqlist_last_node(stack->list));
    aqlist_destroy_node(stack->list, aqlist_last_node(stack->list));
    return item;
}

AQAny aqstack_peek_item(AQStack stack) {
    return aqlist_get_item(aqlist_last_node(stack->list));
}

AQInt aqstack_is_empty(AQStack stack) {
    return (aqlist_num_of_nodes(stack->list) == 0) ? 1 : 0;
}

AQList aqstack_get_list(AQStack stack) {
    return stack->list;
}

AQListNode aqstack_get_list_node(AQStack stack) {
    return aqlist_last_node(stack->list);
}

AQStackBuffer aqstackbuffer_new(void) {
    return aqstackbuffer_new_with_allocator(aqmem_default_allocator());
}

AQStackBuffer aqstackbuffer_new_with_allocator(AQAllocator allocator) {
    AQStackBuffer stack = aq_new(struct AQStackBuffer_s,allocator);
    stack->flag = AQStackBufferFlag;
    stack->destroyer = (AQDestroyerFuncType)aqstackbuffer_destroy;
    stack->rate = 50;
    stack->index = 0;
    stack->count = 0;
    stack->buffer = aq_new_array(allocator);
    return stack;
}

void aqstackbuffer_destroy(AQStackBuffer stack) {
    if (stack != NULL) {
        AQAllocator allocator = stack->buffer->allocator;
        aqarray_destroy(stack->buffer);
        aq_free(stack,allocator);
    }
}

void aqstackbuffer_set_rate(AQStackBuffer stack, AQULong rate) {
    stack->rate = rate/2;
}

void aqstackbuffer_push_item(AQStackBuffer stack, AQAny item) {
    if (stack->index >= stack->rate * stack->count) {
        stack->count += 2;
        aqarray_add_space(stack->buffer,stack->rate*2);
    }
    aqarray_set_item(stack->buffer, stack->index, item);
    stack->index++;
}

AQAny aqstackbuffer_pop_item(AQStackBuffer stack) {
    if ( stack->index == 0 ) return NULL;
    AQAny item = aqarray_get_item(stack->buffer,stack->index-1);
    aqarray_set_item(stack->buffer, stack->index-1, NULL);
    if ((stack->rate*2) + stack->index < stack->rate * stack->count) {
        aqarray_remove_space(stack->buffer,stack->rate);
        stack->count--;
    }
    stack->index--;
    return item;
}

AQAny aqstackbuffer_peek_item(AQStackBuffer stack) {
    return aqarray_get_item(stack->buffer,stack->index-1);
}

AQInt aqstackbuffer_is_empty(AQStackBuffer stack) {
    return stack->index == 0 ? 1 : 0;
}

AQArray aqstackbuffer_get_array(AQStackBuffer stack) {
    return stack->buffer;
}

AQMultiTypeArray aqmta_new(void) {
    return aqmta_new_with_allocator(aqmem_default_allocator());
}

AQMultiTypeArray aqmta_new_with_allocator(AQAllocator allocator) {
    AQMultiTypeArray mta = aq_new(struct AQMultiTypeArray_s,allocator);
    mta->flag = AQMultiTypeArrayFlag;
    mta->destroyer = (AQDestroyerFuncType)aqmta_destroy;
    mta->allocator = allocator;
    int i = 0; 
    while (i < 11) {
      mta->item_arrays[i] = NULL;
      mta->num_of_items[i] = 0; 
      i++;
    } 
    return mta;
}

void aqmta_destroy(AQMultiTypeArray mta) {
    if (mta != NULL) {
      AQAllocator allocator = mta->allocator;
      int i = 0; 
      while (i < 11) {
        if (mta->item_arrays[i] != NULL) aq_free(mta->item_arrays[i],allocator);
        i++;
    } 
      aq_free(mta,allocator);
    }
}

#define aq_mta_container_set_case(type,mta,container,index)\
   case type##Flag:\
    container.type##Val = ((type*)(mta->item_arrays[type##Flag-1]))[index];\
   break

#define aq_mta_set_container_from_mta(mta,container,index)\
  switch (container.type) {\
    aq_mta_container_set_case(AQByte,mta,container,index);\
    aq_mta_container_set_case(AQSByte,mta,container,index);\
    aq_mta_container_set_case(AQShort,mta,container,index);\
    aq_mta_container_set_case(AQUShort,mta,container,index);\
    aq_mta_container_set_case(AQInt,mta,container,index);\
    aq_mta_container_set_case(AQUInt,mta,container,index);\
    aq_mta_container_set_case(AQLong,mta,container,index);\
    aq_mta_container_set_case(AQULong,mta,container,index);\
    aq_mta_container_set_case(AQFloat,mta,container,index);\
    aq_mta_container_set_case(AQDouble,mta,container,index);\
    aq_mta_container_set_case(AQAny,mta,container,index);\
    default:\
    break;\
 }  

#define aq_mta_define_add(type)\
 AQInt aqmta_add_item_##type(AQMultiTypeArray mta, type value) {\
    AQULong num_of_items = mta->num_of_items[type##Flag-1];\
    if (mta->item_arrays[type##Flag-1] == NULL) {\
      mta->item_arrays[type##Flag-1] = aq_make_c_array(1,type,mta->allocator);\
    } else {\
      mta->item_arrays[type##Flag-1] = aq_realloc(mta->item_arrays[type##Flag-1],\
        num_of_items+1,num_of_items,type,1,mta->allocator);\
    }\
    if (mta->item_arrays[type##Flag-1] == NULL) return 0;\
    ((type*)mta->item_arrays[type##Flag-1])[num_of_items] = value;\
    mta->num_of_items[type##Flag-1] = num_of_items + 1;\
    return 1;\
 }\
 
 aq_mta_define_add(AQByte);
 aq_mta_define_add(AQSByte);
 aq_mta_define_add(AQShort);
 aq_mta_define_add(AQUShort);
 aq_mta_define_add(AQInt);
 aq_mta_define_add(AQUInt);
 aq_mta_define_add(AQLong);
 aq_mta_define_add(AQULong);
 aq_mta_define_add(AQFloat);
 aq_mta_define_add(AQDouble);
 aq_mta_define_add(AQAny);
 
 #define aq_mta_define_remove_item(type)\
 AQInt aqmta_remove_item_##type(AQMultiTypeArray mta) {\
    return aqmta_remove_space_##type(mta,1);\
 }\
 
 aq_mta_define_remove_item(AQByte);
 aq_mta_define_remove_item(AQSByte);
 aq_mta_define_remove_item(AQShort);
 aq_mta_define_remove_item(AQUShort);
 aq_mta_define_remove_item(AQInt);
 aq_mta_define_remove_item(AQUInt);
 aq_mta_define_remove_item(AQLong);
 aq_mta_define_remove_item(AQULong);
 aq_mta_define_remove_item(AQFloat);
 aq_mta_define_remove_item(AQDouble);
 aq_mta_define_remove_item(AQAny); 
 
 #define aq_mta_define_add_space(type)\
  AQInt aqmta_add_space_##type(AQMultiTypeArray mta, AQULong space_to_add) {\
    if (mta->item_arrays[type##Flag-1] == NULL) {\
      mta->num_of_items[type##Flag-1] = space_to_add;\
      mta->item_arrays[type##Flag-1] = aq_make_c_array(space_to_add,type,mta->allocator);\
    } else {\
      AQULong num_of_items = mta->num_of_items[type##Flag-1] += space_to_add;\
      mta->item_arrays[type##Flag-1] = aq_realloc(mta->item_arrays[type##Flag-1],\
        num_of_items,num_of_items-space_to_add,type,1,mta->allocator);\
    }\
    if (mta->item_arrays[type##Flag-1] == NULL) return 0;\
    return 1;\
 }\
 
 aq_mta_define_add_space(AQByte);
 aq_mta_define_add_space(AQSByte);
 aq_mta_define_add_space(AQShort);
 aq_mta_define_add_space(AQUShort);
 aq_mta_define_add_space(AQInt);
 aq_mta_define_add_space(AQUInt);
 aq_mta_define_add_space(AQLong);
 aq_mta_define_add_space(AQULong);
 aq_mta_define_add_space(AQFloat);
 aq_mta_define_add_space(AQDouble);
 aq_mta_define_add_space(AQAny);

#define aq_mta_define_remove_space(type)\
  AQInt aqmta_remove_space_##type(AQMultiTypeArray mta, AQULong space_to_remove) {\
    if (mta->item_arrays[type##Flag-1] == NULL) {\
     return 0;\
    } else {\
      AQULong num_of_items = mta->num_of_items[type##Flag-1] -= space_to_remove;\
      mta->item_arrays[type##Flag-1] = aq_realloc(mta->item_arrays[type##Flag-1],\
        num_of_items,num_of_items,type,1,mta->allocator);\
    }\
    if (mta->item_arrays[type##Flag-1] == NULL) return 0;\
    return 1;\
 }\
 
 aq_mta_define_remove_space(AQByte);
 aq_mta_define_remove_space(AQSByte);
 aq_mta_define_remove_space(AQShort);
 aq_mta_define_remove_space(AQUShort);
 aq_mta_define_remove_space(AQInt);
 aq_mta_define_remove_space(AQUInt);
 aq_mta_define_remove_space(AQLong);
 aq_mta_define_remove_space(AQULong);
 aq_mta_define_remove_space(AQFloat);
 aq_mta_define_remove_space(AQDouble);
 aq_mta_define_remove_space(AQAny);

#define aq_mta_define_get(type)\
 type aqmta_get_item_##type(AQMultiTypeArray mta, AQULong index) {\
     if (mta->item_arrays[type##Flag-1] != NULL) {\
         if (mta->num_of_items[type##Flag-1] > index) {\
             return ((type*)mta->item_arrays[type##Flag-1])[index];\
         }\
     }\
     return 0;\
 }\
 
 aq_mta_define_get(AQByte);
 aq_mta_define_get(AQSByte);
 aq_mta_define_get(AQShort);
 aq_mta_define_get(AQUShort);
 aq_mta_define_get(AQInt);
 aq_mta_define_get(AQUInt);
 aq_mta_define_get(AQLong);
 aq_mta_define_get(AQULong);
 aq_mta_define_get(AQFloat);
 aq_mta_define_get(AQDouble);
 aq_mta_define_get(AQAny);
 
#define aq_mta_define_set(type)\
 void aqmta_set_item_##type(AQMultiTypeArray mta, AQULong index, type value) {\
     if (mta->item_arrays[type##Flag-1] != NULL) {\
         if (mta->num_of_items[type##Flag-1] > index) {\
              ((type*)mta->item_arrays[type##Flag-1])[index] = value;\
         }\
     }\
 }\
 
 aq_mta_define_set(AQByte);
 aq_mta_define_set(AQSByte);
 aq_mta_define_set(AQShort);
 aq_mta_define_set(AQUShort);
 aq_mta_define_set(AQInt);
 aq_mta_define_set(AQUInt);
 aq_mta_define_set(AQLong);
 aq_mta_define_set(AQULong);
 aq_mta_define_set(AQFloat);
 aq_mta_define_set(AQDouble);
 aq_mta_define_set(AQAny);
 
AQULong aqmta_get_num_of_items(AQMultiTypeArray mta, AQTypeFlag type_flag) {
    if (type_flag < 0 || type_flag > 10) return 0;
    return mta->num_of_items[type_flag];
}
 
AQULong aqmta_get_num_of_items_all_types(AQMultiTypeArray mta) {
    int i = 0;
    AQULong num_of_items = 0;
    while (i < 11) {
        num_of_items += mta->num_of_items[i];
        i++;
    }  
    return num_of_items;
}
 
AQMTAContainer aqmta_get_container(AQMultiTypeArray mta, AQULong index) {
    int i = 0;
    AQMTAContainer container;
    container.flag = AQMTAContainerFlag;
    while (i < 11) {
        if (index >= mta->num_of_items[i]) {
            index = index - mta->num_of_items[i];
            i++;
        } else if (index >= 0) {
            container.type = i+1;
            aq_mta_set_container_from_mta(mta,container,index);
            return container;   
        }
    }    
    container.type = AQEmptyFlag;
    container.AQByteVal = 0;
    return container;
}   
 
void aqmta_iterate_all_types_with(AQIteratorFuncType iterator, AQMultiTypeArray mta) {
    int i = 0;
    int j = 0;
    AQMTAContainer container;
    while (i < 11) {
        j = 0;
        container.type = i+1;
        while (j < mta->num_of_items[i]) {
            aq_mta_set_container_from_mta(mta,container,j);
            iterator((void*)&container);
            j++;
        }
        i++;
    }
}    

AQMTAStackBuffer aqmtastackbuffer_new(void) {
    return aqmtastackbuffer_new_with_allocator(aqmem_default_allocator());
}

AQMTAStackBuffer aqmtastackbuffer_new_with_allocator(AQAllocator allocator) {
    AQMTAStackBuffer stack = aq_new(struct AQMTAStackBuffer_s,allocator);
    stack->flag = AQMTAStackBufferFlag;
    stack->destroyer = (AQDestroyerFuncType)aqmtastackbuffer_destroy;
    stack->data_buffer = aq_new_mta(allocator);
    stack->type_buffer = NULL;
    stack->type_buffer_size = 0;
    stack->type_index = 0;
    stack->type_count = 0;
    int i = 0; 
    while (i < 11) {
        stack->indexes[i] = 0; 
        i++;
    }
    i = 0;
    while (i < 11) {
        stack->counts[i] = 0; 
        i++;
    }  
    stack->rate = 50;
    return stack;
}

void aqmtastackbuffer_destroy(AQMTAStackBuffer stack) {
    if (stack == NULL) return;
    AQAllocator allocator = stack->data_buffer->allocator;
    if (stack->data_buffer != NULL) aqmta_destroy(stack->data_buffer);
    if (stack->type_buffer != NULL) aq_free(stack->type_buffer,allocator);
    aq_free(stack,allocator);
}

void aqmtastackbuffer_set_rate(AQMTAStackBuffer stack, AQULong rate) {
    stack->rate = rate/2;
}

static void aqinternal_mtastackbuffer_add_space_to_type_buffer(
  AQMTAStackBuffer stack,
  AQULong space_to_add) {
    AQAllocator allocator = stack->data_buffer->allocator;   
    AQULong size = stack->type_buffer_size += space_to_add;
    if (stack->type_buffer == NULL) {
        stack->type_buffer = aq_make_c_array(size,AQTypeFlag,allocator);
    } else {
        stack->type_buffer = aq_realloc(stack->type_buffer,
         size,size-space_to_add,AQTypeFlag,1,allocator);
    }
}

static void aqinternal_mtastackbuffer_remove_space_from_type_buffer(
  AQMTAStackBuffer stack,
  AQULong space_to_remove) {
    AQAllocator allocator = stack->data_buffer->allocator;   
    AQULong size = stack->type_buffer_size -= space_to_remove;
    if (stack->type_buffer == NULL) {
        //do nothing
    } else {
        stack->type_buffer = aq_realloc(stack->type_buffer,
         size,size,AQTypeFlag,1,allocator);
    }
}

#define aq_mtastackbuffer_define_push_item(type)\
 void aqmtastackbuffer_push_item##type(AQMTAStackBuffer stack, type item) {\
     AQULong index = stack->indexes[type##Flag-1];\
     AQULong count = stack->counts[type##Flag-1];\
     if (index >= stack->rate * count) {\
         stack->counts[type##Flag-1] += 2;\
         aq_mta_add_space(type,stack->data_buffer,stack->rate*2);\
     }\
     if (stack->type_index >= stack->rate * stack->type_count) {\
         stack->type_count += 2;\
         aqinternal_mtastackbuffer_add_space_to_type_buffer(stack,stack->rate*2);\
     }\
     aq_mta_set_item(type,stack->data_buffer,index,item);\
     stack->type_buffer[stack->type_index] = type##Flag;\
     stack->indexes[type##Flag-1]++;\
     stack->type_index++;\
    }
    
aq_mtastackbuffer_define_push_item(AQByte);
aq_mtastackbuffer_define_push_item(AQSByte); 
aq_mtastackbuffer_define_push_item(AQShort); 
aq_mtastackbuffer_define_push_item(AQUShort); 
aq_mtastackbuffer_define_push_item(AQInt); 
aq_mtastackbuffer_define_push_item(AQUInt); 
aq_mtastackbuffer_define_push_item(AQLong); 
aq_mtastackbuffer_define_push_item(AQULong); 
aq_mtastackbuffer_define_push_item(AQFloat); 
aq_mtastackbuffer_define_push_item(AQDouble); 
aq_mtastackbuffer_define_push_item(AQAny);                                         
    
AQTypeFlag aqmtastackbuffer_peek_type(AQMTAStackBuffer stack) {
   if (stack->type_index == 0) return 0;
   return stack->type_buffer[stack->type_index-1];
}    

#define aq_mtastackbuffer_define_pop_item(type)\
 type aqmtastackbuffer_pop_item##type(AQMTAStackBuffer stack) {\
     if ( stack->type_index == 0 ) return 0;\
     if ( stack->type_buffer[stack->type_index-1] != type##Flag ) return 0;\
     type item = aq_mta_get_item(type,stack->data_buffer,stack->indexes[type##Flag-1]-1);\
     aq_mta_set_item(type,stack->data_buffer, stack->indexes[type##Flag-1]-1, 0);\
     if ((stack->rate*2) + stack->indexes[type##Flag-1] < stack->rate * stack->counts[type##Flag-1]) {\
         aq_mta_remove_space(type,stack->data_buffer,stack->rate);\
         stack->counts[type##Flag-1]--;\
     }\
     if ((stack->rate*2) + stack->type_index < stack->rate * stack->type_count) {\
         aqinternal_mtastackbuffer_remove_space_from_type_buffer(stack,stack->rate);\
         stack->type_count--;\
     }\
     stack->indexes[type##Flag-1]--;\
     stack->type_index--;\
     return item;\
 }\
 
aq_mtastackbuffer_define_pop_item(AQByte);
aq_mtastackbuffer_define_pop_item(AQSByte); 
aq_mtastackbuffer_define_pop_item(AQShort); 
aq_mtastackbuffer_define_pop_item(AQUShort); 
aq_mtastackbuffer_define_pop_item(AQInt); 
aq_mtastackbuffer_define_pop_item(AQUInt); 
aq_mtastackbuffer_define_pop_item(AQLong); 
aq_mtastackbuffer_define_pop_item(AQULong); 
aq_mtastackbuffer_define_pop_item(AQFloat); 
aq_mtastackbuffer_define_pop_item(AQDouble); 
aq_mtastackbuffer_define_pop_item(AQAny);                                         

#define aq_mtastackbuffer_define_peek_item(type)\
 type aqmtastackbuffer_peek_item##type(AQMTAStackBuffer stack) {\
     if ( stack->type_index == 0 ) return 0;\
     if ( stack->type_buffer[stack->type_index-1] != type##Flag ) return 0;\
     return aq_mta_get_item(type,stack->data_buffer,stack->indexes[type##Flag-1]-1);\
 }\
 
aq_mtastackbuffer_define_peek_item(AQByte);
aq_mtastackbuffer_define_peek_item(AQSByte); 
aq_mtastackbuffer_define_peek_item(AQShort); 
aq_mtastackbuffer_define_peek_item(AQUShort); 
aq_mtastackbuffer_define_peek_item(AQInt); 
aq_mtastackbuffer_define_peek_item(AQUInt); 
aq_mtastackbuffer_define_peek_item(AQLong); 
aq_mtastackbuffer_define_peek_item(AQULong); 
aq_mtastackbuffer_define_peek_item(AQFloat); 
aq_mtastackbuffer_define_peek_item(AQDouble); 
aq_mtastackbuffer_define_peek_item(AQAny);    

AQInt aqmtastackbuffer_is_empty(AQMTAStackBuffer stack) {
    return stack->type_index == 0 ? 1 : 0;
}

AQMultiTypeArray aqmtastackbuffer_get_mta(AQMTAStackBuffer stack) {
    return stack->data_buffer;
}

AQStore aqstore_new(void) {
    return aqstore_new_with_allocator(aqmem_default_allocator());
}

AQStore aqstore_new_with_allocator(AQAllocator allocator) {
    AQStore store = aq_new(struct AQStore_s,allocator);
    store->flag = AQStoreFlag;
    store->destroyer = (AQDestroyerFuncType)aqstore_destroy;
    store->dictionary = NULL;
    store->items = NULL;
    store->allocator = allocator;
    return store;
}

AQAllocator aqstore_get_allocator(AQStore store) {
    if ( store->items == NULL ) return NULL;
    return store->allocator;
}

static AQStoreBinaryNode* aqinternal_store_new_binary_node(AQAllocator allocator) {
    AQInt size = 2;
    AQStoreBinaryNode* binary_node = NULL;
    binary_node = aq_make_c_array(size, AQStoreBinaryNode, allocator);
    AQInt i = 0;
    while ( i < size ) {
        binary_node[i].node = NULL;
        binary_node[i].next_binary_node = NULL;
        i++;
    }
    return binary_node;
}

static void aqinternal_store_destroy_binary_nodes(AQStoreBinaryNode* binary_nodes, AQAllocator allocator) {
    int i = 0;
    while ( i < 2 ) {
        if ( binary_nodes[i].next_binary_node != NULL )  {
            aqinternal_store_destroy_binary_nodes(binary_nodes[i].next_binary_node,allocator);
            binary_nodes[i].next_binary_node = NULL;
        }
        i++;
    }
    aq_free(binary_nodes,allocator);
}

void aqstore_destroy(AQStore store) {
    if (store == NULL) return;
    if (store->dictionary != NULL) aqinternal_store_destroy_binary_nodes(store->dictionary,store->allocator);
    if (store->items != NULL) aqlist_destroy(store->items);
    aq_free(store,store->allocator);
}

#define aq_internal_store_loop_start \
AQInt i = 0;\
AQInt j = 0;\
AQInt value = 0;\
AQSByte byte = 0;\
AQInt size = (int) strlen(label);\
AQInt size_in_bits = sizeof(AQChar) * CHAR_BIT;\
AQStoreBinaryNode* current_binary_node = NULL;\
if ( store->dictionary == NULL ) store->dictionary = aqinternal_store_new_binary_node(store->allocator);\
current_binary_node = store->dictionary;\
while ( i < size ) {\
  byte = label[i];\
  j = 0;\
  while ( j < size_in_bits ) {\
    value = (byte < 0) ? 1 : 0;\
    byte = byte << 1;\
    if ( i == ( size - 1 ) && j == ( size_in_bits - 1 ) )

#define aq_internal_store_loop_phase_2 \
 else {\
  if ( current_binary_node[value].next_binary_node == NULL )

#define aq_internal_store_loop_phase_3(retval) \
current_binary_node = current_binary_node[value].next_binary_node;\
if ( current_binary_node == NULL ) return retval

#define aq_internal_store_loop_end(retval) \
}\
j++;\
}\
i++;\
}\
return retval

static AQInt aqinternal_store_build_node(AQStore store,
   const AQChar* label, AQListNode node) {
   aq_internal_store_loop_start {
        current_binary_node[value].node = node;
        aqinternal_list_set_string(node,
         aqstring_new_from_c_string_with_allocator(label,store->allocator)
          );
        return 1;
      } aq_internal_store_loop_phase_2 {
         current_binary_node[value].next_binary_node =
          aqinternal_store_new_binary_node(store->allocator);
      }
      aq_internal_store_loop_phase_3(0);
      aq_internal_store_loop_end(0);
}

static AQListNode aqinternal_store_retrieve_node(AQStore store, const AQChar* label) {
  aq_internal_store_loop_start {
        return current_binary_node[value].node;
      } aq_internal_store_loop_phase_2 {
        return NULL;
      }
      aq_internal_store_loop_phase_3(NULL);
      aq_internal_store_loop_end(NULL);
}

static AQListNode aqinternal_store_remove_node(AQStore store, const AQChar* label) {
  aq_internal_store_loop_start {
      AQListNode retnode = current_binary_node[value].node;
      aqstring_destroy(aqinternal_list_get_string(current_binary_node[value].node));
      aqinternal_list_set_string(current_binary_node[value].node, NULL);
      current_binary_node[value].node = NULL;
      return retnode;
      } aq_internal_store_loop_phase_2 {
        return NULL;
      }
      aq_internal_store_loop_phase_3(NULL);
      aq_internal_store_loop_end(NULL);
}

static AQListNode aqinternal_store_add_item(AQStore store, AQAny item) {
    if ( store->items == NULL ) store->items = aq_new_list(store->allocator);
    return aqlist_add_item(store->items, item);
}

static AQInt aqinternal_store_set_item(AQStore store, AQAny item, const AQChar* label) {
    AQListNode node = aqinternal_store_retrieve_node(store,label);
    if ( node == NULL ) return 0;
    aqlist_set_item(node, item);
    return 1;
}

AQInt aqstore_add_item(AQStore store, AQAny item, const AQChar* label) {
    if ( !(aqstore_item_exists(store, label)) ) {
        AQListNode node = aqinternal_store_add_item(store, item);
        if ( !aqinternal_store_build_node(store, label, node) ) return 0;
    } else {
        return aqinternal_store_set_item(store, item, label);
    }
    return 1;
}

AQInt aqstore_remove_item(AQStore store, const AQChar* label) {
    AQListNode node = aqinternal_store_remove_node(store,label);
    if ( node == NULL ) return 0;
    aqlist_destroy_node(store->items,node);
    return 1;
}

AQAny aqstore_get_item(AQStore store, const AQChar* label) {
    AQListNode node = aqinternal_store_retrieve_node(store,label);
    if ( node == NULL ) return NULL;
    return aqlist_get_item(node);
}

AQAny aqstore_get_item_with_character(AQStore store, AQInt character) {
    const AQInt* text = &character;
    AQString string = aqstring_new_from_utf32(text,1);
    AQChar* c_string = aqstring_get_c_string(string);
    AQAny item = aqstore_get_item(store,c_string);
    free(c_string);
    return item;
}

AQInt aqstore_item_exists(AQStore store, const AQChar* label) {
    AQListNode node = aqinternal_store_retrieve_node(store,label);
    if ( node == NULL ) return 0;
    return 1;
}

AQInt aqstore_num_of_items(AQStore store) {
    return aqlist_num_of_nodes(aqstore_get_list(store));
}

AQInt aqstore_is_store_empty(AQStore store) {
    return (aqlist_num_of_nodes(store->items) == 0) ? 1 : 0;
}

AQInt aqstore_add_item_to_list(AQStore store, AQAny item) {
    if ( aqinternal_store_add_item(store, item) == NULL ) return 0;
    return 1;
}

AQList aqstore_get_list(AQStore store) {
    if ( store->items == NULL ) store->items = aqlist_new();
    return store->items;
}

AQString aqstore_label_from_list_node(AQListNode node) {
    return aqinternal_list_get_string(node);
}

void aqstore_iterate_store_with(AQIteratorFuncType iterator, AQStore store) {
    if (store->items != NULL) aqlist_iterate_with(iterator, store->items);
}

AQAny aqany_new(AQAny any, AQULong size) {
    AQByte* ptr = malloc(size);
    memcpy(ptr, any, size);
    return ptr;
}

void aqany_destroy(AQAny any) {
    free(any);
}

AQFloat aqmath_dot2(const AQFloat2 v0, const AQFloat2 v1) {
    const AQFloat2 v2 = v0 * v1;
    return aq_x(v2) + aq_y(v2);
}

AQFloat aqmath_dot3(const AQFloat3 v0, const AQFloat3 v1) {
    const AQFloat3 v2 = v0 * v1;
    return aq_x(v2) + aq_y(v2) + aq_z(v2);
}

AQFloat aqmath_dot4(const AQFloat4 v0, const AQFloat4 v1) {
    const AQFloat4 v2 = v0 * v1;
    return aq_x(v2) + aq_y(v2) + aq_z(v2) + aq_w(v2);
}

AQFloat aqmath_length2(const AQFloat2 v0) {
    return sqrtf(aqmath_dot2(v0,v0));
}

AQFloat aqmath_length3(const AQFloat3 v0) {
    return sqrtf(aqmath_dot3(v0,v0));
}

AQFloat aqmath_length4(const AQFloat4 v0) {
    return sqrtf(aqmath_dot4(v0,v0));
}

AQFloat aqmath_distance2(const AQFloat2 v0, const AQFloat2 v1) {
    return aqmath_length2(v0 - v1);
}

AQFloat aqmath_distance3(const AQFloat3 v0, const AQFloat3 v1) {
    return aqmath_length3(v0 - v1);
}

AQFloat aqmath_distance4(const AQFloat4 v0, const AQFloat4 v1) {
    return aqmath_length4(v0 - v1);
}

AQFloat2 aqmath_norm2(const AQFloat2 v0) {
    return v0 * 1/(sqrtf(aqmath_dot2(v0,v0)));
}

AQFloat3 aqmath_norm3(const AQFloat3 v0) {
    return v0 *  1/(sqrtf(aqmath_dot3(v0,v0)));
}

AQFloat4 aqmath_norm4(const AQFloat4 v0) {
    return v0 *  1/(sqrtf(aqmath_dot4(v0,v0)));
}

AQFloat3 aqmath_cross(const AQFloat3 v0, const AQFloat3 v1) {
    AQFloat3 v2;
    aq_x(v2) = aq_y(v0) * aq_z(v1) - aq_z(v0) * aq_y(v1);
    aq_y(v2) = aq_z(v0) * aq_x(v1) - aq_x(v0) * aq_z(v1);
    aq_z(v2) = aq_x(v0) * aq_y(v1) - aq_y(v0) * aq_x(v1);
    return v2;
}

AQInt aqmath_are_equal2(const AQFloat2 v0, const AQFloat2 v1) {
    if (aq_x(v0) == aq_x(v1) && aq_y(v0) == aq_y(v1) ) return 1;
    return 0;
}

AQInt aqmath_are_equal3(const AQFloat3 v0, const AQFloat3 v1) {
   if (aq_x(v0) == aq_x(v1) && aq_y(v0) == aq_y(v1)
    && aq_z(v0) == aq_z(v1) ) return 1;
   return 0;
}

AQInt aqmath_are_equal4(const AQFloat4 v0, const AQFloat4 v1) {
   if (aq_x(v0) == aq_x(v1) && aq_y(v0) == aq_y(v1)
    && aq_z(v0) == aq_z(v1) && aq_w(v0) == aq_w(v1) ) return 1;
   return 0;
}

void aqmath_min_max2(AQFloat* min, AQFloat* max, const AQFloat2 v0) {
    int i = 0;
    *min = v0[i];
    *max = v0[i];
    while ( i < 2 ) {
        *min = ( v0[i] < *min ) ? v0[i] : *min;
        *max = ( v0[i] > *max ) ? v0[i] : *max;
        i++;
    }
}

 void aqmath_clamp2(AQFloat2 v0, const AQFloat min, const AQFloat max) {
    int i = 0;
    while ( i < 2 ) {
        v0[i] = ( v0[i] > max ) ? max : v0[i];
        v0[i] = ( v0[i] < min ) ? min : v0[i];
        i++;
    }
}

void aqmath_min_max3(AQFloat* min, AQFloat* max, const AQFloat3 v0) {
    int i = 0;
    *min = v0[i];
    *max = v0[i];
    while ( i < 3 ) {
        *min = ( v0[i] < *min ) ? v0[i] : *min;
        *max = ( v0[i] > *max ) ? v0[i] : *max;
        i++;
    }
}

void aqmath_clamp3(AQFloat3 v0, const AQFloat min, const AQFloat max) {
    int i = 0;
    while ( i < 3 ) {
        v0[i] = ( v0[i] > max ) ? max : v0[i];
        v0[i] = ( v0[i] < min ) ? min : v0[i];
        i++;
    }
}

void aqmath_min_max4(AQFloat* min, AQFloat* max, const AQFloat4 v0) {
    int i = 0;
    *min = v0[i];
    *max = v0[i];
    while ( i < 4 ) {
        *min = ( v0[i] < *min ) ? v0[i] : *min;
        *max = ( v0[i] > *max ) ? v0[i] : *max;
        i++;
    }
}

 void aqmath_clamp4(AQFloat4 v0, const AQFloat min, const AQFloat max) {
    int i = 0;
    while ( i < 4 ) {
        v0[i] = ( v0[i] > max ) ? max : v0[i];
        v0[i] = ( v0[i] < min ) ? min : v0[i];
        i++;
    }
}

AQFloat4x4 aqmath_identity_matrix(void) {
    AQFloat4x4 matrix = { .matrix = {
                            {1,0,0,0},
                            {0,1,0,0},
                            {0,0,1,0},
                            {0,0,0,1}
                          }
                        };
    return matrix;
}

void aqmath_get_matrix_float_array(AQFloat4x4 m0, AQFloatPtrToArray m1) {
    m1[0] = aq_m1(m0);
    m1[1] = aq_m2(m0);
    m1[2] = aq_m3(m0);
    m1[3] = aq_m4(m0);
    m1[4] = aq_m5(m0);
    m1[5] = aq_m6(m0);
    m1[6] = aq_m7(m0);
    m1[7] = aq_m8(m0);
    m1[8] = aq_m9(m0);
    m1[9] = aq_m10(m0);
    m1[10] = aq_m11(m0);
    m1[11] = aq_m12(m0);
    m1[12] = aq_m13(m0);
    m1[13] = aq_m14(m0);
    m1[14] = aq_m15(m0);
    m1[15] = aq_m16(m0);
}

AQFloat4x4 aqmath_get_matrix_from_matrix_float_array(AQFloatPtrToArray m1) {
    AQFloat4x4 m0;
    aq_m1(m0) = m1[0];
    aq_m2(m0) = m1[1];
    aq_m3(m0) = m1[2];
    aq_m4(m0) = m1[3];
    aq_m5(m0) = m1[4];
    aq_m6(m0) = m1[5];
    aq_m7(m0) = m1[6];
    aq_m8(m0) = m1[7];
    aq_m9(m0) = m1[8];
    aq_m10(m0) = m1[9];
    aq_m11(m0) = m1[10];
    aq_m12(m0) = m1[11];
    aq_m13(m0) = m1[12];
    aq_m14(m0) = m1[13];
    aq_m15(m0) = m1[14];
    aq_m16(m0) = m1[15];
    return m0;
}

AQFloat4x4 aqmath_matrix_multiply(const AQFloat4x4 m0, const AQFloat4x4 m1) {
    AQFloat4 retvec;
    AQFloat4x4 retmatrix;
    AQFloat4 mv0 = {aq_m1(m0),aq_m2(m0),aq_m2(m0),aq_m4(m0)};
    AQFloat4 mv1 = {aq_m5(m0),aq_m6(m0),aq_m7(m0),aq_m8(m0)};
    AQFloat4 mv2 = {aq_m9(m0),aq_m10(m0),aq_m11(m0),aq_m12(m0)};
    AQFloat4 mv3 = {aq_m13(m0),aq_m14(m0),aq_m15(m0),aq_m16(m0)};
    retvec = mv0*aq_m1(m1) + mv1*aq_m2(m1) + mv2*aq_m3(m1) + mv3*aq_m4(m1);
    aq_m1(retmatrix) = aq_x(retvec);
    aq_m2(retmatrix) = aq_y(retvec);
    aq_m3(retmatrix) = aq_z(retvec);
    aq_m4(retmatrix) = aq_w(retvec);
    retvec = mv0*aq_m5(m1) + mv1*aq_m6(m1) + mv2*aq_m7(m1) + mv3*aq_m8(m1);
    aq_m5(retmatrix) = aq_x(retvec);
    aq_m6(retmatrix) = aq_y(retvec);
    aq_m7(retmatrix) = aq_z(retvec);
    aq_m8(retmatrix) = aq_w(retvec);
    retvec = mv0*aq_m9(m1) + mv1*aq_m10(m1) + mv2*aq_m11(m1) + mv3*aq_m12(m1);
    aq_m9(retmatrix) = aq_x(retvec);
    aq_m10(retmatrix) = aq_y(retvec);
    aq_m11(retmatrix) = aq_z(retvec);
    aq_m12(retmatrix) = aq_w(retvec);
    retvec = mv0*aq_m13(m1) + mv1*aq_m14(m1) + mv2*aq_m15(m1) + mv3*aq_m16(m1);
    aq_m13(retmatrix) = aq_x(retvec);
    aq_m14(retmatrix) = aq_y(retvec);
    aq_m15(retmatrix) = aq_z(retvec);
    aq_m16(retmatrix) = aq_w(retvec);
    return retmatrix;
}

AQFloat4x4 aqmath_perspective_matrix(AQFloat feild_of_view, AQFloat aspect,
  AQFloat near_z, AQFloat far_z) {
    AQFloat4x4 retmatrix = aqmath_identity_matrix();
    const AQFloat rad = feild_of_view*(M_PI/180);
    const AQFloat num = 1.f/tanf(rad/2.f);
    aq_m1(retmatrix) = num/aspect;
	aq_m6(retmatrix) = num;
	aq_m11(retmatrix) = -((far_z+near_z)/(near_z-far_z));
	aq_m12(retmatrix) = -1.f;
	aq_m15(retmatrix) = -((2.f*far_z*near_z)/(near_z-far_z));
	aq_m16(retmatrix) = 0.f;
    return retmatrix;
}

AQFloat4x4 aqmath_perspective_matrix_from_res(AQInt width, AQInt height,
   AQFloat feild_of_view, float near_z, float far_z ) {
    float aspect = ((float)width)/((float)height);
    return aqmath_perspective_matrix(feild_of_view, aspect, near_z, far_z);
}

AQFloat4x4 aqmath_orthographic_matrix(AQFloat left, AQFloat right, AQFloat bottom,
   AQFloat top, AQFloat near_z, AQFloat far_z) {
	AQFloat4x4 retmatrix = aqmath_identity_matrix();
	aq_m1(retmatrix) = 2.f/(right-left);
	aq_m6(retmatrix) = 2.f/(top-bottom);
	aq_m11(retmatrix) = -2.f/(far_z-near_z);
	aq_m13(retmatrix) = -(right+left)/(right-left);
	aq_m14(retmatrix) = -(top+bottom)/(top-bottom);
	aq_m15(retmatrix) = -(far_z+near_z)/(far_z-near_z);
	aq_m16(retmatrix) = 1.f;
    return retmatrix;
}

AQFloat4x4 aqmath_translate_matrix(AQFloat4x4 m0, AQFloat x_trans,
  AQFloat y_trans, AQFloat z_trans) {
    AQFloat3 retvec;
    AQFloat3 v0 = {aq_m1(m0),aq_m2(m0),aq_m3(m0)};
    AQFloat3 v1 = {aq_m5(m0),aq_m6(m0),aq_m7(m0)};
    AQFloat3 v2 = {aq_m9(m0),aq_m10(m0),aq_m11(m0)};
    AQFloat3 xv = {x_trans,x_trans,x_trans};
    AQFloat3 yv = {y_trans,y_trans,y_trans};
    AQFloat3 zv = {z_trans,z_trans,z_trans};
    retvec = v0*xv + v1*yv + v2*zv;
    aq_m13(m0) += aq_x(retvec);
    aq_m14(m0) += aq_y(retvec);
    aq_m15(m0) += aq_z(retvec);
    return m0;
}

AQFloat4x4 aqmath_scale_matrix(AQFloat4x4 m0, AQFloat x_scale,
  AQFloat y_scale, AQFloat z_scale) {
    AQFloat4 svec = {x_scale,y_scale,z_scale,1.f};
    m0.matrix[0] *= svec;
    m0.matrix[1] *= svec;
    m0.matrix[2] *= svec;
    m0.matrix[3] *= svec;
    return m0;
}

AQFloat4 aqmath_get_quaternion_rotation(AQFloat x, AQFloat y, AQFloat z, AQFloat degrees) {
    AQFloat4 axis_angle = {x,y,z,degrees};
    double radians = (axis_angle[3] / 180) * M_PI;
    AQFloat value = sin(radians / 2);
    AQFloat3 value_vec = {value,value,value,0};
    AQFloat4 quaternion = value_vec * axis_angle;
    quaternion[3] = cos(radians / 2);
    return quaternion;
}

 AQFloat4 aqmath_merge_quaternion_rotation(AQFloat4 q0, AQFloat4 q1) {
    AQFloat4 quaternion;
    quaternion[0] = q0[3] * q1[0] + q0[0] * q1[3] + q0[1] * q1[2] - q0[2] * q1[1];
    quaternion[1] = q0[3] * q1[1] + q0[1] * q1[3] + q0[2] * q1[0] - q0[0] * q1[2];
    quaternion[2] = q0[3] * q1[2] + q0[2] * q1[3] + q0[0] * q1[1] - q0[1] * q1[0];
    quaternion[3] = q0[3] * q1[3] - q0[0] * q1[0] - q0[1] * q1[1] - q0[2] * q1[2];
    return quaternion;
}

AQFloat4x4 aqmath_get_rotation_matrix_from_quaternion(AQFloat4 quaternion) {
    AQFloat4x4 matrix;
    AQFloat x,y,z,w;
    x = aq_x(quaternion);
    y = aq_y(quaternion);
    z = aq_z(quaternion);
    w = aq_w(quaternion);
    aq_m1(matrix) = 1 - (2 * (y*y+z*z));
    aq_m2(matrix) = (2 * (x*y+z*w));
    aq_m3(matrix) = (2 * (x*z-y*w));
    aq_m4(matrix) = (2 * (x*y-z*w));
    aq_m5(matrix) = 1 - (2 * (x*x+z*z));
    aq_m6(matrix) = (2 * (y*z+x*w));
    aq_m7(matrix) = (2 * (x*z+y*w));
    aq_m8(matrix) = (2 * (y*z-x*w));
    aq_m9(matrix) = 1 - (2 * (x*x+y*y));
    return matrix;
}

AQFloat4x4 aqmath_rotate_x(AQFloat4x4 m0, AQFloat degrees) {
    AQFloat4 q0 = aqmath_get_quaternion_rotation(1,0,0,degrees);
    AQFloat4x4 m1 = aqmath_get_rotation_matrix_from_quaternion(q0);
    return aqmath_matrix_multiply(m0,m1);
}

AQFloat4x4 aqmath_rotate_y(AQFloat4x4 m0, AQFloat degrees) {
    AQFloat4 q0 = aqmath_get_quaternion_rotation(0,1,0,degrees);
    AQFloat4x4 m1 = aqmath_get_rotation_matrix_from_quaternion(q0);
    return aqmath_matrix_multiply(m0,m1);
}

AQFloat4x4 aqmath_rotate_z(AQFloat4x4 m0, AQFloat degrees) {
    AQFloat4 q0 = aqmath_get_quaternion_rotation(0,0,1,degrees);
    AQFloat4x4 m1 = aqmath_get_rotation_matrix_from_quaternion(q0);
    return aqmath_matrix_multiply(m0,m1);
}

AQFloat4x4 aqmath_rotate_matrix(AQFloat4x4 m0, AQFloat degrees,
   AQFloat x_axis, AQFloat y_axis, AQFloat z_axis) {
    AQFloat4 q0 = aqmath_get_quaternion_rotation(x_axis,y_axis,z_axis,degrees);
    AQFloat4x4 m1 = aqmath_get_rotation_matrix_from_quaternion(q0);
    return aqmath_matrix_multiply(m0,m1);
}

AQFloat4x4 aqmath_transform_matrix(AQFloat3 pos, AQFloat3 rot, float scale) {
    AQFloat4x4 m0 = aqmath_identity_matrix();
    m0 = aqmath_translate_matrix(m0,aq_x(pos),aq_y(pos),aq_z(pos));
    m0 = aqmath_rotate_matrix(m0,aq_x(rot),1,0,0);
    m0 = aqmath_rotate_matrix(m0,aq_y(rot),0,1,0);
    m0 = aqmath_rotate_matrix(m0,aq_z(rot),0,0,1);
    return aqmath_scale_matrix(m0,scale,scale,scale);
}

AQFloat4x4 aqmath_view_matrix(AQFloat3 pos, AQFloat pitch,
  AQFloat yaw, AQFloat roll) {
    AQFloat4x4 m0 = aqmath_identity_matrix();
    m0 = aqmath_rotate_matrix(m0,pitch,1,0,0);
    m0 = aqmath_rotate_matrix(m0,yaw,0,1,0);
    m0 = aqmath_rotate_matrix(m0,roll,0,0,1);
    pos = -pos;
    return aqmath_translate_matrix(m0,aq_x(pos),aq_y(pos),aq_z(pos));
}

//xoshiro256**
static inline AQULong aqinternal_math_random_rotate64(AQULong x, AQInt k) {
    return (x << k) | (x >> (64 - k));
}

static AQULong aqinternal_math_random_next_seed(AQULong x) {
	AQULong z = (x + UINT64_C(0x9E3779B97F4A7C15));
	z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
	return z ^ (z >> 31);
}

void aqmath_seed_random_state(AQRandState rs, AQULong seed) {
    rs->state[0] = aqinternal_math_random_next_seed(seed);
    rs->state[1] = aqinternal_math_random_next_seed(rs->state[0]);
    rs->state[2] = aqinternal_math_random_next_seed(rs->state[1]);
    rs->state[3] = aqinternal_math_random_next_seed(rs->state[2]);
}

void aqmath_seed_random_state_with_time(AQRandState rs) {
    aqmath_seed_random_state(rs,(AQULong)time(NULL));
}

AQULong aqmath_random_value(AQRandState rs) {
	AQULong *s = rs->state;
	AQULong const result = 
	 aqinternal_math_random_rotate64(s[1] * 5, 7) * 9;
	AQULong const t = s[1] << 17;
	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];
	s[2] ^= t;
	s[3] = 
	 aqinternal_math_random_rotate64(s[3], 45);
	return result;
}

#define AQ_RANDOM_MAX UINT64_C(18446744073709551615)

AQDouble aqmath_a_random_double(AQRandState rs, AQDouble min, AQDouble max) {
    return 
     ((AQDouble)aqmath_random_value(rs) / AQ_RANDOM_MAX) * (max - min) + min;
}

AQULong aqmath_a_random_long(AQRandState rs, AQULong min, AQULong max) {
    return (AQULong)(aqmath_a_random_double(rs,min,max));
}

AQUInt aqmath_a_random_int(AQRandState rs, AQUInt min, AQUInt max) {
    return (AQUInt)(aqmath_a_random_double(rs,min,max));
}
