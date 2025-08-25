#include "board-animation.hpp"
#include "cell.hpp"

// same bank as Board
#pragma clang section text = ".prg_rom_4.text.board-animation"
#pragma clang section rodata = ".prg_rom_4.rodata.board-animation"

bool BoardAnimation::paused = false;

const BoardAnimFrame BoardAnimation::block_jiggle[] = {
    {CellType::Jiggling, 8},
    {CellType::Marshmallow, 8},
    {CellType::Jiggling, 8},
    {CellType::Marshmallow, 0}};
const BoardAnimFrame BoardAnimation::block_move_right[] = {
    {CellType::LeanLeft, 4}, {CellType::Maze, 0}};
const BoardAnimFrame BoardAnimation::block_move_left[] = {
    {CellType::LeanRight, 4}, {CellType::Maze, 0}};
const BoardAnimFrame BoardAnimation::block_arrive_right[] = {
    {CellType::Maze, 4},
    {CellType::LeanLeft, 4},
    {CellType::LeanRight, 4},
    {CellType::LeanLeft, 4},
    {CellType::Marshmallow, 0}};
const BoardAnimFrame BoardAnimation::block_arrive_left[] = {
    {CellType::Maze, 4},
    {CellType::LeanRight, 4},
    {CellType::LeanLeft, 4},
    {CellType::LeanRight, 4},
    {CellType::Marshmallow, 0}};
const BoardAnimFrame BoardAnimation::block_break_right[] = {
    {CellType::LeanLeft, 4},
    {CellType::LeanRight, 4},
    {CellType::LeanLeft, 4},
    {CellType::Maze, 0}};
const BoardAnimFrame BoardAnimation::block_break_left[] = {
    {CellType::LeanRight, 4},
    {CellType::LeanLeft, 4},
    {CellType::LeanRight, 4},
    {CellType::Maze, 0}};

BoardAnimation::BoardAnimation() : cells(nullptr), finished(true) {}

BoardAnimation::BoardAnimation(const BoardAnimFrame (*cells)[], u8 row,
                               u8 column)
    : cells(cells), current_cell(&(*cells)[0]), current_frame(0), row(row),
      column(column), finished(false) {}

void BoardAnimation::update() {
  if (paused || finished) {
    return;
  }
  if (current_cell->duration == 0) {
    finished = true;
  } else if (++current_frame >= current_cell->duration) {
    current_frame = 0;
    current_cell++;
  }
}
