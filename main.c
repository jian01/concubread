#include <stdio.h>
#include <wchar.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/wait.h>
#include "debug_utils.h"
#include "mensajes.h"
#include "recepcionista.h"
#include "lock_utils.h"
#include "resource_manager.h"


#define INVALID_PARAMS_EXIT_CODE -1
#define UNABLE_TO_OPEN_DEBUG_FILE_EXIT_CODE -2
#define CHILD_ERROR_EXIT_STATUS -3
#define CHILD_WAIT_ERROR_EXIT_CODE -4
#define RESOURCE_MANAGER_ERROR_EXIT_CODE -9

void fatal_error_abort(FILE* debug_file, const char* message, int exit_code){
  fatal(debug_file, message);
  free_all_resources();
  _exit(exit_code);
}


int main(int argc, char * argv[]) {
  if(!initialize_resource_manager()) fatal_error_abort(NULL, FATAL_RESOURCE_MANAGER, RESOURCE_MANAGER_ERROR_EXIT_CODE);
  FILE* debug_file = NULL;
  char* debug_filename = NULL;
  if(argc == 6){
    debug_filename = argv[5];
  }
  if (argc < 4) fatal_error_abort(debug_file, FATAL_PARAMETROS_FALTANTES, INVALID_PARAMS_EXIT_CODE);
  int recepcionistas = atoi(argv[1]);
  int pizzeros = atoi(argv[2]);
  int panaderos = atoi(argv[3]);
  if(panaderos < 1 || pizzeros < 1 || recepcionistas < 1) fatal_error_abort(debug_file, FATAL_CAMPOS_NEGATIVOS, INVALID_PARAMS_EXIT_CODE);

  if(debug_filename){
    debug_file = safe_fopen(debug_filename, "we,ccs=UTF-8");
    if(!debug_file) fatal_error_abort(NULL, FATAL_ARCHIVO_LOG, UNABLE_TO_OPEN_DEBUG_FILE_EXIT_CODE);
  }
  if(debug_file){
    debug(debug_file, CANTIDAD_RECEPCIONISTAS, recepcionistas);
    debug(debug_file, CANTIDAD_PIZZEROS, pizzeros);
    debug(debug_file, CANTIDAD_PANADEROS, panaderos);
  }
  // Comienzo inicialización de recepcionistas
  for(int i=0;i<recepcionistas;i++){
    safe_fclose(debug_file);
    pid_t pid = safe_fork();
    if(pid==0){
      return recepcionista(argv[4], debug_filename);
    }
  }

  if(debug_filename){
    debug_file = safe_fopen(debug_filename, "ae,ccs=UTF-8");
    if(!debug_file) fatal_error_abort(NULL, FATAL_ARCHIVO_LOG, UNABLE_TO_OPEN_DEBUG_FILE_EXIT_CODE);
  }

  int status = 0;
  pid_t wpid;
  while ((wpid = wait(&status)) > 0){
    if(WEXITSTATUS(status) != 0) fatal_error_abort(debug_file, FATAL_ERROR_CHILD, CHILD_ERROR_EXIT_STATUS);
  }
  if(errno != ECHILD) fatal_error_abort(debug_file, FATAL_ERROR_WAIT, CHILD_WAIT_ERROR_EXIT_CODE);

  free_all_resources();
  return 0;
}
