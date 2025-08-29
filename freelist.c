/**
 * Free list Allocator w Next Fit
 */

#include <unistd.h>

#define NULL ((void*)0)
#define OS_ALLOCATION_SIZE 1024

typedef long Align;

union header{
    struct{
        union header *next_block;
        unsigned size;
    } s;
    Align x;    // useless, just to force the correct alignment of the union
};

typedef union header Header;

static Header base;

static Header* free_block = NULL;

static void free(Header* block){
    Header* header = block - 1;
    if (free_block == NULL){    // free list is blank
        header->s.next_block = &base;
        base.s.next_block = header;
        free_block = &base;
        return NULL;
    }
    header->s.next_block = free_block->s.next_block;
    free_block->s.next_block = header;
    return NULL;
}

static Header* request_memory(unsigned bytes_required) {
    void* new_memory_pointer;
    Header* new_memory_header;
    if (bytes_required < OS_ALLOCATION_SIZE){
        bytes_required = OS_ALLOCATION_SIZE;
    }
    new_memory_pointer = sbrk(bytes_required);
    if (new_memory_pointer == (char *) -1){
        return NULL;
    }
    new_memory_header = (Header*)new_memory_pointer;
    new_memory_header->s.size = bytes_required;     // bytes_required is memalloc's needed_units, which includes the header size
    free(new_memory_header+1);
    return free_block;
}

void* memalloc(unsigned bytes_required){
    Header* current_block;
    Header* previous_block;
    Header* request_memory(unsigned);
    unsigned needed_units;

    needed_units = (bytes_required + sizeof(Header) - 1) / sizeof(Header) + 1;

    previous_block = free_block;
    if (previous_block == NULL) {     // first time we call memalloc
        /**
         * free_block is the pointer to the next free block
         * at the beginning, it's equal to the address of base
         * base is the "dummy node", the first element of a circular list
         */
        base.s.next_block = free_block = previous_block = &base;
        base.s.size = 0;
    }

    /**
     * iterate the whole list until
     * 1) a block with a sufficient size is found, or
     * 2) a block with a sufficient size hasn't been found
     */
    for (
        current_block = previous_block->s.next_block;                                       
        ;                                                                                      
        previous_block = current_block, current_block = current_block->s.next_block
    ){
        if (current_block->s.size == needed_units) {
            /* the current block is exactly the size asked */
            
            previous_block->s.next_block = current_block->s.next_block;
            free_block = previous_block;
            return (void *)(current_block + 1); // skip the header, return the address of the first usable memory

        } else if (current_block->s.size > needed_units) { 
            /**
             * current block is bigger than the size asked 
             * it will be splitted in two:
             * - first part, remaining size, added the header and corrects the pointers to and from
             * - second part, size asked, removed from the list
            */

            current_block->s.size = current_block->s.size - needed_units;
            current_block += current_block->s.size; // current block pointer shifted to the end of the non-required memory space, so at the start of the allocated block's header
            current_block->s.size = needed_units;   // current block size is updated
            
            // at this point current_block is the header of the allocated block.

            free_block = previous_block;    // the next available free block is the original block, that now is deprived of the allocated space
            
            return (void *)(current_block + 1); // skip the header
        }

        if (current_block == free_block) {  // got back to the last allocated block, iterated over the entire circular list
            current_block = request_memory(needed_units);
            if (current_block == NULL){
                return NULL;
            }
        }
    }

}