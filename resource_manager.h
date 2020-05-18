#ifndef _RESOURCE_MANAGER
#define _RESOURCE_MANAGER
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool initialize_resource_manager();

void* safe_malloc(size_t size);

void* shared_malloc(size_t size, const char *shared_memory_file);

FILE* safe_fopen(const char *filename, const char *mode);

int fpipe(FILE* pipefile[2]);

FILE* create_lockfile();

int safe_fclose(FILE *stream);

pid_t safe_fork();

void free_all_resources();

#endif
