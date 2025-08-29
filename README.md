# Memory Allocators

## Linear Allocator
Allocates memory sequentially from a fixed buffer. Memory must be freed in LIFO order.
Uses a single pointer that advances for alloc() and resets on afree().

Inspired by K&R chapter 5.4.


## Free List Allocator w Next Fit
Allocates memory by organizing free memory in blocks, and managing a free block list. Uses the next fit strategy to choose which block to allocate.
Splits bigger-than-required blocks, and asks more heap from the OS via sbrk() when the free list is exhausted.

Inspired by K&R chapter 8.7.

TODOs:
- Coalescing function inside free
- Implement best fit strategy