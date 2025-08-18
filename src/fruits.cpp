#include "fruits.hpp"
#include "animation.hpp"
#include "bank-helper.hpp"
#include "banked-asset-helpers.hpp"
#include "ggsound.hpp"
#include "metasprites.hpp"
#include "unicorn.hpp"
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

const s8 Fruits::fruit_rows[Fruits::NUM_FRUITS][4] = {{1, 5, 5, 9},
                                                      {3, 7, 3, 7}};

const Sprite *Fruits::high_fruits[] = {
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

const Sprite *Fruits::low_fruits[] = {
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

void Fruits::spawn_on_board(u8 fruit_index) {
  auto fruit = fruits[fruit_index];
  fruit.row = -1;
  fruit.column = 0;

  Fruit::Type type = fruit_types_per_stage[(u8)current_stage][RAND_UP_TO(
      sizeof(fruit_types_per_stage[(u8)current_stage]))];

  fruit.low_metasprite = low_fruits[(u8)type];
  fruit.high_metasprite = high_fruits[(u8)type];

  // pick a random row
  static_assert(sizeof(fruit_rows[0]) == 4);
  fruit.row = fruit_rows[fruit_index][RAND_UP_TO_POW2(2)]; // see assert above
  if (board.row_filled((u8)fruit.row)) {
    // if row is bad, give up for now, we try next frame
    return;
  }

  fruit.column = (u8)banked_lambda(Board::BANK, [this, &fruit]() {
    return board.random_free_column((u8)fruit.row);
  });

  fruit.state = Fruit::State::Dropping;
  fruit.x = (u8)((fruit.column << 4) + board.origin_x);
  fruit.y = (u8)((fruit.row << 4) + board.origin_y);
  fruit.raindrop_y = fruit.y.get();
  while (fruit.raindrop_y >= DROP_SPEED) {
    fruit.raindrop_y -= DROP_SPEED;
  }
  fruit.life = EXPIRATION_TIME;
  banked_lambda(Animation::BANK, [this]() {
    splash_animation.reset();
    unsplash_animation.reset();
  });
}

Fruits::Fruits(Board &board)
    : board(board), score_value(SCORE_VALUE_START),
      splash_animation(&splash_cells), unsplash_animation(&unsplash_cells) {
  spawn_timer = SPAWN_DELAY /
                2; // just so player don't wait too much to see the first fruit
  for (auto fruit : fruits) {
    fruit.state = Fruit::State::Inactive;
  }
  active_fruits = 0;
}

void Fruits::update(Unicorn &unicorn, bool &snack_was_eaten, bool can_spawn) {
#pragma clang loop unroll(enable)
  for (auto fruit : fruits) {
    switch (fruit.state) {
    case Fruit::State::Inactive:
      break;
    case Fruit::State::Dropping:
      if (fruit.raindrop_y == fruit.y) {
        if (splash_animation.current_cell_flags(AnimationFlags::splash__sfx)) {
          // near splash 14
          // TODO: check if this is ok
          GGSound::play_sfx(SFX::Snackspawn, GGSound::SFXPriority::One);
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
        fruit.state = Fruit::State::Crushed;
      } else if ((unicorn.state == Unicorn::State::Idle ||
                  unicorn.state == Unicorn::State::Moving) &&
                 (unicorn.x.whole + 8) >> 4 == fruit.column &&
                 (unicorn.y.whole + 8) >> 4 == fruit.row) {
        fruit.state = Fruit::State::Inactive;
        active_fruits--;
        banked_lambda(Unicorn::BANK, [this, &unicorn]() {
          unicorn.feed(FRUIT_NUTRITION);
          unicorn.add_score(score_value);
        });
        if (score_value < SCORE_VALUE_MAX) {
          score_value += SCORE_VALUE_INCREMENT;
        }
        snack_was_eaten = true;
      } else if (fruit.state == Fruit::State::Active) {
        if (--fruit.life == 0) {
          fruit.state = Fruit::State::Despawning;
          fruit.despawn_counter = DESPAWN_DELAY;
        } else {
          fruit.bobbing_counter++;
        }
      } else { // if (fruit.state == Fruit::State::Despawning) {
        if (--fruit.despawn_counter == 0) {
          fruit.state = Fruit::State::Inactive;
          active_fruits--;
          score_value = SCORE_VALUE_START;
        }
      }
      break;
    case Fruit::State::Crushed:
      if (unsplash_animation.finished) {
        fruit.state = Fruit::State::Inactive;
        active_fruits--;
        score_value = SCORE_VALUE_START;
      }
      break;
    }
  }

  if (!can_spawn) {
    return;
  }

  if (spawn_timer >= SPAWN_DELAY) {
    for (u8 fruit_index = 0; fruit_index < NUM_FRUITS; fruit_index++) {
      if (fruits[fruit_index].state == Fruit::State::Inactive) {
        spawn_on_board(fruit_index);
        if (fruits[fruit_index].state == Fruit::State::Dropping) {
          active_fruits++;
          spawn_timer -= SPAWN_DELAY;
        }
        break;
      }
    }
  } else if (active_fruits < NUM_FRUITS) {
    spawn_timer++;
  }
}

void Fruits::render_fruit(u8 fruit_index, int y_scroll) {
  auto fruit = fruits[fruit_index];
  switch (fruit.state) {
  case Fruit::State::Despawning:
    if ((fruit.despawn_counter & 0b111) == 0b100) {
      break;
    }
  case Fruit::State::Active:
    banked_oam_meta_spr(METASPRITES_BANK, fruit.x, fruit.y - y_scroll,
                        (fruit.bobbing_counter & 0b10000)
                            ? fruit.high_metasprite
                            : fruit.low_metasprite);
    break;
  case Fruit::State::Dropping:
    if (fruit.y == fruit.raindrop_y) {
      if (splash_animation.current_cell_flags(
              AnimationFlags::splash__high_fruit)) {
        // splash anim 14 & 15
        banked_oam_meta_spr(METASPRITES_BANK, fruit.x, fruit.y - y_scroll,
                            fruit.high_metasprite);
      } else if (splash_animation.current_cell_flags(
                     AnimationFlags::splash__low_fruit)) {
        // splash anim 16 & 17
        banked_oam_meta_spr(METASPRITES_BANK, fruit.x, fruit.y - y_scroll,
                            fruit.low_metasprite);
      }
      // NOTE: assumes this runs on fixed bank
      {
        ScopedBank animationBank(Animation::BANK);
        splash_animation.update(fruit.x, fruit.y - y_scroll);
      }
    } else {
      auto near_shadow = (fruit.y - fruit.raindrop_y) <= 48;
      auto &metasprite =
          near_shadow ? Metasprites::RainShadowB : Metasprites::RainShadowA;

      auto shadow_position = fruit.raindrop_y - y_scroll;
      if (shadow_position > 0 && shadow_position < 0xe0) {
        u8 drop_tile = near_shadow ? 0xb2 : 0xb1;

        oam_spr(fruit.x + 4, (u8)(shadow_position), drop_tile, 0);
      }
      banked_oam_meta_spr(METASPRITES_BANK, fruit.x, fruit.y - y_scroll,
                          metasprite);
    }
    break;
  case Fruit::State::Crushed: {
    ScopedBank animationBank(Animation::BANK);
    unsplash_animation.update(
        fruit.x,
        fruit.y - y_scroll +
            8); // animation runs 8 pixels below the fruit reference point
    break;
  }
  case Fruit::State::Inactive:
    break;
  }
}

void Fruits::render_below_player(int y_scroll, u8 y_player) {
#pragma clang loop unroll(enable)
  for (u8 i = 0; i < NUM_FRUITS; i++) {
    if (fruits[i].y > y_player) {
      render_fruit(i, y_scroll);
    };
  }
}

void Fruits::render_above_player(int y_scroll, u8 y_player) {
#pragma clang loop unroll(enable)
  for (u8 i = 0; i < NUM_FRUITS; i++) {
    if (fruits[i].y <= y_player) {
      render_fruit(i, y_scroll);
    };
  }
}
