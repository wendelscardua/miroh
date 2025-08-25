#include "board-animation.hpp"
#include "cell.hpp"

// same bank as Board
#pragma clang section text = ".prg_rom_4.text.board-animation"
#pragma clang section rodata = ".prg_rom_4.rodata.board-animation"

bool BoardAnimation::paused = false;

const BoardAnimFrame BoardAnimation::block_jiggle[] = {
    {{.cell_type = CellType::Jiggling}, 8},
    {{.cell_type = CellType::Marshmallow}, 8},
    {{.cell_type = CellType::Jiggling}, 8},
    {{.cell_type = CellType::Marshmallow}, 0},
    {{.trigger = BoardAnimTrigger::None}, 0}};
const BoardAnimFrame BoardAnimation::block_move_right[] = {
    {{.cell_type = CellType::LeanLeft}, 4},
    {{.cell_type = CellType::Maze}, 0},
    {{.trigger = BoardAnimTrigger::DropFromAbove}, 0}};
const BoardAnimFrame BoardAnimation::block_move_left[] = {
    {{.cell_type = CellType::LeanRight}, 4},
    {{.cell_type = CellType::Maze}, 0},
    {{.trigger = BoardAnimTrigger::DropFromAbove}, 0}};
const BoardAnimFrame BoardAnimation::block_arrive_right[] = {
    {{.cell_type = CellType::Maze}, 4},
    {{.cell_type = CellType::LeanLeft}, 4},
    {{.cell_type = CellType::LeanRight}, 4},
    {{.cell_type = CellType::LeanLeft}, 4},
    {{.cell_type = CellType::Marshmallow}, 0},
    {{.trigger = BoardAnimTrigger::FallDown}, 0}};
const BoardAnimFrame BoardAnimation::block_arrive_left[] = {
    {{.cell_type = CellType::Maze}, 4},
    {{.cell_type = CellType::LeanRight}, 4},
    {{.cell_type = CellType::LeanLeft}, 4},
    {{.cell_type = CellType::LeanRight}, 4},
    {{.cell_type = CellType::Marshmallow}, 0},
    {{.trigger = BoardAnimTrigger::FallDown}, 0}};
const BoardAnimFrame BoardAnimation::block_break_right[] = {
    {{.cell_type = CellType::LeanLeft}, 4},
    {{.cell_type = CellType::LeanRight}, 4},
    {{.cell_type = CellType::LeanLeft}, 4},
    {{.cell_type = CellType::Maze}, 0},
    {{.trigger = BoardAnimTrigger::DropFromAbove}, 0}};
const BoardAnimFrame BoardAnimation::block_break_left[] = {
    {{.cell_type = CellType::LeanRight}, 4},
    {{.cell_type = CellType::LeanLeft}, 4},
    {{.cell_type = CellType::LeanRight}, 4},
    {{.cell_type = CellType::Maze}, 0},
    {{.trigger = BoardAnimTrigger::DropFromAbove}, 0}};
const BoardAnimFrame BoardAnimation::block_start_falling[] = {
    {{.cell_type = CellType::Marshmallow}, 4},
    {{.cell_type = CellType::Maze}, 0},
    {{.trigger = BoardAnimTrigger::None}, 0}};
const BoardAnimFrame BoardAnimation::block_finish_falling[] = {
    {{.cell_type = CellType::Maze}, 4},
    {{.cell_type = CellType::Jiggling}, 4},
    {{.cell_type = CellType::Marshmallow}, 4},
    {{.cell_type = CellType::Jiggling}, 4},
    {{.cell_type = CellType::Marshmallow}, 0},
    {{.trigger = BoardAnimTrigger::FallDown}, 0}};
const BoardAnimFrame BoardAnimation::block_start_dropping[] = {
    {{.cell_type = CellType::Marshmallow}, 4},
    {{.cell_type = CellType::Maze},
     8}, // small delay to slow the chain reaction
    {{.cell_type = CellType::Maze}, 0},
    {{.trigger = BoardAnimTrigger::DropFromAbove}, 0}};

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
    current_cell++;
  } else if (++current_frame >= current_cell->duration) {
    current_frame = 0;
    current_cell++;
  }
}
