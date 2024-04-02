#ifndef prometheus_h
#define prometheus_h

#include "deimos.h"

AQInt prometheus_output_file(DeimosFile file, AQDataStructure data);
AQDataStructure prometheus_load_file(DeimosFile file);

#endif /* prometheus_h */
