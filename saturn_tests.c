#include "src/aquarius.h"
#include "src/deimos.h"
#include "src/prometheus.h"
  
AQAny test_malloc(AQAny allocation_data, AQULong size_in_bytes) {
    puts("Hello from the world!!!!!!!!!");
    return malloc(size_in_bytes);
}

void test_free(AQAny allocation_data, AQAny data_to_be_freed) {
    puts("FREE!!!!!");
    return free(data_to_be_freed);
}

void test_mta(AQAny data) {
    AQMTAContainer container = *((AQMTAContainer*)data);
    aq_mta_define_itemvar(iterator_item);
    aq_mta_get_itemvar_from_container(container,iterator_item);
    if (iterator_item_AQInt == 42) puts("YES!!!!!42");
    if (iterator_item_AQInt == 958754) puts("YES!!!!!958754");
    if (iterator_item_AQByte == 255) puts("YES!!!!!255");
}

typedef struct {
  PROMETHEUS_SERIALIZATION_BASE_CLASS
  AQInt count;
} TestDataStructure;

void destroy_TestDataStructure(AQDataStructure ds) {
   free(ds);
}

AQInt get_count_for_TestDataStructure(AQDataStructure ds, AQULong* index,
 AQMTAContainer* container) {
    AQULong i = *index;
    TestDataStructure* test = ds;
    if (i >= test->count) return PROMETHEUS_LIST_DONE;
    AQMTAContainer the_container;
    the_container.flag = AQMTAContainerFlag;
    the_container.type = AQIntFlag;
    the_container.AQIntVal = i+1;
    *container = the_container;
    i++;
    *index = i;
    return PROMETHEUS_LIST_NOT_DONE;
}

AQInt output_container_TestDataStructure(DeimosFile file, AQDataStructure ds, 
 PrometheusPrintListLambda print_list, PrometheusPrintBlockLambda print_block) {
    return print_list(file,ds,get_count_for_TestDataStructure);
}

AQInt serialize_TestDataStructure(DeimosFile file, AQChar* label, AQDataStructure ds,
 PrometheusAccessOutputContainerLambda output_container) {
    output_container(file,label,aqstr("#TestDataStructure"),
        ds,output_container_TestDataStructure);
    return 1;
}

TestDataStructure* new_TestDataStructure(AQInt count) {
    TestDataStructure* ds = aq_new(TestDataStructure);
    ds->flag = AQDestroyableFlag;
    ds->destroyer = destroy_TestDataStructure;
    ds->serialize = serialize_TestDataStructure;
    ds->count = count;
    return ds;
}

AQInt process_TestDataStructure(AQDataStructure ds, AQTypeFlag* type, AQULong* index,
 AQMTAContainer* container) {
    *index = -1;
    ((TestDataStructure*)ds)->count = container->AQIntVal;
    return 1;
}

AQDataStructure generator_TestDataStructure(PrometheusDeserializer deserializer, 
  AQChar* adder_type, AQDataStructure super_ds, PrometheusAccessBlockGeneratorLambda block_generator,
   PrometheusAccessValueGeneratorLambda value_generator) {
     AQDataStructure ds_to_add = new_TestDataStructure(0);
      ds_to_add = value_generator(deserializer,ds_to_add,
       process_TestDataStructure,(AQTypeFlag[]){AQIntFlag},1);
     if (ds_to_add == NULL) return NULL;
     return ds_to_add;
}

AQDataStructure adder_TestDataStructure(AQDataStructure ds,
 AQDataStructure ds_to_add, AQChar* label) {
    return ds_to_add;
}

