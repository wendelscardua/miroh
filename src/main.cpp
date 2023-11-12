#include <nesdoug.h>
#include <neslib.h>

#include "bank-helper.hpp"
#include "common.hpp"
#include "gameplay.hpp"
#include "ggsound.hpp"
#include "maze-defs.hpp"
#include "title-screen.hpp"

#include "soundtrack-ptr.hpp"

GameState current_game_state;
GameState previous_game_state;
u16 high_score[NUM_MAZES];
Maze maze;

static void main_init() {
  previous_game_state = GameState::None;
  current_game_state = GameState::TitleScreen;
  for (u8 i = 0; i < NUM_MAZES; i++) {
    high_score[i] = 0;
  }

  maze = 0;

  ppu_off();

  // set 8x8 sprite mode
  oam_size(0);

  // Use lower half of PPU memory for background tiles
  bank_bg(0);
  // ... and upper half for sprites
  bank_spr(1);

  set_vram_buffer();

  {
    ScopedBank scopedBank(GET_BANK(song_list));
    GGSound::init(GGSound::Region::NTSC, song_list, sfx_list, instrument_list,
                  GET_BANK(song_list));
  }
}

int main() {
  main_init();

  Board board;

  while (true) {
    switch (current_game_state) {
    case GameState::TitleScreen: {
      ScopedBank scopedBank(TitleScreen::BANK);
      TitleScreen titleScreen(board);
      titleScreen.loop();
    }; break;
    case GameState::Gameplay: {
      ScopedBank scopedBank(Gameplay::BANK);
      Gameplay gameplay(board);
      gameplay.loop();
    };
    default:
      break;
    }
  }
}
