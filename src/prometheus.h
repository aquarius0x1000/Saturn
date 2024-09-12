#ifndef prometheus_h
#define prometheus_h

#include "deimos.h"

#define PRO_BLOCK_NOT_DONE 0
#define PRO_BLOCK_DONE 1

typedef struct Prometheus_s* Prometheus;

typedef AQInt (*PrometheusGetValueLambda)(AQDataStructure ds, AQMTAContainer* container);
typedef AQInt (*PrometheusGetBlockLambda)(AQDataStructure ds);
typedef AQInt (*PrometheusPrintListLambda)(DeimosFile file, AQDataStructure ds, 
  PrometheusGetValueLambda get_value);
typedef AQInt (*PrometheusPrintBlockLambda)(DeimosFile file, AQDataStructure ds, 
 PrometheusGetBlockLambda get_block);  
typedef AQInt (*PrometheusGetContainerLambda)(DeimosFile file, AQDataStructure ds, 
    PrometheusPrintListLambda print_list, PrometheusPrintBlockLambda print_block);
typedef AQInt (*PrometheusSerializerLambda)(Prometheus prometheus);

AQInt prometheus_output_file(DeimosFile file, AQDataStructure data);
AQDataStructure prometheus_load_file(DeimosFile file);

#endif /* prometheus_h */
