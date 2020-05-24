#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include "lock_utils.h"
#include "resource_manager.h"
#include "mensajes.h"
#include "panic_utils.h"
#include "debug_utils.h"
#include "constants.h"
#include "especialista_masa_madre.h"

#define MALLOC_ERROR_EXIT_CODE -1
#define GETLINE_ERROR_EXIT_CODE -3
#define FREEING_RESOURCE_EXIT_CODE -4
#define LOCK_ERROR_EXIT_CODE -5
#define SIGNAL_SET_ERROR_EXIT_CODE -6
#define CHILD_ERROR_EXIT_STATUS -20
#define FORK_ERROR_EXIT_CODE -12



void alimentar_masa_madre(size_t* masas_madres_por_etapa){
  masas_madres_por_etapa[ETAPAS_MASA_MADRE-1] += masas_madres_por_etapa[ETAPAS_MASA_MADRE-2];
  for(size_t i=ETAPAS_MASA_MADRE-2;i>0;i--){
    masas_madres_por_etapa[i] = masas_madres_por_etapa[i-1];
  }
  masas_madres_por_etapa[0] = 0;
}

int signal_child(pid_t father_pid, size_t masas_por_alimentar, size_t masas_listas){
  debug(ALIMENTANDO_MASA_MADRE, masas_por_alimentar, masas_listas);
  sleep(TIEMPO_ALIMENTAR_MASA_MADRE);
  free_all_resources();
  if(kill(father_pid, SIGALRM) < 0) return -1;
  return 0;
}

int especialista_masa_madre(FILE* pedidos_masa_read_end, FILE* especialista_masas_write_end){
  /*
  Especialista de masa madre

  El especialista de masa madre alimenta la masa periodicamente y a medida le piden masa, si tiene listas las entrega

  Devuelve 0 si fue exitoso, otro numero en caso contrario. Termina exitosamente solo si le piden terminar (SIGUSR1).
  */
  char* buffer = (char *)safe_malloc(DEFAULT_BUFFER_LEN * sizeof(char));
  size_t masas_madres_por_etapa[ETAPAS_MASA_MADRE];
  size_t por_entregar = 0;
  size_t entregadas = 0;
  int read_result = 1;
  if(!buffer) fatal_error_abort(FATAL_MALLOC, MALLOC_ERROR_EXIT_CODE);
  memset(buffer, 0, DEFAULT_BUFFER_LEN);

  void sigalarm_handler(){
    block_signals();
    alimentar_masa_madre(masas_madres_por_etapa);
    if(!restore_signals()) fatal_error_abort(FATAL_SIGNAL_SET, SIGNAL_SET_ERROR_EXIT_CODE);
    debug(MASA_MADRE_ALIMENTADA, masas_madres_por_etapa[ETAPAS_MASA_MADRE-1]);
    pid_t father = getpid();
    pid_t signal_child_pid = safe_fork(false);
    if(signal_child_pid<0) fatal_error_abort(FATAL_FORK, FORK_ERROR_EXIT_CODE);
    if(signal_child_pid == 0){
      if(!restore_signals()) fatal_error_abort(FATAL_SIGNAL_SET, SIGNAL_SET_ERROR_EXIT_CODE);
      signal_child(father, masas_madres_por_etapa[0], masas_madres_por_etapa[ETAPAS_MASA_MADRE-1]);
      exit(0);
    }
  }

  if(sigaction(SIGALRM, &(struct sigaction const){.sa_handler = sigalarm_handler,.sa_flags = SA_NODEFER},0) < 0){
    fatal_error_abort(FATAL_SIGNAL_SET, SIGNAL_SET_ERROR_EXIT_CODE);
  }

  info(INICIANDO_ESPECIALISTA_MASA_MADRE);
  for(size_t i=0;i<ETAPAS_MASA_MADRE;i++) masas_madres_por_etapa[i] = 0;
  masas_madres_por_etapa[0] = MASAS_MADRE_INICIALES;

  // Alimentador masa madre
  pid_t father = getpid();
  pid_t signal_child_pid = safe_fork(false);
  if(signal_child_pid<0) fatal_error_abort(FATAL_FORK, FORK_ERROR_EXIT_CODE);
  if(signal_child_pid == 0){
    return signal_child(father, masas_madres_por_etapa[0], masas_madres_por_etapa[ETAPAS_MASA_MADRE-1]);
  }

  do {
    if(por_entregar > 0 && masas_madres_por_etapa[ETAPAS_MASA_MADRE-1] > 0){
      block_signals();
      int write_result = write(fileno(especialista_masas_write_end), MASA_KEYWORD, DEFAULT_BUFFER_LEN);
      if(!restore_signals()) fatal_error_abort(FATAL_SIGNAL_SET, SIGNAL_SET_ERROR_EXIT_CODE);
      if(write_result > 0){
        debug(ENTREGANDO_MASA_MADRE);
        block_signals();
        por_entregar--;
        entregadas++;
        masas_madres_por_etapa[ETAPAS_MASA_MADRE-1]--;
        if(!restore_signals()) fatal_error_abort(FATAL_SIGNAL_SET, SIGNAL_SET_ERROR_EXIT_CODE);
      } else {
        error(ESPECIALISTA_MASA_MADRE_ERROR_AL_ENTREGAR);
      }
    } else {
      read_result = read(fileno(pedidos_masa_read_end), buffer, DEFAULT_BUFFER_LEN);
      if(read_result < 0){
        if(errno == EINTR) continue;
        fatal_error_abort(FATAL_LECTURA, GETLINE_ERROR_EXIT_CODE);
      }
      if(read_result > 0 && strncmp(buffer, MASA_KEYWORD, MASA_KEYWORD_LEN) == 0){
        debug(ENCOLANDO_MASA_MADRE);
        block_signals();
        masas_madres_por_etapa[0]++;
        por_entregar++;
        if(!restore_signals()) fatal_error_abort(FATAL_SIGNAL_SET, SIGNAL_SET_ERROR_EXIT_CODE);
      } else if(read_result > 0){
        error(ERROR_MENSAJE_NO_COMPRENDIDO, buffer);
      }
    }

  } while(read_result || por_entregar);

  int status = 0;
  pid_t wpid;
  while ((wpid = wait(&status)) > 0){
    if(WEXITSTATUS(status) != 0) fatal_error_abort(FATAL_ERROR_CHILD, CHILD_ERROR_EXIT_STATUS);
  }

  info(ESPECIALISTA_STOP);

  free_all_resources();
  return 0;
}
