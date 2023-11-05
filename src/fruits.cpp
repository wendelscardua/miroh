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
  for (u8 tries = 0; tries < 4; tries++) {
    static_assert(sizeof(fruit_rows[0]) == 4);
    s8 candidate_row =
        fruit_rows[fruit_index][rand8() & 0b11]; // see assert above
    if (!board.row_filled(candidate_row)) {
      fruit.row = candidate_row;
      break;
    }
  }

  if (fruit.row < 0) {
    // no good row? give up for now, we'll try next frame
    return;
  }

  // now we do the same for column
  for (u8 tries = 0; tries < 4; tries++) {
    s8 candidate_column = (s8)RAND_UP_TO(WIDTH);
    if (!board.occupied(fruit.row, candidate_column)) {
      fruit.column = candidate_column;
      break;
    }
  }
  if (fruit.column < 0) {
    // no good column? give up for now, we'll try next frame
    return;
  }

  fruit.active = true;
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
    fruit.active = false;
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
    if (fruit.active) {
      if (board.occupied(fruit.row, fruit.column)) {
        fruit.active = false;
        active_fruits--;
      } else if ((player.state == Player::State::Idle ||
                  player.state == Player::State::Moving) &&
                 (player.x.whole + 8) >> 4 == fruit.column &&
                 (player.y.whole + 8) >> 4 == fruit.row) {
        fruit.active = false;
        active_fruits--;
        player.feed(FRUIT_NUTRITION);
      } else if (--fruit.life == 0) {
        fruit.active = false;
        active_fruits--;
      }
    }
  }

  if (fruit_credits > 0 && active_fruits < NUM_FRUITS &&
      ++spawn_timer > SPAWN_DELAY) {
    START_MESEN_WATCH(4);
    for (u8 fruit_index = 0; fruit_index < NUM_FRUITS; fruit_index++) {
      if (!fruits[fruit_index].active) {
        START_MESEN_WATCH(5);
        spawn_on_board(fruit_index);
        STOP_MESEN_WATCH(5);
        if (fruits[fruit_index].active) {
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

const u8 *const high_fruits[]{metasprite_AppleHigh, metasprite_CornHigh,
                              metasprite_PearHigh, metasprite_AvocadoHigh};

const u8 *const low_fruits[]{metasprite_AppleLow, metasprite_CornLow,
                             metasprite_PearLow, metasprite_AvocadoLow};

void Fruits::render(int y_scroll) {
  bool state = (get_frame_count() & 0b10000);
  for (auto fruit : fruits) {
    if (fruit.active) {
      Fruit::Type type = fruit.type;
      banked_oam_meta_spr(fruit.x, fruit.y - y_scroll,
                          state ? high_fruits[(u8)type] : low_fruits[(u8)type]);
    };
    state = !state;
  }
}
