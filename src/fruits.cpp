#include "fruits.hpp"
#include "metasprites.hpp"
#include "player.hpp"
#include <nesdoug.h>
#include <neslib.h>

void Fruit::render(u8 origin_x, u8 origin_y) {
  oam_meta_spr(origin_x + (u8)(column << 4),
               origin_y + (u8)(row << 4),
               metasprite_fruit);
}

void Fruit::spawn_on_board(Board& board) {
  row = -1;
  column = -1;
  for(u8 tries = 0; tries < SIZE / 2; tries++) {
    s8 i = rand8() & 0x0f;
    if (i < SIZE && board.tally[i] < SIZE) {
      row = i;
      break;
    }
  }
  if (row < 0) {
    // random row failed, look each row
    // arbitrarily look from up to bottom or from bottom to up
    if (get_frame_count() & 0b1) {
      for(s8 i = 0; i < SIZE; i++) {
        if (board.tally[i] < SIZE) {
          row = i;
          break;
        }
      }
    } else {
      for(s8 i = SIZE - 1; i >= 0; i--) {
        if (board.tally[i] < SIZE) {
          row = i;
          break;
        }
      }
    }
    if (row < 0) {
      // no good row? well, give up spawning then
      return;
    }
  }

  // now we do the same for column
  for(u8 tries = 0; tries < SIZE / 2; tries++) {
    s8 j = rand8() & 0x0f;
    if (j < SIZE && !board.cell[row][j].occupied) {
      column = j;
      break;
    }
  }
  if (column < 0) {
    // random column failed, look each column
    // arbitrarily look from left to right or from right to left
    if ((get_frame_count()^row) & 0b1) {
      for(s8 j = 0; j < SIZE; j++) {
        if (!board.cell[row][j].occupied) {
          column = j;
          break;
        }
      }
    } else {
      for(s8 j = SIZE - 1; j >= 0; j--) {
        if (!board.cell[row][j].occupied) {
          column = j;
          break;
        }
      }
    }
    if (column < 0) {
      // no good column? well, give up spawning then
      // wait a minute, that shoulnd't even be possible?
      return;
    }
  }
  active = true;
}

Fruits::Fruits(Board& board) : board(board) {
  spawn_timer = SPAWN_DELAY / 2; // just so player don't wat too much to see the first fruit
  for (auto &fruit : fruits) {
    fruit.active = false;
  }
  active_fruits = 0;
}

void Fruits::update(Player& player) {
  for(auto& fruit : fruits) {
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

  if (active_fruits < NUM_FRUITS && ++spawn_timer > SPAWN_DELAY) {
    spawn_timer = 0;
    for (auto &fruit : fruits) {
      if (!fruit.active) {
        fruit.spawn_on_board(board);
        if (fruit.active) active_fruits++;
        break;
      }
    }
  }
}

void Fruits::render() {
  for(auto& fruit : fruits) {
    if (fruit.active) {
      fruit.render(board.origin_x, board.origin_y);
    };
  }
}
