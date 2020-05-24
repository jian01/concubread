#ifndef _RESOURCE_MANAGER
#define _RESOURCE_MANAGER
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*
El resource manager es el encargado de alocar recursos, interactuar con archivos y crear procesos de forma "segura".
Esto implica que se conserva un registro dinamico de todos los recursos solicitados para liberarlos todos
juntos cuando asi se solicite (ej: finalizacion del proceso). Esto facilita el uso de recursos compartidos y la concurrencia.

Todo recurso o archivo solicitado por esta API debe ser o bien liberado con sus propia API o esperar a liberarlo con free_all_resources al finalizar el proceso.

El resource manager tambien garantiza que todos los procesos respondan de forma agradable al SIGINT, liberando todos su recursos.
*/

/*
Inicializa los recursos necesarios por el resource manager, reservado su memoria y registrando el handler de SIGINT

Inicializar el resource manager implica que por lo menos todos los forks se deben realizar utilizandolo.
*/
bool initialize_resource_manager();

/*
Wrapper de malloc que aloca la memoria pero ademas registra

Devuelve un puntero a la memoria solicitada o NULL en caso de error.
No se garantiza que errno cumpla las mismas condiciones que malloc.
No tiene un free, debe ser liberada con free_all_resources o
en caso de querer liberar memoria sin liberar todo, usar directamente la syscall malloc.
*/
void* safe_malloc(size_t size);

/*
Función análoga a malloc pero para memoria compartida

Devuelve un puntero a la memoria solicitada o NULL en caso de error.
La unica forma correcta de liberar estos recursos es con free_all_resources
*/
void* shared_malloc(size_t size, const char *shared_memory_file);

/*
Wrapper de fopen para abrir archivos

Devuelve NULL en caso de error, un FILE* si fue exitosa.
se garantiza que errno cumpla las mismas condiciones que fopen.
Se pueden cerrar con safe_fclose
*/
FILE* safe_fopen(const char *filename, const char *mode);

/*
Crea un pipe y devuelve los FILE* asociados a sus extremos

Devuelve 0 si fue exitosa, negativo en otro caso.
No se garantiza que errno cumpla lo mismo que la syscall pipe
Se puede liberar cada file* con safe_fclose
*/
int fpipe(FILE* pipefile[2]);

/*
Crea un lockfile con el archivo "name"

Devuelve NULL si fallo. Se puede liberar con safe_fclose
*/
FILE* create_lockfile(const char* name);

/*
Libera cualquiera de los FILE* creados con el resource manager, no asi si fueron creados en otro lado

Devuelve -1 si fallo, 0 sino, no garantiza que errno cumpla lo mismo que fclose.
*/
int safe_fclose(FILE *stream);

/*
Wrapper de fork que registra los procesos hijos

Devuelve lo mismo que fork, no garantiza setear el errno como fork lo haria.
Si attach es true:
  Los procesos se registran para ser cerrados y liberados automaticamente con free_all_resources, por lo que
  un proceso hijo creado con esta llamada no puede sobrevivir al padre.
Si attach es false:
  Permite que quien lo llame garantice el fin del proceso hijo, teniendo en cuenta que si no lo hace quedara corriendo
*/
pid_t safe_fork(bool attach);

/*
Libera todos los recursos creados con el resource manager

1. Mata a todos los hijos abiertos, liberando sus recursos registrados en el resource manager.
Esto lo hace para todos los hijos si se inicializo el resource manager alguna vez, sin importar si fueron creados con safe_fork o fork.

2. Libera todo lo que fuera alocado con safe_malloc

3. Cierra todos los archivos, pipes o lockfiles que fueran abiertos con el resource manager pero no liberados con safe_fclose.

4. Libera toda la memoria compartida alguna vez creada

5. Libera todos los recursos asociados al resource manager
*/
void free_all_resources();

/*
Las operaciones cuya atomicidad se vean afectadas por señales y sea crucial para un correcto
funcionamiento garantizar que no deben ser interrumpidas deben llamar antes a la funcion block_signal garantizando se que bloquearan
todas las señales que estan permitidas bloquear (todas menos SIGKILL y SIGTERM).

Devuelve true si es exitosa.
*/
bool block_signals();

/*
Vuelve las señales al estado previo a la llamada de block_signals, por lo que no se pierden mascaras que se pudieran tener.

Devuelve true si fue exitosa.
*/
bool restore_signals();

#endif
