#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "myMalloc.h"
#include "printing.h"
/* Due to the way assert() prints error messges we use out own assert function
 * for deteminism when testing assertions
 */
#ifdef TEST_ASSERT
  inline static void assert(int e) {
    if (!e) {
      const char * msg = "Assertion Failed!\n";
      write(2, msg, strlen(msg));
      exit(1);
    }
  }
#else
  #include <assert.h>
#endif
/*
 * Mutex to ensure thread safety for the freelist
 */
static pthread_mutex_t mutex;
/*
 * Array of sentinel nodes for the freelists
 */
header freelistSentinels[N_LISTS];
/*
 * Pointer to the second fencepost in the most recently allocated chunk from
 * the OS. Used for coalescing chunks
 */
header * lastFencePost;
/*
 * Pointer to maintian the base of the heap to allow printing based on the
 * distance from the base of the heap
 */ 
void * base;
/*
 * List of chunks allocated by  the OS for printing boundary tags
 */
header * osChunkList [MAX_OS_CHUNKS];
size_t numOsChunks = 0;
/*
 * direct the compiler to run the init function before running main
 * this allows initialization of required globals
 */
static void init (void) __attribute__ ((constructor));
// Helper functions for manipulating pointers to headers
static inline header * get_header_from_offset(void * ptr, ptrdiff_t off);
static inline header * get_left_header(header * h);
static inline header * ptr_to_header(void * p);
bool freeblock_check(header *hdr);	
void insert_freelist(header *hdr);	
size_t calc_allocate_size(size_t raw_size);
int get_list_index(size_t size);
// Helper functions for allocating more memory from the OS
static inline void initialize_fencepost(header * fp, size_t left_size);
static inline void insert_os_chunk(header * hdr);
static inline void insert_fenceposts(void * raw_mem, size_t size);
static header * allocate_chunk(size_t size);
// Helper functions for freeing a block
static inline void deallocate_object(void * p);
// Helper functions for allocating a block
static inline header * allocate_object(size_t raw_size);
// Helper functions for verifying that the data structures are structurally 
// valid
static inline header * detect_cycles();
static inline header * verify_pointers();
static inline bool verify_freelist();
static inline header * verify_chunk(header * chunk);
static inline bool verify_tags();
static void init();
static bool isMallocInitialized;
/**
 * @brief Helper function to retrieve a header pointer from a pointer and an 
 *        offset
 *
 * @param ptr base pointer
 * @param off number of bytes from base pointer where header is located
 *
 * @return a pointer to a header offset bytes from pointer
 */
static inline header * get_header_from_offset(void * ptr, ptrdiff_t off) {
	return (header *)((char *) ptr + off);
}
/**
 * @brief Helper function to get the header to the right of a given header
 *
 * @param h original header
 *
 * @return header to the right of h
 */
header * get_right_header(header * h) {
	return get_header_from_offset(h, get_size(h));
}
/**
 * @brief Helper function to get the header to the left of a given header
 *
 * @param h original header
 *
 * @return header to the right of h
 */
inline static header * get_left_header(header * h) {
  return get_header_from_offset(h, -h->left_size);
}
/**
 * @brief Fenceposts are marked as always allocated and may need to have
 * a left object size to ensure coalescing happens properly
 *
 * @param fp a pointer to the header being used as a fencepost
 * @param left_size the size of the object to the left of the fencepost
 */
inline static void initialize_fencepost(header * fp, size_t left_size) {
	set_state(fp,FENCEPOST);
	set_size(fp, ALLOC_HEADER_SIZE);
	fp->left_size = left_size;
}
/**
 * @brief Helper function to maintain list of chunks from the OS for debugging
 *
 * @param hdr the first fencepost in the chunk allocated by the OS
 */
inline static void insert_os_chunk(header * hdr) {
  if (numOsChunks < MAX_OS_CHUNKS) {
    osChunkList[numOsChunks++] = hdr;
  }
}
/**
 * @brief given a chunk of memory insert fenceposts at the left and 
 * right boundaries of the block to prevent coalescing outside of the
 * block
 *
 * @param raw_mem a void pointer to the memory chunk to initialize
 * @param size the size of the allocated chunk
 */
inline static void insert_fenceposts(void * raw_mem, size_t size) {
  // Convert to char * before performing operations
  char * mem = (char *) raw_mem;
  // Insert a fencepost at the left edge of the block
  header * leftFencePost = (header *) mem;
  initialize_fencepost(leftFencePost, ALLOC_HEADER_SIZE);
  // Insert a fencepost at the right edge of the block
  header * rightFencePost = get_header_from_offset(mem, size - ALLOC_HEADER_SIZE);
  initialize_fencepost(rightFencePost, size - 2 * ALLOC_HEADER_SIZE);
}
/**
 * @brief Allocate another chunk from the OS and prepare to insert it
 * into the free list
 *
 * @param size The size to allocate from the OS
 *
 * @return A pointer to the allocable block in the chunk (just after the 
 * first fencpost)
 */
