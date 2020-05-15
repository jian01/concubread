#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include "resource_manager.h"
#include "debug_utils.h"
#include "mensajes.h"
#include "lock_utils.h"

#define INITIAL_CAPACITY 100
#define RESIZE_FACTOR 2
#define FATAL_ERROR_EXIT_CODE -9
#define FATAL_ERROR_MESSAGE "Fatal error in resource manager"

typedef struct resource_tracker {
  FILE** opened_files;
  size_t file_capacity;
  size_t file_quantity;
  void** alloqued;
  size_t alloqued_capacity;
  size_t alloqued_quantity;
  pid_t* childs;
  size_t child_capacity;
  size_t child_quantity;
  int errno_backup;
  sigset_t previous_mask;
} resource_tracker_t;

resource_tracker_t* global_resource_tracker = NULL;

void resource_fatal_error(){
  fatal(NULL, FATAL_ERROR_MESSAGE);
  free_all_resources();
  exit(FATAL_ERROR_EXIT_CODE);
}

void backup_errno(){
  if(!global_resource_tracker) return;
  global_resource_tracker->errno_backup = errno;
}

void restore_errno(){
  if(!global_resource_tracker) return;
  errno = global_resource_tracker->errno_backup;
}

bool block_signals(){
  if(!global_resource_tracker) return false;
  sigset_t mask;
  sigfillset(&mask);
  return sigprocmask(SIG_SETMASK, &mask, &global_resource_tracker->previous_mask) == 0;
}

bool restore_signals(){
  if(!global_resource_tracker) return false;
  return sigprocmask(SIG_UNBLOCK, &global_resource_tracker->previous_mask, NULL) == 0;
}

void handle_sigint(int sig){
  fatal(NULL, FATAL_SIGINT);
  free_all_resources();
  exit(0);
}

bool initialize_resource_manager(){
  if(global_resource_tracker) return false;
  global_resource_tracker = malloc(sizeof(resource_tracker_t));
  if(!global_resource_tracker) return false;
  global_resource_tracker->opened_files = malloc(sizeof(FILE*)*INITIAL_CAPACITY);
  if(!global_resource_tracker->opened_files){
    free(global_resource_tracker);
    global_resource_tracker = NULL;
    return false;
  }
  global_resource_tracker->file_capacity = INITIAL_CAPACITY;
  global_resource_tracker->file_quantity = 0;
  global_resource_tracker->alloqued = malloc(sizeof(void*)*INITIAL_CAPACITY);
  if(!global_resource_tracker->alloqued){
    free(global_resource_tracker->opened_files);
    free(global_resource_tracker);
    global_resource_tracker = NULL;
    return false;
  }
  global_resource_tracker->alloqued_capacity = INITIAL_CAPACITY;
  global_resource_tracker->alloqued_quantity = 0;
  global_resource_tracker->childs = malloc(sizeof(pid_t)*INITIAL_CAPACITY);
  if(!global_resource_tracker->childs){
    free(global_resource_tracker->opened_files);
    free(global_resource_tracker->alloqued);
    free(global_resource_tracker);
    global_resource_tracker = NULL;
    return false;
  }
  global_resource_tracker->child_capacity = INITIAL_CAPACITY;
  global_resource_tracker->child_quantity = 0;
  if(signal(SIGINT, handle_sigint) == SIG_ERR){
    free_all_resources();
    global_resource_tracker = NULL;
    return false;
  }
  return true;
}

void* safe_malloc(size_t size){
  block_signals();
  void* memory = malloc(size);
  backup_errno();
  if(!memory){
    if(!restore_signals()) resource_fatal_error();
    restore_errno();
    return NULL;
  }
  if(global_resource_tracker->alloqued_quantity < global_resource_tracker->alloqued_capacity){
    global_resource_tracker->alloqued[global_resource_tracker->alloqued_quantity] = memory;
    global_resource_tracker->alloqued_quantity++;
  } else {
    void** aux_alloqued = realloc(global_resource_tracker->alloqued, RESIZE_FACTOR*global_resource_tracker->alloqued_capacity*sizeof(void*));
    backup_errno();
    if(!aux_alloqued){
      free(memory);
      if(!restore_signals()) resource_fatal_error();
      restore_errno();
      return NULL;
    }
    global_resource_tracker->alloqued_capacity = RESIZE_FACTOR*global_resource_tracker->alloqued_capacity;
    global_resource_tracker->alloqued = aux_alloqued;
    global_resource_tracker->alloqued[global_resource_tracker->alloqued_quantity] = memory;
    global_resource_tracker->alloqued_quantity++;
  }
  if(!restore_signals()) resource_fatal_error();
  return memory;
}

