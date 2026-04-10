#include "../include/rb_tree_test.h"
#include "../include/allocator_test.h"

void general_test ( void ) {    
    bool test = tree_test();
    if ( test ) tree_test_performance();
    example();     
}

int main( void ) {
    example();  
    return 0; 
}
