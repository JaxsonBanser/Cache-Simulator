////////////////////////////////////////////////////////////////////////////////
// Main File:        p3heap.c
// This File:        p3heap.c
// Other Files:      (name of all other files if any)
// Semester:         CS 354 Lecture 003 SP25
// Instructor:       Yiyin Shen
// 
// Author:           Jaxson Banser
// Email:            banser@wisc.edu
// CS Login:         jaxson
/////////////////////////////// OPTIONAL WORK LOG ///////////////////////////////
// Document your work sessions here or on your copy of https://bit.ly/cs354-work-log
// Keep track of commands, structures, code that you have learned.
// This will help you focus your review on this from each program that are new to you.  
/////////////////////// REQUIRED -- OTHER SOURCES OF HELP ///////////////////////
// Persons:         N/A
//
// Online sources:  N/A
// 
// AI tools:        N/A 
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Copyright Jim Skrentny & Deb Deppeler 2020-2024
// Adopted by permission Yiyin Shen SP 2025
// Posting or sharing this file is prohibited, including any changes/additions.
///////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include "p3Heap.h"

/*
 * This structure serves as the header for each allocated and free block.
 * It also serves as the footer for each free block.
 */
typedef struct blockHeader {           

    /*
     * 1) The size of each heap block must be a multiple of 8
     * 2) heap blocks have blockHeaders that contain size and status bits
     * 3) free heap block contain a footer, but we can use the blockHeader 
     *.
     * All heap blocks have a blockHeader with size and status
     * Free heap blocks have a blockHeader as its footer with size only
     *
     * Status is stored using the two least significant bits.
     *   Bit0 => least significant bit, last bit
     *   Bit0 == 0 => free block
     *   Bit0 == 1 => allocated block
     *
     *   Bit1 => second last bit 
     *   Bit1 == 0 => previous block is free
     *   Bit1 == 1 => previous block is allocated
     * 
     * Start Heap: 
     *  The blockHeader for the first block of the heap is after skip 4 bytes.
     *  This ensures alignment requirements can be met.
     * 
     * End Mark: 
     *  The end of the available memory is indicated using a size_status of 1.
     * 
     * Examples:
     * 
     * 1. Allocated block of size 24 bytes:
     *    Allocated Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 25
     *      If the previous block is allocated p-bit=1 size_status would be 27
     * 
     * 2. Free block of size 24 bytes:
     *    Free Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 24
     *      If the previous block is allocated p-bit=1 size_status would be 26
     *    Free Block Footer:
     *      size_status should be 24
     */
    int size_status;

} blockHeader;         

/* Global variable - DO NOT CHANGE NAME or TYPE. 
 * It must point to the first block in the heap and is set by init_heap()
 * i.e., the block at the lowest address.
 */
blockHeader *heap_start = NULL;     

/* Size of heap allocation padded to round to nearest page size.
 */
int alloc_size;

/*
 * Additional global variables may be added as needed below
 * TODO: add global variables needed by your function
 */




/* 
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block (payload) on success.
 * Returns NULL on failure.
 *
 * This function must:
 * - Check size - Return NULL if size < 1 
 * - Determine block size rounding up to a multiple of 8 
 *   and possibly adding padding as a result.
 *
 * - Use BEST-FIT PLACEMENT POLICY to chose a free block
 *
 * - If the BEST-FIT block that is found is exact size match
 *   - 1. Update all heap blocks as needed for any affected blocks
 *   - 2. Return the address of the allocated block payload
 *
 * - If the BEST-FIT block that is found is large enough to split 
 *   - 1. SPLIT the free block into two valid heap blocks:
 *         1. an allocated block
 *         2. a free block
 *         NOTE: both blocks must meet heap block requirements 
 *       - Update all heap block header(s) and footer(s) 
 *              as needed for any affected blocks.
 *   - 2. Return the address of the allocated block payload
 *
 *   Return if NULL unable to find and allocate block for required size
 *
 * Note: payload address that is returned is NOT the address of the
 *       block header.  It is the address of the start of the 
 *       available memory for the requesterr.
 *
 * Tips: Be careful with pointer arithmetic and scale factors.
 */
