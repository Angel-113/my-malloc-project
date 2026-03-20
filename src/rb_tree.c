#include "../include/base.h"
#include "../include/rb_tree.h"
#include "header.h"
#include <stdbool.h>
#include <stdio.h>

static node_t __sentinel_value = {
  0, &__sentinel_value, &__sentinel_value, &__sentinel_value
};

node_t* __sentinel = &__sentinel_value; 

typedef enum ChildKind {
    LEFT,
    RIGHT,
    ROOT
} ChildKind; 

/* Some helpers */

static void set_footer ( node_t* node ); 
static header_t* get_footer ( node_t* node );
static node_t* get_substitute ( node_t* node );  
static void disconnect_node ( node_t* node ); 
static void left_rotate ( node_t** root, node_t* node );
static void right_rotate ( node_t** root, node_t* node );
static void left_right_rotate ( node_t** root, node_t* node );
static void right_left_rotate ( node_t** root, node_t* node ); 
static void fix_deletion ( node_t** root, node_t* result, bool black_token );
static void fix_subtree ( node_t** root, node_t* current_subtree ); 
static void swap_nodes ( node_t** root,  node_t* inorder, node_t* node ); /* Just changes the tree hierarchy */
static ChildKind get_child_kind ( node_t* node ); 

/* Implementations */

node_t* insert ( node_t** root, node_t* new_node ) { /* top-down insertion */
    u64 target = get_size(new_node->header);
    node_t* current = *root;
    node_t* parent = __sentinel; 

    while ( current != __sentinel ) {
        if ( get_color(current->left->header) && get_color(current->right->header) ) {
            set_color(&current->header, __red);
            set_color(&current->left->header, __black);
            set_color(&current->right->header, __black);
            if ( current == *root ) set_color( &(*root)->header, __black );
            else if ( get_color(current->parent->header) ) fix_subtree(root, current);
        }
        parent = current; 
        current = target < get_size(current->header) ? current->left : current->right;
    }

    /* insert node */
    node_t** child = target < get_size(parent->header) ? &parent->left : &parent->right;
    *child = new_node;
    (*child)->parent = parent;
        
    /* perform last fix if needed */
    if ( get_color((*child)->parent->header) )
        fix_subtree(root, *child);  

    set_color(&(*root)->header, __black); 
    
    return new_node;     
}

node_t* delete ( node_t** root, node_t* node ) { /* bottom-up deletion */
    if ( !root || *root == __sentinel ) {
        print_error("Corrupted tree: root == NULL\n");
        return NULL; 
    }

    else if ( !node || node == __sentinel ) {
        print_error("Cannot delete a NULL pointer\n");
        return NULL; 
    }

    node_t* result = get_substitute(node);
    node_t* parent = node->parent;
    node_t** new_child = node == *root ?
                        &(*root) : (parent->left == node ? &parent->left : &parent->right); 
    
    bool black_token = !get_color(node->header); 
 
    if ( result == __sentinel ) { /* no child */
        (*new_child) = __sentinel; 
        disconnect_node(node); 
    }
    else if ( result == node->left || result == node->right ) { /* one child */
        (*new_child) = result;
        (*new_child)->left = result->left;
        (*new_child)->right = result->right;
        (*new_child)->parent = new_child == &(*root) ? __sentinel : parent; 
        disconnect_node(node);
    }
    else { /* two child */
        swap_nodes(root, result, node);
        delete(root, node); 
    }

    fix_deletion(root, result, black_token);
        
    return node; 
}

node_t* init_node ( void* ptr, u64 size, bool color, bool status ) { /* init node assumes that ptr will be node's address */
    node_t* node = ptr;
    set_color(&node->header, color);
    set_status(&node->header,  status);
    set_size(&node->header, size);
    set_footer(node);
    node->parent = __sentinel;
    node->left = __sentinel;
    node->right = __sentinel;
    return node; 
}

node_t* search ( node_t* root, u64 target ) { /* best fit algorithm */
    node_t* current = root;
    while ( current->left != __sentinel && current->right != __sentinel ) {
        if ( get_size(current->header) == target ) break; 
        current = target < get_size(current->header) ? current->left : current->right;  
    }
    return current; 
}

