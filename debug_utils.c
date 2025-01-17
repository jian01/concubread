#include <stdio.h>
#include "debug_utils.h"
#include "lock_utils.h"
#include "resource_manager.h"
#include "mensajes.h"
#include "panic_utils.h"

#define MAXIMUM_LOG_STRING_LEN 500
#define FATAL_FORMAT_NOT_UNICODE ""FATAL_TAG_NOT_UNICODE""LOG_DATA_FORMAT"%s\n"
#define FATAL_FORMAT L""FATAL_TAG""LOG_DATA_FORMAT"%s\n"
#define ERROR_FORMAT L""ERROR_TAG""LOG_DATA_FORMAT"%ls\n"
#define INFO_FORMAT L""INFO_TAG""LOG_DATA_FORMAT"%ls\n"
#define DEBUG_FORMAT L""DEBUG_TAG""LOG_DATA_FORMAT"%ls\n"

#define SIGNAL_RESTORE_FATAL -20


FILE* debug_file = NULL;

void set_debug_file(FILE* _debug_file){
  /*
  Setea el debug file a ser utilizado para imprimir los logs
  */
  debug_file = _debug_file;
}

void print_fatal(const char* filename, int line_no, long pid, const char* format, ...){
  /*
  Funcion para imprimir un error fatal, imprime (si existe) al logfile y a stderr.
  Registra la linea de codigo y el pid que la llamo
  */
  va_list args;
  va_start(args, format);

  char formatted_message[MAXIMUM_LOG_STRING_LEN];
  vsnprintf(formatted_message, MAXIMUM_LOG_STRING_LEN, format, args);

  fprintf(stderr, FATAL_FORMAT_NOT_UNICODE, filename, line_no, pid, formatted_message);

  if (debug_file){
    if(!acquire_exclusive_lock(fileno(debug_file))) return;
    fseek(debug_file, 0, SEEK_CUR);
    fwprintf(debug_file, FATAL_FORMAT, filename, line_no, pid, formatted_message);
    fflush(debug_file);
    release_locked_file(fileno(debug_file));
  }
}

bool print_error(const char* filename, int line_no, long pid, const wchar_t* format, ...){
  /*
  Funcion para imprimir un error, imprime (si existe) al logfile.
  Registra la linea de codigo y el pid que la llamo

  Devuelve true si la escritura fue exitosa
  */
  if(!debug_file) return true;
  block_signals();
  va_list args;
  va_start(args, format);

  wchar_t formatted_message[MAXIMUM_LOG_STRING_LEN];
  vswprintf(formatted_message, MAXIMUM_LOG_STRING_LEN, format, args);

  if(!acquire_exclusive_lock(fileno(debug_file))) return false;
  fseek(debug_file, 0, SEEK_CUR);
  fwprintf(debug_file, ERROR_FORMAT, filename, line_no, pid, formatted_message);
  fflush(debug_file);
  release_locked_file(fileno(debug_file));
  if(!restore_signals()) fatal_error_abort(FATAL_IGNORE_SIGNAL, SIGNAL_RESTORE_FATAL);
  return true;
}

bool print_info(const char* filename, int line_no, long pid, const wchar_t* format, ...){
  /*
  Funcion para imprimir un mensaje de info, imprime (si existe) al logfile.
  Registra la linea de codigo y el pid que la llamo

  Devuelve true si la escritura fue exitosa
  */
  if(!debug_file) return true;
  block_signals();
  va_list args;
  va_start(args, format);

  wchar_t formatted_message[MAXIMUM_LOG_STRING_LEN];
  vswprintf(formatted_message, MAXIMUM_LOG_STRING_LEN, format, args);
  va_end(args);

  if(!acquire_exclusive_lock(fileno(debug_file))) return false;
  fseek(debug_file, 0, SEEK_CUR);
  fwprintf(debug_file, INFO_FORMAT, filename, line_no, pid, formatted_message);
  fflush(debug_file);
  release_locked_file(fileno(debug_file));
  if(!restore_signals()) fatal_error_abort(FATAL_IGNORE_SIGNAL, SIGNAL_RESTORE_FATAL);
  return true;
}

bool print_debug(const char* filename, int line_no, long pid, const wchar_t* format, ...){
  /*
  Funcion para imprimir un log de debug, imprime (si existe) al logfile.
  Registra la linea de codigo y el pid que la llamo

  Devuelve true si la escritura fue exitosa
  */
  if(!debug_file) return true;
  block_signals();
  va_list args;
  va_start(args, format);

  wchar_t formatted_message[MAXIMUM_LOG_STRING_LEN];
  vswprintf(formatted_message, MAXIMUM_LOG_STRING_LEN, format, args);

  if(!acquire_exclusive_lock(fileno(debug_file))) return false;
  fseek(debug_file, 0, SEEK_CUR);
  fwprintf(debug_file, DEBUG_FORMAT, filename, line_no, pid, formatted_message);
  fflush(debug_file);
  release_locked_file(fileno(debug_file));
  if(!restore_signals()) fatal_error_abort(FATAL_IGNORE_SIGNAL, SIGNAL_RESTORE_FATAL);
  return true;
}
