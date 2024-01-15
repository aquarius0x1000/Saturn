#include "prometheus.h"

static AQInt pro_output_container(DeimosFile file,
     AQDataStructure data, AQChar* label);

static void pro_output_list(DeimosFile file,
     AQDataStructureFlag flag, AQDataStructure data) {
    deimos_output_character(file,'(');
    if (flag == AQMTAContainerFlag) {
        AQMTAContainer* container = ((AQMTAContainer*)data);
        switch(container->type) {
            case AQIntFlag:
              deimos_output_integer(file,container->AQIntVal);
            break;
        }
    }
    if (flag == AQMultiTypeArrayFlag) {
        aq_mta_foreach(AQInt,i,(AQMultiTypeArray)data) {
            deimos_output_integer(file,aq_mta_get_item(AQInt,(AQMultiTypeArray)data,i));
            if (i+1 < aqmta_get_num_of_items_all_types((AQMultiTypeArray)data))
             deimos_output_character(file,',');
        } 
    }
    deimos_output_character(file,')');
    deimos_output_character(file,';');
    deimos_output_character(file,'\n');     
}

static void pro_output_block(DeimosFile file,
     AQDataStructureFlag flag, AQDataStructure data) {
    if (flag == AQMTAContainerFlag ||
       flag == AQMultiTypeArrayFlag) {
           pro_output_list(file,flag,data);
           return;
    }    
    deimos_output_character(file,'{');
    deimos_output_character(file,'\n');
    deimos_output_add_to_tab(file,1);
    if (flag == AQArrayFlag) {
        aq_array_foreach(index,(AQArray)data) {
            pro_output_container(file,
            aqarray_get_item((AQArray)data,index),
            NULL);
        }
    }
    if (flag == AQStoreFlag) {
        aq_store_foreach(node,(AQStore)data) {
            pro_output_container(file,
            aqlist_get_item(node),
            aqstring_get_c_string(
              aqstore_label_from_list_node(node)
             )
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
    deimos_output_sub_from_tab(file,1);
    deimos_output_tab(file);
    deimos_output_character(file,'}');
    deimos_output_character(file,';'); 
    deimos_output_character(file,'\n');     
}

static AQInt pro_output_container(DeimosFile file,
     AQDataStructure data, AQChar* label) {
    if (data == NULL) return 0;
    AQDataStructureFlag flag = aqdatastructure_get_flag(data);
    deimos_output_tab(file);  
    deimos_output_character(file,'[');
    if (label != NULL) {
        deimos_output_string(file,label);
        deimos_output_character(file,',');
    }
    if (flag == AQMTAContainerFlag)
       deimos_output_string(file,"@Int");
    if (flag == AQMultiTypeArrayFlag)
       deimos_output_string(file,"@Ints");
    if (flag == AQArrayFlag)
       deimos_output_string(file,"@Array");
    if (flag == AQStoreFlag)
       deimos_output_string(file,"@Store");
    if (flag == AQStringFlag)
       deimos_output_string(file,"@Text");       
    deimos_output_character(file,']'); 
    deimos_output_character(file,' ');
    deimos_output_character(file,':');
    deimos_output_character(file,' ');
    pro_output_block(file,flag,data);
    return 0;  
}

AQInt prometheus_output_file(DeimosFile file, AQDataStructure data) {
    return pro_output_container(file,data,NULL);
}