#pragma once

#include <soa.h>

#include "board.hpp"
#include "common.hpp"
#include "player.hpp"

#define INITIAL_CREDITS 1
#define SPAWN_DELAY 480

#define FRUIT_NUTRITION 5

struct Fruit {
  s8 row;
  s8 column;
  u8 x;
  u8 y;
  u16 life;
  bool active;
};

#define SOA_STRUCT Fruit
#define SOA_MEMBERS                                                            \
  MEMBER(row)                                                                  \
  MEMBER(column)                                                               \
  MEMBER(x)                                                                    \
  MEMBER(y)                                                                    \
  MEMBER(life)                                                                 \
  MEMBER(active)

#include <soa-struct.inc>

class Fruits {
  static constexpr u8 NUM_FRUITS = 5;
  static constexpr u16 EXPIRATION_TIME = 15 * 60;
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
