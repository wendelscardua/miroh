#include "utils.hpp"
#include "assets.hpp"
#include <neslib.h>

__attribute__((section(".prg_rom_fixed.text"))) void u8_to_text(u8 score_text[],
                                                                u8 value) {
  score_text[0] = 0;
  if (value >= 80) {
    score_text[0] |= 8;
    value -= 80;
  }
  if (value >= 40) {
    score_text[0] |= 4;
    value -= 40;
  }
  if (value >= 20) {
    score_text[0] |= 2;
    value -= 20;
  }
  if (value >= 10) {
    score_text[0] |= 1;
    value -= 10;
  }
  if (score_text[0] == 0) {
    score_text[0] = DARK_ZERO_TILE;
  } else {
    score_text[0] += DIGITS_BASE_TILE;
  }
  score_text[1] = DIGITS_BASE_TILE + (u8)value;
}

__attribute__((always_inline)) void int_to_text(u8 score_text[], u16 value) {
  score_text[0] = 0;
  if (value >= 8000) {
    score_text[0] |= 8;
    value -= 8000;
  }
  if (value >= 4000) {
    score_text[0] |= 4;
    value -= 4000;
  }
  if (value >= 2000) {
    score_text[0] |= 2;
    value -= 2000;
  }
  if (value >= 1000) {
    score_text[0] |= 1;
    value -= 1000;
  }
  score_text[0] += DIGITS_BASE_TILE;

  score_text[1] = 0;
  if (value >= 800) {
    score_text[1] |= 8;
    value -= 800;
  }
  if (value >= 400) {
    score_text[1] |= 4;
    value -= 400;
  }
  if (value >= 200) {
    score_text[1] |= 2;
    value -= 200;
  }
  if (value >= 100) {
    score_text[1] |= 1;
    value -= 100;
  }
  score_text[1] += DIGITS_BASE_TILE;

  score_text[2] = 0;
  if (value >= 80) {
    score_text[2] |= 8;
    value -= 80;
  }
  if (value >= 40) {
    score_text[2] |= 4;
    value -= 40;
  }
  if (value >= 20) {
    score_text[2] |= 2;
    value -= 20;
  }
  if (value >= 10) {
    score_text[2] |= 1;
    value -= 10;
  }
  score_text[2] += DIGITS_BASE_TILE;

  score_text[3] = DIGITS_BASE_TILE + (u8)value;

  // leading zeroes are darker
  for (u8 i = 0; i < 3; i++) {
    if (score_text[i] > DIGITS_BASE_TILE) {
      break;
    }
    score_text[i] = DARK_ZERO_TILE;
  }
}

__attribute__((section(".prg_rom_fixed.text"))) u8 rand_up_to(u8 n) {
  u8 result = rand8() & 31;
  while (result >= n) {
    result -= n;
  }
  return result;
}