#include "../src/aquarius.h"

static AQAny test_malloc(AQAny allocation_data, AQULong size_in_bytes) {
    puts("Hello from the world!!!!!!!!!");
    return malloc(size_in_bytes);
}

static AQStatus test_free(AQAny allocation_data, AQAny data_to_be_freed) {
    puts("FREE!!!!!");
    free(data_to_be_freed);
    return AQSuccessValue;
}

static AQStatus test_mta(AQAny data) {
    AQMTAContainer container = *((AQMTAContainer*)data);
    aq_mta_define_itemvar(iterator_item);
    aq_mta_get_itemvar_from_container(container,iterator_item);
    if (iterator_item_AQInt == 42) puts("YES!!!!!42");
    if (iterator_item_AQInt == 958754) puts("YES!!!!!958754");
    if (iterator_item_AQByte == 255) puts("YES!!!!!255");
    return AQSuccessValue; 
}

void saturn_test_3(void) {
    AQAllocatorStruct allocator = {
      .allocator_function = test_malloc,
      .free_function = test_free,
      .data = NULL
    };
    
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
}
