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

/**
 * @brief　loop_buffer本身空间对外暴露是无限大的
 * [1][2][3][4][5][6][7][8][9][10][11][12][13]
 * [2][3][4][5][6][7][8][9][10][11][12][13][14]
 */
typedef struct loop_buffer_s loop_buffer_t;
struct loop_buffer_s {};
uint32_t loop_buffer_get_useful_start_offset(loop_buffer_t *buffer);
uint32_t loop_buffer_get_useful_end_offset(loop_buffer_t *buffer);
/**
 * @brief 获得偏移在offset位置的数据
 * @warning:如果buffer大小50,当存储的数据超过50的时候，则抛弃偏移小的数据，
 * 即如果此时调用loop_buffer_get_data(0)得到的数据是假的的数据，默认返回零,
 * @param offset
 * @return uint8_t
 */
uint8_t loop_buffer_get_data(loop_buffer_t *buffer, int offset);
uint8_t loop_buffer_push_data(loop_buffer_t *buffer, uint8_t data);