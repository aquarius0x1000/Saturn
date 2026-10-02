#include "./src/saturn.h"

#define run_test(id)\
 void saturn_test_##id(void);\
 if (test_id == 0 || test_id == id) {\
  printf("Running test %d...\n",id);\
  saturn_test_##id();\
  printf("Ran test %d.\n",id);\
 }

int main(int argc, const char **argv) {
    printf("TEST: %s\n",argv[1]); //[@Int]:(test_id);
    AQString string = aqstring_new_from_c_string(argv[1]);
    DeimosFile file = deimos_get_file_from_string(string,DeimosReadModeFlag);
    PrometheusDeserializer deserializer = prometheus_deserializer_new(file);
    AQDataStructure aq_data_structure = prometheus_deserialize(deserializer);
    if (aq_data_structure == NULL) puts("MAIN: aq_data_structure is NULL!");
    if (aq_data_structure == NULL) exit(1);
    if (aqds_get_flag(aq_data_structure) != AQMTAContainerFlag) puts("MAIN: aq_data_structure is not an int!");
    if (aqds_get_flag(aq_data_structure) != AQMTAContainerFlag) exit(1);
    AQMTAContainer* container = (AQMTAContainer*)aq_data_structure;
    if (container->type != AQIntFlag) puts("MAIN: container is not an int!");
    if (container->type != AQIntFlag) exit(1);
    int test_id = container->AQIntVal;
    free(aq_data_structure);
    aq_destroy(deserializer);
    aq_destroy(file);
    run_test(1);
    run_test(2);
    run_test(3);
    run_test(4);
    run_test(5);
}
