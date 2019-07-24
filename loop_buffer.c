#include "loop_buffer.h"
#include <string.h>


static size_t get_next_offset2(loop_buffer_t *queue, uint32_t cur_off) {
  if (cur_off + queue->each_element_size * 2 <= queue->capacity)
    return cur_off + queue->each_element_size;
  return 0;
}

void __loop_buffer_init_xxxx(loop_buffer_t *buffer, uint8_t *buf,
                             size_t capacity, size_t each_element_size) {
  buffer->buf = buf;
  buffer->capacity = capacity - capacity % each_element_size;
  buffer->each_element_size = each_element_size;
  buffer->useful_start_offset = -1;
  buffer->useful_end_offset = -1;
  buffer->write_offset = 0;
  return;
}
void __loop_buffer_push_xxxx(loop_buffer_t *buffer, const void *data) {
  memcpy(buffer->buf + buffer->write_offset, data, buffer->each_element_size);
  buffer->write_offset = get_next_offset2(buffer, buffer->write_offset);
  buffer->useful_end_offset += 1;
  if (buffer->useful_start_offset == -1) {
    buffer->useful_start_offset += 1;
  }
  if (buffer->useful_end_offset + 1 - buffer->useful_start_offset >
      buffer->capacity / buffer->each_element_size) {
    buffer->useful_start_offset += 1;
  }
}
bool __loop_buffer_get_xxxx(loop_buffer_t *buffer, int32_t offset, void *data) {
  if (offset < buffer->useful_start_offset) {
    return false;
  }

  if (offset > buffer->useful_end_offset) {
    return false;
  }

  size_t real_off =
      (buffer->write_offset + buffer->capacity -
       (buffer->useful_end_offset + 1 - offset) * buffer->each_element_size) %
      buffer->capacity;
  memcpy(data, buffer->buf + real_off, buffer->each_element_size);
  return true;
}
int32_t loop_buffer_get_useful_start_offset(loop_buffer_t *buffer) {
  return buffer->useful_start_offset;
}
int32_t loop_buffer_get_useful_end_offset(loop_buffer_t *buffer) {
  return buffer->useful_end_offset;
}