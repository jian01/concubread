#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <string.h>
#include "resource_manager.h"
#include "debug_utils.h"
#include "mensajes.h"
#include "lock_utils.h"

#define INITIAL_CAPACITY 100
#define LOCKFILE_SIZE 100
#define RESIZE_FACTOR 2
#define FATAL_ERROR_EXIT_CODE -9
#define SHARED_MEMORY_INT 0
#define FATAL_ERROR_MESSAGE "Fatal error in resource manager"

typedef struct resource_tracker {
  FILE** opened_files;
  size_t file_capacity;
  size_t file_quantity;
  void** alloqued;
  size_t alloqued_capacity;
  size_t alloqued_quantity;
  void** shared_att;
  size_t shared_att_capacity;
  size_t shared_att_quantity;
  int* shared_mem_ids;
  size_t shared_mem_ids_capacity;
  size_t shared_mem_ids_quantity;
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
  /*
  Las operaciones cuya atomicidad se vean afectadas por señales y sea crucial para un correcto
  funcionamiento garantizar que no deben ser interrumpidas deben llamar antes a la funcion block_signal garantizando se que bloquearan
  todas las señales que estan permitidas bloquear (todas menos SIGKILL y SIGTERM).

  Devuelve true si es exitosa.
  */
  if(!global_resource_tracker) return false;
  sigset_t mask;
  sigfillset(&mask);
  return sigprocmask(SIG_SETMASK, &mask, &global_resource_tracker->previous_mask) == 0;
}

bool restore_signals(){
  /*
  Vuelve las señales al estado previo a la llamada de block_signals, por lo que no se pierden mascaras que se pudieran tener.

  Devuelve true si fue exitosa.
  */
  if(!global_resource_tracker) return false;
  sigdelset(&global_resource_tracker->previous_mask, SIGINT);
  return sigprocmask(SIG_SETMASK, &global_resource_tracker->previous_mask, NULL) == 0;
}

void handle_sigint(int sig){
  /*
  SIGINT handler
  */
  fatal(FATAL_SIGINT);
  free_all_resources();
  exit(0);
}

bool array_append_resize_if_needed(void** array, size_t* capacity, size_t quantity, size_t size_of_element){
  /*
  Redimensiona un arreglo dinamico de ser necesario
  */
  if(*capacity < quantity) return true;
  void* aux = realloc(*array, RESIZE_FACTOR*(*capacity)*size_of_element);
  if(!aux){
    error(L"array_append_resize_if_needed: realloc (%s)", strerror(errno));
    return false;
  }
  *capacity *= RESIZE_FACTOR;
  *array = aux;
  return true;
}

bool register_file_pointer(FILE* file){
  /*
  Registra un FILE*
  */
  if(!file) return false;
  if(!global_resource_tracker) return false;
  if(!array_append_resize_if_needed((void**)&global_resource_tracker->opened_files, &global_resource_tracker->file_capacity,
    global_resource_tracker->file_quantity, sizeof(FILE*))) return false;
  global_resource_tracker->opened_files[global_resource_tracker->file_quantity] = file;
  global_resource_tracker->file_quantity++;
  return true;
}

