# Memory Allocators

## Linear Allocator
Allocates memory sequentially from a fixed buffer. Memory must be freed in LIFO order.
Uses a single pointer that advances for alloc() and resets on afree().

Inspired by K&R chapter 5.4.


## Free List Allocator w Next Fit
Inspired by K&R chapter 8.7

Few improvements:
- Allocation and free calls don't have to follow LIFO
- Initial memory buffer taken from OS, not from a limited-length array

Free memory is managed with a free list containing memory blocks.

Each block stores:
- size
- next block pointer
- memory area

The last block points to the first one.

When a memory allocation is requested, the allocator iterates over the free-list until the **first** block with a sufficient size is found, starting from the position of the latest free block found. If the found block is too big, split the block and only the required size is allocated. If no blocks are available, a new memory section is asked to the OS, and it will be inserted into the list. 

The release of a block raises an iteration of the list, with the goal of finding the right position to insert the freed block in. If two free blocks are adjacent, those will be merged.

The memory returned by the allocator must me correctly-aligned to the type of object it must contain. If you manage to align the memory for the most restrictive type (the highest one) you are okay for all the other types.

In order to align everything, every block starts with a header (union to handle the alignment).
Every block allocated must be a multiple of the header and is a multiple of its size. The function will return the pointer to the free space, just after the header.