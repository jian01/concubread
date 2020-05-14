#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "lock_utils.h"
#include "recepcionista.h"
#include "debug_utils.h"
#include "mensajes_log.h"

#define PIZZA_KEYWORD "pizza"
#define PAN_KEYWORD "pan"
#define MAXIMUM_BUFFER_LEN 10
#define MALLOC_ERROR_EXIT_CODE 1
#define GETLINE_ERROR_EXIT_CODE 2
#define FREEING_RESOURCE_EXIT_CODE 3
#define LOCK_ERROR_EXIT_CODE 4

int recepcionista(FILE* input_file, FILE* debug_file){
  char *buffer = (char *)malloc(MAXIMUM_BUFFER_LEN * sizeof(char));
  if(!buffer){
    print_error(debug_file, ERROR_MALLOC, (long)getpid());
    return MALLOC_ERROR_EXIT_CODE; // TODO: salida correcta
  }
  print_info(debug_file, INICIADO_RECEPCIONISTA, (long)getpid());
  do {
    if(!acquire_write_lock(fileno(input_file))) return LOCK_ERROR_EXIT_CODE; // TODO: salida correcta
    fseek(input_file, 0, SEEK_CUR);
    buffer = fgets(buffer, MAXIMUM_BUFFER_LEN, input_file);
    fflush(input_file);
    release_locked_file(fileno(input_file));
    if(!buffer && errno != 0){
      print_error(debug_file, ERROR_LECTURA_RECEPCIONISTA, (long)getpid());
      return GETLINE_ERROR_EXIT_CODE; // TODO: salida correcta
    }
    if(buffer && strncmp(buffer, PIZZA_KEYWORD, 5) == 0){
      print_debug(debug_file, RECEPCIONISTA_PIZZA_PEDIDA, (long)getpid());
    } else if(buffer && strncmp(buffer, PAN_KEYWORD, 3) == 0){
      print_debug(debug_file, RECEPCIONISTA_PAN_PEDIDO, (long)getpid());
    } else if(buffer){
      print_error(debug_file, ERROR_PEDIDO_NO_COMPRENDIDO, buffer, (long)getpid());
    }
  } while(buffer);

  print_debug(debug_file, RECEPCIONISTA_STOP, (long)getpid());

  if(debug_file){
    if(!fclose(debug_file)) return FREEING_RESOURCE_EXIT_CODE; // TODO: salida elegante
  }

  if(!fclose(input_file)) return FREEING_RESOURCE_EXIT_CODE; // TODO: salida elegante

  free(buffer);
  return 0;
}
