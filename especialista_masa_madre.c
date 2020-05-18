#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
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
#define SIGNAL_IGNORE_ERROR_EXIT_CODE -6

void alimentar_masa_madre(size_t* masas_madres_por_etapa){
  sleep(TIEMPO_ALIMENTAR_MASA_MADRE);
  masas_madres_por_etapa[ETAPAS_MASA_MADRE-1] += masas_madres_por_etapa[ETAPAS_MASA_MADRE-2];
  for(size_t i=ETAPAS_MASA_MADRE-2;i>0;i--){
    masas_madres_por_etapa[i] = masas_madres_por_etapa[i-1];
  }
  masas_madres_por_etapa[0] = 0;
}

int especialista_masa_madre(size_t* shared_especialista_pedidos, FILE* shared_especialista_pedidos_lock,
  FILE* especialista_masas_write_end){
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGPIPE);
  if(sigprocmask(SIG_SETMASK, &mask, NULL)) fatal_error_abort(FATAL_ERROR_SIGPIPE_IGNORE, SIGNAL_IGNORE_ERROR_EXIT_CODE);
  char* buffer = (char *)safe_malloc(DEFAULT_BUFFER_LEN * sizeof(char));
  size_t masas_madres_por_etapa[ETAPAS_MASA_MADRE];
  size_t por_entregar = 0;
  size_t entregadas = 0;
  if(!buffer) fatal_error_abort(FATAL_MALLOC, MALLOC_ERROR_EXIT_CODE);
  memset(buffer, 0, DEFAULT_BUFFER_LEN);
  info(INICIANDO_ESPECIALISTA_MASA_MADRE);
  for(size_t i=0;i<ETAPAS_MASA_MADRE;i++) masas_madres_por_etapa[i] = 0;
  masas_madres_por_etapa[0] = MASAS_MADRE_INICIALES;
  while(true){
    debug(ALIMENTANDO_MASA_MADRE, masas_madres_por_etapa[0], masas_madres_por_etapa[ETAPAS_MASA_MADRE-1]);
    alimentar_masa_madre(masas_madres_por_etapa);
    debug(MASA_MADRE_ALIMENTADA, masas_madres_por_etapa[ETAPAS_MASA_MADRE-1]);
    if(masas_madres_por_etapa[ETAPAS_MASA_MADRE-1] > 0){
      int write_result = write(fileno(especialista_masas_write_end), MASA_KEYWORD, DEFAULT_BUFFER_LEN);
      if(write_result < 0 && errno==EPIPE) break;
      if(write_result > 0){
        debug(ENTREGANDO_MASA_MADRE);
        por_entregar--;
        entregadas++;
        masas_madres_por_etapa[ETAPAS_MASA_MADRE-1]--;
      } else {
        error(ESPECIALISTA_MASA_MADRE_ERROR_AL_ENTREGAR);
      }
    }
    if(!acquire_exclusive_lock(fileno(shared_especialista_pedidos_lock))) fatal_error_abort(FATAL_ACQUIRE_LOCK, LOCK_ERROR_EXIT_CODE);
    if(*shared_especialista_pedidos > 0){
      debug(ENCOLANDO_MASA_MADRE);
      masas_madres_por_etapa[0] += *shared_especialista_pedidos;
      por_entregar += *shared_especialista_pedidos;
      *shared_especialista_pedidos = 0;
    }
    release_locked_file(fileno(shared_especialista_pedidos_lock));
  }

  debug(ESPECIALISTA_STOP, entregadas);

  free_all_resources();
  return 0;
}
