#include "queue.h"
#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define MALLOC_FAILURE_MESSAGE "Malloc failed to return a valid memory location\n"
#define QUEUE_FULL_ERROR "Push attempted while queue was already full\n"
#define QUEUE_EMPTY_ERROR "Pull attempted while queue was already empty\n"
// Allocates memory for the queue (the data* pointer) and initializes all
// parts of the data structure. Prints out an error message if malloc() fails
// and calls assert(false) to print-out line-number information and die.
// The queue is empty after initialization. To fill the queue with known
// values (e.g. zeros), call queue_overwritePush() up to queue_size() times.
void queue_init(queue_t *q, queue_size_t size, const char *name){
    q->data = malloc(sizeof(queue_data_t) * size);
    if(q->data == NULL){
        printf(MALLOC_FAILURE_MESSAGE);
        assert(false);
    }
    q->indexIn = 0;
    q->indexOut = 0;
    q->elementCount = 0;
    q->underflowFlag = false;
    q->overflowFlag = false;
    q->size = size;
    strcpy(q->name, name);
}

// Get the user-assigned name for the queue.
const char *queue_name(queue_t *q){
    return q->name;
}

// Returns the capacity of the queue.
queue_size_t queue_size(queue_t *q){
    return q->size;
}

// Returns true if the queue is full.
bool queue_full(queue_t *q){
    return q->elementCount == q->size;
}

// Returns true if the queue is empty.
bool queue_empty(queue_t *q){
    return q->elementCount == 0;
}

// If the queue is not full, pushes a new element into the queue and clears the
// underflowFlag. IF the queue is full, set the overflowFlag, print an error
// message and DO NOT change the queue.
void queue_push(queue_t *q, queue_data_t value){
    q->underflowFlag = false;
    // Check if the queue is full
    if(queue_full(q)){
        q->overflowFlag = true;
        printf(QUEUE_FULL_ERROR);
    }else{
        // Increment indexIn after assigning the object, then check if it needs to be looped around to the beginning of the array.
        q->data[q->indexIn] = value;
        q->indexIn++;
        if(q->indexIn >= q->size){
            q->indexIn = 0;
        }
        q->elementCount++;
    }
}

// If the queue is not empty, remove and return the oldest element in the queue.
// If the queue is empty, set the underflowFlag, print an error message, and DO
// NOT change the queue.
queue_data_t queue_pop(queue_t *q){
    q->overflowFlag = false;
    // Check if the queue is empty
    if(queue_empty(q)){
        q->underflowFlag = true;
        printf(QUEUE_EMPTY_ERROR);
    }else{
        // Increment indexOut after assigning the object, then check if it needs to be looped around to the beginning of the array.
        queue_data_t temp = q->data[q->indexOut++];
        if(q->indexOut >= q->size){
            q->indexOut = 0;
        }
        q->elementCount--;
        return temp;
    }
}

// If the queue is full, call queue_pop() and then call queue_push().
// If the queue is not full, just call queue_push().
void queue_overwritePush(queue_t *q, queue_data_t value){
    if(queue_full(q)){
        queue_pop(q);
    }
    queue_push(q, value);
}

// Provides random-access read capability to the queue.
// Low-valued indexes access older queue elements while higher-value indexes
// access newer elements (according to the order that they were added). Print a
// meaningful error message if an error condition is detected.
queue_data_t queue_readElementAt(queue_t *q, queue_index_t index){
    return q->data[(index + q->indexOut) % q->size];
}

// Returns a count of the elements currently contained in the queue.
queue_size_t queue_elementCount(queue_t *q){
    return q->elementCount;
}

// Returns true if an underflow has occurred (queue_pop() called on an empty
// queue).
bool queue_underflow(queue_t *q){
    return q->underflowFlag;
}

// Returns true if an overflow has occurred (queue_push() called on a full
// queue).
bool queue_overflow(queue_t *q){
    return q->overflowFlag;
}

// Frees the storage that you malloc'd before.
void queue_garbageCollect(queue_t *q){
    free(q->data);
}