bool initialize_resource_manager(){
  /*
  Inicializa los recursos necesarios por el resource manager, reservado su memoria y registrando el handler de SIGINT

  Inicializar el resource manager implica que por lo menos todos los forks se deben realizar utilizandolo.
  */
  block_signals();
  if(global_resource_tracker) return false;
  global_resource_tracker = malloc(sizeof(resource_tracker_t));
  if(!global_resource_tracker) return false;
  global_resource_tracker->opened_files = malloc(sizeof(FILE*)*INITIAL_CAPACITY);
  if(!global_resource_tracker->opened_files){
    free(global_resource_tracker);
    global_resource_tracker = NULL;
    if(!restore_signals()) resource_fatal_error();
    return false;
  }
  global_resource_tracker->file_capacity = INITIAL_CAPACITY;
  global_resource_tracker->file_quantity = 0;
  global_resource_tracker->alloqued = malloc(sizeof(void*)*INITIAL_CAPACITY);
  if(!global_resource_tracker->alloqued){
    free(global_resource_tracker->opened_files);
    free(global_resource_tracker);
    global_resource_tracker = NULL;
    if(!restore_signals()) resource_fatal_error();
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
    if(!restore_signals()) resource_fatal_error();
    return false;
  }
  global_resource_tracker->child_capacity = INITIAL_CAPACITY;
  global_resource_tracker->child_quantity = 0;

  global_resource_tracker->shared_att = malloc(sizeof(void*)*INITIAL_CAPACITY);
  if(!global_resource_tracker->shared_att){
    free(global_resource_tracker->childs);
    free(global_resource_tracker->opened_files);
    free(global_resource_tracker->alloqued);
    free(global_resource_tracker);
    global_resource_tracker = NULL;
    if(!restore_signals()) resource_fatal_error();
    return false;
  }
  global_resource_tracker->shared_att_capacity = INITIAL_CAPACITY;
  global_resource_tracker->shared_att_quantity = 0;

  global_resource_tracker->shared_mem_ids = malloc(sizeof(int)*INITIAL_CAPACITY);
  if(!global_resource_tracker->childs){
    free(global_resource_tracker->shared_att);
    free(global_resource_tracker->childs);
    free(global_resource_tracker->opened_files);
    free(global_resource_tracker->alloqued);
    free(global_resource_tracker);
    global_resource_tracker = NULL;
    if(!restore_signals()) resource_fatal_error();
    return false;
  }
  global_resource_tracker->shared_mem_ids_capacity = INITIAL_CAPACITY;
  global_resource_tracker->shared_mem_ids_quantity = 0;

  if(signal(SIGINT, handle_sigint) == SIG_ERR){
    free_all_resources();
    global_resource_tracker = NULL;
    return false;
    if(!restore_signals()) resource_fatal_error();
  }
  if(!restore_signals()) resource_fatal_error();
  return true;
}

void* safe_malloc(size_t size){
  /*
  Wrapper de malloc que aloca la memoria pero ademas registra

  Devuelve un puntero a la memoria solicitada o NULL en caso de error.
  No se garantiza que errno cumpla las mismas condiciones que malloc.
  No tiene un free, debe ser liberada con free_all_resources o
  en caso de querer liberar memoria sin liberar todo, usar directamente la syscall malloc.
  */
  if(!global_resource_tracker) return NULL;
  block_signals();
  void* memory = malloc(size);
  backup_errno();
  if(!memory){
    error(L"safe_malloc: malloc (%s)", strerror(errno));
    if(!restore_signals()) resource_fatal_error();
    restore_errno();
    return NULL;
  }
  if(!array_append_resize_if_needed((void**)&global_resource_tracker->alloqued, &global_resource_tracker->alloqued_capacity,
    global_resource_tracker->alloqued_quantity, sizeof(void*))){
      free(memory);
      if(!restore_signals()) resource_fatal_error();
      return NULL;
    }
  global_resource_tracker->alloqued[global_resource_tracker->alloqued_quantity] = memory;
  global_resource_tracker->alloqued_quantity++;
  if(!restore_signals()) resource_fatal_error();
  return memory;
}

