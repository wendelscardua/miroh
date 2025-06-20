#pragma once

#include <soa.h>

#include "animation-defs.hpp"
#include "animation.hpp"
#include "board.hpp"
#include "common.hpp"
#include "metasprites.hpp"
#include "unicorn.hpp"

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
  u8 column;
  u8 x;
  u8 y;

  union {
    u8 raindrop_y;
    u8 despawn_counter;
    u8 bobbing_counter;
  };
  u16 life;
  const Sprite *low_metasprite;
  const Sprite *high_metasprite;
  State state;
};

#define SOA_STRUCT Fruit
#define SOA_MEMBERS                                                            \
  MEMBER(row)                                                                  \
  MEMBER(column)                                                               \
  MEMBER(x)                                                                    \
  MEMBER(y)                                                                    \
  MEMBER(bobbing_counter)                                                      \
  MEMBER(raindrop_y)                                                           \
  MEMBER(despawn_counter)                                                      \
  MEMBER(life)                                                                 \
  MEMBER(low_metasprite)                                                       \
  MEMBER(high_metasprite)                                                      \
  MEMBER(state)

#include <soa-struct.inc>

class Fruits {
  static constexpr u8 NUM_FRUITS = 2;
  static constexpr u16 EXPIRATION_TIME = 12 * 60;
  static constexpr u8 DROP_SPEED = 12;
  static constexpr u8 DESPAWN_DELAY = 23;
  static constexpr s8 fruit_rows[][4] = {{1, 5, 5, 9}, {3, 7, 3, 7}};

  static constexpr const Sprite *high_fruits[] = {
      Metasprites::AppleHigh,      Metasprites::CornHigh,
      Metasprites::PearHigh,       Metasprites::AvocadoHigh,
      Metasprites::EggplantHigh,   Metasprites::KiwiHigh,
      Metasprites::BroccoliHigh,   Metasprites::GreenPeasHigh,
      Metasprites::StrawberryHigh, Metasprites::CherriesHigh,
      Metasprites::GrapesHigh,     Metasprites::CucumberHigh,
      Metasprites::ClementineHigh, Metasprites::HallabongHigh,
      Metasprites::CarrotHigh,     Metasprites::BerriesHigh,
      Metasprites::BlueCornHigh,   Metasprites::BananasHigh,
      Metasprites::SweetPotatoHigh};

  static constexpr const Sprite *low_fruits[] = {
      Metasprites::AppleLow,      Metasprites::CornLow,
      Metasprites::PearLow,       Metasprites::AvocadoLow,
      Metasprites::EggplantLow,   Metasprites::KiwiLow,
      Metasprites::BroccoliLow,   Metasprites::GreenPeasLow,
      Metasprites::StrawberryLow, Metasprites::CherriesLow,
      Metasprites::GrapesLow,     Metasprites::CucumberLow,
      Metasprites::ClementineLow, Metasprites::HallabongLow,
      Metasprites::CarrotLow,     Metasprites::BerriesLow,
      Metasprites::BlueCornLow,   Metasprites::BananasLow,
      Metasprites::SweetPotatoLow};

  static_assert(sizeof(fruit_rows) == 4 * Fruits::NUM_FRUITS);

  soa::Array<Fruit, NUM_FRUITS> fruits;
  u8 active_fruits;
  Board &board;
  u16 spawn_timer;

  Animation splash_animation{&splash_cells};

public:
  static constexpr u16 SPAWN_DELAY = 5 * 60;
  static constexpr u8 FRUIT_NUTRITION = 3;

  Fruits(Board &board);

  void update(Unicorn &player, bool &snack_was_eaten, bool can_spawn);

  void spawn_on_board(u8 fruit_index);

  __attribute((noinline)) void render_fruit(Fruit fruit, int y_scroll);
  __attribute((noinline)) void render_below_player(int y_scroll, u8 y_player);
  __attribute((noinline)) void render_above_player(int y_scroll, u8 y_player);
};
