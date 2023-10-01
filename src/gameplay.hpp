#pragma once

#include "board.hpp"
#include "fruits.hpp"
#include "input-mode.hpp"
#include "polyomino.hpp"
#include "player.hpp"

#define INITIAL_SPAWN_DELAY 150

class Gameplay {
  u16 spawn_timer;
public:
  Board board;
  Player player;
  Polyomino polyomino;
  Fruits fruits;
  InputMode input_mode;

  Gameplay();
  ~Gameplay();
  void loop();
private:
  void render();
};
