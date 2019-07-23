#include "gtest/gtest.h"
#include "glog/logging.h"
extern "C" {
#include "loop_queue.h"
LOOP_QUEUE_ENABLE_SUPPORT(uint8_t);
LOOP_QUEUE_ENABLE_SUPPORT(uint16_t);
LOOP_QUEUE_ENABLE_SUPPORT(uint32_t);
}

#ifndef ARRARY_SIZE
#define ARRARY_SIZE(arrary) sizeof(arrary) / sizeof(arrary[0])
#endif

TEST(TestLoopQueue, test_uint8_t) {
  {
    loop_queue_t loop_queue;
    uint8_t buf[16] = {0};

    loop_queue_init_uint8_t(&loop_queue, buf, ARRARY_SIZE(buf));
    ASSERT_TRUE(loop_queue_is_empty(&loop_queue));
    ASSERT_TRUE(!loop_queue_is_full(&loop_queue));
    ASSERT_TRUE(loop_queue_get_size(&loop_queue) == 0);

    ASSERT_TRUE(loop_queue_push_one_uint8_t(&loop_queue, 1));
    ASSERT_TRUE(!loop_queue_is_empty(&loop_queue));
    ASSERT_TRUE(loop_queue_get_size(&loop_queue) == 1);
    uint8_t value = 0;
    ASSERT_TRUE(loop_queue_pop_one_uint8_t(&loop_queue, &value));
    ASSERT_TRUE(value == 1);
    ASSERT_TRUE(loop_queue_is_empty(&loop_queue));
    ASSERT_TRUE(loop_queue_get_size(&loop_queue) == 0);
  }

  {
    loop_queue_t loop_queue;
    uint8_t buf[16] = {0};

    loop_queue_init_uint8_t(&loop_queue, buf, ARRARY_SIZE(buf));
    
    for(unsigned j = 0; j < 2; ++j) {
      for (unsigned i = 0; i < 15; ++i) {
        ASSERT_TRUE(loop_queue_push_one_uint8_t(&loop_queue, i));
      }
      ASSERT_TRUE(loop_queue_is_full(&loop_queue));
      ASSERT_TRUE(!loop_queue_push_one_uint8_t(&loop_queue, 20));
      ASSERT_TRUE(!loop_queue_push_one_uint8_t(&loop_queue, 20));
      ASSERT_TRUE(!loop_queue_push_one_uint8_t(&loop_queue, 20));

      for (unsigned i = 0; i < 15; ++i) {
        uint8_t value = 0;
        ASSERT_TRUE(loop_queue_pop_one_uint8_t(&loop_queue, &value));
        ASSERT_TRUE(value == i);
      }
      ASSERT_TRUE(loop_queue_is_empty(&loop_queue));
    }
  }
}

TEST(TestLoopQueue, test_uint32_t) {
  {
    loop_queue_t loop_queue;
    uint32_t buf[16] = {0};

    loop_queue_init_uint32_t(&loop_queue, buf, ARRARY_SIZE(buf));
    ASSERT_TRUE(loop_queue_is_empty(&loop_queue));
    ASSERT_TRUE(!loop_queue_is_full(&loop_queue));
    ASSERT_TRUE(loop_queue_get_size(&loop_queue) == 0);

    ASSERT_TRUE(loop_queue_push_one_uint32_t(&loop_queue, 1));
    ASSERT_TRUE(!loop_queue_is_empty(&loop_queue));
    ASSERT_TRUE(loop_queue_get_size(&loop_queue) == 1);
    uint32_t value = 0;
    ASSERT_TRUE(loop_queue_pop_one_uint32_t(&loop_queue, &value));
    ASSERT_TRUE(value == 1);
    ASSERT_TRUE(loop_queue_is_empty(&loop_queue));
    ASSERT_TRUE(loop_queue_get_size(&loop_queue) == 0);
  }

  {
    loop_queue_t loop_queue;
    uint32_t buf[16] = {0};

    loop_queue_init_uint32_t(&loop_queue, buf, ARRARY_SIZE(buf));

    for (unsigned j = 0; j < 2; ++j) {
      for (unsigned i = 0; i < 15; ++i) {
        ASSERT_TRUE(loop_queue_push_one_uint32_t(&loop_queue, i));
      }
      ASSERT_TRUE(loop_queue_is_full(&loop_queue));
      ASSERT_TRUE(!loop_queue_push_one_uint32_t(&loop_queue, 20));
      ASSERT_TRUE(!loop_queue_push_one_uint32_t(&loop_queue, 20));
      ASSERT_TRUE(!loop_queue_push_one_uint32_t(&loop_queue, 20));

      for (unsigned i = 0; i < 15; ++i) {
        uint32_t value = 0;
        ASSERT_TRUE(loop_queue_pop_one_uint32_t(&loop_queue, &value));
        ASSERT_TRUE(value == i);
      }
      ASSERT_TRUE(loop_queue_is_empty(&loop_queue));
    }
  }
}