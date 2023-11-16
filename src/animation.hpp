#pragma once

#include "banked-asset-helpers.hpp"
#include "common.hpp"

struct AnimCell {
  const unsigned char *metasprite;
  u8 duration;
};

class Animation {
public:
  const AnimCell (&cells)[];
  u8 current_frame;
  u8 current_cell;
  u8 length;
  bool finished;

  Animation(const AnimCell (&cells)[], u8 length)
      : cells(cells), current_frame(0), current_cell(0), length(length),
        finished(false) {}

  void reset() {
    current_frame = 0;
    current_cell = 0;
    finished = false;
  }

  void update(char x, int y) {
    banked_oam_meta_spr(x, y, cells[current_cell].metasprite);
    current_frame++;
    if (current_frame >= cells[current_cell].duration) {
      current_frame = 0;
      current_cell++;
      if (current_cell == length) {
        current_cell = 0;
        finished = true;
      }
    }
  }
};