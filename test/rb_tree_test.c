#include "../include/rb_tree_test.h"
#include "../include/rand_test.h"
#include "../include/rb_tree.h"

#include <math.h>
#include <stdlib.h>
#include <time.h>

#ifndef MAX_NODES
#define MAX_NODES 1000000
#endif

#ifndef NODES_TO_INSERT
#define NODES_TO_INSERT 750000
#endif

#ifndef NODES_TO_DELETE
#define NODES_TO_DELETE 500000
#endif

static node_t *root = NULL;
static node_t *nodes[MAX_NODES] = {0};
static pcg32_random_t my_rng = {0};

/* helpers */
static void init_tester(void);
static void insert_nodes(u32 nodes);
static void delete_nodes(u32 nodes);

static bool check_red_red(node_t *root);
static bool blacks_property(node_t *root);
static bool count_blacks(node_t *root, i64 *blacks);
static bool red_root(node_t *root);
static bool red_red(node_t *node);

static double get_time(struct timespec start, struct timespec end);

/* this function intends to be sort of a heavy test */
void tree_test_performance(void) {
  puts("\033[36mPerformance test\033[0m");
  fflush(stdout);
  struct timespec start, end;

  u32 limit_insertion = NODES_TO_INSERT;
  u32 limit_deletion = NODES_TO_DELETE;

  clock_gettime(CLOCK_MONOTONIC, &start);
  insert_nodes(limit_insertion);
  clock_gettime(CLOCK_MONOTONIC, &end);
  double result_insertion = get_time(start, end);

  /* In order to not introduce false time, I'm going to delete the first
   * $limit_deletion$ nodes */
  puts(">Deleting nodes");
  fflush(stdout);

  clock_gettime(CLOCK_MONOTONIC, &start);
  for (i32 i = 0; i < limit_deletion; i++) {
    node_t *node = nodes[i];
    delete (&root, node);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  puts(">Finished deleting secuential nodes");
  fflush(stdout);

  double result_deletion = get_time(start, end);

  puts("\033[36m--> Performance Report <--  \033[0m");
  fflush(stdout);

  fprintf(stdout,
          "\x1b[32mTotal time insertion (%ld)\x1b[0m: %f | \x1b[32maverage "
          "time per insertion\x1b[0m: %f \n",
          limit_insertion, result_insertion,
          (result_insertion / (float)limit_insertion));
  fflush(stdout);

  fprintf(stdout,
          "\x1b[32mTotal time deletion (%ld)\x1b[0m: %f | \x1b[32maverage time "
          "per deletion\x1b[0m: %f \n",
          limit_deletion, result_deletion,
          (result_deletion / (float)limit_deletion));
  fflush(stdout);

  puts("\033[36mPerformance test finished\033[0m");
  fflush(stdout);
}

static double get_time(struct timespec start, struct timespec end) {
  return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

bool tree_test(void) {
  puts("\033[36mTest initiated\033[0m");
  fflush(stdout);
  init_tester();
  if (!tree_test_insertion(NODES_TO_INSERT))
    return false;
  if (!tree_test_deletion(NODES_TO_DELETE))
    return false;
  puts("\033[36mTest finished\033[0m");
  fflush(stdout);
  return true;
}

static bool test_helper(void) {
  bool result = true;
  bool red_root_check = red_root(root);
  bool red_red_check = red_red(root);
  bool blacks_property_check = blacks_property(root);
  result = red_red_check && !red_root_check && blacks_property_check;
  return result;
}

bool tree_test_deletion(u32 n_nodes) {
  puts("--> Testing deletion <--");
  fflush(stdout);
  delete_nodes(n_nodes);
  bool result = test_helper();
  fprintf(!result ? stderr : stdout,
          !result ? "\033[0;31m --> Tree test deletion failed <-- \033[0m\n"
                  : "\x1b[32m --> Tree test deletion passed <-- \x1b[0m\n");
  fflush(stdout);
  puts("--> Finished test deletion <--");
  fflush(stdout);
  return result;
}

bool tree_test_insertion(u32 n_nodes) {
  puts("--> Testing insertion <--");
  fflush(stdout);
  insert_nodes(n_nodes);
  bool result = test_helper();
  fprintf(!result ? stderr : stdout,
          !result ? "\033[0;31m --> Tree test insertion failed <-- \033[0m\n"
                  : "\x1b[32m --> Tree test insertion passed <-- \x1b[0m\n");
  fflush(stdout);
  puts("--> Finished test insertion <--");
  fflush(stdout);
  return result;
}

static bool count_blacks(node_t *root, i64 *blacks) {
  if (root == __sentinel) {
    (*blacks)++;
    return true;
  } else if (!get_color(root->header))
    (*blacks)++;

  i64 left_count = 0;
  i64 right_count = 0;
  bool left = count_blacks(root->left, &left_count);
  bool right = count_blacks(root->right, &right_count);

  if (!left || !right || left_count != right_count)
    return false;

  (*blacks) += left_count;
  return true;
}

static bool check_red_red(node_t *root) {
  if (root == __sentinel)
    return true;
  if (get_color(root->header) &&
      (get_color(root->left->header) || get_color(root->right->header)))
    return false;
  else if (!check_red_red(root->left) || !check_red_red(root->right))
    return false;
  return true;
}

static bool blacks_property(node_t *root) {
  i64 blacks = 0;
  if (!count_blacks(root, &blacks)) {
    fprintf(stderr, ">Blacks property violated\n");
    fflush(stderr);
    return false;
  }
  fprintf(stdout, ">Blacks property preserved\n");
  fflush(stdout);
  return true;
}

static bool red_root(node_t *root) {
  if (get_color(root->header)) {
    fprintf(stderr, ">Root can't be red\n");
    fflush(stderr);
    return true;
  }
  fprintf(stdout, ">Root is not red \n");
  fflush(stdout);
  return false;
}

static bool red_red(node_t *root) {
  if (!check_red_red(root)) {
    fprintf(stderr, ">Red-red violation has ocurred\n");
    fflush(stderr);
    return false;
  }
  fprintf(stdout, ">Red-red test passed\n");
  fflush(stdout);
  return true;
}

static void insert_nodes(u32 n_nodes) {
  if (root == __sentinel || !root)
    init_tester();
  puts(">Inserting nodes");
  fflush(stdout);
  for (i32 i = 0; i <= n_nodes % MAX_NODES; i++) {
    u32 random_size = randint32_bounded(&my_rng, 500);
    nodes[i] = malloc(sizeof(node_t) + sizeof(header_t) +
                      random_size * sizeof(unsigned char));
    nodes[i] = init_node(nodes[i], random_size, __red, __free);
    insert(&root, nodes[i]);
  }
  puts(">Finished inserting nodes");
  fflush(stdout);
}

static void delete_nodes(u32 n_nodes) {
  if (root == __sentinel || !root) {
    fprintf(stderr, "Cannot delete nodes from an empty tree (root == NULL)\n");
    fflush(stderr);
    return;
  }
  puts(">Deleting nodes");
  fflush(stdout);
  while (n_nodes--) {
    u32 idx = randint32_bounded(&my_rng, MAX_NODES);
    while (!nodes[idx])
      idx = randint32_bounded(&my_rng, MAX_NODES);
    delete (&root, nodes[idx]);
    nodes[idx] = NULL;
  }
  puts(">Finished deleting nodes");
  fflush(stdout);
}

static void init_tester(void) {
  if (root)
    return;
  init_prng(time(NULL), (intptr_t)&my_rng);
  u64 random_size = randint32_bounded(&my_rng, MAX_NODES);
  root = malloc(sizeof(node_t) + sizeof(header_t) +
                random_size * sizeof(unsigned char));
  root = init_node(root, random_size, __black, __free);
}
