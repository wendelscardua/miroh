#pragma once

#include "common.hpp"

struct AnimCell {
  const u8 index;
  const unsigned char *metasprite;
  const u8 duration;
};

class Animation {
public:
  static constexpr u8 BANK = 5; // same as Unicorn

  static bool paused;
  u8 current_frame;
  bool finished;

  Animation(const AnimCell (*cells)[]);

  void reset();

  void update(char x, int y);

  u8 current_cell_index() const;

private:
  const AnimCell *current_cell;
  const AnimCell (*cells)[];
};