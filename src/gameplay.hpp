#pragma once

#include "board.hpp"

class Gameplay {
public:
  Board board;
  Gameplay();
  ~Gameplay();
  void loop();
};
