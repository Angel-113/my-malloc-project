#include "../include/header.h"

#define MSB ((sizeof(header_t) * 8) - 1) /* most significant bit */
#define SECOND_MSB (MSB - 1)

u64 get_size ( header_t header ) {
    u64 size = header;
    size = size & ~( (u64) 1 << MSB ) ; 
    return size & ~( (u64) 1 << SECOND_MSB ); 
}

bool get_color ( header_t header ) { /* second MSB */
    return header >> SECOND_MSB & 1;
}

bool get_status ( header_t header ) { /* MSB */
    return header >> MSB & 1;
}

void set_size ( header_t* header, u64 size ) {
    if ( get_color (size) || get_status(size) ) {
        print_error("Size can't have 63th or 64th bits on\n");
        return; 
    }
    
    header_t current_header = size; 
    set_color( &current_header, get_color(*header) );
    set_status( &current_header, get_status(*header) );
    *header = current_header;  
}

void set_color ( header_t* header, bool color ) {
    *header = (*header & ~((u64) 1 << SECOND_MSB) | ((u64)color << SECOND_MSB));
}

void set_status ( header_t* header, bool status ) {
    *header = (*header & ~((u64) 1 << MSB) | ((u64)status << MSB)); 
}
