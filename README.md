# Memory Allocators

## Linear Allocator
Allocates memory sequentially from a fixed buffer. Memory must be freed in LIFO order.
Uses a single pointer that advances for alloc() and resets on afree().

Inspired by R&K chapter 5.4.