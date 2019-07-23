cmake_minimum_required(VERSION 3.13)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "./lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# PLATFORM can be pc 3329 警告配置
set(
  COMMON_FLAGS
  " ${COMMON_FLAGS} -Werror=return-type -Werror=parentheses -Wfatal-errors  -Wall  -Wno-deprecated-declarations -Wno-unused-variable "
  )
# 栈信息配置
set(
  COMMON_FLAGS
  "${COMMON_FLAGS} -fexceptions  -ffunction-sections  -rdynamic -funwind-tables -fasynchronous-unwind-tables "
  )
# 调试和编译等级配置
set(COMMON_FLAGS "${COMMON_FLAGS} -O1 -g3")
# 如果使用c++17,在pc下编译会rest_rpc会出错，原因是c++17对应的gcc版本是需要大于6.0的，而pc上不是
#
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -std=c++14 -pthread -Wno-reorder")
set(
  CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -DWEBRTC_APM_DEBUG_DUMP=0 -DWEBRTC_POSIX -DWEBRTC_LINUX -DASIO_STANDALONE"
  )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ")

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

# PC
project(HappySwitchSoundBoxV2)

####  ####
add_executable(human_detect_data_analys
human_detect_data_analys.cc
)
