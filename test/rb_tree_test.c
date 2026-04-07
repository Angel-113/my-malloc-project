#include "../include/rb_tree.h"
#include "../include/rb_tree_test.h"
#include "../include/rand.h"

#include <time.h>
#include <math.h>
#include <stdlib.h>

#define MAX_NODES 100000
#define NODES_TO_INSERT 50000
#define NODES_TO_DELETE 50000

static node_t* root = NULL;
static node_t* nodes[MAX_NODES] = { 0 }; 
static pcg32_random_t my_rng = { 0 };

/* helpers */
static void init_rng ( time_t seed );
static u64 get_rng64 ( void );
static u32 get_rng32 ( void );
static u32 get_rng32_bounded ( u32 bound ); 
static void init_tester ( void );
static void insert_nodes ( u32 nodes );
static void delete_nodes ( u32 nodes );

static bool check_red_red ( node_t* root ); 
static bool blacks_property ( node_t* root);
static bool count_blacks ( node_t* root, i64* blacks );
static bool red_root ( node_t* root );
static bool red_red ( node_t* node );

static double get_time ( struct timespec start, struct timespec end ); 

void tree_test_performance ( void ) { /* This function intends to be sort of a heavy test */
    puts("\033[36mPerformance test\033[0m");
    struct timespec start, end; 

    u32 limit_insertion =  NODES_TO_INSERT; 
    u32 limit_deletion = NODES_TO_DELETE; 

    clock_gettime(CLOCK_MONOTONIC, &start);
    insert_nodes(limit_insertion);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double result_insertion = get_time(start, end);  

    /* In order to not introduce false time, I'm going to delete the first $limit_deletion$ nodes */
    puts(">Deleting nodes");
    clock_gettime(CLOCK_MONOTONIC, &start);
    for ( i32 i = 0; i < limit_deletion; i++ ) {
        node_t* node = nodes[i];
        delete(&root, node); 
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    puts(">Finished deleting secuential nodes");  
    double result_deletion = get_time(start, end);  
    puts("");
    
    puts("\033[36m--> Performance Report <--  \033[0m");
    puts("");
    fprintf(stdout, "\x1b[32mTotal time insertion (%ld)\x1b[0m: %f | \x1b[32maverage time per insertion\x1b[0m: %f \n",
            limit_insertion, result_insertion, (result_insertion/(float)limit_insertion) );
    fprintf(stdout, "\x1b[32mTotal time deletion (%ld)\x1b[0m: %f | \x1b[32maverage time per deletion\x1b[0m: %f \n",
            limit_deletion, result_deletion, (result_deletion/(float)limit_deletion) ); 
    puts("");
    puts("\033[36mPerformance test finished\033[0m");  
}

static double get_time( struct timespec start, struct timespec end ) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9; 
}

bool tree_test( void ) {
    puts("");
    puts("\033[36mTest initiated\033[0m"); 
    init_tester(); 
    if ( !tree_test_insertion(NODES_TO_INSERT) ) return false;
    if ( !tree_test_deletion(NODES_TO_DELETE) ) return false;
    puts("\033[36mTest finished\033[0m");
    puts("");
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

bool tree_test_deletion ( u32 n_nodes ) {
    puts("");
    puts("--> Testing deletion <--"); 
    delete_nodes(n_nodes);
    bool result = test_helper();
    fprintf( !result ? stderr : stdout, !result ? "\033[0;31m --> Tree test deletion failed <-- \033[0m\n" : "\x1b[32m --> Tree test deletion passed <-- \x1b[0m\n" );
    puts("");
    puts("--> Finished test deletion <--");
    puts("");
    return result;  
}

bool tree_test_insertion ( u32 n_nodes ) {
    puts("");
    puts("--> Testing insertion <--"); 
    insert_nodes(n_nodes);
    bool result = test_helper();  
    fprintf( !result ? stderr : stdout, !result ? "\033[0;31m --> Tree test insertion failed <-- \033[0m\n" : "\x1b[32m --> Tree test insertion passed <-- \x1b[0m\n" ); 
    puts("");
    puts("--> Finished test insertion <--"); 
    puts("");
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


static void insert_nodes ( u32 n_nodes ) {
    if ( root == __sentinel || !root ) init_tester();
    puts("");
    puts(">Inserting nodes");  
    for ( i32 i = 0; i <= n_nodes % MAX_NODES; i++ ) {
        u64 random_size = get_rng32_bounded(500);
        nodes[i] = malloc(sizeof(node_t) + sizeof(header_t) + random_size * sizeof(unsigned char));
        nodes[i] = init_node(nodes[i], random_size, __red, __free);
        insert(&root, nodes[i]);
    }
    puts(">Finished inserting nodes");
    puts(""); 
}

 static void delete_nodes ( u32 n_nodes ) {
    if ( root == __sentinel || !root ) {
        fprintf(stderr, "Cannot delete nodes from an empty tree (root == NULL)\n");
        return; 
    }
    puts("");
    puts(">Deleting nodes"); 
    while ( n_nodes-- ) {
        i64 idx = get_rng32() % MAX_NODES;
        while ( !nodes[idx] ) idx = get_rng32() % MAX_NODES;
        delete(&root, nodes[idx]);
        nodes[idx] = NULL; 
    }
    puts(">Finished deleting nodes");
    puts(""); 
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

static u32 get_rng32_bounded ( u32 bound ) {
    return pcg32_boundedrand_r(&my_rng, bound); 
}

static void init_tester ( void ) { 
    if ( root ) return;
    init_rng(time(NULL));
    u64 random_size = get_rng32() % 1000;
    root = malloc( sizeof(node_t) + sizeof(header_t) + random_size * sizeof(unsigned char) );
    root = init_node(root, random_size, __black, __free); 
} 
