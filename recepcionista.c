#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "lock_utils.h"
#include "recepcionista.h"
#include "debug_utils.h"
#include "mensajes.h"
#include "resource_manager.h"

#define PIZZA_KEYWORD "pizza"
#define PAN_KEYWORD "pan"
#define MAXIMUM_BUFFER_LEN 10
#define MALLOC_ERROR_EXIT_CODE -1
#define GETLINE_ERROR_EXIT_CODE -3
#define FREEING_RESOURCE_EXIT_CODE -4
#define LOCK_ERROR_EXIT_CODE -5


void fatal_error_abort(FILE* debug_file, const char* message, int exit_code);


int recepcionista(FILE* input_file, FILE* debug_file){
  char *buffer = (char *)safe_malloc(MAXIMUM_BUFFER_LEN * sizeof(char));
  if(!buffer) fatal_error_abort(debug_file, FATAL_MALLOC, MALLOC_ERROR_EXIT_CODE);
  info(debug_file, INICIADO_RECEPCIONISTA);
  do {
    if(!acquire_write_lock(fileno(input_file))) fatal_error_abort(debug_file, FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);
    fseek(input_file, 0, SEEK_CUR);
    buffer = fgets(buffer, MAXIMUM_BUFFER_LEN, input_file);
    fflush(input_file);
    release_locked_file(fileno(input_file));
    if(!buffer && errno != 0) fatal_error_abort(debug_file, FATAL_LECTURA_RECEPCIONISTA, GETLINE_ERROR_EXIT_CODE);
    if(buffer && strncmp(buffer, PIZZA_KEYWORD, 5) == 0){
      debug(debug_file, RECEPCIONISTA_PIZZA_PEDIDA);
      sleep(1);
    } else if(buffer && strncmp(buffer, PAN_KEYWORD, 3) == 0){
      debug(debug_file, RECEPCIONISTA_PAN_PEDIDO);
      sleep(1);
    } else if(buffer){
      error(debug_file, ERROR_PEDIDO_NO_COMPRENDIDO, buffer);
    }
  } while(buffer);

  debug(debug_file, RECEPCIONISTA_STOP);

  free_all_resources();
  return 0;
}
