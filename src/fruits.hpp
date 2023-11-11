#pragma once

#include <soa.h>

#include "board.hpp"
#include "common.hpp"
#include "metasprites.hpp"
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
  union {
    u8 dropping_counter;
    u8 bobbing_counter;
  };
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
  MEMBER(dropping_counter)                                                     \
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
  static constexpr const u8 *splash_metasprite[] = {
      metasprite_Splash1,  metasprite_Splash1,  metasprite_Splash1,
      metasprite_Splash2,  metasprite_Splash2,  metasprite_Splash2,
      metasprite_Splash3,  metasprite_Splash3,  metasprite_Splash3,
      metasprite_Splash4,  metasprite_Splash4,  metasprite_Splash4,
      metasprite_Splash5,  metasprite_Splash5,  metasprite_Splash5,
      metasprite_Splash6,  metasprite_Splash6,  metasprite_Splash6,
      metasprite_Splash7,  metasprite_Splash7,  metasprite_Splash7,
      metasprite_Splash8,  metasprite_Splash8,  metasprite_Splash8,
      metasprite_Splash9,  metasprite_Splash9,  metasprite_Splash9,
      metasprite_Splash10, metasprite_Splash10, metasprite_Splash10,
      metasprite_Splash11, metasprite_Splash11, metasprite_Splash11,
      metasprite_Splash12, metasprite_Splash12, metasprite_Splash12,
      metasprite_Splash13, metasprite_Splash13, metasprite_Splash13,
      metasprite_Splash14, metasprite_Splash14, metasprite_Splash14,
      metasprite_Splash15, metasprite_Splash15, metasprite_Splash15,
      metasprite_Splash16, metasprite_Splash16, metasprite_Splash16,
      metasprite_Splash17, metasprite_Splash17, metasprite_Splash17,
  };
  static constexpr u8 SPLASH_FRAMES = 51;
  static_assert(sizeof(splash_metasprite) ==
                SPLASH_FRAMES * sizeof(splash_metasprite[0]));

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
  u8 current_level;

public:
  static constexpr u16 SPAWN_DELAY = 5 * 60;
  static constexpr u8 FRUIT_NUTRITION = 3;

  Fruits(Board &board, u8 current_level);

  void update(Player &player);

  void spawn_on_board(u8 fruit_index);

  void render_fruit(Fruit fruit, int y_scroll) const;
  void render_below_player(int y_scroll, int y_player);
  void render_above_player(int y_scroll, int y_player);
};
