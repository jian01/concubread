#ifndef _MAESTRO_PIZZERO
  #define _MAESTRO_PIZZERO
  #include <stdio.h>

  int maestro_pizzero(FILE* pizzero_read_end, FILE* repartidor_write_end,
                      FILE* especialista_masas_read_end,
                      size_t* shared_especialista_pedidos, FILE* shared_especialista_pedidos_lock);

#endif
