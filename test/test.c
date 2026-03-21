#include "../include/test.h"
#include "../include/rb_tree.h"
#include "../include/header.h"
#include "../include/rand.h"

#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#define MAX_NODES 1000

static node_t* root = NULL;
static node_t* nodes[MAX_NODES] = { 0 }; 
static pcg32_random_t my_rng = { 0 };

/* helpers */
static void init_rng ( time_t seed );
static u64 get_rng64 ( void );
static u32 get_rng32 ( void );
static void init_tester ( void );
static void insert_nodes ( void );
static void delete_nodes ( u64 n_nodes );

static bool tree_test_insertion ( void );
static bool tree_test_deletion( void ); 

static bool check_red_red ( node_t* root ); 
static bool blacks_property ( node_t* root);
static bool count_blacks ( node_t* root, i64* blacks );
static bool red_root ( node_t* root );
static bool red_red ( node_t* node );

int main( void ) {
    puts("\033[36mTest initiated\033[0m"); 
    tree_test();
    puts("\033[36mTest finished\033[0m"); 
    return 0; 
}

bool tree_test( void ) {
    init_tester(); 
    if ( !tree_test_insertion() ) return false;
    if ( !tree_test_deletion() ) return false;    
    return true; 
}

static bool test_helper ( void ) {
    bool result = true;
    bool red_root_check = red_root(root);
    bool red_red_check = red_red(root); 
    bool blacks_property_check = blacks_property(root); 
    result = red_red_check && !red_root_check && blacks_property_check; 
    return result; 
}

static bool tree_test_deletion ( void ) {
    puts("--> Testing deletion <--"); 
    delete_nodes(get_rng32() % 100);
    bool result = test_helper();
    fprintf( !result ? stderr : stdout, !result ? "\033[0;31m --> Tree test deletion failed <-- \033[0m\n" : "\x1b[32m --> Tree test deletion passed <-- \x1b[0m\n" ); 
    puts("--> Finished test deletion <--");
    return result;  
}

static bool tree_test_insertion ( void ) {
    puts("--> Testing insertion <--"); 
    insert_nodes();
    bool result = test_helper();  
    fprintf( !result ? stderr : stdout, !result ? "\033[0;31m --> Tree test insertion failed <-- \033[0m\n" : "\x1b[32m --> Tree test insertion passed <-- \x1b[0m\n" ); 
    puts("--> Finished test insertion <--"); 
    return result; 
}

static bool count_blacks ( node_t* root, i64* blacks ) {
    if ( root == __sentinel ) {
        (*blacks)++; 
        return true; 
    }
    else if ( !get_color(root->header) ) (*blacks)++;

    i64 left_count = 0; i64 right_count = 0;
    bool left = count_blacks(root->left, &left_count);
    bool right = count_blacks(root->right, &right_count);

    if ( !left || !right || left_count != right_count ) return false; 

    (*blacks) += left_count; 
    return true; 
}

static bool check_red_red ( node_t* root ) {
    if ( root == __sentinel ) return true;  
    if ( get_color(root->header) && ( get_color(root->left->header) || get_color(root->right->header) ) ) 
        return false;  
    else if ( !check_red_red(root->left) || !check_red_red(root->right) ) return false; 
    return true; 
}

static bool blacks_property ( node_t* root ) {
    i64 blacks = 0;
    if ( !count_blacks(root, &blacks) ) {
        fprintf(stderr, ">Blacks property violated\n");
        return false; 
    }
    fprintf(stdout, ">Blacks property preserved\n"); 
    return true; 
}

static bool red_root ( node_t* root ) {
    if ( get_color(root->header) ) {
        fprintf(stderr, ">Root can't be red\n");
        return true;    
    }
    fprintf(stdout, ">Root is not red \n"); 
    return false; 
}

static bool red_red ( node_t* root ) {
    if ( !check_red_red(root) ) {
        fprintf(stderr, ">Red-red violation has ocurred\n");
        return false; 
    }
    fprintf(stdout, ">Red-red test passed\n"); 
    return true; 
}


static void insert_nodes ( void ) {
    if ( root == __sentinel || !root ) init_tester();
    puts(">Inserting nodes");  
    for ( i32 i = 0; i < MAX_NODES; i++ ) {
        u64 random_size = get_rng32() % 100;
        nodes[i] = malloc(sizeof(node_t) + sizeof(header_t) + random_size * sizeof(unsigned char));
        nodes[i] = init_node(nodes[i], random_size, __red, __free);
        insert(&root, nodes[i]);
    }
    puts(">Finished inserting nodes"); 
}

 static void delete_nodes ( u64 n_nodes ) {
    if ( root == __sentinel || !root ) {
        fprintf(stderr, "Cannot delete nodes from an empty tree (root == NULL)\n");
        return; 
    }
    puts(">Deleting nodes"); 
    while ( n_nodes-- ) {
        i64 idx = get_rng32() % 1000;
        while ( !nodes[idx] ) idx = get_rng32() % 1000;
        delete(&root, nodes[idx]);
        nodes[idx] = NULL; 
    }
    puts(">Finished deleting nodes"); 
}

static void init_rng ( time_t seed ) {
    pcg32_srandom_r(&my_rng, seed, (intptr_t)&my_rng); 
}

static u64 get_rng64 ( void ) { /* for getting a random 64 bit integer */
    return ldexpl(pcg32_random_r(&my_rng), -32); 
}

static u32 get_rng32 ( void ) { /* generated a 32 bit integer */
    return pcg32_random_r(&my_rng); 
}

static void init_tester ( void ) { 
    if ( root ) return;
    init_rng(time(NULL));
    u64 random_size = get_rng32() % 1000;
    root = malloc( sizeof(node_t) + sizeof(header_t) + random_size * sizeof(unsigned char) );
    root = init_node(root, random_size, __black, __free); 
} 
