#include "board.hpp"
#include "attributes.hpp"
#include "bag.hpp"
#include "bank-helper.hpp"
#include "common.hpp"
#include "log.hpp"
#include "maze-defs.hpp"
#include <cstdio>
#include <nesdoug.h>
#include <neslib.h>

Cell::Cell() : walls(0), occupied(false), parent(NULL) {}

void Cell::reset() {
  this->walls = 0;
  this->parent = this;
}

Cell *Cell::representative() {
  Cell *temp = this;
  while(temp != temp->parent) {
    temp->parent = temp->parent->parent;
    temp = temp->parent;
  }
  return temp;
}

void Cell::join(Cell *other) {
  Cell *x = this->representative();
  Cell *y = other->representative();
  if (x != y) y->parent = x;
}

Board::Board(u8 origin_x, u8 origin_y) : origin_x(origin_x), origin_y(origin_y) {
  // reset tally
  for(u8 i = 0; i < HEIGHT; i++) {
    tally[i] = 0;
    deleted[i] = 0;
  }
  cracking_row = -1;
  cracking_column = -1;
  erasing_row = -1;
  erasing_column = -1;
  dropping_column = -1;
  dropping_row = -1;

  // reset walls
  for(u8 i = 0; i < HEIGHT; i++) {
    for(u8 j = 0; j < WIDTH; j++) {
      cell[i][j].reset();
    }
  }

#define NEED_WALL(direction) (template_cell.maybe_##direction##_wall && ((rand8() & 0b11) == 0))
  banked_lambda(GET_BANK(mazes), [this]() {
    static_assert(sizeof(TemplateCell) == 1, "TemplateCell is too big");

    // read required walls from template
    for(u8 i = 0; i < HEIGHT; i++) {
      for(u8 j = 0; j < WIDTH; j++) {
        TemplateCell template_cell = mazes[maze]->template_cells[i][j];
        if (template_cell.value != 0xff) {
          cell[i][j].walls = template_cell.walls;
        }
      }
    }

    // read "maybe" walls from template
    for(u8 i = 0; i < HEIGHT; i++) {
      for(u8 j = 0; j < WIDTH; j++) {
        TemplateCell template_cell = mazes[maze]->template_cells[i][j];

        if (template_cell.value == 0xff) {
          // use the old berzerk algorithm
          // assumes the cell is on the valid range
          switch(rand8() & 0b11) {
          case 0:
            cell[i][j].right_wall = true;
            cell[i][j + 1].left_wall = true;
            break;
          case 1:
            cell[i][j + 1].down_wall = true;
            cell[i + 1][j + 1].up_wall = true;
            break;
          case 2:
            cell[i + 1][j].right_wall = true;
            cell[i + 1][j + 1].left_wall = true;
            break;
          case 3:
            cell[i][j].down_wall = true;
            cell[i + 1][j].up_wall = true;
            break;
          }
          continue;
        }

        if (NEED_WALL(up)) {
          cell[i][j].up_wall = true;
          if (i > 0) { cell[i - 1][j].down_wall = true; }
        }
        if (NEED_WALL(down)) {
          cell[i][j].down_wall = true;
          if (i < HEIGHT - 1) { cell[i + 1][j].up_wall = true; }
        }
        if (NEED_WALL(left)) {
          cell[i][j].left_wall = true;
          if (j > 0) { cell[i][j - 1].right_wall = true; }
        }
        if (NEED_WALL(right)) {
          cell[i][j].right_wall = true;
          if (j < WIDTH - 1) { cell[i][j + 1].left_wall = true; }
        }
      }
    }
  });

  // border walls
  for(u8 i = 0; i < HEIGHT; i++) {
    cell[i][0].left_wall = true;
    cell[i][WIDTH - 1].right_wall = true;
  }

  for(u8 j = 0; j < WIDTH; j++) {
    cell[0][j].up_wall = true;
    cell[HEIGHT - 1][j].down_wall = true;
  }

  // union-find-ish-ly ensure all cells are reachable
  for(u8 i = 0; i < HEIGHT; i++) {
    for(u8 j = 0; j < WIDTH; j++) {
      auto current_cell = &cell[i][j];
      if (j < WIDTH - 1 && !current_cell->right_wall) {
        current_cell->join(&cell[i][j+1]);
      }
      if (i < HEIGHT - 1 && !current_cell->down_wall) {
        current_cell->join(&cell[i+1][j]);
      }
    }
  }

  Bag<u8, HEIGHT> row_bag([](auto * bag){
    for(u8 i = 0; i < HEIGHT; i++) { bag->insert(i); }
  });
  Bag<u8, WIDTH> column_bag([](auto * bag){
    for(u8 j = 0; j < WIDTH; j++) { bag->insert(j); }
  });

  for(u8 rows = 0; rows < HEIGHT; rows++) {
    u8 i = row_bag.take();
    for(u8 columns = 0; columns < WIDTH; columns++) {
      u8 j = column_bag.take();
      auto current_cell = &cell[i][j];
      auto right_cell = j < WIDTH - 1 ? &cell[i][j + 1] : NULL;
      auto down_cell = i < HEIGHT -1 ? &cell[i + 1][j] : NULL;

      bool down_first = rand8() & 0b1;

      if (down_first && down_cell && current_cell->representative() != down_cell->representative()) {
        current_cell->down_wall = false;
        down_cell->up_wall = false;
        down_cell->join(current_cell);
      }

      if (right_cell && current_cell->representative() != right_cell->representative()) {
        current_cell->right_wall = false;
        right_cell->left_wall = false;
        right_cell->join(current_cell);
      }

      if (!down_first && down_cell && current_cell->representative() != down_cell->representative()) {
        current_cell->down_wall = false;
        down_cell->up_wall = false;
        down_cell->join(current_cell);
      }
    }
  }
}

