#include "../include/base.h"
#include <stdio.h>

void print_error(const char *error) { fprintf(stderr, error); }

i64 cmpt_array_grwth_factor(u64 n) { return ((n + (n >> 1)) / n); }
