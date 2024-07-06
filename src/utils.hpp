#pragma once

#include "common.hpp"
#define RAND_UP_TO(n) ((u8)(((u16)rand8() * (u16)(n)) >> 8))
#define RAND_UP_TO_POW2(n) (rand8() & ((1 << n) - 1))

// [bank 0]
__attribute__((section(".prg_rom_fixed.text"))) void u8_to_text(u8 score_text[],
                                                                u8 value);
// [bank 0]
__attribute__((section(".prg_rom_fixed.text"))) void
int_to_text(u8 score_text[], u16 value);