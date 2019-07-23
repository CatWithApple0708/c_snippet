#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "loop_buffer.h"
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
  uint32_t offset = 0;
};

typedef void (*on_judge_result_t)(human_moving_direction_t direction, int num);
struct human_detecter_handler_s;
typedef struct human_detecter_handler_s human_detecter_handler_t;

