cmake_minimum_required(VERSION 3.22.1)

# Correct variable for C23 standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(MyMalloc C)

# 1. Define the Core Library
# This tells CMake to compile your sources into a library named 'malloc_core'
add_library(malloc_core
    src/base.c
    src/allocator.c
    src/header.c
    src/rb_tree.c
)

# Allow other targets to see the 'include' folder automatically
target_include_directories(malloc_core PUBLIC include)

# 2. Define the Test Executable
# You need to link the test runner (test.c) and the random number generator (rand.c)
add_executable(run_tests
    test/test.c
    test/rand.c
    test/rb_tree_test.c
    test/allocator_test.c
)

# Link the executable to your malloc core
target_link_libraries(run_tests PRIVATE malloc_core)

# 3. Enable CTest
enable_testing()
add_test(NAME MainTest COMMAND run_tests)
