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
void start_mesen_watch(u8 label) { POKE(0x4020, label); }
void stop_mesen_watch() { POKE(0x4021, 0); }