Board::~Board() {}

void Board::render() {
  for(s8 i = 0; i < HEIGHT; i++) {
    for(s8 j = 0; j < WIDTH; j++) {
      restore_maze_cell(i, j);
      flush_vram_update2();
    }
  }
  for(u8 meta_x = origin_x >> 4; meta_x < ((origin_x >> 4) + WIDTH); meta_x++) {
    for(u8 meta_y = origin_y >> 4; meta_y < ((origin_y >> 4) + HEIGHT); meta_y++) {
      Attributes::set(meta_x, meta_y, WALL_ATTRIBUTE);
    }
  }
  Attributes::update_vram();
}

bool Board::occupied(s8 row, s8 column) {
  if (column < 0 ||
      column > WIDTH - 1 ||
      row > HEIGHT - 1)
    return true;

  if (row < 0) return false;

  return cell[row][column].occupied;
}

void Board::occupy(s8 row, s8 column) {
  if (!cell[row][column].occupied) { // just to be safe
    cell[row][column].occupied = true;
    tally[row]++;
  }
}

void Board::free(s8 row, s8 column) {
  if (cell[row][column].occupied) { // just to be safe
    cell[row][column].occupied = false;
    tally[row]--;
  }
}

void Board::block_maze_cell(s8 row, s8 column) {
  int position = NTADR_A((origin_x >> 3) + (column << 1), (origin_y >> 3) + (row << 1));
  multi_vram_buffer_horz((const u8[2]){0x74, 0x75}, 2, position);
  multi_vram_buffer_horz((const u8[2]){0x84, 0x85}, 2, position+0x20);
  Attributes::set((u8) ((origin_x >> 4) + column), (u8) ((origin_y >> 4) + row), BLOCK_ATTRIBUTE);
  occupy(row, column);
}

void Board::restore_maze_cell(s8 row, s8 column) {
  auto current_cell = &cell[row][column];
  int position = NTADR_A((origin_x >> 3) + (column << 1), (origin_y >> 3) + (row << 1));
  char metatile_top[2];
  char metatile_bottom[2];

  if (current_cell->up_wall) {
    if (current_cell->left_wall) { // up left
      metatile_top[0] = TILE_BASE + 7;
    } else { // up !left
      metatile_top[0] = TILE_BASE + 1;
    }
  } else {
    if (current_cell->left_wall) { // !up left
      metatile_top[0] = TILE_BASE + 6;
    } else { // !up !left
      if (row == 0 || column == 0)
        metatile_top[0] = TILE_BASE + 0;
      else
        metatile_top[0] = TILE_BASE + 9;
    }
  }

  // top right tile
  if (current_cell->up_wall) {
    if (current_cell->right_wall) { // up right
      metatile_top[1] = TILE_BASE + 3;
    } else { // up !right
      metatile_top[1] = TILE_BASE + 1;
    }
  } else {
    if (current_cell->right_wall) { // !up right
      metatile_top[1] = TILE_BASE + 2;
    } else { // !up !right
      if (row == 0 || column == WIDTH - 1)
        metatile_top[1] = TILE_BASE + 0;
      else
        metatile_top[1] = TILE_BASE + 10;
    }
  }

  // bottom left tile
  if (current_cell->down_wall) {
    if (current_cell->left_wall) { // down left
      metatile_bottom[0] = TILE_BASE + 8;
    } else { // down !left
      metatile_bottom[0] = TILE_BASE + 4;
    }
  } else {
    if (current_cell->left_wall) { // !down left
      metatile_bottom[0] = TILE_BASE + 6;
    } else { // !down !left
      if (row == HEIGHT - 1 || column == 0)
        metatile_bottom[0] = TILE_BASE + 0;
      else
        metatile_bottom[0] = TILE_BASE + 12;
    }
  }

  // bottom right tile (vram_adr auto advanced)
  if (current_cell->down_wall) {
    if (current_cell->right_wall) { // down right
      metatile_bottom[1] = TILE_BASE + 5;
    } else { // down !right
      metatile_bottom[1] = TILE_BASE + 4;
    }
  } else {
    if (current_cell->right_wall) { // !down right
      metatile_bottom[1] = TILE_BASE + 2;
    } else { // !down !right
      if (row == HEIGHT - 1 || column == WIDTH - 1)
        metatile_bottom[1] = TILE_BASE + 0;
      else
        metatile_bottom[1] = TILE_BASE + 11;
    }
  }

  if (mazes[maze]->has_special_cells && mazes[maze]->is_special[row][column]) {
    metatile_top[0] += SPECIAL_DELTA;
    metatile_top[1] += SPECIAL_DELTA;
    metatile_bottom[0] += SPECIAL_DELTA;
    metatile_bottom[1] += SPECIAL_DELTA;
  }

  multi_vram_buffer_horz(metatile_top, 2, position);
  multi_vram_buffer_horz(metatile_bottom, 2, position+0x20);

  Attributes::set((u8) ((origin_x >> 4) + column), (u8) ((origin_y >> 4) + row), WALL_ATTRIBUTE);

  free(row, column);
}

