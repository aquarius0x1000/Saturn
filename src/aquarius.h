#ifndef aquarius_h
#define aquarius_h

#include <math.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <inttypes.h>

#ifdef __GNUC__
 #pragma GCC diagnostic ignored "-Wunused-value"
 #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
#ifdef __clang__
 #pragma clang diagnostic ignored "-Wunused-value"
 #pragma clang diagnostic ignored "-Wunused-but-set-variable"
#endif

#ifndef M_PI
 #define M_PI 3.1415926535897932384626433832
#endif

typedef float AQFloat;
typedef double AQDouble;
typedef char AQChar;
typedef unsigned char AQByte;
typedef signed char AQSByte;
typedef signed short AQShort;
typedef unsigned short AQUShort;
typedef signed int AQInt;
typedef unsigned int AQUInt;

#ifdef _WIN32
 typedef signed long long AQLong ;
 typedef unsigned long long AQULong ;
#else
 typedef signed long AQLong ;
 typedef unsigned long AQULong ;
#endif

typedef enum {
  AQEmptyFlag = 0,  
  AQByteFlag = 1,
  AQSByteFlag = 2,
  AQShortFlag = 3,
  AQUShortFlag = 4,
  AQIntFlag = 5,
  AQUIntFlag = 6,
  AQLongFlag = 7,
  AQULongFlag = 8,
  AQFloatFlag = 9,
  AQDoubleFlag = 10,
  AQAnyFlag = 11
} AQTypeFlag;

#define aq_vectordef(base_type,vector_input_size,new_vector_type) \
 typedef base_type new_vector_type __attribute__ ((vector_size (sizeof(base_type)*vector_input_size)))

#define aq_x(vec) vec[0]
#define aq_y(vec) vec[1]
#define aq_z(vec) vec[2]
#define aq_w(vec) vec[3]

#define aq_m1(m) m.matrix[0][0]
#define aq_m2(m) m.matrix[1][0]
#define aq_m3(m) m.matrix[2][0]
#define aq_m4(m) m.matrix[3][0]

#define aq_m5(m) m.matrix[0][1]
#define aq_m6(m) m.matrix[1][1]
#define aq_m7(m) m.matrix[2][1]
#define aq_m8(m) m.matrix[3][1]

#define aq_m9(m)  m.matrix[0][2]
#define aq_m10(m) m.matrix[1][2]
#define aq_m11(m) m.matrix[2][2]
#define aq_m12(m) m.matrix[3][2]

#define aq_m13(m) m.matrix[0][3]
#define aq_m14(m) m.matrix[1][3]
#define aq_m15(m) m.matrix[2][3]
#define aq_m16(m) m.matrix[3][3]

aq_vectordef(AQByte,2,AQByte2);
aq_vectordef(AQInt,2,AQInt2);
aq_vectordef(AQFloat,2,AQFloat2);
aq_vectordef(AQDouble,2,AQDouble2);

aq_vectordef(AQByte,4,AQByte4);
aq_vectordef(AQInt,4,AQInt4);
aq_vectordef(AQFloat,4,AQFloat4);
aq_vectordef(AQDouble,4,AQDouble4);

typedef AQByte4 AQByte3;
typedef AQInt4 AQInt3;
typedef AQFloat4 AQFloat3;
typedef AQDouble4 AQDouble3;

typedef struct {
  AQFloat4 matrix[4];
} AQFloat4x4;

typedef AQFloat* AQFloatPtrToArray;
#define aq_math_declare_matrix_float_array(name) AQFloat name[16]

typedef struct {
  AQULong state[4];
} AQRandStateStruct;

typedef AQRandStateStruct* AQRandState;

typedef enum {
  AQNoDataStructureFlag,
  AQArrayFlag,
  AQStringFlag,
  AQListFlag,
  AQListNodeFlag,
  AQStackFlag,
  AQStackBufferFlag,
  AQMultiTypeArrayFlag,
  AQMTAStackBufferFlag,
  AQMTAContainerFlag,
  AQStoreFlag,
} AQDataStructureFlag;

typedef struct AQDataStructure_s* AQDataStructure;
typedef struct AQArray_s* AQArray;
typedef struct AQString_s* AQString;
typedef struct AQListNode_s* AQListNode;
typedef struct AQList_s* AQList;
typedef struct AQStack_s* AQStack;
typedef struct AQStackBuffer_s* AQStackBuffer;
typedef struct AQMultiTypeArray_s* AQMultiTypeArray;
typedef struct AQMTAStackBuffer_s* AQMTAStackBuffer;
typedef struct AQStore_s* AQStore;
typedef void* AQAny;

