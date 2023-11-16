#pragma once

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

  Animation(const AnimCell (&cells)[], u8 length);

  void reset();

  void update(char x, int y);
};