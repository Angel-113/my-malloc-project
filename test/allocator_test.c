#include "../include/allocator_test.h"
#include "allocator.h"

static bool new_example(void) {
  puts("Multiple nodes");
  int *arr[50] = {0};
  for (int i = 0; i < 50; i++) {
    arr[i] = allocate(sizeof(int));
    *arr[i] = (i + 1);
  }
  for (int i = 0; i < 50; i++)
    printf("%d\n", *arr[i]);
  puts("Deallocating multiple nodes");
  for (int i = 0; i < 50; i++)
    deallocate(arr[i]);
  return true;
}

bool example(void) {
  puts("Allocator test - Example");
  puts("Single node");
  int *arr = allocate(100 * sizeof(int));
  for (int i = 0; i < 100; i++)
    arr[i] = (i + 1);
  for (int i = 0; i < 100; i++)
    printf("%d\n", arr[i]);
  deallocate(arr);
  puts("Deallocating single node");
  new_example();
  puts("Allocator test finish");
  return true;
}
