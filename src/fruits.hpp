#pragma once

#include <soa.h>

#include "board.hpp"
#include "common.hpp"
#include "player.hpp"

#define INITIAL_CREDITS 1
#define SPAWN_DELAY 480

#define FRUIT_NUTRITION 5

struct Fruit {
  enum class Type : u8 {
    Apple,
    Corn,
    Pear,
    Avocado,
  };

  s8 row;
  s8 column;
  u8 x;
  u8 y;
  u16 life;
  bool active;
  Type type;
};

#define SOA_STRUCT Fruit
#define SOA_MEMBERS                                                            \
  MEMBER(row)                                                                  \
  MEMBER(column)                                                               \
  MEMBER(x)                                                                    \
  MEMBER(y)                                                                    \
  MEMBER(life)                                                                 \
  MEMBER(active)                                                               \
  MEMBER(type)

#include <soa-struct.inc>

class Fruits {
  static constexpr u8 NUM_FRUITS = 2;
  static constexpr u16 EXPIRATION_TIME = 15 * 60;
  soa::Array<Fruit, NUM_FRUITS> fruits;
  u8 active_fruits;
  Board &board;
  u8 fruit_credits;
  u16 spawn_timer;
  u8 current_level;

public:
  Fruits(Board &board, u8 current_level);

  void update(Player &player, bool blocks_placed, u8 lines_filled);

  void spawn_on_board(soa::Ptr<Fruit> fruit);

  void render(int y_scroll);
};
