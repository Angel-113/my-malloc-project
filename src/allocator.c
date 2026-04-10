#include "../include/allocator.h"
#include "../include/rb_tree.h"
#include "header.h"

#include <limits.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

static u64 page_size = 0; 
static const header_t PAGE_HEADER = 0;
static const header_t PAGE_FOOTER = LLONG_MAX; 

#define PAGES( size ) (((size) + (page_size - 1)) & ~(page_size - 1))
#define MAX_THREADS 10
#define MIN_SEG_SIZE 3 * sizeof(header_t)

static u32 id_current_root = 0; 
static node_t* current_roots[ MAX_THREADS ] = { 0 }; 

static node_t** get_current_root ( void );
static u16 get_current_root_id ( void ); 
static void memcopy ( void* src, void* dest, u64 size );
static void* add_mem_page( u64 n ); 

static node_t* split_node ( node_t** a, u64 size ); 
static void* search_node ( node_t** root, u64 size ); 
static void* create_root ( node_t** root, u64 size ); 
static void* split_segment ( node_t** root, u64 size );
static node_t* init_page ( u64 n );

static void* standard_allocation ( u64 size );
static void* minimum_allocation ( u64 size );
static void* large_allocation ( u64 size ); 

void* allocate ( u64 size ) {
     page_size = !page_size ? sysconf(_SC_PAGESIZE) : page_size;      
     node_t** root = get_current_root();
     void* ptr = NULL;
     if ( !(*root) ) ptr = create_root(root, size);
     else if
     ( (*root)->left != __sentinel || (*root)->right != __sentinel )
         ptr = search_node(root, size); 
     else ptr = split_segment(root, size); 
     return ptr; 
}

static void* create_root ( node_t** root, u64 size ) {
    node_t* page = init_page(1);

    if ( page == __sentinel ) {
        fprintf(stderr, "Not enough memory in the system\n");
        return NULL; 
    }
     
    node_t* node = (node_t*)((u8 *)page + sizeof(header_t));
    node = init_node(node, size, __red, __in_use);

    u64 root_size = page_size - (size + 6 * sizeof(header_t));
    *root = init_node(get_next_node(node), root_size, __black, __free);  

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
    u64 curr_seg_size = get_size((*root)->header) + 2 * sizeof(header_t);
    node_t* node = NULL; 

    if ( curr_seg_size - (size + 4 * sizeof(header_t)) >= MIN_SEG_SIZE )
        node = split_node(root, size);      
    else {
        node = *root;
        
        node_t* new_page = init_page(1);
        u64 root_size = page_size - 4 * sizeof(header_t);

        *root = (node_t*)((u8 *)new_page + sizeof(header_t));
        *root = init_node(*root, root_size, __black, __free); 
    }
    
    return (void *)((u8 *)node + sizeof(header_t)); 
}

void* reallocate ( void* ptr, u64 size ) {
    node_t* node = get_node(ptr);

    if ( get_size(node->header) <= size ) {
        fprintf(stderr, "Reallocation size needs to be bigger than previous size\n");
        return NULL; 
    }
    
    void* new_ptr = allocate(size);
    memcopy(ptr, new_ptr, get_size(node->header)); 

    return new_ptr;  
}

void deallocate ( void* ptr ) {
    node_t* node = get_node(ptr);

    node_t* node_to_merge = __sentinel;
    node_t* merged_node = __sentinel; 
    node_t** root = get_current_root();

    printf("Root status: %llu\n", get_size((*root)->header));
        
    if ( get_status(node->header) ) {
        fprintf(stderr, "Double free operation\n");
        return; 
    }

    header_t* header_prev = (header_t *)((u8 *)node - sizeof(header_t));
    header_t* header_next = (header_t *)((u8 *)node + get_size(node->header) + sizeof(header_t));

    bool left = *header_prev == PAGE_HEADER ? false : true;
    bool right = *header_next == PAGE_FOOTER ? false : true;

    bool left_is_root = left ? (get_prev_node(node) == *root ? true : false) : false;
    bool right_is_root =
        right ? left_is_root ? false : (get_next_node(node) == *root ? true : false) : false;  
    bool node_is_root = node == *root ? true : false; 
    
    if ( left && right ) { /* merge three nodes */
        delete(root, get_prev_node(node));
        delete(root, get_next_node(node));
        merged_node = merge_nodes(merge_nodes(node, get_prev_node(node)), get_next_node(node)); 
    }
    else node_to_merge = left ? get_prev_node(node) : right ? get_next_node(node) : __sentinel;

    if ( node_to_merge != __sentinel ) {
        delete(root, node_to_merge); 
        merged_node = merge_nodes(node, node_to_merge);          
    }
    
    if ( merged_node != __sentinel ) node = merged_node; 

    if( (left_is_root || right_is_root || node_is_root) && merged_node != __sentinel )
        *root = merged_node;
        
    set_status(&node->header, __free);
    set_color(&node->header, __red);

    insert(get_current_root(), node);

    if ( *root == __sentinel ) puts("Root is __sentinel");
}

static node_t* split_node ( node_t** node, u64 size ) {
    u64 prev_size = get_size((*node)->header) + 2 * sizeof(header_t);
    bool color = get_color((*node)->header);

    size = size < MIN_SEG_SIZE ? MIN_SEG_SIZE : size; 
    node_t* new_node = init_node(*node, size, __red, __in_use);
    u64 new_size = prev_size - (size + 4 * sizeof(header_t));
    *node = init_node(get_next_node(new_node), new_size, color, __free);

    return new_node; 
}

static node_t** get_current_root ( void ) { /* will help to mange threads */
    return &current_roots[get_current_root_id()]; 
}

static u16 get_current_root_id ( void ) { /* main root is 0 */
    return 0; 
}

static node_t* init_page ( u64 n ) {
    void* start = add_mem_page(n);

    if ( !start ) return __sentinel;
    
    u64 size = page_size * n - 2 * sizeof(header_t);
    node_t* node = init_node(start, size, __red, __free);
    node->header = PAGE_HEADER;
    header_t* footer = (header_t*)((u8 *)get_next_node(node) - sizeof(header_t));
    *footer = PAGE_FOOTER;  

    return node; 
}

static void* add_mem_page( u64 n ) { /* syscall for mempages */
    return mmap(NULL, 1, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

/*
    TODO: Work on optimizations for copying memory segments based
    on 64-bit, 32-bit, 16-bit and 8-bit chunks. 
*/

static void memcopy ( void* src, void* dest, u64 size ) {
    u64 res_8 = (size - (size % 8)) / 8;
    u64 res_4 = (res_8 - (res_8 % 4)) / 4;
    u64 res_2 = (res_4 - (res_4 % 2)) / 2;

    u32* start = (u32 *)((u64 *)src + res_8);

    for ( i64 i = 0; i < 8 * res_8; i++ ) ((u64 *)dest)[i] = ((u64 *)src)[i];      
}
