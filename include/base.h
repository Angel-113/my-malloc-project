#ifndef BASE_H
#define BASE_H

#include <stdbool.h>
#include <stdio.h>

typedef unsigned long long u64;
typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef char i8;
typedef short i16;
typedef long int i32;
typedef long long int i64;

extern i64 cmpt_array_grwth_factor(u64 n);
extern void print_error(const char *error);

#ifndef MALLOC_TEST_HOOK
#define MALLOC_TEST_HOOK 1
#endif

#endif
