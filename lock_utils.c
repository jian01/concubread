#include "lock_utils.h"
#include <unistd.h>
#include <fcntl.h>

bool acquire_exclusive_lock(int filedes){
  /*
  Adquiere el lock exclusivo de un archivo. Devuelve true si pudo adquirirlo, false en caso de error.
  */
  struct flock write_lock;
  write_lock.l_type = F_WRLCK;
  write_lock.l_whence = SEEK_SET;
  write_lock.l_len = 0;
  write_lock.l_start = 0;
  int code = fcntl(filedes, F_SETLKW, &write_lock);
  if(code!=0) return false;
  return true;
}

bool acquire_exclusive_lock_nonblocking(int filedes){
  /*
  Adquiere el lock exclusivo de un archivo de forma no bloqueante.

  Devuelve true si pudo adquirirlo, false en caso de error.
  */
  struct flock write_lock;
  write_lock.l_type = F_WRLCK;
  write_lock.l_whence = SEEK_SET;
  write_lock.l_len = 0;
  write_lock.l_start = 0;
  return fcntl(filedes, F_SETLK, &write_lock)==0;
}

bool acquire_read_lock(int filedes){
  /*
  Adquiere el lock no exclusivo de un archivo.

  Devuelve true si pudo adquirirlo, false en caso de error.
  */
  struct flock read_lock;
  read_lock.l_type = F_RDLCK;
  read_lock.l_whence = SEEK_SET;
  read_lock.l_len = 0;
  read_lock.l_start = 0;
  int code = fcntl(filedes, F_SETLKW, &read_lock);
  if(code!=0) return false;
  return true;
}

void release_locked_file(int filedes){
  /*
  Libera el lock de un archivo, solo falla si el archivo no existe.
  */
  struct flock unlock;
  unlock.l_type = F_UNLCK;
  unlock.l_whence = SEEK_SET;
  unlock.l_len = 0;
  unlock.l_start = 0;
  fcntl(filedes, F_SETLK, &unlock);
}