void* shared_malloc(size_t size, const char* shared_memory_file){
  /*
  Función análoga a malloc pero para memoria compartida

  Devuelve un puntero a la memoria solicitada o NULL en caso de error.
  La unica forma correcta de liberar estos recursos es con free_all_resources
  */
  if(!global_resource_tracker) return NULL;
  block_signals();
  key_t clave = ftok(shared_memory_file, SHARED_MEMORY_INT);
  if(clave == -1){
  error(L"shared_malloc: ftok (%s)", strerror(errno));
    return NULL;
  }
  int shmId = shmget(clave, size, 0644|IPC_CREAT);
  if(shmId == -1){
    error(L"shared_malloc: shmget (%s)", strerror(errno));
    return NULL;
  }
  void* ptr = shmat(shmId,NULL,0);
  if ( ptr == (void *) -1 ) {
    error(L"shared_malloc: shmat (%s)", strerror(errno));
    shmctl(shmId, IPC_RMID, NULL);
    return NULL;
  }

  if(!array_append_resize_if_needed((void**)&global_resource_tracker->shared_att, &global_resource_tracker->shared_att_capacity,
    global_resource_tracker->shared_att_quantity, sizeof(void*))){
      shmdt((void*)ptr);
      shmctl(shmId, IPC_RMID, NULL);
      resource_fatal_error();
    }
  global_resource_tracker->shared_att[global_resource_tracker->shared_att_quantity] = ptr;
  global_resource_tracker->shared_att_quantity++;

  if(!array_append_resize_if_needed((void**)&global_resource_tracker->shared_mem_ids, &global_resource_tracker->shared_mem_ids_capacity,
    global_resource_tracker->shared_mem_ids_quantity, sizeof(int))){
      shmctl(shmId, IPC_RMID, NULL);
      resource_fatal_error();
    }
  global_resource_tracker->shared_mem_ids[global_resource_tracker->shared_mem_ids_quantity] = shmId;
  global_resource_tracker->shared_mem_ids_quantity++;

  if(!restore_signals()) resource_fatal_error();
  return ptr;
}

FILE* safe_fopen(const char *filename, const char *mode){
  /*
  Wrapper de fopen para abrir archivos

  Devuelve NULL en caso de error, un FILE* si fue exitosa.
  se garantiza que errno cumpla las mismas condiciones que fopen.
  Se pueden cerrar con safe_fclose
  */
  if(!global_resource_tracker) return NULL;
  block_signals();
  FILE* file = fopen(filename, mode);
  backup_errno();
  if(!file){
    if(!restore_signals()) resource_fatal_error();
    restore_errno();
    return NULL;
  }
  if(!register_file_pointer(file)){
    if(!fclose(file)) resource_fatal_error();
    if(!restore_signals()) resource_fatal_error();
    return NULL;
  }
  if(!restore_signals()) resource_fatal_error();
  return file;
}

int fpipe(FILE* pipefile[2]){
  if(!pipefile) return -1;
  block_signals();
  int pipefd[2];
  int pipe_result = pipe(pipefd);
  backup_errno();
  if(pipe_result != 0){
    if(!restore_signals()) resource_fatal_error();
    restore_errno();
    return -1;
  }
  FILE* read = fdopen(pipefd[0], "r");
  if(!read){
    error(L"fpipe: read fdopen (%s)", strerror(errno));
    close(pipefd[0]);
    close(pipefd[1]);
    if(!restore_signals()) resource_fatal_error();
    return -1;
  }
  if(!register_file_pointer(read)){
    fclose(read);
    close(pipefd[1]);
    if(!restore_signals()) resource_fatal_error();
    return -1;
  }
  FILE* write = fdopen(pipefd[1], "w");
  if(!write){
    error(L"fpipe: write fdopen (%s)", strerror(errno));
    fclose(read);
    close(pipefd[1]);
    if(!restore_signals()) resource_fatal_error();
    return -1;
  }
  if(!register_file_pointer(write)){
    fclose(read);
    fclose(write);
    if(!restore_signals()) resource_fatal_error();
    return -1;
  }
  pipefile[0] = read;
  pipefile[1] = write;
  if(!restore_signals()) resource_fatal_error();
  return 0;
}

FILE* create_lockfile(const char* name){
  block_signals();
  int lockfile_fd = open(name,O_CREAT|O_WRONLY,0777);
  if(lockfile_fd < 0){
    error(L"create_lockfile: open (%s)", strerror(errno));
    if(!restore_signals()) resource_fatal_error();
    return NULL;
  }
  FILE* lockfile = fdopen(lockfile_fd, "w");
  if(!lockfile){
    error(L"create_lockfile: fdopen (%s)", strerror(errno));
    close(lockfile_fd);
    if(!restore_signals()) resource_fatal_error();
    return NULL;
  }
  if(!register_file_pointer(lockfile)){
    fclose(lockfile);
    if(!restore_signals()) resource_fatal_error();
    return NULL;
  }
  if(!restore_signals()) resource_fatal_error();
  return lockfile;
}

