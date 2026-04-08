#include "../include/allocator.h"
#include "../include/base.h"
#include "../include/rb_tree.h"
#include "header.h"
#include <stdio.h>
#include <sys/mman.h>

#define PAGES( size ) (((size) + (PAGE - 1)) & ~(PAGE - 1))
#define PAGE 4092
#define MAX_THREADS 10
#define MIN_SEG_SIZE 3 * sizeof(header_t)

static u32 id_current_root = 0; 
static node_t* current_roots[ MAX_THREADS ] = { 0 }; 

static node_t** get_current_root ( void );
static u16 get_current_root_id ( void ); 
static bool memcopy ( void* src, void* dest, u64 size );

static void* search_node ( node_t** root, u64 size ); 
static void* create_root ( node_t** root, u64 size ); 
static void* split_segment ( node_t** root, u64 size ); 

static node_t* add_mem_page( u64 size ) { /* syscall for mempages */
    return  mmap(NULL, PAGES(size), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

void* allocate ( u64 size ) { 
     node_t** root = get_current_root();
     void* ptr = NULL;
     if ( !(*root) ) ptr = create_root(root, size);
     else if ( (*root)->left != __sentinel || (*root)->right != __sentinel ) ptr = search_node(root, size); 
     else ptr = split_segment(root, size); 
     return ptr; 
}

static void* create_root ( node_t** root, u64 size ) {
    *root = add_mem_page(PAGES(size));
    if ( !*root ) {
        fprintf(stderr, "Not enough memory in the system\n");
        return NULL; 
    }
    node_t* node = init_node(*root, size, __red, __in_use);
    void* new_start = (void *)((u8 *)node + size + 2 * sizeof(header_t));
    u64 new_size = PAGES(size) - (get_size(node->header) + 2 * sizeof(header_t));  
    *root = init_node(new_start, new_size, __red, __free);
    return (void *)((u8 *)node + sizeof(header_t)); 
}

static void* search_node ( node_t** root, u64 size ) {
    node_t* node = search(*root, size);
    void* ptr = NULL; 
    if ( node != __sentinel ) {
        delete(root, node);
        ptr = (void *)((u8 *)node + sizeof(header_t)); 
    }
    else {
        ptr = create_root(&node, size); 
        insert(root, node); 
    }
    return ptr; 
}

static void* split_segment ( node_t** root, u64 size ) {
    void* ptr = NULL; 
    i64 curr_seg_size = get_size((*root)->header) + 2 * sizeof(header_t);
    i64 diff = curr_seg_size - (size + 2 * sizeof(header_t));
    if ( diff >= MIN_SEG_SIZE ) {
        node_t* node = init_node( *root, size, __red, __in_use );
        void* new_start = (void *)((u8 *)node + size + 2 * sizeof(header_t)); 
        *root = init_node(new_start, diff, __black, __free); 
        ptr = (void *)((u8 *)node + sizeof(header_t)); 
    }
    else {
        node_t* node = *root;
        *root = add_mem_page(PAGES(1));
        if ( !(*root) ) {
            fprintf(stderr, "Not enough memory system\n");
            return NULL;
        }
        *root = init_node(*root, PAGES(1), __black, __free); 
        ptr = (void *)((u8 *)node + sizeof(header_t));  
    }
    return ptr; 
}

void* reallocate ( void* ptr, u64 size ) {
    node_t* node = get_node(ptr);

    if ( get_size(node->header) <= size ) {
        fprintf(stderr, "Reallocation size needs to be bigger than previous size\n");
        return NULL; 
    }
    
    void* new_ptr = allocate(size);
    u8* aux1 = ptr;
    u8* aux2 = new_ptr; 
    for ( i64 i = 0; i < size; i++ ) aux2[i] = aux1[i];

    return new_ptr;  
}

void deallocate ( void* ptr ) {
    node_t* node = get_node(ptr);

    if ( get_status(node->header) ) {
        fprintf(stderr, "Double free operation\n");
        return; 
    }

    /*
        Before trying to merge nodes, we need to know the memory segments we're working with in order
        to avoid segmentation faults. By now, I think this is enough. How should I store the pairs of
        addresses that each page gave in add_mem_page()?   
     */
    
    set_status(&node->header, __free);
    set_color(&node->header, __red);
    insert(get_current_root(), node); 
}

static node_t** get_current_root ( void ) { /* will help to mange threads */
    return current_roots[0]->header == 0 ? NULL : &current_roots[0]; 
}

static u16 get_current_root_id ( void ) { /* main root is 0 */
    return 0; 
}
