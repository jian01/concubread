#ifndef _REPARTIDOR
  #define _REPARTIDOR
  #include <stdio.h>
  #include <sys/types.h>
  #include "stock_tracking.h"

  int repartidor(FILE* repartidor_read_end, pedidos_count_t* shared_count, FILE* shared_count_lockfile,
                  FILE* repartidor_pipe_rd_lockfile);

#endif
