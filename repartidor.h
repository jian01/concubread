#ifndef _REPARTIDOR
  #define _REPARTIDOR
  #include <stdio.h>
  #include <sys/types.h>
  #include "stock_tracking.h"

  /*
  Repartidor:

  Entrega los envios de los maestros panaderos y pizzeros.
  Termina cuando los maestros pizzeros y panaderos no van a enviar mas comida (exit code 0) o si hubo un error (exit code != 0)
  */
  int repartidor(FILE* repartidor_read_end, pedidos_count_t* shared_count, FILE* shared_count_lockfile,
                  FILE* repartidor_pipe_rd_lockfile);

#endif
