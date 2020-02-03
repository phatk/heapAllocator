/* File: segment.c
 * ---------------
 * Handles low-level storage underneath the heap allocator. It reserves
 * the large memory segment using the OS-level mmap facility.
 *
 * Written by jzelenski, updated Spring 2018
 */

/* File: segment.c
 * ----------------
 * Interfaces with low-level OS allocator to set up the large
 * memory segment to be used by the heap allocator. Information
 * on how to use these routines as a client is documented
 * in the segment.h header file.
 *
 *  Written by jzelenski, updated Spring 2018
 */

#include "segment.h"
#include <assert.h>
#include <sys/mman.h>

// Place segment at fixed address, as default addresses are quite high and easily
// mistaken for stack addresses
#define HEAP_START_HINT (void *)0x107000000L

static void * segment_start = NULL;
static size_t segment_size = 0;

void *heap_segment_start(void)
{
    return segment_start;
}

size_t heap_segment_size(void)
{
    return segment_size;
}

void *init_heap_segment(size_t total_size)
{
    // Discard any previous segment via unmap
    if (segment_start != NULL) {
        if (munmap(segment_start, total_size) == -1) return NULL;
        segment_start = NULL;
        segment_size = 0;
    }
    // Re-initialize by reserving entire segment with mmap
    segment_start = mmap(HEAP_START_HINT, total_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    assert(segment_start != MAP_FAILED);
    segment_size = total_size;
    return segment_start;
}
