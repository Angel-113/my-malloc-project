#include "../include/allocator.h"
#include "../include/allocator_helper.h"
#include "../include/rb_tree.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

#define MAX_THREADS 10
#define MIN_SEG_SIZE (3 * sizeof(header_t))

#ifndef MAX_STD_ALLOC_SIZE

#define MAX_STD_ALLOC_SIZE (128000)
#define MIN_STD_ALLOC_SIZE (MIN_SEG_SIZE)

#endif

/* API implementations */
void *allocate(u64 size) {
  void *ptr = NULL;
  if (size > MAX_STD_ALLOC_SIZE)
    ptr = large_allocation(size);
  else if (size < MIN_STD_ALLOC_SIZE)
    ptr = minimum_allocation(size);
  else
    ptr = standard_allocation(size);
  return ptr;
}

void *reallocate(void *ptr, u64 size) {
  if (get_status(get_node(ptr)->header) || !ptr) {
    print_error("Cannot reallocate memory from a freed node\n");
    return NULL;
  }

  if (get_size(get_node(ptr)->header) >= size) {
    print_error("Reallocation size must be bigger\n");
    return NULL;
  }

  void *new = allocate(size);

  if (!new) {
    print_error("Not enough memory in the system\n");
    return NULL;
  }

  memcpy(new, ptr, get_size(get_node(ptr)->header));
  deallocate(ptr);
  ptr = new;

  return new;
}

void deallocate(void *ptr) {
  if (!ptr) {
    puts("Freeing NULL ptr");
    return;
  }

  if (get_status(get_node(ptr)->header)) {
    puts("Double free operation");
    return;
  }

  u64 size = get_size(get_node(ptr)->header);
  if (size > MAX_STD_ALLOC_SIZE)
    large_deallocation(ptr);
  else if (size < MIN_STD_ALLOC_SIZE)
    minimum_deallocation(ptr);
  else
    standard_deallocation(ptr);
}