static header * allocate_chunk(size_t size) {
  void * mem = sbrk(size);
  
  insert_fenceposts(mem, size);
  header * hdr = (header *) ((char *)mem + ALLOC_HEADER_SIZE);
  set_state(hdr, UNALLOCATED);
  set_size(hdr, size - 2 * ALLOC_HEADER_SIZE);
  hdr->left_size = ALLOC_HEADER_SIZE;
  return hdr;
}
size_t calc_allocate_size(size_t raw_size) {
	size_t rounded_size = ((raw_size + ALLOC_HEADER_SIZE + 15 )/16)*16 ;
	if (rounded_size < sizeof(header)) {
		return sizeof(header);
	}
	return rounded_size;
}
int get_list_index(size_t size) {
	if (size > (N_LISTS - 1) * 16) {
		return N_LISTS - 1;
	}
		return (size / 16) - 1;
}
bool freeblock_check(header* hdr) {
	int listidx = get_list_index(get_size(hdr) - ALLOC_HEADER_SIZE);
	if (listidx != N_LISTS - 1) {
		return false;
	}
	return true;
}
void insert_freelist(header * hdr){
	int listidx = get_list_index(get_size(hdr) - ALLOC_HEADER_SIZE);
	header * freelist = &freelistSentinels[listidx];
	if (freelist->next == freelist) {
		freelist->prev = hdr;
	}
	freelist->next->prev = hdr;
	hdr->prev = freelist;
	hdr->next = freelist->next;
	freelist->next = hdr;
	return;
}
static inline header * allocate_object(size_t raw_size) {
    if (raw_size == 0){
        return NULL;
    }
    size_t allocated_size = calc_allocate_size(raw_size);
    int listidx = get_list_index(allocated_size - ALLOC_HEADER_SIZE);
    header * freelist = &freelistSentinels[listidx];
    while (freelist->next == freelist && listidx < N_LISTS - 1) {
        listidx++;
        freelist = &freelistSentinels[listidx];
    }
    header * current;
    for (current = freelist->next; current != freelist || listidx != N_LISTS - 1; current = current->next) {
        size_t current_size = get_size(current);
        if (current_size == allocated_size || (current_size - allocated_size) < sizeof(header)) {
            set_state(current, ALLOCATED);
            current->next->prev = current->prev;
            current->prev->next = current->next;
            return (header *)current->data;    
        } 
        else {
		/*
            bool fl = freeblock_check(current);
            set_size(current, get_size(current) - allocated_size);
            char * newpointer = (char *)current + get_size(current);
            header * newHeader = (header *)newpointer;
            set_size(newHeader, allocated_size);
	    newHeader->left_size = get_size(current);
            set_state(newHeader, ALLOCATED);
            char * rightpointer = (char *)newHeader + get_size(newHeader);
            header * rightHeader = (header *)rightpointer;
            rightHeader->left_size = get_size(newHeader);
            if (!(fl == true && get_list_index(get_size(current) - ALLOC_HEADER_SIZE) == N_LISTS - 1)) {
                current->next->prev = current->prev;
                current->prev->next = current->next;
                insert_freelist(current);
            }
            return (header *)newHeader->data;
	    
	    *
        
	bool fl = freeblock_check(current);
    // Create newHeader at the current position
    header * newHeader = current;
    // Assign values to newHeader
    set_size(newHeader, allocated_size);
    // Update newHeader's state to ALLOCATED
    set_state(newHeader, ALLOCATED);
    // Calculate the remaining size after allocation
    size_t remaining_size = current_size - allocated_size;
    // Create remaining block header at the new position
    char * remaining_pointer = (char *)newHeader + allocated_size;
    header * remainingHeader = (header *)remaining_pointer;
    // Assign values to remainingHeader
    set_size(remainingHeader, remaining_size);
    remainingHeader->left_size = allocated_size;
    // Update the rightHeader's left_size
    char * rightpointer = (char *)remainingHeader + remaining_size;
    header * rightHeader = (header *)rightpointer;
    rightHeader->left_size = remaining_size;
    // Remove the current block from the freelist if necessary
    if (fl) {
        current->next->prev = current->prev;
        current->prev->next = current->next;
    }
    // Insert the remaining block into the free list
    insert_freelist(remainingHeader);
    // Return newHeader->data
    return (header *)newHeader->data;
*/    
		bool fl = freeblock_check(current);
    // Create newHeader at the current position
    header * newHeader = current;
    // Assign values to newHeader
    set_size(newHeader, allocated_size);
    // Update newHeader's state to ALLOCATED
    set_state(newHeader, ALLOCATED);
    // Calculate the remaining size after allocation
    size_t remaining_size = current_size - allocated_size;
    // Create remaining block header at the new position
    char * remaining_pointer = (char *)newHeader + allocated_size;
    header * remainingHeader = (header *)remaining_pointer;
    // Assign values to remainingHeader
    set_size(remainingHeader, remaining_size);
    remainingHeader->left_size = allocated_size;
    // Update the rightHeader's left_size
    char * rightpointer = (char *)remainingHeader + remaining_size;
    header * rightHeader = (header *)rightpointer;
    rightHeader->left_size = remaining_size;
    // Remove the current block from the freelist if necessary
    if (fl) {
        current->next->prev = current->prev;
        current->prev->next = current->next;
    }
    // Insert the remaining block into the correct free list
    int remaining_list_idx = get_list_index(remaining_size - ALLOC_HEADER_SIZE);
    if (remaining_list_idx != N_LISTS - 1 || remaining_size != 0) {
        insert_freelist(remainingHeader);
    }
    // Return newHeader->data
    return (header *)newHeader->data;
	/*
bool fl = freeblock_check(current);
    // Create newHeader at the current position
    header * newHeader = current;
    // Assign values to newHeader
    set_size(newHeader, allocated_size);
    // Update newHeader's state to ALLOCATED
    set_state(newHeader, ALLOCATED);
    // Calculate the remaining size after allocation
    size_t remaining_size = current_size - allocated_size;
    // Create remaining block header at the new position
    char * remaining_pointer = (char *)newHeader + allocated_size;
    header * remainingHeader = (header *)remaining_pointer;
    // Assign values to remainingHeader
    set_size(remainingHeader, remaining_size);
    remainingHeader->left_size = allocated_size;
    // Update the rightHeader's left_size
    char * rightpointer = (char *)remainingHeader + remaining_size;
    header * rightHeader = (header *)rightpointer;
    rightHeader->left_size = remaining_size;
    // Debug: Print the sizes and list index before removing from freelist
    printf("Current block size: %zu\n", current_size);
    printf("Allocated size: %zu\n", allocated_size);
    printf("Remaining size: %zu\n", remaining_size);
    // Remove the current block from the freelist if necessary
    if (fl) {
        current->next->prev = current->prev;
        current->prev->next = current->next;
    }
    // Determine the correct free list index for the remaining block
    int remaining_list_idx = get_list_index(remaining_size - ALLOC_HEADER_SIZE);
    printf("Remaining list index: %d\n", remaining_list_idx);
    // Debug: Check if the remaining block is being inserted correctly
    if (remaining_list_idx != N_LISTS - 1 || remaining_size != 0) {
        printf("Inserting remaining block into freelist at index %d\n", remaining_list_idx);
        insert_freelist(remainingHeader);
    } else {
        printf("Skipping insertion into freelist due to size\n");
    }
    // Return newHeader->data
    return (header *)newHeader->data;
    */
	}
    }
    header * newHeader = allocate_chunk(ARENA_SIZE);
    header * left_fencepost = get_header_from_offset(newHeader, -ALLOC_HEADER_SIZE);
    header * right_fencepost = get_header_from_offset(newHeader, get_size(newHeader));
    header * last_fencepost = get_header_from_offset(left_fencepost, -ALLOC_HEADER_SIZE);
    if (last_fencepost == lastFencePost) {
        header * last_block = get_left_header(last_fencepost);
        if (get_state(last_block) == UNALLOCATED) {
            bool fl = freeblock_check(last_block);
            set_size(last_block, get_size(last_block) + get_size(newHeader) + 2 * ALLOC_HEADER_SIZE);
            set_state(last_block, UNALLOCATED);
            right_fencepost->left_size = get_size(last_block);
            if (!(fl == true && get_list_index(get_size(last_block) - ALLOC_HEADER_SIZE) == N_LISTS - 1)) {
                last_block->next->prev = last_block->prev;
                last_block->prev->next = last_block->next;
                insert_freelist(last_block);
            }
            lastFencePost = right_fencepost;
            return allocate_object(raw_size);
        }
        else {
            set_state(newHeader, UNALLOCATED);
            insert_freelist(newHeader);
            lastFencePost = right_fencepost;
            return allocate_object(raw_size);
        }
    }
    else {
        set_state(newHeader, UNALLOCATED);
        insert_freelist(newHeader);
        lastFencePost = right_fencepost;
        return allocate_object(raw_size);
    }
}
/**
 * @brief Helper allocate an object given a raw request size from the user
 *
 * @param raw_size number of bytes the user needs
 *
 * @return A block satisfying the user's request
static inline header * allocate_object(size_t raw_size) {
	if (raw_size == 0){
		return NULL;
	}
	//Calculate the size to be allocated, including size of header
	size_t allocated_size = calc_allocate_size(raw_size);
	//Find the appropriate free list to look for a block to allocate
	int listidx = get_list_index(allocated_size - ALLOC_HEADER_SIZE) ;
	header * freelist = &freelistSentinels[listidx];
	
	//Use first non-empty list
	while (freelist->next == freelist && listidx < N_LISTS -1){
		listidx++;
		freelist = &freelistSentinels[listidx];
	}
	
	header *current;
	for (current = freelist->next; current != freelist || listidx != N_LISTS -1 ; current = current->next){
		size_t current_size = get_size (current);
		
		//Case1: No need to break the chunk into two parts
		if (current_size == allocated_size || (current_size - allocated_size) < sizeof(header) ){
			//Update current block's state to ALLOCATED
			set_state(current, ALLOCATED);
			//Remove current block from freelist
			current->next->prev = current->prev;
			current->prev->next = current->next;
			//Return current->data
			return (header*)current->data;	
		} //Case2: Need to break the chunk into two
		else{
			bool fl = freeblock_check(current);
			//Update current block's size
			set_size(current, get_size(current) - allocated_size);
			
			//Get to the point of split & make newHeader
			char * newpointer = (char *)current + get_size(current);
			header * newHeader = (header *)newpointer;
			//Assign values to newheader
			set_size(newHeader, allocated_size);
			newHeader->left_size = get_size(current);
			//Update newHeader's state to ALLOCATED
			set_state(newHeader, ALLOCATED);
			
			//Get the rightHeader
			char * rightpointer = (char *)newHeader + get_size(newHeader);
			header * rightHeader = (header*)rightpointer;
			
			//Update rightHeader's left_size
			rightHeader->left_size = get_size(newHeader);
			
			//Call this if a block is in freelist and you did some modfications on it
			if ( !(fl == true && get_list_index(get_size(current) - ALLOC_HEADER_SIZE) == N_LISTS -1) ) {
				current->next->prev = current->prev;
				current->prev->next = current->next;
				insert_freelist(current);
			}
			
			//Return newheader->data
			return (header*)newHeader->data;
		}
	}
	//Case3: Memory requested cannot be fulfilled, allocate new memory chunk
	//Variable declaration & their positions 
	//last_block | last_fencepost || left_fencepost | newHeader | right_fencepost
	header * newHeader = allocate_chunk(ARENA_SIZE);
	header * left_fencepost = get_header_from_offset(newHeader, -ALLOC_HEADER_SIZE);
	header * right_fencepost = get_header_from_offset(newHeader, get_size(newHeader));
	**last_fencepost is the left header of left_fencepost, not the actual last_fencepost*** 
	**However lastFencePost, the global variable is***
	header * last_fencepost = get_header_from_offset(left_fencepost, -ALLOC_HEADER_SIZE);
		
	//CaseA: If two chunks are adjacent, coalesce them
	if (last_fencepost == lastFencePost) {
		//CaseAA: If last_block in previous chunk is UNALLOCATED, coalesce them
		header * last_block = get_left_header(last_fencepost);
		if (get_state(last_block) == UNALLOCATED) {
			bool fl = freeblock_check(last_block);
			//Update last_block's size = last_block + newHeader + 2*fencepost
			set_size(last_block, get_size(last_block) + get_size(newHeader) + 2 * ALLOC_HEADER_SIZE);
			//Update the state of last_block
			set_state(last_block, UNALLOCATED);
			
			//Update right_fencepost's left_size
			right_fencepost->left_size = get_size(last_block);
			//Call this if a block is in freelist and you did some modfications on it
			if (!(fl == true && get_list_index(get_size(last_block) - ALLOC_HEADER_SIZE) == N_LISTS - 1)) {
				last_block->next->prev = last_block->prev;
				last_block->prev->next = last_block->next;
				insert_freelist(last_block);
			}
			//Update lastFencePost
			lastFencePost = right_fencepost;
			//Call allocate again
			return allocate_object(raw_size);
			
		
		}	//Case AB: If last_block is ALLOCATED
		else {
			//Update the size of last_fencepost = newHeader + 2*fencepost
			set_size(last_fencepost, get_size(newHeader) + 2 * ALLOC_HEADER_SIZE);
			//Update right_fencepost's left_size
			right_fencepost->left_size = get_size(last_fencepost);
			//Update lasr_fencepost's state
			set_state(last_fencepost, UNALLOCATED);
			//Call this if a block is not in freelist and you want to insert it
			insert_freelist(last_fencepost);
			//Update lastFencePost 
			lastFencePost = right_fencepost;
			//Call allocate again
			return allocate_object(raw_size);
		}
	}	//CaseB: If two chunks are not adjacent
	else {
		//Call this if a block is not in freelist and you want to insert it
		insert_freelist(newHeader);
		//Update lastFencePost & insert last_block to os chunk
		lastFencePost = right_fencepost;
		insert_os_chunk(left_fencepost);
		//Call allocate again
		return allocate_object(raw_size);
	}
}
 * @brief Helper to get the header from a pointer allocated with malloc
 *
 * @param p pointer to the data region of the block
 *
 * @return A pointer to the header of the block
 */
