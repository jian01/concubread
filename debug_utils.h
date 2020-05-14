#ifndef _DEBUG_UTILS
#define _DEBUG_UTILS
#include <stdarg.h>
#include <stdbool.h>
#include <wchar.h>

bool print_error(FILE* debug_file, const wchar_t* format, ...);

bool print_info(FILE* debug_file, const wchar_t* format, ...);

bool print_debug(FILE* debug_file, const wchar_t* format, ...);
#endif