typedef struct {
  AQDataStructureFlag flag;  
  AQTypeFlag type;
    union {
      AQByte AQByteVal;
      AQSByte AQSByteVal;
      AQShort AQShortVal;
      AQUShort AQUShortVal;
      AQInt AQIntVal;
      AQUInt AQUIntVal;
      AQLong AQLongVal;
      AQULong AQULongVal;
      AQFloat AQFloatVal;
      AQDouble AQDoubleVal;
      AQAny AQAnyVal;
    }; 
} AQMTAContainer;

typedef void (*AQIteratorFuncType)(AQAny data);
typedef AQAny (*AQGetDataFromArrayFuncType)(AQAny array, AQULong index);
typedef AQAny (*AQAllocatorFuncType)(AQAny allocation_data, AQULong size_in_bytes);
typedef void (*AQAllocatorFreeFuncType)(AQAny allocation_data, AQAny data_to_be_freed);

typedef struct {
  AQAny data;
  AQAllocatorFuncType allocator_function;
  AQAllocatorFreeFuncType free_function;
} AQAllocatorStruct;

typedef AQAllocatorStruct* AQAllocator;


#define aq_generic_test_does_not_have_va_args(A,B,...) B()
#define aq_generic_test_does_not_have_va_args_not(A,B,...) A(__VA_ARGS__)
#define aq_generic_test(A,B,does_have_va_args,...) aq_generic_test_does_not_have_va_args ## does_have_va_args (A,B,__VA_ARGS__)
#define aq_generic(A,B,...) aq_generic_test(A,B,__VA_OPT__(_not),__VA_ARGS__)


#define aq_alloc(...) _Generic((__VA_ARGS__), \
  default: aqmem_malloc, \
  AQAllocator: aqmem_malloc_with_allocator \
)(__VA_ARGS__)

#define aq_realloc0(...) _Generic((__VA_ARGS__), \
  default: aqmem_realloc, \
  AQAllocator: aqmem_realloc_with_allocator \
)(__VA_ARGS__)

#define aq_free(...) _Generic((__VA_ARGS__), \
  default: aqmem_free, \
  AQAllocator: aqmem_free_with_allocator \
)(__VA_ARGS__)

#define aq_make_c_array(size, type,...)\
 (type*) aq_alloc(sizeof(type) * (size) __VA_OPT__(,) __VA_ARGS__)
#define aq_new(type, ...)\
 aq_make_c_array(1,type __VA_OPT__(,) __VA_ARGS__)
#define aq_realloc(data, newsize, oldsize, type, NULLonError0No1Yes, ...)\
 (type*) aq_realloc0( data, sizeof(type) * (newsize),\
  sizeof(type) * (oldsize), NULLonError0No1Yes __VA_OPT__(,) __VA_ARGS__)

AQAllocator aqmem_default_allocator(void);
AQAny aqmem_malloc(AQULong size_in_bytes);
AQAny aqmem_malloc_with_allocator(AQULong size_in_bytes, AQAllocator allocator);
void aqmem_free(AQAny data);
void aqmem_free_with_allocator(AQAny data, AQAllocator allocator);
AQAny aqmem_realloc(AQAny data, AQULong newsize,
    AQULong oldsize, AQInt NULLonError0No1Yes);
AQAny aqmem_realloc_with_allocator(AQAny data, AQULong newsize,
    AQULong oldsize, AQInt NULLonError0No1Yes, AQAllocator allocator);

AQDataStructureFlag aqds_get_flag(AQDataStructure ds);        

    
#define aq_new_array(...)\
 aq_generic(aqarray_new_with_allocator,aqarray_new,__VA_ARGS__)
#define aq_new_array_with_base_size(...) _Generic((__VA_ARGS__), \
  default: aqarray_new_with_base_size, \
  AQAllocator: aqarray_new_with_base_size_with_allocator \
)(__VA_ARGS__)
#define aq_array_foreach(index,array) for (AQULong index = 0; index < aqarray_get_num_of_items(array); index++)