static inline header * ptr_to_header(void * p) {
  return (header *)((char *) p - ALLOC_HEADER_SIZE); //sizeof(header));
}
static inline void deallocate_object(void * p) {
    if (p == NULL) {
        return;
    }
    header * currHeader = ptr_to_header(p);
    if (get_state(currHeader) == UNALLOCATED) {
        printf("Double Free Detected\n");
        printf("Assertion Failed!\n");
        assert(true);
        exit(1);
    }
    set_state(currHeader, UNALLOCATED);
    header * leftHeader = get_left_header(currHeader);
    header * rightHeader = get_right_header(currHeader);
    if (get_state(leftHeader) == UNALLOCATED && get_state(rightHeader) == UNALLOCATED) {
        bool flLeft = freeblock_check(leftHeader);
        bool flRight = freeblock_check(rightHeader);
        set_size(leftHeader, get_size(leftHeader) + get_size(currHeader) + get_size(rightHeader));
        rightHeader->next->prev = rightHeader->prev;
        rightHeader->prev->next = rightHeader->next;
        header * rightRightHeader = get_right_header(rightHeader);
        rightRightHeader->left_size = get_size(leftHeader);
        if (!(flLeft && get_list_index(get_size(leftHeader) - ALLOC_HEADER_SIZE) == N_LISTS - 1)) {
            leftHeader->next->prev = leftHeader->prev;
            leftHeader->prev->next = leftHeader->next;
            insert_freelist(leftHeader);
        }
    }
    else if (get_state(leftHeader) == UNALLOCATED) {
        bool fl = freeblock_check(leftHeader);
        set_size(leftHeader, get_size(leftHeader) + get_size(currHeader));
        rightHeader->left_size = get_size(leftHeader);
        if (!(fl && get_list_index(get_size(leftHeader) - ALLOC_HEADER_SIZE) == N_LISTS - 1)) {
            leftHeader->next->prev = leftHeader->prev;
            leftHeader->prev->next = leftHeader->next;
            insert_freelist(leftHeader);
        }
    }
    else if (get_state(rightHeader) == UNALLOCATED) {
        set_state(currHeader, UNALLOCATED);
        header * rightRightHeader = get_right_header(rightHeader);
        rightRightHeader->left_size = get_size(currHeader) + get_size(rightHeader);
        set_size(currHeader, get_size(currHeader) + get_size(rightHeader));
        rightHeader->next->prev = rightHeader->prev;
        rightHeader->prev->next = rightHeader->next;
        insert_freelist(currHeader);
    }
    else {
        set_state(currHeader, UNALLOCATED);
        insert_freelist(currHeader);
    }
}
/**
 * @brief Helper to manage deallocation of a pointer returned by the user
 *
 * @param p The pointer returned to the user by a call to malloc
 
static inline void deallocate_object(void * p) {
	
	//If pointer is NULL, no-op 
	if (p == NULL) {
		return;
	}
	//Make flags, 0 = allocated or fencepost, 1 = unallocated
	int leftflag = 0;
	int rightflag = 0;
	//Make current header from void *p
	header * currHeader = get_header_from_offset((char*)p, -ALLOC_HEADER_SIZE);
	//Check for double_free
	if (get_state(currHeader) == UNALLOCATED) {
		printf("Double Free Detected\n");
		printf("Assertion Failed!\n");
		assert(true);
		exit(1);
	}
	set_state(currHeader, UNALLOCATED);
	//Make neighbor headers
	header * leftHeader = get_left_header(currHeader);
	header * rightHeader = get_right_header(currHeader);
	//Check flag
	if (get_state(leftHeader) == UNALLOCATED) {
		leftflag = 1;
	}
	if (get_state(rightHeader) == UNALLOCATED) {
		rightflag = 1;
	}
	//Case1: Coalesce with both sides
	if (leftflag && rightflag) {
		bool fl = freeblock_check(leftHeader);
		set_state(leftHeader, UNALLOCATED);
		//Update left header's size
		set_size(leftHeader, get_size(leftHeader) + get_size(currHeader) + get_size(rightHeader));
		//Remove rightheader from the freelist
		rightHeader->next->prev = rightHeader->prev;
		rightHeader->prev->next = rightHeader->next;
		//Update rightrightheader's left_size
		header * rightrightheader = get_right_header(rightHeader);
		rightrightheader->left_size = get_size(leftHeader);
		//Call this if a block is in freelist and you did some modfications on it
		if (!(fl == true && get_list_index(get_size(leftHeader)-ALLOC_HEADER_SIZE) == N_LISTS - 1)) {
			leftHeader->next->prev = leftHeader->prev;
			leftHeader->prev->next = leftHeader->next;
			insert_freelist(leftHeader);
		}
	
	}	//Case2: Coalesce with left
	else if (leftflag) {
		bool fl = freeblock_check(leftHeader);
		set_state(leftHeader, UNALLOCATED);
		//Update left header's size
		set_size(leftHeader, get_size(leftHeader) + get_size(currHeader));
		//Update right header's left_size
		rightHeader->left_size = get_size(leftHeader);
		//Call this if a block is in freelist and you did some modfications on it
		if (!(fl == true && get_list_index(get_size(leftHeader) - ALLOC_HEADER_SIZE) == N_LISTS - 1)) {
			leftHeader->next->prev = leftHeader->prev;
			leftHeader->prev->next = leftHeader->next;
			insert_freelist(leftHeader);
		}
	}	//Case3: Coalesce with the right
	else if (rightflag) {
		//Change the currHeader's state
		set_state(currHeader, UNALLOCATED);
		//Update rightrightheader's left_size
		header * rightrightHeader = get_right_header(rightHeader);
		rightrightHeader->left_size = get_size(currHeader) + get_size(rightHeader);
		//Update currHeader's size
		set_size(currHeader, get_size(currHeader) + get_size(rightHeader));
		//Remove rightheader from the freelist
		rightHeader->next->prev = rightHeader->prev;
		rightHeader->prev->next = rightHeader->next;
		//Call this if a block is not in freelist and you want to insert it
		insert_freelist(currHeader);
	}	//Case4: No colescing
	else {
		//Change currHeader's state
		set_state(currHeader, UNALLOCATED);
		//Call this if a block is not in freelist and you want to insert it
		insert_freelist(currHeader);
	}
	return;
	(void) p;
	assert(false);
	exit(1);
}
 * @brief Helper to detect cycles in the free list
 * https://en.wikipedia.org/wiki/Cycle_detection#Floyd's_Tortoise_and_Hare
 *
 * @return One of the nodes in the cycle or NULL if no cycle is present
 */
