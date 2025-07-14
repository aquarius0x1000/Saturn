#include "../src/aquarius.h"
#include "../src/prometheus.h"

static AQAny test_malloc(AQAny allocation_data, AQULong size_in_bytes) {
    return malloc(size_in_bytes);
}

static AQStatus test_free(AQAny allocation_data, AQAny data_to_be_freed) {
    free(data_to_be_freed);
    return AQSuccessValue;
}

void saturn_test_5(void) {
    AQAllocatorStruct allocator = {
      .allocator_function = test_malloc,
      .free_function = test_free,
      .data = NULL
    };
    
    aq_print(c_string,"Hello World!!!\n");
   
}
