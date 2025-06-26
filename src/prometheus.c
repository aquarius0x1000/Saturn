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
 
static AQStatus prometheus_internal_serialize(DeimosFile file, AQChar* label, PrometheusDataStructure ds);
 
static AQStatus prometheus_internal_output_list(DeimosFile file, AQDataStructure ds, 
 PrometheusGetValueLambda get_value) {  
    PrometheusListStatus result = PrometheusListDone;
    AQULong index = 0;
    AQInt last_result = 0;
    AQMTAContainer container;
    if (deimos_output_character(file,'(') == DeimosFailure) return AQFailureValue;
   next:
    last_result = result;
    result = get_value(ds,&index,&container);
    if (result && last_result) deimos_output_character(file,','); 
    if (result == PrometheusListDone) goto end;
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
    if (deimos_output_character(file,')') == DeimosFailure) return AQFailureValue;
    return AQSuccessValue;
}
 
static AQStatus prometheus_internal_output_block(DeimosFile file, AQDataStructure ds, 
 PrometheusOutputBlockLambda output_block) {
    PrometheusBlockStatus result = PrometheusBlockNotDone;
    if (deimos_output_character(file,'{') == DeimosFailure) return AQFailureValue;
    deimos_output_character(file,'\n');    
    deimos_output_add_to_tab(file,1);    
   loop:    
    result = output_block(file,ds);
    if (result == PrometheusBlockNotDone) goto loop;
    if (result != PrometheusBlockDone) return AQFailureValue;
    deimos_output_sub_from_tab(file,1);
    deimos_output_tab(file);
    if (deimos_output_character(file,'}') == DeimosFailure) return AQFailureValue;
    return AQSuccessValue;
} 

static AQStatus prometheus_internal_output_container(DeimosFile file, AQChar* label, 
 AQChar* type, AQDataStructure ds, PrometheusOutputContainerLambda output_container) {
    AQStatus status;
    if (deimos_output_tab(file) == DeimosFailure) return AQFailureValue;  
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
    status = output_container(file,ds,prometheus_internal_output_list,prometheus_internal_output_block);
    deimos_output_character(file,';'); 
    if (deimos_output_character(file,'\n') == DeimosFailure) return AQFailureValue;     
    return status; 
}

/////////////////////////////////////////////////////Start AQDataStructures//////////////////////////////////////////

static PrometheusBlockStatus prometheus_internal_output_array_block(DeimosFile file, AQDataStructure ds) {
    aq_array_foreach(index,ds) {
        prometheus_serialize(file,aqarray_get_item(ds,index)); 
    }
    return PrometheusBlockDone;
}

static AQStatus prometheus_internal_output_array_container(DeimosFile file, AQDataStructure ds, 
 PrometheusPrintListLambda print_list, PrometheusPrintBlockLambda print_block) {
    return print_block(file,ds,prometheus_internal_output_array_block);
}

#define prometheus_macro_define_get_values(the_type)\
 static PrometheusListStatus prometheus_internal_get_##the_type##s_for_list(AQDataStructure ds, AQULong* index,\
 AQMTAContainer* container) {\
    AQULong i = *index;\
    if (i >= aq_mta_get_num_of_items(the_type,ds)) return PrometheusListDone;\
    AQMTAContainer the_container;\
    the_container.flag = AQMTAContainerFlag;\
    the_container.type = the_type##Flag;\
    the_container.the_type##Val = aq_mta_get_item(the_type,ds,i);\
    *container = the_container;\
    i++;\
    *index = i;\
    if (the_type##Flag == AQAnyFlag) {\
        the_type item = the_container.the_type##Val;\
        if (aq_get_ds_flag_for_any(item) != AQStringFlag) return PrometheusListDone;\
    }\
    return PrometheusListNotDone;\
}\
static AQStatus prometheus_internal_output_##the_type##s_container(DeimosFile file, AQDataStructure ds,\
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

