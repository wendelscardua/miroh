#pragma once

#include <soa.h>

#include "board.hpp"
#include "common.hpp"
#include "player.hpp"

struct Fruit {
  enum class Type : u8 {
    Apple,
    Corn,
    Pear,
    Avocado,
    Eggplant,
    Kiwi,
    Broccoli,
    GreenPeas,
    Strawberry,
    Cherries,
    Grapes,
    Cucumber,
    Clementine,
    Hallabong,
    Carrot,
    Berries,
    BlueCorn,
    Bananas,
    SweetPotato
  };

  enum class State {
    Inactive,
    Dropping,
    Active,
    Despawning,
  };

  s8 row;
  s8 column;
  u8 x;
  u8 y;
  u16 life;
  bool active;
  Type type;
  State state;
};

#define SOA_STRUCT Fruit
#define SOA_MEMBERS                                                            \
  MEMBER(row)                                                                  \
  MEMBER(column)                                                               \
  MEMBER(x)                                                                    \
  MEMBER(y)                                                                    \
  MEMBER(life)                                                                 \
  MEMBER(active)                                                               \
  MEMBER(type)                                                                 \
  MEMBER(state)

#include <soa-struct.inc>

class Fruits {
  static constexpr u8 NUM_FRUITS = 2;
  static constexpr u16 EXPIRATION_TIME = 15 * 60;
  static constexpr s8 fruit_rows[][4] = {{1, 5, 5, 9}, {3, 7, 3, 7}};

  static_assert(sizeof(fruit_rows) == 4 * Fruits::NUM_FRUITS);

  soa::Array<Fruit, NUM_FRUITS> fruits;
  u8 active_fruits;
  Board &board;
  u8 fruit_credits;
  u16 spawn_timer;
  u8 current_level;

public:
  static constexpr u8 INITIAL_CREDITS = 1;
  static constexpr u16 SPAWN_DELAY = 480;
  static constexpr u8 FRUIT_NUTRITION = 3;

  Fruits(Board &board, u8 current_level);

  void update(Player &player, bool blocks_placed, u8 lines_filled);

  void spawn_on_board(u8 fruit_index);

  void render_below_player(int y_scroll, int y_player);
  void render_above_player(int y_scroll, int y_player);
};
