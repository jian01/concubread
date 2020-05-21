#ifndef _MAESTRO_PIZZERO
  #define _MAESTRO_PIZZERO
  #include <stdio.h>

  /*
  Maestro pizzero:

  El maestro pizzero recibe los pedidos del recepcionista y hornea las pizzas a medida el maestro de masa madre le provee la masa.
  El tiempo de horneado es una variable aleatoria de distribucion uniforme, cuando termina entrega la pizza al repartidor

  Devuelve 0 como exit code si fue exitoso, otro numero en caso contrario
  */

  int maestro_pizzero(FILE* pizzero_read_end, FILE* repartidor_write_end,
                      FILE* especialista_masas_read_end,
                      size_t* shared_especialista_pedidos, FILE* shared_especialista_pedidos_lock,
                      FILE* pizzero_pipe_rd_lockfile, FILE* especialista_masa_rd_lockfile);

#endif
