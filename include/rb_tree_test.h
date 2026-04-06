#ifndef RB_TREE_TEST_H
#define RB_TREE_TEST_H

#include "../include/base.h"

#define MAX_NODES 1000

extern bool tree_test_insertion ( u32 nodes );
extern bool tree_test_deletion ( u32 nodes );
extern bool tree_test ( void );
extern void tree_test_performance ( u32 insert_nodes, u32 deleted_nodes );  

#endif
