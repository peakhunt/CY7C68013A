#pragma once

#include <stdint.h>

#define RING_BUFFER_SIZE 128 // Must be power of 2 for the mask to work

typedef struct
{
  uint8_t buffer[RING_BUFFER_SIZE];
  uint8_t head; // Write index
  uint8_t tail; // Read index
} RingBuffer;

// a) Returns the number of bytes currently stored
static inline uint8_t
rb_available(RingBuffer *rb)
{
  return (uint8_t)(rb->head - rb->tail) & (RING_BUFFER_SIZE - 1);
}

// b) Adds new data
static inline void
rb_push(RingBuffer *rb, uint8_t data)
{
  rb->buffer[rb->head] = data;
  rb->head = (rb->head + 1) & (RING_BUFFER_SIZE - 1); // Wraps 128 to 0
}

// c) Removes and returns data
static inline uint8_t
rb_pop(RingBuffer *rb)
{
  uint8_t data = rb->buffer[rb->tail];
  rb->tail = (rb->tail + 1) & (RING_BUFFER_SIZE - 1); // Wraps 128 to 0
  return data;
}
