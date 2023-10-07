#pragma once

#include <soa.h>

#include "board.hpp"
#include "common.hpp"
#include "player.hpp"

#define NUM_FRUITS 8
#define INITIAL_CREDITS 1
#define SPAWN_DELAY 480

#define FRUIT_NUTRITION 5

struct Fruit {
  s8 row;
  s8 column;
  u8 x;
  u8 y;
  bool active;
};

#define SOA_STRUCT Fruit
#define SOA_MEMBERS                                                            \
  MEMBER(row)                                                                  \
  MEMBER(column)                                                               \
  MEMBER(x)                                                                    \
  MEMBER(y)                                                                    \
  MEMBER(active)

#include <soa-struct.inc>

class Fruits {
  soa::Array<Fruit, NUM_FRUITS> fruits;
  u8 active_fruits;
  Board &board;
  u8 fruit_credits;
  u16 spawn_timer;

public:
  Fruits(Board &board);

  void update(Player &player, bool blocks_placed, u8 lines_filled);

  void spawn_on_board(soa::Ptr<Fruit> fruit);

  void render();
};
