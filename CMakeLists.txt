cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(MyMalloc C)

# Define the Core Library
add_library(malloc_core
    src/base.c
    src/allocator.c
    src/header.c
    src/rb_tree.c
)

# Allow other targets to see the 'include' folder automatically
target_include_directories(malloc_core PUBLIC include)

# Define the Test Executable
add_executable(run_tests
    test/test.c
    test/rand.c
    test/rb_tree_test.c
    test/allocator_test.c
)

# Link the executable to your malloc core
target_link_libraries(run_tests PRIVATE malloc_core)

# Testing
set(NODES_TO_INSERT 750000 CACHE STRING "Number of nodes to insert in tree tests")
set(NODES_TO_DELETE 500000 CACHE STRING "Number of nodes to delete in tree tests")

target_compile_definitions(run_tests PRIVATE
    NODES_TO_INSERT=${NODES_TO_INSERT}
    NODES_TO_DELETE=${NODES_TO_DELETE}
)

enable_testing()
add_test(NAME tree_general COMMAND run_tests "tree general")
add_test(NAME tree_insertion COMMAND run_tests "tree insertion")
add_test(NAME tree_deletion COMMAND run_tests "tree deletion")
add_test(NAME tree_performance COMMAND run_tests "tree performance")
