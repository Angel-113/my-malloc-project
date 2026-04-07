#include "../include/allocator.h"
#include "../include/base.h"
#include "../include/rb_tree.h"
#include <sys/mman.h>

#define PAGES( size ) (((size) + (PAGE - 1)) & ~(PAGE - 1))
#define PAGE 4092
#define MAX_THREADS 10

static u32 id_current_root = 0; 
static node_t* current_roots[ MAX_THREADS ] = { 0 }; 

static node_t** get_current_root ( void );
static u16 get_current_root_id ( void ); 
static bool memcopy ( void* src, void* dest, u64 size );

static node_t* add_mem_page( u64 size ) { /* syscall for mempages */
    return  mmap(NULL, PAGES(size), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

void* allocate ( u64 size ) { 
    void* ptr = NULL;
    node_t** root = get_current_root(); 

    if ( !root ) {
        *root = add_mem_page( size );
        node_t* used_node = init_node( *root, size, __red, __in_use );
        *root = (node_t *)((u8 *)used_node + size + 2 * sizeof(header_t));
        *root = init_node(*root, PAGES(size) - (size + 4 * sizeof(header_t)), __black, __free );  
        ptr = (u8 *)used_node + sizeof(header_t);
        current_roots[0] = *root; 
    }
    else if ( (*root)->left == (*root)->right ) { /* still splitting memory */
        ptr = (u8 *)root + sizeof(header_t);
        u64 new_size = get_size( (*root)->header ) - size;
        bool status = get_status( (*root)->header );
        bool color = get_color( (*root)->header );

        if ( new_size <= size ) { /* last split in the memory segment */
            node_t* new_node = init_node(root, new_size - sizeof(header_t), __red, __free);  
            *root = add_mem_page(new_size);
            *root = init_node(root, PAGES(size), __black, __free); 
            insert( root, new_node ); 
        }
        
        else  { /* main split */
            *root = (node_t *)( (u8*) root + 2 * sizeof(header_t) + size );
            *root = init_node( *root, size, color, status ); 
        }

        current_roots[get_current_root_id()] = *root; /* update the root */
    }
    else { /* now we search in a free list */
        node_t* result = search (*root, size);
        delete(root, result);
        ptr = (u8 *)result + sizeof(header_t); 
    }

    return ptr; /* let the user handle the NULL case */  
}

void* reallocate ( void* ptr, u64 size ) {
    void* new_ptr = allocate ( size ); 
    if ( !new_ptr ) {
        print_error("Malloc function returned NULL ptr\n"); 
        return NULL; 
    }
    return memcopy(ptr, new_ptr, size) ? new_ptr : NULL; /* let the user handle the NULL case */ 
}

void deallocate ( void* ptr ) {
    node_t* node = (node_t *) ((u8*) ptr - sizeof(header_t));

    if ( get_status(node->header) ) {
        print_error("Double free operation\n"); 
        return;     
    }

    set_status(&node->header, __free);

    node_t* prev_node = get_prev_node(node); 
    node_t* next_node = get_next_node(node);

    /* merge nodes */    
    node_t* merged_node = __sentinel;
    if ( get_status(prev_node->header) ) {
        delete(get_current_root(), prev_node);
        merged_node = merge_nodes(prev_node, node); 
    }

    if ( get_status(next_node->header) ) {
        delete(get_current_root(), next_node);
        merged_node = merged_node == __sentinel ? merge_nodes(node, next_node) : merge_nodes(merged_node, next_node); 
    }

    if ( merged_node == __sentinel ) insert(get_current_root(), node);
    else insert(get_current_root(), merged_node); 
}

static bool memcopy( void* src, void* dest, u64 size ) { 
    u8* src_aux = src;
    u8* dest_aux = dest;

    if ( src + size == dest || dest + size == src ) {
        print_error("overlapping memory segments\n");
        return false; 
    }
    
    for ( u64 i = 0; i < size; i++ ) dest_aux[i] = src_aux[i];  

    return true;
}

node_t** get_current_root ( void ) { /* will help to mange threads */
    return current_roots[0]->header == 0 ? NULL : &current_roots[0]; 
}

u16 get_current_root_id ( void ) { /* main root is 0 */
    return 0; 
}
