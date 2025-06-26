#include "../src/aquarius.h"

void saturn_test_1(void) {
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
    
    aq_destroy(array);
    aq_destroy(list);
    aq_destroy(stack);
    aq_destroy(stackbuffer);
    aq_destroy(store);

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

}
