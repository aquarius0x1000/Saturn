#include "../src/aquarius.h"
#include "../src/prometheus.h"

static AQAny test_malloc(AQAny allocation_data, AQULong size_in_bytes) {
    return malloc(size_in_bytes);
}

static void test_free(AQAny allocation_data, AQAny data_to_be_freed) {
    return free(data_to_be_freed);
}

typedef struct {
  PROMETHEUS_SERIALIZATION_BASE_CLASS
  AQInt count;
} TestDataStructure;

static AQStatus destroy_TestDataStructure(AQDataStructure ds) {
   free(ds);
   return AQSuccessValue;
}

static PrometheusListStatus get_count_for_TestDataStructure(AQDataStructure ds, AQULong* index,
 AQMTAContainer* container) {
    AQULong i = *index;
    TestDataStructure* test = ds;
    if (i >= test->count) return PrometheusListDone;
    AQMTAContainer the_container;
    the_container.flag = AQMTAContainerFlag;
    the_container.type = AQIntFlag;
    the_container.AQIntVal = i+1;
    *container = the_container;
    i++;
    *index = i;
    return PrometheusListNotDone;
}

static AQStatus output_container_TestDataStructure(DeimosFile file, AQDataStructure ds, 
 PrometheusPrintListLambda print_list, PrometheusPrintBlockLambda print_block) {
    return print_list(file,ds,get_count_for_TestDataStructure);
}

static AQStatus serialize_TestDataStructure(DeimosFile file, AQChar* label, AQDataStructure ds,
 PrometheusAccessOutputContainerLambda output_container) {
    return output_container(file,label,aqstr("#TestDataStructure"),
        ds,output_container_TestDataStructure);
}

static TestDataStructure* new_TestDataStructure(AQInt count) {
    TestDataStructure* ds = aq_new(TestDataStructure);
    ds->flag = AQDestroyableFlag;
    ds->destroyer = destroy_TestDataStructure;
    ds->serialize = serialize_TestDataStructure;
    ds->count = count;
    return ds;
}

static AQStatus process_TestDataStructure(AQDataStructure ds, AQTypeFlag* type, AQULong* index,
 AQMTAContainer* container) {
    *index = -1;
    ((TestDataStructure*)ds)->count = container->AQIntVal;
    return AQSuccessValue;
}

static AQDataStructure generator_TestDataStructure(PrometheusDeserializer deserializer, 
  AQChar* adder_type, AQDataStructure super_ds, PrometheusAccessBlockGeneratorLambda block_generator,
   PrometheusAccessValueGeneratorLambda value_generator) {
     AQDataStructure ds_to_add = new_TestDataStructure(0);
      ds_to_add = value_generator(deserializer,ds_to_add,
       process_TestDataStructure,(AQTypeFlag[]){AQIntFlag},1);
     if (ds_to_add == NULL) return NULL;
     return ds_to_add;
}

static AQDataStructure adder_TestDataStructure(AQDataStructure ds,
 AQDataStructure ds_to_add, AQChar* label) {
    return ds_to_add;
}

static void test_mta(AQAny data) {
    AQMTAContainer container = *((AQMTAContainer*)data);
    aq_mta_define_itemvar(iterator_item);
    aq_mta_get_itemvar_from_container(container,iterator_item);
    if (iterator_item_AQInt == 42) puts("YES!!!!!42");
    if (iterator_item_AQInt == 958754) puts("YES!!!!!958754");
    if (iterator_item_AQByte == 255) puts("YES!!!!!255");
}

