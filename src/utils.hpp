#pragma once

#include "common.hpp"
// #define RAND_UP_TO(n) ((u8)(((u16)rand8() * (u16)(n)) >> 8))
// #define RAND_UP_TO(n) (rand8() % (n))
#define RAND_UP_TO(n) (rand_up_to(n))
#define RAND_UP_TO_POW2(n) (rand8() & ((1 << n) - 1))

void u8_to_text(u8 score_text[], u8 value);
void int_to_text(u8 score_text[], u16 value);

u8 rand_up_to(u8 n);