#include "fruits.hpp"
#include "metasprites.hpp"
#include "player.hpp"
#include <nesdoug.h>
#include <neslib.h>

void Fruits::spawn_on_board(soa::Ptr<Fruit> fruit) {
  fruit.row = -1;
  fruit.column = -1;
  for(u8 tries = 0; tries < HEIGHT / 2; tries++) {
    s8 i = rand8() & 0x0f;
    if (i < HEIGHT && board.tally[i] < WIDTH) {
      fruit.row = i;
      break;
    }
  }
  if (fruit.row < 0) {
    // random row failed, look each row
    // arbitrarily look from up to bottom or from bottom to up
    if (get_frame_count() & 0b1) {
      for(s8 i = 0; i < HEIGHT; i++) {
        if (board.tally[i] < WIDTH) {
          fruit.row = i;
          break;
        }
      }
    } else {
      for(s8 i = HEIGHT - 1; i >= 0; i--) {
        if (board.tally[i] < WIDTH) {
          fruit.row = i;
          break;
        }
      }
    }
    if (fruit.row < 0) {
      // no good row? well, give up spawning then
      return;
    }
  }

  // now we do the same for column
  for(u8 tries = 0; tries < WIDTH / 2; tries++) {
    s8 j = rand8() & 0x0f;
    if (j < WIDTH && !board.cell[fruit.row][j].occupied) {
      fruit.column = j;
      break;
    }
  }
  if (fruit.column < 0) {
    // random column failed, look each column
    // arbitrarily look from left to right or from right to left
    if ((get_frame_count()^fruit.row) & 0b1) {
      for(s8 j = 0; j < WIDTH; j++) {
        if (!board.cell[fruit.row][j].occupied) {
          fruit.column = j;
          break;
        }
      }
    } else {
      for(s8 j = WIDTH - 1; j >= 0; j--) {
        if (!board.cell[fruit.row][j].occupied) {
          fruit.column = j;
          break;
        }
      }
    }
    if (fruit.column < 0) {
      // no good column? well, give up spawning then
      // wait a minute, that shoulnd't even be possible?
      return;
    }
  }
  fruit.active = true;
  fruit.x = (u8)((fruit.column << 4) + board.origin_x);
  fruit.y = (u8)((fruit.row << 4) + board.origin_y);
}

Fruits::Fruits(Board& board) : board(board) {
  fruit_credits = INITIAL_CREDITS;
  spawn_timer = SPAWN_DELAY / 2; // just so player don't wait too much to see the first fruit
  for (auto fruit : fruits) {
    fruit.active = false;
  }
  active_fruits = 0;
}

void Fruits::update(Player& player, bool blocks_placed) {
  if (blocks_placed) fruit_credits++;

  for(auto fruit : fruits) {
    if (fruit.active) {
      if (board.occupied(fruit.row, fruit.column)) {
        fruit.active = false;
        active_fruits--;
      } else if (player.state == Player::State::Idle &&
                 player.x.whole >> 4 == fruit.column &&
                 player.y.whole >> 4 == fruit.row) {
        fruit.active = false;
        active_fruits--;
        player.feed(FRUIT_NUTRITION);
      }
    }
  }

  if (fruit_credits > 0 && active_fruits < NUM_FRUITS && ++spawn_timer > SPAWN_DELAY) {
    spawn_timer = 0;
    for (auto fruit : fruits) {
      if (!fruit.active) {
        spawn_on_board(fruit);
        if (fruit.active) {
          active_fruits++;
          fruit_credits--;
        }
        break;
      }
    }
  }
}

void Fruits::render() {
  for(auto fruit : fruits) {
    if (fruit.active) {
      oam_meta_spr(fruit.x, fruit.y, metasprite_fruit);
    };
  }
}
