#pragma once

#include "board.hpp"
#include "common.hpp"
#include <neslib.h>

class WorldMap {
public:
  static constexpr u8 BANK = 2;

  Board &board;

  WorldMap(Board &board);
  ~WorldMap();
  void render_sprites();
  void loop();
};
