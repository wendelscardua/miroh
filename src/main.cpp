#include <bank.h>
#include <nesdoug.h>
#include <neslib.h>

#include "bank-helper.hpp"
#include "common.hpp"
#include "ggsound.hpp"
#include "title-screen.hpp"
#include "gameplay.hpp"

#include "soundtrack-ptr.hpp"

GameMode current_mode;
GameMode previous_mode;
u16 high_score;

static void main_init() {
  previous_mode = GameMode::None;
  current_mode = GameMode::TitleScreen;
  high_score = 0;

  ppu_off();

  set_mirroring(MIRROR_HORIZONTAL);

  // set 8x8 sprite mode
  oam_size(0);

  // Use lower half of PPU memory for background tiles
  bank_bg(0);
  // ... and upper half for sprites
  bank_spr(1);

  set_vram_buffer();

  set_prg_bank(GET_BANK(song_list));
  GGSound::init(GGSound::Region::NTSC,
                song_list,
                sfx_list,
                instrument_list,
                GET_BANK(song_list));
}

int main() {
  main_init();

  while (true) {
    switch (current_mode) {
    case GameMode::TitleScreen:
      banked_lambda(0, [](){
        TitleScreen titleScreen;
        titleScreen.loop();
      });
      break;
    case GameMode::Gameplay:
      banked_lambda(0, [](){
        Gameplay gameplay;
        gameplay.loop();
      });
    default:
      break;
    }
  }
}
