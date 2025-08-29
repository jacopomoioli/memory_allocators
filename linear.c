/*
* Linear Allocator
*
* Allocates memory sequentially from a fixed buffer. Memory must be freed in LIFO order.
* Uses a single pointer that advances for alloc() and resets on afree()
*
*/

#include <stdio.h>

#define ALLOCSIZE 10000

static char allocbuf[ALLOCSIZE];
static char* allocp = allocbuf;

char* alloc(int size);
void afree(char* pointer);


char* alloc(int size){
    if((allocp + size) > (allocbuf + ALLOCSIZE)){
        return 0;
    }
    allocp += size;
    return allocp - size;
}

void afree(char* pointer){
    allocp = pointer;
}

int main(int argc, char** argv){
    char* pointer = alloc(10);
    printf("Base addr: 0x%p\nNext base addr: 0x%p\n\n", pointer, allocp);
    afree(pointer);
    printf("Base addr: 0x%p\nNext base addr: 0x%p\n\n", pointer, allocp);

    return 0;
}