AQArray aqarray_new(void);
AQArray aqarray_new_with_allocator(AQAllocator allocator);
AQArray aqarray_new_with_base_size(AQULong base_size);
AQArray aqarray_new_with_base_size_with_allocator(AQULong base_size, AQAllocator allocator);
void aqarray_destroy(AQArray array);
void aqarray_add_item(AQArray array, AQAny item);
void aqarray_remove_item(AQArray array);
void aqarray_add_array(AQArray array, AQAny items[], AQULong num_of_items_to_add);
void aqarray_add_space(AQArray array, AQULong space_to_add);
void aqarray_remove_space(AQArray array, AQULong space_to_remove);
AQAny aqarray_get_item(AQArray array, AQULong index);
AQInt aqarray_set_item(AQArray array, AQULong index, AQAny item);
AQULong aqarray_get_num_of_items(AQArray array);
void aqarray_iterate_with(AQIteratorFuncType iterator, AQArray array);
 
 
#define aq_new_string(string,...) _Generic((string __VA_OPT__(,) __VA_ARGS__), \
  default: aqstring_new_from_buffer, \
  AQAllocator: aqstring_new_from_buffer_with_allocator \
)(string, sizeof(string) __VA_OPT__(,) __VA_ARGS__)
#define aq_add_strings(a, b,...) _Generic((a,b __VA_OPT__(,) __VA_ARGS__), \
  default: aqstring_new_from_two_strings, \
  AQAllocator: aqstring_new_from_two_strings_with_allocator \
)(a, b __VA_OPT__(,) __VA_ARGS__)
#define aqstr(string,...) aq_new_string(string __VA_OPT__(,) __VA_ARGS__)
#define aqstrU8(string,...) aq_new_string(u8##string __VA_OPT__(,) __VA_ARGS__)
#define aqstrfree(string) aqstring_destroy(string)
#define aqstrprint(string) aqstring_print(string)
#define aq_string_for_each_byte(index,string)\
 for (AQULong index = 0; index < aqstring_get_size_in_bytes(string); index++)
#define aq_string_for_each_character(index,string)\
 for (AQULong index = 0; index < aqstring_get_length(string); index++) 

AQString aqstring_new(AQULong size_in_bytes);
AQString aqstring_new_with_allocator(AQULong size_in_bytes, AQAllocator allocator);
AQString aqstring_new_from_utf32(const AQUInt* text, AQULong num_of_characters);
AQString aqstring_new_from_utf32_with_allocator(const AQUInt* text,
   AQULong num_of_characters, AQAllocator allocator);
AQString aqstring_new_from_buffer(const AQChar* text, AQULong size_in_bytes);
AQString aqstring_new_from_buffer_with_allocator(const AQChar* text,
   AQULong size_in_bytes, AQAllocator allocator);
AQString aqstring_new_from_c_string(const AQChar* text);
AQString aqstring_new_from_c_string_with_allocator(const AQChar* text,
   AQAllocator allocator);
AQString aqstring_new_from_two_strings(AQString a, AQString b);
AQString aqstring_new_from_two_strings_with_allocator(AQString a,
   AQString b, AQAllocator allocator);
void aqstring_destroy(AQString string);
AQULong aqstring_get_size(AQString string);
AQULong aqstring_get_size_in_bytes(AQString string);
AQULong aqstring_get_length(AQString string);
AQChar* aqstring_get_c_string(AQString string);
AQUInt aqstring_get_character(AQString string, AQULong index, AQSByte* offset);
AQByte aqstring_get_byte(AQString string, AQULong index);
void aqstring_set_byte(AQString string, AQULong index, AQByte byte);
AQString aqstring_append(AQString base_string, AQString appending_string);
AQString aqstring_copy(AQString string);
AQInt aqstring_are_equal(AQString a, AQString b);
AQChar* aqstring_convert_to_c_string(AQString string);
void aqstring_print(AQString string);
AQUInt* aqstring_get_utf32_string(AQString string, AQULong* length);
AQString aqstring_get_string_for_ascii(AQString string);
AQString aqstring_swap_escape_sequences_with_characters(AQString string);
void aqstring_iterate_bytes_with(AQIteratorFuncType iterator, AQString string);
void aqstring_iterate_characters_with(AQIteratorFuncType iterator, AQString string);



#define aq_new_list(...)\
 aq_generic(aqlist_new_with_allocator,aqlist_new,__VA_ARGS__)

#define aq_new_list_from_array(...) _Generic((__VA_ARGS__), \
  default: aqlist_new_from_array, \
  AQAllocator: aqlist_new_from_array_with_allocator \
)(__VA_ARGS__)
#define aq_list_foreach(node,list) for (AQListNode node = aqlist_first_node(list); node != NULL; node = aqlist_next_node(node))

AQList aqlist_new(void);
AQList aqlist_new_with_allocator(AQAllocator allocator);
AQList aqlist_new_from_array(AQAny array,
   AQGetDataFromArrayFuncType GetDataFromArrayFunc, AQULong size);
AQList aqlist_new_from_array_with_allocator(AQAny array,
 AQGetDataFromArrayFuncType GetDataFromArrayFunc,
  AQULong size, AQAllocator allocator);
AQListNode aqlist_new_node(AQList list, AQListNode before,
   AQListNode after, AQAny item);
