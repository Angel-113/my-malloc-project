#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "../include/rb_tree.h"
#include "base.h"

#include <limits.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

extern void *allocate(u64 size);
extern void *reallocate(void *ptr, u64 size);
extern void deallocate(void *ptr);

#endif
