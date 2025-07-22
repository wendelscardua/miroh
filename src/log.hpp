#pragma once

#include "common.hpp"

void put_hexdigit(u8 h);
void put_hex(u8 h);
void put_hex(u16 h);

void start_mesen_watch(const char *addr);
void stop_mesen_watch(const char *addr);
void break_mesen(u8 label);

#ifdef NDEBUG
#define START_MESEN_WATCH(addr)                                                \
  do {                                                                         \
  } while (0)
#define STOP_MESEN_WATCH(addr)                                                 \
  do {                                                                         \
  } while (0)
#define BREAK_MESEN(label)                                                     \
  do {                                                                         \
  } while (0)
#else
#define START_MESEN_WATCH(addr) start_mesen_watch(addr)
#define STOP_MESEN_WATCH(addr) stop_mesen_watch(addr)
#define BREAK_MESEN(label) break_mesen(label)
#endif