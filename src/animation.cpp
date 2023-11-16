#include "animation.hpp"
#include "banked-asset-helpers.hpp"

Animation::Animation(const AnimCell (&cells)[], u8 length)
    : cells(cells), current_frame(0), current_cell(0), length(length),
      finished(false) {}

void Animation::reset() {
  current_frame = 0;
  current_cell = 0;
  finished = false;
}

void Animation::update(char x, int y) {
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