FILE* safe_fopen(const char *filename, const char *mode){
  block_signals();
  FILE* file = fopen(filename, mode);
  backup_errno();
  if(!file){
    if(!restore_signals()) resource_fatal_error();
    restore_errno();
    return NULL;
  }
  if(global_resource_tracker->file_quantity < global_resource_tracker->file_capacity){
    global_resource_tracker->opened_files[global_resource_tracker->file_quantity] = file;
    global_resource_tracker->file_quantity++;
  } else {
    FILE** aux = realloc(global_resource_tracker->opened_files, RESIZE_FACTOR*global_resource_tracker->file_capacity*sizeof(FILE*));
    backup_errno();
    if(!aux){
      if(!fclose(file)) resource_fatal_error();
      if(!restore_signals()) resource_fatal_error();
      restore_errno();
      return NULL;
    }
    global_resource_tracker->file_capacity = RESIZE_FACTOR*global_resource_tracker->file_capacity;
    global_resource_tracker->opened_files = aux;
    global_resource_tracker->opened_files[global_resource_tracker->file_quantity] = file;
    global_resource_tracker->file_quantity++;
  }
  if(!restore_signals()) resource_fatal_error();
  return file;
}

int safe_fclose(FILE *stream){
  block_signals();
  size_t i = 0;
  size_t actual_file_quantity = global_resource_tracker->file_quantity;
  for(;i<actual_file_quantity;i++){
    if(global_resource_tracker->opened_files[i] == stream){
      global_resource_tracker->file_quantity--;
      break;
    }
  }
  i += 1;
  for(;i<actual_file_quantity;i++){
    global_resource_tracker->opened_files[i-1] = global_resource_tracker->opened_files[i];
  }
  if(actual_file_quantity == global_resource_tracker->file_quantity) return -1;
  int result = fclose(stream);
  if(!restore_signals()) resource_fatal_error();
  return result;
}

pid_t safe_fork(){
  block_signals();
  pid_t pid = fork();
  backup_errno();
  if(pid == -1){
    if(!restore_signals()) resource_fatal_error();
    restore_errno();
    return -1;
  }
  if(pid == 0){
    global_resource_tracker->child_quantity = 0;
    if(!restore_signals()) resource_fatal_error();
    return 0;
  }
  if(global_resource_tracker->child_quantity < global_resource_tracker->child_capacity){
    global_resource_tracker->childs[global_resource_tracker->child_quantity] = pid;
    global_resource_tracker->child_quantity++;
  } else {
    pid_t* aux = realloc(global_resource_tracker->childs, RESIZE_FACTOR*global_resource_tracker->child_capacity*sizeof(pid_t));
    backup_errno();
    if(!aux){
      kill(pid, SIGKILL);
      if(!restore_signals()) resource_fatal_error();
      restore_errno();
      resource_fatal_error();
    }
    global_resource_tracker->child_capacity = RESIZE_FACTOR*global_resource_tracker->child_capacity;
    global_resource_tracker->childs = aux;
    global_resource_tracker->childs[global_resource_tracker->child_quantity] = pid;
    global_resource_tracker->child_quantity++;
  }
  if(!restore_signals()) resource_fatal_error();
  return pid;
}

void free_all_resources(){
  if(!global_resource_tracker) return;
  for(size_t i=0; i<global_resource_tracker->child_quantity; i++){
    kill(global_resource_tracker->childs[i], SIGKILL);
  }
  global_resource_tracker->child_quantity = 0;
  for(size_t i=0; i<global_resource_tracker->alloqued_quantity; i++){
    free(global_resource_tracker->alloqued[i]);
  }
  global_resource_tracker->alloqued_quantity = 0;
  for(size_t i=0; i<global_resource_tracker->file_quantity; i++){
    release_locked_file(fileno(global_resource_tracker->opened_files[i]));
    fclose(global_resource_tracker->opened_files[i]);
  }
  global_resource_tracker->file_quantity = 0;
  free(global_resource_tracker->childs);
  free(global_resource_tracker->opened_files);
  free(global_resource_tracker->alloqued);
  free(global_resource_tracker);
  global_resource_tracker = NULL;
  return;
}
