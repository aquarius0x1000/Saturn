#include "prometheus.h"
 
struct PrometheusDeserializer_s {
  AQ_DATA_STRUCTURE_BASE_CLASS  
  DeimosFile file;
  AQStore generators;
  AQStore adders;  
};
 
struct PrometheusDataStructure_s {
  PROMETHEUS_SERIALIZATION_BASE_CLASS
};
 
static AQInt prometheus_internal_serialize(DeimosFile file, AQChar* label, PrometheusDataStructure ds);
 
 static AQInt prometheus_internal_output_list(DeimosFile file, AQDataStructure ds, 
  PrometheusGetValueLambda get_value) {  
    AQInt result = 0;
    AQULong index = 0;
    AQInt last_result = 0;
    AQMTAContainer container;
    deimos_output_character(file,'(');
   next:
    last_result = result;
    result = get_value(ds,&index,&container);
    if (result && last_result) deimos_output_character(file,','); 
    if (result == PROMETHEUS_LIST_DONE) goto end;
    switch(container.type) {
            case AQByteFlag:
              deimos_output_byte(file,container.AQByteVal);
            break;
            case AQSByteFlag:
              deimos_output_sbyte(file,container.AQSByteVal);
            break;
            case AQShortFlag:
              deimos_output_short(file,container.AQShortVal);
            break;
            case AQUShortFlag:
              deimos_output_ushort(file,container.AQUShortVal);
            break;
            case AQIntFlag:
              deimos_output_integer(file,container.AQIntVal);
            break;
            case AQUIntFlag:
              deimos_output_uinteger(file,container.AQUIntVal);
            break;
            case AQLongFlag:
              deimos_output_long(file,container.AQLongVal);
            break;
            case AQULongFlag:
              deimos_output_ulong(file,container.AQULongVal);
            break;
            case AQFloatFlag:
              deimos_output_float(file,container.AQFloatVal);
            break;
            case AQDoubleFlag:
              deimos_output_double(file,container.AQDoubleVal);
            break;
            case AQAnyFlag:
                if (aqds_get_flag(container.AQAnyVal) == AQStringFlag) {
                    deimos_output_character(file,'"');
                    deimos_output_string(file,aqstring_get_c_string(container.AQAnyVal));
                    deimos_output_character(file,'"');
                }
            break;
            default:
            break;
        }      
   end:     
    if (result) goto next;
    deimos_output_character(file,')'); 
    return 1;
}
 
static AQInt prometheus_internal_output_block(DeimosFile file, AQDataStructure ds, 
 PrometheusOutputBlockLambda output_block) {
    AQInt result = 0;
    deimos_output_character(file,'{');
    deimos_output_character(file,'\n');    
    deimos_output_add_to_tab(file,1);    
   loop:    
    result = output_block(file,ds);
    if (result == PROMETHEUS_BLOCK_NOT_DONE) goto loop;
    if (result != PROMETHEUS_BLOCK_DONE) return 0;
    deimos_output_sub_from_tab(file,1);
    deimos_output_tab(file);
    deimos_output_character(file,'}');
} 

static AQInt prometheus_internal_output_container(DeimosFile file, AQChar* label, 
 AQChar* type, AQDataStructure ds, PrometheusOutputContainerLambda output_container) {
    deimos_output_tab(file);  
    deimos_output_character(file,'[');
    if (label != NULL) {
        deimos_output_character(file,'"');
        deimos_output_string(file,label);
        deimos_output_character(file,'"');
        deimos_output_character(file,',');
    }
    deimos_output_string(file,type);
    deimos_output_character(file,']'); 
    deimos_output_character(file,' ');
    deimos_output_character(file,':');
    deimos_output_character(file,' ');
    output_container(file,ds,prometheus_internal_output_list,prometheus_internal_output_block);
    deimos_output_character(file,';'); 
    deimos_output_character(file,'\n');     
    return 1; 
}

/////////////////////////////////////////////////////Start AQDataStructures//////////////////////////////////////////

static AQInt prometheus_internal_output_array_block(DeimosFile file, AQDataStructure ds) {
    aq_array_foreach(index,ds) {
       prometheus_serialize(file,aqarray_get_item(ds,index)); 
    }
    return PROMETHEUS_BLOCK_DONE;
}

