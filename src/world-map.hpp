#pragma once

#include "common.hpp"
#include <neslib.h>

class WorldMap {
public:
  static constexpr u8 BANK = 2;

  WorldMap();
  ~WorldMap();
  void loop();
};
