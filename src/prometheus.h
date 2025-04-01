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
 AQString type, AQDataStructure ds, PrometheusOutputContainerLambda output_container);    
typedef AQInt (*PrometheusSerializerLambda)(DeimosFile file, AQChar* label, AQDataStructure ds,
 PrometheusAccessOutputContainerLambda output_container);
typedef AQInt (*PrometheusProcessValueLambda)(AQDataStructure ds, AQTypeFlag* type, AQULong* index, 
 AQMTAContainer* container);
typedef AQDataStructure (*PrometheusAccessBlockGeneratorLambda)(PrometheusDeserializer deserializer, 
 AQChar* adder_type, AQDataStructure ds);
typedef AQDataStructure (*PrometheusAccessValueGeneratorLambda)(PrometheusDeserializer deserializer, 
 AQDataStructure ds, PrometheusProcessValueLambda process_value, AQTypeFlag* type_array, AQULong type_count); 
typedef AQDataStructure (*PrometheusGeneratorLambda)(PrometheusDeserializer deserializer, 
 AQChar* adder_type, AQDataStructure super_ds, PrometheusAccessBlockGeneratorLambda block_generator,
 PrometheusAccessValueGeneratorLambda value_generator);
typedef AQDataStructure (*PrometheusAdderLambda)(AQDataStructure ds, AQDataStructure ds_to_add, AQChar* label);

#define PROMETHEUS_SERIALIZATION_BASE_CLASS\
 AQ_DATA_STRUCTURE_BASE_CLASS\
 PrometheusSerializerLambda serialize;
 
 
AQInt prometheus_serialize(DeimosFile file, PrometheusDataStructure ds);
AQInt prometheus_serialize_with_label(DeimosFile file, AQString label, PrometheusDataStructure ds);

PrometheusDeserializer prometheus_deserializer_new(DeimosFile file);
void prometheus_deserializer_destroy(PrometheusDeserializer deserializer);
AQInt prometheus_register_deserializer(PrometheusDeserializer deserializer, AQString name,
 PrometheusGeneratorLambda generator, PrometheusAdderLambda adder);
AQDataStructure prometheus_deserialize(PrometheusDeserializer deserializer);

#endif /* prometheus_h */
