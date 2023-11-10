#include "fruits.hpp"
#include "banked-asset-helpers.hpp"
#include "board.hpp"
#include "log.hpp"
#include "metasprites.hpp"
#include "player.hpp"
#include "utils.hpp"
#include <nesdoug.h>
#include <neslib.h>

const Fruit::Type fruit_types_per_level[][4] = {
    // Starlit Stables
    {Fruit::Type::Apple, Fruit::Type::Corn, Fruit::Type::Pear,
     Fruit::Type::Avocado},
    // Rainbow Retreat
    {Fruit::Type::Eggplant, Fruit::Type::Kiwi, Fruit::Type::Broccoli,
     Fruit::Type::GreenPeas},
    // Fairy Forest
    {Fruit::Type::Strawberry, Fruit::Type::Cherries, Fruit::Type::Grapes,
     Fruit::Type::Cucumber},
    // Glittery Grotto
    {Fruit::Type::Clementine, Fruit::Type::Hallabong, Fruit::Type::Carrot,
     Fruit::Type::Berries},
    // Marshmallow Mountain
    {Fruit::Type::Berries, Fruit::Type::BlueCorn, Fruit::Type::Bananas,
     Fruit::Type::SweetPotato},
};

void Fruits::spawn_on_board(u8 fruit_index) {
  auto fruit = fruits[fruit_index];
  fruit.row = -1;
  fruit.column = -1;

  fruit.type = fruit_types_per_level[current_level][RAND_UP_TO(
      sizeof(fruit_types_per_level[current_level]))];

  // pick a random row
  static_assert(sizeof(fruit_rows[0]) == 4);
  fruit.row = fruit_rows[fruit_index][RAND_UP_TO_POW2(2)]; // see assert above
  if (board.row_filled(fruit.row)) {
    // if row is bad, give up for now, we try next frame
    return;
  }

  s8 possible_columns[WIDTH];
  u8 max_possible_columns = 0;
  u16 bits = board.occupied_bitset[(u8)fruit.row];
  for (s8 j = 0; j < WIDTH; j++) {
    if (!(bits & 0b1)) {
      possible_columns[max_possible_columns++] = j;
    }
    bits >>= 1;
  }

  fruit.column = possible_columns[RAND_UP_TO(max_possible_columns)];

  fruit.state = Fruit::State::Active;
  fruit.x = (u8)((fruit.column << 4) + board.origin_x);
  fruit.y = (u8)((fruit.row << 4) + board.origin_y);
  fruit.life = EXPIRATION_TIME;
}

Fruits::Fruits(Board &board, u8 current_level)
    : board(board), current_level(current_level) {
  fruit_credits = INITIAL_CREDITS;
  spawn_timer = SPAWN_DELAY /
                2; // just so player don't wait too much to see the first fruit
  for (auto fruit : fruits) {
    fruit.state = Fruit::State::Inactive;
  }
  active_fruits = 0;
}

void Fruits::update(Player &player, bool blocks_placed, u8 lines_filled) {
  if (lines_filled) {
    spawn_timer += SPAWN_DELAY / 2 * lines_filled;
    fruit_credits += lines_filled;
  } else if (blocks_placed) {
    fruit_credits++;
  }

  for (auto fruit : fruits) {
    if (fruit.state == Fruit::State::Active) {
      if (board.occupied(fruit.row, fruit.column)) {
        fruit.state = Fruit::State::Inactive;
        active_fruits--;
      } else if ((player.state == Player::State::Idle ||
                  player.state == Player::State::Moving) &&
                 (player.x.whole + 8) >> 4 == fruit.column &&
                 (player.y.whole + 8) >> 4 == fruit.row) {
        fruit.state = Fruit::State::Inactive;
        active_fruits--;
        player.feed(FRUIT_NUTRITION);
      } else if (--fruit.life == 0) {
        fruit.state = Fruit::State::Inactive;
        active_fruits--;
      }
    }
  }

  if (fruit_credits > 0 && active_fruits < NUM_FRUITS &&
      ++spawn_timer > SPAWN_DELAY) {
    START_MESEN_WATCH(4);
    for (u8 fruit_index = 0; fruit_index < NUM_FRUITS; fruit_index++) {
      if (fruits[fruit_index].state == Fruit::State::Inactive) {
        START_MESEN_WATCH(5);
        spawn_on_board(fruit_index);
        STOP_MESEN_WATCH(5);
        if (fruits[fruit_index].state == Fruit::State::Active) {
          active_fruits++;
          fruit_credits--;
          spawn_timer -= SPAWN_DELAY;
        }
        break;
      }
    }
    STOP_MESEN_WATCH(4);
  }
}

const u8 *const high_fruits[] = {
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

const u8 *const low_fruits[] = {
    metasprite_AppleLow,      metasprite_CornLow,      metasprite_PearLow,
    metasprite_AvocadoLow,    metasprite_EggplantLow,  metasprite_KiwiLow,
    metasprite_BroccoliLow,   metasprite_GreenPeasLow, metasprite_StrawberryLow,
    metasprite_CherriesLow,   metasprite_GrapesLow,    metasprite_CucumberLow,
    metasprite_ClementineLow, metasprite_HallabongLow, metasprite_CarrotLow,
    metasprite_BerriesLow,    metasprite_BlueCornLow,  metasprite_BananasLow,
    metasprite_SweetPotatoLow};

void Fruits::render_below_player(int y_scroll, int y_player) {
  bool state = (get_frame_count() & 0b10000);
  for (auto fruit : fruits) {
    if (fruit.state == Fruit::State::Active && fruit.y > y_player) {
      Fruit::Type type = fruit.type;
      banked_oam_meta_spr(fruit.x, fruit.y - y_scroll,
                          state ? high_fruits[(u8)type] : low_fruits[(u8)type]);
    };
    state = !state;
  }
}

void Fruits::render_above_player(int y_scroll, int y_player) {
  bool state = (get_frame_count() & 0b10000);
  for (auto fruit : fruits) {
    if (fruit.state == Fruit::State::Active && fruit.y <= y_player) {
      Fruit::Type type = fruit.type;
      banked_oam_meta_spr(fruit.x, fruit.y - y_scroll,
                          state ? high_fruits[(u8)type] : low_fruits[(u8)type]);
    };
    state = !state;
  }
}
