#pragma once
/*
   _                  ___       __  __
  | |   ___  ___ _ __| _ )_  _ / _|/ _|___ _ _
  | |__/ _ \/ _ \ '_ \ _ \ || |  _|  _/ -_) '_|
  |____\___/\___/ .__/___/\_,_|_| |_| \___|_|
                |_|
*/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
/**
 * @brief 获得偏移在offset位置的数据
 * @warning:如果buffer大小50,当存储的数据超过50的时候，则抛弃偏移小的数据，
 * 即如果此时调用loop_buffer_get_data(0)得到的数据是假的的数据，默认返回零,
 * @param offset
 * @return uint8_t
 */

/**
 * @brief　loop_buffer本身空间对外暴露是无限大的
 * [1][2][3][4][5][6][7][8][9][10][11][12][13]
 * [2][3][4][5][6][7][8][9][10][11][12][13][14]
 */
typedef struct loop_buffer_s loop_buffer_t;
struct loop_buffer_s {
  uint8_t *buf;
  size_t capacity;

  int32_t useful_start_offset;  // real_offset = read_offset*each_element_size
  int32_t useful_end_offset;
  size_t write_offset;
  size_t each_element_size;
};
/**
 * @brief internal func don't use it directly
 *
 * @param buffer
 * @param buf
 * @param capacity
 * @param each_element_size
 */
void __loop_buffer_init_xxxx(loop_buffer_t *buffer, uint8_t *buf,
                             size_t capacity, size_t each_element_size);
void __loop_buffer_push_xxxx(loop_buffer_t *buffer, const void *data);
bool __loop_buffer_get_xxxx(loop_buffer_t *buffer, int32_t offset, void *data);
int32_t loop_buffer_get_useful_start_offset(loop_buffer_t *buffer);
int32_t loop_buffer_get_useful_end_offset(loop_buffer_t *buffer);

#define LOOP_BUFFER_ENABLE_SUPPORT(type)                                       \
  static inline void loop_buffer_init_##type(loop_buffer_t *buffer, type *buf, \
                                             size_t capacity) {                \
    __loop_buffer_init_xxxx(buffer, (uint8_t *)buf, capacity * sizeof(type),   \
                            sizeof(type));                                     \
  }                                                                            \
  static inline void loop_buffer_push_##type(loop_buffer_t *buffer,            \
                                             const type *data) {               \
    __loop_buffer_push_xxxx(buffer, (void *)data);                             \
  }                                                                            \
  static inline type loop_buffer_get_##type(loop_buffer_t *buffer,             \
                                            int32_t offset) {                  \
    type ret;                                                                  \
    if (!__loop_buffer_get_xxxx(buffer, offset, &ret)) {                       \
      memset(&ret, 0, sizeof(ret));                                            \
    }                                                                          \
    return ret;                                                                \
  }

// LOOP_BUFFER_ENABLE_SUPPORT(uint8_t);
// LOOP_BUFFER_ENABLE_SUPPORT(uint16_t);
// LOOP_BUFFER_ENABLE_SUPPORT(uint32_t);
