#include "../include/allocator_test.h"
#include "../include/rb_tree_test.h"

#include <stdlib.h>
#include <string.h>

#ifndef MAX_NODES
#define MAX_NODES 1000000
#endif

#ifndef NODES_TO_INSERT
#define NODES_TO_INSERT 750000
#endif

#ifndef NODES_TO_DELETE
#define NODES_TO_DELETE 500000
#endif

static bool gnrltree_test(void);
static bool tree_testing(const char *test_type);
static bool allocator_testing(const char *test_type, const char *example_type);
static bool allocator_examples(u8 example_type);

int main(int argc, char **argv) {
  if (argc < 1) {
    puts("Must select a module to test");
    return 0;
  }
  if (!strcmp(argv[0], "tree"))
    return tree_testing(argv[1]);
  else if (!strcmp(argv[0], "allocator"))
    return allocator_testing(argv[1], argv[2]);
  else
    puts("Selection is invalid");
}

static bool gnrltree_test(void) {
  bool ans = tree_test();
  if (ans)
    tree_test_performance();
  return ans;
}

static bool tree_testing(const char *test_type) {
  if (!strcmp(test_type, "general"))
    return gnrltree_test();
  else if (!strcmp(test_type, "insertion"))
    return tree_test_insertion(NODES_TO_INSERT);
  else if (!strcmp(test_type, "deletion"))
    return tree_test_deletion(NODES_TO_DELETE);
  else if (!strcmp(test_type, "performance"))
    tree_test_performance();
  else
    puts("Selection is invalid");
  return 0;
}

static bool allocator_testing(const char *test_type, const char *example_type) {
  if (!strcmp(test_type, "examples") && example_type)
    return allocator_examples(atoi(example_type));
  else if (!strcmp(test_type, "main_test"))
    return main_test();
  return 0;
}

static bool allocator_examples(u8 example_type) {
  switch (example_type) {
  case 1:
    example1();
    break;
  case 2:
    example2();
    break;
  case 3:
    example3();
    break;
  case 4:
    example4();
    break;
  case 5:
    example5();
    break;
  }
  return 0;
}
