#pragma once
/*

   ___       __ _      _ _         _
  |_ _|_ _  / _(_)_ _ (_) |_ ___  /_\  _ _ _ _ __ _ _ _ _  _
   | || ' \|  _| | ' \| |  _/ -_)/ _ \| '_| '_/ _` | '_| || |
  |___|_||_|_| |_|_||_|_|\__\___/_/ \_\_| |_| \__,_|_|  \_, |
                                                        |__/

*/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief　loop_buffer本身空间对外暴露是无限大的，可以无限向buffer中push数据，
 * 但由于实际上空间是有限的，当push的数据大于buffer自身的大小的时，将丢弃掉最先push进去的数据
 *假设buffer大小为13
 *1.         [1][2][3][4][5][6][7][8][9][10][11][12][13]
 *2.  <  [1]>[2][3][4][5][6][7][8][9][10][11][12][13][14]
 *3.  <[1][2]>[3][4][5][6][7][8][9][10][11][12][13][14][15]
 *当处于情况１时候，通过infinite_arrary_get_xxxx({0~12});均可以获得相应的有效数据
 *当处于情况２的时候，通过infinite_arrary_get_xxxx({1-13});可以获得相应的有效数据，infinite_arrary_get_xxxx(0),会返回默认值
 */
/**
 * @Usage:
 *  在使用的ｃ文件中头部，调用下面的宏，声明方法。
 *  INFINITE_ARRARY_ENABLE_SUPPORT(uint8_t)
 *
 * void usage(){
 * uint8_t buffer[20] = {0};
 * infinite_arrary_init_uint8_t(&loopbuff, buffer, ARRARY_SIZE(buffer));//初始化
 * infinite_arrary_push_uint8_t(&loopbuff, value);//push一个数据到数组中
 * infinite_arrary_get_uint8_t(&loopbuff, 0); //获得偏移为零的位置的数据
 * }
 *
 */

typedef struct __infinite_arrary_s __infinite_arrary_t;
struct __infinite_arrary_s {
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
void __infinite_arrary_init_xxxx(__infinite_arrary_t *arrary, uint8_t *buf,
                                 size_t capacity, size_t each_element_size);
/**
* @brief internal func don't use it directly
*
* @param buffer
* @param buf
* @param capacity
* @param each_element_size
*/
void __infinite_arrary_push_xxxx(__infinite_arrary_t *arrary, const void *data);
/**
* @brief internal func don't use it directly
*
* @param buffer
* @param buf
* @param capacity
* @param each_element_size
*/
bool __infinite_arrary_get_xxxx(__infinite_arrary_t *arrary, int32_t offset,
                                void *data);
/**
 * @brief infinite_arrary_get_useful_start_offset　获得有效的开始偏移
 *
 * @param buffer
 * @return int32_t
 */
int32_t __infinite_arrary_get_useful_start_offset(__infinite_arrary_t *arrary);
/**
 * @brief infinite_arrary_get_useful_start_offset　获得有效的结束偏移
 *
 * @param buffer
 * @return int32_t
 */
int32_t __infinite_arrary_get_useful_end_offset(__infinite_arrary_t *arrary);

/**
 * @brief 使能对类型type的数据的支持
 *
 * @brief初始化buffer
 * @param arrary handler
 * @param buf 为infinite_arrary_t分配的空间
 * @param capacity 空间的大小
 * infinite_arrary_init_xxxx(__infinite_arrary_t *arrary, type *buf, size_t
 * capacity)
 *
 * @brief push 一个数据到数组中
 * @param arrary handler
 * @param data
 * infinite_arrary_push_xxxx(__infinite_arrary_t *arrary,const type *data)
 *
 * @brief 根据偏移拿到一个数据
 * @param arrary handler
 * @return data
 * type infinite_arrary_get_xxxx(__infinite_arrary_t *arrary,int32_t offset)
 */
#define INFINITE_ARRARY_ENABLE_SUPPORT(name, type)                        \
  typedef struct { __infinite_arrary_t handler; } infinite_arrary_##name; \
  static inline void infinite_arrary_init_##name(                         \
      infinite_arrary_##name *arrary, type *buf, size_t capacity) {       \
    __infinite_arrary_init_xxxx(&(arrary->handler), (uint8_t *)buf,       \
                                capacity * sizeof(type), sizeof(type));   \
  }                                                                       \
  static inline void infinite_arrary_push_##name(                         \
      infinite_arrary_##name *arrary, const type data) {                  \
    __infinite_arrary_push_xxxx(&arrary->handler, (void *)&data);         \
  }                                                                       \
  static inline type infinite_arrary_get_##name(                          \
      infinite_arrary_##name *arrary, int32_t offset) {                   \
    type ret;                                                             \
    if (!__infinite_arrary_get_xxxx(&arrary->handler, offset, &ret)) {    \
      memset(&ret, 0, sizeof(ret));                                       \
    }                                                                     \
    return ret;                                                           \
  }                                                                       \
  static inline int32_t infinite_arrary_get_useful_start_offset_##name(   \
      infinite_arrary_##name *arrary) {                                   \
    return __infinite_arrary_get_useful_start_offset(&arrary->handler);   \
  }                                                                       \
                                                                          \
  static inline int32_t infinite_arrary_get_useful_end_offset_##name(     \
      infinite_arrary_##name *arrary) {                                   \
    return __infinite_arrary_get_useful_end_offset(&arrary->handler);     \
  }

INFINITE_ARRARY_ENABLE_SUPPORT(u8, uint8_t);
INFINITE_ARRARY_ENABLE_SUPPORT(u16, uint16_t);
INFINITE_ARRARY_ENABLE_SUPPORT(u32, uint32_t);
INFINITE_ARRARY_ENABLE_SUPPORT(i8, int8_t);
INFINITE_ARRARY_ENABLE_SUPPORT(i16, int16_t);
INFINITE_ARRARY_ENABLE_SUPPORT(i32, int32_t);
