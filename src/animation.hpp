#pragma once

#include "banked-asset-helpers.hpp"
#include "common.hpp"
#include <cstdint>
#include <soa.h>

struct AnimCell {
  const unsigned char *metasprite;
  u8 duration;
};

template <uint8_t N> class Animation {
public:
  const soa::Array<AnimCell, N> &cells;
  u8 current_frame;
  u8 current_cell;
  bool finished;

  Animation(const soa::Array<AnimCell, N> &cells)
      : cells(cells), current_frame(0), current_cell(0), finished(false) {}

  void reset() {
    current_frame = 0;
    current_cell = 0;
    finished = false;
  }

  void update(char x, int y) {
    banked_oam_meta_spr(x, y, cells[current_cell]->metasprite);
    current_frame++;
    if (current_frame >= cells[current_cell]->duration) {
      current_frame = 0;
      current_cell++;
      if (current_cell == N) {
        current_cell = 0;
        finished = true;
      }
    }
  }
};