void aqlist_destroy(AQList list);
void aqlist_destroy_node(AQList list, AQListNode node);
void aqlist_destroy_node_with_index(AQList list, AQULong index);
AQListNode aqlist_destroy_node_get_next(AQList list, AQListNode node);
void aqlist_destroy_all_nodes(AQList list);
AQListNode aqlist_add_item(AQList list, AQAny item);
AQListNode aqlist_add_node(AQList list, AQListNode node);
void aqlist_node_swap(AQList list, AQListNode node_a, AQListNode node_b);
void aqlist_item_swap(AQListNode node_a, AQListNode node_b);
void aqlist_insert_a_to_b(AQList list, AQListNode node_a, AQListNode node_b);
AQListNode aqlist_move_node(AQListNode node, AQList list_a, AQList list_b);
void aqlist_copy(AQList list_a, AQList list_b);
void aqlist_copy_from_array(AQList list, AQAny array,
  AQGetDataFromArrayFuncType GetDataFromArrayFunc, AQULong size);
void aqlist_set_item(AQListNode node, AQAny item);
AQAny aqlist_get_item(AQListNode node);
AQListNode aqlist_next_node(AQListNode node);
AQListNode aqlist_previous_node(AQListNode node);
AQListNode aqlist_first_node(AQList list);
AQListNode aqlist_last_node(AQList list);
AQLong aqlist_num_of_nodes(AQList list);
AQListNode aqlist_get_node(AQList list, AQULong index);
AQULong aqlist_get_index(AQList list, AQListNode node);
AQListNode aqlist_next_node_after_n(AQListNode node, AQULong n);
AQListNode aqlist_previous_node_after_n(AQListNode node, AQULong n);
void aqlist_iterate_with(AQIteratorFuncType iterator, AQList list);


#define aq_new_stack(...)\
 aq_generic(aqstack_new_with_allocator,aqstack_new,__VA_ARGS__)

AQStack aqstack_new(void);
AQStack aqstack_new_with_allocator(AQAllocator allocator);
void aqstack_destroy(AQStack stack);
void aqstack_push_item(AQStack stack, AQAny item);
AQAny aqstack_pop_item(AQStack stack);
AQAny aqstack_peek_item(AQStack stack);
AQInt aqstack_is_empty(AQStack stack);
AQList aqstack_get_list(AQStack stack);
AQListNode aqstack_get_list_node(AQStack stack);


#define aq_new_stackbuffer(...)\
 aq_generic(aqstackbuffer_new_with_allocator,aqstackbuffer_new,__VA_ARGS__)

AQStackBuffer aqstackbuffer_new(void);
AQStackBuffer aqstackbuffer_new_with_allocator(AQAllocator allocator);
void aqstackbuffer_destroy(AQStackBuffer stack);
void aqstackbuffer_set_rate(AQStackBuffer stack, AQULong rate);
void aqstackbuffer_push_item(AQStackBuffer stack, AQAny item);
AQAny aqstackbuffer_pop_item(AQStackBuffer stack);
AQAny aqstackbuffer_peek_item(AQStackBuffer stack);
AQInt aqstackbuffer_is_empty(AQStackBuffer stack);
AQArray aqstackbuffer_get_array(AQStackBuffer stack);


#define aq_new_mta(...)\
 aq_generic(aqmta_new_with_allocator,aqmta_new,__VA_ARGS__)

#define aq_mta_add_item(type,mta,value)\
 aqmta_add_item_##type(mta,value)

#define aq_mta_remove_item(type,mta)\
 aqmta_remove_item_##type(mta)
 
#define aq_mta_add_space(type,mta,space_to_add)\
 aqmta_add_space_##type(mta,space_to_add) 

#define aq_mta_remove_space(type,mta,space_to_remove)\
 aqmta_remove_space_##type(mta,space_to_remove)
   
#define aq_mta_get_item(type,mta,index)\
 aqmta_get_item_##type(mta,index)
 
#define aq_mta_set_item(type,mta,index,value)\
 aqmta_set_item_##type(mta,index,value)
 
