#ifndef _EXPERTO_MASA_MADRE
  #define _EXPERTO_MASA_MADRE
  #include <stdio.h>

  int especialista_masa_madre(size_t* shared_especialista_pedidos, FILE* shared_especialista_pedidos_lock,
    FILE* especialista_masas_write_end);

#endif
