#ifndef _DEBUG_UTILS
#define _DEBUG_UTILS
#include <stdarg.h>
#include <stdbool.h>
#include <wchar.h>
#include <sys/types.h>
#include <unistd.h>

// Macros para agregar contexto a la impresion de logs
#define debug(...) print_debug(__FILE__, __LINE__, (long)getpid(), __VA_ARGS__)
#define info(...) print_info(__FILE__, __LINE__, (long)getpid(), __VA_ARGS__)
#define error(...) print_error(__FILE__, __LINE__, (long)getpid(), __VA_ARGS__)
#define fatal(...) print_fatal(__FILE__, __LINE__, (long)getpid(), __VA_ARGS__)

/*
Setea el debug file a ser utilizado para imprimir los logs
*/
void set_debug_file(FILE* _debug_file);

/*
Funcion para imprimir un error, imprime (si existe) al logfile.
Registra la linea de codigo y el pid que la llamo

Devuelve true si la escritura fue exitosa
*/
bool print_error(const char* filename, int line_no, long pid, const wchar_t* format, ...);

/*
Funcion para imprimir un error fatal, imprime (si existe) al logfile y a stderr.
Registra la linea de codigo y el pid que la llamo
*/
void print_fatal(const char* filename, int line_no, long pid, const char* format, ...);

/*
Funcion para imprimir un mensaje de info, imprime (si existe) al logfile.
Registra la linea de codigo y el pid que la llamo

Devuelve true si la escritura fue exitosa
*/
bool print_info(const char* filename, int line_no, long pid, const wchar_t* format, ...);

/*
Funcion para imprimir un log de debug, imprime (si existe) al logfile.
Registra la linea de codigo y el pid que la llamo

Devuelve true si la escritura fue exitosa
*/
bool print_debug(const char* filename, int line_no, long pid, const wchar_t* format, ...);
#endif
