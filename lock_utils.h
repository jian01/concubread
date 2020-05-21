#ifndef _LOCK_UTILS
#define _LOCK_UTILS
#include <stdbool.h>

/*
Adquiere el lock exclusivo de un archivo. Devuelve true si pudo adquirirlo, false en caso de error.
*/
bool acquire_exclusive_lock(int filedes);

/*
Adquiere el lock exclusivo de un archivo de forma no bloqueante.

Devuelve true si pudo adquirirlo, false en caso de error.
*/
bool acquire_exclusive_lock_nonblocking(int filedes);

/*
Adquiere el lock no exclusivo de un archivo.

Devuelve true si pudo adquirirlo, false en caso de error.
*/
bool acquire_read_lock(int filedes);

/*
Libera el lock de un archivo, solo falla si el archivo no existe.
*/
void release_locked_file(int filedes);
#endif
