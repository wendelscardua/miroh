#include "fruits.hpp"
#include "banked-asset-helpers.hpp"
#include "log.hpp"
#include "utils.hpp"
#include <nesdoug.h>
#include <neslib.h>

const Fruit::Type fruit_types_per_stage[][4] = {
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

  Fruit::Type type = fruit_types_per_stage[(u8)current_stage][RAND_UP_TO(
      sizeof(fruit_types_per_stage[(u8)current_stage]))];

  fruit.low_metasprite = low_fruits[(u8)type];
  fruit.high_metasprite = high_fruits[(u8)type];

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

  fruit.state = Fruit::State::Dropping;
  fruit.x = (u8)((fruit.column << 4) + board.origin_x);
  fruit.y = (u8)((fruit.row << 4) + board.origin_y);
  fruit.raindrop_y = fruit.y.get();
  while (fruit.raindrop_y >= DROP_SPEED) {
    fruit.raindrop_y -= DROP_SPEED;
  }
  fruit.life = EXPIRATION_TIME;
  splash_animation.reset();
}

Fruits::Fruits(Board &board) : board(board) {
  spawn_timer = SPAWN_DELAY /
                2; // just so player don't wait too much to see the first fruit
  for (auto fruit : fruits) {
    fruit.state = Fruit::State::Inactive;
  }
  active_fruits = 0;
}

void Fruits::update(Player &player, bool &snack_was_eaten) {
  for (auto fruit : fruits) {
    switch (fruit.state) {
    case Fruit::State::Inactive:
      break;
    case Fruit::State::Dropping:
      if (fruit.raindrop_y == fruit.y) {
        if (splash_animation.current_cell == 13 &&
            splash_animation.current_frame == 0) {
          // near splash 14
          // TODO: check if this is ok
          banked_play_sfx(SFX::Snackspawn, GGSound::SFXPriority::One);
        }
        if (splash_animation.finished) {
          fruit.state = Fruit::State::Active;
          fruit.bobbing_counter = 0;
        }
      } else {
        fruit.raindrop_y += DROP_SPEED;
      }
      break;
    case Fruit::State::Active:
    case Fruit::State::Despawning:
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
        snack_was_eaten = true;
      } else if (fruit.state == Fruit::State::Active && --fruit.life == 0) {
        fruit.state = Fruit::State::Despawning;
        fruit.despawn_counter = DESPAWN_DELAY;
      } else if (fruit.state == Fruit::State::Despawning &&
                 --fruit.despawn_counter == 0) {
        fruit.state = Fruit::State::Inactive;
        active_fruits--;
      } else {
        fruit.bobbing_counter++;
      }
      break;
    }
  }

  if (spawn_timer >= SPAWN_DELAY) {
    START_MESEN_WATCH(4);
    for (u8 fruit_index = 0; fruit_index < NUM_FRUITS; fruit_index++) {
      if (fruits[fruit_index].state == Fruit::State::Inactive) {
        START_MESEN_WATCH(5);
        spawn_on_board(fruit_index);
        STOP_MESEN_WATCH(5);
        if (fruits[fruit_index].state == Fruit::State::Dropping) {
          active_fruits++;
          spawn_timer -= SPAWN_DELAY;
        }
        break;
      }
    }
    STOP_MESEN_WATCH(4);
  } else if (active_fruits < NUM_FRUITS) {
    spawn_timer++;
  }
}

void Fruits::render_fruit(Fruit fruit, int y_scroll) {
  switch (fruit.state) {
  case Fruit::State::Despawning:
    if ((fruit.despawn_counter & 0b111) == 0b100) {
      break;
    }
  case Fruit::State::Active:
    banked_oam_meta_spr(fruit.x, fruit.y - y_scroll,
                        (fruit.bobbing_counter & 0b10000)
                            ? fruit.high_metasprite
                            : fruit.low_metasprite);
    break;
  case Fruit::State::Dropping:
    if (fruit.y == fruit.raindrop_y) {
      // splash anim
      splash_animation.update(fruit.x, fruit.y - y_scroll);
      if (splash_animation.current_cell == 13 ||
          splash_animation.current_cell == 14) {
        // splash anim 14 & 15
        banked_oam_meta_spr(fruit.x, fruit.y - y_scroll, fruit.high_metasprite);
      } else if (splash_animation.current_cell >= 15) {
        // splash anim 16 & 17
        banked_oam_meta_spr(fruit.x, fruit.y - y_scroll, fruit.low_metasprite);
      }
    } else if (fruit.y - fruit.raindrop_y <= 48) {
      // reaching target position
      if (fruit.raindrop_y - y_scroll > 0 &&
          fruit.raindrop_y - y_scroll < 0xe0) {
        oam_spr(fruit.x + 4, (u8)(fruit.raindrop_y - y_scroll), 0xb2, 0);
      }
      banked_oam_meta_spr(fruit.x, fruit.y - y_scroll, metasprite_RainShadowB);
    } else {
      // far from target
      if (fruit.raindrop_y - y_scroll > 0 &&
          fruit.raindrop_y - y_scroll < 0xe0) {
        oam_spr(fruit.x + 4, (u8)(fruit.raindrop_y - y_scroll), 0xb1, 0);
      }
      banked_oam_meta_spr(fruit.x, fruit.y - y_scroll, metasprite_RainShadowB);
    }
    break;
  case Fruit::State::Inactive:
    break;
  }
}

void Fruits::render_below_player(int y_scroll, int y_player) {
  for (Fruit fruit : fruits) {
    if (fruit.y > y_player) {
      render_fruit(fruit, y_scroll);
    };
  }
}

void Fruits::render_above_player(int y_scroll, int y_player) {
  for (Fruit fruit : fruits) {
    if (fruit.y <= y_player) {
      render_fruit(fruit, y_scroll);
    };
  }
}
