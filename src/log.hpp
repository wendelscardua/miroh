#pragma once

#include "common.hpp"
#include <peekpoke.h>

extern "C" void __putchar(char c);
void put_hexdigit(u8 h);
void put_hex(u8 h);
void put_hex(u16 h);
