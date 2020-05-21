#include "panic_utils.h"
#include "resource_manager.h"
#include "debug_utils.h"

void _fatal_error_abort(const char* filename, int line_no, long pid, const char* message, int exit_code){
  /*
  Se aborta con un error fatal, incluye el contexto en el cual sucedio, un mensaje a imprimir y un exit code a emitir
  */
  print_fatal(filename, line_no, pid, message);
  free_all_resources();
  exit(exit_code);
}
