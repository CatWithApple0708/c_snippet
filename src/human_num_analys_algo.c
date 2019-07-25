#include "human_num_analys_algo.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef BUILD_ASSERT
#define BUILD_ASSERT(cond)               \
  do {                                   \
    (void)sizeof(char[1 - 2 * !(cond)]); \
  } while (0)
#endif

#ifndef ARRARY_SIZE
#define ARRARY_SIZE(arrary) sizeof(arrary) / sizeof(arrary[0])
#endif

static __attribute__((__unused__)) void build_check_func() {
  BUILD_ASSERT(kOrigionDataProcessWindowsSize <= 256);
  BUILD_ASSERT(kOrigionDataProcessWindowsSize != 0);
  //计算理由，人通过短时间内触发的信号最多5个，
  // AS,AE,BS,AS(第二个人),BE，如果数据被及时处理，则buffer中不会积累过多的数据
  //当出现类似AS,AE,BS,AS,AE,AS,AE, 将会移除一组AS,AE
  BUILD_ASSERT(kSignalArrarySzie >= 10);
  BUILD_ASSERT(kHighNumSumAverageThreshold < kMaxNumSumAverage);
  BUILD_ASSERT(kHighNumSumAverageThreshold > kMinNumSumAverage);
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
  ir_signal_t result = {.type = type, .offset = offset, .remove_flag = false};
  return result;
}
static bool is_increase_order(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4) {
  if (a1 < a2 && a2 < a3 && a3 < a4) {
    return true;
  }
  return false;
}
/**
 * @brief push一个原始数据到buffer中
 * 
 * @param handlers 
 * @param domain 
 * @param data 
 */
void static push_origion_data_to_buffer(human_detecter_handler_t *handlers,
                                        ir_receive_signal_time_domain_e domain,
                                        uint8_t data) {
  if (domain == kADomain) {
    infinite_arrary_push_u8(&handlers->a_origion_data, data);
  } else {
    infinite_arrary_push_u8(&handlers->b_origion_data, data);
  }
}
void static push_num_sum_average_to_buffer(
    human_detecter_handler_t *handlers, 
    uint8_t aData,uint8_t bData) {
    infinite_arrary_push_u8(&handlers->a_num_sum_average, aData);
    infinite_arrary_push_u8(&handlers->b_num_sum_average, bData);
  if (handlers->on_new_num_sum_average)
    handlers->on_new_num_sum_average(handlers,aData, bData);
}
/**
 * @brief push一个红外信号到AStart AEnd BStart Bend到signals中
 *
 * @param handlers
 * @param signal
 */
void static push_ir_signal_to_signals(human_detecter_handler_t *handlers,
                                      ir_signal_t signal) {
  if (handlers->n_singls == ARRARY_SIZE(handlers->signals)) {
    return;
  }
  //状态切换
  if (signal.type == kAStartType) {
    handlers->a_ir_receive_state = kOnReceiveingIrDataState;
  } else if (signal.type == kAStopType) {
    handlers->a_ir_receive_state = kOnReceiveingNothingState;
  } else if (signal.type == kBStartType) {
    handlers->b_ir_receive_state = kOnReceiveingIrDataState;
  } else if (signal.type == kBStopType) {
    handlers->b_ir_receive_state = kOnReceiveingNothingState;
  }
  handlers->signals[handlers->n_singls] = signal;
  handlers->n_singls++;

  if (handlers->on_new_signal) handlers->on_new_signal(handlers, signal);
}
/**
 * @brief 清理掉remove_flag == true的 ir_signal
 *
 * @param handlers
 * @param offset 在signals数组中的偏移
 */
void static clear_ir_signal_to_signals(human_detecter_handler_t *handlers) {
  if (!handlers->some_signals_need_to_removed) return;
  ir_signal_t signals[kSignalArrarySzie];
  int16_t n_signals = 0;
  for (int16_t i = 0; i < handlers->n_singls; i++) {
    if (!handlers->signals[i].remove_flag) {
      signals[n_signals] = handlers->signals[i];
      n_signals++;
    }
  }
  memcpy(handlers->signals, signals, n_signals);
  handlers->n_singls = n_signals;
  handlers->some_signals_need_to_removed = false;
}
/**
 * @brief 设置ir_signal_remove_flag =
 * true,注意在调用完该方法后，需要调用clear_ir_signal_to_signals才能真正的将ir_singnal从数组中清理掉
 *
 * @param signal
 */
