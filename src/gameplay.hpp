#pragma once

#include "board.hpp"
#include "input-mode.hpp"
#include "polyomino.hpp"
#include "player.hpp"

class Gameplay {
public:
  Board board;
  Player player;
  Polyomino polyomino;
  InputMode input_mode;

  Gameplay();
  ~Gameplay();
  void loop();
private:
  void render();
};
