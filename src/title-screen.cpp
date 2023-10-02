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

const unsigned char menu_text[24*3]={
	0x00,0x00,0x00,0x00,0x32,0x45,0x41,0x44,0x00,0x49,0x4e,0x53,0x54,0x52,0x55,0x43,0x54,0x49,0x4f,0x4e,0x53,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x33,0x54,0x41,0x52,0x54,0x00,0x47,0x41,0x4d,0x45,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

__attribute__((noinline)) TitleScreen::TitleScreen() : state(State::PressStart) {
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
        current_option = 0;
      }
      break;
    case State::Options:
      if (pressed & (PAD_START | PAD_A)) {
        if (current_option == 0) {
          state = State::HowToPlay;
          scroll(0, 240);
        } else if (current_option == 1) {
          state = State::PressStart;
          current_mode = GameMode::Gameplay;
        } else {
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
        }
        break;
      } else if (pressed & (PAD_UP|PAD_LEFT)) {
        current_option = current_option == 0 ? 2 : current_option - 1;
      } else if (pressed & (PAD_DOWN|PAD_RIGHT|PAD_SELECT|PAD_B)) {
        current_option = current_option == 2 ? 0 : current_option + 1;
      }

      if (current_option == 0) {
        if (get_frame_count() & 0b10000) {
          oam_meta_spr(0x30, 0x70, metasprite_Menutaur1);
        } else {
          oam_meta_spr(0x30, 0x70, metasprite_Menutaur2);
        }
      } else if (current_option == 1) {
        if (get_frame_count() & 0b1000) {
          oam_meta_spr(0x48, 0x78, metasprite_Menutaur1);
        } else {
          oam_meta_spr(0x48, 0x78, metasprite_Menutaur2);
        }
      } else {
        if (get_frame_count() & 0b1000) {
          oam_meta_spr(0x60, 0x80, metasprite_Menutaur1);
        } else {
          oam_meta_spr(0x60, 0x80, metasprite_Menutaur2);
        }
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