#define prometheus_macro_get_values(the_type,type_name)\
 if (the_type##Flag != AQAnyFlag)\
  if (aq_mta_get_num_of_items(the_type,ds) > 0) {\
      if (aq_mta_get_num_of_items(the_type,ds) == 1) {\
          AQMTAContainer* container =\
           aq_new(AQMTAContainer,deimos_get_allocator(file));\
         container->flag = AQMTAContainerFlag;\
         container->type = the_type##Flag;\
         container->the_type##Val =\
          aq_mta_get_item(the_type,ds,0);\
         prometheus_serialize(file,container);\
         aq_free(container,deimos_get_allocator(file));\
      }\
     if (aq_mta_get_num_of_items(the_type,ds) > 1)\
      prometheus_internal_output_container(file,NULL,type_name,ds,\
          prometheus_internal_output_##the_type##s_container);\
  }
 
 static PrometheusBlockStatus prometheus_internal_output_items_block(DeimosFile file, AQDataStructure ds) {
    aq_mta_foreach(AQAny,index,ds) {
        AQDataStructure item = aq_mta_get_item(AQAny,ds,index);
        if (aqds_get_flag(item) == AQStringFlag) {
            if ((index+1) >= aq_mta_get_num_of_items(AQAny,ds))
             goto serialize;
            item = aq_mta_get_item(AQAny,ds,index+1);
            if (aqds_get_flag(item) == AQStringFlag) {
                AQULong count = 0;
                item = aq_mta_get_item(AQAny,ds,index);
                while (aqds_get_flag(item) == AQStringFlag) {
                    count++;
                    if ((index+count) >= aq_mta_get_num_of_items(AQAny,ds))
                     break;
                    item = aq_mta_get_item(AQAny,ds,index+count); 
                }
                prometheus_macro_get_values(AQAny,"@Texts");
                index += count;
                if ((index) >= aq_mta_get_num_of_items(AQAny,ds))
                 break;         
            }
        }
       serialize:
        prometheus_serialize(file,aq_mta_get_item(AQAny,ds,index)); 
    }
    return PrometheusBlockDone;
}

static AQStatus prometheus_internal_output_items_container(DeimosFile file, AQDataStructure ds, 
 PrometheusPrintListLambda print_list, PrometheusPrintBlockLambda print_block) {
    return print_block(file,ds,prometheus_internal_output_items_block);
}

#define prometheus_macro_get_items(type,type_name)\
 if (aq_mta_get_num_of_items(type,ds) > 0) {\
     prometheus_internal_output_container(file,NULL,type_name,ds,\
         prometheus_internal_output_items_container);\
 }

static PrometheusBlockStatus prometheus_internal_output_mta_block(DeimosFile file, AQDataStructure ds) {
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
    prometheus_macro_get_items(AQAny,"@Items");
    return PrometheusBlockDone;
}

static AQStatus prometheus_internal_output_mta_container(DeimosFile file, AQDataStructure ds, 
 PrometheusPrintListLambda print_list, PrometheusPrintBlockLambda print_block) {
    return print_block(file,ds,prometheus_internal_output_mta_block);
}

static PrometheusListStatus prometheus_internal_get_value_for_list(AQDataStructure ds, AQULong* index,
 AQMTAContainer* container) {
    AQULong i = *index; 
    *container = *((AQMTAContainer*)ds);
    i++;
    *index = i;
    return (i == 1) ? PrometheusListNotDone : PrometheusListDone;
}

static AQStatus prometheus_internal_output_value_container(DeimosFile file, AQDataStructure ds, 
 PrometheusPrintListLambda print_list, PrometheusPrintBlockLambda print_block) {
    return print_list(file,ds,prometheus_internal_get_value_for_list);
}

#define prometheus_macro_get_value(the_type,type_name)\
 if (((AQMTAContainer*)ds)->type == the_type##Flag) {\
     status = prometheus_internal_output_container(file,label,type_name,ds,\
         prometheus_internal_output_value_container);\
 }

static PrometheusListStatus prometheus_internal_get_text_for_list(AQDataStructure ds, AQULong* index,
 AQMTAContainer* container) {
    AQULong i = *index; 
    AQMTAContainer the_container;
    the_container.flag = AQMTAContainerFlag;
    the_container.type = AQAnyFlag;
    the_container.AQAnyVal = ds;
    *container = the_container;
    i++;
    *index = i;
    return (i == 1) ? PrometheusListNotDone : PrometheusListDone;
}

static AQStatus prometheus_internal_output_text_container(DeimosFile file, AQDataStructure ds, 
 PrometheusPrintListLambda print_list, PrometheusPrintBlockLambda print_block) {
    return print_list(file,ds,prometheus_internal_get_text_for_list);
}

static PrometheusBlockStatus prometheus_internal_output_store_block(DeimosFile file, AQDataStructure ds) {
    aq_store_foreach(node,ds) {
        AQStatus result = prometheus_serialize_with_label(file,
           aqstore_label_from_list_node(node),aqlist_get_item(node));
        if (!result) return PrometheusBlockDone;    
    }
    return PrometheusBlockDone;
}

static AQStatus prometheus_internal_output_store_container(DeimosFile file, AQDataStructure ds, 
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

static AQStatus prometheus_internal_validate_type(AQString type) {
    aq_string_for_each_character(index,type) {
            AQSByte offset = 0;
            if (aqstring_get_size(type) > 100) return AQFailureValue;
            AQInt character = aqstring_get_character(type,index,&offset);
            if (index == 0 && character != '#') return AQFailureValue;
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
            return AQFailureValue;
            index += offset; 
    }
    return AQSuccessValue;
}

static AQStatus prometheus_access_output_container(DeimosFile file, AQChar* label, 
 AQString type, AQDataStructure ds, PrometheusOutputContainerLambda output_container) {
   if (type != NULL) {
        if (!prometheus_internal_validate_type(type))
         return AQFailureValue;
    }   
    return 
     prometheus_internal_output_container(file,
         label,aqstring_convert_to_c_string(type),ds,output_container);
}

static AQStatus prometheus_internal_serialize(DeimosFile file, AQChar* label, PrometheusDataStructure ds) {
    AQStatus status;
    if (ds == NULL) return AQFailureValue;
    if (ds->flag == AQDestroyableFlag) {
        if (ds->serialize != NULL) status = 
         ds->serialize(file,label,ds,prometheus_access_output_container);
    }
    switch (ds->flag) {
        case AQArrayFlag:
            status = 
             prometheus_internal_output_container(file,label,
              "@Array",ds,prometheus_internal_output_array_container);
        break;
        
        case AQMultiTypeArrayFlag:
            status = 
             prometheus_internal_output_container(file,label,
              "@Values",ds,prometheus_internal_output_mta_container);
        break;
        
        case AQStringFlag:
            status = 
             prometheus_internal_output_container(file,label,
              "@Text",ds,prometheus_internal_output_text_container);
        break;
        
        case AQStoreFlag:
            status = 
             prometheus_internal_output_container(file,label,
              "@Store",ds,prometheus_internal_output_store_container);
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
         status = AQFailureValue;
        break;
    }
    return status;
}

AQStatus prometheus_serialize(DeimosFile file, PrometheusDataStructure ds) {
    return prometheus_internal_serialize(file,NULL,ds);
}

AQStatus prometheus_serialize_with_label(DeimosFile file, AQString label, PrometheusDataStructure ds) {
    AQChar* valid_label = 
     prometheus_internal_validate_label(label);
    if (valid_label == NULL) return AQFailureValue;
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
    if ((deimos_peek_utf32_character(file,&offset) != '['
     && deimos_peek_utf32_character(file,&offset) != EOF) 
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

static AQStatus prometheus_internal_register_deserializer(PrometheusDeserializer deserializer, AQChar* type,
 PrometheusGeneratorLambda generator, PrometheusAdderLambda adder) {
    AQStatus result; 
    result = aqstore_add_item(deserializer->generators,generator,type);
    if (!result) return AQFailureValue;
    result = aqstore_add_item(deserializer->adders,adder,type);
    return result;
}

/////////////////////////////////////////////////////Start AQDataStructures//////////////////////////////////////////

struct ItemsDataStructure_s {
  PROMETHEUS_SERIALIZATION_BASE_CLASS
  AQMultiTypeArray mta;
  AQAllocator allocator;
};

typedef struct ItemsDataStructure_s* ItemsDataStructure;

static AQStatus prometheus_internal_destroy_item_data_structure(ItemsDataStructure item_ds) {
   return aq_free(item_ds,item_ds->allocator);
}

static ItemsDataStructure prometheus_internal_new_item_data_structure(AQMultiTypeArray mta, AQAllocator allocator) {
    ItemsDataStructure item_ds = aq_new(struct ItemsDataStructure_s,allocator);
    item_ds->flag = AQDestroyableFlag;
    item_ds->destroyer = prometheus_internal_destroy_item_data_structure;
    item_ds->serialize = NULL;
    item_ds->mta = mta;
    item_ds->allocator = allocator;
    return item_ds;
}

#define prometheus_macro_define_gen_value(the_type)\
 static AQStatus prometheus_internal_process_value_##the_type(AQDataStructure ds, AQTypeFlag* type, AQULong* index,\
  AQMTAContainer* container) {\
     *index = -1;\
     AQMTAContainer* the_container = ds;\
     the_container->type = container->type;\
     the_container->the_type##Val =  container->the_type##Val;\
     return AQSuccessValue;\
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
     prometheus_internal_value_generator_##the_type,prometheus_internal_value_adder);\
 if (!result) return AQFailureValue;     

#define prometheus_macro_define_gen_values(the_type)\
 static AQStatus prometheus_internal_process_values_##the_type(AQDataStructure ds, AQTypeFlag* type, AQULong* index,\
  AQMTAContainer* container) {\
     *index = -1;\
     aq_mta_add_item(the_type,ds,container->the_type##Val);\
     return AQSuccessValue;\
 }\
 static AQDataStructure prometheus_internal_values_generator_##the_type(PrometheusDeserializer deserializer,\
  AQChar* adder_type, AQDataStructure super_ds, PrometheusAccessBlockGeneratorLambda block_generator,\
   PrometheusAccessValueGeneratorLambda value_generator) {\
     if (the_type##Flag != AQAnyFlag)\
      if (aqds_get_flag(super_ds) != AQMultiTypeArrayFlag) return NULL;\
     if (the_type##Flag == AQAnyFlag) {\
         if (aqds_get_flag(super_ds) != AQDestroyableFlag)\
          return NULL;\
         if (((PrometheusDataStructure)super_ds)->destroyer !=\
           ((AQDestroyerLambda)prometheus_internal_destroy_item_data_structure))\
          return NULL;\
     }\
     if (the_type##Flag == AQAnyFlag) super_ds = ((ItemsDataStructure)super_ds)->mta;\
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
     prometheus_internal_values_generator_##the_type,prometheus_internal_values_adder);\
 if (!result) return AQFailureValue; 

static AQDataStructure prometheus_internal_mta_generator(PrometheusDeserializer deserializer, 
 AQChar* adder_type, AQDataStructure super_ds, PrometheusAccessBlockGeneratorLambda block_generator,
  PrometheusAccessValueGeneratorLambda value_generator) {
    AQMultiTypeArray mta = aq_new_mta(deimos_get_allocator(deserializer->file)); 
    if (mta == NULL) return NULL;
    return block_generator(deserializer,adder_type,mta);
}

static AQDataStructure prometheus_internal_mta_adder(AQDataStructure ds,
 AQDataStructure ds_to_add, AQChar* label) {
    if (aqds_get_flag(ds_to_add) == AQMTAContainerFlag) {
        AQMTAContainer container = *((AQMTAContainer*)ds_to_add);
        AQAllocator allocator = aqmta_get_allocator(ds);
        aq_mta_define_itemvar(item);
        aq_mta_get_itemvar_from_container(container,item);
        if (type_of_item == AQByteFlag) 
         aq_mta_add_item(AQByte,ds,item_AQByte);
        if (type_of_item == AQSByteFlag) 
         aq_mta_add_item(AQSByte,ds,item_AQSByte);
        if (type_of_item == AQShortFlag) 
         aq_mta_add_item(AQShort,ds,item_AQShort);
        if (type_of_item == AQUShortFlag) 
         aq_mta_add_item(AQUShort,ds,item_AQUShort);
        if (type_of_item == AQIntFlag) 
         aq_mta_add_item(AQInt,ds,item_AQInt);
        if (type_of_item == AQUIntFlag) 
         aq_mta_add_item(AQUInt,ds,item_AQUInt);
        if (type_of_item == AQLongFlag) 
         aq_mta_add_item(AQLong,ds,item_AQLong);
        if (type_of_item == AQULongFlag) 
         aq_mta_add_item(AQULong,ds,item_AQULong);
        if (type_of_item == AQFloatFlag) 
         aq_mta_add_item(AQFloat,ds,item_AQFloat);
        if (type_of_item == AQDoubleFlag) 
         aq_mta_add_item(AQDouble,ds,item_AQDouble); 
        aq_free(ds_to_add,allocator);
        return ds;
    }
    if (aqds_get_flag(ds_to_add) != AQMultiTypeArrayFlag)
     if (aqds_get_flag(ds_to_add) != AQDestroyableFlag)
      return NULL;
    if (aqds_get_flag(ds_to_add) == AQDestroyableFlag)  
     if (((PrometheusDataStructure)ds_to_add)->destroyer 
      != ((AQDestroyerLambda)prometheus_internal_destroy_item_data_structure))
        return NULL;
    if (aqds_get_flag(ds_to_add) == AQDestroyableFlag) 
      aq_destroy(ds_to_add);
    return ds;
}

static AQDataStructure prometheus_internal_item_generator(PrometheusDeserializer deserializer, 
 AQChar* adder_type, AQDataStructure super_ds, PrometheusAccessBlockGeneratorLambda block_generator,
  PrometheusAccessValueGeneratorLambda value_generator) {
    if (aqds_get_flag(super_ds) != AQMultiTypeArrayFlag)
     return NULL;  
    ItemsDataStructure item_ds = 
     prometheus_internal_new_item_data_structure(
      (AQMultiTypeArray)super_ds,deimos_get_allocator(deserializer->file)); 
    if (item_ds == NULL) return NULL;
    return block_generator(deserializer,adder_type,item_ds);
}

static AQDataStructure prometheus_internal_item_adder(AQDataStructure ds,
 AQDataStructure ds_to_add, AQChar* label) {
    if (aqds_get_flag(ds_to_add) != AQMultiTypeArrayFlag)
     aq_mta_add_item(AQAny,((ItemsDataStructure)ds)->mta,ds_to_add);
    if (aqds_get_flag(ds_to_add) == AQMultiTypeArrayFlag)
     if (((ItemsDataStructure)ds)->mta != ((AQMultiTypeArray)ds_to_add))
      aq_mta_add_item(AQAny,((ItemsDataStructure)ds)->mta,ds_to_add);
    return ds;
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
    AQStatus result; 
    result = aqstore_add_item((AQStore)ds,ds_to_add,label);
    if (!result) return NULL;
    return ds_to_add;
}

static AQStatus prometheus_internal_register_aqdatastructures(PrometheusDeserializer deserializer) {
    AQStatus result;
    prometheus_macro_register_value(AQByte,"@Byte");
    prometheus_macro_register_value(AQSByte,"@SByte");
    prometheus_macro_register_value(AQShort,"@Short");
    prometheus_macro_register_value(AQUShort,"@UShort");
    prometheus_macro_register_value(AQInt,"@Int");
    prometheus_macro_register_value(AQUInt,"@UInt");
    prometheus_macro_register_value(AQLong,"@Long");
    prometheus_macro_register_value(AQULong,"@ULong");
    prometheus_macro_register_value(AQFloat,"@Float");
    prometheus_macro_register_value(AQDouble,"@Double");
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
    if (!result) return AQFailureValue;
    result = prometheus_internal_register_deserializer(deserializer,"@Items",
     prometheus_internal_item_generator,prometheus_internal_item_adder);
    if (!result) return AQFailureValue;
    result = prometheus_internal_register_deserializer(deserializer,"@Array",
     prometheus_internal_array_generator,prometheus_internal_array_adder);
    if (!result) return AQFailureValue;   
    result = prometheus_internal_register_deserializer(deserializer,"@Store",
     prometheus_internal_store_generator,prometheus_internal_store_adder);
    if (!result) return AQFailureValue;   
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
    aq_free(deserializer,deimos_get_allocator(deserializer->file));
}

AQStatus prometheus_register_deserializer(PrometheusDeserializer deserializer, AQString name,
 PrometheusGeneratorLambda generator, PrometheusAdderLambda adder) {
    if (!prometheus_internal_validate_type(name)) return AQFailureValue;
    AQChar* valid_type = aqstring_get_c_string(name);
    return prometheus_internal_register_deserializer(deserializer,valid_type,generator,adder);
}

AQDataStructure prometheus_deserialize(PrometheusDeserializer deserializer) {
    return prometheus_internal_deserialize(deserializer);
}