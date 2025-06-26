#include <stdio.h>
#include <stdlib.h>

#define run_test(id)\
 void saturn_test_##id(void);\
 if (test_id == 0 || test_id == id) {\
  printf("Running test %d...\n",id);\
  saturn_test_##id();\
  printf("Ran test %d.\n",id);\
 }

int main(int argc, const char **argv) {
    if (argc < 3) abort();
    int test_id = atoi(argv[2]);
    run_test(1);
    run_test(2);
    run_test(3);
    run_test(4);
    run_test(5);
}
