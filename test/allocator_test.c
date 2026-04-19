#include "../include/allocator_test.h"
#include "../include/allocator.h"
#include "rand_test.h"

#include <stdio.h>
#include <time.h>

#ifndef MAX_ALLOCATIONS
#define MAX_ALLOCATIONS 1000000
#endif

#ifndef MAX_ROUNDS
#define MAX_ROUNDS 1000000000000
#endif

static void *allocations[MAX_ALLOCATIONS] = {0};
static pcg32_random_t my_rng = (pcg32_random_t){0};
static bool tester_ready = false;

static void init_tester(void);
static void gnr_alloc_seq(void); /* generates a random allocation sequence */

bool main_test(void) {
  init_tester();
  gnr_alloc_seq();
  bool ans = true;

  return ans;
}

void example2(void) {
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
}

void example1(void) {
  puts("Single node");
  int *arr = allocate(100 * sizeof(int));
  for (int i = 0; i < 100; i++)
    arr[i] = (i + 1);
  for (int i = 0; i < 100; i++)
    printf("%d\n", arr[i]);
  deallocate(arr);
  puts("Deallocated single node");
}

void example3(void) {
  example1();
  example2();
}

void example4(void) {
  example2();
  example1();
}

void example5(void) {
  example3();
  example4();
  example3();
}

static void init_tester(void) {
  my_rng = init_prng(time(NULL), (uintptr_t)&my_rng);
  tester_ready = true;
}

static void gnr_alloc_seq(void) {
  if (!tester_ready)
    init_tester();
  for (u64 i = 0; i < MAX_ROUNDS; i++) {
    u64 current = randint32_bounded(&my_rng, MAX_ALLOCATIONS);
    if (allocations[current])
      deallocate(allocations[current]);
    else {
      u64 size = randint32_bounded(
          &my_rng, 128000); /* testing normal size allocations */
      allocations[current] = allocate(size);
    }
  }
}
