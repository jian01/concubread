#ifndef _RECEPCIONISTA
  #define _RECEPCIONISTA
  #include <stdio.h>
  #include "stock_tracking.h"

  int recepcionista(FILE* pizzero_write_end, FILE* panadero_write_end, FILE* input_file,
                    pedidos_count_t* shared_count, FILE* shared_count_lockfile);

#endif
