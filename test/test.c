#include "../include/rb_tree_test.h"

int main( void ) {
    bool test = tree_test();
    if ( test ) tree_test_performance();
    return 0; 
}
