#ifndef _PANIC_UTILS
  #define _PANIC_UTILS
  #define fatal_error_abort(...) _fatal_error_abort(__FILE__, __LINE__, (long)getpid(), __VA_ARGS__)

  void _fatal_error_abort(const char* filename, int line_no, long pid, const char* message, int exit_code);
#endif
