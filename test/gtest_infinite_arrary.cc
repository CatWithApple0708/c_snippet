#include "gtest/gtest.h"
#include "glog/logging.h"
extern "C" {
#include "infinite_arrary.h"
}

#ifndef ARRARY_SIZE
#define ARRARY_SIZE(arrary) sizeof(arrary) / sizeof(arrary[0])
#endif

TEST(TestLoopBuffer, test_u8) {
  {
    infinite_arrary_u8 loopbuff;
    uint8_t buffer[20] = {0};
    infinite_arrary_init_u8(&loopbuff, buffer, ARRARY_SIZE(buffer));

    EXPECT_TRUE(infinite_arrary_get_useful_start_offset_u8(&loopbuff) < 0);
    EXPECT_TRUE(infinite_arrary_get_useful_end_offset_u8(&loopbuff) < 0);

    for (unsigned i = 0; i < 20; ++i) {
      uint8_t value = i;
      infinite_arrary_push_u8(&loopbuff, value);
    }

    for (unsigned i = 0; i < 20; ++i) {
      EXPECT_EQ(infinite_arrary_get_u8(&loopbuff, i), (int32_t)i);
    }
  }

  {
    infinite_arrary_u8 loopbuff;
    uint8_t buffer[20] = {0};
    infinite_arrary_init_u8(&loopbuff, buffer, ARRARY_SIZE(buffer));

    EXPECT_TRUE(infinite_arrary_get_useful_start_offset_u8(&loopbuff) < 0);
    EXPECT_TRUE(infinite_arrary_get_useful_end_offset_u8(&loopbuff) < 0);

    for (unsigned i = 0; i < 40; ++i) {
      uint8_t value = i;
      infinite_arrary_push_u8(&loopbuff, value);

      if (i < 20) {
        EXPECT_EQ(infinite_arrary_get_useful_start_offset_u8(&loopbuff),
                  (int32_t)0);
        EXPECT_EQ(infinite_arrary_get_useful_end_offset_u8(&loopbuff),
                  (int32_t)i);
      } else {
        EXPECT_EQ(infinite_arrary_get_useful_start_offset_u8(&loopbuff),
                  (int32_t)i - 19);
        EXPECT_EQ(infinite_arrary_get_useful_end_offset_u8(&loopbuff),
                  (int32_t)i);
      }
    }

    for (unsigned i = 0; i < 40; ++i) {
      if (i < 20) {
        EXPECT_EQ(infinite_arrary_get_u8(&loopbuff, i), (uint8_t)0);
      } else {
        EXPECT_EQ(infinite_arrary_get_u8(&loopbuff, i), (uint8_t)i);
      }
    }
  }
}

TEST(TestLoopBuffer, test_u32) {
  {
    infinite_arrary_u32 loopbuff;
    uint32_t buffer[20] = {0};
    infinite_arrary_init_u32(&loopbuff, buffer, ARRARY_SIZE(buffer));

    EXPECT_TRUE(infinite_arrary_get_useful_start_offset_u32(&loopbuff) <
                0);
    EXPECT_TRUE(infinite_arrary_get_useful_end_offset_u32(&loopbuff) < 0);

    for (unsigned i = 0; i < 20; ++i) {
      uint32_t value = i;
      infinite_arrary_push_u32(&loopbuff, value);
    }

    for (unsigned i = 0; i < 20; ++i) {
      EXPECT_EQ(infinite_arrary_get_u32(&loopbuff, i), (uint32_t)i);
    }
  }

  {
    infinite_arrary_u32 loopbuff;
    uint32_t buffer[20] = {0};
    infinite_arrary_init_u32(&loopbuff, buffer, ARRARY_SIZE(buffer));

    EXPECT_TRUE(infinite_arrary_get_useful_start_offset_u32(&loopbuff) <
                0);
    EXPECT_TRUE(infinite_arrary_get_useful_end_offset_u32(&loopbuff) < 0);

    for (unsigned i = 0; i < 40; ++i) {
      uint32_t value = i;
      infinite_arrary_push_u32(&loopbuff, value);

      if (i < 20) {
        EXPECT_EQ(infinite_arrary_get_useful_start_offset_u32(&loopbuff),
                  (int32_t)0);
        EXPECT_EQ(infinite_arrary_get_useful_end_offset_u32(&loopbuff),
                  (int32_t)i);
      } else {
        EXPECT_EQ(infinite_arrary_get_useful_start_offset_u32(&loopbuff),
                  (int32_t)i - 19);
        EXPECT_EQ(infinite_arrary_get_useful_end_offset_u32(&loopbuff),
                  (int32_t)i);
      }
    }

    for (unsigned i = 0; i < 40; ++i) {
      if (i < 20) {
        EXPECT_EQ(infinite_arrary_get_u32(&loopbuff, i), (uint32_t)0);
      } else {
        EXPECT_EQ(infinite_arrary_get_u32(&loopbuff, i), (uint32_t)i);
      }
    }
  }
}