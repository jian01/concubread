#include "panic_utils.h"
#include "resource_manager.h"
#include "debug_utils.h"

void _fatal_error_abort(const char* filename, int line_no, long pid, const char* message, int exit_code){
  print_fatal(filename, line_no, pid, message);
  free_all_resources();
  exit(exit_code);
}
