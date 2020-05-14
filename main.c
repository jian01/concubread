#include <stdio.h>
#include <wchar.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "debug_utils.h"
#include "mensajes_log.h"
#include "recepcionista.h"
#include "lock_utils.h"

#define INVALID_PARAMS_EXIT_CODE 1
#define UNABLE_TO_OPEN_INPUT_FILE 2
#define UNABLE_TO_OPEN_DEBUG_FILE_EXIT_CODE 3

int main(int argc, char * argv[]) {
  FILE* debug_file = stderr;
  if (argc < 4){
    fprintf(stderr, "%s\n", ERROR_PARAMETROS_FALTANTES);
    exit(INVALID_PARAMS_EXIT_CODE);
  }
  int recepcionistas = atoi(argv[1]);
  int pizzeros = atoi(argv[2]);
  int panaderos = atoi(argv[3]);
  if(panaderos < 1 || pizzeros < 1 || recepcionistas < 1){
    fprintf(stderr, "%s\n", ERROR_CAMPOS_NEGATIVOS);
    exit(INVALID_PARAMS_EXIT_CODE);
  }
  FILE* input_file = fopen(argv[4], "r+");
  if(!input_file){
    fprintf(stderr, "%s\n", ERROR_ARCHIVO_ENTRADA); // TODO: salida elegante
    exit(UNABLE_TO_OPEN_INPUT_FILE);
  }
  if(argc == 6){
    debug_file = fopen(argv[5], "w,ccs=UTF-8");
    if(!debug_file){
      fprintf(stderr, "%s\n", ERROR_ARCHIVO_LOG); // TODO: salida elegante
      exit(UNABLE_TO_OPEN_DEBUG_FILE_EXIT_CODE);
    }
  }
  if(debug_file){
    print_debug(debug_file, CANTIDAD_RECEPCIONISTAS, recepcionistas);
    print_debug(debug_file, CANTIDAD_PIZZEROS, pizzeros);
    print_debug(debug_file, CANTIDAD_PANADEROS, panaderos);
  }

  // Comienzo inicialización de recepcionistas
  for(int i=0;i<recepcionistas;i++){
    pid_t pid = fork();
    if(pid==0){
      return recepcionista(input_file, debug_file);
    }
    // TODO: acordarme los pids en caso de tener que abortar
  }

  // TODO: waits lindos

  if(debug_file){
    if(!fclose(debug_file)) exit(errno); // TODO: salida elegante
  }
  return 0;
}
