#include "../include/allocator.h"
#include "base.h"
#include "header.h"

static u64 page_size = 0;
static const header_t PAGE_HEADER = 0;
static const header_t PAGE_FOOTER = LLONG_MAX;

#define PAGES(size) (((size) + (page_size - 1)) & ~(page_size - 1))
#define HEADERS(n) ((u64)((n) * sizeof(header_t)))
#define MAX_THREADS 10
#define MIN_SEG_SIZE (u64)(3 * sizeof(header_t))

static u32 id_current_root = 0;
static node_t *current_roots[MAX_THREADS] = {0};

static node_t **get_current_root(void);
static u16 get_current_root_id(void);
static void memcopy(void *src, void *dest, u64 size);
static void *add_mem_page(u64 n);

static node_t *split_node(node_t **a, u64 size);
static void *search_node(node_t **root, u64 size);
static void *create_root(node_t **root, u64 size);
static void *split_segment(node_t **root, u64 size);
static node_t *init_page(u64 n);

static void *standard_allocation(u64 size);
static void *minimum_allocation(u64 size);
static void *large_allocation(u64 size);

void *allocate(u64 size) {
  page_size = !page_size ? sysconf(_SC_PAGESIZE) : page_size;
  node_t **root = get_current_root();

  printf("Root status: %llu\n", get_size((*root)->header));
  void *ptr = NULL;
  if (!(*root))
    ptr = create_root(root, size);
  else if ((*root)->left != __sentinel || (*root)->right != __sentinel)
    ptr = search_node(root, size);
  else
    ptr = split_segment(root, size);
  return ptr;
}

static void *create_root(node_t **root, u64 size) {
  node_t *page = init_page(1);

  if (page == __sentinel) {
    print_error("Not enough memory in the system\n");
    return NULL;
  }

  node_t *node = (node_t *)((u8 *)page + HEADERS(1));
  node = init_node(node, size, __red, __in_use);

  u64 root_size = page_size - (size + HEADERS(6));
  *root = init_node(get_next_node(node), root_size, __black, __free);

  return (void *)((u8 *)node + HEADERS(1));
}

static void *search_node(node_t **root, u64 size) {
  node_t *node = search(*root, size);
  void *ptr = NULL;

  if (node != __sentinel) {
    delete (root, node);
    ptr = (void *)((u8 *)node + HEADERS(1));
  } else {
    ptr = create_root(&node, size);
    insert(root, node);
  }

  return ptr;
}

static void *split_segment(node_t **root, u64 size) {
  u64 curr_seg_size = get_size((*root)->header) + HEADERS(2);
  node_t *node = NULL;

  if (curr_seg_size - (size + HEADERS(4) >= MIN_SEG_SIZE))
    node = split_node(root, size);
  else {
    node = *root;

    node_t *new_page = init_page(1);
    u64 root_size = page_size - HEADERS(4);

    *root = (node_t *)((u8 *)new_page + HEADERS(1));
    *root = init_node(*root, root_size, __black, __free);
  }

  return (void *)((u8 *)node + HEADERS(1));
}

void *reallocate(void *ptr, u64 size) {
  node_t *node = get_node(ptr);

  if (get_size(node->header) <= size) {
    print_error("Reallocation size needs to be bigger\n");
    return NULL;
  }

  void *new_ptr = allocate(size);
  memcopy(ptr, new_ptr, get_size(node->header));

  set_color(&node->header, __red);
  set_status(&node->header, __free);
  deallocate(ptr);

  return new_ptr;
}

void deallocate(void *ptr) {
  node_t *node = get_node(ptr);

  node_t *node_to_merge = __sentinel;
  node_t *merged_node = __sentinel;
  node_t **root = get_current_root();

  printf("Root status: %llu\n", get_size((*root)->header));

  if (get_status(node->header)) {
    fprintf(stderr, "Double free operation\n");
    return;
  }

  header_t *header_prev = (header_t *)((u8 *)node - HEADERS(1));
  header_t *header_next =
      (header_t *)((u8 *)node + get_size(node->header) + HEADERS(1));

  bool left = *header_prev == PAGE_HEADER ? false : get_status(*header_prev);
  bool right = *header_next == PAGE_FOOTER ? false : get_status(*header_next);

  if (left && right) { /* merge three nodes */
    delete (root, get_prev_node(node));
    delete (root, get_next_node(node));
    merged_node = merge_nodes(merge_nodes(node, get_prev_node(node)),
                              get_next_node(node));
  } else
    node_to_merge = left    ? get_prev_node(node)
                    : right ? get_next_node(node)
                            : __sentinel;

  if (node_to_merge != __sentinel) {
    delete (root, node_to_merge);
    merged_node = merge_nodes(node, node_to_merge);
  }

  if (merged_node != __sentinel)
    node = merged_node;

  set_status(&node->header, __free);
  set_color(&node->header, __red);

  insert(get_current_root(), node);

  if (*root == __sentinel)
    puts("Root is __sentinel");
}

static node_t *split_node(node_t **node, u64 size) {
  u64 prev_size = get_size((*node)->header) + HEADERS(2);
  bool color = get_color((*node)->header);

  size = size < MIN_SEG_SIZE ? MIN_SEG_SIZE : size;
  node_t *new_node = init_node(*node, size, __red, __in_use);
  u64 new_size = prev_size - (size + HEADERS(4));
  *node = init_node(get_next_node(new_node), new_size, color, __free);

  return new_node;
}

/* This function will help to manage threads */
static node_t **get_current_root(void) {
  return &current_roots[get_current_root_id()];
}

/* Main root is 0 */
static u16 get_current_root_id(void) { return 0; }

static node_t *init_page(u64 n) {
  void *start = add_mem_page(n);

  if (!start)
    return __sentinel;

  u64 size = page_size * n - HEADERS(2);
  node_t *node = init_node(start, size, __red, __free);
  node->header = PAGE_HEADER;
  header_t *footer = (header_t *)((u8 *)get_next_node(node) - HEADERS(1));
  *footer = PAGE_FOOTER;

  return node;
}

static void *add_mem_page(u64 n) { /* syscall for mempages */
  return mmap(NULL, page_size * n, PROT_READ | PROT_WRITE,
              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

/*
    TODO: Work on optimizations for copying memory segments based
    on 64-bit, 32-bit, 16-bit and 8-bit chunks.
*/

static void memcopy(void *src, void *dest, u64 size) {
  u64 res_8 = (size - (size % 8)) / 8;
  u64 res_4 = (res_8 - (res_8 % 4)) / 4;
  u64 res_2 = (res_4 - (res_4 % 2)) / 2;

  u32 *start = (u32 *)((u64 *)src + res_8);

  for (i64 i = 0; i < 8 * res_8; i++)
    ((u64 *)dest)[i] = ((u64 *)src)[i];
}
