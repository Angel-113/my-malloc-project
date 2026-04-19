#include "../include/allocator_helper.h"
#include "../include/rb_tree.h"

#include <limits.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

static u64 page_size = 0;
static const header_t PAGE_HEADER = 0;
static const header_t PAGE_FOOTER = ULLONG_MAX;

#define THREADS 10
static node_t *ROOTS[THREADS] = {0};

#define PAGES(size) (((size) + (page_size - 1)) & ~(page_size - 1))
#define HEADERS(n) ((n) * sizeof(header_t))

#define MIN_STD_ALLOC_SIZE (3 * sizeof(header_t))
#define MAX_STD_ALLOC_SIZE (128000)

#ifndef MALLOC_TEST_HOOK
#define MALLOC_TEST_HOOK 0
#endif

static node_t *create_page(u64 size);
static void set_page_headers(node_t *page);
static node_t *split_node(node_t **node, u64 size);
static node_t *bestfit_search(node_t **root, u64 size);

__attribute__((constructor(101))) static void init_page_size(void);

/* these functions will help to introduce multithreading to this program */
static node_t **get_current_root(void);
static u64 get_root_id(u64 pid);

#if MALLOC_TEST_HOOK

#include <time.h>

typedef struct page_s {
  void *start;
  u64 bytes;
} page_t;

static u64 current_idx = 0;
static u64 current_size = 10;
static page_t *PAGES = NULL;

static void add_page(page_t page);

/* variables to measure performance and guide tests */

time_t last_getusage_call = 0;

static float ext_frag = 0;
static float int_frag = 0;
static u64 maxfree = 0;
static u64 maxinuse = 0;
static u64 crrnt_mem_rqstd = 0;
static u64 crrnt_mem_inuse = 0;
static u64 crrnt_mem_freed = 0;

#endif

/* main functions implementations */

/* allocation */
void *standard_allocation(unsigned long long size) {
  void *ptr = NULL;
  node_t **root = get_current_root();
  node_t *node = NULL;

  if (!page_size)
    init_page_size();

  if (!*root) {
    *root = create_page(size);
    node = split_node(root, size);
  } else if ((*root)->left != __sentinel || (*root)->right != __sentinel)
    node = bestfit_search(root, size);
  else
    node = split_node(root, size);

  if (!node) {
    u64 size_root = get_size((*root)->header);
    if (size > size_root) {
      *root = create_page(size);
      node = split_node(root, size);
    } else {
      node = *root;
      *root = create_page(1);
    }
  }

  ptr = (void *)((u8 *)node + HEADERS(1));

  set_color(&(*root)->header, __black);
  set_status(&(*root)->header, __free);
  set_status(&node->header, __in_use);

  return ptr;
}
/* sharded lists in a future */
void *minimum_allocation(unsigned long long size) {
  return standard_allocation(MIN_STD_ALLOC_SIZE);
}

void *large_allocation(unsigned long long size) {
  if (!page_size)
    init_page_size();
  void *ptr = create_page(size + HEADERS(2));
  set_status(&get_node(ptr)->header, __in_use);
  return ptr;
}

/* deallocation */
void standard_deallocation(void *ptr) {
  node_t *node = get_node(ptr);
  node_t **root = get_current_root();
  set_status(&node->header, __free);
  set_color(&node->header, __red);
  insert(root, node);
}

/* sharded lists in a future */
void minimum_deallocation(void *ptr) { standard_deallocation(ptr); }

void large_deallocation(void *ptr) {
  node_t *node = get_node(ptr);
  munmap(node, get_size(node->header) + HEADERS(2));
}

