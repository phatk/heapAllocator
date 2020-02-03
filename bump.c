/* File: bump.c
 * ------------
 * A very simple "bump" allocator.
 * An allocation request is serviced by tacking on the requested
 * space to the end of the heap thus far. 
 * Free is a no-op: blocks are never coalesced or reused.
 * Realloc is implemented using malloc/memcpy/free. Operations
 * are fast, but utilization is terrible. It is also missing
 * attention to robustness in terms of handling all well-formed
 * requests.
 *
 * Shows the very simplest of approaches; there are better options!
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allocator.h"
#include "debug_break.h"

static void *segment_start;
static size_t segment_size, nused;

// Efficient bitwise round up to nearest multiple (you saw this code in lab1)
// NOTE: mult has to be power of 2 for the bitwise trick to work!
size_t roundup(size_t sz, size_t mult)
{
    return (sz + mult-1) & ~(mult-1);
}

// initialize global variables based on segment boundaries
bool myinit(void *start, size_t size)
{
    segment_start = start;
    segment_size = size;
    nused = 0;
    return true;
}

// place block at end of heap
// no search means fast but no recycle makes for terrible utilization
void *mymalloc(size_t requestedsz)
{
    size_t needed = roundup(requestedsz, ALIGNMENT);
    if (needed + nused > segment_size)
        return NULL;
    void *ptr = (char *)segment_start + nused;
    nused += needed;
    return ptr;
}

// free does nothing.  fast!... but lame :(
void myfree(void *ptr)
{
}

// realloc built on malloc/memcpy/free is easy to write but not particularly efficient
// version below is not robust either (your version should be, hint!)
void *myrealloc(void *oldptr, size_t newsz)
{
    void *newptr = mymalloc(newsz);
    memcpy(newptr, oldptr, newsz);
    myfree(oldptr);
    return newptr;
}

// required debugging routine to detect/report inconsistencies in heap data structures
bool validate_heap(void)
{
    // The bump allocator doesn't have much it can check
    if (nused > segment_size) {
        printf("Oops! Have used more heap than total available?!\n");
        breakpoint();   // call this function to stop in gdb so you can poke around
        return false;
    }
    return true;
}

// This optional function dumps the raw heap contents.
// This function is not called from anywhere, it is just here to
// demonstrate how such a function might be a useful debugging aid.
// You are not required to implement such a function in your own
// allocators, but if you do, you can then call the function
// from gdb to view the contents of the heap segment. 
// For the bump allocator, the heap contains no block headers or 
// heap housekeeping to provide structure, so all that can be displayed 
// is a dump of the raw bytes. For a more structured allocator, you 
// could implement dump_heap to instead just print the block headers, 
// which would be less overwhelming to wade through for debugging.
void dump_heap(void)
{
    printf("Heap segment starts at address %p, ends at %p. %lu bytes currently used.", 
        segment_start, (char *)segment_start + segment_size, nused);
    for (int i = 0; i < nused; i++) {
        unsigned char *cur = (unsigned char *)segment_start + i;
        if (i % 32 == 0) printf("\n%p: ", cur);
        printf("%02x ", *cur);
    }
}