bool Board::ongoing_line_clearing() {
  bool ongoing = false;

  if (cracking_row < 0) {
    for(s8 i = erasing_row + 1; i < HEIGHT; i++) {
      if (tally[i] == WIDTH) {
        deleted[i] = true;
        cracking_row = i;
        cracking_column = 0;
        ongoing = true;
        break;
      }
    }
  } else {
    ongoing = true;
    Attributes::set((u8) ((origin_x >> 4) + cracking_column), (u8) ((origin_y >> 4) + cracking_row), FLASH_ATTRIBUTE);
    int position = NTADR_A((origin_x >> 3) + (cracking_column << 1), (origin_y >> 3) + (cracking_row << 1));
    multi_vram_buffer_horz((const u8[2]){0x76, 0x77}, 2, position);
    multi_vram_buffer_horz((const u8[2]){0x86, 0x87}, 2, position+0x20);

    cracking_column++;
    if (cracking_column == WIDTH) {
      if (erasing_row < 0) {
        erasing_row = cracking_row;
        erasing_column = 0;
      }
      cracking_row = -1;
    }
  }

  if (erasing_row >= 0) {
    ongoing = true;

    restore_maze_cell(erasing_row, erasing_column);

    erasing_column++;
    if (erasing_column == WIDTH) {
      erasing_column = 0;

      for(erasing_row++; erasing_row < HEIGHT; erasing_row++) {
        if (tally[erasing_row] == WIDTH) break;
      }

      if (erasing_row == HEIGHT) {
        erasing_row = -1;
      }
    }
  }

  if (dropping_column < 0) {
    if (!ongoing && erasing_row < 0 && cracking_row < 0 && line_gravity_enabled) {
      // dropping column will only start doing stuff if it ever becomes zero
      for(auto deleted_row : deleted) {
        if (deleted_row) {
          dropping_column = 0;
          dropping_row = -1;
          ongoing = true;
          break;
        }
      }
    }
  } else if (dropping_row < 0) {
    ongoing = true;

    for(s8 i = 0; i < HEIGHT; i++) {
      dropping_column_new_states[i] = occupied(i, dropping_column);
    }

    s8 current_row = HEIGHT - 1;
    s8 source_row = current_row;

    while(current_row >= 0) {
      while (source_row >= 0 && deleted[source_row]) {
        source_row--;
      }
      if (source_row < current_row) {
        bool source_occupied = source_row < 0 ? false : dropping_column_new_states[source_row];

        if (source_occupied) {
          dropping_column_new_states[current_row] = true;
          if (source_row >= 0) dropping_column_new_states[source_row] = false;
        } else {
          dropping_column_new_states[current_row] = false;
        }
      }

      current_row--;
      source_row--;
    }

    dropping_row = HEIGHT - 1;
  } else { // dropping row and column >= 0

    // we don't have budget to do too many changes per frame
    // so we arbitrarily limit them
    for(u8 changes = 0; dropping_row >= 0 && changes < LINE_CLEARING_BUDGET; dropping_row--) {
      if (dropping_column_new_states[dropping_row] != occupied(dropping_row, dropping_column)) {
        changes++;
        if (dropping_column_new_states[dropping_row]) {
          block_maze_cell(dropping_row, dropping_column);
        } else {
          restore_maze_cell(dropping_row, dropping_column);
        }
      }
    }

    if (dropping_row < 0) {
      dropping_column++;
      if (dropping_column == WIDTH) {
        for(s8 i = 0; i < HEIGHT; i++) {
          deleted[i] = false;
        }
        dropping_column = -1;
      }
    }
  }

  return ongoing;
}
