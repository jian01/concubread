#ifndef _MAESTRO_PANADERO
  #define _MAESTRO_PANADERO
  #include <stdio.h>

  /*
  Maestro panadero:

  El maestro panadero recibe los pedidos del recepcionista y hornea los panes a medida el maestro de masa madre le provee la masa.
  El tiempo de horneado es constante, cuando termina entrega el pan al repartidor

  Devuelve 0 como exit code si fue exitoso, otro numero en caso contrario
  */

  int maestro_panadero(FILE* panadero_read_end, FILE* repartidor_write_end,
                      FILE* especialista_masas_read_end, FILE* pedidos_especialista_write_end,
                      FILE* pizzero_pipe_rd_lockfile, FILE* especialista_masa_rd_lockfile);

#endif