static AQInt prometheus_internal_output_array_container(DeimosFile file, AQDataStructure ds, 
 PrometheusPrintListLambda print_list, PrometheusPrintBlockLambda print_block) {
    return print_block(file,ds,prometheus_internal_output_array_block);
}

#define prometheus_macro_define_get_values(the_type)\
 static AQInt prometheus_internal_get_##the_type##s_for_list(AQDataStructure ds, AQULong* index,\
 AQMTAContainer* container) {\
    AQULong i = *index;\
    if (i >= aq_mta_get_num_of_items(the_type,ds)) return PROMETHEUS_LIST_DONE;\
    AQMTAContainer the_container;\
    the_container.flag = AQMTAContainerFlag;\
    the_container.type = the_type##Flag;\
    the_container.the_type##Val = aq_mta_get_item(the_type,ds,i);\
    *container = the_container;\
    i++;\
    *index = i;\
    return PROMETHEUS_LIST_NOT_DONE;\
}\
static AQInt prometheus_internal_output_##the_type##s_container(DeimosFile file, AQDataStructure ds,\
 PrometheusPrintListLambda print_list, PrometheusPrintBlockLambda print_block) {\
    return print_list(file,ds,prometheus_internal_get_##the_type##s_for_list);\
}

prometheus_macro_define_get_values(AQByte);
prometheus_macro_define_get_values(AQSByte);
prometheus_macro_define_get_values(AQShort);
prometheus_macro_define_get_values(AQUShort);
prometheus_macro_define_get_values(AQInt);
prometheus_macro_define_get_values(AQUInt);
prometheus_macro_define_get_values(AQLong);
prometheus_macro_define_get_values(AQULong);
prometheus_macro_define_get_values(AQFloat);
prometheus_macro_define_get_values(AQDouble);
prometheus_macro_define_get_values(AQAny);

#define prometheus_macro_get_values(type,type_name)\ 
 if (aq_mta_get_num_of_items(type,ds) > 0) {\
     prometheus_internal_output_container(file,NULL,type_name,ds,\
         prometheus_internal_output_##type##s_container);\
 }

static AQInt prometheus_internal_output_mta_block(DeimosFile file, AQDataStructure ds) {
    prometheus_macro_get_values(AQByte,"@Bytes");
    prometheus_macro_get_values(AQSByte,"@SBytes");
    prometheus_macro_get_values(AQShort,"@Shorts");
    prometheus_macro_get_values(AQUShort,"@UShorts");
    prometheus_macro_get_values(AQInt,"@Ints");
    prometheus_macro_get_values(AQUInt,"@UInts");
    prometheus_macro_get_values(AQLong,"@Longs");
    prometheus_macro_get_values(AQULong,"@Ulongs");
    prometheus_macro_get_values(AQFloat,"@Floats");
    prometheus_macro_get_values(AQDouble,"@Doubles");
    prometheus_macro_get_values(AQAny,"@Texts");
    return PROMETHEUS_BLOCK_DONE;
}

static AQInt prometheus_internal_output_mta_container(DeimosFile file, AQDataStructure ds, 
 PrometheusPrintListLambda print_list, PrometheusPrintBlockLambda print_block) {
    return print_block(file,ds,prometheus_internal_output_mta_block);
}

static AQInt prometheus_internal_get_value_for_list(AQDataStructure ds, AQULong* index,
 AQMTAContainer* container) {
    AQULong i = *index; 
    AQMTAContainer the_container;
    *container = *((AQMTAContainer*)ds);
    i++;
    *index = i;
    return (i == 1) ? PROMETHEUS_LIST_NOT_DONE : PROMETHEUS_LIST_DONE;
}

static AQInt prometheus_internal_output_value_container(DeimosFile file, AQDataStructure ds, 
 PrometheusPrintListLambda print_list, PrometheusPrintBlockLambda print_block) {
    return print_list(file,ds,prometheus_internal_get_value_for_list);
}

#define prometheus_macro_get_value(the_type,type_name)\
 if (((AQMTAContainer*)ds)->type == the_type##Flag) {\
     prometheus_internal_output_container(file,label,type_name,ds,\
         prometheus_internal_output_value_container);\
 }

static AQInt prometheus_internal_get_text_for_list(AQDataStructure ds, AQULong* index,
 AQMTAContainer* container) {
    AQULong i = *index; 
    AQMTAContainer the_container;
    the_container.flag = AQMTAContainerFlag;
    the_container.type = AQAnyFlag;
    the_container.AQAnyVal = ds;
    *container = the_container;
    i++;
    *index = i;
    return (i == 1) ? PROMETHEUS_LIST_NOT_DONE : PROMETHEUS_LIST_DONE;
}

static AQInt prometheus_internal_output_text_container(DeimosFile file, AQDataStructure ds, 
 PrometheusPrintListLambda print_list, PrometheusPrintBlockLambda print_block) {
    return print_list(file,ds,prometheus_internal_get_text_for_list);
}

static AQInt prometheus_internal_output_store_block(DeimosFile file, AQDataStructure ds) {
    aq_store_foreach(node,ds) {
        AQInt result = prometheus_serialize_with_label(file,
           aqstore_label_from_list_node(node),aqlist_get_item(node));
        if (!result) return PROMETHEUS_BLOCK_DONE;    
    }
    return PROMETHEUS_BLOCK_DONE;
}

static AQInt prometheus_internal_output_store_container(DeimosFile file, AQDataStructure ds, 
 PrometheusPrintListLambda print_list, PrometheusPrintBlockLambda print_block) {
    return print_block(file,ds,prometheus_internal_output_store_block);
}

/////////////////////////////////////////////////////End AQDataStructures//////////////////////////////////////////

static AQChar* prometheus_internal_validate_label(AQString label) {
    aq_string_for_each_character(index,label) {
        AQSByte offset = 0;
        AQInt character;
        if (aqstring_get_size(label) > 100) return NULL;
        character = aqstring_get_character(label,index,&offset);
        if (character == '"') return NULL;
        index += offset;
    }
    return aqstring_get_c_string(label);
}

static AQInt prometheus_internal_validate_type(AQString type) {
    aq_string_for_each_character(index,type) {
            AQSByte offset = 0;
            if (aqstring_get_size(type) > 100) return 0;
            AQInt character = aqstring_get_character(type,index,&offset);
            if (index == 0 && character != '#') return 0;
            if (isspace(character) 
              || character == '@'
              || character == '$'
              || character == '!'
              || character == '?'
              || character == '%'
              || character == '+'
              || character == '/'
              || character == '.'
              || character == '\''
              || character == '\\'
              || character == '"'
              || character == '*'
              || character == '^'
              || character == '&'
              || character == '('
              || character == ')'
              || character == '{'
              || character == '}'
              || character == '['
              || character == ']'
              || character == '='
              || character == '>'
              || character == '<'
              || character == ',') 
            return 0;
            index += offset; 
    }
}

static AQInt prometheus_access_output_container(DeimosFile file, AQChar* label, 
 AQString type, AQDataStructure ds, PrometheusOutputContainerLambda output_container) {
   if (type != NULL) {
        if (!prometheus_internal_validate_type(type))
         return 0;
    }   
    return 
     prometheus_internal_output_container(file,
         label,aqstring_convert_to_c_string(type),ds,output_container);
}

static AQInt prometheus_internal_serialize(DeimosFile file, AQChar* label, PrometheusDataStructure ds) {
    if (ds == NULL) return 0;
    if (ds->flag == AQDestroyableFlag) {
        if (ds->serialize != NULL) ds->serialize(file,label,ds,prometheus_access_output_container);
    }
    switch (ds->flag) {
        case AQArrayFlag:
            prometheus_internal_output_container(file,label,"@Array",ds,prometheus_internal_output_array_container);
        break;
        
        case AQMultiTypeArrayFlag:
            prometheus_internal_output_container(file,label,"@Values",ds,prometheus_internal_output_mta_container);
        break;
        
        case AQStringFlag:
            prometheus_internal_output_container(file,label,"@Text",ds,prometheus_internal_output_text_container);
        break;
        
        case AQStoreFlag:
            prometheus_internal_output_container(file,label,"@Store",ds,prometheus_internal_output_store_container);
        break;
        
        case AQMTAContainerFlag:
            prometheus_macro_get_value(AQByte,"@Byte");
            prometheus_macro_get_value(AQSByte,"@SByte");
            prometheus_macro_get_value(AQShort,"@Short");
            prometheus_macro_get_value(AQUShort,"@UShort");
            prometheus_macro_get_value(AQInt,"@Int");
            prometheus_macro_get_value(AQUInt,"@UInt");
            prometheus_macro_get_value(AQLong,"@Long");
            prometheus_macro_get_value(AQULong,"@ULong");
            prometheus_macro_get_value(AQFloat,"@Float");
            prometheus_macro_get_value(AQDouble,"@Double");
        break;
        
        default:
         return 0;
        break;
    }
    return 1;
}

AQInt prometheus_serialize(DeimosFile file, PrometheusDataStructure ds) {
    return prometheus_internal_serialize(file,NULL,ds);
}

AQInt prometheus_serialize_with_label(DeimosFile file, AQString label, PrometheusDataStructure ds) {
    AQChar* valid_label = 
     prometheus_internal_validate_label(label);
    if (valid_label == NULL) return 0;
    return prometheus_internal_serialize(file,valid_label,ds);
}

static AQDataStructure prometheus_internal_generate_container(PrometheusDeserializer deserializer, 
 AQChar* adder_type, AQDataStructure ds);

#define prometheus_macro_validate_character(character)\
 if (deimos_peek_utf32_character(file,&offset) == character) {\
    deimos_advance_file_position(file,offset);\
} else {\
    return NULL;\
} 

#define prometheus_macro_validate_last_character(character)\
 if (deimos_peek_last_utf32_character(file,0) == character) {\
} else {\
    return NULL;\
} 

#define prometheus_macro_if_not_character(character,label)\
 if (deimos_peek_utf32_character(file,&offset) == character) {\
    deimos_advance_file_position(file,offset);\
} else {\
    goto label;\
}

#define prometheus_macro_if_not_last_character(character,label)\
 if (deimos_peek_last_utf32_character(file,0) == character) {\
} else {\
    goto label;\
}

static AQDataStructure prometheus_internal_generate_value(PrometheusDeserializer deserializer, 
 AQDataStructure ds, PrometheusProcessValueLambda process_value, AQTypeFlag* type_array, AQULong type_count) { 
    AQULong offset; 
    AQTypeFlag type;
    AQULong index = -1;
    AQMTAContainer container;
    container.flag = AQMTAContainerFlag;
    DeimosFile file = deserializer->file; 
    prometheus_macro_validate_character(':');
    prometheus_macro_validate_character('(');
   next:
    index++;
    if (index >= type_count) goto end;
    type = type_array[index];
    container.type = type;
    switch(type) {
            case AQByteFlag:
                container.AQByteVal = deimos_get_byte(file);
            break;
            case AQSByteFlag:
                container.AQSByteVal = deimos_get_sbyte(file);
            break;
            case AQShortFlag:
                container.AQShortVal = deimos_get_short(file);
            break;
            case AQUShortFlag:
                container.AQUShortVal = deimos_get_ushort(file);
            break;
            case AQIntFlag:
                container.AQIntVal = deimos_get_integer(file);
            break;
            case AQUIntFlag:
                container.AQUIntVal = deimos_get_uinteger(file);
            break;
            case AQLongFlag:
                container.AQLongVal = deimos_get_long(file);
            break;
            case AQULongFlag:
                container.AQULongVal = deimos_get_ulong(file);
            break;
            case AQFloatFlag:
                container.AQFloatVal = deimos_get_float(file);
            break;
            case AQDoubleFlag:
                container.AQDoubleVal = deimos_get_double(file);
            break;
            case AQAnyFlag:
                container.AQAnyVal = deimos_get_string(file,'"','"');
                deimos_advance_file_position(file,1);
            break;
            default:
            break;
        }
    if (!process_value(ds,&type,&index,&container))
     return NULL;         
   end:
    if ((index+1) < type_count) {
        prometheus_macro_if_not_last_character(',',fin); 
        goto next;
    }
   fin: 
    prometheus_macro_validate_last_character(')');
    prometheus_macro_validate_character(';');
    return ds;
}

static AQDataStructure prometheus_internal_generate_block(PrometheusDeserializer deserializer, 
 AQChar* adder_type, AQDataStructure ds) {
    AQULong offset;
    AQLong balance = 0;
    DeimosFile file = deserializer->file;   
    if (deimos_peek_utf32_character(file,&offset) == ':') {
        deimos_advance_file_position(file,offset);
        if (deimos_peek_utf32_character(file,&offset) == '{') {   
            deimos_advance_file_position(file,offset);     
           get_container:
            prometheus_internal_generate_container(deserializer,adder_type,ds);
            if (deimos_peek_utf32_character(file,&offset) == '}') {
                deimos_advance_file_position(file,offset);
                if (deimos_peek_utf32_character(file,&offset) == ';') { 
                    deimos_advance_file_position(file,offset);
                    return ds;            
                }           
            } else if (deimos_peek_utf32_character(file,&offset) == '[') {
                goto get_container;
            }              
        }
    }
    //fail
    goto the_loop;
   end_block: 
    if (deimos_peek_utf32_character(file,&offset) == '}') {
        deimos_advance_file_position(file,offset);
        if (deimos_peek_utf32_character(file,&offset) == ';') { 
            deimos_advance_file_position(file,offset);
            if (deimos_peek_utf32_character(file,&offset) == '[')
             goto get_container;
        }
    }
   the_loop:
    balance = 1;
   loop:
    if (deimos_peek_utf32_character(file,&offset) != '['
     && deimos_peek_utf32_character(file,&offset) != EOF 
     || (balance != 0 
     && deimos_peek_utf32_character(file,&offset) == '[' 
     && deimos_peek_utf32_character(file,&offset) != EOF)) {
        if (deimos_peek_utf32_character(file,&offset) == '[')
         balance++; 
        if (deimos_peek_utf32_character(file,&offset) == ';')
         balance--;
        if (balance < 0) {         
            if (deimos_peek_utf32_character(file,&offset) == ';') { 
                deimos_advance_file_position(file,offset);
                if (deimos_peek_utf32_character(file,&offset) == '[')
                 goto get_container;
            } else if (deimos_peek_utf32_character(file,&offset) == '}') {
             goto end_block;    
            }        
            return ds;
        }   
        deimos_advance_file_position(file,offset);
        if (deimos_peek_utf32_character(file,&offset) == '[' 
         && balance == 0)
          goto get_container;
        goto loop;
    }
    return ds;      
}

static AQInt prometheus_internal_register_deserializer(PrometheusDeserializer deserializer, AQChar* type,
 PrometheusGeneratorLambda generator, PrometheusAdderLambda adder) {
    AQInt result = 0; 
    result = aqstore_add_item(deserializer->generators,generator,type);
    if (!result) return 0;
    result = aqstore_add_item(deserializer->adders,adder,type);
    return result;
}

/////////////////////////////////////////////////////Start AQDataStructures//////////////////////////////////////////

#define prometheus_macro_define_gen_value(the_type)\
 static AQInt prometheus_internal_process_value_##the_type(AQDataStructure ds, AQTypeFlag* type, AQULong* index,\ 
  AQMTAContainer* container) {\
     *index = -1;\
     AQMTAContainer* the_container = ds;\
     the_container->type = container->type;\
     the_container->the_type##Val =  container->the_type##Val;\
     return 1;\
 }\
 static AQDataStructure prometheus_internal_value_generator_##the_type(PrometheusDeserializer deserializer,\ 
  AQChar* adder_type, AQDataStructure super_ds, PrometheusAccessBlockGeneratorLambda block_generator,\
   PrometheusAccessValueGeneratorLambda value_generator) {\
     AQMTAContainer* container = aq_new(AQMTAContainer,deimos_get_allocator(deserializer->file));\
     container->flag = AQMTAContainerFlag;\
     AQMTAContainer* the_container =\
      value_generator(deserializer,container,\
       prometheus_internal_process_value_##the_type,(AQTypeFlag[]){the_type##Flag},1);\
     if (the_container == NULL) return NULL;\
     if (the_container->type == AQAnyFlag) return the_container->AQAnyVal;\
     return the_container;\
 }

prometheus_macro_define_gen_value(AQByte);
prometheus_macro_define_gen_value(AQSByte);
prometheus_macro_define_gen_value(AQShort);
prometheus_macro_define_gen_value(AQUShort);
prometheus_macro_define_gen_value(AQInt);
prometheus_macro_define_gen_value(AQUInt);
prometheus_macro_define_gen_value(AQLong);
prometheus_macro_define_gen_value(AQULong);
prometheus_macro_define_gen_value(AQFloat);
prometheus_macro_define_gen_value(AQDouble);
prometheus_macro_define_gen_value(AQAny);

static AQDataStructure prometheus_internal_value_adder(AQDataStructure ds,
 AQDataStructure ds_to_add, AQChar* label) {
    return ds_to_add;
}

#define prometheus_macro_register_value(the_type,type_name)\
 result = prometheus_internal_register_deserializer(deserializer,type_name,\
       prometheus_internal_value_generator_##the_type,prometheus_internal_value_adder);

#define prometheus_macro_define_gen_values(the_type)\
 static AQInt prometheus_internal_process_values_##the_type(AQDataStructure ds, AQTypeFlag* type, AQULong* index,\ 
  AQMTAContainer* container) {\
     *index = -1;\
     aq_mta_add_item(the_type,ds,container->the_type##Val);\
     return 1;\
 }\
 static AQDataStructure prometheus_internal_values_generator_##the_type(PrometheusDeserializer deserializer,\ 
  AQChar* adder_type, AQDataStructure super_ds, PrometheusAccessBlockGeneratorLambda block_generator,\
   PrometheusAccessValueGeneratorLambda value_generator) {\
     if (aqds_get_flag(super_ds) != AQMultiTypeArrayFlag) return NULL;\
     AQDataStructure ds_to_add =\
      value_generator(deserializer,super_ds,\
       prometheus_internal_process_values_##the_type,(AQTypeFlag[]){the_type##Flag},1);\
     if (ds_to_add == NULL) return NULL;\
     return ds_to_add;\
 }

prometheus_macro_define_gen_values(AQByte);
prometheus_macro_define_gen_values(AQSByte);
prometheus_macro_define_gen_values(AQShort);
prometheus_macro_define_gen_values(AQUShort);
prometheus_macro_define_gen_values(AQInt);
prometheus_macro_define_gen_values(AQUInt);
prometheus_macro_define_gen_values(AQLong);
prometheus_macro_define_gen_values(AQULong);
prometheus_macro_define_gen_values(AQFloat);
prometheus_macro_define_gen_values(AQDouble);
prometheus_macro_define_gen_values(AQAny);

static AQDataStructure prometheus_internal_values_adder(AQDataStructure ds,
 AQDataStructure ds_to_add, AQChar* label) {
    return ds_to_add;
}

#define prometheus_macro_register_values(the_type,type_name)\
 result = prometheus_internal_register_deserializer(deserializer,type_name,\
       prometheus_internal_values_generator_##the_type,prometheus_internal_values_adder);

static AQDataStructure prometheus_internal_mta_generator(PrometheusDeserializer deserializer, 
 AQChar* adder_type, AQDataStructure super_ds, PrometheusAccessBlockGeneratorLambda block_generator,
  PrometheusAccessValueGeneratorLambda value_generator) {
    AQMultiTypeArray mta = aq_new_mta(deimos_get_allocator(deserializer->file)); 
    if (mta == NULL) return NULL;
    return block_generator(deserializer,adder_type,mta);
}

static AQDataStructure prometheus_internal_mta_adder(AQDataStructure ds,
 AQDataStructure ds_to_add, AQChar* label) {
    return ds_to_add;
}

static AQDataStructure prometheus_internal_array_generator(PrometheusDeserializer deserializer, 
 AQChar* adder_type, AQDataStructure super_ds, PrometheusAccessBlockGeneratorLambda block_generator,
  PrometheusAccessValueGeneratorLambda value_generator) {
    AQArray array = aq_new_array(deimos_get_allocator(deserializer->file)); 
    if (array == NULL) return NULL;
    return block_generator(deserializer,adder_type,array);
}

static AQDataStructure prometheus_internal_array_adder(AQDataStructure ds,
 AQDataStructure ds_to_add, AQChar* label) {
    aqarray_add_item((AQArray)ds,ds_to_add);
    return ds_to_add;
}

static AQDataStructure prometheus_internal_store_generator(PrometheusDeserializer deserializer, 
 AQChar* adder_type, AQDataStructure super_ds, PrometheusAccessBlockGeneratorLambda block_generator,
  PrometheusAccessValueGeneratorLambda value_generator) {
    AQStore store = aq_new_store(deimos_get_allocator(deserializer->file)); 
    if (store == NULL) return NULL;
    return block_generator(deserializer,adder_type,store);
}

static AQDataStructure prometheus_internal_store_adder(AQDataStructure ds,
 AQDataStructure ds_to_add, AQChar* label) {
    AQInt result = 0; 
    result = aqstore_add_item((AQStore)ds,ds_to_add,label);
    if (!result) return NULL;
    return ds_to_add;
}

static AQInt prometheus_internal_register_aqdatastructures(PrometheusDeserializer deserializer) {
    AQInt result = 0;
    prometheus_macro_register_value(AQByte,"@Byte");
    prometheus_macro_register_value(AQSByte,"@SByte");
    prometheus_macro_register_value(AQShort,"@Short");
    prometheus_macro_register_value(AQUShort,"@UShort");
    prometheus_macro_register_value(AQInt,"@Int");
    prometheus_macro_register_value(AQUInt,"@UInt");
    prometheus_macro_register_value(AQLong,"@Long");
    prometheus_macro_register_value(AQULong,"@ULong");
    prometheus_macro_register_value(AQByte,"@Float");
    prometheus_macro_register_value(AQByte,"@Double");
    prometheus_macro_register_value(AQAny,"@Text");
    prometheus_macro_register_values(AQByte,"@Bytes");
    prometheus_macro_register_values(AQSByte,"@SByte");
    prometheus_macro_register_values(AQShort,"@Short");
    prometheus_macro_register_values(AQUShort,"@UShort");
    prometheus_macro_register_values(AQInt,"@Ints");
    prometheus_macro_register_values(AQUInt,"@UInts");
    prometheus_macro_register_values(AQLong,"@Longs");
    prometheus_macro_register_values(AQULong,"@ULongs");
    prometheus_macro_register_values(AQFloat,"@Floats");
    prometheus_macro_register_values(AQDouble,"@Doubles");
    prometheus_macro_register_values(AQAny,"@Texts");
    result = prometheus_internal_register_deserializer(deserializer,"@Values",
       prometheus_internal_mta_generator,prometheus_internal_mta_adder);
    result = prometheus_internal_register_deserializer(deserializer,"@Array",
       prometheus_internal_array_generator,prometheus_internal_array_adder);
    result = prometheus_internal_register_deserializer(deserializer,"@Store",
       prometheus_internal_store_generator,prometheus_internal_store_adder);  
    return result; 
}

/////////////////////////////////////////////////////End AQDataStructures//////////////////////////////////////////

static AQDataStructure prometheus_internal_generate_container(PrometheusDeserializer deserializer, 
 AQChar* adder_type, AQDataStructure ds) {
    AQULong offset;
    AQString type = NULL;
    AQString label = NULL;
    AQString valid_type = NULL;
    AQDataStructure ret_ds = NULL;
    AQDataStructure ds_to_add = NULL;
    DeimosFile file = deserializer->file;
    if (deimos_peek_utf32_character(file,&offset) != '[') return NULL;
    deimos_advance_file_position(file,offset);   
    if (deimos_peek_utf32_character(file,&offset) == '@'
      || deimos_peek_utf32_character(file,&offset) == '#') {  
   validate_type:
        if (deimos_peek_utf32_character(file,&offset) == '@') {
            type = deimos_get_string(file,'@',']');
            if (aqstring_get_size(type) > 7) return NULL;
            valid_type = aq_add_strings(aqstr("@"),type);
        }
        if (deimos_peek_utf32_character(file,&offset) == '#') {
            type = deimos_get_string(file,'#',']');
            valid_type = aq_add_strings(aqstr("#"),type);
            if (!prometheus_internal_validate_type(valid_type)) return NULL;
        }
        PrometheusGeneratorLambda generator = 
         aqstore_get_item(deserializer->generators,aqstring_get_c_string(valid_type));
        if (generator == NULL) return NULL;
        ds_to_add = generator(deserializer,aqstring_get_c_string(valid_type),ds,
         prometheus_internal_generate_block,prometheus_internal_generate_value);
        aqstring_destroy(type);
        aqstring_destroy(valid_type);
        type = NULL;
        valid_type = NULL;
        if (ds == NULL) return ds_to_add;
        if (adder_type == NULL) return ds_to_add;
        PrometheusAdderLambda adder =
         aqstore_get_item(deserializer->adders,adder_type);
        if (adder == NULL) return NULL;
        ret_ds = adder(ds,ds_to_add,aqstring_get_c_string(label));
        return ret_ds;
    }
    if (deimos_peek_utf32_character(file,&offset) != '"')
     return NULL;
    label = deimos_get_string(file,'"','"');   
    if (prometheus_internal_validate_label(label) == NULL)
     return NULL;
    if (deimos_peek_utf32_character(file,&offset) != ',')
     return NULL; 
    deimos_advance_file_position(file,offset);
    if (deimos_peek_utf32_character(file,&offset) != '@'
      && deimos_peek_utf32_character(file,&offset) != '#')
     return NULL;    
    goto validate_type;
    return NULL; 
}

static AQDataStructure prometheus_internal_get_container(PrometheusDeserializer deserializer) {
    return prometheus_internal_generate_container(deserializer,NULL,NULL);
}

static AQDataStructure prometheus_internal_deserialize(PrometheusDeserializer deserializer) {
    AQULong offset;
    AQArray array = NULL;
    AQDataStructure ds = NULL;
    DeimosFile file = deserializer->file;
   get_container:
    if (deimos_peek_utf32_character(file,&offset) != '[')
        return NULL;       
    if (deimos_peek_utf32_character(file,&offset) == '[')    
        ds = prometheus_internal_get_container(deserializer);
    if (deimos_peek_utf32_character(file,&offset) == EOF 
         && array == NULL)
        return ds;
    if (deimos_peek_utf32_character(file,&offset) == EOF
         && array != NULL) {
        aqarray_add_item(array,ds);
        return array;               
    }
    if (deimos_peek_utf32_character(file,&offset) == '[') {
        if (array == NULL) array = aq_new_array();
        aqarray_add_item(array,ds);
        ds = NULL;
        goto get_container;
    }
    return ds;
}

PrometheusDeserializer prometheus_deserializer_new(DeimosFile file) {
    PrometheusDeserializer deserializer = aq_new(struct PrometheusDeserializer_s);
    deserializer->file = file;
    deserializer->generators = aq_new_store(deimos_get_allocator(file));
    deserializer->adders = aq_new_store(deimos_get_allocator(file));
    deserializer->destroyer = prometheus_deserializer_destroy;
    if (!prometheus_internal_register_aqdatastructures(deserializer))
     return NULL;
    return deserializer;
}
 
void prometheus_deserializer_destroy(PrometheusDeserializer deserializer) {
    aqstore_destroy(deserializer->generators);
    aqstore_destroy(deserializer->adders);
    free(deserializer);
}

AQInt prometheus_register_deserializer(PrometheusDeserializer deserializer, AQString name,
 PrometheusGeneratorLambda generator, PrometheusAdderLambda adder) {
    if (!prometheus_internal_validate_type(name)) return 0;
    AQChar* valid_type = aqstring_get_c_string(name);
    return prometheus_internal_register_deserializer(deserializer,valid_type,generator,adder);
}

AQDataStructure prometheus_deserialize(PrometheusDeserializer deserializer) {
    return prometheus_internal_deserialize(deserializer);
}