node_t* get_node ( void* ptr ) { /* get_node assumes ptr = (u8 *)original_node + sizeof(Header); */
    return (node_t *)((u8 *)ptr - sizeof(header_t)); 
}

node_t* merge_nodes ( node_t* a, node_t* b ) { /* merge_nodes assumes a and b are free memory contiguous nodes */
    node_t* left = a < b ? a : b;
    node_t* right = left == a ? b : a; 

    if ( right != get_next_node(left) ) {
        print_error("Trying to merge non-memory-contiguous nodes\n");
        return __sentinel;  
    }
    
    if ( get_status(a->header) || get_status(b->header) ) {
        print_error("Trying to merge non-free nodes\n");
        return __sentinel; 
    }

    u64 new_size = get_size(a->header) + get_size(b->header) + 2 * sizeof(header_t); 
    left = init_node(left, new_size, __red, __free); 

    return left; 
}

node_t* get_next_node ( node_t* node ) {
    return (node_t *)( (u8 *)get_footer(node) + sizeof(header_t) ); 
}

node_t* get_prev_node ( node_t* node ) {
    header_t* prev_footer = (header_t *)((u8 *) node - sizeof(header_t));
    return (node_t *) ((u8 *)prev_footer - sizeof(header_t) - get_size(*prev_footer));
}

/* Helper implementations */

void set_footer (node_t* node) { /* A node's footer must be equal to its header */
    header_t* footer = get_footer(node);
    *footer = node->header; 
}

static header_t* get_footer ( node_t* node ) {
    return (header_t *)( (u8 *)node + sizeof(header_t) + get_size(node->header) );
}

static void disconnect_node ( node_t* node ) {
    node->parent = node->left = node->right = __sentinel; 
}

static ChildKind get_child_kind( node_t* child ) {
    return child->parent == __sentinel ? ROOT : ( child->parent->left == child ? LEFT : RIGHT ); 
}

static void left_rotate ( node_t** root,  node_t* node ) { /* perform rotation by pushing node down and indirect linking */
    node_t* current_parent = node;
    node_t* current_right = node->right;
    node_t* new_right = current_right->left; 

    node_t** new_node = node != *root ?
                        ((node->parent)->left == node ? &(node->parent)->left : &(node->parent)->right) : root;
    
    *new_node = current_right;
    (*new_node)->left = current_parent;
    (*new_node)->parent = current_parent->parent;

    current_parent->parent = *new_node;
    current_parent->right = new_right;
    new_right->parent = current_parent; 
}

static void right_rotate ( node_t** root, node_t* node ) {
    node_t* current_parent = node;
    node_t* current_left = node->left;
    node_t* new_left = current_left->right;

    node_t** new_node = node != *root ?
                        ((node->parent)->left == node ? &(node->parent)->left : &(node->parent)->right) : root;
    
    *new_node = current_left;
    (*new_node)->right = current_parent;
    (*new_node)->parent = current_parent->parent;

    current_parent->parent = *new_node;
    current_parent->left = new_left;
    new_left->parent = current_parent;
}

static void left_right_rotate ( node_t** root, node_t* node ) {
    left_rotate(root, node->left);
    right_rotate(root, node);
}

static void right_left_rotate ( node_t** root, node_t* node ) {
    right_rotate(root, node->right); 
    left_rotate(root, node);
}

static void swap_nodes ( node_t** root,  node_t* inorder, node_t* node ) { /* inorder->left is __sentinel */
    node_t* node_parent = node->parent;
    node_t* node_left = node->left;
    node_t* node_right = node->right;
    node_t* inor_parent = inorder->parent;
    node_t* inor_right = inorder->right;

    bool color_node = get_color(node->header);
    bool color_inor = get_color(inorder->header); 

    node_t** new_node = node_parent != __sentinel ?
                    (node_parent->left == node ? &node_parent->left : &node_parent->right) : root;
    
    node_t** new_inorder = inor_parent != node ? 
                        (inor_parent->left == inorder ? &inor_parent->left : &inor_parent->right) : &inorder;

    /* perform swap by indirect linking */
    (*new_node) = inorder;
    (*new_inorder) = node; 

    (*new_node)->parent = node_parent; 
    (*new_node)->right = node_right == inorder ? *new_inorder : node_right;
    (*new_node)->left = node_left;

    (*new_inorder)->parent = inor_parent != node ? inor_parent : *new_node; 
    (*new_inorder)->right = inor_right;
    (*new_inorder)->left = __sentinel;

    /* preserve color: if node was black, then, new_node is black  */
    set_color(&(*new_inorder)->header, color_inor);
    set_color(&(*new_node)->header, color_node); 
}

