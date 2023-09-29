#include "log.hpp"

void __putchar(char c) { POKE(0x4018, c); }
