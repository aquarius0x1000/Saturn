#include "../src/saturn.h"

void saturn_test_5(void) {
    DeimosFile file = deimos_open_file("test.bin",DeimosWriteModeFlag);
    
    AQMultiTypeArray mta = aqmta_new();
    
    aq_mta_add_item(AQByte,mta,234);
    aq_mta_add_item(AQSByte,mta,-1);
    aq_mta_add_item(AQInt,mta,455);
    aq_mta_add_item(AQInt,mta,234);
    aq_mta_add_item(AQFloat,mta,1.4526f);
    
    AQString string = aqstr("Hello World!!!");
    
    AQList list = aqlist_new();
    
    aqlist_add_item(list,mta);
    aqlist_add_item(list,string);
    
    AQString string2 = aqstr("Hello World.");
    
    AQArray array = aqarray_new();
    
    aqarray_add_item(array,list);
    aqarray_add_item(array,string2);
    
    deimos_output_binary_aqds(file,array);
    
    aq_destroy(mta);
    aq_destroy(string);
    aq_destroy(list);
    aq_destroy(string2);
    aq_destroy(array);
    
    deimos_close_file(file);
    
    file = deimos_open_file("test.bin",DeimosReadModeFlag);
    
    AQDataStructure aqds = deimos_get_binary_aqds(file);
    
    if (aqds_get_flag(aqds) != AQArrayFlag) puts("aqds is not AQArray!");
    if (aqds_get_flag(aqds) != AQArrayFlag) exit(1);
    
    AQArray array_from_bin = (AQArray)aqds;
    
    aqds = aqarray_get_item(array_from_bin,1);
    
    if (aqds_get_flag(aqds) != AQStringFlag) puts("aqds is not AQString!");
    if (aqds_get_flag(aqds) != AQStringFlag) exit(1);
    
    AQString string2_from_bin = (AQString)aqds;
    
    printf("string2 is: %s\n",aqstring_get_c_string(string2_from_bin));
        
    aqds = aqarray_get_item(array_from_bin,0);
    
    if (aqds_get_flag(aqds) != AQListFlag) puts("aqds is not AQList!");
    if (aqds_get_flag(aqds) != AQListFlag) exit(1);
    
    AQList list_from_bin = (AQList)aqds;
    
    aqds = aqlist_get_node(list_from_bin,1);
    aqds = aqlist_get_item(aqds);
    
    if (aqds_get_flag(aqds) != AQStringFlag) puts("aqds is not AQString!!");
    if (aqds_get_flag(aqds) != AQStringFlag) exit(1);
    
    AQString string_from_bin = (AQString)aqds;
    
    printf("string is: %s\n",aqstring_get_c_string(string_from_bin));
        
    aqds = aqlist_get_node(list_from_bin,0);
    aqds = aqlist_get_item(aqds);
    
    if (aqds_get_flag(aqds) != AQMultiTypeArrayFlag) puts("aqds is not AQMultiTypeArray!");
    if (aqds_get_flag(aqds) != AQMultiTypeArrayFlag) exit(1);
    
    AQMultiTypeArray mta_from_bin = (AQMultiTypeArray)aqds;
    
    printf("MTA: %d\n",aq_mta_get_item(AQByte,mta_from_bin,0));
    printf("MTA: %d\n",aq_mta_get_item(AQSByte,mta_from_bin,0));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta_from_bin,0));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta_from_bin,1));
    printf("MTA: %f\n",aq_mta_get_item(AQFloat,mta_from_bin,0));
    
    aq_destroy(mta_from_bin);
    aq_destroy(string_from_bin);
    aq_destroy(list_from_bin);
    aq_destroy(string2_from_bin);
    aq_destroy(array_from_bin);
    
    deimos_close_file(file);
}
