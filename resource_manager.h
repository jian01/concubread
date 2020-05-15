#ifndef _RESOURCE_MANAGER
#define _RESOURCE_MANAGER
#include <stdio.h>
#include <stdlib.h>

bool initialize_resource_manager();

void* safe_malloc(size_t size);

FILE* safe_fopen(const char *filename, const char *mode);

int safe_fclose(FILE *stream);

pid_t safe_fork();

void free_all_resources();

#endif
