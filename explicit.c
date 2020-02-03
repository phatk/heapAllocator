#include "allocator.h"
#include "debug_break.h"
#include <stdio.h>
#include <string.h>

#define USED 1
#define FREE 0
#define FREELIST_SIZE 2 * sizeof(Free_list*)
#define MINIMUM_FREE 16
#define SIZEOFHEADER 8
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

// another struct header that keeps trakc of the headers that are free
typedef struct Free_list
{
    struct Free_list* next;
    struct Free_list* prev;
} Free_list;

//new global variable
static Free_list *root_free_list;

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
    root_free_list = NULL;
    return true;
}

Header *get_header(Free_list *ptr)
{
    return (Header*)((char*)ptr - SIZEOFHEADER);
}

// a helper function in mymalloc
void delete_list(Free_list *ptr) 
{
    if ((ptr->prev == NULL) && (ptr->next == NULL)) root_free_list = NULL;
    // meaning that you are deleting a header that is the first on the list
    else if (ptr->prev == NULL) {
        root_free_list = ptr->next;
        root_free_list->prev = NULL;
    // meaning that you are deleting a header that is at the end of the list
    } else if (ptr->next == NULL) {
        ptr->prev->next = NULL;
    // meaning that you are deleting a header that is in between the list    
    } else {
        ptr->prev->next = ptr->next;
        ptr->next->prev = ptr->prev;
    }
}

// a helper function for myfree
void add_root(Free_list *ptr)
{
    if (!root_free_list) {
        root_free_list = ptr;
        root_free_list->next = NULL;
        root_free_list->prev = NULL;
    } else {
        ptr->next = root_free_list;     
        root_free_list->prev = ptr;
        root_free_list = ptr;
        root_free_list->prev = NULL;
    }
}

void coalesce(Header *temp)
{
    void *temp_right = (char *)temp + SIZEOFHEADER + temp->payload_size;
    while((temp_right < end) && (((Header*)temp_right)->if_used == 0)) {
        Free_list* list_next = (Free_list*)((char*)temp_right + SIZEOFHEADER);
        delete_list(list_next);
        temp->payload_size += ((Header*)temp_right)->payload_size + SIZEOFHEADER;
        temp_right = (char*)temp_right + ((Header*)temp_right)->payload_size + SIZEOFHEADER;
    }
}

// helper function for mymalloc
// update the header appropriately depending on if mymalloc is updating a free header or not
void *update_header(void *cur_header, bool malloc_free_header, size_t needed)
{
    ((Header*)cur_header)->if_used = USED;
    if (malloc_free_header) {
        // i did not re-implement splitting. not enough time to apply it to explicit
        return (char*)cur_header + SIZEOFHEADER;
    } 
    ((Header*)cur_header)->payload_size = needed;
    end = (char*)cur_header + needed + SIZEOFHEADER;
    return (char*)cur_header + SIZEOFHEADER;
}

void *mymalloc(size_t requestedsz)    
{
    requestedsz = roundup(requestedsz, ALIGNMENT);
    if (requestedsz < 16) requestedsz = 16;
    Free_list* cur_header = root_free_list;
    if (requestedsz < MINIMUM_FREE) requestedsz = MINIMUM_FREE;
    while (cur_header != NULL) {
        // enough space and is free in the list of free node
        if (get_header(cur_header)->payload_size >= requestedsz) {
            //get_header(cur_header)->if_used = USED;
            delete_list(cur_header);
            return update_header(get_header(cur_header), true, requestedsz);
        }
        cur_header = cur_header->next;
    }
    return update_header(end, false, requestedsz);
}

void myfree(void *ptr)
{
    if (!ptr) return; // edge case when myfree an invalid pointer
    Header *header = (Header*)((char*)ptr - SIZEOFHEADER);
    if (header->if_used == FREE) return;
    coalesce(header);
    Free_list *list = (Free_list*)((char*)header + SIZEOFHEADER);
    add_root(list);
    header->if_used = FREE;
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
    size_t new_size = roundup(newsz + SIZEOFHEADER, ALIGNMENT);
    if (new_size < 16) new_size = 16;
    // ???
    // edge case when its being overwritten by enlarging the size
    if (new_size < sizeof(Header) + FREELIST_SIZE) new_size = SIZEOFHEADER + FREELIST_SIZE;
    void *old_header = (char*)oldptr - SIZEOFHEADER;
    size_t original_size = ((Header*)old_header)->payload_size;
    coalesce(old_header);
    size_t new_header_size = ((Header*)old_header)->payload_size;
    if (new_size <= new_header_size) {
        update_header(old_header, true, new_size);
        return oldptr;
    } else {
        void *newptr = mymalloc(newsz);
        if (newptr == NULL) return NULL;
        memcpy(newptr, oldptr, original_size);
        myfree(oldptr);
        return newptr;
    }
    //return newptr;
}

bool validate_heap()
{

    void *cur_header = beginning;
    int total_heap = 0;
    // check that you can traverse from the beginning to the end and that the total_heap
    // is reasonable
    while (cur_header != end) {
        cur_header = (char*)cur_header + SIZEOFHEADER + ((Header*)cur_header)->payload_size;
        total_heap += ((Header*)cur_header)->payload_size;
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