static inline header * detect_cycles() {
  for (int i = 0; i < N_LISTS; i++) {
    header * freelist = &freelistSentinels[i];
    for (header * slow = freelist->next, * fast = freelist->next->next; 
         fast != freelist; 
         slow = slow->next, fast = fast->next->next) {
      if (slow == fast) {
        return slow;
      }
    }
  }
  return NULL;
}
/**
 * @brief Helper to verify that there are no unlinked previous or next pointers
 *        in the free list
 *
 * @return A node whose previous and next pointers are incorrect or NULL if no
 *         such node exists
 */
static inline header * verify_pointers() {
  for (int i = 0; i < N_LISTS; i++) {
    header * freelist = &freelistSentinels[i];
    for (header * cur = freelist->next; cur != freelist; cur = cur->next) {
      if (cur->next->prev != cur || cur->prev->next != cur) {
        return cur;
      }
    }
  }
  return NULL;
}
static bool verify_freelist() {
    for (int i = 0; i < N_LISTS; i++) {
        header * freelist = &freelistSentinels[i];
        for (header * cur = freelist->next; cur != freelist; cur = cur->next) {
            // Ensure that all blocks in the freelist are unallocated
            if (get_state(cur) != UNALLOCATED) {
                printf("Freelist Verification Failed: Allocated block in freelist\n");
                return false;
            }
        }
    }
    return true;
}
static bool verify_tags() {
    for (size_t i = 0; i < numOsChunks; i++) {
        header * chunk = osChunkList[i];
        for (; get_state(chunk) != FENCEPOST; chunk = get_right_header(chunk)) {
            // Ensure that left_size of the right block matches the size of the current block
            if (chunk->left_size != get_left_header(chunk)->left_size) {
                printf("Boundary Tag Verification Failed: Mismatched left_size\n");
                return false;
            }
        }
    }
    return true;
}
/**
 * @brief Verify the structure of the free list is correct by checkin for 
 *        cycles and misdirected pointers
 *
 * @return true if the list is valid
static inline bool verify_freelist() {
  header * cycle = detect_cycles();
  if (cycle != NULL) {
    fprintf(stderr, "Cycle Detected\n");
    print_sublist(print_object, cycle->next, cycle);
    return false;
  }
  header * invalid = verify_pointers();
  if (invalid != NULL) {
    fprintf(stderr, "Invalid pointers\n");
    print_object(invalid);
    return false;
  }
  return true;
}
 * @brief Helper to verify that the sizes in a chunk from the OS are correct
 *        and that allocated node's canary values are correct
 *
 * @param chunk AREA_SIZE chunk allocated from the OS
 *
 * @return a pointer to an invalid header or NULL if all header's are valid
 */
