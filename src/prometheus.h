#ifndef prometheus_h
#define prometheus_h

#include "deimos.h"

#define PROMETHEUS_BLOCK_NOT_DONE 0
#define PROMETHEUS_BLOCK_DONE 1

#define PROMETHEUS_LIST_NOT_DONE 1
#define PROMETHEUS_LIST_DONE 0

typedef struct PrometheusDeserializer_s* PrometheusDeserializer;
typedef struct PrometheusDataStructure_s* PrometheusDataStructure;

typedef AQInt (*PrometheusGetValueLambda)(AQDataStructure ds, AQULong* index,
 AQMTAContainer* container);
typedef AQInt (*PrometheusOutputBlockLambda)(DeimosFile file, AQDataStructure ds);
typedef AQInt (*PrometheusPrintListLambda)(DeimosFile file, AQDataStructure ds, 
  PrometheusGetValueLambda get_value);
typedef AQInt (*PrometheusPrintBlockLambda)(DeimosFile file, AQDataStructure ds, 
 PrometheusOutputBlockLambda output_block);  
typedef AQInt (*PrometheusOutputContainerLambda)(DeimosFile file, AQDataStructure ds, 
    PrometheusPrintListLambda print_list, PrometheusPrintBlockLambda print_block);
typedef AQInt (*PrometheusAccessOutputContainerLambda)(DeimosFile file, AQChar* label, 
 AQChar* type, AQDataStructure ds, PrometheusOutputContainerLambda output_container);    
typedef AQInt (*PrometheusSerializerLambda)(DeimosFile file, AQChar* label, AQDataStructure ds, PrometheusAccessOutputContainerLambda output_container);

#define PROMETHEUS_SERIALIZATION_BASE_CLASS\
 AQ_DATA_STRUCTURE_BASE_CLASS\
 PrometheusSerializerLambda serialize;
 
 
AQInt prometheus_serialize(DeimosFile file, PrometheusDataStructure ds);

AQInt prometheus_output_file(DeimosFile file, AQDataStructure data);
AQDataStructure prometheus_load_file(DeimosFile file);

#endif /* prometheus_h */