void* balloc(int size) {     
	if (size < 1) {
		return NULL;
	}

	//Rounds up the size to the nearest multiple of 8 and adds the extra space
	// required for the buffer
	int block_size = size + sizeof(blockHeader);
	while (block_size % 8 != 0) {
		block_size++;
	}

	//Originally set to the starting block. Used to track the blocks
    blockHeader *curr_loc = heap_start; 
	
    //Used to keep track of the best fit
    blockHeader *best_fit = NULL;
	
	//Runs until curr_loc's curr_size == 1 aka the end of the heap
	while (curr_loc -> size_status != 1) {
		int curr_size = curr_loc -> size_status & ~3;
		
	    //The current block is free
		if (!(curr_loc -> size_status & 1)) {
		    if (curr_size >= block_size) {
				//Checks to see if the curr_loc is a better fit
                if (!best_fit || curr_size < (best_fit->size_status & ~3)) {
                    best_fit = curr_loc;
                }

				//Curr_loc is a perfect fit
                if (curr_size == block_size) {
                    break;
                }
            }
		}
	//Updates current location
	curr_loc =(blockHeader*)((void*) curr_loc + curr_size);
	}
	


	//Theres no valid space for the block
	if (best_fit == NULL) {
		return NULL;
	//There is a valid space
	}
	//Size of the best_fit block
	int best_size = best_fit -> size_status & ~3;		
	//The size left after the new block is allocated until the next block
	// or the end
	int remaining_size = best_size - block_size;

	//Block needs to be split
	if (remaining_size >= 8) {		

		//Sets the new allocated block's size and allocated + p bits
		best_fit -> size_status = block_size | 1 | (best_fit->size_status & 2);
			
		//Creates the new free block 
		blockHeader *free_block = (blockHeader*)((void*)best_fit +
										block_size);
		free_block -> size_status = remaining_size | 2;

		//Creates the footer for the free block
		blockHeader *footer = (blockHeader*)((void*)free_block + 
									remaining_size - sizeof(blockHeader));
		footer -> size_status = remaining_size;
			
		//Finds the next block
		blockHeader *next_block = (blockHeader*)((void*)free_block + remaining_size);

		if (next_block -> size_status !=1){
				next_block -> size_status &= ~2;
		} 
	} else {
		//Sets the new allocated block's size and allocated + p bits
		best_fit -> size_status |= 1;

		//Finds the next block
		blockHeader *next_block = (blockHeader*)((void*)best_fit + best_size);

		if (next_block -> size_status != 1) {
			next_block -> size_status |= 2;	
		}
	}
	
    return (void*)((char*)best_fit + sizeof(blockHeader));
} 

/* 
 * Function for freeing up a previously allocated block.
 * Argument ptr: address of the block to be freed up.
 * Returns 0 on success.
 * Returns -1 on failure.
 * This function should:
 * - Return -1 if ptr is NULL.
 * - Return -1 if ptr is not a multiple of 8.
 * - Return -1 if ptr is outside of the heap space.
 * - Return -1 if ptr block is already freed.
 * - Update header(s) and footer as needed.
 *
 * If free results in two or more adjacent free blocks,
 * they will be immediately coalesced into one larger free block.
 * so free blocks require a footer (blockHeader works) to store the size
 *
 * TIP: work on getting immediate coalescing to work after your code 
 *      can pass the tests in partA and partB of tests/ directory.
 *      Submit code that passes partA and partB to Canvas before continuing.
 */                    
int bfree(void *ptr) {    
	if (ptr == NULL) {
		return -1;
	}

	//Sets curr_block to the location of ptr casted to type blockHeader
	// and curr_size to the size of the block	
	blockHeader *curr_block = (blockHeader*)((void*)ptr - sizeof(blockHeader));
	int curr_size = curr_block -> size_status & ~3;

	//Checks if the block is free
	if (!(curr_block -> size_status & 1)) {
		return -1;
	}
	
	//Checks if the block is a valid multiple of 8
	if (curr_size % 8 != 0) {
		return -1;
	}

	//Checks if the block is outside the heap space
	if ((void*)curr_block < (void*)heap_start || (void*)curr_block >= ((void*)
		heap_start + alloc_size)) {
		return -1;
	}
	
	//Sets the curr block to be free
	curr_block -> size_status &= ~1;

	//Gets the next block
	blockHeader *next_block = (blockHeader*)((void*)curr_block + curr_size);
	int next_block_size = next_block -> size_status & ~3;

	//Checks to see that the next block is not allocated and is not the end
	// of the heap section	
	if (next_block -> size_status != 1 && !(next_block -> size_status & 1)) {
		//Adds the next block to the curr block's size
		curr_size += next_block_size;
		curr_block -> size_status = curr_size | (curr_block -> size_status & 2);

	//Sets the next block's p-bit to 0
	} else if (next_block -> size_status & 1) {
		next_block -> size_status &= ~2;
	}

	//Creates the footer and moves it to the end of the newly freed block
    blockHeader *footer = (blockHeader*)((char*)curr_block + curr_size - 
							sizeof(blockHeader));
    footer->size_status = curr_size;

	if (!(curr_block -> size_status & 2)) {
		//Gets the size of the previous block
		blockHeader *prev_footer = (blockHeader*)((void*)curr_block - 
										sizeof(blockHeader));
		int prev_size = prev_footer -> size_status;

		//Gets the previous block and changes it's header
		blockHeader *prev_block = (blockHeader*)((void*)curr_block -
								prev_size);
		prev_block -> size_status = (curr_size + prev_size) | (prev_block
									-> size_status & 2);

		curr_size += prev_size;
	}

	footer -> size_status = curr_size;
	
    return 0;
} 


