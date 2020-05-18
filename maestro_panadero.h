#ifndef _MAESTRO_PANADERO
  #define _MAESTRO_PANADERO
  #include <stdio.h>

  int maestro_panadero(FILE* panadero_read_end, FILE* repartidor_write_end,
                      FILE* especialista_masas_read_end,
                      size_t* shared_especialista_pedidos, FILE* shared_especialista_pedidos_lock);

#endif
