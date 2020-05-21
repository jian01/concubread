#ifndef _EXPERTO_MASA_MADRE
  #define _EXPERTO_MASA_MADRE
  #include <stdio.h>

  /*
  Especialista de masa madre

  El especialista de masa madre alimenta la masa periodicamente y a medida le piden masa, si tiene listas las entrega

  Devuelve 0 si fue exitoso, otro numero en caso contrario.
  */

  int especialista_masa_madre(size_t* shared_especialista_pedidos, FILE* shared_especialista_pedidos_lock,
    FILE* especialista_masas_write_end);

#endif
