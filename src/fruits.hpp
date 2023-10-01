#pragma once

#include "board.hpp"
#include "common.hpp"
#include "player.hpp"
#include <array>

#define NUM_FRUITS 8
#define SPAWN_DELAY 480

#define FRUIT_NUTRITION 4

struct Fruit {
  s8 row;
  s8 column;
  bool active;

  void render(u8 origin_x, u8 origin_y);

  // tries to place fruit on an unoccupied place on board (assuming there is one)
  void spawn_on_board(Board& board);
};

class Fruits {
  std::array<Fruit, NUM_FRUITS> fruits;
  u8 active_fruits;
  Board& board;
  u16 spawn_timer;
public:
  Fruits(Board& board);

  void update(Player& player);

  void render();
};
