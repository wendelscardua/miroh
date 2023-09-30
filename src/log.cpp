#include "log.hpp"
#include "common.hpp"

void __putchar(char c) { POKE(0x4018, c); }
void put_hexdigit(u8 h) {
  if (h < 10) __putchar('0' + h);
  else __putchar('a' + h - 10);
}
void put_hex(u8 h) {
  put_hexdigit(h >> 4);
  put_hexdigit(h & 0x0f);
}
void put_hex(u16 h) {
  put_hex((u8)(h >> 8));
  put_hex((u8)h);
}
