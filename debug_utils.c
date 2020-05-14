#include <stdio.h>
#include "debug_utils.h"
#include "lock_utils.h"
#include "mensajes_log.h"

#define MAXIMUM_LOG_STRING_LEN 300
#define ERROR_FORMAT L""ERROR_TAG"%ls\n"
#define INFO_FORMAT L""INFO_TAG"%ls\n"
#define DEBUG_FORMAT L""DEBUG_TAG"%ls\n"

bool print_error(FILE* debug_file, const wchar_t* format, ...){
  if(!debug_file) return true;
  va_list args;
  va_start(args, format);

  wchar_t formatted_message[MAXIMUM_LOG_STRING_LEN];
  vswprintf(formatted_message, MAXIMUM_LOG_STRING_LEN, format, args);

  if(!acquire_write_lock(fileno(debug_file))) return false;
  fseek(debug_file, 0, SEEK_CUR);
  fwprintf(debug_file, ERROR_FORMAT, formatted_message);
  fflush(debug_file);
  release_locked_file(fileno(debug_file));
  return true;
}

bool print_info(FILE* debug_file, const wchar_t* format, ...){
  if(!debug_file) return true;
  va_list args;
  va_start(args, format);

  wchar_t formatted_message[MAXIMUM_LOG_STRING_LEN];
  vswprintf(formatted_message, MAXIMUM_LOG_STRING_LEN, format, args);
  va_end(args);

  if(!acquire_write_lock(fileno(debug_file))) return false;
  fseek(debug_file, 0, SEEK_CUR);
  fwprintf(debug_file, INFO_FORMAT, formatted_message);
  fflush(debug_file);
  release_locked_file(fileno(debug_file));
  return true;
}

bool print_debug(FILE* debug_file, const wchar_t* format, ...){
  if(!debug_file) return true;
  va_list args;
  va_start(args, format);

  wchar_t formatted_message[MAXIMUM_LOG_STRING_LEN];
  vswprintf(formatted_message, MAXIMUM_LOG_STRING_LEN, format, args);

  if(!acquire_write_lock(fileno(debug_file))) return false;
  fseek(debug_file, 0, SEEK_CUR);
  fwprintf(debug_file, DEBUG_FORMAT, formatted_message);
  fflush(debug_file);
  release_locked_file(fileno(debug_file));
  return true;
}
