#ifndef _LOCK_UTILS
#define _LOCK_UTILS
#include <stdbool.h>

bool acquire_write_lock(int filedes);

bool acquire_read_lock(int filedes);

void release_locked_file(int filedes);
#endif
