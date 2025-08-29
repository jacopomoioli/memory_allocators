/**
 * Free list Allocator w Next Fit
 */

#include <unistd.h>
#include <stdio.h>

#define DEBUG 1

#if DEBUG
#define DEBUG_PRINT(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif

#define OS_ALLOCATION_SIZE 1024

typedef long Align;

union header{
    struct{
        union header *next_block;
        unsigned size;
    } s;
    Align x;    // useless, just a trick to force the correct alignment of the union
};

typedef union header Header;

static Header base;
static Header* free_block = NULL;
static int handled_memory = 0;


static void memfree(void* block){
    Header* header = block - 1;
    DEBUG_PRINT("Freeing block %p of size %d", block, header->s.size);

    if (free_block == NULL){    // free list is blank
        header->s.next_block = &base;
        base.s.next_block = header;
        free_block = &base;
        return;
    }
    header->s.next_block = free_block->s.next_block;
    free_block->s.next_block = header;
    return;
}

static Header* request_memory(unsigned bytes_required) {
    DEBUG_PRINT("Requested %d bytes of memory from the OS", bytes_required);
    void* new_memory_pointer;
    Header* new_memory_header;
    if (bytes_required < OS_ALLOCATION_SIZE){
        bytes_required = OS_ALLOCATION_SIZE;
    }
    new_memory_pointer = sbrk(bytes_required * sizeof(Header));
    if (new_memory_pointer == (char *) -1){
        return NULL;
    }
    new_memory_header = (Header*)new_memory_pointer;
    new_memory_header->s.size = bytes_required;     // bytes_required is memalloc's needed_units, which includes the header size
    memfree((void*)new_memory_header+1);
    handled_memory += bytes_required;
    return free_block;
}

void* memalloc(unsigned bytes_required){
    Header* current_block;
    Header* previous_block;
    unsigned needed_units;

    DEBUG_PRINT("Asked block of %d bytes (payload)", bytes_required);

    needed_units = (bytes_required + sizeof(Header) - 1) / sizeof(Header) + 1;

    previous_block = free_block;
    if (previous_block == NULL) {     // first time we call memalloc
        DEBUG_PRINT("Calling memalloc for the first time");
        /**
         * free_block is the pointer to the position of the last successfully visited block.
         * it's used as the starting block for each next memalloc() instead of starting from the dummy node, hence the name "next-fit"
         * even if for some reason on K&R this is called "first-fit"
         * 
         * at the beginning, it's equal to the address of base.
         * base is the "dummy node", the first element of a circular list.
         */
        base.s.next_block = free_block = previous_block = &base;
        base.s.size = 0;
        DEBUG_PRINT("Base block: %p", &base);
        DEBUG_PRINT("Next block: %p", (void*)base.s.next_block);
    }

    DEBUG_PRINT("Starting free-list iteration");
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
        DEBUG_PRINT("current block: %p, size: %d", current_block, current_block->s.size);
        if (current_block->s.size == needed_units) {
            DEBUG_PRINT("current block has the same size of needed units");
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
            DEBUG_PRINT("current block is bigger than needed units");
            
            current_block->s.size = current_block->s.size - needed_units;
            DEBUG_PRINT("[splitting]:unused block: %p, size: %d", current_block, current_block->s.size);
            current_block += current_block->s.size; // current block pointer shifted to the end of the non-required memory space, so at the start of the allocated block's header
            current_block->s.size = needed_units;   // current block size is updated
            DEBUG_PRINT("[splitting]:used block: %p, size: %d", current_block, current_block->s.size);
            
            // at this point current_block is the header of the allocated block.

            free_block = previous_block;    // the next available free block is the original block, that now is deprived of the allocated space
            
            return (void *)(current_block + 1); // skip the header
        } else {
            DEBUG_PRINT("current block is smaller than needed units");
        }

        if (current_block == free_block) {  // got back to the last allocated block, iterated over the entire circular list
            DEBUG_PRINT("completely iterated the free list without finding any block of sufficient size. Entire handled memory: %d", handled_memory);
            if ((current_block = request_memory(OS_ALLOCATION_SIZE)) == NULL){
                DEBUG_PRINT("request_memory failed. no available memory");
                return NULL;
            }
        }
    }

}


int main(int argc, char** argv){
    void* ptr1 = memalloc(10);
    void* ptr2 = memalloc(20);
    void* ptr3 = memalloc(2048);
    printf("ptr1: %p\n", ptr1);
    printf("ptr2: %p\n", ptr2);
    printf("ptr3: %p\n", ptr3);
    memfree(ptr1);
}