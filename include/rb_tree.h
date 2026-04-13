#ifndef RB_TREE_H
#define RB_TREE_H

#include "base.h"
#include "header.h"

/*
    |- - -|- - -  - --|- - -|
    |  H  |           |  F  |
    |  E  |           |  O  |
    |  A  |  D A T A  |  O  |
    |  D  |           |  T  |
    |  E  |           |  E  |
    |  R  |           |  R  |
    |- - -|- - - -- --|- - -|

    You could see the DATA segment as:
    union {
      void* ptr; (buffer)
      Node* nodes[3]; (parent, left and right)
    }
 */

typedef struct Node {
  header_t header;
  struct Node *parent;
  struct Node *right;
  struct Node *left;
} node_t;

extern node_t *__sentinel;

extern node_t *insert(node_t **root, node_t *new_node);
extern node_t *delete (node_t **root, node_t *node);
extern node_t *init_node(void *ptr, u64 size, bool color, bool status);
extern node_t *search(node_t *root, u64 target);
extern node_t *get_node(void *ptr);
extern node_t *get_next_node(node_t *node);
extern node_t *get_prev_node(node_t *node);
extern node_t *merge_nodes(node_t *a, node_t *b);

#endif
