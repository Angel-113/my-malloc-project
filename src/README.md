
## Helper module: `header`

This works as metadata of every node.

- `extern header_t init_header ( u64 size, bool color, bool status )`. Initializes the metadata inside a `unsigned long long` integer.
- `extern u64 get_size ( header_t header )`. Returns the header's 62 least significant bits formatted as a 64 bit integer.
- `extern bool get_color ( header_t header )`. Returns the color: `true` if it is red and `false` otherwise. It is a bitwise operation on the second MSB.
- `extern bool get_status ( header_t header )`. Returns the status: `true` if it is `__free` and `false` if it is `__in_use`. It is a bitwise operation on the MSB.
- `extern void set_status ( header_t* header, bool status )`. Modifies the current header status to store the value `status`.
- `extern void set_color ( header_t* header, bool color )`. Modifies the current header color to hold the given one `color`.
- `extern void set_size ( header_t* header, u64 size )`. Modifies the current header size to store the given `size`. 

## Core module: `rb_tree`

The functions provided by this red-black tree implementation.

- `extern node_t* ìnsert ( node_t** root, node_t* node )`. An implementation of the *top-down* insertion method. Time complexity: $O(log(n))$.
- `èxtern node_t* delete ( node_t** root, node_t* node )`. An implementation of the *bottom-up* deletion method. Time complexity: $O(log(n))$.
- `extern node_t* init_node ( void* ptr )`. This function takes the memory address `(unsigned long long) ptr` and from there, it initialize a `node_t*`.
- `extern node_t* get_next_node ( node_t* node )`. Given a node, it returns the immediate next node in the contiguous memory block.
- `extern node_t* get_prev_node ( node_t* node )`. Given a node, it returns the immediate previous node in the contiguous memory block.
- `extern node_t* merge_nodes ( node_t* a, node_t* b )`. Given two memory contiguous nodes, this function returns a merged node of size `get_size(a->header) + get_size(b->header) + 2 * sizeof(header_t)`. 

