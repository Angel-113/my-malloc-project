#ifndef ALLOCATOR_HELPER
#define ALLOCATOR_HELPER

#ifndef MALLOC_TEST_HOOK
#define MALLOC_TEST_HOOK 0
#endif

#if MALLOC_TEST_HOOK
#include "base.h"

extern bool check_pages_integrity(void);

extern float ext_fragmntn(void);
extern float int_fragmntn(void);

extern u64 check_total_mem_requested(void);
extern u64 check_total_mem_in_use(void);
extern u64 check_total_mem_free(void);

#endif

extern void *standard_allocation(unsigned long long size);
extern void *minimum_allocation(unsigned long long size);
extern void *large_allocation(unsigned long long size);

extern void standard_deallocation(void *ptr);
extern void minimum_deallocation(void *ptr);
extern void large_deallocation(void *ptr);

#endif
