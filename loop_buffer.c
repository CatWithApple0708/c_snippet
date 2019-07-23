#include "loop_buffer.h"

uint32_t loop_buffer_get_useful_start_offset(loop_buffer_t *buffer) {
  return 0;
}
uint32_t loop_buffer_get_useful_end_offset(loop_buffer_t *buffer) { return 0; }
uint8_t loop_buffer_get_data(loop_buffer_t *buffer, int offset) { return 0; }
uint8_t loop_buffer_push_data(loop_buffer_t *buffer, uint8_t data) { return 0; }
