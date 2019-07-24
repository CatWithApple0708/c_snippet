#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "infinite_arrary.h"
#include "loop_queue.h"
/*IrSignals数组的Size---存放 AStart,AEnd,BStart,Bend的数组*/
#define kSignalArrarySzie (10)

/*原始数据移动窗口的大小*/
#define kOrigionDataProcessWindowsSize (20)
/*原始数据窗口每次移动的距离*/
#define kOrigionDataProcessWindowsMoveSize (1)
/*利用数量和，和能量均值，做相应计算时候的窗口的大小*/
#define kPrepareIrSignalsProcessWindowSize (4)
/*能量均值的最大值*/
#define kMaxPowerAverage (100)
/*数量统计的求和的最大值，应该等于原始数据移动窗口的大小*/
#define kMaxNumSum (20)
/*能量均值的最小值*/
#define kMinPowerAverage (0)
/*数量统计的求和的最小值*/
#define kMinNumSum (0)

/*判断信号是Start信号的能量阈值*/
#define kHighPowerThreshold (75)
/*判断信号是Start信号的数量和阈值*/
#define kHighNumSumThreshold (15)
/*判断信号是End信号的能量阈值*/
#define kLowPowerThreshold (25)
/*判断信号是End信号的数量和阈值*/
#define kLowNumSumThreshold (5)

#define kRecomendedProcessPeriod (16)

#define kLoopQueueSize (50)
#define kInfiniteArrarySize (100)

/**
 * @brief 红外接收信号的时域
 *
 */
typedef enum {
  kADomain,
  kBDomain,
} ir_receive_signal_time_domain_e;

/**
 * @brief 人移动的方向
 */
typedef enum {
  kInDirection,
  kOutDirection,
} human_moving_direction_t;

/**
 * @brief ir_signal_type,分别为AStart,BStart,AEnd,BEnd
 */
typedef enum {
  kAStartType = 1,
  kAStopType = 2,
  kBStartType = 3,
  kBStopType = 4,
} ir_receive_signal_type_e;

/**
 * @是否处于红外接收状态
 * AStart ----接收到红外信号-----AEnd
 *      _______________________
 *______|                      |_____________
 *      kOnReceiveingIrDataState-----kOnReceiveingNothingState
 */
typedef enum {
  kOnReceiveingNothingState = 0,
  kOnReceiveingIrDataState = 1,  //处于红外接收态
} ir_receiver_state_e;

typedef struct ir_signal_s ir_signal_t;
/**
 * @brief 接收到的红外信号
 *
 */
struct ir_signal_s {
  ir_receive_signal_type_e type;
  //数量求和buffer，或者能量平均buffer的offset
  uint32_t offset;
};

typedef void (*on_judge_result_t)(human_moving_direction_t direction, int num);
typedef struct human_detecter_handler_s human_detecter_handler_t;

struct human_detecter_handler_s {
  // WARNING:此处存储能量，能量按照百分比存储即数值0-100,发送的红外全部收到100
  uint8_t a_origion_data_queue_buf[kLoopQueueSize];
  uint8_t b_origion_data_queue_buf[kLoopQueueSize];

  uint8_t a_origion_data_buffer_buf[kInfiniteArrarySize];
  uint8_t b_origion_data_buffer_buf[kInfiniteArrarySize];
  uint8_t a_num_sum_buffer_buf[kInfiniteArrarySize];
  uint8_t b_num_sum_buffer_buf[kInfiniteArrarySize];
  uint8_t a_power_average_buffer_buf[kInfiniteArrarySize];
  uint8_t b_power_average_buffer_buf[kInfiniteArrarySize];

  loop_queue_u8 a_origion_data_queue;
  loop_queue_u8 b_origion_data_queue;

  // 原始数据buffer
  infinite_arrary_u8 a_origion_data_buffer;
  infinite_arrary_u8 b_origion_data_buffer;

  // 数量统计 buffer
  infinite_arrary_u8 a_num_sum_buffer;
  infinite_arrary_u8 b_num_sum_buffer;

  // 能量统计 buffer
  infinite_arrary_u8 a_power_average_buffer;
  infinite_arrary_u8 b_power_average_buffer;

  // AB开始结束
  ir_signal_t signals[kSignalArrarySzie];
  uint8_t signals_size;
  /**
   * @brief 表示原始数据已经有多少已经被处理了
   */
  uint32_t orgion_data_process_offset;

  /**
   * @brief 依赖 num_sum 和　power_average去计算signal时候的偏移
   */
  uint32_t prepare_ir_signals_process_offset;

  ir_receiver_state_e a_ir_receive_state;
  ir_receiver_state_e b_ir_receive_state;

  on_judge_result_t on_judge_result;
};
/**
 * @brief 模块初始化
 *
 */
void human_num_analys_init(human_detecter_handler_t* handlers);

/**
 * @brief 压入一个新的红外原始数据
 *
 * @param doamin
 * @param origion_data (0--100)
 */
void human_num_analys_push_origion_data(human_detecter_handler_t* handlers,
                                        ir_receive_signal_time_domain_e doamin,
                                        uint8_t origion_data);
/**
 * @brief 人数检测分析程序，应当运行在某个定时器中，每当数据更新的时候，运行一次
 */
void human_num_analys_process(human_detecter_handler_t* handlers);

/**
 * @brief 获得human_num_analys_process所期望的被调度的周期
 *
 * @return int 返回推荐的处理周期ms
 */
int human_num_analys_get_recommend_process_period(
    human_detecter_handler_t* handlers);
/**
 * @brief 每当重新开启一次新的计算的时候，用来复位状态
 *
 */
void human_num_analys_reset(human_detecter_handler_t* handlers);
