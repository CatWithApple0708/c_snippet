#include "human_num_analys_algo.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "macro.h"

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

struct human_detecter_handler_s {
  // WARNING:此处存储能量，能量按照百分比存储即数值0-100,发送的红外全部收到100
  loop_queue_t a_origion_data_queue;
  loop_queue_t b_origion_data_queue;

  // 原始数据buffer
  loop_buffer_t a_origion_data_buffer;
  loop_buffer_t b_origion_data_buffer;


  // 能量统计 buffer
  loop_buffer_t a_num_average_buffer;
  loop_buffer_t b_num_average_buffer;

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

/*
   ___ _        _   _
  / __| |_ __ _| |_(_)__
  \__ \  _/ _` |  _| / _|
  |___/\__\__,_|\__|_\__|

*/

static human_detecter_handler_t human_detecter_handler;
static human_detecter_handler_t *s_phuman_detecter_handler =
    &human_detecter_handler;

/**
 * @brief 构造 ir_signal_t
 *
 * @param domain
 * @param isStartSignal
 * @param offset
 * @return ir_signal_t
 */
static ir_signal_t build_ir_signal(ir_receive_signal_time_domain_e domain,
                                   bool isStartSignal, uint32_t offset) {
  ir_receive_signal_type_e type;
  if (domain == kADomain) {
    type = isStartSignal ? kAStartType : kAStopType;
  } else {
    type = isStartSignal ? kBStartType : kBStopType;
  }
  ir_signal_t result = {
      .type = type,
      .offset = offset,
  };
  return result;
}
static bool is_increase_order(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4) {
  if (a1 < a2 && a2 < a3 && a3 < a4) {
    return true;
  }
  return false;
}

void static push_origion_data_to_buffer(ir_receive_signal_time_domain_e domain,
                                        uint8_t data) {}
void static push_num_sum_data_to_buffer(ir_receive_signal_time_domain_e domain,
                                        uint8_t data) {}
void static push_power_average_data_to_buffer(
    ir_receive_signal_time_domain_e domain, uint8_t data) {}
void static push_ir_signal_to_signals(ir_signal_t signal) {}
void static clear_ir_signal_to_signals(uint8_t offset) {}
void human_detecter_process_start(){};
/*
      _        _
   __| |___ __| |__ _ _ _ ___
  / _` / -_) _| / _` | '_/ -_)
  \__,_\___\__|_\__,_|_| \___|

*/

/**
 * @brief 从原始数据的Queue中，将数据读到buffer中
 *
 */
static void read_origion_data_from_queue_to_buffer();
/**
 * @brief 填充 数量求和buffer, 和能量buffer
 *
 * @param offset
 * @return true
 * @return false
 */
static bool prepare_num_sum_and_power_average(uint32_t offset);

/**
 * @brief prepare_ir_signals 内部调用
 *
 * @param domain
 * @param cur_offset
 * @param max_offset
 * @param signals_update
 */
static void prepare_ir_signals_internal(ir_receive_signal_time_domain_e domain,
                                        uint32_t cur_offset,
                                        uint32_t max_offset,
                                        bool *signals_update);
/**
 * @brief 填充ir_signals
 *
 * @param domain
 * @param cur_offset
 * @param max_offset
 * @param signals_update
 */
static bool prepare_ir_signals(uint32_t offset, bool *signals_update);
/**
 * @brief 根据ir_signals判断人数和人的移动方向
 *
 */
static void judge_human_num_and_moving_direction();

/*
   _            _                   _
  (_)_ __  _ __| |___ _ __  ___ _ _| |_
  | | '  \| '_ \ / -_) '  \/ -_) ' \  _|
  |_|_|_|_| .__/_\___|_|_|_\___|_||_\__|
          |_|
*/
static void read_origion_data_from_queue_to_buffer() {
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
static bool prepare_num_sum_and_power_average(uint32_t offset) {
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

static void prepare_ir_signals_internal(ir_receive_signal_time_domain_e domain,
                                        uint32_t cur_offset,
                                        uint32_t max_offset,
                                        bool *signals_update) {
  loop_buffer_t *num_sum_buf;
  loop_buffer_t *num_average_buf;
  ir_receiver_state_e *ir_receive_state;

  if (domain == kADomain) {
    num_average_buf = &s_phuman_detecter_handler->a_num_average_buffer;
    ir_receive_state = &s_phuman_detecter_handler->a_ir_receive_state;
  } else {
    num_average_buf = &s_phuman_detecter_handler->b_num_average_buffer;
    ir_receive_state = &s_phuman_detecter_handler->b_ir_receive_state;
  }

  // toStartState
  if (*ir_receive_state == kOnReceiveingNothingState) {
    uint8_t curPowerAverageValue =
        loop_buffer_get_data(num_average_buf, cur_offset);

    //   判断是否是上升趋势
    bool is_up_tend = true;
    for (unsigned i = cur_offset + 1; i < max_offset; ++i) {
      if (loop_buffer_get_data(num_average_buf, i) <
          loop_buffer_get_data(num_average_buf, i - 1)) {
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
        loop_buffer_get_data(num_average_buf, cur_offset);

    bool is_down_tend = true;
    for (unsigned i = cur_offset + 1; i < max_offset; ++i) {
      if (loop_buffer_get_data(num_average_buf, i) >
          loop_buffer_get_data(num_average_buf, i - 1)) {
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

static bool prepare_ir_signals(uint32_t offset, bool *signals_update) {
  loop_buffer_t *apa_buffer =
      &s_phuman_detecter_handler->a_num_average_buffer;

  loop_buffer_t *bpa_buffer =
      &s_phuman_detecter_handler->b_num_average_buffer;
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
/**
 * @brief 
 *
 */

//判断方式:
// 首先找到一个开始，然后等待其对应的结束。
// 判断开始和结束之间是否存在另外一个信号的开始，如果不存在，则抛弃当前这两个信号的开始和结束
// 否则等待这个信号的结束的到来。
// 根据这四个信号判断人移动的方向
//　判断完成后抛弃这四个信号，对剩下的信号做相同的处理。
static void judge_human_num_and_moving_direction() {
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
/*
   ___     _
  | __|_ _| |_ ___ _ _ _ _
  | _|\ \ /  _/ -_) '_| ' \
  |___/_\_\\__\___|_| |_||_|

*/
void human_num_analys_process() {
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
