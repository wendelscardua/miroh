#pragma once

#include "common.hpp"
#define RAND_UP_TO(n) ((u8)(((u16)rand8() * (u16)(n)) >> 8))
#define RAND_UP_TO_POW2(n) (rand8() & ((1 << n) - 1))

static constexpr u8 INT_TO_TEXT_BANK = 0;
// [bank 0]
void u8_to_text(u8 score_text[], u8 value);
// [bank 0]
void int_to_text(u8 score_text[], u16 value);