
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
#include "spdlog/logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
extern "C" {
#include "human_num_analys_algo.h"
}
#include "../test_utils/nlohmann/json.hpp"

// input data.......
// {
//   "A" : [],
//   "B" : []
// }

using namespace std;
using namespace spdlog;
/*************************static funcion***********************************/
static inline string human_moving_direction_t_to_str(
    const human_moving_direction_t &direction) {
  string ret;
  if (direction == kInDirection) {
    ret = "kInDirection";
  } else if (direction == kOutDirection) {
    ret = "kOutDirection";
  } else {
    ret = "unkown";
  }
  return ret;
}
// shared_ptr<uint8_t> a;
static string ir_signal_t_to_str(const ir_signal_t &ir_signal) {
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
  return ret;
}

/*************************gloable***********************************/
typedef shared_ptr<logger> spdlogger_t;
size_t s_i = 0;

shared_ptr<logger> s_judgeResultLogger;
shared_ptr<logger> s_numSumAverageLogger;
shared_ptr<logger> s_signalLogger;
shared_ptr<logger> s_console;

void confingLogger() {
  s_console = spdlog::stdout_color_mt("console");
  auto sink = s_console->sinks()[0];
  s_judgeResultLogger =
      basic_logger_mt("judgeResultLogger", "judgeResultLogger.log",true);
  s_numSumAverageLogger =
      basic_logger_mt("numSumAverageLogger", "numSumAverageLogger.log", true);
  s_signalLogger = basic_logger_mt("signalLogger", "signalLogger.log", true);

  s_judgeResultLogger->sinks().push_back(sink);
  s_numSumAverageLogger->sinks().push_back(sink);
  s_signalLogger->sinks().push_back(sink);
}

static void on_judge_result(human_detecter_handler_t *handler,
                            human_moving_direction_t direction, int num) {
  s_judgeResultLogger->info("judge_result {},{}",
                            human_moving_direction_t_to_str(direction), num);
}
static void on_new_num_sum_average(human_detecter_handler_t *handler,
                                   uint8_t adata, uint8_t bdata) {
  s_numSumAverageLogger->info("{} num_sum_agerage {},{}", s_i, adata, bdata);
}
static void on_new_signal(human_detecter_handler_t *handler,
                          ir_signal_t new_signal) {
  s_signalLogger->info("signal {}", ir_signal_t_to_str(new_signal));
}

vector<uint8_t> a_data;
vector<uint8_t> b_data;

int main(int argc, char *const argv[]) {
  human_detecter_handler_t handler;
  confingLogger();

  human_num_analys_init_for_debug(&handler, on_new_num_sum_average,
                                  on_new_signal, on_judge_result);
  if (argc != 2) {
    s_console->error("./test_human_num_analys_algo data.json");
    return -1;
  }

  std::ifstream infile(argv[1], std::ios::binary | std::ios::in);
  nlohmann::json j = nlohmann::json::parse(infile);
  for (auto &var : j["A"]) a_data.push_back(var);
  for (auto &var : j["B"]) b_data.push_back(var);

  if (a_data.size() != b_data.size()) {
    s_console->error("a_data.size() not equal b_data.size()");
    return -1;
  }
  s_console->info("total {}", a_data.size());

  for (s_i = 0; s_i < a_data.size(); s_i++) {
    /* code */
    human_num_analys_push_origion_data(&handler, kADomain,
                                       a_data[s_i] * 100 / 23);
    human_num_analys_push_origion_data(&handler, kBDomain,
                                       b_data[s_i] * 100 / 23);
    human_num_analys_process(&handler);
  }

  s_console->info("end.........");
  return 0;
}
