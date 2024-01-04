#ifndef deimos_h
#define deimos_h

#include "aquarius.h"

typedef struct DeimosFile_s* DeimosFile;

typedef enum { 
 DeimosReadModeFlag, 
 DeimosWriteModeFlag 
} DeimosFileModeFlag;

AQAny deimos_load_library_file(const AQChar* filepath);
AQInt deimos_free_library(AQAny library);
AQAny deimos_get_function(AQAny library, const AQChar* name);
DeimosFile deimos_open_file(const AQChar* filepath, DeimosFileModeFlag mode);
void deimos_close_file(DeimosFile file);
FILE* deimos_get_file_struct(DeimosFile file);
AQULong deimos_get_file_position(DeimosFile file);
AQInt deimos_set_file_position(DeimosFile file, AQULong position);
AQInt deimos_output_character(DeimosFile file, AQInt value);
AQInt deimos_output_string(DeimosFile file, AQChar* value);
AQInt deimos_output_integer(DeimosFile file, AQInt value);
AQInt deimos_output_float(DeimosFile file, AQFloat value);
AQUInt deimos_read_32bit_int(DeimosFile file, AQInt* error);
AQInt deimos_write_32bit_int(DeimosFile file, AQInt num);
AQFloat deimos_read_32bit_float(DeimosFile file, AQInt* error);
AQInt deimos_write_32bit_float(DeimosFile file, AQFloat num);
AQInt deimos_get_utf32_character(DeimosFile file);

#endif /* deimos_h */
