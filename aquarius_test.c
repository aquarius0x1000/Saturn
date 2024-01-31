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

void test_aquarius(void) {
    AQArray array = aqarray_new();
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
    
    printf("MTA size for AQInt: %d\n",aq_mta_get_num_of_items(AQInt,mta));
    printf("MTA size for AQByte: %d\n",aq_mta_get_num_of_items(AQByte,mta));
    printf("MTA size for All: %d\n",aqmta_get_num_of_items_all_types(mta));
    
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
    
    AQStore store99 = aqstore_new();
    AQArray array99 = aqarray_new();
    AQMultiTypeArray mta99 = aqmta_new();
    AQMultiTypeArray mta100 = aqmta_new();
    AQString string99 = aqstr("Hello World!!!!",&allocator);
    aq_mta_add_item(AQInt,mta99,42);
    aq_mta_add_item(AQInt,mta99,958754);
    aq_mta_add_item(AQInt,mta99,255);
    aq_mta_add_item(AQFloat,mta99,255.0255f);
    aq_mta_add_item(AQDouble,mta99,255.000255);
    
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
    
    prometheus_output_file(file,store99);
    
    aqstring_destroy(string99);
    aqmta_destroy(mta99);
    aqmta_destroy(mta100);
    aqarray_destroy(array99);
    aqstore_destroy(store99);
    
    deimos_close_file(file);
}

int main(int argc, const char **argv) {
    test_aquarius();
}
