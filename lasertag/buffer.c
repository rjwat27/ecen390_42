#include "buffer.h"
#define MAX_CAPACITY 200
// Type of elements in the buffer.
typedef uint32_t buffer_data_t;
volatile static uint32_t elementCount;
volatile static uint32_t indexIn;
volatile static uint32_t indexOut;
volatile static uint32_t data[MAX_CAPACITY];

// Initialize the buffer to empty.
void buffer_init(void) {
  elementCount = 0;
  indexIn = 0;
  indexOut = 0;
}

// Add a value to the buffer. Overwrite the oldest value if full.
void buffer_pushover(buffer_data_t value) {
  // Check if queue is full
  if (elementCount == MAX_CAPACITY) {
    buffer_pop();
  }
  // Increment indexIn after assigning the object, then check if it needs to
  // be looped around to the beginning of the array.
  data[indexIn] = value;
  indexIn++;
  if (indexIn >= MAX_CAPACITY) {
    indexIn = 0;
  }
  elementCount++;
}

// Remove a value from the buffer. Return zero if empty.
buffer_data_t buffer_pop(void) {
  // Check if the queue is empty
  if (elementCount == 0) {
    return 0;
  } else {
    // Increment indexOut after assigning the object, then check if it needs to
    // be looped around to the beginning of the array.
    buffer_data_t temp = data[indexOut++];
    if (indexOut >= MAX_CAPACITY) {
      indexOut = 0;
    }
    elementCount--;
    return temp;
  }
}

// Return the number of elements in the buffer.
uint32_t buffer_elements(void) { return elementCount; }

// Return the capacity of the buffer in elements.
uint32_t buffer_size(void) { return MAX_CAPACITY; }