static node_t* get_substitute ( node_t* node ) {
    node_t* result = __sentinel; 
    if ( node->left != __sentinel && node->right != __sentinel ) {
        result = node->right;
        while ( result->left != __sentinel ) result = result->left;
    }
    else if ( node->left != __sentinel || node->right != __sentinel ) 
        result = node->left != __sentinel ? node->left : node->right; 
    
    return result; 
}

static void fix_deletion ( node_t** root, node_t* result, bool black_token ) { /* bottom-up deletion */
    node_t* current = result;
    
    while ( current != *root && black_token ) { /* push the black_token up the tree */
        node_t* sibling = current->parent->left == current ? current->parent->right : current->parent->left;        
        node_t* grandparent = current->parent->parent;

        if ( current == __sentinel ) break;  /* reach the root */
        
        bool color_parent = get_color(current->parent->header);
        bool color_sibling = get_color(sibling->header);
        bool color_current = get_color(current->header); 
        ChildKind current_kind = get_child_kind(current);

        if ( get_color(current->header) ) { /* found a red node */
            set_color(&current->header, __black);
            black_token = !black_token; 
        }
        else if ( get_color(sibling->header) ) { /* double black violations and red sibling */
            set_color(&(current->parent)->header, __red);
            set_color(&sibling->header, __black); 
            current_kind == LEFT ? left_rotate(root, current->parent) : right_rotate(root, current->parent); 
            continue; 
        }
        else if ( get_color(sibling->left->header) || get_color(sibling->right->header) ) { /* double black violatons and black sibling */
            node_t** near = current_kind == LEFT ? &sibling->left : &sibling->right;
            node_t** far = current_kind == LEFT ? &sibling->right : &sibling->left;  

            if ( !get_color((*far)->header) ) 
                current_kind == LEFT ? right_rotate(root, sibling) : left_rotate(root, sibling); 

            set_color(&sibling->header, get_color(current->parent->header));
            set_color(&current->parent->header, __black);
            set_color(&(*far)->header, __black); 
                        
            current_kind == LEFT ? left_rotate(root, current->parent) : right_rotate(root, current->parent); 
            black_token = !black_token;       
        }
        else { /* black sibling and two black nephews */
             set_color(&sibling->header, __red);
             if ( get_color(sibling->parent->header) ) {
                set_color(&(sibling->parent)->header, __black);
                black_token = !black_token;
            } 
            current = sibling->parent;
            continue; 
        }
        
        current = grandparent; 
    } 
}

static void fix_subtree ( node_t** root, node_t* current_subtree ) { /* fix insertion before inserting */
    node_t* parent = current_subtree->parent;
    node_t* grandpa = parent->parent;
    node_t* sibling = parent == grandpa->left ? grandpa->right : grandpa->left; 

    if ( get_color(current_subtree->parent->header) && !get_color(sibling->header) ) { /* perform fixes */
        ChildKind current_kind = get_child_kind(current_subtree);
        ChildKind parent_kind = get_child_kind(current_subtree->parent);

        if ( current_kind == parent_kind ) { /* straight line */
            set_color(&parent->header, __black);
            set_color(&grandpa->header, __red); 
            current_kind == LEFT ? right_rotate(root, grandpa) : left_rotate(root, grandpa);
        }
        else { /* zig-zag */
            set_color(&grandpa->header, __red);
            set_color(&current_subtree->header, __black);  
            current_kind == LEFT ? right_left_rotate(root, grandpa) : left_right_rotate(root, grandpa);  
        }
    }    
}
