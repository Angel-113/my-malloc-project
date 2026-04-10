#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "base.h"

extern void* allocate ( u64 size );
extern void* reallocate ( void* ptr, u64 size );
extern void deallocate ( void* ptr ); 

#endif