static inline header * verify_chunk(header * chunk) {
	if (get_state(chunk) != FENCEPOST) {
		fprintf(stderr, "Invalid fencepost\n");
		print_object(chunk);
		return chunk;
	}
	
	for (; get_state(chunk) != FENCEPOST; chunk = get_right_header(chunk)) {
		if (get_size(chunk)  != get_right_header(chunk)->left_size) {
			fprintf(stderr, "Invalid sizes\n");
			print_object(chunk);
			return chunk;
		}
	}
	
	return NULL;
}
/**
 * @brief For each chunk allocated by the OS verify that the boundary tags
 *        are consistent
 *
 * @return true if the boundary tags are valid
static inline bool verify_tags() {
  for (size_t i = 0; i < numOsChunks; i++) {
    header * invalid = verify_chunk(osChunkList[i]);
    if (invalid != NULL) {
      return invalid;
    }
  }
  return NULL;
}
 * @brief Initialize mutex lock and prepare an initial chunk of memory for allocation
 */
static void init() {
  // Initialize mutex for thread safety
  pthread_mutex_init(&mutex, NULL);
#ifdef DEBUG
  // Manually set printf buffer so it won't call malloc when debugging the allocator
  setvbuf(stdout, NULL, _IONBF, 0);
#endif // DEBUG
  // Allocate the first chunk from the OS
  header * block = allocate_chunk(ARENA_SIZE);
  header * prevFencePost = get_header_from_offset(block, -ALLOC_HEADER_SIZE);
  insert_os_chunk(prevFencePost);
  lastFencePost = get_header_from_offset(block, get_size(block));
  // Set the base pointer to the beginning of the first fencepost in the first
  // chunk from the OS
  base = ((char *) block) - ALLOC_HEADER_SIZE; //sizeof(header);
  // Initialize freelist sentinels
  for (int i = 0; i < N_LISTS; i++) {
    header * freelist = &freelistSentinels[i];
    freelist->next = freelist;
    freelist->prev = freelist;
  }
  // Insert first chunk into the free list
  header * freelist = &freelistSentinels[N_LISTS - 1];
  freelist->next = block;
  freelist->prev = block;
  block->next = freelist;
  block->prev = freelist;
}
/* 
 * External interface
 */
