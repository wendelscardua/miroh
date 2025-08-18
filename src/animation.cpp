#include "animation.hpp"
#include "banked-asset-helpers.hpp"
#include "metasprites.hpp"

// same as Unicorn
#pragma clang section text = ".prg_rom_5.text.animation"
#pragma clang section rodata = ".prg_rom_5.rodata.animation"

bool Animation::paused = false;

Animation::Animation(const AnimCell (*cells)[])
    : finished(false), current_cell(&(*cells)[0]), cells(cells) {}

void Animation::reset() {
  current_cell = &(*cells)[0];
  finished = false;
}

void Animation::update(char x, int y) {
  banked_oam_meta_spr(METASPRITES_BANK, x, y, current_cell->metasprite);
  if (paused) {
    return;
  }
  current_cell++;
  if (current_cell->flags == 0) {
    current_cell = &(*cells)[0];
    finished = true;
  }
}

u8 Animation::current_cell_flags() const { return current_cell->flags; }
bool Animation::current_cell_flags(u8 flags) const {
  return flags & current_cell->flags;
}