
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
extern "C" {
#include "human_num_analys_algo.h"
}
#include "../test_utils/nlohmann/json.hpp"

using namespace std;
using namespace spdlog;
/*************************static funcion***********************************/
template <typename OStream>
static inline OStream &operator<<(OStream &os,
                                  const human_moving_direction_t &direction);
template <typename OStream>
static inline OStream &operator<<(OStream &os, const ir_signal_t &ir_signal);

/*************************gloable***********************************/
typedef shared_ptr<logger> spdlogger_t;
human_detecter_handler_t handler;

// uint8_t
spdlogger_t s_judgeResultLogger;
spdlogger_t s_numSumAverageLogger;
spdlogger_t s_signalLogger;
spdlogger_t s_console;

void confingLogger() {
  s_console = spdlog::stdout_color_mt("console");
  auto sink = s_console->sinks()[0];
  s_judgeResultLogger =
      basic_logger_mt("judgeResultLogger", "judgeResultLogger.log");
  s_numSumAverageLogger =
      basic_logger_mt("numSumAverageLogger", "numSumAverageLogger.log");
  s_signalLogger = basic_logger_mt("signalLogger", "signalLogger.log");
  s_judgeResultLogger->sinks().push_back(sink);
  s_numSumAverageLogger->sinks().push_back(sink);
  s_signalLogger->sinks().push_back(sink);
}

static void on_judge_result(human_detecter_handler_t *handler,
                            human_moving_direction_t direction, int num) {
  // s_judgeResultLogger->info("judge_result {},{}", direction, num);
}
static void on_new_num_sum_average(human_detecter_handler_t *handler,
                                   uint8_t adata, uint8_t bdata) {
  s_numSumAverageLogger->info("num_sum_agerage {},{}", adata, bdata);
}
static void on_new_signal(human_detecter_handler_t *handler,
                          ir_signal_t new_signal) {
  // s_numSumAverageLogger->info("signal {}", new_signal);
}

vector<uint8_t> orgion_data;

int main(int argc, char *const argv[]) {
  confingLogger();

  human_num_analys_init_for_debug(&handler, on_new_num_sum_average,
                                  on_new_signal, on_judge_result);
  if (argc != 2) {
    s_console->error("./test_human_num_analys_algo data.json");
  }

  std::ifstream infile(argv[1], std::ios::binary | std::ios::in);
  nlohmann::json j = nlohmann::json::parse(infile);
  for (auto &var : j) orgion_data.push_back(var);

  bool a_domain = true;
  for (auto &var : orgion_data) {
    if (a_domain) {
      human_num_analys_push_origion_data(&handler, kADomain, var);
    } else {
      human_num_analys_push_origion_data(&handler, kBDomain, var);
    }
    human_num_analys_process(&handler);
    a_domain = !a_domain;
  }
  s_console->info("end.........");
  return 0;
}

/*
   _   _ _   _ _
  | | | | |_(_) |___
  | |_| |  _| | (_-<
   \___/ \__|_|_/__/

*/
template <typename OStream>
static inline OStream &operator<<(OStream &os,
                                  human_moving_direction_t direction) {
  string ret;
  if (direction == kInDirection) {
    ret = "kInDirection";
  } else if (direction == kOutDirection) {
    ret = "kOutDirection";
  } else {
    ret = "unkown";
  }
  return os << ret;
}

template <typename OStream>
static inline OStream &operator<<(OStream &os, ir_signal_t ir_signal) {
  string ret;
  if (ir_signal.type == kAStartType) {
    ret = fmt::format("{}_{}", ir_signal.offset, "kAStartType");
  } else if (ir_signal.type == kAStopType) {
    ret = fmt::format("{}_{}", ir_signal.offset, "kAStopType");
  } else if (ir_signal.type == kBStartType) {
    ret = fmt::format("{}_{}", ir_signal.offset, "kBStartType");
  } else if (ir_signal.type == kBStopType) {
    ret = fmt::format("{}_{}", ir_signal.offset, "kBStopType");
  } else {
    ret = fmt::format("{}_{}", ir_signal.offset, "unkownType");
  }
  return os << ret;
}