#pragma once

#ifndef BUILD_ASSERT
#define BUILD_ASSERT(cond)               \
  do {                                   \
    (void)sizeof(char[1 - 2 * !(cond)]); \
  } while (0)
#endif

#ifndef ARRARY_SIZE
#define ARRARY_SIZE(arrary) sizeof(arrary) / sizeof(arrary[0])
#endif