/* helpers implementations */
static node_t *create_page(u64 size) {
  u64 pages = PAGES(size);
  u64 node_size = pages * page_size - HEADERS(2);

  node_t *node = mmap(NULL, pages * page_size, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (node == MAP_FAILED) {
    print_error("Not enough memory in the system\n");
    return NULL;
  }

  set_page_headers(node);
  node = init_node((u8 *)node + HEADERS(1), size - HEADERS(4), __red, __free);
  return node;
}

static void set_page_headers(node_t *page) {
  header_t *header = (header_t *)page;
  header_t *footer = (header_t *)((u8 *)page + get_size(page->header));
  *header = PAGE_HEADER;
  *footer = PAGE_FOOTER;
}

static node_t *split_node(node_t **node, u64 size) {
  if (get_size((*node)->header) < size)
    return NULL;

  bool color = node == get_current_root() ? __black : __red;
  u64 node_size = get_size((*node)->header) + HEADERS(2);
  node_t *new_node = NULL;

  if (node_size - (size + HEADERS(4)) >= MIN_STD_ALLOC_SIZE) {
    new_node = *node;
    new_node = init_node(new_node, size, __red, __free);
    *node = get_next_node(new_node);
    node_size -= (size + HEADERS(4));
    *node = init_node(*node, node_size, color, __free);
  }

  return new_node;
}

static node_t *bestfit_search(node_t **root, u64 size) {
  node_t *node = search(*root, size);
  node_t *new_node = NULL;

  if (node == __sentinel)
    node = create_page(size);

  new_node = split_node(&node, size);

  if (new_node) {
    insert(root, node);
    node = new_node;
  }

  return node;
}

static node_t **get_current_root(void) { return &ROOTS[get_root_id(0)]; }
static u64 get_root_id(u64 pid) { return pid; }

__attribute__((constructor(101))) static void init_page_size(void) {
  page_size = sysconf(_SC_PAGESIZE);
}

/* function for testing allocator */
#if MALLOC_TEST_HOOK

static void add_page(page_t page) {
  if (!PAGES)
    PAGES = malloc(sizeof(page_t) * 10);
  else if (current_idx + 1 >= current_size) {
    current_size += (current + (current >> 1)) / current;
    PAGES = realloc(PAGES, current_size);
  }
  PAGES[current_idx++] = page;
}

static u64 getpage_size(page_t page) { return page.bytes + HEADERS(2); }

static void ttl_mem_rqstd(void) {
  u64 crrnt_mem_rqstd = 0;
  for (i64 i = 0; i <= current_idx; i++)
    crrnt_mem_rqstd += getpage_size(PAGES[i]);
}

static void get_mem_usage(void) {
  crrnt_mem_rqstd = 0;

  for (i64 i = 0; i <= current_idx; i++) {
    header_t *current_header = (header_t *)((u8 *)PAGES[i] + HEADERS(1));

    while (current_header != PAGE_FOOTER) {
      bool status = get_status(*current_header);
      u64 size = get_size(*current_header);

      u64 *current_item = status ? &crrnt_mem_freed : &crrnt_mem_inuse;
      u64 *current_max = status ? &maxfree : &maxinuse;

      *current_max = *current_max <= size ? size : *current_max;
      *current_item += get_size(*current_header);

      current_header = (header_t *)((u8 *)current_header + size + HEADERS(1));
    }

    crrnt_mem_rqstd += getpage_size(PAGES[i]);
  }

  ext_frag = (float)((float)max_free / (float)crrnt_mem_freed);
  int_frag = crrnt_mem_inuse - crrnt_mem_rqstd;
}

u64 check_total_mem_requested(void) {
  ttl_mem_rqstd();
  return crrnt_mem_rqstd;
}

/* this works, I'll optimize it later : april 19th */

u64 check_total_mem_in_use(void) {
  time_t current = time(NULL);
  /* I'm waiting at least 30 cycles to recompute the tests vars */
  if (current - last_getmem_call >= 30)
    get_mem_usage();
  return crrnt_mem_inuse;
}

u64 check_total_mem_free(void) {
  time_t current = time(NULL);
  if (current - last_getmem_call >= 30)
    get_mem_usage();
  return crrnt_mem_freed;
}

float ext_fragmntn(void) {
  time_t current = time(NULL);
  if (current - last_getmem_call >= 30)
    get_mem_usage();
  return ext_frag;
}

#endif
