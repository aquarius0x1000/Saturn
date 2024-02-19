#include "prometheus.h"

static AQInt pro_output_container(DeimosFile file,
     AQDataStructure data, AQChar* label, AQTypeFlag flag_type_for_values);

static void pro_output_list(DeimosFile file,
     AQDataStructureFlag flag, AQDataStructure data,
      AQTypeFlag flag_type_for_values) {
    deimos_output_character(file,'(');
    if (flag == AQMTAContainerFlag) {
        AQMTAContainer* container = ((AQMTAContainer*)data);
        switch(container->type) {
            case AQByteFlag:
              deimos_output_byte(file,container->AQByteVal);
            break;
            case AQSByteFlag:
              deimos_output_sbyte(file,container->AQSByteVal);
            break;
            case AQShortFlag:
              deimos_output_short(file,container->AQShortVal);
            break;
            case AQUShortFlag:
              deimos_output_ushort(file,container->AQUShortVal);
            break;
            case AQIntFlag:
              deimos_output_integer(file,container->AQIntVal);
            break;
            case AQUIntFlag:
              deimos_output_uinteger(file,container->AQUIntVal);
            break;
            case AQLongFlag:
              deimos_output_long(file,container->AQLongVal);
            break;
            case AQULongFlag:
              deimos_output_ulong(file,container->AQULongVal);
            break;
            case AQFloatFlag:
              deimos_output_float(file,container->AQFloatVal);
            break;
            case AQDoubleFlag:
              deimos_output_double(file,container->AQDoubleVal);
            break;
            default:
            break;
        }
    }
    if (flag == AQMultiTypeArrayFlag) {
        #define for_each_type(type,output_type)\
        if (flag_type_for_values == type##Flag)\
        aq_mta_foreach(type,i,(AQMultiTypeArray)data) {\
            deimos_output_##output_type(file,aq_mta_get_item(type,(AQMultiTypeArray)data,i));\
            if (i+1 < aq_mta_get_num_of_items(type,(AQMultiTypeArray)data))\
             deimos_output_character(file,',');\
        }
        for_each_type(AQByte,byte)
        for_each_type(AQSByte,sbyte)
        for_each_type(AQShort,short)
        for_each_type(AQUShort,ushort)
        for_each_type(AQInt,integer)
        for_each_type(AQUInt,uinteger)
        for_each_type(AQLong,long)
        for_each_type(AQULong,ulong)
        for_each_type(AQFloat,float)
        for_each_type(AQDouble,double)
    }
    deimos_output_character(file,')');
    deimos_output_character(file,';');
    deimos_output_character(file,'\n');     
}

static void pro_output_block(DeimosFile file,
     AQDataStructureFlag flag, AQDataStructure data,
      AQTypeFlag flag_type_for_values) {
    if (flag == AQMTAContainerFlag) {
           pro_output_list(file,flag,data,AQEmptyFlag);
           return;
    }
    if (flag == AQMultiTypeArrayFlag &&
       flag_type_for_values != AQEmptyFlag) {
          pro_output_list(file,flag,data,flag_type_for_values);
          return;
    }
    deimos_output_character(file,'{');
    deimos_output_character(file,'\n');
    deimos_output_add_to_tab(file,1);
    if (flag == AQArrayFlag) {
        aq_array_foreach(index,(AQArray)data) {
            pro_output_container(file,
             aqarray_get_item((AQArray)data,index),
              NULL,AQEmptyFlag);
        }
    }
    if (flag == AQStoreFlag) {
        aq_store_foreach(node,(AQStore)data) {
            pro_output_container(file,
            aqlist_get_item(node),
            aqstring_get_c_string(
              aqstore_label_from_list_node(node)
             ),AQEmptyFlag
            );
           }
    }
    if (flag == AQStringFlag) {
        deimos_output_tab(file);
        deimos_output_character(file,'"');
        deimos_output_string(file,aqstring_get_c_string((AQString)data));
        deimos_output_character(file,'"');
        deimos_output_character(file,'\n');
    }
     if (flag == AQMultiTypeArrayFlag &&
        flag_type_for_values == AQEmptyFlag) {
        #define output_values_for_type(type)\ 
         if (aq_mta_get_num_of_items(type,(AQMultiTypeArray)data) > 0)\
          pro_output_container(file,data,NULL,type##Flag);
        output_values_for_type(AQByte)
        output_values_for_type(AQSByte)
        output_values_for_type(AQShort)
        output_values_for_type(AQUShort)      
        output_values_for_type(AQInt)
        output_values_for_type(AQUInt)
        output_values_for_type(AQLong)
        output_values_for_type(AQULong)
        output_values_for_type(AQFloat)
        output_values_for_type(AQDouble)                                     
    }
    deimos_output_sub_from_tab(file,1);
    deimos_output_tab(file);
    deimos_output_character(file,'}');
    deimos_output_character(file,';'); 
    deimos_output_character(file,'\n');     
}

static AQInt pro_output_container(DeimosFile file,
     AQDataStructure data, AQChar* label, 
     AQTypeFlag flag_type_for_values) {
    if (data == NULL) return 0;
    AQDataStructureFlag flag = aqds_get_flag(data);
    deimos_output_tab(file);  
    deimos_output_character(file,'[');
    if (label != NULL) {
        deimos_output_string(file,label);
        deimos_output_character(file,',');
    }
    if (flag_type_for_values == AQEmptyFlag) {
      if (flag == AQMTAContainerFlag) {
          switch (((AQMTAContainer*)data)->type) {
              case AQByteFlag:
                deimos_output_string(file,"@Byte");
              break;
              case AQSByteFlag:
                deimos_output_string(file,"@SByte");
              break;
              case AQShortFlag:
                deimos_output_string(file,"@Short");
              break;
              case AQUShortFlag:
                deimos_output_string(file,"@UShort");
              break;
              case AQIntFlag:
                deimos_output_string(file,"@Int");
              break;
              case AQUIntFlag:
                deimos_output_string(file,"@UInt");
              break;
              case AQLongFlag:
                deimos_output_string(file,"@Long");
              break;
              case AQULongFlag:
                deimos_output_string(file,"@ULong");
              break;
              case AQFloatFlag:
                deimos_output_string(file,"@Float");
              break;
              case AQDoubleFlag:
                deimos_output_string(file,"@Double");
              break;
              default:
              break;
        }
    }
    if (flag == AQMultiTypeArrayFlag)
       deimos_output_string(file,"@Values");
    if (flag == AQArrayFlag)
       deimos_output_string(file,"@Array");
    if (flag == AQStoreFlag)
       deimos_output_string(file,"@Store");
    if (flag == AQStringFlag)
       deimos_output_string(file,"@Text");     
    } else {
        switch (flag_type_for_values) {
              case AQByteFlag:
                deimos_output_string(file,"@Bytes");
              break;
              case AQSByteFlag:
                deimos_output_string(file,"@SBytes");
              break;
              case AQShortFlag:
                deimos_output_string(file,"@Shorts");
              break;
              case AQUShortFlag:
                deimos_output_string(file,"@UShorts");
              break;
              case AQIntFlag:
                deimos_output_string(file,"@Ints");
              break;
              case AQUIntFlag:
                deimos_output_string(file,"@UInts");
              break;
              case AQLongFlag:
                deimos_output_string(file,"@Longs");
              break;
              case AQULongFlag:
                deimos_output_string(file,"@ULongs");
              break;
              case AQFloatFlag:
                deimos_output_string(file,"@Floats");
              break;
              case AQDoubleFlag:
                deimos_output_string(file,"@Doubles");
              break;
              default:
              break;
          }
    }     
    deimos_output_character(file,']'); 
    deimos_output_character(file,' ');
    deimos_output_character(file,':');
    deimos_output_character(file,' ');
    pro_output_block(file,flag,data,flag_type_for_values);
    return 0;  
}

AQInt prometheus_output_file(DeimosFile file, AQDataStructure data) {
    return pro_output_container(file,data,NULL,AQEmptyFlag);
}

static AQInt pro_validate_name(AQString name) {
  aq_string_for_each_character(index,name) {
    AQULong offset = 0;
    AQInt character = 
     aqstring_get_character(name,index,&offset);
     if (!isdigit(character) &&
        !isalpha(character)) return 0;
    index += offset;
  }
  return 1;
}

static AQDataStructure prometheus_get_container(DeimosFile file, AQDataStructure sds);

typedef AQDataStructure (*pro_get_block_type)(DeimosFile file,
   AQTypeFlag flag_type_for_ds, AQDataStructure sds);

#define pro_define_get_value(main_type,aq_type)\       
 static AQDataStructure pro_get_value_##main_type(DeimosFile file,\
    AQTypeFlag flag_type_for_ds, AQDataStructure sds) {\
     AQULong offset;\
     if (deimos_peek_utf32_character(file,&offset) == ':') {\
         deimos_advance_file_position(file,offset);\
         if (deimos_peek_utf32_character(file,&offset) == '(') {\
             deimos_advance_file_position(file,offset);\
             aq_type value = deimos_get_##main_type(file);\
             if (deimos_peek_utf32_character(file,&offset) == ';') {\
                 deimos_advance_file_position(file,offset);\
                 AQMTAContainer* mta_container = aq_new(AQMTAContainer);\
                 mta_container->flag = AQMTAContainerFlag;\
                 mta_container->type = aq_type##Flag;\
                 mta_container->AQIntVal = value;\
                 return (AQDataStructure)mta_container;\
             }\
         }\  
     }\
     return NULL;\
  }
  
pro_define_get_value(byte,AQByte);
pro_define_get_value(sbyte,AQSByte);
pro_define_get_value(short,AQShort);
pro_define_get_value(ushort,AQUShort);
pro_define_get_value(integer,AQInt);
pro_define_get_value(uinteger,AQUInt);
pro_define_get_value(long,AQLong);
pro_define_get_value(ulong,AQULong);
pro_define_get_value(float,AQFloat);
pro_define_get_value(double,AQDouble);

#define pro_define_get_values(main_type,aq_type)\
 static AQDataStructure pro_get_values_##main_type(DeimosFile file,\
    AQTypeFlag flag_type_for_ds, AQDataStructure sds) {\
     AQULong offset;\
     AQInt character;\
     if (deimos_peek_utf32_character(file,&offset) == ':') {\
         deimos_advance_file_position(file,offset);\
         if (deimos_peek_utf32_character(file,&offset) == '(') {\
             deimos_advance_file_position(file,offset);\
           loop:\
             aq_type value = deimos_get_##main_type(file);\
             character = deimos_peek_last_utf32_character(file,0);\
             if (character == ',' || character == ')') {\
                 aq_mta_add_item(aq_type,sds,value);\
                 if (character == ',') goto loop;\
                 if (deimos_peek_utf32_character(file,&offset) == ';') {\
                    deimos_advance_file_position(file,offset);\ 
                    return sds;\ 
                 }\
             }\
         }\
     }\
     return NULL;\
 }\
 
 
