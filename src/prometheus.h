#ifndef prometheus_h
#define prometheus_h

#include "deimos.h"

typedef char* (*ProGetStringFuncType)(AQDataStructure ds);
typedef AQInt (*ProGetValueFuncType)(AQDataStructure ds, AQMTAContainer* container);

AQInt prometheus_output_file(DeimosFile file, AQDataStructure data);
AQDataStructure prometheus_load_file(DeimosFile file);

#endif /* prometheus_h */
