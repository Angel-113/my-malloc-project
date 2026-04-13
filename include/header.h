#ifndef HEADER_H
#define HEADER_H

#include "base.h"

typedef u64 header_t;

extern bool get_color(header_t header);
extern bool get_status(header_t header);
extern u64 get_size(header_t header);

extern void set_color(header_t *header, bool color);
extern void set_status(header_t *header, bool status);
extern void set_size(header_t *header, u64 size);

#define __red 1
#define __black 0
#define __free 1
#define __in_use 0

#endif