pro_define_get_values(byte,AQByte);
pro_define_get_values(sbyte,AQSByte);
pro_define_get_values(short,AQShort);
pro_define_get_values(ushort,AQUShort);
pro_define_get_values(integer,AQInt);
pro_define_get_values(uinteger,AQUInt);
pro_define_get_values(long,AQLong);
pro_define_get_values(ulong,AQULong);
pro_define_get_values(float,AQFloat);
pro_define_get_values(double,AQDouble);

static AQDataStructure pro_get_string(DeimosFile file,
    AQTypeFlag flag_type_for_ds, AQDataStructure sds) {
    AQULong offset;  
    if (deimos_peek_utf32_character(file,&offset) == ':') {
         deimos_advance_file_position(file,offset);
         if (deimos_peek_utf32_character(file,&offset) == '{') {
             AQString string = deimos_get_string(file,'"','"');
             if (deimos_peek_utf32_character(file,&offset) == '}') {
                 deimos_advance_file_position(file,offset);
                 if (deimos_peek_utf32_character(file,&offset) == ';') { 
                    deimos_advance_file_position(file,offset);
                    return (AQDataStructure)string;
             }   }
         }
    }
}

static AQDataStructure prometheus_get_container(DeimosFile file, AQDataStructure sds) {
  //init dispactch
  //parse [name,type], use get_string, peek, and sds(super data structure)
  //parse block, via dispactch
  return NULL;
}

AQDataStructure prometheus_load_file(DeimosFile file) {
    //return pro_get_container(file,NULL);
}