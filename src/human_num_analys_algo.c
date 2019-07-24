#include "human_num_analys_algo.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "macro.h"

static __attribute__((__unused__)) void build_check_func() {
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
}
/*
   ___ _        _   _
  / __| |_ __ _| |_(_)__
  \__ \  _/ _` |  _| / _|
  |___/\__\__,_|\__|_\__|

*/

// static human_detecter_handler_t human_detecter_handler;
// static human_detecter_handler_t *handlers =
//     &human_detecter_handler;

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

void static push_origion_data_to_buffer(human_detecter_handler_t *handlers,
                                        ir_receive_signal_time_domain_e domain,
                                        uint8_t data) {}
void static push_num_sum_data_to_buffer(human_detecter_handler_t *handlers,
                                        ir_receive_signal_time_domain_e domain,
                                        uint8_t data) {}
void static push_power_average_data_to_buffer(
    human_detecter_handler_t *handlers, ir_receive_signal_time_domain_e domain,
    uint8_t data) {}
void static push_ir_signal_to_signals(human_detecter_handler_t *handlers,
                                      ir_signal_t signal) {}
void static clear_ir_signal_to_signals(human_detecter_handler_t *handlers,
                                       uint8_t offset) {}
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
static void read_origion_data_from_queue_to_buffer(
    human_detecter_handler_t *handlers);
/**
 * @brief 填充 数量求和buffer, 和能量buffer
 *
 * @param offset
 * @return true
 * @return false
 */
static bool prepare_num_sum_and_power_average(
    human_detecter_handler_t *handlers, uint32_t offset);

/**
 * @brief prepare_ir_signals 内部调用
 *
 * @param domain
 * @param cur_offset
 * @param max_offset
 * @param signals_update
 */
