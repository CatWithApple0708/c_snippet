#include "loop_queue.h"
#include <string.h>

static size_t get_next_offset2(__loop_queue_t *queue, uint32_t cur_off) {
  if (cur_off + queue->each_element_size * 2 <= queue->capacity)
    return cur_off + queue->each_element_size;
  return 0;
}

void __loop_queue_init_xxxx(__loop_queue_t *queue, size_t each_element_size,
                                uint8_t *data, size_t size) {
  queue->buf = data;
  queue->capacity = size;
  queue->read_offset = 0;
  queue->write_offset = 0;
  queue->each_element_size = each_element_size;
}

bool __loop_queue_pop_one_xxxx(__loop_queue_t *queue, void *data) {
  //当前队列为空
  if (__loop_queue_is_empty(queue)) return false;
  memcpy(data, queue->buf + queue->read_offset, queue->each_element_size);
  queue->read_offset = get_next_offset2(queue, queue->read_offset);
  return true;
}

bool __loop_queue_push_one_xxxx(__loop_queue_t *queue, const void *data) {
  //当前queue中数据已经满了
  if (__loop_queue_is_full(queue)) {
    return false;
  }

  uint8_t *write_add = queue->buf + queue->write_offset;

  memcpy(write_add, data, queue->each_element_size);

  queue->write_offset = get_next_offset2(queue, queue->write_offset);

  return true;
}

bool __loop_queue_is_full(__loop_queue_t *queue) {
  if (get_next_offset2(queue, queue->write_offset) == queue->read_offset) {
    return true;
  }
  return false;
}
bool __loop_queue_is_empty(__loop_queue_t *queue) {
  if (queue->read_offset == queue->write_offset) {
    return true;
  }
  return false;
}

size_t __loop_queue_get_size(__loop_queue_t *queue) {
  if (queue->read_offset > queue->write_offset) {
    return (queue->write_offset + queue->capacity - queue->read_offset) /
           queue->each_element_size;
  } else if (queue->read_offset < queue->write_offset) {
    return (queue->write_offset - queue->read_offset) /
           queue->each_element_size;
  } else {
    return 0;
  }
}
