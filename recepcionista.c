#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include "lock_utils.h"
#include "debug_utils.h"
#include "panic_utils.h"
#include "mensajes.h"
#include "resource_manager.h"
#include "constants.h"
#include "recepcionista.h"

#define MALLOC_ERROR_EXIT_CODE -1
#define GETLINE_ERROR_EXIT_CODE -3
#define FREEING_RESOURCE_EXIT_CODE -4
#define LOCK_ERROR_EXIT_CODE -5


int recepcionista(FILE* pizzero_write_end, FILE* panadero_write_end, FILE* input_file,
                  pedidos_count_t* shared_count, FILE* shared_count_lockfile){

  char *buffer = (char *)safe_malloc(DEFAULT_BUFFER_LEN * sizeof(char));
  char* aux_buffer;
  size_t pizzas_procesadas = 0;
  size_t panes_procesados = 0;
  if(!buffer) fatal_error_abort(FATAL_MALLOC, MALLOC_ERROR_EXIT_CODE);
  memset(buffer, 0, DEFAULT_BUFFER_LEN);
  info(INICIADO_RECEPCIONISTA);
  do {
    aux_buffer = NULL;
    if(!acquire_exclusive_lock(fileno(input_file))) fatal_error_abort(FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);
    fseek(input_file, 0, SEEK_CUR);
    aux_buffer = fgets(buffer, DEFAULT_BUFFER_LEN, input_file);
    if(!aux_buffer && errno != 0) fatal_error_abort(FATAL_LECTURA, GETLINE_ERROR_EXIT_CODE);
    fflush(input_file);
    release_locked_file(fileno(input_file));
    if(aux_buffer){
      if(aux_buffer && strncmp(buffer, PIZZA_KEYWORD, PIZZA_KEYWORD_LEN) == 0){
        debug(RECEPCIONISTA_PIZZA_PEDIDA);
        if(!acquire_exclusive_lock(fileno(shared_count_lockfile))) fatal_error_abort(FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);
        shared_count->pizzas.por_ordenar++;
        release_locked_file(fileno(shared_count_lockfile));
      } else if(aux_buffer && strncmp(buffer, PAN_KEYWORD, PAN_KEYWORD_LEN) == 0){
        debug(RECEPCIONISTA_PAN_PEDIDO);
        if(!acquire_exclusive_lock(fileno(shared_count_lockfile))) fatal_error_abort(FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);
        shared_count->panes.por_ordenar++;
        release_locked_file(fileno(shared_count_lockfile));
      } else if(aux_buffer){
        error(ERROR_MENSAJE_NO_COMPRENDIDO, buffer);
      }
    }
    if(!acquire_exclusive_lock(fileno(shared_count_lockfile))) fatal_error_abort(FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);
    if(shared_count->pizzas.por_ordenar > 0){
      if(sprintf(buffer, "%s%d", PIZZA_KEYWORD, shared_count->cant_pedidos) < 0) fatal_error_abort(FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);

      shared_count->pizzas.por_ordenar--;
      int pedido_actual = shared_count->cant_pedidos;
      shared_count->cant_pedidos++;
      release_locked_file(fileno(shared_count_lockfile));
      int write_result = write(fileno(pizzero_write_end), buffer, DEFAULT_BUFFER_LEN);
      if(!acquire_exclusive_lock(fileno(shared_count_lockfile))) fatal_error_abort(FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);
      shared_count->pizzas.por_ordenar++;

      if(write_result > 0){
        debug(RECEPCIONISTA_PIDIENDO_PIZZA, pedido_actual);
        shared_count->pizzas.por_ordenar--;
        shared_count->pizzas.por_entregar++;
        pizzas_procesadas++;
      } else {
        error(RECEPCIONISTA_ERROR_PROCESAR_ORDEN, pedido_actual);
      }
    }
    release_locked_file(fileno(shared_count_lockfile));
    if(!acquire_exclusive_lock(fileno(shared_count_lockfile))) fatal_error_abort(FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);
    if(shared_count->panes.por_ordenar > 0){
      if(sprintf(buffer, "%s%d", PAN_KEYWORD, shared_count->cant_pedidos) < 0) fatal_error_abort(FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);

      shared_count->panes.por_ordenar--;
      int pedido_actual = shared_count->cant_pedidos;
      shared_count->cant_pedidos++;
      release_locked_file(fileno(shared_count_lockfile));
      int write_result = write(fileno(panadero_write_end), buffer, DEFAULT_BUFFER_LEN);
      if(!acquire_exclusive_lock(fileno(shared_count_lockfile))) fatal_error_abort(FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);
      shared_count->panes.por_ordenar++;

      if(write_result > 0){
        debug(RECEPCIONISTA_PIDIENDO_PAN, pedido_actual);
        shared_count->panes.por_ordenar--;
        shared_count->panes.por_entregar++;
        panes_procesados++;
      } else {
        error(RECEPCIONISTA_ERROR_PROCESAR_ORDEN, pedido_actual);
      }
    }
    release_locked_file(fileno(shared_count_lockfile));
  } while(aux_buffer || shared_count->pizzas.por_ordenar || shared_count->pizzas.por_ordenar);
  debug(RECEPCIONISTA_STOP, pizzas_procesadas, panes_procesados);

  free_all_resources();
  return 0;
}
