#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "resource_manager.h"
#include "panic_utils.h"
#include "lock_utils.h"
#include "debug_utils.h"
#include "constants.h"
#include "mensajes.h"
#include "repartidor.h"
#include "recepcionista.h"

#define MALLOC_ERROR_EXIT_CODE -1
#define GETLINE_ERROR_EXIT_CODE -3
#define FREEING_RESOURCE_EXIT_CODE -4
#define LOCK_ERROR_EXIT_CODE -5


int repartidor(FILE* repartidor_read_end, pedidos_count_t* shared_count, FILE* shared_count_lockfile,
                FILE* repartidor_pipe_rd_lockfile){
  /*
  Repartidor:

  Entrega los envios de los maestros panaderos y pizzeros.
  Termina cuando los maestros pizzeros y panaderos no van a enviar mas comida (exit code 0) o si hubo un error (exit code != 0)
  */
  char *buffer = (char *)safe_malloc(DEFAULT_BUFFER_LEN * sizeof(char));
  size_t pizzas_procesadas = 0;
  size_t panes_procesados = 0;
  if(!buffer) fatal_error_abort(FATAL_MALLOC, MALLOC_ERROR_EXIT_CODE);
  memset(buffer, 0, DEFAULT_BUFFER_LEN);
  info(INICIANDO_REPARTIDOR);
  int read_result = 0;
  do {
    if(!acquire_exclusive_lock(fileno(repartidor_pipe_rd_lockfile))) fatal_error_abort(FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);
    read_result = read(fileno(repartidor_read_end), buffer, DEFAULT_BUFFER_LEN);
    release_locked_file(fileno(repartidor_pipe_rd_lockfile));
    if(read_result < 0) fatal_error_abort(FATAL_LECTURA, GETLINE_ERROR_EXIT_CODE);
    if(read_result > 0 && strncmp(buffer, PAN_KEYWORD, PAN_KEYWORD_LEN) == 0){
      int id_actual = atoi(buffer+PAN_KEYWORD_LEN);
      debug(REPARTIDOR_RECIBIENDO_PAN, id_actual);
      if(!acquire_exclusive_lock(fileno(shared_count_lockfile))) fatal_error_abort(FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);
      shared_count->panes.por_entregar--;
      shared_count->panes.entregados++;
      release_locked_file(fileno(shared_count_lockfile));
      panes_procesados++;
    } else if(read_result > 0 && strncmp(buffer, PIZZA_KEYWORD, PIZZA_KEYWORD_LEN) == 0){
      int id_actual = atoi(buffer+PIZZA_KEYWORD_LEN);
      debug(REPARTIDOR_RECIBIENDO_PIZZA, id_actual);
      if(!acquire_exclusive_lock(fileno(shared_count_lockfile))) fatal_error_abort(FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);
      shared_count->pizzas.por_entregar--;
      shared_count->pizzas.entregados++;
      release_locked_file(fileno(shared_count_lockfile));
      pizzas_procesadas++;
    } else if(read_result > 0){
      error(ERROR_MENSAJE_NO_COMPRENDIDO, buffer);
    }
  } while(read_result);

  info(REPARTIDOR_STOP, pizzas_procesadas, panes_procesados);

  free_all_resources();
  return 0;
}
