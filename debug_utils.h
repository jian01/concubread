#ifndef _DEBUG_UTILS
#define _DEBUG_UTILS
#include <stdarg.h>
#include <stdbool.h>
#include <wchar.h>
#include <sys/types.h>
#include <unistd.h>

#define debug(...) print_debug(__FILE__, __LINE__, (long)getpid(), __VA_ARGS__)
#define info(...) print_info(__FILE__, __LINE__, (long)getpid(), __VA_ARGS__)
#define error(...) print_error(__FILE__, __LINE__, (long)getpid(), __VA_ARGS__)
#define fatal(...) print_fatal(__FILE__, __LINE__, (long)getpid(), __VA_ARGS__)

bool print_error(const char* filename, int line_no, long pid, FILE* debug_file, const wchar_t* format, ...);

void print_fatal(const char* filename, int line_no, long pid, FILE* debug_file, const char* format, ...);

bool print_info(const char* filename, int line_no, long pid, FILE* debug_file, const wchar_t* format, ...);

bool print_debug(const char* filename, int line_no, long pid, FILE* debug_file, const wchar_t* format, ...);
#endif