void saturn_test_4(void) {
    AQAllocatorStruct allocator = {
      .allocator_function = test_malloc,
      .free_function = test_free,
      .data = NULL
    };
    
    DeimosFile file = deimos_open_file("TEST.pro",DeimosWriteModeFlag);
    DeimosFile file2 = deimos_get_file_from_string(aqstr("",&allocator),DeimosWriteModeFlag);
    
    AQStore store99 = aqstore_new();
    AQArray array99 = aqarray_new();
    AQMultiTypeArray mta99 = aqmta_new();
    AQMultiTypeArray mta100 = aqmta_new();
    AQString string99 = aqstr("Hello World!!!!",&allocator);
    aq_mta_add_item(AQInt,mta99,42);
    aq_mta_add_item(AQInt,mta99,958754);
    aq_mta_add_item(AQInt,mta99,255);
    aq_mta_add_item(AQFloat,mta99,255.0255f);
    aq_mta_add_item(AQDouble,mta99,255.0255);
    
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta99,0));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta99,1));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta99,2));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta99,0));
    
    aqmta_iterate_all_types_with(test_mta,mta99);
    
    aq_mta_add_item(AQByte,mta100,255);
    aq_mta_add_item(AQInt,mta100,420);
    aq_mta_add_item(AQInt,mta100,9587540);
    aq_mta_add_item(AQInt,mta100,2550);
    
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta100,0));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta100,1));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta100,2));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta100,0));
    
    
    aqarray_add_item(array99,mta99);
    aqarray_add_item(array99,mta100);
    aqarray_add_item(array99,string99);
    
    aqstore_add_item(store99,array99,"TESTDATA");
    
    prometheus_serialize(file,(AQDataStructure)store99);
    prometheus_serialize(file2,(AQDataStructure)store99);
    
    aqstring_destroy(string99);
    aqmta_destroy(mta99);
    aqmta_destroy(mta100);
    aqarray_destroy(array99);
    aqstore_destroy(store99);
    
    deimos_close_file(file);
    
    AQString file_string = 
     aqstring_copy(deimos_get_file_string(file2));
    
    AQString file_string_2 = aqstring_copy(file_string);
    
    deimos_close_file(file2);
    
    printf("PRINT: %s",aqstring_get_c_string(file_string));
   
    file = deimos_open_file("TEST.pro",DeimosReadModeFlag,&allocator);
    PrometheusDeserializer the_deserializer = prometheus_deserializer_new(file);
    AQStore test_data = (AQStore)prometheus_deserialize(the_deserializer);
    if (test_data == NULL) puts("NO!");
    AQArray test_array = aqstore_get_item(test_data,"TESTDATA");
    AQMultiTypeArray test_mta_data = aqarray_get_item(test_array,0);
    aqmta_iterate_all_types_with(test_mta,test_mta_data);
    AQString test_string = aqarray_get_item(test_array,2);
    printf("test_string is %s\n",aqstring_get_c_string(test_string));
    aq_destroy(test_array);
    aq_destroy(test_data);
    aq_destroy(test_string);
    aq_destroy(test_mta_data);
    aq_destroy(the_deserializer);
    aq_destroy(file);
    
    file2 = deimos_get_file_from_string(file_string,DeimosReadModeFlag);
    the_deserializer = prometheus_deserializer_new(file2);
    test_data = (AQStore)prometheus_deserialize(the_deserializer);
    if (test_data == NULL) puts("NO!!");
    test_array = aqstore_get_item(test_data,"TESTDATA");
    test_mta_data = aqarray_get_item(test_array,0);
    aqmta_iterate_all_types_with(test_mta,test_mta_data);
    test_string = aqarray_get_item(test_array,2);
    printf("test_string 2 is %s\n",aqstring_get_c_string(test_string));
    aq_destroy(test_array);
    aq_destroy(test_data);
    aq_destroy(test_string);
    aq_destroy(test_mta_data);
    aq_destroy(the_deserializer);
    aq_destroy(file2);
    
    DeimosFile file3 = deimos_get_file_from_string(file_string_2,DeimosReadModeFlag);
    DeimosFile file4 = deimos_get_file_from_string(aqstr(""),DeimosWriteModeFlag);
    deimos_get_base_32_star_encode(file3,file4);
    
    
    AQString b32s_string = 
     aqstring_copy(deimos_get_file_string(file4));
    
    printf("PRINT: %s\n",aqstring_get_c_string(b32s_string));
    
    aq_destroy(file3);
    aq_destroy(file4);
    
    file3 = deimos_get_file_from_string(b32s_string,DeimosReadModeFlag);
    file4 = deimos_get_file_from_string(aqstr(""),DeimosWriteModeFlag);
    deimos_get_base_32_star_decode(file3,file4);
    
    b32s_string = 
     aqstring_copy(deimos_get_file_string(file4));
     
    printf("PRINT: %s",aqstring_get_c_string(b32s_string));
     
    aq_destroy(file3);
    aq_destroy(file4); 
    
    aq_print(c_string,"Hello Wordl!\n");
    aq_print(c_string,"And the number is: ");
    aq_print(ulong,38475893L);
    aq_print(c_string,".\n");
    
    aq_print(c_string,"TEST2.pro\n");
    
    file = deimos_open_file("TEST2.pro",DeimosWriteModeFlag,&allocator);
    AQArray array999 = aqarray_new();
    AQArray array1000 = aqarray_new();
    AQArray array1001 = aqarray_new();
    AQArray array1002 = aqarray_new();
    
    aqarray_add_item(array999,array1000);
    aqarray_add_item(array999,array1001);
    aqarray_add_item(array999,array1002);
    
    AQArray array2999 = aqarray_new();
    AQMultiTypeArray mta2999 = aqmta_new();
    AQMultiTypeArray mta2000 = aqmta_new();
    AQString string2999 = aqstr("Hello World!!!!");
    aq_mta_add_item(AQInt,mta2999,42);
    aq_mta_add_item(AQInt,mta2999,958754);
    aq_mta_add_item(AQInt,mta2999,255);
    aq_mta_add_item(AQFloat,mta2999,255.0255f);
    aq_mta_add_item(AQDouble,mta2999,255.0255);
    
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta2999,0));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta2999,1));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta2999,2));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta2999,0));
    
    aqmta_iterate_all_types_with(test_mta,mta2999);
    
    aq_mta_add_item(AQByte,mta2000,255);
    aq_mta_add_item(AQByte,mta2000,155);
    aq_mta_add_item(AQInt,mta2000,420);
    aq_mta_add_item(AQInt,mta2000,9587540);
    aq_mta_add_item(AQInt,mta2000,2550);
    aq_mta_add_item(AQAny,mta2000,string2999);
    aq_mta_add_item(AQAny,mta2000,string2999);
    aq_mta_add_item(AQAny,mta2000,mta2999);
    aq_mta_add_item(AQAny,mta2000,string2999);
    aq_mta_add_item(AQAny,mta2000,mta2999);
    aq_mta_add_item(AQAny,mta2000,string2999);
    aq_mta_add_item(AQAny,mta2000,string2999);
    
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta2000,0));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta2000,1));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta2000,2));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta2000,0));
    
    TestDataStructure* ds500 = new_TestDataStructure(5);
    
    AQMTAContainer* container = aq_new(AQMTAContainer);
    container->flag = AQMTAContainerFlag;
    container->type = AQIntFlag;
    container->AQIntVal = 999;
   
    AQList list = aqlist_new();
    aqlist_add_item(list,mta2000);
    aqlist_add_item(list,string2999);
    aqlist_add_item(list,ds500);
    aqlist_add_item(list,ds500);
    aqlist_add_item(list,container);
    
    aqarray_add_item(array2999,mta2999);
    aqarray_add_item(array2999,mta2000);
    aqarray_add_item(array2999,string2999);
    aqarray_add_item(array2999,ds500);
    aqarray_add_item(array2999,container);
    aqarray_add_item(array2999,list);
    
    
    aqarray_add_item(array999,array2999);
     
    AQStore store500 = aqstore_new();
    
    aqstore_add_item(store500,array999,"THE ARRAY");
    
    prometheus_serialize(file,(PrometheusDataStructure)array999);
   
    prometheus_serialize(file,(PrometheusDataStructure)store500);
    
    aq_free(container);
    aq_destroy(list);
    aq_destroy(ds500);
    aq_destroy(store500);
    aq_destroy(array999);
    aq_destroy(array1000);
    aq_destroy(array1001);
    aq_destroy(array1002);
    aq_destroy(file);
    
    file = deimos_open_file("TEST2.pro",DeimosReadModeFlag);
    PrometheusDeserializer deserializer = prometheus_deserializer_new(file);
    prometheus_register_deserializer(deserializer,aqstr("#TestDataStructure"),
     generator_TestDataStructure,adder_TestDataStructure);
    AQDataStructure aq_data_structure = prometheus_deserialize(deserializer);
    if (aq_data_structure == NULL) puts("aq_data_structure is null!");
    if (aqds_get_flag(aq_data_structure) == AQArrayFlag) puts("aq_data_structure is array!");
    if (aqds_get_flag(aq_data_structure) == AQStoreFlag) puts("aq_data_structure is store!");
    
    aq_array_foreach(index,aq_data_structure) {
        AQDataStructure ds = aqarray_get_item(aq_data_structure,index);
        if (ds == NULL) printf("ds is null, and index is: %d\n",index);
        if (ds != NULL) {
            if (aqds_get_flag(ds) == AQArrayFlag) puts("aq_data_structure is array!2");
            if (aqds_get_flag(ds) == AQStoreFlag) puts("aq_data_structure is store!2");
            
            if (aqds_get_flag(ds) == AQArrayFlag) {
                aq_array_foreach(index2,ds) {
                    AQDataStructure ds2 = aqarray_get_item(ds,index2);
                    if (aqds_get_flag(ds2) == AQArrayFlag) puts("ds2 is array!2");
                    if (aqds_get_flag(ds2) == AQStoreFlag) puts("ds2 is store!2");
                }
            }
            
            if (aqds_get_flag(ds) == AQStoreFlag) {
                aq_store_foreach(node,ds) {
                    AQDataStructure ds2 = aqlist_get_item(node);
                    if (aqds_get_flag(ds2) == AQArrayFlag) puts("ds2 is array!2");
                    if (aqds_get_flag(ds2) == AQStoreFlag) puts("ds2 is store!2");
                    if (aqds_get_flag(ds2) == AQArrayFlag) {
                        aq_array_foreach(index3,ds2) {
                            AQDataStructure ds3 = aqarray_get_item(ds2,index3);
                            if (aqds_get_flag(ds3) == AQArrayFlag) puts("ds3 is array!3");
                            if (aqds_get_flag(ds3) == AQStoreFlag) puts("ds3 is store!3");
                        }
                    }
                }
            }
         }
    }
    
    DeimosFile file999 = 
     deimos_get_file_from_string(aqstr("",&allocator),DeimosWriteModeFlag);
    prometheus_serialize(file999,(PrometheusDataStructure)aqarray_get_item(aq_data_structure,0));
    prometheus_serialize(file999,(PrometheusDataStructure)aqarray_get_item(aq_data_structure,1));
    AQString file_string_999 = 
     aqstring_copy(deimos_get_file_string(file999));
    
    printf("PRINT FILE STRING: %s",aqstring_get_c_string(file_string_999));
    
    aq_destroy(deserializer);
    aq_destroy(file_string_999);
    aq_destroy(file999);
    aq_destroy(file);
}