void test_saturn(void) {
    AQArray array = aq_new_array();
    int number = 1;
    aqarray_add_item(array,aq_any(number));
    number = 2;
    aqarray_add_item(array,aq_any(number));
    number = 3;
    aqarray_add_item(array,aq_any(number));
    number = 4;
    aqarray_add_item(array,aq_any(number));
    number = 5;
    aqarray_add_item(array,aq_any(number));

    aq_array_foreach(i,array) {
        printf("%d\n", aq_get(int,aqarray_get_item(array,i)));
    }

    AQList list = aqlist_new();
    number = 6;
    aqlist_add_item(list,aq_any(number));
    number = 7;
    aqlist_add_item(list,aq_any(number));
    number = 8;
    aqlist_add_item(list,aq_any(number));
    number = 9;
    aqlist_add_item(list,aq_any(number));
    number = 10;
    aqlist_add_item(list,aq_any(number));

    aq_list_foreach(node,list) {
        printf("%d\n", aq_get(int,aqlist_get_item(node)));
    }

    AQStack stack = aqstack_new();
    number = 15;
    aqstack_push_item(stack,aq_any(number));
    number = 14;
    aqstack_push_item(stack,aq_any(number));
    number = 13;
    aqstack_push_item(stack,aq_any(number));
    number = 12;
    aqstack_push_item(stack,aq_any(number));
    number = 11;
    aqstack_push_item(stack,aq_any(number));

    //11
    printf("Peek: %d\n", aq_get(int,aqstack_peek_item(stack)));
    printf("Pop: %d\n", aq_get(int,aqstack_pop_item(stack)));
    //12
    printf("Peek: %d\n", aq_get(int,aqstack_peek_item(stack)));
    printf("Pop: %d\n", aq_get(int,aqstack_pop_item(stack)));
    //13
    printf("Peek: %d\n", aq_get(int,aqstack_peek_item(stack)));
    printf("Pop: %d\n", aq_get(int,aqstack_pop_item(stack)));
    //14
    printf("Peek: %d\n", aq_get(int,aqstack_peek_item(stack)));
    printf("Pop: %d\n", aq_get(int,aqstack_pop_item(stack)));
    //15
    printf("Peek: %d\n", aq_get(int,aqstack_peek_item(stack)));
    printf("Pop: %d\n", aq_get(int,aqstack_pop_item(stack)));

    printf("is stack empty: %d\n", aqstack_is_empty(stack));
    
    
    AQStackBuffer stackbuffer = aqstackbuffer_new();
    number = 15;
    aqstackbuffer_push_item(stackbuffer,aq_any(number));
    number = 14;
    aqstackbuffer_push_item(stackbuffer,aq_any(number));
    number = 13;
    aqstackbuffer_push_item(stackbuffer,aq_any(number));
    number = 12;
    aqstackbuffer_push_item(stackbuffer,aq_any(number));
    number = 11;
    aqstackbuffer_push_item(stackbuffer,aq_any(number));
    
    //11
    printf("Peek: %d\n", aq_get(int,aqstackbuffer_peek_item(stackbuffer)));
    printf("Pop: %d\n",aq_get(int,aqstackbuffer_pop_item(stackbuffer)));
    
    //12
    printf("Peek: %d\n", aq_get(int,aqstackbuffer_peek_item(stackbuffer)));
    printf("Pop: %d\n", aq_get(int,aqstackbuffer_pop_item(stackbuffer)));
    //13
    printf("Peek: %d\n", aq_get(int,aqstackbuffer_peek_item(stackbuffer)));
    printf("Pop: %d\n", aq_get(int,aqstackbuffer_pop_item(stackbuffer)));
    //14
    printf("Peek: %d\n", aq_get(int,aqstackbuffer_peek_item(stackbuffer)));
    printf("Pop: %d\n", aq_get(int,aqstackbuffer_pop_item(stackbuffer)));
    //15
    printf("Peek: %d\n", aq_get(int,aqstackbuffer_peek_item(stackbuffer)));
    printf("Pop: %d\n", aq_get(int,aqstackbuffer_pop_item(stackbuffer)));

    printf("is stack buffer empty: %d\n", aqstackbuffer_is_empty(stackbuffer));


    AQStore store = aqstore_new();
    number = 16;
    aqstore_add_item(store,aq_any(number),"foo");
    number = 17;
    aqstore_add_item(store,aq_any(number),"bar");
    number = 18;
    aqstore_add_item(store,aq_any(number),"foobar");
    number = 19;
    aqstore_add_item(store,aq_any(number),"bob");
    number = 20;
    aqstore_add_item(store,aq_any(number),"😀");

    printf("from %s: %d\n","foo",aq_get(int,aqstore_get_item(store,"foo")));
    printf("from %s: %d\n","bar",aq_get(int,aqstore_get_item(store,"bar")));
    printf("from %s: %d\n","foobar",aq_get(int,aqstore_get_item(store,"foobar")));
    printf("from %s: %d\n","bob",aq_get(int,aqstore_get_item(store,"bob")));
    printf("from %s: %d\n","😀",aq_get(int,aqstore_get_item(store,"😀"))); 

    aq_store_foreach(node,store) {
        printf("from foreach, %s: %d\n",
         aqstring_get_c_string(aqstore_label_from_list_node(node)),
          aq_get(int,aqlist_get_item(node)));
    }

    AQFloat4 v0 = {1,2,3,4};
    AQFloat4 v1 = {5,6,7,8};
    AQFloat4 v2 = v0 + v1;

    printf("%f,%f,%f,%f\n", v2[0],v2[1],v2[2],v2[3]);

    AQInt4 iv0 = {1,2,3,4};
    AQInt4 iv1 = {5,6,7,8};
    AQInt4 iv2 = iv0 % iv1;

    printf("%d,%d,%d,%d\n", iv2[0],iv2[1],iv2[2],iv2[3]);

    AQFloat2 dotvec0 = {1,2};
    AQFloat3 dotvec1 = {1,2,3};
    AQFloat4 dotvec2 = {1,2,3,4};

    printf("%f,%f,%f\n",aqmath_dot2(dotvec0,dotvec0),
                        aqmath_dot3(dotvec1,dotvec1),
                        aqmath_dot4(dotvec2,dotvec2));

    if (aqmath_are_equal4(v0,v0)) puts("Hello");

    v0 = -v0;
    printf("%f,%f,%f,%f\n",
                        aq_x(v0),
                        aq_y(v0),
                        aq_z(v0),
                        aq_w(v0));

    aq_math_declare_matrix_float_array(m0);
    AQFloat4x4 m1 = aqmath_identity_matrix();

    aqmath_get_matrix_float_array(m1,m0);
    printf("Hello: %f\n",m0[0]);

    AQAny test = aq_alloc(1);
    free(test);

    AQAllocatorStruct allocator = {
      .allocator_function = test_malloc,
      .free_function = test_free,
      .data = NULL
    };

    test = aq_alloc(1,&allocator);
    free(test);

    AQArray array2 = aq_new_array(&allocator);
    
    AQMultiTypeArray mta = aq_new_mta(&allocator);
    
    aq_mta_add_item(AQInt,mta,42);
    aq_mta_add_item(AQInt,mta,958754);
    aq_mta_add_item(AQByte,mta,255);
    
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta,0));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta,1));
    printf("MTA: %d\n",aq_mta_get_item(AQByte,mta,0));
    
    aqmta_iterate_all_types_with(test_mta,mta);
    
    aq_print(c_string,"MTA size for AQInt: ");
    aq_print(ulong,aq_mta_get_num_of_items(AQInt,mta));
    aq_print(c_string,".\n");
    
    aq_print(c_string,"MTA size for AQByte: ");
    aq_print(ulong,aq_mta_get_num_of_items(AQByte,mta));
    aq_print(c_string,".\n");
    
    aq_print(c_string,"MTA size for All: ");
    aq_print(ulong,aqmta_get_num_of_items_all_types(mta)); 
    aq_print(c_string,".\n");
    
    
    aq_mta_foreach(AQInt,i,mta) {
        printf("MTA foreach int: %d\n",aq_mta_get_item(AQInt,mta,i));
    }
    
    aq_mta_foreach(AQByte,i,mta) {
        printf("MTA foreach byte: %d\n",aq_mta_get_item(AQByte,mta,i));
    }
    
    aq_mta_foreachtype(i,mta) {
        AQMTAContainer container = aqmta_get_container(mta,i);
        aq_mta_define_itemvar(iterator_item);
        aq_mta_get_itemvar_from_container(container,iterator_item);
        if (iterator_item_AQInt == 42) puts("YES! 42");
        if (iterator_item_AQInt == 958754) puts("YES! 958754");
        if (iterator_item_AQByte == 255) puts("YES! 255");   
    }
    
    AQMTAStackBuffer mtasb = aq_new_mtastackbuffer(&allocator);
    
    aq_mta_define_itemvar(pop_item);
    
    aq_mtastackbuffer_push_item(AQInt,mtasb,444);
    aq_mtastackbuffer_push_item(AQInt,mtasb,404);
    aq_mtastackbuffer_push_item(AQInt,mtasb,304);
    aq_mtastackbuffer_push_item(AQByte,mtasb,255);
    aq_mtastackbuffer_push_item(AQByte,mtasb,128);
    
    aq_mtastackbuffer_peek_item(mtasb,pop_item);
    if (type_of_pop_item == AQByteFlag) printf("MTA BYTE: %d\n",pop_item_AQByte);  
    aq_mtastackbuffer_pop_item(mtasb,pop_item);
    if (type_of_pop_item == AQByteFlag) printf("MTA BYTE: %d\n",pop_item_AQByte);
    aq_mtastackbuffer_pop_item(mtasb,pop_item);
    if (type_of_pop_item == AQByteFlag) printf("MTA BYTE: %d\n",pop_item_AQByte);
    aq_mtastackbuffer_pop_item(mtasb,pop_item);
    if (type_of_pop_item == AQIntFlag) printf("MTA INT: %d\n",pop_item_AQInt);
    
    aqarray_destroy(array2);
    aqarray_destroy(array);
    aqlist_destroy(list);
    aqstack_destroy(stack);
    aqstackbuffer_destroy(stackbuffer);
    aqstore_destroy(store);
    aqmta_destroy(mta);
    aqmtastackbuffer_destroy(mtasb);
    puts("TEST!");
    int* ints_test = aq_make_c_array(10,int,&allocator);
    ints_test[9] = 300;
    printf("TEST: %d\n",ints_test[9]);
    aq_free(ints_test,&allocator);
    
    
    AQRandStateStruct rs;
    aqmath_seed_random_state_with_time(&rs);
    
    int loop = 0;
    while (loop < 25) {
         printf("TEST RANDOM: %f\n",aqmath_a_random_double(&rs,0,1));
        loop++;
    }
    printf("%s\n", "Hello World!!!");
    
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
    
    prometheus_output_file(file,(AQDataStructure)store99);
    prometheus_output_file(file2,(AQDataStructure)store99);
    
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
    AQStore test_data = (AQStore)prometheus_load_file(file);
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
    aq_destroy(file);
    
    file2 = deimos_get_file_from_string(file_string,DeimosReadModeFlag);
    test_data = (AQStore)prometheus_load_file(file2);
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
    
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta2000,0));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta2000,1));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta2000,2));
    printf("MTA: %d\n",aq_mta_get_item(AQInt,mta2000,0));
    
    TestDataStructure* ds500 = new_TestDataStructure(5);
    
    AQMTAContainer* container = aq_new(AQMTAContainer);
    container->flag = AQMTAContainerFlag;
    container->type = AQIntFlag;
    container->AQIntVal = 999;
    
    aqarray_add_item(array2999,mta2999);
    aqarray_add_item(array2999,mta2000);
    aqarray_add_item(array2999,string2999);
    aqarray_add_item(array2999,ds500);
    aqarray_add_item(array2999,container);
    
    
    aqarray_add_item(array999,array2999);
     
    AQStore store500 = aqstore_new();
    
    aqstore_add_item(store500,array999,"THE ARRAY");
    
    //prometheus_output_file(file,(AQDataStructure)array999);
    prometheus_serialize(file,(PrometheusDataStructure)array999);
    
    prometheus_serialize(file,(PrometheusDataStructure)store500);
    
    free(container);
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
    
    printf("PRINT FILE STRING: %s\n",aqstring_get_c_string(file_string_999));
    
    aq_destroy(file);
    aq_destroy(file999);
    aq_destroy(file_string_999);
    aq_destroy(deserializer);
}

int main(int argc, const char **argv) {
    test_saturn();
}
