#ifndef _RECEPCIONISTA
  #define _RECEPCIONISTA
  #include <stdio.h>
  #include "stock_tracking.h"

  /*
  Recepcionista:

  Lee un archivo de pedidos y los despacha a los distintos cocineros correspondientes.
  Termina cuando no hay mas pedidos (exit code 0) o si hubo un error (exit code != 0)
  */
  int recepcionista(FILE* pizzero_write_end, FILE* panadero_write_end, FILE* input_file,
                    pedidos_count_t* shared_count, FILE* shared_count_lockfile);

#endif
