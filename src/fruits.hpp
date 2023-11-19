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
  s8 column;
  u8 x;
  u8 y;

  u8 bobbing_counter;
  union {
    u8 raindrop_y;
    u8 despawn_counter;
  };
  u16 life;
  const u8 *low_metasprite;
  const u8 *high_metasprite;
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

  static constexpr const u8 *high_fruits[] = {
      metasprite_AppleHigh,      metasprite_CornHigh,
      metasprite_PearHigh,       metasprite_AvocadoHigh,
      metasprite_EggplantHigh,   metasprite_KiwiHigh,
      metasprite_BroccoliHigh,   metasprite_GreenPeasHigh,
      metasprite_StrawberryHigh, metasprite_CherriesHigh,
      metasprite_GrapesHigh,     metasprite_CucumberHigh,
      metasprite_ClementineHigh, metasprite_HallabongHigh,
      metasprite_CarrotHigh,     metasprite_BerriesHigh,
      metasprite_BlueCornHigh,   metasprite_BananasHigh,
      metasprite_SweetPotatoHigh};

  static constexpr const u8 *low_fruits[] = {
      metasprite_AppleLow,      metasprite_CornLow,
      metasprite_PearLow,       metasprite_AvocadoLow,
      metasprite_EggplantLow,   metasprite_KiwiLow,
      metasprite_BroccoliLow,   metasprite_GreenPeasLow,
      metasprite_StrawberryLow, metasprite_CherriesLow,
      metasprite_GrapesLow,     metasprite_CucumberLow,
      metasprite_ClementineLow, metasprite_HallabongLow,
      metasprite_CarrotLow,     metasprite_BerriesLow,
      metasprite_BlueCornLow,   metasprite_BananasLow,
      metasprite_SweetPotatoLow};

  static_assert(sizeof(fruit_rows) == 4 * Fruits::NUM_FRUITS);

  soa::Array<Fruit, NUM_FRUITS> fruits;
  u8 active_fruits;
  Board &board;
  u16 spawn_timer;

  Animation splash_animation{&splash_cells,
                             sizeof(splash_cells) / sizeof(AnimCell)};

public:
  static constexpr u16 SPAWN_DELAY = 5 * 60;
  static constexpr u8 FRUIT_NUTRITION = 3;

  Fruits(Board &board);

  void update(Unicorn &player, bool &snack_was_eaten, bool can_spawn);

  void spawn_on_board(u8 fruit_index);

  void render_fruit(Fruit fruit, int y_scroll);
  void render_below_player(int y_scroll, int y_player);
  void render_above_player(int y_scroll, int y_player);
};
