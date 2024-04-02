#ifndef deimos_h
#define deimos_h

#include "aquarius.h"

#include <float.h>

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
AQInt deimos_advance_file_position(DeimosFile file, AQULong offset);
AQInt deimos_retreat_file_position(DeimosFile file, AQULong offset);
AQString deimos_get_string(DeimosFile file, AQInt start, AQInt end);
AQByte deimos_get_byte(DeimosFile file);
AQSByte deimos_get_sbyte(DeimosFile file);
AQShort deimos_get_short(DeimosFile file);
AQUShort deimos_get_ushort(DeimosFile file);
AQInt deimos_get_integer(DeimosFile file);
AQUInt deimos_get_uinteger(DeimosFile file);
AQLong deimos_get_long(DeimosFile file);
AQULong deimos_get_ulong(DeimosFile file);
AQFloat deimos_get_float(DeimosFile file);
AQDouble deimos_get_double(DeimosFile file);
AQInt deimos_output_add_to_tab(DeimosFile file, AQInt value);
AQInt deimos_output_sub_from_tab(DeimosFile file, AQInt value);
AQInt deimos_output_tab(DeimosFile file);
AQInt deimos_output_character(DeimosFile file, AQInt value);
AQInt deimos_output_string(DeimosFile file, AQChar* value);
AQInt deimos_output_byte(DeimosFile file, AQByte value);
AQInt deimos_output_sbyte(DeimosFile file, AQSByte value);
AQInt deimos_output_short(DeimosFile file, AQShort value);
AQInt deimos_output_ushort(DeimosFile file, AQUShort value);
AQInt deimos_output_integer(DeimosFile file, AQInt value);
AQInt deimos_output_uinteger(DeimosFile file, AQUInt value);
AQInt deimos_output_long(DeimosFile file, AQLong value);
AQInt deimos_output_ulong(DeimosFile file, AQULong value);
AQInt deimos_output_float(DeimosFile file, AQFloat value);
AQInt deimos_output_double(DeimosFile file, AQDouble value);
AQUInt deimos_read_32bit_int(DeimosFile file, AQInt* error);
AQInt deimos_write_32bit_int(DeimosFile file, AQInt num);
AQFloat deimos_read_32bit_float(DeimosFile file, AQInt* error);
AQInt deimos_write_32bit_float(DeimosFile file, AQFloat num);
AQInt deimos_get_utf32_character(DeimosFile file);
AQInt deimos_peek_utf32_character(DeimosFile file, AQULong* offset);
AQInt deimos_peek_last_utf32_character(DeimosFile file, AQULong offset);

#endif /* deimos_h */
