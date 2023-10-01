#pragma once

#include "board.hpp"
#include "common.hpp"
#include "player.hpp"
#include <array>

#define NUM_FRUITS 8
#define SPAWN_DELAY 480

#define FRUIT_NUTRITION 8

struct Fruit {
  s8 row;
  s8 column;
  u8 x;
  u8 y;
  bool active;

  void render();

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
