/*
 * mm_alloc.c
 * 
 * Implements dynamic memory allocation.
 */

#include "mm_alloc.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Represents TRUE or FALSE.
 */
typedef enum {
    true,
    false
} bool;

/*
 * A doubly linked list containing information about memory blocks.
 */
typedef struct info_list {
    struct info_list* next; /* The next element of the list */
    struct info_list* prev; /* The previous element of the list */
    int size;               /* Size of allocated memory */
    bool free;              /* True if memory block is free */
} info_list;

/* The head of the list, initialized to NULL. */
void* head = NULL;

/*
 * Requests a chunk (block) of memory.
 */
info_list* request_block(info_list* last, size_t size) {
    info_list* block = sbrk(size + sizeof(info_list));

    if (block == (void*) -1) {
        return NULL;
    }

    if (last != NULL) {
        last->next = block;
        block->prev = last;
    }
    block->next = NULL;
    block->size = size;
    block->free = false;
    return block;
}

/*
 * Finds the next available block of memory. If the block is larger than
 * needed, splits the block into blocks of sizes SIZE and 
 * CURR_BLOCK->SIZE - SIZE.
 */
info_list* find_free_block(size_t size) {
    info_list* curr_block = head;

    while (curr_block != NULL 
            && (curr_block->free == false || curr_block->size < size)) {
        curr_block = curr_block->next;
    }

    if (curr_block != NULL && curr_block->size > size) {
        size_t temp = curr_block->size - size;
        curr_block->size = size;
        void* loc = (void*) ((char*) curr_block + size + sizeof(info_list));
        info_list* temp_block;
        temp_block = (info_list*) loc;
        curr_block->next = temp_block;
        temp_block->size = temp;
        temp_block->next = NULL;
        temp_block->free = true;
        temp_block->prev = curr_block;
    }

    return curr_block;
}

/*
 * Allocates a zero-initialized memory block of size SIZE.
 */
void* mm_malloc(size_t size) {    
    if (size <= 0) {
        return NULL;
    }
    
    info_list* block;
    if (head == NULL) {
        block = request_block(head, size);
        if (block == NULL) {
            return NULL;
        }
        head = block;
    } else {
        block = find_free_block(size);
        if (block == NULL) {
            block = request_block(head, size);
            if (block == NULL) {
                return NULL;
            }
        } else {
            block->free = false;
        }
    }
    return memset(block + 1, 0, block->size);
}

/*
 * Reallocates a block of memory with a different size.
 * Otherwise, functions the same as mm_malloc().
 */
void* mm_realloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return mm_malloc(size);
    }

    info_list* block = (info_list*) ptr - 1; 
    int curr_size = block->size;
    if (size < curr_size) {
        curr_size = size;
    }
   
    void* ptr_2;
    if ((ptr_2 =/*=*/ mm_malloc(size)) == NULL) {
        return NULL;
    }
    
    memcpy(ptr_2, ptr, curr_size);
    mm_free(ptr);
    return ptr_2; 
}

/*
 * Frees a block of memory. If adjacent chunks are also free, coalesces into
 * one block.
 */
void mm_free(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    info_list* block = (info_list*) ptr - 1;
    if (block != NULL) {

        info_list* n = block->next;
        info_list* p = block->prev;

        if (n != NULL && n->free == true) {
            info_list* temp = n->next;
            block->next = temp;
            block->size += n->size;
            if (temp != NULL) {
                temp->prev = block;
            }
        }

        if (p != NULL && p->free == true) {
            info_list* temp = block->next;
            p->next = block->next;
            p->size += block->size;
            temp->prev = p;
            block = p;
        }

        block->free = true;
    }
}