void * my_malloc(size_t size) {
  pthread_mutex_lock(&mutex);
  header * hdr = allocate_object(size); 
  pthread_mutex_unlock(&mutex);
  return hdr;
}
void * my_calloc(size_t nmemb, size_t size) {
  return memset(my_malloc(size * nmemb), 0, size * nmemb);
}
void * my_realloc(void * ptr, size_t size) {
  void * mem = my_malloc(size);
  memcpy(mem, ptr, size);
  my_free(ptr);
  return mem; 
}
void my_free(void * p) {
  pthread_mutex_lock(&mutex);
  deallocate_object(p);
  pthread_mutex_unlock(&mutex);
}
bool verify() {
  return verify_freelist() && verify_tags();
}
/*
#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "myMalloc.h"
#include "printing.h"
* Due to the way assert() prints error messges we use out own assert function
 * for deteminism when testing assertions
 
#ifdef TEST_ASSERT
  inline static void assert(int e) {
    if (!e) {
      const char * msg = "Assertion Failed!\n";
      write(2, msg, strlen(msg));
      exit(1);
    }
  }
#else
  #include <assert.h>
#endif
 * Mutex to ensure thread safety for the freelist
 
static pthread_mutex_t mutex;
 * Array of sentinel nodes for the freelists
 
header freelistSentinels[N_LISTS];
 * Pointer to the second fencepost in the most recently allocated chunk from
 * the OS. Used for coalescing chunks
 
header * lastFencePost;
 * Pointer to maintian the base of the heap to allow printing based on the
 * distance from the base of the heap
  
void * base;
 * List of chunks allocated by  the OS for printing boundary tags
header * osChunkList [MAX_OS_CHUNKS];
size_t numOsChunks = 0;
 * direct the compiler to run the init function before running main
 * this allows initialization of required globals
 
static void init (void) __attribute__ ((constructor));
// Helper functions for manipulating pointers to headers
static inline header * get_header_from_offset(void * ptr, ptrdiff_t off);
static inline header * get_left_header(header * h);
static inline header * ptr_to_header(void * p);
// Helper functions for allocating more memory from the OS
static inline void initialize_fencepost(header * fp, size_t left_size);
static inline void insert_os_chunk(header * hdr);
static inline void insert_fenceposts(void * raw_mem, size_t size);
static header * allocate_chunk(size_t size);
// Helper functions for freeing a block
static inline void deallocate_object(void * p);
// Helper functions for allocating a block
static inline header * allocate_object(size_t raw_size);
// Helper functions for verifying that the data structures are structurally 
// valid
static inline header * detect_cycles();
static inline header * verify_pointers();
static inline bool verify_freelist();
static inline header * verify_chunk(header * chunk);
static inline bool verify_tags();
static void init();
static bool isMallocInitialized;
 * @brief Helper function to retrieve a header pointer from a pointer and an 
 *        offset
 *
 * @param ptr base pointer
 * @param off number of bytes from base pointer where header is located
 *
 * @return a pointer to a header offset bytes from pointer
 
static inline header * get_header_from_offset(void * ptr, ptrdiff_t off) {
	return (header *)((char *) ptr + off);
}
 * @brief Helper function to get the header to the right of a given header
 *
 * @param h original header
 *
 * @return header to the right of h
 
header * get_right_header(header * h) {
	return get_header_from_offset(h, get_size(h));
}
 * @brief Helper function to get the header to the left of a given header
 *
 * @param h original header
 *
 * @return header to the right of h
 
inline static header * get_left_header(header * h) {
  return get_header_from_offset(h, -h->left_size);
}
 * @brief Fenceposts are marked as always allocated and may need to have
 * a left object size to ensure coalescing happens properly
 *
 * @param fp a pointer to the header being used as a fencepost
 * @param left_size the size of the object to the left of the fencepost
 
inline static void initialize_fencepost(header * fp, size_t left_size) {
	set_state(fp,FENCEPOST);
	set_size(fp, ALLOC_HEADER_SIZE);
	fp->left_size = left_size;
}
bool check_free_block(header *hdr);
void add_to_freelist(header *hdr);
size_t calculate_allocation_size(size_t raw_size);
int find_freelist_index(size_t size);
size_t calculate_allocation_size(size_t raw_size) {
    size_t rounded_size = ((raw_size + ALLOC_HEADER_SIZE + 7) / 8) * 8;
    if (rounded_size < sizeof(header)) {
        return sizeof(header);
    }
    return rounded_size;
}
int get_list_index(size_t size) {
    if (size > (N_LISTS - 1) * 8) {
        return N_LISTS - 1;
    }
    return (size / 8) - 1;
}
bool freeblock_check(header *hdr) {
    int listidx = get_list_index(get_size(hdr) - ALLOC_HEADER_SIZE);
    if (listidx != N_LISTS - 1) {
        return false;
    }
    return true;
}
void insert_freelist(header *hdr) {
    int listidx = get_list_index(get_size(hdr) - ALLOC_HEADER_SIZE);
    header *freelist = &freelistSentinels[listidx];
    if (freelist->next == freelist) {
        freelist->prev = hdr;
    }
    freelist->next->prev = hdr;
    hdr->prev = freelist;
    hdr->next = freelist->next;
    freelist->next = hdr;
    return;
}
 * @brief Helper function to maintain list of chunks from the OS for debugging
 *
 * @param hdr the first fencepost in the chunk allocated by the OS
 
inline static void insert_os_chunk(header * hdr) {
  if (numOsChunks < MAX_OS_CHUNKS) {
    osChunkList[numOsChunks++] = hdr;
  }
}
 * @brief given a chunk of memory insert fenceposts at the left and 
 * right boundaries of the block to prevent coalescing outside of the
 * block
 *
 * @param raw_mem a void pointer to the memory chunk to initialize
 * @param size the size of the allocated chunk
 
inline static void insert_fenceposts(void * raw_mem, size_t size) {
  // Convert to char * before performing operations
  char * mem = (char *) raw_mem;
  // Insert a fencepost at the left edge of the block
  header * leftFencePost = (header *) mem;
  initialize_fencepost(leftFencePost, ALLOC_HEADER_SIZE);
  // Insert a fencepost at the right edge of the block
  header * rightFencePost = get_header_from_offset(mem, size - ALLOC_HEADER_SIZE);
  initialize_fencepost(rightFencePost, size - 2 * ALLOC_HEADER_SIZE);
}
 * @brief Allocate another chunk from the OS and prepare to insert it
 * into the free list
 *
 * @param size The size to allocate from the OS
 *
 * @return A pointer to the allocable block in the chunk (just after the 
 * first fencpost)
 
static header * allocate_chunk(size_t size) {
  void * mem = sbrk(size);
  
  insert_fenceposts(mem, size);
  header * hdr = (header *) ((char *)mem + ALLOC_HEADER_SIZE);
  set_state(hdr, UNALLOCATED);
  set_size(hdr, size - 2 * ALLOC_HEADER_SIZE);
  hdr->left_size = ALLOC_HEADER_SIZE;
  return hdr;
}
 * @brief Helper allocate an object given a raw request size from the user
 *
 * @param raw_size number of bytes the user needs
 *
 * @return A block satisfying the user's request
 
static inline header * allocate_object(size_t raw_size) {
    if (raw_size == 0) {
        return NULL;
    }
    size_t allocated_size = calculate_allocation_size(raw_size);
    int listidx = get_list_index(allocated_size - ALLOC_HEADER_SIZE);
    header *freelist = &freelistSentinels[listidx];
    while (freelist->next == freelist && listidx < N_LISTS - 1) {
        listidx++;
        freelist = &freelistSentinels[listidx];
    }
    header *current;
    for (current = freelist->next; current != freelist || listidx != N_LISTS - 1; current = current->next) {
        size_t current_size = get_size(current);
        if (current_size == allocated_size || (current_size - allocated_size) < sizeof(header)) {
            set_state(current, ALLOCATED);
            current->next->prev = current->prev;
            current->prev->next = current->next;
            return (header *)current->data;
        } else {
            bool isFreeBlock = freeblock_check(current);
            set_size(current, get_size(current) - allocated_size);
            char *newpointer = (char *)current + get_size(current);
            header *newHeader = (header *)newpointer;
            set_size(newHeader, allocated_size);
            newHeader->left_size = get_size(current);
            set_state(newHeader, ALLOCATED);
            char *rightpointer = (char *)newHeader + get_size(newHeader);
            header *rightHeader = (header *)rightpointer;
            rightHeader->left_size = get_size(newHeader);
            if (!(isFreeBlock && get_list_index(get_size(current) - ALLOC_HEADER_SIZE) == N_LISTS - 1)) {
                current->next->prev = current->prev;
                current->prev->next = current->next;
                insert_freelist(current);
            }
            return (header *)newHeader->data;
        }
    }
    header *newHeader = allocate_chunk(ARENA_SIZE);
    header *left_fencepost = get_header_from_offset(newHeader, -ALLOC_HEADER_SIZE);
    header *right_fencepost = get_header_from_offset(newHeader, get_size(newHeader));
    header *last_fencepost = get_header_from_offset(left_fencepost, -ALLOC_HEADER_SIZE);
    if (last_fencepost == lastFencePost) {
        header *last_block = get_left_header(last_fencepost);
        if (get_state(last_block) == UNALLOCATED) {
            bool isFreeBlock = freeblock_check(last_block);
            set_size(last_block, get_size(last_block) + get_size(newHeader) + 2 * ALLOC_HEADER_SIZE);
            set_state(last_block, UNALLOCATED);
            right_fencepost->left_size = get_size(last_block);
            if (!(isFreeBlock && get_list_index(get_size(last_block) - ALLOC_HEADER_SIZE) == N_LISTS - 1)) {
                last_block->next->prev = last_block->prev;
                last_block->prev->next = last_block->next;
                insert_freelist(last_block);
            }
            lastFencePost = right_fencepost;
            return allocate_object(raw_size);
        } else { 
            set_size(last_fencepost, get_size(newHeader) + 2 * ALLOC_HEADER_SIZE);
            right_fencepost->left_size = get_size(last_fencepost);
            set_state(last_fencepost, UNALLOCATED);
            insert_freelist(last_fencepost);
            lastFencePost = right_fencepost;
            return allocate_object(raw_size);
        }
    } else { 
        insert_freelist(newHeader);
        lastFencePost = right_fencepost;
        insert_os_chunk(left_fencepost);
        return allocate_object(raw_size);
    }
}
  if (raw_size == 0) {
	  return NULL;
  }
  size_t total_size = (raw_size + 7) & ~7;
  total_size += ALLOC_HEADER_SIZE;
  if(total_size < sizeof(header)) {
	  total_size = sizeof(header); 
  }
  
  int listInx = listIndex(total_size - ALLOC_HEADER_SIZE);
  for (int i = listInx; i < N_LISTS; i++) {
	  header *freelist = &freelistSentinels[i];
	  
	  for (header *cur = freelist->next; cur != freelist; cur = cur->next) {
		  size_t size = get_size(cur);
		  if (size >= total_size) {
			  if (size == total_size || (size - total_size) < sizeof(header)) {
				  set_state(cur, ALLOCATED);
				  cur->next->prev = cur->prev;
				  cur->prev->next = cur->next;
				  return (header*)cur->data;
			  } else {
				  set_size(cur, size - total_size);
				  header *new = (header *)((char *)cur + get_size(cur));
				  set_size(new, total_size);
				  new->left_size = get_size(cur);
				  set_state(new, ALLOCATED);
				  header *right = (header *)((char *)new + get_size(new));
				  right->left_size = get_size(new);
				  if (listIndex(get_size(cur) - ALLOC_HEADER_SIZE) != N_LISTS - 1) {
					  cur->next->prev = cur->prev;
					  cur->prev->next = cur->next;
					  int ind = listIndex(get_size(cur) - ALLOC_HEADER_SIZE);
					  header * free = &freelistSentinels[ind];
					  cur->next = free->next;
					  cur->prev = free;
					  if (free->next == free) {
						  free->prev = cur;
					  }
					  else {
						  free->next->prev = cur;
					  }
					  free->next = cur;
				  }
				  return (header*)new->data;
			  }
		  }
	  }
  }
  
  header *new = allocate_chunk(ARENA_SIZE);
  header *left = get_header_from_offset(new, -ALLOC_HEADER_SIZE);
  header *right = get_header_from_offset(new, get_size(new));
  header *last = get_header_from_offset(left, -ALLOC_HEADER_SIZE);
  if (last == last) {
	  header *block = get_left_header(last);
	  if (get_state(block) == UNALLOCATED) {
            set_size(block, get_size(block) + get_size(new) + 2 * ALLOC_HEADER_SIZE);
            set_state(block, UNALLOCATED);
            right->left_size = get_size(block);
	    if (listIndex(get_size(block) - ALLOC_HEADER_SIZE) != N_LISTS - 1) {
		    block->next->prev = block->prev;
		    block->prev->next = block->next;
		    int ind = listIndex(get_size(block) - ALLOC_HEADER_SIZE);
		    header * free = &freelistSentinels[ind];
		    block->next = free->next;
		    block->prev = free;
		    if (free->next == free) {
			    free->prev = block;
		    }
		    else {
			    free->next->prev = block;
		    }
		    free->next = block;
	    }
	    last = right;
	    return allocate_object(raw_size);
	  } else {
		  set_size(last, get_size(new) + 2 * ALLOC_HEADER_SIZE);
		  right->left_size = get_size(last);
		  set_state(last, UNALLOCATED);
		  int ind = listIndex(get_size(last) - ALLOC_HEADER_SIZE);
		  header * free = &freelistSentinels[ind];
		  last->next = free->next;
                  last->prev = free;
		  if (free->next == free) {
			  free->prev = last;
		  }
		  else {
			  free->next->prev = last;
		  }
		  free->next = last;
		  last = right;
		  return allocate_object(raw_size);
	  }
  } else {
	  int ind = listIndex(get_size(new) - ALLOC_HEADER_SIZE);
          header * free = &freelistSentinels[ind];
	  new->next = free->next;
	  new->prev = free;
	  if (free->next == free) {
		  free->prev =new;
	  }
	  else {
		  free->next->prev = new;
	  }
	  free->next = new;
	  last = right;
	  insert_os_chunk(left);
	  return allocate_object(raw_size);
    }
  
 * @brief Helper to get the header from a pointer allocated with malloc
 *
 * @param p pointer to the data region of the block
 *
 * @return A pointer to the header of the block
 
static inline header * ptr_to_header(void * p) {
  return (header *)((char *) p - ALLOC_HEADER_SIZE); //sizeof(header));
}
 * @brief Helper to manage deallocation of a pointer returned by the user
 *
 * @param p The pointer returned to the user by a call to malloc
 
static inline void deallocate_object(void * p) {
    if (p == NULL) {
        return;
    }
    bool left_unallocated = false;
    bool right_unallocated = false;
    header *currHeader = get_header_from_offset((char*)p, -ALLOC_HEADER_SIZE);
    if (get_state(currHeader) == UNALLOCATED) {
        printf("Double Free Detected\n");
        printf("Assertion Failed!\n");
        assert(true);
        exit(1);
    }
    set_state(currHeader, UNALLOCATED);
    header *leftHeader = get_left_header(currHeader);
    header *rightHeader = get_right_header(currHeader);
    if (get_state(leftHeader) == UNALLOCATED) {
        left_unallocated = true;
    }
    if (get_state(rightHeader) == UNALLOCATED) {
        right_unallocated = true;
    }
    if (left_unallocated && right_unallocated) {
        bool left_in_freelist = freeblock_check(leftHeader);
        
        set_size(leftHeader, get_size(leftHeader) + get_size(currHeader) + get_size(rightHeader));
        rightHeader->next->prev = rightHeader->prev;
        rightHeader->prev->next = rightHeader->next;
        header *rightrightHeader = get_right_header(rightHeader);
        rightrightHeader->left_size = get_size(leftHeader);
        if (!(left_in_freelist && get_list_index(get_size(leftHeader) - ALLOC_HEADER_SIZE) == N_LISTS - 1)) {
            leftHeader->next->prev = leftHeader->prev;
            leftHeader->prev->next = leftHeader->next;
            insert_freelist(leftHeader);
        }
    } else if (left_unallocated) {
        bool left_in_freelist = freeblock_check(leftHeader);
        set_size(leftHeader, get_size(leftHeader) + get_size(currHeader));
        rightHeader->left_size = get_size(leftHeader);
        if (!(left_in_freelist && get_list_index(get_size(leftHeader) - ALLOC_HEADER_SIZE) == N_LISTS - 1)) {
            leftHeader->next->prev = leftHeader->prev;
            leftHeader->prev->next = leftHeader->next;
            insert_freelist(leftHeader);
        }
    } else if (right_unallocated) {
        set_size(currHeader, get_size(currHeader) + get_size(rightHeader));
        header *rightrightHeader = get_right_header(rightHeader);
        rightrightHeader->left_size = get_size(currHeader);
        rightHeader->next->prev = rightHeader->prev;
        rightHeader->prev->next = rightHeader->next;
        insert_freelist(currHeader);
    } else {
        insert_freelist(currHeader);
    }
    return;
    (void)p;
    assert(false);
    exit(1);
}
  
 
	if (p == NULL) {
		return;
	}
	header *cur = get_header_from_offset((char*)p, -ALLOC_HEADER_SIZE);
	if (get_state(cur) == UNALLOCATED) {
		exit(1);
	}
	set_state(cur, UNALLOCATED);
	header *left = get_left_header(cur);
	header *right = get_right_header(cur);
	int flag1 = (get_state(left) == UNALLOCATED) ? 1 : 0;
	int flag2 = (get_state(right) == UNALLOCATED) ? 1 : 0;
	int ind = listIndex(get_size(left) - ALLOC_HEADER_SIZE);
	if (flag1 && flag2){
		bool boo = true;
		if (ind != N_LISTS - 1) {
			boo= false;
		}
		set_state(left, UNALLOCATED);
		set_size(left, get_size(left) + get_size(cur) + get_size(right));
		right->next->prev = right->prev;
		right->prev->next = right->next;
		header *right2 = get_right_header(right);
		right2->left_size = get_size(left);
		if (!(boo && listIndex(get_size(left) - ALLOC_HEADER_SIZE) == N_LISTS - 1)) {
			left->next->prev = left->prev;
			left->prev->next = left->next;
			int ind = listIndex(get_size(left) - ALLOC_HEADER_SIZE);
			header * free = &freelistSentinels[ind];
			left->next = free->next;
			left->prev = free;
			if (free->next == free) {
				free->prev = left;
			}
			else {
				free->next->prev = left;
			}
			free->next = left;
			return;
		}
	}
	else if (flag1) {
		bool boo = true;
		if (ind != N_LISTS - 1) {
			boo = false;
		}
		set_state(left, UNALLOCATED);
		set_size(left, get_size(left) + get_size(cur));
		right->left_size = get_size(left);
		if (!(boo && listIndex(get_size(left) - ALLOC_HEADER_SIZE) == N_LISTS - 1)) {
			left->next->prev = left->prev;
			left->prev->next = left->next;
			int ind = listIndex(get_size(left) - ALLOC_HEADER_SIZE);
			header * free = &freelistSentinels[ind];
			left->next = free->next;
			left->prev = free;
			if (free->next == free) {
				free->prev = left;
			}
			else {
				free->next->prev = left;
			}
			free->next = left;
		}	
	}else if (flag2) {
	set_state(cur, UNALLOCATED);
	header *right2 = get_right_header(right);
	right2->left_size = get_size(cur) + get_size(right);
	set_size(cur, get_size(cur) + get_size(right));
	right->next->prev = right->prev;
	right->prev->next = right->next;
	int ind = listIndex(get_size(cur) - ALLOC_HEADER_SIZE);
                header * free = &freelistSentinels[ind];
                cur->next = free->next;
                cur->prev = free;
                if (free->next == free) {
                        free->prev = cur;
                }
                else {
                        free->next->prev = cur;
                }
                free->next = cur;
	} else {
	set_state(cur, UNALLOCATED);
	int ind = listIndex(get_size(cur) - ALLOC_HEADER_SIZE);
                header * free = &freelistSentinels[ind];
                cur->next = free->next;
                cur->prev = free;
                if (free->next == free) {
                        free->prev = cur;
                }
                else {
                        free->next->prev = cur;
                }
                free->next = cur;
		return;
	}
 * @brief Helper to detect cycles in the free list
 * https://en.wikipedia.org/wiki/Cycle_detection#Floyd's_Tortoise_and_Hare
 *
 * @return One of the nodes in the cycle or NULL if no cycle is present
 
static inline header * detect_cycles() {
  for (int i = 0; i < N_LISTS; i++) {
    header * freelist = &freelistSentinels[i];
    for (header * slow = freelist->next, * fast = freelist->next->next; 
         fast != freelist; 
         slow = slow->next, fast = fast->next->next) {
      if (slow == fast) {
        return slow;
      }
    }
  }
  return NULL;
}
 * @brief Helper to verify that there are no unlinked previous or next pointers
 *        in the free list
 *
 * @return A node whose previous and next pointers are incorrect or NULL if no
 *         such node exists
 
static inline header * verify_pointers() {
  for (int i = 0; i < N_LISTS; i++) {
    header * freelist = &freelistSentinels[i];
    for (header * cur = freelist->next; cur != freelist; cur = cur->next) {
      if (cur->next->prev != cur || cur->prev->next != cur) {
        return cur;
      }
    }
  }
  return NULL;
}
 * @brief Verify the structure of the free list is correct by checkin for 
 *        cycles and misdirected pointers
 *
 * @return true if the list is valid
 
static inline bool verify_freelist() {
  header * cycle = detect_cycles();
  if (cycle != NULL) {
    fprintf(stderr, "Cycle Detected\n");
    print_sublist(print_object, cycle->next, cycle);
    return false;
  }
  header * invalid = verify_pointers();
  if (invalid != NULL) {
    fprintf(stderr, "Invalid pointers\n");
    print_object(invalid);
    return false;
  }
  return true;
}
 * @brief Helper to verify that the sizes in a chunk from the OS are correct
 *        and that allocated node's canary values are correct
 *
 * @param chunk AREA_SIZE chunk allocated from the OS
 *
 * @return a pointer to an invalid header or NULL if all header's are valid
 */
