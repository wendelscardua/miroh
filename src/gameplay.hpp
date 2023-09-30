#pragma once

#include "board.hpp"
#include "player.hpp"

class Gameplay {
public:
  Board board;
  Player player;
  Gameplay();
  ~Gameplay();
  void loop();
private:
  void render();
};