static void prepare_ir_signals_internal(human_detecter_handler_t *handlers,
                                        ir_receive_signal_time_domain_e domain,
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
static bool prepare_ir_signals(human_detecter_handler_t *handlers,
                               uint32_t offset, bool *signals_update);
/**
 * @brief 根据ir_signals判断人数和人的移动方向
 *
 */
static void judge_human_num_and_moving_direction(
    human_detecter_handler_t *handlers);

/*
   _            _                   _
  (_)_ __  _ __| |___ _ __  ___ _ _| |_
  | | '  \| '_ \ / -_) '  \/ -_) ' \  _|
  |_|_|_|_| .__/_\___|_|_|_\___|_||_\__|
          |_|
*/
static void read_origion_data_from_queue_to_buffer(
    human_detecter_handler_t *handlers) {
  while (loop_queue_get_size_u8(&handlers->a_origion_data_queue) != 0) {
    uint8_t a_origion_data;
    loop_queue_pop_one_u8(&handlers->a_origion_data_queue, &a_origion_data);
    push_origion_data_to_buffer(handlers, kADomain, a_origion_data);
  }

  while (loop_queue_get_size_u8(&handlers->b_origion_data_queue) != 0) {
    uint8_t b_origion_data;
    loop_queue_pop_one_u8(&handlers->b_origion_data_queue, &b_origion_data);
    push_origion_data_to_buffer(handlers, kBDomain, b_origion_data);
  }
}

/**
 * @brief
 * 数量求和并存储
 * 能量求和求平均
 */
static bool prepare_num_sum_and_power_average(
    human_detecter_handler_t *handlers, uint32_t offset) {
  //如果数据还没有达到窗口的大小，则返回等待数据准备ok
  infinite_arrary_u8 *aod_buffer = &handlers->a_origion_data_buffer;
  infinite_arrary_u8 *bod_buffer = &handlers->b_origion_data_buffer;

  if (offset + kOrigionDataProcessWindowsSize <
      infinite_arrary_get_useful_end_offset_u8(aod_buffer)) {
    /* code */
    return false;
  }

  if (offset + kOrigionDataProcessWindowsSize <
      infinite_arrary_get_useful_end_offset_u8(bod_buffer)) {
    /* code */
    return false;
  }

  //对于num的求和使用的uint8_t 进行存储，当kWindowsSize大于256时候，会溢出.

  uint8_t a_num_sum = 0;
  for (unsigned i = offset; i < kOrigionDataProcessWindowsSize; ++i) {
    a_num_sum = +(infinite_arrary_get_u8(aod_buffer, i) == 0) ? 0 : 1;
  }

  uint8_t b_num_sum = 0;
  for (unsigned i = offset; i < kOrigionDataProcessWindowsSize; ++i) {
    b_num_sum = +(infinite_arrary_get_u8(bod_buffer, i) == 0) ? 0 : 1;
  }

  uint8_t a_power_average = 0;
  {
    uint32_t sum_power = 0;
    for (unsigned i = offset; i < kOrigionDataProcessWindowsSize; ++i) {
      sum_power = +infinite_arrary_get_u8(aod_buffer, i);
    }
    a_power_average = sum_power / kOrigionDataProcessWindowsSize;
  }

  uint8_t b_power_average = 0;
  {
    uint32_t sum_power = 0;
    for (unsigned i = offset; i < kOrigionDataProcessWindowsSize; ++i) {
      sum_power = +infinite_arrary_get_u8(bod_buffer, i);
    }
    b_power_average = sum_power / kOrigionDataProcessWindowsSize;
  }

  push_num_sum_data_to_buffer(handlers, kADomain, a_num_sum);
  push_num_sum_data_to_buffer(handlers, kBDomain, b_num_sum);
  push_power_average_data_to_buffer(handlers, kADomain, a_power_average);
  push_power_average_data_to_buffer(handlers, kBDomain, b_power_average);

  //   *odp_offset+=kOrigionDataProcessWindowsMoveSize;
  return true;
}

static void prepare_ir_signals_internal(human_detecter_handler_t *handlers,
                                        ir_receive_signal_time_domain_e domain,
                                        uint32_t cur_offset,
                                        uint32_t max_offset,
                                        bool *signals_update) {
  infinite_arrary_u8 *num_sum_buf;
  infinite_arrary_u8 *power_average_buf;
  ir_receiver_state_e *ir_receive_state;

  if (domain == kADomain) {
    num_sum_buf = &handlers->a_num_sum_buffer;
    power_average_buf = &handlers->a_power_average_buffer;
    ir_receive_state = &handlers->a_ir_receive_state;
  } else {
    num_sum_buf = &handlers->b_num_sum_buffer;
    power_average_buf = &handlers->b_power_average_buffer;
    ir_receive_state = &handlers->b_ir_receive_state;
  }

  // const static uint8_t kHighPowerThreshold = 75;
  // const static uint8_t kHighNumSumThreshold = 15;
  // const static uint8_t kLowPowerThreshold = 25;
  // const static uint8_t kLowNumSumThreshold = 5;

  // toStartState
  if (*ir_receive_state == kOnReceiveingNothingState) {
    uint8_t curPowerAverageValue =
        infinite_arrary_get_u8(power_average_buf, cur_offset);
    uint8_t cur_num_sum = infinite_arrary_get_u8(num_sum_buf, cur_offset);

    //   判断是否是上升趋势
    bool is_up_tend = true;
    for (unsigned i = cur_offset + 1; i < max_offset; ++i) {
      if (infinite_arrary_get_u8(num_sum_buf, i) <
          infinite_arrary_get_u8(num_sum_buf, i - 1)) {
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
        push_ir_signal_to_signals(handlers, signal);
        *signals_update = true;
      }
    }

  } else if (*ir_receive_state == kOnReceiveingIrDataState) {
    uint8_t curPowerAverageValue =
        infinite_arrary_get_u8(power_average_buf, cur_offset);
    uint8_t cur_num_sum = infinite_arrary_get_u8(num_sum_buf, cur_offset);

    bool is_down_tend = true;
    for (unsigned i = cur_offset + 1; i < max_offset; ++i) {
      if (infinite_arrary_get_u8(num_sum_buf, i) >
          infinite_arrary_get_u8(num_sum_buf, i - 1)) {
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
      push_ir_signal_to_signals(handlers,signal);
      *signals_update = true;
    }
  }
};

static bool prepare_ir_signals(human_detecter_handler_t *handlers,
                               uint32_t offset, bool *signals_update) {
  infinite_arrary_u8 *apa_buffer = &handlers->a_power_average_buffer;

  infinite_arrary_u8 *bpa_buffer = &handlers->b_power_average_buffer;
  //计算A

  if (infinite_arrary_get_useful_end_offset_u8(apa_buffer) <
      offset + kPrepareIrSignalsProcessWindowSize) {
    return false;
  }
  if (infinite_arrary_get_useful_end_offset_u8(bpa_buffer) <
      offset + kPrepareIrSignalsProcessWindowSize) {
    return false;
  }

  prepare_ir_signals_internal(handlers, kADomain, offset,
                              offset + kPrepareIrSignalsProcessWindowSize,
                              signals_update);
  prepare_ir_signals_internal(handlers, kBDomain, offset,
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
static void judge_human_num_and_moving_direction(
    human_detecter_handler_t *handlers) {
  ir_signal_t *signals = handlers->signals;
  if (handlers->signals_size < 4) {
    return;
  }
  //初步过滤掉多余的信号
  // AS,AE,AS,AE , 删除掉 后面的AS,AE
  // BS,BE,BS,BE , 删除掉 后面的BS,BE
  for (unsigned i = 0; i + 4 < handlers->signals_size; ++i) {
    if (signals[i].type == kAStartType && signals[i + 1].type == kAStopType &&
        signals[i + 2].type == kAStartType &&
        signals[i + 3].type == kAStopType) {
      /* code */
      clear_ir_signal_to_signals(handlers,i);
      clear_ir_signal_to_signals(handlers,i + 1);
    } else if (signals[i].type == kAStartType &&
               signals[i + 1].type == kAStopType &&
               signals[i + 2].type == kAStartType &&
               signals[i + 3].type == kAStopType) {
      clear_ir_signal_to_signals(handlers,i);
      clear_ir_signal_to_signals(handlers,i + 1);
    }
  }

  //人数，人的运动方向判断
  bool as_is_ready = false, ae_is_ready = false, bs_is_ready = false,
       be_is_ready = false;
  uint8_t as_of = 0, ae_of = 0, bs_of = 0, be_of = 0;
  for (unsigned i = 0; i < handlers->signals_size; ++i) {
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
      if (handlers->on_judge_result) handlers->on_judge_result(kInDirection, 1);
    } else if (is_increase_order(bs_of, as_of, be_of, ae_of)) {
      // bs,as,be,ae
      //出去一个人
      if (handlers->on_judge_result)
        handlers->on_judge_result(kOutDirection, 1);
    } else if (is_increase_order(as_of, ae_of, bs_of, be_of)) {
      // as ae bs be
      //进来一个人
      if (handlers->on_judge_result) handlers->on_judge_result(kInDirection, 1);
    } else if (is_increase_order(bs_of, be_of, as_of, ae_of)) {
      // bs be as ae
      //出去一个人
      if (handlers->on_judge_result)
        handlers->on_judge_result(kOutDirection, 1);
    }

    //清理处理过的信号
    clear_ir_signal_to_signals(handlers,as_of);
    clear_ir_signal_to_signals(handlers,ae_of);
    clear_ir_signal_to_signals(handlers,bs_of);
    clear_ir_signal_to_signals(handlers,be_of);
  }

  if (handlers->signals_size == ARRARY_SIZE(handlers->signals)) {
    // TODO:这里打印条警告，代码不应该运行到这里
  }
}
/*
   ___     _
  | __|_ _| |_ ___ _ _ _ _
  | _|\ \ /  _/ -_) '_| ' \
  |___/_\_\\__\___|_| |_||_|

*/
void human_num_analys_process(human_detecter_handler_t *handlers) {
  /**
   * 0. 从LoopQueue中读取数据到x_origion_data_buffer中
   * 这里之所以使用Queue,是因为红外数据的接收和处理，二者是异步,需要加锁。
   * 加锁会导致在中断中使用锁，从而导致死锁。循环队列，天然可以无锁操作
   */
  read_origion_data_from_queue_to_buffer(handlers);

  // 1. 数量求和并存储
  // 2. 能量求和求平均
  while (true) {
    uint32_t *odp_offset = &handlers->orgion_data_process_offset;
    if (prepare_num_sum_and_power_average(handlers ,* odp_offset) == false) {
      break;
    }

    uint32_t *pre_irs_offset = &handlers->prepare_ir_signals_process_offset;
    bool ir_signal_update = false;
    if (prepare_ir_signals(handlers ,* pre_irs_offset, &ir_signal_update) ==
        false) {
      *odp_offset = *odp_offset + kOrigionDataProcessWindowsMoveSize;
      continue;
    }

    if (ir_signal_update) {
      judge_human_num_and_moving_direction(handlers);
    }

    *pre_irs_offset = *pre_irs_offset + 1;
    *odp_offset = *odp_offset + kOrigionDataProcessWindowsMoveSize;
  }
};

/**
 * @brief 模块初始化
 *
 */
void human_num_analys_init(human_detecter_handler_t *handlers) {
  
}

/**
 * @brief 压入一个新的红外原始数据
 *
 * @param doamin
 * @param origion_data (0--100)
 */
void human_num_analys_push_origion_data(human_detecter_handler_t *handlers,
                                        ir_receive_signal_time_domain_e doamin,
                                        uint8_t origion_data) {}
/**
 * @brief 人数检测分析程序，应当运行在某个定时器中，每当数据更新的时候，运行一次
 */

/**
 * @brief 获得human_num_analys_process所期望的被调度的周期
 *
 * @return int
 */
int human_num_analys_get_recommend_process_period(
    human_detecter_handler_t *handlers) {
  return kRecomendedProcessPeriod;
}
/**
 * @brief 每当重新开启一次新的计算的时候，用来复位状态
 *
 */
void human_num_analys_reset(human_detecter_handler_t *handlers) {}