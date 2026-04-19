#ifndef ALLOCATOR_H
#define ALLOCATOR_H

extern void *allocate(unsigned long long size);
extern void *reallocate(void *ptr, unsigned long long size);
extern void deallocate(void *ptr);

#endif