#define aq_mta_get_num_of_items(type,mta)\
 aqmta_get_num_of_items(mta,type##Flag-1)
 
#define aq_mta_define_itemvar(varname)\
   AQByte varname##_AQByte = 0;\
   AQSByte varname##_AQSByte = 0;\
   AQShort varname##_AQShort = 0;\
   AQUShort varname##_AQUShort = 0;\
   AQInt varname##_AQInt = 0;\
   AQUInt varname##_AQUInt = 0;\
   AQLong varname##_AQLong = 0;\
   AQULong varname##_AQULong = 0;\
   AQFloat varname##_AQFloat = 0;\
   AQDouble varname##_AQDouble = 0;\
   AQAny varname##_AQAny = NULL;\
   AQTypeFlag type_of_##varname = AQEmptyFlag
 
#define aqmtaget(type,varname)\
 varname##_##type 
   
#define aq_mta_container_get_case(type,container,varname)\
   case type##Flag:\
    varname##_##type = container.type##Val;\
    type_of_##varname = type##Flag;\
   break
 
#define aq_mta_get_itemvar_from_container(container,varname)\
  switch (container.type) {\
    aq_mta_container_get_case(AQByte,container,varname);\
    aq_mta_container_get_case(AQSByte,container,varname);\
    aq_mta_container_get_case(AQShort,container,varname);\
    aq_mta_container_get_case(AQUShort,container,varname);\
    aq_mta_container_get_case(AQInt,container,varname);\
    aq_mta_container_get_case(AQUInt,container,varname);\
    aq_mta_container_get_case(AQLong,container,varname);\
    aq_mta_container_get_case(AQULong,container,varname);\
    aq_mta_container_get_case(AQFloat,container,varname);\
    aq_mta_container_get_case(AQDouble,container,varname);\
    aq_mta_container_get_case(AQAny,container,varname);\
    default:\
    break;\
  }
     
#define aq_mta_declare_add(type)\
 AQInt aqmta_add_item_##type(AQMultiTypeArray mta, type value)
#define aq_mta_declare_remove_item(type)\
 AQInt aqmta_remove_item_##type(AQMultiTypeArray mta)
#define aq_mta_declare_add_space(type)\
 AQInt aqmta_add_space_##type(AQMultiTypeArray mta, AQULong space_to_add)
#define aq_mta_declare_remove_space(type)\
 AQInt aqmta_remove_space_##type(AQMultiTypeArray mta, AQULong space_to_remove)
#define aq_mta_declare_get(type)\
 type aqmta_get_item_##type(AQMultiTypeArray mta, AQULong index) 
#define aq_mta_declare_set(type)\
 void aqmta_set_item_##type(AQMultiTypeArray mta, AQULong index, type value)
#define aq_mta_foreach(type,index,mta) for (AQULong index = 0; index < aq_mta_get_num_of_items(type,mta); index++) 
#define aq_mta_foreachtype(index,mta) for (AQULong index = 0; index < aqmta_get_num_of_items_all_types(mta); index++)

AQMultiTypeArray aqmta_new(void);
AQMultiTypeArray aqmta_new_with_allocator(AQAllocator allocator);
void aqmta_destroy(AQMultiTypeArray mta);
aq_mta_declare_add(AQByte);
aq_mta_declare_add(AQSByte);
aq_mta_declare_add(AQShort);
aq_mta_declare_add(AQUShort);
aq_mta_declare_add(AQInt);
aq_mta_declare_add(AQUInt);
aq_mta_declare_add(AQLong);
aq_mta_declare_add(AQULong);
aq_mta_declare_add(AQFloat);
aq_mta_declare_add(AQDouble);
aq_mta_declare_add(AQAny); 
aq_mta_declare_remove_item(AQByte);
aq_mta_declare_remove_item(AQSByte);
aq_mta_declare_remove_item(AQShort);
aq_mta_declare_remove_item(AQUShort);
aq_mta_declare_remove_item(AQInt);
aq_mta_declare_remove_item(AQUInt);
aq_mta_declare_remove_item(AQLong);
aq_mta_declare_remove_item(AQULong);
aq_mta_declare_remove_item(AQFloat);
aq_mta_declare_remove_item(AQDouble);
aq_mta_declare_remove_item(AQAny); 
aq_mta_declare_add_space(AQByte);
aq_mta_declare_add_space(AQSByte);
aq_mta_declare_add_space(AQShort);
aq_mta_declare_add_space(AQUShort);
aq_mta_declare_add_space(AQInt);
aq_mta_declare_add_space(AQUInt);
aq_mta_declare_add_space(AQLong);
aq_mta_declare_add_space(AQULong);
aq_mta_declare_add_space(AQFloat);
aq_mta_declare_add_space(AQDouble);
aq_mta_declare_add_space(AQAny); 
aq_mta_declare_remove_space(AQByte);
aq_mta_declare_remove_space(AQSByte);
aq_mta_declare_remove_space(AQShort);
aq_mta_declare_remove_space(AQUShort);
aq_mta_declare_remove_space(AQInt);
aq_mta_declare_remove_space(AQUInt);
aq_mta_declare_remove_space(AQLong);
aq_mta_declare_remove_space(AQULong);
aq_mta_declare_remove_space(AQFloat);
aq_mta_declare_remove_space(AQDouble);
aq_mta_declare_remove_space(AQAny); 
aq_mta_declare_get(AQByte);
aq_mta_declare_get(AQSByte);
aq_mta_declare_get(AQShort);
aq_mta_declare_get(AQUShort);
aq_mta_declare_get(AQInt); 
aq_mta_declare_get(AQUInt);
aq_mta_declare_get(AQLong);
aq_mta_declare_get(AQULong);
aq_mta_declare_get(AQFloat);
aq_mta_declare_get(AQDouble);
aq_mta_declare_get(AQAny); 
aq_mta_declare_set(AQByte);
aq_mta_declare_set(AQSByte);
aq_mta_declare_set(AQShort);
aq_mta_declare_set(AQUShort);
aq_mta_declare_set(AQInt);
aq_mta_declare_set(AQUInt);
aq_mta_declare_set(AQLong);
aq_mta_declare_set(AQULong);
aq_mta_declare_set(AQFloat);
aq_mta_declare_set(AQDouble);
aq_mta_declare_set(AQAny);
AQULong aqmta_get_num_of_items(AQMultiTypeArray mta, AQTypeFlag type_flag);
AQULong aqmta_get_num_of_items_all_types(AQMultiTypeArray mta);
AQMTAContainer aqmta_get_container(AQMultiTypeArray mta, AQULong index);
void aqmta_iterate_all_types_with(AQIteratorFuncType iterator, AQMultiTypeArray mta);


#define aq_new_mtastackbuffer(...)\
 aq_generic(aqmtastackbuffer_new_with_allocator,aqmtastackbuffer_new,__VA_ARGS__)
#define aq_mtastackbuffer_push_item(type,stack,item)\
 aqmtastackbuffer_push_item##type(stack,item)
#define aq_mtastackbuffer_pop_item_base(type,stack)\
 aqmtastackbuffer_pop_item##type(stack)
  #define aq_mtastackbuffer_peek_item_base(type,stack)\
 aqmtastackbuffer_peek_item##type(stack)
  
#define aq_mtastackbuffer_pop_item_case(type,stack,varname)\
   case type##Flag:\
    varname##_##type = aq_mtastackbuffer_pop_item_base(type,stack);\
    type_of_##varname = type##Flag;\
   break
   
#define aq_mtastackbuffer_peek_item_case(type,stack,varname)\
   case type##Flag:\
    varname##_##type = aq_mtastackbuffer_peek_item_base(type,stack);\
    type_of_##varname = type##Flag;\
   break   
   
#define aq_mtastackbuffer_pop_item(stack,varname)\
  switch (aqmtastackbuffer_peek_type(stack)) {\
    aq_mtastackbuffer_pop_item_case(AQByte,stack,varname);\
    aq_mtastackbuffer_pop_item_case(AQSByte,stack,varname);\
    aq_mtastackbuffer_pop_item_case(AQShort,stack,varname);\
    aq_mtastackbuffer_pop_item_case(AQUShort,stack,varname);\
    aq_mtastackbuffer_pop_item_case(AQInt,stack,varname);\
    aq_mtastackbuffer_pop_item_case(AQUInt,stack,varname);\
    aq_mtastackbuffer_pop_item_case(AQLong,stack,varname);\
    aq_mtastackbuffer_pop_item_case(AQULong,stack,varname);\
    aq_mtastackbuffer_pop_item_case(AQFloat,stack,varname);\
    aq_mtastackbuffer_pop_item_case(AQDouble,stack,varname);\
    aq_mtastackbuffer_pop_item_case(AQAny,stack,varname);\
    default:\
    break;\
  }

#define aq_mtastackbuffer_peek_item(stack,varname)\
  switch (aqmtastackbuffer_peek_type(stack)) {\
    aq_mtastackbuffer_peek_item_case(AQByte,stack,varname);\
    aq_mtastackbuffer_peek_item_case(AQSByte,stack,varname);\
    aq_mtastackbuffer_peek_item_case(AQShort,stack,varname);\
    aq_mtastackbuffer_peek_item_case(AQUShort,stack,varname);\
    aq_mtastackbuffer_peek_item_case(AQInt,stack,varname);\
    aq_mtastackbuffer_peek_item_case(AQUInt,stack,varname);\
    aq_mtastackbuffer_peek_item_case(AQLong,stack,varname);\
    aq_mtastackbuffer_peek_item_case(AQULong,stack,varname);\
    aq_mtastackbuffer_peek_item_case(AQFloat,stack,varname);\
    aq_mtastackbuffer_peek_item_case(AQDouble,stack,varname);\
    aq_mtastackbuffer_peek_item_case(AQAny,stack,varname);\
    default:\
    break;\
  }    
  
#define aq_mtastackbuffer_declare_push_item(type)\
 void aqmtastackbuffer_push_item##type(AQMTAStackBuffer stack, type item) 
#define aq_mtastackbuffer_declare_pop_item(type)\
 type aqmtastackbuffer_pop_item##type(AQMTAStackBuffer stack)
#define aq_mtastackbuffer_declare_peek_item(type)\
 type aqmtastackbuffer_peek_item##type(AQMTAStackBuffer stack)

AQMTAStackBuffer aqmtastackbuffer_new(void);
AQMTAStackBuffer aqmtastackbuffer_new_with_allocator(AQAllocator allocator);
void aqmtastackbuffer_destroy(AQMTAStackBuffer stack);
void aqmtastackbuffer_set_rate(AQMTAStackBuffer stack, AQULong rate);
AQTypeFlag aqmtastackbuffer_peek_type(AQMTAStackBuffer stack);
aq_mtastackbuffer_declare_push_item(AQByte);
aq_mtastackbuffer_declare_push_item(AQSByte);
aq_mtastackbuffer_declare_push_item(AQShort);
aq_mtastackbuffer_declare_push_item(AQUShort);
aq_mtastackbuffer_declare_push_item(AQInt);
aq_mtastackbuffer_declare_push_item(AQUInt);
aq_mtastackbuffer_declare_push_item(AQLong);
aq_mtastackbuffer_declare_push_item(AQULong);
aq_mtastackbuffer_declare_push_item(AQFloat);
aq_mtastackbuffer_declare_push_item(AQDouble);
aq_mtastackbuffer_declare_push_item(AQAny);
aq_mtastackbuffer_declare_pop_item(AQByte);
aq_mtastackbuffer_declare_pop_item(AQSByte);
aq_mtastackbuffer_declare_pop_item(AQShort);
aq_mtastackbuffer_declare_pop_item(AQUShort);
aq_mtastackbuffer_declare_pop_item(AQInt);
aq_mtastackbuffer_declare_pop_item(AQUInt);
aq_mtastackbuffer_declare_pop_item(AQLong);
aq_mtastackbuffer_declare_pop_item(AQULong);
aq_mtastackbuffer_declare_pop_item(AQFloat);
aq_mtastackbuffer_declare_pop_item(AQDouble);
aq_mtastackbuffer_declare_pop_item(AQAny);
aq_mtastackbuffer_declare_peek_item(AQByte);
aq_mtastackbuffer_declare_peek_item(AQSByte);
aq_mtastackbuffer_declare_peek_item(AQShort);
aq_mtastackbuffer_declare_peek_item(AQUShort);
aq_mtastackbuffer_declare_peek_item(AQInt);
aq_mtastackbuffer_declare_peek_item(AQUInt);
aq_mtastackbuffer_declare_peek_item(AQLong);
aq_mtastackbuffer_declare_peek_item(AQULong);
aq_mtastackbuffer_declare_peek_item(AQFloat);
aq_mtastackbuffer_declare_peek_item(AQDouble);
aq_mtastackbuffer_declare_peek_item(AQAny);
AQInt aqmtastackbuffer_is_empty(AQMTAStackBuffer stack);
AQMultiTypeArray aqmtastackbuffer_get_mta(AQMTAStackBuffer stack);


#define aq_new_store(...) \
 aq_generic(aqstore_new_with_allocator,aqstore_new,__VA_ARGS__)
#define aq_store_foreach(node,store) aq_list_foreach(node,aqstore_get_list(store))

AQStore aqstore_new(void);
AQStore aqstore_new_with_allocator(AQAllocator allocator);
AQAllocator aqstore_get_allocator(AQStore store);
void aqstore_destroy(AQStore store);
AQInt aqstore_add_item(AQStore store, AQAny item, const AQChar* label);
AQInt aqstore_remove_item(AQStore store, const AQChar* label);
AQAny aqstore_get_item(AQStore store, const AQChar* label);
AQInt aqstore_item_exists(AQStore store, const AQChar* label);
AQInt aqstore_num_of_items(AQStore store);
AQInt aqstore_is_store_empty(AQStore store);
AQInt aqstore_add_item_to_list(AQStore store, AQAny item);
AQList aqstore_get_list(AQStore store);
AQString aqstore_label_from_list_node(AQListNode node);
void aqstore_iterate_store_with(AQIteratorFuncType iterator, AQStore store);


#define aq_any(any) aqany_new(&any,sizeof(any))
#define aq_get(type,any) *((type*)any)

AQAny aqany_new(AQAny any, AQULong size);
void aqany_destroy(AQAny any);


AQFloat aqmath_dot2(const AQFloat2 v0, const AQFloat2 v1);
AQFloat aqmath_dot3(const AQFloat3 v0, const AQFloat3 v1);
AQFloat aqmath_dot4(const AQFloat4 v0, const AQFloat4 v1);

AQFloat aqmath_length2(const AQFloat2 v0);
AQFloat aqmath_length3(const AQFloat3 v0);
AQFloat aqmath_length4(const AQFloat4 v0);

AQFloat aqmath_distance2(const AQFloat2 v0, const AQFloat2 v1);
AQFloat aqmath_distance3(const AQFloat3 v0, const AQFloat3 v1);
AQFloat aqmath_distance4(const AQFloat4 v0, const AQFloat4 v1);

AQFloat2 aqmath_norm2(const AQFloat2 v0);
AQFloat3 aqmath_norm3(const AQFloat3 v0);
AQFloat4 aqmath_norm4(const AQFloat4 v0);

AQInt aqmath_are_equal2(const AQFloat2 v0, const AQFloat2 v1);
AQInt aqmath_are_equal3(const AQFloat3 v0, const AQFloat3 v1);
AQInt aqmath_are_equal4(const AQFloat4 v0, const AQFloat4 v1);

AQFloat3 aqmath_cross(const AQFloat3 v0, const AQFloat3 v1);

void aqmath_min_max2(AQFloat* min, AQFloat* max, const AQFloat2 v0);
void aqmath_clamp2(AQFloat2 v0, const AQFloat min, const AQFloat max);

void aqmath_min_max3(AQFloat* min, AQFloat* max, const AQFloat3 v0);
void aqmath_clamp3(AQFloat3 v0, const AQFloat min, const AQFloat max);

void aqmath_min_max4(AQFloat* min, AQFloat* max, const AQFloat4 v0);
void aqmath_clamp4(AQFloat4 v0, const AQFloat min, const AQFloat max);

AQFloat4x4 aqmath_identity_matrix(void);
void aqmath_get_matrix_float_array(AQFloat4x4 m0, AQFloatPtrToArray m1);
AQFloat4x4 aqmath_get_matrix_from_matrix_float_array(AQFloatPtrToArray m1);
AQFloat4x4 aqmath_matrix_multiply(const AQFloat4x4 m0, const AQFloat4x4 m1);
AQFloat4x4 aqmath_perspective_matrix(AQFloat feild_of_view, AQFloat aspect,
   AQFloat near_z, AQFloat far_z);
AQFloat4x4 aqmath_perspective_matrix_from_res(AQInt width, AQInt height,
      AQFloat feild_of_view, float near_z, float far_z );
AQFloat4x4 aqmath_orthographic_matrix(AQFloat left, AQFloat right,AQFloat bottom,
   AQFloat top, AQFloat near_z, AQFloat far_z);
AQFloat4x4 aqmath_translate_matrix(AQFloat4x4 m0, AQFloat x_trans,
   AQFloat y_trans, AQFloat z_trans);
AQFloat4x4 aqmath_scale_matrix(AQFloat4x4 m0, AQFloat x_scale,
   AQFloat y_scale, AQFloat z_scale);
AQFloat4 aqmath_get_quaternion_rotation(AQFloat x, AQFloat y, AQFloat z, AQFloat degrees);
AQFloat4 aqmath_merge_quaternion_rotation(AQFloat4 q0, AQFloat4 q1);
AQFloat4x4 aqmath_get_rotation_matrix_from_quaternion(AQFloat4 quaternion);
AQFloat4x4 aqmath_rotate_x(AQFloat4x4 m0, AQFloat degrees);
AQFloat4x4 aqmath_rotate_y(AQFloat4x4 m0, AQFloat degrees);
AQFloat4x4 aqmath_rotate_z(AQFloat4x4 m0, AQFloat degrees);
AQFloat4x4 aqmath_rotate_matrix(AQFloat4x4 m0, AQFloat degree,
   AQFloat x_axis, AQFloat y_axis, AQFloat z_axis);
AQFloat4x4 aqmath_transform_matrix(AQFloat3 pos, AQFloat3 rot, float scale);
AQFloat4x4 aqmath_view_matrix(AQFloat3 pos, AQFloat pitch,
   AQFloat yaw, AQFloat roll);

void aqmath_seed_random_state(AQRandState rs, AQULong seed);
void aqmath_seed_random_state_with_time(AQRandState rs);
AQULong aqmath_random_value(AQRandState rs);
AQDouble aqmath_a_random_double(AQRandState rs, AQDouble min, AQDouble max);
AQULong aqmath_a_random_long(AQRandState rs, AQULong min, AQULong max);
AQUInt aqmath_a_random_int(AQRandState rs, AQUInt min, AQUInt max);
#endif /* aquarius_h */
