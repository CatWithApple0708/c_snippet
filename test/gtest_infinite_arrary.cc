#include "gtest/gtest.h"
#include "glog/logging.h"
extern "C" {
#include "infinite_arrary.h"
INFINITE_ARRARY_ENABLE_SUPPORT(uint8_t);
INFINITE_ARRARY_ENABLE_SUPPORT(uint16_t);
INFINITE_ARRARY_ENABLE_SUPPORT(uint32_t);
}

#ifndef ARRARY_SIZE
#define ARRARY_SIZE(arrary) sizeof(arrary) / sizeof(arrary[0])
#endif

TEST(TestLoopBuffer, test_uint8_t) {
  {
    infinite_arrary_t loopbuff;
    uint8_t buffer[20] = {0};
    infinite_arrary_init_uint8_t(&loopbuff, buffer, ARRARY_SIZE(buffer));

    EXPECT_TRUE(infinite_arrary_get_useful_start_offset(&loopbuff) < 0);
    EXPECT_TRUE(infinite_arrary_get_useful_end_offset(&loopbuff) < 0);

    for (unsigned i = 0; i < 20; ++i) {
      uint8_t value = i;
      infinite_arrary_push_uint8_t(&loopbuff, value);
    }

    for (unsigned i = 0; i < 20; ++i) {
      EXPECT_EQ(infinite_arrary_get_uint8_t(&loopbuff, i), (int32_t)i);
    }
  }

  {
    infinite_arrary_t loopbuff;
    uint8_t buffer[20] = {0};
    infinite_arrary_init_uint8_t(&loopbuff, buffer, ARRARY_SIZE(buffer));

    EXPECT_TRUE(infinite_arrary_get_useful_start_offset(&loopbuff) < 0);
    EXPECT_TRUE(infinite_arrary_get_useful_end_offset(&loopbuff) < 0);

    for (unsigned i = 0; i < 40; ++i) {
      uint8_t value = i;
      infinite_arrary_push_uint8_t(&loopbuff, value);

      if (i < 20) {
        EXPECT_EQ(infinite_arrary_get_useful_start_offset(&loopbuff), (int32_t)0);
        EXPECT_EQ(infinite_arrary_get_useful_end_offset(&loopbuff), (int32_t)i);
      } else {
        EXPECT_EQ(infinite_arrary_get_useful_start_offset(&loopbuff),
                  (int32_t)i - 19);
        EXPECT_EQ(infinite_arrary_get_useful_end_offset(&loopbuff), (int32_t)i);
      }
    }

    for (unsigned i = 0; i < 40; ++i) {
      if (i < 20) {
        EXPECT_EQ(infinite_arrary_get_uint8_t(&loopbuff, i), (uint8_t)0);
      } else {
        EXPECT_EQ(infinite_arrary_get_uint8_t(&loopbuff, i), (uint8_t)i);
      }
    }
  }
}

TEST(TestLoopBuffer, test_uint32_t) {
  {
    infinite_arrary_t loopbuff;
    uint32_t buffer[20] = {0};
    infinite_arrary_init_uint32_t(&loopbuff, buffer, ARRARY_SIZE(buffer));

    EXPECT_TRUE(infinite_arrary_get_useful_start_offset(&loopbuff) < 0);
    EXPECT_TRUE(infinite_arrary_get_useful_end_offset(&loopbuff) < 0);

    for (unsigned i = 0; i < 20; ++i) {
      uint32_t value = i;
      infinite_arrary_push_uint32_t(&loopbuff, value);
    }

    for (unsigned i = 0; i < 20; ++i) {
      EXPECT_EQ(infinite_arrary_get_uint32_t(&loopbuff, i), (uint32_t)i);
    }
  }

  {
    infinite_arrary_t loopbuff;
    uint32_t buffer[20] = {0};
    infinite_arrary_init_uint32_t(&loopbuff, buffer, ARRARY_SIZE(buffer));

    EXPECT_TRUE(infinite_arrary_get_useful_start_offset(&loopbuff) < 0);
    EXPECT_TRUE(infinite_arrary_get_useful_end_offset(&loopbuff) < 0);

    for (unsigned i = 0; i < 40; ++i) {
      uint32_t value = i;
      infinite_arrary_push_uint32_t(&loopbuff, value);

      if (i < 20) {
        EXPECT_EQ(infinite_arrary_get_useful_start_offset(&loopbuff), (int32_t)0);
        EXPECT_EQ(infinite_arrary_get_useful_end_offset(&loopbuff), (int32_t)i);
      } else {
        EXPECT_EQ(infinite_arrary_get_useful_start_offset(&loopbuff),
                  (int32_t)i - 19);
        EXPECT_EQ(infinite_arrary_get_useful_end_offset(&loopbuff), (int32_t)i);
      }
    }

    for (unsigned i = 0; i < 40; ++i) {
      if (i < 20) {
        EXPECT_EQ(infinite_arrary_get_uint32_t(&loopbuff, i), (uint32_t)0);
      } else {
        EXPECT_EQ(infinite_arrary_get_uint32_t(&loopbuff, i), (uint32_t)i);
      }
    }
  }
}