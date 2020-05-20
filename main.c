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
#include "constants.h"
#include "debug_utils.h"
#include "panic_utils.h"
#include "mensajes.h"
#include "lock_utils.h"
#include "resource_manager.h"
#include "recepcionista.h"
#include "repartidor.h"
#include "stock_tracking.h"
#include "maestro_pizzero.h"
#include "maestro_panadero.h"
#include "especialista_masa_madre.h"


#define INVALID_PARAMS_EXIT_CODE -1
#define UNABLE_TO_OPEN_DEBUG_FILE_EXIT_CODE -2
#define CHILD_ERROR_EXIT_STATUS -3
#define CHILD_WAIT_ERROR_EXIT_CODE -4
#define RESOURCE_MANAGER_ERROR_EXIT_CODE -9
#define UNABLE_TO_OPEN_INPUT_FILE_EXIT_CODE -6
#define UNABLE_TO_OPEN_PIPE -7
#define UNABLE_TO_CLOSE_PIPE -8
#define SHARED_MEMORY_ERROR -10
#define LOCKFILE_ERROR -11

#define SHARED_MEMORY_FILE1 "/bin/bash"
#define SHARED_MEMORY_FILE2 "/bin/cat"


int gerente(FILE* input_file, int recepcionistas, int pizzeros, int panaderos) {
  info(INICIANDO_GERENTE);

  FILE* repartidor_pipes[2];
  FILE* pizzero_pipes[2];
  FILE* panadero_pipes[2];
  FILE* especialista_masa[2];

  if(fpipe(pizzero_pipes)) fatal_error_abort(FATAL_ERROR_PIPE_OPEN, UNABLE_TO_OPEN_PIPE);
  FILE* pizzero_pipe_rd_lockfile = create_lockfile("lockfiles/pizzero_pipe_rd");
  if(!pizzero_pipe_rd_lockfile) fatal_error_abort(FATAL_ERROR_LOCKFILE, LOCKFILE_ERROR);

  if(fpipe(repartidor_pipes)) fatal_error_abort(FATAL_ERROR_PIPE_OPEN, UNABLE_TO_OPEN_PIPE);
  FILE* repartidor_pipe_rd_lockfile = create_lockfile("lockfiles/repartidor_pipe_rd");
  if(!repartidor_pipe_rd_lockfile) fatal_error_abort(FATAL_ERROR_LOCKFILE, LOCKFILE_ERROR);

  if(fpipe(panadero_pipes)) fatal_error_abort(FATAL_ERROR_PIPE_OPEN, UNABLE_TO_OPEN_PIPE);
  FILE* panadero_pipe_rd_lockfile = create_lockfile("lockfiles/panadero_pipe_rd");
  if(!panadero_pipe_rd_lockfile) fatal_error_abort(FATAL_ERROR_LOCKFILE, LOCKFILE_ERROR);

  if(fpipe(especialista_masa)) fatal_error_abort(FATAL_ERROR_PIPE_OPEN, UNABLE_TO_OPEN_PIPE);
  FILE* especialista_masa_rd_lockfile = create_lockfile("lockfiles/especialista_masa_pipe_rd");
  if(!especialista_masa_rd_lockfile) fatal_error_abort(FATAL_ERROR_LOCKFILE, LOCKFILE_ERROR);

  pedidos_count_t* shared_count = shared_malloc(sizeof(pedidos_count_t), SHARED_MEMORY_FILE1);
  if(!shared_count) fatal_error_abort(FATAL_ERROR_SHARED_MEMORY, SHARED_MEMORY_ERROR);
  memset(shared_count,0,sizeof(pedidos_count_t));
  FILE* shared_count_lockfile = create_lockfile("lockfiles/shared_count");
  if(!shared_count_lockfile) fatal_error_abort(FATAL_ERROR_LOCKFILE, LOCKFILE_ERROR);
  size_t* shared_especialista_pedidos = shared_malloc(sizeof(size_t), SHARED_MEMORY_FILE2);
  if(!shared_especialista_pedidos) fatal_error_abort(FATAL_ERROR_SHARED_MEMORY, SHARED_MEMORY_ERROR);
  *shared_especialista_pedidos = 0;
  FILE* shared_especialista_pedidos_lock = create_lockfile("lockfiles/shared_especialista_pedidos");
  if(!shared_especialista_pedidos_lock) fatal_error_abort(FATAL_ERROR_LOCKFILE, LOCKFILE_ERROR);
  pid_t repartidor_pid;
  pid_t especialista_masa_madre_pid;

  info(CONTRATANDO_ESPECIALISTA_MASA_MADRE);

  // Inicializo especialista masa madre
  especialista_masa_madre_pid = safe_fork();
  if(especialista_masa_madre_pid==0){
    if(safe_fclose(repartidor_pipes[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
    if(safe_fclose(repartidor_pipes[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
    if(safe_fclose(pizzero_pipes[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
    if(safe_fclose(pizzero_pipes[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
    if(safe_fclose(panadero_pipes[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
    if(safe_fclose(panadero_pipes[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
    if(safe_fclose(especialista_masa[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
    return especialista_masa_madre(shared_especialista_pedidos, shared_especialista_pedidos_lock, especialista_masa[1]);
  }

  info(CONTRATANDO_REPARTIDOR);

  // Inicializo repartidor
  repartidor_pid = safe_fork();
  if(repartidor_pid==0){
    if(safe_fclose(repartidor_pipes[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
    if(safe_fclose(pizzero_pipes[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
    if(safe_fclose(pizzero_pipes[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
    if(safe_fclose(panadero_pipes[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
    if(safe_fclose(panadero_pipes[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
    if(safe_fclose(especialista_masa[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
    if(safe_fclose(especialista_masa[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
    return repartidor(repartidor_pipes[0], shared_count, shared_count_lockfile, repartidor_pipe_rd_lockfile);
  }

  info(CONTRATANDO_MAESTROS_PIZZEROS, pizzeros);

  // Comienzo inicializacion de maestros pizzeros
  for(int i=0;i<pizzeros;i++){
    pid_t pid = safe_fork();
    if(pid==0){
      if(safe_fclose(repartidor_pipes[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
      if(safe_fclose(pizzero_pipes[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
      if(safe_fclose(panadero_pipes[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
      if(safe_fclose(panadero_pipes[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
      if(safe_fclose(especialista_masa[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
      return maestro_pizzero(pizzero_pipes[0], repartidor_pipes[1], especialista_masa[0], shared_especialista_pedidos,
      shared_especialista_pedidos_lock, pizzero_pipe_rd_lockfile, especialista_masa_rd_lockfile);
    }
  }

  info(CONTRATANDO_MAESTROS_PANADEROS, panaderos);

  // Comienzo inicializacion de maestros panaderos
  for(int i=0;i<panaderos;i++){
    pid_t pid = safe_fork();
    if(pid==0){
      if(safe_fclose(repartidor_pipes[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
      if(safe_fclose(panadero_pipes[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
      if(safe_fclose(pizzero_pipes[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
      if(safe_fclose(pizzero_pipes[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
      if(safe_fclose(especialista_masa[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
      return maestro_panadero(panadero_pipes[0], repartidor_pipes[1], especialista_masa[0], shared_especialista_pedidos,
      shared_especialista_pedidos_lock, panadero_pipe_rd_lockfile, especialista_masa_rd_lockfile);
    }
  }

  info(CONTRATANDO_RECECPIONISTAS, recepcionistas);

  // Comienzo inicialización de recepcionistas
  for(int i=0;i<recepcionistas;i++){
    pid_t pid = safe_fork();
    if(pid==0){
      if(safe_fclose(repartidor_pipes[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
      if(safe_fclose(repartidor_pipes[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
      if(safe_fclose(pizzero_pipes[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
      if(safe_fclose(panadero_pipes[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
      if(safe_fclose(especialista_masa[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
      if(safe_fclose(especialista_masa[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
      return recepcionista(pizzero_pipes[1], panadero_pipes[1], input_file, shared_count, shared_count_lockfile);
    }
  }
  if(safe_fclose(repartidor_pipes[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
  if(safe_fclose(repartidor_pipes[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
  if(safe_fclose(pizzero_pipes[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
  if(safe_fclose(pizzero_pipes[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
  if(safe_fclose(panadero_pipes[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
  if(safe_fclose(panadero_pipes[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
  if(safe_fclose(especialista_masa[0])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);
  if(safe_fclose(especialista_masa[1])) fatal_error_abort(FATAL_ERROR_PIPE_CLOSE, UNABLE_TO_CLOSE_PIPE);

  int status = 0;
  pid_t wpid;
  while ((wpid = wait(&status)) > 0){
    if(WEXITSTATUS(status) != 0) fatal_error_abort(FATAL_ERROR_CHILD, CHILD_ERROR_EXIT_STATUS);
    if(wpid == repartidor_pid) kill(especialista_masa_madre_pid, SIGUSR1);
  }
  if(errno != ECHILD) fatal_error_abort(FATAL_ERROR_WAIT, CHILD_WAIT_ERROR_EXIT_CODE);

  info(CERRANDO_NEGOCIO);

  free_all_resources();
  return 0;
}


int main(int argc, char * argv[]){
  if(!initialize_resource_manager()) fatal_error_abort(FATAL_RESOURCE_MANAGER, RESOURCE_MANAGER_ERROR_EXIT_CODE);
  FILE* debug_file = NULL;
  char* debug_filename = NULL;
  if(argc == 6){
    debug_filename = argv[5];
  }
  if (argc < 4) fatal_error_abort(FATAL_PARAMETROS_FALTANTES, INVALID_PARAMS_EXIT_CODE);
  int recepcionistas = atoi(argv[1]);
  int pizzeros = atoi(argv[2]);
  int panaderos = atoi(argv[3]);
  if(panaderos < 1 || pizzeros < 1 || recepcionistas < 1) fatal_error_abort(FATAL_CAMPOS_NEGATIVOS, INVALID_PARAMS_EXIT_CODE);
  FILE* input_file = safe_fopen(argv[4], "r+");
  if(!input_file) fatal_error_abort(FATAL_ARCHIVO_ENTRADA, UNABLE_TO_OPEN_INPUT_FILE_EXIT_CODE);
  if(debug_filename){
    debug_file = safe_fopen(debug_filename, "w,ccs=UTF-8");
    if(!debug_file) fatal_error_abort(FATAL_ARCHIVO_LOG, UNABLE_TO_OPEN_DEBUG_FILE_EXIT_CODE);
  }
  set_debug_file(debug_file);
  debug(CANTIDAD_RECEPCIONISTAS, recepcionistas);
  debug(CANTIDAD_PIZZEROS, pizzeros);
  debug(CANTIDAD_PANADEROS, panaderos);
  return gerente(input_file, recepcionistas, pizzeros, panaderos);
}