void static ir_signals_set_remove_flag(human_detecter_handler_t *handlers,
                                       ir_signal_t *signal) {
  handlers->some_signals_need_to_removed = true;
  signal->remove_flag = true;
}
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
static bool prepare_num_sum_average(human_detecter_handler_t *handlers,
                                    uint32_t offset);

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
                                        uint32_t window_size,
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
static bool prepare_num_sum_average(human_detecter_handler_t *handlers,
                                    uint32_t offset) {
  //如果数据还没有达到窗口的大小，则返回等待数据准备ok
  infinite_arrary_u8 *aod_buffer = &handlers->a_origion_data;
  infinite_arrary_u8 *bod_buffer = &handlers->b_origion_data;
  if (infinite_arrary_get_useful_end_offset_u8(aod_buffer) <
      (int32_t)(offset + kOrigionDataProcessWindowsSize)) {
    /* code */
    return false;
  }

  if (infinite_arrary_get_useful_end_offset_u8(bod_buffer) <
      (int32_t)(offset + kOrigionDataProcessWindowsSize)) {
    /* code */
    return false;
  }

  //对于num的求和使用的uint8_t 进行存储，当kWindowsSize大于256时候，会溢出.

  uint8_t a_num_sum_average = 0;
  {
    uint32_t sum_power = 0;
    for (unsigned i = offset; i < offset + kOrigionDataProcessWindowsSize;
         ++i) {
      sum_power += infinite_arrary_get_u8(aod_buffer, i);
    }
    a_num_sum_average = sum_power / kOrigionDataProcessWindowsSize;
  }

  uint8_t b_num_sum_average = 0;
  {
    uint32_t sum_power = 0;
    for (unsigned i = offset; i < offset + kOrigionDataProcessWindowsSize;
         ++i) {
      sum_power += infinite_arrary_get_u8(bod_buffer, i);
    }
    b_num_sum_average = sum_power / kOrigionDataProcessWindowsSize;
  }

  push_num_sum_average_to_buffer(handlers, a_num_sum_average,
                                 b_num_sum_average);
  return true;
}

static bool is_up_tend(infinite_arrary_u8 *num_average_buf, uint32_t cur_offset,
                       uint32_t window_size) {
  //判断趋势是上升的条件
  //最大偏移的max_offset拿到的num_average_buf数据大于偏移cur_offset所在的数据
  //连续几个点每个点的数据要大于等于上一个的点的数据
  bool max_lagger_than_min_flag = false;
  if (infinite_arrary_get_u8(num_average_buf, cur_offset + window_size - 1) >
      infinite_arrary_get_u8(num_average_buf, cur_offset)) {
    max_lagger_than_min_flag = true;
  }

  bool is_up_tend = true;
  for (unsigned i = cur_offset + 1; i < cur_offset + window_size; ++i) {
    if (infinite_arrary_get_u8(num_average_buf, i) >=
        infinite_arrary_get_u8(num_average_buf, i - 1)) {
      continue;
    }
    is_up_tend = false;
    break;
  }
  if (max_lagger_than_min_flag && is_up_tend) return true;
  return false;
}
static bool is_down_tend(infinite_arrary_u8 *num_average_buf,
                         uint32_t cur_offset, uint32_t window_size) {
  //逻辑同is_up_tend
  bool max_little_than_min_flag = false;
  if (infinite_arrary_get_u8(num_average_buf, cur_offset + window_size - 1) <
      infinite_arrary_get_u8(num_average_buf, cur_offset)) {
    max_little_than_min_flag = true;
  }

  bool is_dow_tend = true;
  for (unsigned i = cur_offset + 1; i < cur_offset + window_size; ++i) {
    if (infinite_arrary_get_u8(num_average_buf, i) <=
        infinite_arrary_get_u8(num_average_buf, i - 1)) {
      continue;
    }
    is_dow_tend = false;
    break;
  }
  if (max_little_than_min_flag && is_dow_tend) return true;
  return false;
}

