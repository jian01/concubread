#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "lock_utils.h"
#include "debug_utils.h"
#include "panic_utils.h"
#include "mensajes.h"
#include "resource_manager.h"
#include "constants.h"
#include "maestro_pizzero.h"

#define MALLOC_ERROR_EXIT_CODE -1
#define GETLINE_ERROR_EXIT_CODE -3
#define FREEING_RESOURCE_EXIT_CODE -4
#define LOCK_ERROR_EXIT_CODE -5
#define SPRINTF_ERROR_EXIT_CODE -30


int maestro_panadero(FILE* panadero_read_end, FILE* repartidor_write_end,
                    FILE* especialista_masas_read_end,
                    size_t* shared_especialista_pedidos, FILE* shared_especialista_pedidos_lock,
                    FILE* panadero_pipe_rd_lockfile, FILE* especialista_masa_rd_lockfile){
  /*
  Maestro panadero:

  El maestro panadero recibe los pedidos del recepcionista y hornea los panes a medida el maestro de masa madre le provee la masa.
  El tiempo de horneado es constante, cuando termina entrega el pan al repartidor

  Devuelve 0 como exit code si fue exitoso, otro numero en caso contrario
  */
  char* buffer = (char *)safe_malloc(DEFAULT_BUFFER_LEN * sizeof(char));
  int por_entregar_ids[MAXIMO_SIMULTANEO_POR_COCINERO];
  size_t por_entregar = 0;
  size_t por_conseguir_masa_madre = 0;
  size_t entregadas = 0;
  if(!buffer) fatal_error_abort(FATAL_MALLOC, MALLOC_ERROR_EXIT_CODE);
  memset(buffer, 0, DEFAULT_BUFFER_LEN);
  info(INICIANDO_MAESTRO_PANADERO);
  int read_result = 0;
  do {
    if(por_entregar > 0){
      if(sprintf(buffer, "%s%d", PAN_KEYWORD, por_entregar_ids[0]) < 0) fatal_error_abort(FATAL_SPRINTF, SPRINTF_ERROR_EXIT_CODE);
      if(write(fileno(repartidor_write_end), buffer, DEFAULT_BUFFER_LEN) > 0){
        debug(ENTREGANDO_PAN, por_entregar_ids[0]);
        por_entregar--;
        entregadas++;
        for(size_t i=0; i<MAXIMO_SIMULTANEO_POR_COCINERO-1; i++) por_entregar_ids[i] = por_entregar_ids[i+1];
      } else {
        error(MAESTRO_PANADERO_ERROR_AL_ENTREGAR, por_entregar_ids[0]);
      }
    } else if(por_conseguir_masa_madre > 0){
      if(!acquire_exclusive_lock(fileno(shared_especialista_pedidos_lock))) fatal_error_abort(FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);
      *shared_especialista_pedidos = *shared_especialista_pedidos+1;
      release_locked_file(fileno(shared_especialista_pedidos_lock));
      if(!acquire_exclusive_lock(fileno(especialista_masa_rd_lockfile))) fatal_error_abort(FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);
      read_result = read(fileno(especialista_masas_read_end), buffer, DEFAULT_BUFFER_LEN);
      release_locked_file(fileno(especialista_masa_rd_lockfile));
      if(read_result < 0) fatal_error_abort(FATAL_LECTURA, GETLINE_ERROR_EXIT_CODE);
      if(read_result > 0 && strncmp(buffer, MASA_KEYWORD, MASA_KEYWORD_LEN) == 0){
        debug(COCINANDO_PAN, por_entregar_ids[0], TIEMPO_COCCION_PANADERO);
        sleep(TIEMPO_COCCION_PANADERO);
        por_conseguir_masa_madre--;
        por_entregar++;
      } else if(read_result > 0){
        error(ERROR_MENSAJE_NO_COMPRENDIDO, buffer);
      }
    } else {
      if(!acquire_exclusive_lock(fileno(panadero_pipe_rd_lockfile))) fatal_error_abort(FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);
      read_result = read(fileno(panadero_read_end), buffer, DEFAULT_BUFFER_LEN);
      release_locked_file(fileno(panadero_pipe_rd_lockfile));
      if(read_result < 0) fatal_error_abort(FATAL_LECTURA, GETLINE_ERROR_EXIT_CODE);
      if(read_result > 0 && strncmp(buffer, PAN_KEYWORD, PAN_KEYWORD_LEN) == 0){
        int id_actual = atoi(buffer+PAN_KEYWORD_LEN);
        debug(PIDIENDO_MASA_MADRE_PAN, id_actual);
        por_entregar_ids[por_entregar] = id_actual;
        por_conseguir_masa_madre++;
      } else if(read_result > 0){
        error(ERROR_MENSAJE_NO_COMPRENDIDO, buffer);
      }
    }
  } while(read_result || por_entregar);

  info(MAESTRO_PANADERO_STOP, entregadas);

  free_all_resources();
  return 0;
}
