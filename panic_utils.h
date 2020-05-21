#ifndef _PANIC_UTILS
  #define _PANIC_UTILS

  /*
  Macro wrapper para asegurarse que se imprima el contexto (archivo, linea de codigo y pid)
  */
  #define fatal_error_abort(...) _fatal_error_abort(__FILE__, __LINE__, (long)getpid(), __VA_ARGS__)

  /*
  Se aborta con un error fatal, incluye el contexto en el cual sucedio, un mensaje a imprimir y un exit code a emitir
  */
  void _fatal_error_abort(const char* filename, int line_no, long pid, const char* message, int exit_code);
#endif