static void prepare_ir_signals_internal(human_detecter_handler_t *handlers,
                                        ir_receive_signal_time_domain_e domain,
                                        uint32_t cur_offset,
                                        uint32_t windows_size,
                                        bool *signals_update) {
  infinite_arrary_u8 *num_average_buf;
  ir_receiver_state_e *ir_receive_state;

  if (domain == kADomain) {
    num_average_buf = &handlers->a_num_sum_average;
    ir_receive_state = &handlers->a_ir_receive_state;
  } else {
    num_average_buf = &handlers->b_num_sum_average;
    ir_receive_state = &handlers->b_ir_receive_state;
  }

  // toStartState
  if (*ir_receive_state == kOnReceiveingNothingState) {
    uint8_t curNumAverageValue =
        infinite_arrary_get_u8(num_average_buf, cur_offset);
    //   判断是否是上升趋势

    bool is_up_tend_flag =
        is_up_tend(num_average_buf, cur_offset, windows_size);

    bool useful = false;
    if (curNumAverageValue >= kHighNumSumAverageThreshold && is_up_tend_flag) {
      useful = true;
    } else if (curNumAverageValue >= kSecondHighNumSumAverageThreshold) {
      useful = true;
    }

    if (useful) {
      ir_signal_t signal =
          build_ir_signal(domain, true /*isStartSignal*/, cur_offset);
      push_ir_signal_to_signals(handlers, signal);
      *signals_update = true;
    }

  } else if (*ir_receive_state == kOnReceiveingIrDataState) {
    uint8_t curNumAverageValue =
        infinite_arrary_get_u8(num_average_buf, cur_offset);

    bool is_down_tend_flag =
        is_down_tend(num_average_buf, cur_offset, windows_size);

    bool useful = false;
    if (curNumAverageValue <= kLowNumSumAverageThreshold && is_down_tend_flag) {
      useful = true;
    } else if (curNumAverageValue <= kSecondLowNumSumAverageThreshold) {
      useful = true;
    }

    /*TODO:complete it*/
    if (useful) {
      ir_signal_t signal =
          build_ir_signal(domain, false /*isStartSignal*/, cur_offset);
      push_ir_signal_to_signals(handlers, signal);
      *signals_update = true;
    }
  }
};
/**
 * @brief 准备　AS AE BS BE信号
 *
 * @param handlers
 * @param offset
 * @param signals_update
 * @return true
 * @return false
 */
static bool prepare_ir_signals(human_detecter_handler_t *handlers,
                               uint32_t offset, bool *signals_update) {
  //检查计算所需要的数据是否充足，不充足则返回
  if (infinite_arrary_get_useful_end_offset_u8(&handlers->a_num_sum_average) <
      (int32_t)(offset + kPrepareIrSignalsProcessWindowSize)) {
    return false;
  }
  if (infinite_arrary_get_useful_end_offset_u8(&handlers->b_num_sum_average) <
      (int32_t)(offset + kPrepareIrSignalsProcessWindowSize)) {
    return false;
  }
  //准备AS,或者AE
  prepare_ir_signals_internal(handlers, kADomain, offset,
                              kPrepareIrSignalsProcessWindowSize,
                              signals_update);
  //准备BS,或者BE
  prepare_ir_signals_internal(handlers, kBDomain, offset,
                              kPrepareIrSignalsProcessWindowSize,
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
  if (handlers->n_singls < 4) {
    return;
  }
  //初步过滤掉多余的信号
  // AS,AE,AS,AE , 删除掉 后面的AS,AE
  // BS,BE,BS,BE , 删除掉 后面的BS,BE
  for (unsigned i = 0; i + 4 < handlers->n_singls; ++i) {
    if (signals[i].type == kAStartType && signals[i + 1].type == kAStopType &&
        signals[i + 2].type == kAStartType &&
        signals[i + 3].type == kAStopType) {
      /* code */
      ir_signals_set_remove_flag(handlers,&signals[i]);
      ir_signals_set_remove_flag(handlers,&signals[i+1]);
    } else if (signals[i].type == kAStartType &&
               signals[i + 1].type == kAStopType &&
               signals[i + 2].type == kAStartType &&
               signals[i + 3].type == kAStopType) {
      ir_signals_set_remove_flag(handlers,&signals[i]);
      ir_signals_set_remove_flag(handlers,&signals[i + 1]);
    }
  }
  clear_ir_signal_to_signals(handlers);

  //人数，人的运动方向判断
  bool as_is_ready = false, ae_is_ready = false, bs_is_ready = false,
       be_is_ready = false;
  uint8_t as_of = 0, ae_of = 0, bs_of = 0, be_of = 0;
  for (unsigned i = 0; i < handlers->n_singls; ++i) {
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
      // as<bs<ae<be 1,3,2,4
      //进来一个人
      if (handlers->on_judge_result)
        handlers->on_judge_result(handlers, kInDirection, 1);
    } else if (is_increase_order(bs_of, as_of, be_of, ae_of)) {
      // bs,as,be,ae 3,1,4,2
      //出去一个人
      if (handlers->on_judge_result)
        handlers->on_judge_result(handlers, kOutDirection, 1);
    } else if (is_increase_order(as_of, ae_of, bs_of, be_of)) {
      // as ae bs be 1,2,3,4
      //进来一个人
      if (handlers->on_judge_result)
        handlers->on_judge_result(handlers, kInDirection, 1);
    } else if (is_increase_order(bs_of, be_of, as_of, ae_of)) {
      // bs be as ae 3,4,1,2
      //出去一个人
      if (handlers->on_judge_result)
        handlers->on_judge_result(handlers, kOutDirection, 1);
    }

    //清理处理过的信号
    ir_signals_set_remove_flag(handlers,&signals[as_of]);
    ir_signals_set_remove_flag(handlers,&signals[ae_of]);
    ir_signals_set_remove_flag(handlers,&signals[bs_of]);
    ir_signals_set_remove_flag(handlers,&signals[be_of]);

    clear_ir_signal_to_signals(handlers);
  }

  if (handlers->n_singls == ARRARY_SIZE(handlers->signals)) {
    // TODO:这里打印条警告，代码不应该运行到这里
  }
}
/*
   ___     _
  | __|_ _| |_ ___ _ _ _ _
  | _|\ \ /  _/ -_) '_| ' \
  |___/_\_\\__\___|_| |_||_|
*/
/**
 * @brief 人数检测分析程序，应当运行在某个定时器中，每当数据更新的时候，运行一次
 */
