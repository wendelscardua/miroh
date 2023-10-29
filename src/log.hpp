#pragma once

#include "common.hpp"

void put_hexdigit(u8 h);
void put_hex(u8 h);
void put_hex(u16 h);

void start_mesen_watch(u8 label);
void stop_mesen_watch();

#ifdef NDEBUG
#define START_MESEN_WATCH(label)                                               \
  do {                                                                         \
  } while (0)
#define STOP_MESEN_WATCH(label)                                                \
  do {                                                                         \
  } while (0)
#else
#define START_MESEN_WATCH(label) start_mesen_watch(label)
#define STOP_MESEN_WATCH stop_mesen_watch()
#endif