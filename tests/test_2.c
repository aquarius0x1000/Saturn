#include "../src/aquarius.h"

static AQAny test_malloc(AQAny allocation_data, AQULong size_in_bytes) {
    puts("Hello from the world!!!!!!!!!");
    return malloc(size_in_bytes);
}

static void test_free(AQAny allocation_data, AQAny data_to_be_freed) {
    puts("FREE!!!!!");
    return free(data_to_be_freed);
}

void saturn_test_2(void) {
    AQAllocatorStruct allocator = {
      .allocator_function = test_malloc,
      .free_function = test_free,
      .data = NULL
    };

    AQAny test = aq_alloc(1,&allocator);
    free(test);
    
    AQArray array = aq_new_array(&allocator);
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

    AQList list = aq_new_list(&allocator);
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

    AQStack stack = aq_new_stack(&allocator);
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
    
    
    AQStackBuffer stackbuffer = aq_new_stackbuffer(&allocator);
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


    AQStore store = aq_new_store(&allocator);
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

    aq_destroy(array);
    aq_destroy(list);
    aq_destroy(stack);
    aq_destroy(stackbuffer);
    aq_destroy(store);
}
