#include "animation.hpp"
#include "banked-asset-helpers.hpp"

// same as Unicorn
#pragma clang section text = ".prg_rom_5.text.animation"
#pragma clang section rodata = ".prg_rom_5.rodata.animation"

bool Animation::paused = false;

Animation::Animation(const AnimCell (*cells)[], u8 length)
    : cells(cells), current_cell(&(*cells)[0]), current_frame(0),
      current_cell_index(0), length(length), finished(false) {}

void Animation::reset() {
  current_frame = 0;
  current_cell_index = 0;
  current_cell = &(*cells)[0];
  finished = false;
}

void Animation::update(char x, int y) {
  banked_oam_meta_spr(x, y, current_cell->metasprite);
  if (paused)
    goto exit;
  current_frame++;
  if (current_frame >= current_cell->duration) {
    current_frame = 0;
    current_cell_index++;
    if (current_cell_index == length) {
      current_cell_index = 0;
      current_cell = &(*cells)[0];
      finished = true;
    } else {
      current_cell++;
    }
  }
exit:
}