void human_num_analys_process(human_detecter_handler_t *handlers) {
  /**
   * 0. 从LoopQueue中读取数据到x_origion_data_buffer中
   * 这里之所以使用Queue,是因为红外数据的接收和处理，二者是异步,需要加锁。
   * 加锁会导致在中断中使用锁，从而导致死锁。循环队列，天然可以无锁操作
   */
  read_origion_data_from_queue_to_buffer(handlers);

  while (true) {
    //准备num_sum_average
    uint32_t *odp_offset = &handlers->orgion_data_process_offset;
    if (prepare_num_sum_average(handlers, *odp_offset) == false) {
      break;
    }

    uint32_t *pre_irs_offset = &handlers->prepare_ir_signals_process_offset;
    bool ir_signal_update = false;
    //准备AStart AEnd BStart BEnd信号
    if (prepare_ir_signals(handlers, *pre_irs_offset, &ir_signal_update) ==
        false) {
      *odp_offset = *odp_offset + kOrigionDataProcessWindowsMoveSize;
      continue;
    }
    //判断人移动的方向
    if (ir_signal_update) {
      judge_human_num_and_moving_direction(handlers);
    }

    *pre_irs_offset = *pre_irs_offset + 1;
    *odp_offset = *odp_offset + kOrigionDataProcessWindowsMoveSize;
  }
};

/**
 * @brief 模块初始化
 */
void human_num_analys_init(human_detecter_handler_t *handlers,
                           on_judge_result_t on_judge_result) {
  memset(handlers, 0, sizeof(human_detecter_handler_t));
  loop_queue_init_u8(&handlers->a_origion_data_queue,
                     handlers->a_origion_data_queue_buf,
                     ARRARY_SIZE(handlers->a_origion_data_queue_buf));
  loop_queue_init_u8(&handlers->b_origion_data_queue,
                     handlers->b_origion_data_queue_buf,
                     ARRARY_SIZE(handlers->b_origion_data_queue_buf));

  infinite_arrary_init_u8(&handlers->a_origion_data,
                          handlers->a_origion_data_buf,
                          ARRARY_SIZE(handlers->a_origion_data_buf));
  infinite_arrary_init_u8(&handlers->b_origion_data,
                          handlers->b_origion_data_buf,
                          ARRARY_SIZE(handlers->b_origion_data_buf));

  infinite_arrary_init_u8(&handlers->a_num_sum_average,
                          handlers->a_num_sum_average_buf,
                          ARRARY_SIZE(handlers->a_num_sum_average_buf));
  infinite_arrary_init_u8(&handlers->b_num_sum_average,
                          handlers->b_num_sum_average_buf,
                          ARRARY_SIZE(handlers->b_num_sum_average_buf));

  handlers->a_ir_receive_state = kOnReceiveingNothingState;
  handlers->b_ir_receive_state = kOnReceiveingNothingState;
  handlers->on_judge_result = on_judge_result;
}

void human_num_analys_init_for_debug(
    human_detecter_handler_t *handlers,
    on_new_num_sum_average_t on_new_num_sum_average,
    on_new_signal_t on_new_signal, on_judge_result_t on_judge_result) {
  human_num_analys_init(handlers, on_judge_result);
  handlers->on_new_num_sum_average = on_new_num_sum_average;
  handlers->on_new_signal = on_new_signal;
  handlers->debug_mode = true;
}

/**
 * @brief 压入一个新的红外原始数据
 *
 * @param doamin
 * @param origion_data (0--100)
 */
void human_num_analys_push_origion_data(human_detecter_handler_t *handlers,
                                        ir_receive_signal_time_domain_e doamin,
                                        uint8_t origion_data) {
  if (doamin == kADomain) {
    loop_queue_push_one_u8(&handlers->a_origion_data_queue, origion_data);
  } else {
    loop_queue_push_one_u8(&handlers->b_origion_data_queue, origion_data);
  }
}
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
 */
void human_num_analys_reset(human_detecter_handler_t *handlers) {
  if (!handlers->debug_mode)
    human_num_analys_init(handlers, handlers->on_judge_result);
  else
    human_num_analys_init_for_debug(handlers, handlers->on_new_num_sum_average,
                                    handlers->on_new_signal,
                                    handlers->on_judge_result);
}