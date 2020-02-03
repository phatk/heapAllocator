#include "allocator.h"
#include "debug_break.h"
#include <stdio.h>
#include <string.h>

#define USED 1
#define FREE 0
static void *beginning, *end;

// optional global variable that is not used in mymalloc, myrealloc, or my free
// but is included in validate_heap to ensure that the housekeeping inforomation is reasonable
// declare as void* to save memory instead of size_t
static void *e_heap; 

// a struct header that keeps track of the size and if the header is used or not
// if_used is 1 if it is used and 0 if it is freed
typedef struct header 
{
    int if_used, payload_size;
} Header;

#define SIZEOFHEADER sizeof(Header)

// same roundup function as in bump allocator
size_t roundup(size_t sz, size_t mult)
{
    return (sz + mult - 1) & ~(mult - 1);
}

bool myinit(void *start, size_t size)
{
    beginning = start;
    end = start;
    e_heap = &size;
    return true;
}

// helper function for mymalloc
// return true if the header is free and has enough space
bool header_free_enough_space(void *cur_header, size_t needed) 
{
    return (((Header*)cur_header)->if_used == FREE) && (needed <= ((Header*)cur_header)->payload_size);
}

// helper function for mymalloc
// update the header appropriately depending on if mymalloc is updating a free header or not
void *update_header(void *cur_header, bool malloc_free_header, size_t needed)
{
    ((Header*)cur_header)->if_used = USED;
    if (malloc_free_header) {
        // perform splitting
        // make sure that the leftover splitting is possible before doing so
        if (needed < ((Header*)cur_header)->payload_size) {
            int leftover = ((Header*)cur_header)->payload_size - needed - SIZEOFHEADER; 
            if(leftover >= ALIGNMENT) {
                ((Header*)cur_header)->payload_size = needed;
                Header *temp = (Header *) ((char *)cur_header + SIZEOFHEADER + needed);
                temp->payload_size = leftover;
                temp->if_used = FREE;
                return (char*)cur_header + SIZEOFHEADER;
            }
            // if remaining size is not enough, ignore the calculator for leftover and return normally as in
            // needed is equal to payload_size for that header
        }
        return (char*)cur_header + SIZEOFHEADER;
    } 
    ((Header*)cur_header)->payload_size = needed;
    end = (char*)cur_header + needed + SIZEOFHEADER;
    return (char*)cur_header + SIZEOFHEADER;
}

void *mymalloc(size_t requestedsz)    
{
    requestedsz = roundup(requestedsz, ALIGNMENT);
    void *cur_header = beginning;
    // traverse all the used header until the end of last header in the heap
    // pass a boolean to indicate if update a header that is free and large enough or create a new header at the end
    // this is important because you do not update the end if you are inserting into a free header
    while(cur_header < end) {
        if(header_free_enough_space(cur_header, requestedsz)) {
            return update_header(cur_header, true, requestedsz);
        }
        cur_header = (char*)cur_header + SIZEOFHEADER + ((Header*)cur_header)->payload_size;
    }
    return update_header(cur_header, false, requestedsz);
}

void myfree(void *ptr)
{
    if (!ptr) return; // edge case when myfree an invalid pointer
    ptr = (char*)ptr - SIZEOFHEADER;
    ((Header*)ptr)->if_used = FREE;
}

// myrealloc only occurs if you are reallocing into a bigger size. if myrealloc a smaller size, return the 
// same pointer
void *myrealloc(void *oldptr, size_t newsz)
{   
    if (!oldptr) return mymalloc(newsz); // edge case when myrealloc a new pointer.
    if (newsz == 0) {  // edge case when myrealloc a size of 0
        myfree(oldptr);
        return NULL;
    }
    Header *temp = (Header *) ((char *)oldptr - SIZEOFHEADER);
    if (newsz <= temp->payload_size) return oldptr; // edge case when myrealloc a smaller size
    void *newptr = mymalloc(newsz);
    memcpy(newptr, oldptr, temp->payload_size);
    myfree(oldptr);
    return newptr;
}

bool validate_heap()
{
    void *cur_header = beginning;
    int total_heap = 0;
    // check that you can traverse from the beginning to the end and that the total_heap
    // is reasonable
    while (cur_header != end) {
        cur_header = (char*)cur_header + SIZEOFHEADER + ((Header*)cur_header)->payload_size;
        total_heap += + ((Header*)cur_header)->payload_size;
    }
    if (cur_header != end) {
        printf("You traverse past the last available space after header! Bug alert!\n");
        breakpoint();
        return false;
    }
    if (total_heap > (*(size_t*) e_heap)) {
        printf("Size of total_heap is not reasonable. You did something wrong.\n");
        breakpoint();
        return false;
    }
    return true;
}

