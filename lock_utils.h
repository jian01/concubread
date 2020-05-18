#ifndef _LOCK_UTILS
#define _LOCK_UTILS
#include <stdbool.h>

bool acquire_exclusive_lock(int filedes);

bool acquire_exclusive_lock_nonblocking(int filedes);

bool acquire_read_lock(int filedes);

void release_locked_file(int filedes);
#endif
