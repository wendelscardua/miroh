#include <bank.h>
#include <nesdoug.h>
#include <neslib.h>

#include "bank-helper.hpp"

#include "chr-data.hpp"
#include "common.hpp"
#include "donut.hpp"
#include "ggsound.hpp"
#include "metasprites.hpp"
#include "nametables.hpp"
#include "palettes.hpp"
#include "soundtrack.hpp"
#include "title-screen.hpp"

#pragma clang section text = ".prg_rom_0.text"
#pragma clang section rodata = ".prg_rom_0.rodata"

const unsigned char menu_text[24 * 3] = {
    0x00, 0x00, 0x23, 0x4f, 0x4e, 0x54, 0x52, 0x4f, 0x4c, 0x53, 0x00, 0x00,
    0x00, 0x23, 0x52, 0x45, 0x44, 0x49, 0x54, 0x53, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x33, 0x54, 0x41, 0x52, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x2f, 0x50, 0x54, 0x49, 0x4f, 0x4e, 0x53, 0x00, 0x00, 0x00, 0x00 };

const TitleScreen::MenuOption left_of[] = {
  TitleScreen::MenuOption::Controls,// Controls
  TitleScreen::MenuOption::Controls,// Credits
  TitleScreen::MenuOption::Start,   // Start
  TitleScreen::MenuOption::Start,   // Settings
};

const TitleScreen::MenuOption right_of[] = {
  TitleScreen::MenuOption::Credits,// Controls
  TitleScreen::MenuOption::Credits,// Credits
  TitleScreen::MenuOption::Settings,   // Start
  TitleScreen::MenuOption::Settings,   // Settings
};

const TitleScreen::MenuOption above_of[] = {
    TitleScreen::MenuOption::Controls, // Controls
    TitleScreen::MenuOption::Credits,  // Credits
    TitleScreen::MenuOption::Controls, // Start
    TitleScreen::MenuOption::Credits,  // Settings
};

const TitleScreen::MenuOption below_of[] = {
  TitleScreen::MenuOption::Start,// Controls
  TitleScreen::MenuOption::Settings,// Credits
  TitleScreen::MenuOption::Start,   // Start
  TitleScreen::MenuOption::Settings,   // Settings
};

const TitleScreen::MenuOption next[] = {
    TitleScreen::MenuOption::Start,    // Controls
    TitleScreen::MenuOption::Settings,  // Credits
    TitleScreen::MenuOption::Credits, // Start
    TitleScreen::MenuOption::Controls, // Settings
};

const u8 option_mino_x[] = {
  0x20, // Controls
  0x78, // Credits
  0x20, // Start
  0x78, // Settings
};

const u8 option_mino_y[] = {
  0x70, // Controls
  0x70, // Credits
  0x80, // Start
  0x80, // Settings
};

const u8 option_mino_frame_mod[] = {
  0x10, // Controls
  0x10, // Credits
  0x08, // Start
  0x20, // Settings
};

__attribute__((noinline)) TitleScreen::TitleScreen() :
  state(State::PressStart),
  current_option(MenuOption::Controls) {
    set_chr_bank(0);

    banked_lambda(GET_BANK(bg_chr), [] () {
      // assume all chr are on same bank
      vram_adr(PPU_PATTERN_TABLE_0);
      Donut::decompress_to_ppu((void *)&bg_chr, PPU_PATTERN_TABLE_SIZE / 64);

      vram_adr(PPU_PATTERN_TABLE_1);
      Donut::decompress_to_ppu((void *)&sprites_chr, PPU_PATTERN_TABLE_SIZE / 64);
    });

    banked_lambda(GET_BANK(title_nam), [] () {
      // idem nametables
      vram_adr(NAMETABLE_D);
      vram_write(how_to_nam, 1024);

      vram_adr(NAMETABLE_A);
      vram_write(title_nam, 1024);
    });

    banked_lambda(GET_BANK(bg_palette), [] () {
      // idem palettes
      pal_bg(bg_palette);
      pal_spr(sprites_player_palette);
    });

    pal_bright(0);

    oam_clear();

    scroll(0, 0);

    ppu_on_all();

    banked_lambda(GET_BANK(song_list), [] () {
      GGSound::play_song(Song::Miroh);
    });

    pal_fade_to(0, 4);
}

__attribute__((noinline)) TitleScreen::~TitleScreen() {
    pal_fade_to(4, 0);
    ppu_off();
}

__attribute__((noinline)) void TitleScreen::loop() {
  while(current_mode == GameMode::TitleScreen) {
    ppu_wait_nmi();

    oam_clear();

    pad_poll(0);

    rand16();

    u8 pressed = get_pad_new(0);

    switch(state) {
    case State::PressStart:
      if (pressed & (PAD_START | PAD_A)) {
        multi_vram_buffer_horz(menu_text, 24, NTADR_A(4, 15));
        multi_vram_buffer_horz(menu_text+24, 24, NTADR_A(4, 16));
        multi_vram_buffer_horz(menu_text+48, 24, NTADR_A(4, 17));

        banked_lambda(GET_BANK(sfx_list), []() {
          GGSound::play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);
        });
        state = State::Options;
        current_option = MenuOption::Controls;
      }
      break;
    case State::Options:
      if (pressed & (PAD_START | PAD_A)) {
        switch(current_option) {
        case MenuOption::Controls:
          state = State::HowToPlay;
          scroll(0, 240);
          break;
        case MenuOption::Credits:
          state = State::Credits;
          pal_fade_to(4, 0);
          ppu_off();

          banked_lambda(GET_BANK(credits_nam), [] () {
            vram_adr(NAMETABLE_D);
            vram_write(credits_nam, 1024);
          });

          scroll(0, 240);

          ppu_on_all();
          pal_fade_to(0, 4);
          break;
        case MenuOption::Start:
          state = State::PressStart;
          current_mode = GameMode::Gameplay;
          break;
        case MenuOption::Settings:
          break;
        }
        break;
      } else if (pressed & PAD_UP) {
        current_option = above_of[(u8) current_option];
      } else if (pressed & PAD_DOWN) {
        current_option = below_of[(u8) current_option];
      } else if (pressed & PAD_LEFT) {
        current_option = left_of[(u8) current_option];
      } else if (pressed & PAD_RIGHT) {
        current_option = right_of[(u8) current_option];
      } else if (pressed & (PAD_SELECT|PAD_B)) {
        current_option = next[(u8)current_option];
      }

      if (get_frame_count() & option_mino_frame_mod[(u8) current_option]) {
        oam_meta_spr(option_mino_x[(u8) current_option], option_mino_y[(u8) current_option], metasprite_Menutaur1);
      } else {
        oam_meta_spr(option_mino_x[(u8) current_option], option_mino_y[(u8) current_option], metasprite_Menutaur2);
      }
      break;
    case State::HowToPlay:
      if (pressed & (PAD_START | PAD_A)) {
        scroll(0, 0);
        state = State::Options;
      }
      break;
    case State::Credits:
      if (pressed & (PAD_START | PAD_A)) {
        state = State::Options;
        pal_fade_to(4, 0);
        ppu_off();

        banked_lambda(GET_BANK(credits_nam), [] () {
          vram_adr(NAMETABLE_D);
          vram_write(how_to_nam, 1024);
        });
        scroll(0, 0);

        ppu_on_all();
        pal_fade_to(0, 4);
      }
      break;
    }
  }
}