int safe_fclose(FILE *stream){
  /*
  Libera cualquiera de los FILE* creados con el resource manager, no asi si fueron creados en otro lado

  Devuelve -1 si fallo, 0 sino, no garantiza que errno cumpla lo mismo que fclose.
  */
  if(!global_resource_tracker) return -1;
  block_signals();
  size_t i = 0;
  size_t actual_file_quantity = global_resource_tracker->file_quantity;
  for(;i<actual_file_quantity;i++){
    if(global_resource_tracker->opened_files[i] == stream){
      global_resource_tracker->file_quantity--;
      break;
    }
  }
  for(;i<actual_file_quantity;i++){
    global_resource_tracker->opened_files[i] = global_resource_tracker->opened_files[i+1];
  }
  if(actual_file_quantity == global_resource_tracker->file_quantity) return -1;
  int result = fclose(stream);
  if(!restore_signals()) resource_fatal_error();
  return result;
}

pid_t safe_fork(bool attach){
  /*
  Wrapper de fork que registra los procesos hijos

  Devuelve lo mismo que fork, no garantiza setear el errno como fork lo haria.
  Si attach es true:
    Los procesos se registran para ser cerrados y liberados automaticamente con free_all_resources, por lo que
    un proceso hijo creado con esta llamada no puede sobrevivir al padre.
  Si attach es false:
    Permite que quien lo llame garantice el fin del proceso hijo, teniendo en cuenta que si no lo hace quedara corriendo
  */
  if(!global_resource_tracker) return -1;
  block_signals();
  pid_t pid = fork();
  backup_errno();
  if(pid == -1){
    error(L"safe_fork: fork (%s)", strerror(errno));
    if(!restore_signals()) resource_fatal_error();
    restore_errno();
    return -1;
  }
  if(pid == 0){
    global_resource_tracker->child_quantity = 0;
    global_resource_tracker->shared_mem_ids_quantity = 0;
    if(signal(SIGINT, handle_sigint) == SIG_ERR) resource_fatal_error();
    if(!restore_signals()) resource_fatal_error();
    return 0;
  }
  if(attach){
    if(!array_append_resize_if_needed((void**)&global_resource_tracker->childs, &global_resource_tracker->child_capacity,
      global_resource_tracker->child_quantity, sizeof(int))){
        kill(pid, SIGKILL);
        if(!restore_signals()) resource_fatal_error();
        resource_fatal_error();
      }
    global_resource_tracker->childs[global_resource_tracker->child_quantity] = pid;
    global_resource_tracker->child_quantity++;
  }
  if(!restore_signals()) resource_fatal_error();
  return pid;
}

void free_all_resources(){
  /*
  Libera todos los recursos creados con el resource manager

  1. Mata a todos los hijos abiertos, liberando sus recursos registrados en el resource manager.
  Esto lo hace para todos los hijos si se inicializo el resource manager alguna vez, sin importar si fueron creados con safe_fork o fork.

  2. Libera todo lo que fuera alocado con safe_malloc

  3. Cierra todos los archivos, pipes o lockfiles que fueran abiertos con el resource manager pero no liberados con safe_fclose.

  4. Libera toda la memoria compartida alguna vez creada

  5. Libera todos los recursos asociados al resource manager
  */
  if(!global_resource_tracker) return;
  block_signals();
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
  for(size_t i=0; i<global_resource_tracker->shared_att_quantity; i++){
    shmdt((void*)global_resource_tracker->shared_att[i]);
  }
  global_resource_tracker->shared_att_quantity = 0;
  for(size_t i=0; i<global_resource_tracker->shared_mem_ids_quantity; i++){
    shmctl(global_resource_tracker->shared_mem_ids[i], IPC_RMID, NULL);
  }
  global_resource_tracker->shared_mem_ids_quantity = 0;
  free(global_resource_tracker->childs);
  free(global_resource_tracker->opened_files);
  free(global_resource_tracker->alloqued);
  free(global_resource_tracker->shared_att);
  free(global_resource_tracker->shared_mem_ids);
  free(global_resource_tracker);
  global_resource_tracker = NULL;
  restore_signals();
  return;
}