/*
static inline header * verify_chunk(header * chunk) {
	if (get_state(chunk) != FENCEPOST) {
		fprintf(stderr, "Invalid fencepost\n");
		print_object(chunk);
		return chunk;
	}
	
	for (; get_state(chunk) != FENCEPOST; chunk = get_right_header(chunk)) {
		if (get_size(chunk)  != get_right_header(chunk)->left_size) {
			fprintf(stderr, "Invalid sizes\n");
			print_object(chunk);
			return chunk;
		}
	}
	
	return NULL;
}
 * @brief For each chunk allocated by the OS verify that the boundary tags
 *        are consistent
 *
 * @return true if the boundary tags are valid
 */
/*
static inline bool verify_tags() {
  for (size_t i = 0; i < numOsChunks; i++) {
    header * invalid = verify_chunk(osChunkList[i]);
    if (invalid != NULL) {
      return invalid;
    }
  }
  return NULL;
}
 * @brief Initialize mutex lock and prepare an initial chunk of memory for allocation
 */
/*
static void init() {
  // Initialize mutex for thread safety
  pthread_mutex_init(&mutex, NULL);
#ifdef DEBUG
  // Manually set printf buffer so it won't call malloc when debugging the allocator
  setvbuf(stdout, NULL, _IONBF, 0);
#endif // DEBUG
  // Allocate the first chunk from the OS
  header * block = allocate_chunk(ARENA_SIZE);
  header * prevFencePost = get_header_from_offset(block, -ALLOC_HEADER_SIZE);
  insert_os_chunk(prevFencePost);
  lastFencePost = get_header_from_offset(block, get_size(block));
  // Set the base pointer to the beginning of the first fencepost in the first
  // chunk from the OS
  base = ((char *) block) - ALLOC_HEADER_SIZE; //sizeof(header);
   Initialize freelist sentinels
  for (int i = 0; i < N_LISTS; i++) {
    header * freelist = &freelistSentinels[i];
    freelist->next = freelist;
    freelist->prev = freelist;
  }
  
  // Insert first chunk into the free list
  header * freelist = &freelistSentinels[N_LISTS - 1];
  freelist->next = block;
  freelist->prev = block;
  block->next = freelist;
  block->prev = freelist;
}
 
 * External interface
 */
/*
void * my_malloc(size_t size) {
  pthread_mutex_lock(&mutex);
  header * hdr = allocate_object(size); 
  pthread_mutex_unlock(&mutex);
  return hdr;
}
void * my_calloc(size_t nmemb, size_t size) {
  return memset(my_malloc(size * nmemb), 0, size * nmemb);
}
void * my_realloc(void * ptr, size_t size) {
  void * mem = my_malloc(size);
  memcpy(mem, ptr, size);
  my_free(ptr);
  return mem; 
}
void my_free(void * p) {
  pthread_mutex_lock(&mutex);
  deallocate_object(p);
  pthread_mutex_unlock(&mutex);
}
bool verify() {
  return verify_freelist() && verify_tags();
}
*/


