#include "log.hpp"
#include "common.hpp"
#include <cstdio>
#include <peekpoke.h>

void put_hexdigit(u8 h) {
  if (h < 10)
    putchar('0' + h);
  else
    putchar('a' + h - 10);
}
void put_hex(u8 h) {
  put_hexdigit(h >> 4);
  put_hexdigit(h & 0x0f);
}
void put_hex(u16 h) {
  put_hex((u8)(h >> 8));
  put_hex((u8)h);
}
void start_mesen_watch(const char *addr) {
  u16 address = (u16)(uintptr_t)addr;
  POKE(0x4020, (address >> 8) & 0xFF);
  POKE(0x4020, address & 0xFF);
}
void stop_mesen_watch(const char *addr) {
  u16 address = (u16)(uintptr_t)addr;
  POKE(0x4021, (address >> 8) & 0xFF);
  POKE(0x4021, address & 0xFF);
}
void break_mesen(u8 label) { POKE(0x4019, label); }