#pragma once

#include "common.hpp"

struct AnimCell {
  const unsigned char *metasprite;
  u8 duration;
};

class Animation {
public:
  static constexpr u8 BANK = 5; // same as Unicorn

  const AnimCell (*cells)[];
  const AnimCell *current_cell;
  static bool paused;
  u8 current_frame;
  u8 current_cell_index;
  bool finished;

  Animation(const AnimCell (*cells)[]);

  void reset();

  void update(char x, int y);
};