#include <stdbool.h>
#include <stdint.h>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "loop_queue.h"
#include "loop_buffer.h"
#include "macro.h"
using namespace std;
#if 0
uint8_t table[] = {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                   20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
                   20, 20, 20, 20, 20, 20, 20, 0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
#define ARRARY_SIZE(arrary) sizeof(arrary) / sizeof(arrary[0])

uint8_t average(uint8_t *begin, uint32_t useful_size, uint32_t totalNum) {
  uint32_t sum = 0;
  for (uint32_t i = 0; i < useful_size; ++i) {
    sum += begin[i];
  }
  return sum / totalNum;
}
#endif


/*

   _  _                      ___      _          _
  | || |_  _ _ __  __ _ _ _ |   \ ___| |_ ___ __| |_ ___ _ _
  | __ | || | '  \/ _` | ' \| |) / -_)  _/ -_) _|  _/ -_) '_|
  |_||_|\_,_|_|_|_\__,_|_||_|___/\___|\__\___\__|\__\___|_|


*/
// 约定:
// 1. 收到的原始
//
//
const static uint8_t kSignalArrarySzie = 10;

const static uint8_t kOrigionDataProcessWindowsSize = 20;
const static uint8_t kOrigionDataProcessWindowsMoveSize = 1;
const static uint8_t kPrepareIrSignalsProcessWindowSize = 4;

const static uint8_t kMaxPowerAverage = 100;
const static uint8_t kMaxNumSum = 20;  //

const static uint8_t kMinPowerAverage = 0;
const static uint8_t kMinNumSum = 0;  //

const static uint8_t kHighPowerThreshold = 75;
const static uint8_t kHighNumSumThreshold = 15;
const static uint8_t kLowPowerThreshold = 25;
const static uint8_t kLowNumSumThreshold = 5;

BUILD_ASSERT(kOrigionDataProcessWindowsSize <= 256);
BUILD_ASSERT(kOrigionDataProcessWindowsSize != 0);
//计算理由，人通过短时间内触发的信号最多5个，
// AS,AE,BS,AS(第二个人),BE，如果数据被及时处理，则buffer中不会积累过多的数据
//当出现类似AS,AE,BS,AS,AE,AS,AE, 将会移除一组AS,AE
BUILD_ASSERT(kSignalArrarySzie >= 10);
BUILD_ASSERT(kMaxNumSum == kOrigionDataProcessWindowsSize);
BUILD_ASSERT(kHighPowerThreshold < kMaxPowerAverage);
BUILD_ASSERT(kHighNumSumThreshold < kMaxNumSum);
BUILD_ASSERT(kLowPowerThreshold < kMaxPowerAverage);
BUILD_ASSERT(kLowNumSumThreshold < kMaxNumSum);

typedef enum {
  kADomain,
  kBDomain,
} ir_receive_signal_time_domain_e;

typedef enum {
  kInDirection,
  kOutDirection,
} human_moving_direction_t;

typedef enum {
  kAStartType = 1,
  kAStopType = 2,
  kBStartType = 3,
  kBStopType = 4,
} ir_receive_signal_type_e;

typedef enum {
  kOnReceiveingNothingState = 0,
  kOnReceiveingIrDataState = 1,  //处于红外接收态
} ir_receiver_state_e;

typedef struct ir_signal_s ir_signal_t;
struct ir_signal_s {
  ir_receive_signal_type_e type;
  //数量求和buffer，或者能量平均buffer的offset
  uint32_t offset = 0;
};
ir_signal_t build_ir_signal(ir_receive_signal_time_domain_e domain,
                            bool isStartSignal, uint32_t offset) {
  ir_receive_signal_type_e type;
  if (domain == kADomain) {
    type = isStartSignal ? kAStartType : kAStopType;
  } else {
    type = isStartSignal ? kBStartType : kBStopType;
  }
  ir_signal_t result = {
    .type = type, .offset = offset,
  };
  return result;
}

typedef void (*on_judge_result_t)(human_moving_direction_t direction, int num);
typedef struct human_detecter_handler_s human_detecter_handler_t;
struct human_detecter_handler_s {
  // WARNING:此处存储能量，能量按照百分比存储即数值0-100,发送的红外全部收到100
  loop_queue_t a_origion_data_queue;
  loop_queue_t b_origion_data_queue;

  // 原始数据buffer
  loop_buffer_t a_origion_data_buffer;
  loop_buffer_t b_origion_data_buffer;

  // 数量统计 buffer
  loop_buffer_t a_num_sum_buffer;
  loop_buffer_t b_num_sum_buffer;

  // 能量统计 buffer
  loop_buffer_t a_power_average_buffer;
  loop_buffer_t b_power_average_buffer;

  // AB开始结束
  ir_signal_t signals[kSignalArrarySzie];
  uint8_t signals_size = 0;
  /**
   * @brief 表示原始数据已经有多少已经被处理了
   */
  uint32_t orgion_data_process_offset = 0;

  /**
   * @brief 依赖 num_sum 和　power_average去计算signal时候的偏移
   */
  uint32_t prepare_ir_signals_process_offset = 0;

  ir_receiver_state_e a_ir_receive_state;
  ir_receiver_state_e b_ir_receive_state;

  on_judge_result_t on_judge_result;
};

static human_detecter_handler_t human_detecter_handler;
static human_detecter_handler_t *s_phuman_detecter_handler =
    &human_detecter_handler;

void push_origion_data_to_buffer(ir_receive_signal_time_domain_e domain,
                                 uint8_t data) {}
void push_num_sum_data_to_buffer(ir_receive_signal_time_domain_e domain,
                                 uint8_t data) {}
void push_power_average_data_to_buffer(ir_receive_signal_time_domain_e domain,
                                       uint8_t data) {}
void push_ir_signal_to_signals(ir_signal_t signal) {}
void clear_ir_signal_to_signals(uint8_t offset) {}
void human_detecter_process_start(){
    //清空所有数据
};



void read_origion_data_from_queue_to_buffer() {
  while (loop_queue_get_size(&human_detecter_handler.a_origion_data_queue) !=
         0) {
    uint8_t a_origion_data =
        loop_queue_read_one(&human_detecter_handler.a_origion_data_queue);
    push_origion_data_to_buffer(kADomain, a_origion_data);
  }

  while (loop_queue_get_size(&human_detecter_handler.b_origion_data_queue) !=
         0) {
    uint8_t b_origion_data =
        loop_queue_read_one(&human_detecter_handler.b_origion_data_queue);
    push_origion_data_to_buffer(kBDomain, b_origion_data);
  }
}

/**
 * @brief
 * 数量求和并存储
 * 能量求和求平均
 */
bool prepare_num_sum_and_power_average(uint32_t offset) {
  //如果数据还没有达到窗口的大小，则返回等待数据准备ok
  loop_buffer_t *aod_buffer = &s_phuman_detecter_handler->a_origion_data_buffer;
  loop_buffer_t *bod_buffer = &s_phuman_detecter_handler->b_origion_data_buffer;

  if (offset + kOrigionDataProcessWindowsSize <
      loop_buffer_get_useful_end_offset(aod_buffer)) {
    /* code */
    return false;
  }

  if (offset + kOrigionDataProcessWindowsSize <
      loop_buffer_get_useful_end_offset(bod_buffer)) {
    /* code */
    return false;
  }

  //对于num的求和使用的uint8_t 进行存储，当kWindowsSize大于256时候，会溢出.

  uint8_t a_num_sum = 0;
  for (unsigned i = offset; i < kOrigionDataProcessWindowsSize; ++i) {
    a_num_sum = +(loop_buffer_get_data(aod_buffer, i) == 0) ? 0 : 1;
  }

  uint8_t b_num_sum = 0;
  for (unsigned i = offset; i < kOrigionDataProcessWindowsSize; ++i) {
    b_num_sum = +(loop_buffer_get_data(bod_buffer, i) == 0) ? 0 : 1;
  }

  uint8_t a_power_average = 0;
  {
    uint32_t sum_power = 0;
    for (unsigned i = offset; i < kOrigionDataProcessWindowsSize; ++i) {
      sum_power = +loop_buffer_get_data(aod_buffer, i);
    }
    a_power_average = sum_power / kOrigionDataProcessWindowsSize;
  }

  uint8_t b_power_average = 0;
  {
    uint32_t sum_power = 0;
    for (unsigned i = offset; i < kOrigionDataProcessWindowsSize; ++i) {
      sum_power = +loop_buffer_get_data(bod_buffer, i);
    }
    b_power_average = sum_power / kOrigionDataProcessWindowsSize;
  }

  push_num_sum_data_to_buffer(kADomain, a_num_sum);
  push_num_sum_data_to_buffer(kBDomain, b_num_sum);
  push_power_average_data_to_buffer(kADomain, a_power_average);
  push_power_average_data_to_buffer(kBDomain, b_power_average);

  //   *odp_offset+=kOrigionDataProcessWindowsMoveSize;
  return true;
}

void prepare_ir_signals_internal(ir_receive_signal_time_domain_e domain,
                                 uint32_t cur_offset, uint32_t max_offset,
                                 bool *signals_update) {
  loop_buffer_t *num_sum_buf;
  loop_buffer_t *power_average_buf;
  ir_receiver_state_e *ir_receive_state;

  if (domain == kADomain) {
    num_sum_buf = &s_phuman_detecter_handler->a_num_sum_buffer;
    power_average_buf = &s_phuman_detecter_handler->a_power_average_buffer;
    ir_receive_state = &s_phuman_detecter_handler->a_ir_receive_state;
  } else {
    num_sum_buf = &s_phuman_detecter_handler->b_num_sum_buffer;
    power_average_buf = &s_phuman_detecter_handler->b_power_average_buffer;
    ir_receive_state = &s_phuman_detecter_handler->b_ir_receive_state;
  }

// const static uint8_t kHighPowerThreshold = 75;
// const static uint8_t kHighNumSumThreshold = 15;
// const static uint8_t kLowPowerThreshold = 25;
// const static uint8_t kLowNumSumThreshold = 5;

  // toStartState
  if (*ir_receive_state == kOnReceiveingNothingState) {
    uint8_t curPowerAverageValue =
        loop_buffer_get_data(power_average_buf, cur_offset);
    uint8_t cur_num_sum = loop_buffer_get_data(num_sum_buf, cur_offset);

    //   判断是否是上升趋势
    bool is_up_tend = true;
    for (unsigned i = cur_offset + 1; i < max_offset; ++i) {
      if (loop_buffer_get_data(num_sum_buf, i) <
          loop_buffer_get_data(num_sum_buf, i - 1)) {
        is_up_tend = false;
        break;
      }
    }
    bool useful = false;

    if (curPowerAverageValue >= kHighPowerThreshold &&
        cur_num_sum >= kHighNumSumThreshold && is_up_tend) {
      useful = true;
    } else if (curPowerAverageValue == kMaxPowerAverage &&
               cur_num_sum == kMaxNumSum) {
      useful = true;
    }

    if (useful) {
      if (curPowerAverageValue >= kHighPowerThreshold &&
          cur_num_sum >= kHighNumSumThreshold && is_up_tend) {
        ir_signal_t signal =
            build_ir_signal(domain, true /*isStartSignal*/, cur_offset);
        push_ir_signal_to_signals(signal);
        *signals_update = true;
      }
    }

  } else if (*ir_receive_state == kOnReceiveingIrDataState) {
    uint8_t curPowerAverageValue =
        loop_buffer_get_data(power_average_buf, cur_offset);
    uint8_t cur_num_sum = loop_buffer_get_data(num_sum_buf, cur_offset);

    bool is_down_tend = true;
    for (unsigned i = cur_offset + 1; i < max_offset; ++i) {
      if (loop_buffer_get_data(num_sum_buf, i) >
          loop_buffer_get_data(num_sum_buf, i - 1)) {
        is_down_tend = false;
        break;
      }
    }

    bool useful = false;
    if (curPowerAverageValue <= kLowPowerThreshold &&
        cur_num_sum <= kLowNumSumThreshold && is_down_tend) {
      useful = true;
    } else if (curPowerAverageValue == kMinPowerAverage &&
               cur_num_sum == kMinNumSum) {
      useful = true;
    }

    /*TODO:complete it*/
    if (useful) {
      ir_signal_t signal =
          build_ir_signal(domain, false /*isStartSignal*/, cur_offset);
      push_ir_signal_to_signals(signal);
      *signals_update = true;
    }
  }
};

bool prepare_ir_signals(uint32_t offset, bool *signals_update) {
  loop_buffer_t *ans_buffer = &s_phuman_detecter_handler->a_num_sum_buffer;
  loop_buffer_t *apa_buffer =
      &s_phuman_detecter_handler->a_power_average_buffer;

  loop_buffer_t *bns_buffer = &s_phuman_detecter_handler->b_num_sum_buffer;
  loop_buffer_t *bpa_buffer =
      &s_phuman_detecter_handler->b_power_average_buffer;
  //计算A

  if (loop_buffer_get_useful_end_offset(apa_buffer) <
      offset + kPrepareIrSignalsProcessWindowSize) {
    return false;
  }
  if (loop_buffer_get_useful_end_offset(bpa_buffer) <
      offset + kPrepareIrSignalsProcessWindowSize) {
    return false;
  }

  prepare_ir_signals_internal(kADomain, offset,
                              offset + kPrepareIrSignalsProcessWindowSize,
                              signals_update);
  prepare_ir_signals_internal(kBDomain, offset,
                              offset + kPrepareIrSignalsProcessWindowSize,
                              signals_update);
  return true;
}

bool is_increase_order(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4) {
  if (a1 < a2 && a2 < a3 && a3 < a4) {
    return true;
  }
  return false;
}

//判断方式:
// 首先找到一个开始，然后等待其对应的结束。
// 判断开始和结束之间是否存在另外一个信号的开始，如果不存在，则抛弃当前这两个信号的开始和结束
// 否则等待这个信号的结束的到来。
// 根据这四个信号判断人移动的方向
//　判断完成后抛弃这四个信号，对剩下的信号做相同的处理。
void judge_human_num_and_moving_direction() {
  ir_signal_t *signals = s_phuman_detecter_handler->signals;
  if (s_phuman_detecter_handler->signals_size < 4) {
    return;
  }
  //初步过滤掉多余的信号
  // AS,AE,AS,AE , 删除掉 后面的AS,AE
  // BS,BE,BS,BE , 删除掉 后面的BS,BE
  for (unsigned i = 0; i + 4 < s_phuman_detecter_handler->signals_size; ++i) {
    if (signals[i].type == kAStartType && signals[i + 1].type == kAStopType &&
        signals[i + 2].type == kAStartType &&
        signals[i + 3].type == kAStopType) {
      /* code */
      clear_ir_signal_to_signals(i);
      clear_ir_signal_to_signals(i + 1);
    } else if (signals[i].type == kAStartType &&
               signals[i + 1].type == kAStopType &&
               signals[i + 2].type == kAStartType &&
               signals[i + 3].type == kAStopType) {
      clear_ir_signal_to_signals(i);
      clear_ir_signal_to_signals(i + 1);
    }
  }

  //人数，人的运动方向判断
  bool as_is_ready, ae_is_ready, bs_is_ready, be_is_ready = false;
  uint8_t as_of, ae_of, bs_of, be_of = 0;
  for (unsigned i = 0; i < s_phuman_detecter_handler->signals_size; ++i) {
    if (signals[i].type == kAStartType) {
      as_is_ready = true;
      as_of = i;
    } else if (signals[i].type == kAStopType) {
      ae_is_ready = true;
      ae_of = i;
    } else if (signals[i].type == kBStartType) {
      bs_is_ready = true;
      bs_of = i;
    } else if (signals[i].type == kBStopType) {
      be_is_ready = true;
      be_of = i;
    }
    if (as_is_ready && ae_is_ready && bs_is_ready && be_is_ready) {
      break;
    }
  }

  if (as_is_ready && ae_is_ready && bs_is_ready && be_is_ready) {
    if (is_increase_order(as_of, bs_of, ae_of, be_of)) {
      // as<bs<ae<be
      //进来一个人
      if (s_phuman_detecter_handler->on_judge_result)
        s_phuman_detecter_handler->on_judge_result(kInDirection, 1);
    } else if (is_increase_order(bs_of, as_of, be_of, ae_of)) {
      // bs,as,be,ae
      //出去一个人
      if (s_phuman_detecter_handler->on_judge_result)
        s_phuman_detecter_handler->on_judge_result(kOutDirection, 1);
    } else if (is_increase_order(as_of, ae_of, bs_of, be_of)) {
      // as ae bs be
      //进来一个人
      if (s_phuman_detecter_handler->on_judge_result)
        s_phuman_detecter_handler->on_judge_result(kInDirection, 1);
    } else if (is_increase_order(bs_of, be_of, as_of, ae_of)) {
      // bs be as ae
      //出去一个人
      if (s_phuman_detecter_handler->on_judge_result)
        s_phuman_detecter_handler->on_judge_result(kOutDirection, 1);
    }

    //清理处理过的信号
    clear_ir_signal_to_signals(as_of);
    clear_ir_signal_to_signals(ae_of);
    clear_ir_signal_to_signals(bs_of);
    clear_ir_signal_to_signals(be_of);
  }

  if (s_phuman_detecter_handler->signals_size ==
      ARRARY_SIZE(s_phuman_detecter_handler->signals)) {


    // TODO:这里打印条警告，代码不应该运行到这里
  }
}
//AS AE BS AS AE AS AE AS AE AS AE
void onTimer() {
  /**
   * 0. 从LoopQueue中读取数据到x_origion_data_buffer中
   * 这里之所以使用Queue,是因为红外数据的接收和处理，二者是异步,需要加锁。
   * 加锁会导致在中断中使用锁，从而导致死锁。循环队列，天然可以无锁操作
   */
  read_origion_data_from_queue_to_buffer();

  // 1. 数量求和并存储
  // 2. 能量求和求平均
  while (true) {
    uint32_t *odp_offset =
        &s_phuman_detecter_handler->orgion_data_process_offset;
    if (prepare_num_sum_and_power_average(*odp_offset) == false) {
      break;
    }

    uint32_t *pre_irs_offset =
        &s_phuman_detecter_handler->prepare_ir_signals_process_offset;
    bool ir_signal_update = false;
    if (prepare_ir_signals(*pre_irs_offset, &ir_signal_update) == false) {
      *odp_offset = *odp_offset + kOrigionDataProcessWindowsMoveSize;
      continue;
    }

    if (ir_signal_update) {
      judge_human_num_and_moving_direction();
    }

    *pre_irs_offset = *pre_irs_offset + 1;
    *odp_offset = *odp_offset + kOrigionDataProcessWindowsMoveSize;
  }
};

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @retrn int
 */

// 进去的情况
// AS   BS   AE   BE
//               AS   BS   AE   BE

//
// AS   BS   AE  AS BE
//
// 简单判断
//  AS   BS   AE   BE
//                      AS   BS   AE   BE
//
//  AS   BS   AE   BE
//         AS   BS   AE   BE
//         ||
//         ||
// 无法区分
//  AS   BS  xxxxxx
//                  AE   BE

// 合法:
// AS BS AE BE
// AS BS AE AS BE
// BS AS BE AE
// BS AS BE BS AE
// 不合法:
// AS BS BE AE
// BS AS AE BE

int main(int argc, char *const argv[]) { return 0; }