/* 
 * Initializes the memory allocator.
 * Called ONLY once by a program.
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */                    
int init_heap(int sizeOfRegion) {    

    static int allocated_once = 0; //prevent multiple myInit calls

    int   pagesize; // page size
    int   padsize;  // size of padding when heap size not a multiple of page size
    void* mmap_ptr; // pointer to memory mapped area
    int   fd;

    blockHeader* end_mark;

    if (0 != allocated_once) {
        fprintf(stderr, 
                "Error:mem.c: InitHeap has allocated space during a previous call\n");
        return -1;
    }

    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }

    // Get the pagesize from O.S. 
    pagesize = getpagesize();

    // Calculate padsize as the padding required to round up sizeOfRegion 
    // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;

    alloc_size = sizeOfRegion + padsize;

    // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    mmap_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == mmap_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }

    allocated_once = 1;

    // for double word alignment and end mark
    alloc_size -= 8;

    // Initially there is only one big free block in the heap.
    // Skip first 4 bytes for double word alignment requirement.
    heap_start = (blockHeader*) mmap_ptr + 1;

    // Set the end mark
    end_mark = (blockHeader*)((void*)heap_start + alloc_size);
    end_mark->size_status = 1;

    // Set size in header
    heap_start->size_status = alloc_size;

    // Set p-bit as allocated in header
    // note a-bit left at 0 for free
    heap_start->size_status += 2;

    // Set the footer
    blockHeader *footer = (blockHeader*) ((void*)heap_start + alloc_size - 4);
    footer->size_status = alloc_size;

    return 0;
} 

/* STUDENTS MAY EDIT THIS FUNCTION, but do not change function header.
 * TIP: review this implementation to see one way to traverse through
 *      the blocks in the heap.
 *
 * Can be used for DEBUGGING to help you visualize your heap structure.
 * It traverses heap blocks and prints info about each block found.
 * 
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block 
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts) 
 * t_End    : address of the last byte in the block 
 * t_Size   : size of the block as stored in the block header
 */                     
void disp_heap() {     

    int    counter;
    char   status[6];
    char   p_status[6];
    char * t_begin = NULL;
    char * t_end   = NULL;
    int    t_size;

    blockHeader *current = heap_start;
    counter = 1;

    int used_size =  0;
    int free_size =  0;
    int is_used   = -1;

    fprintf(stdout, 
            "********************************** HEAP: Block List ****************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout, 
            "--------------------------------------------------------------------------------\n");

    while (current->size_status != 1) {
        t_begin = (char*)current;
        t_size = current->size_status;

        if (t_size & 1) {
            // LSB = 1 => used block
            strcpy(status, "alloc");
            is_used = 1;
            t_size = t_size - 1;
        } else {
            strcpy(status, "FREE ");
            is_used = 0;
        }

        if (t_size & 2) {
            strcpy(p_status, "alloc");
            t_size = t_size - 2;
        } else {
            strcpy(p_status, "FREE ");
        }

        if (is_used) 
            used_size += t_size;
        else 
            free_size += t_size;

        t_end = t_begin + t_size - 1;

        fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%4i\n", counter, status, 
                p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);

        current = (blockHeader*)((char*)current + t_size);
        counter = counter + 1;
    }

    fprintf(stdout, 
            "--------------------------------------------------------------------------------\n");
    fprintf(stdout, 
            "********************************************************************************\n");
    fprintf(stdout, "Total used size = %4d\n", used_size);
    fprintf(stdout, "Total free size = %4d\n", free_size);
    fprintf(stdout, "Total size      = %4d\n", used_size + free_size);
    fprintf(stdout, 
            "********************************************************************************\n");
    fflush(stdout);

    return;  
} 


// p3Heap.c (SP25)                     
                                       
