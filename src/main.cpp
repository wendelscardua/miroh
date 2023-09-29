#include <bank.h>
#include <nesdoug.h>
#include <neslib.h>

#include "bank-helper.hpp"
#include "common.hpp"
#include "ggsound.hpp"
#include "title-screen.hpp"

#include "soundtrack-ptr.hpp"

GameMode current_mode;
GameMode previous_mode;

static void main_init() {
  previous_mode = GameMode::None;
  current_mode = GameMode::TitleScreen;

  ppu_off();

  set_mirroring(MIRROR_VERTICAL);

  // set 8x16 sprite mode
  oam_size(1);

  // Use lower half of PPU memory for background tiles
  bank_bg(0);

  set_vram_buffer();

  GGSound::init(GGSound::Region::NTSC,
                song_list,
                sfx_list,
                instrument_list,
                GET_BANK(song_list));
}

int main() {
  main_init();

  while (true) {
    ppu_wait_nmi();

    if (current_mode != previous_mode) {
      switch (previous_mode) {
      case GameMode::TitleScreen:
        TitleScreen::deinit();
        break;
      default:
        break;
      }
      ppu_wait_nmi();
      switch (current_mode) {
      case GameMode::TitleScreen:
        TitleScreen::init();
        break;
      default:
        break;
      }
      previous_mode = current_mode;
    }

    pad_poll(0);

    switch (current_mode) {
    case GameMode::TitleScreen:
      TitleScreen::update();
      break;
    default:
      break;
    }

    #ifndef NDEBUG
    gray_line();
    #endif
  }
}
