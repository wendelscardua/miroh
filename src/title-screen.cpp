#include <bank.h>
#include <nesdoug.h>
#include <neslib.h>

#include "bank-helper.hpp"

#include "banked-asset-helpers.hpp"
#include "chr-data.hpp"
#include "common.hpp"
#include "donut.hpp"
#include "ggsound.hpp"
#include "maze-defs.hpp"
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
    0x00, 0x2f, 0x50, 0x54, 0x49, 0x4f, 0x4e, 0x53, 0x00, 0x00, 0x00, 0x00};

const unsigned char settings_text[24 * 3] = {
    0x00, 0x00, 0x00, 0x2c, 0x49, 0x4e, 0x45, 0x00, 0x47, 0x52, 0x41, 0x56,
    0x49, 0x54, 0x59, 0x1a, 0x1c, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x2d, 0x41, 0x5a, 0x45, 0x1a, 0x00, 0x1c, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x32, 0x45, 0x54, 0x55, 0x52, 0x4e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const TitleScreen::MenuOption left_of[] = {
    TitleScreen::MenuOption::Controls, // Controls
    TitleScreen::MenuOption::Controls, // Credits
    TitleScreen::MenuOption::Start,    // Start
    TitleScreen::MenuOption::Start,    // Settings
};

const TitleScreen::MenuOption right_of[] = {
    TitleScreen::MenuOption::Credits,  // Controls
    TitleScreen::MenuOption::Credits,  // Credits
    TitleScreen::MenuOption::Settings, // Start
    TitleScreen::MenuOption::Settings, // Settings
};

const TitleScreen::MenuOption above_of[] = {
    TitleScreen::MenuOption::Controls, // Controls
    TitleScreen::MenuOption::Credits,  // Credits
    TitleScreen::MenuOption::Controls, // Start
    TitleScreen::MenuOption::Credits,  // Settings
};

const TitleScreen::MenuOption below_of[] = {
    TitleScreen::MenuOption::Start,    // Controls
    TitleScreen::MenuOption::Settings, // Credits
    TitleScreen::MenuOption::Start,    // Start
    TitleScreen::MenuOption::Settings, // Settings
};

const TitleScreen::MenuOption next[] = {
    TitleScreen::MenuOption::Start,    // Controls
    TitleScreen::MenuOption::Settings, // Credits
    TitleScreen::MenuOption::Credits,  // Start
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

const u8 setting_mino_y[] = {
    0x70, // Line gravity
    0x78, // Maze
    0x80, // Return
};

const TitleScreen::SettingsOption setting_above[] = {
    TitleScreen::SettingsOption::Return,
    TitleScreen::SettingsOption::LineGravity,
    TitleScreen::SettingsOption::Maze,
};

const TitleScreen::SettingsOption setting_below[] = {
    TitleScreen::SettingsOption::Maze,
    TitleScreen::SettingsOption::Return,
    TitleScreen::SettingsOption::LineGravity,
};

__attribute__((noinline)) TitleScreen::TitleScreen()
    : state(State::PressStart), current_option(MenuOption::Controls) {
  set_chr_bank(0);

  banked_lambda(GET_BANK(bg_chr), []() {
    // assume all chr are on same bank
    vram_adr(PPU_PATTERN_TABLE_0);
    Donut::decompress_to_ppu((void *)&bg_chr, PPU_PATTERN_TABLE_SIZE / 64);

    vram_adr(PPU_PATTERN_TABLE_1);
    Donut::decompress_to_ppu((void *)&sprites_chr, PPU_PATTERN_TABLE_SIZE / 64);
  });

  banked_lambda(GET_BANK(title_nam), []() {
    // idem nametables
    vram_adr(NAMETABLE_D);
    vram_write(how_to_nam, 1024);

    vram_adr(NAMETABLE_A);
    vram_write(title_nam, 1024);
  });

  banked_lambda(GET_BANK(bg_palette), []() {
    // idem palettes
    pal_bg(bg_palette);
    pal_spr(sprites_player_palette);
  });

  pal_bright(0);

  oam_clear();

  scroll(0, 0);

  ppu_on_all();

  banked_lambda(GET_BANK(song_list), []() { GGSound::play_song(Song::Miroh); });

  pal_fade_to(0, 4);
}

__attribute__((noinline)) TitleScreen::~TitleScreen() {
  pal_fade_to(4, 0);
  ppu_off();
}

__attribute__((noinline)) void TitleScreen::loop() {
  while (current_mode == GameMode::TitleScreen) {
    ppu_wait_nmi();

    oam_clear();

    pad_poll(0);

    rand16();

    u8 pressed = get_pad_new(0);

    switch (state) {
    case State::PressStart:
      if (pressed & (PAD_START | PAD_A)) {
        multi_vram_buffer_horz(menu_text, 24, NTADR_A(4, 15));
        multi_vram_buffer_horz(menu_text + 24, 24, NTADR_A(4, 16));
        multi_vram_buffer_horz(menu_text + 48, 24, NTADR_A(4, 17));

        banked_lambda(GET_BANK(sfx_list), []() {
          GGSound::play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);
        });
        state = State::Options;
        current_option = MenuOption::Controls;
      }
      break;
    case State::Options:
      if (pressed & (PAD_START | PAD_A)) {
        switch (current_option) {
        case MenuOption::Controls:
          banked_lambda(GET_BANK(sfx_list), []() {
            GGSound::play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);
          });
          state = State::HowToPlay;
          how_to_animation_framecount = 60;
          how_to_animation_step = 0;
          scroll(0, 240);
          break;
        case MenuOption::Credits:
          banked_lambda(GET_BANK(sfx_list), []() {
            GGSound::play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);
          });
          state = State::Credits;
          pal_fade_to(4, 0);
          ppu_off();

          banked_lambda(GET_BANK(credits_nam), []() {
            vram_adr(NAMETABLE_D);
            vram_write(credits_nam, 1024);
          });

          scroll(0, 240);

          ppu_on_all();
          pal_fade_to(0, 4);
          break;
        case MenuOption::Start:
          banked_lambda(GET_BANK(sfx_list), []() {
            GGSound::play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);
          });
          state = State::PressStart;
          current_mode = GameMode::Gameplay;
          break;
        case MenuOption::Settings:
          banked_lambda(GET_BANK(sfx_list), []() {
            GGSound::play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);
          });
          state = State::Settings;
          current_setting = SettingsOption::LineGravity;
          multi_vram_buffer_horz(settings_text, 24, NTADR_A(4, 15));
          multi_vram_buffer_horz(settings_text + 24, 24, NTADR_A(4, 16));
          multi_vram_buffer_horz(settings_text + 48, 24, NTADR_A(4, 17));
          break;
        }
        break;
      } else if (pressed & PAD_UP) {
        banked_lambda(GET_BANK(sfx_list), []() {
          GGSound::play_sfx(SFX::Turn_left, GGSound::SFXPriority::One);
        });
        current_option = above_of[(u8)current_option];
      } else if (pressed & PAD_DOWN) {
        banked_lambda(GET_BANK(sfx_list), []() {
          GGSound::play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);
        });
        current_option = below_of[(u8)current_option];
      } else if (pressed & PAD_LEFT) {
        banked_lambda(GET_BANK(sfx_list), []() {
          GGSound::play_sfx(SFX::Turn_left, GGSound::SFXPriority::One);
        });
        current_option = left_of[(u8)current_option];
      } else if (pressed & PAD_RIGHT) {
        banked_lambda(GET_BANK(sfx_list), []() {
          GGSound::play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);
        });
        current_option = right_of[(u8)current_option];
      } else if (pressed & (PAD_SELECT | PAD_B)) {
        banked_lambda(GET_BANK(sfx_list), []() {
          GGSound::play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);
        });
        current_option = next[(u8)current_option];
      }

      if (get_frame_count() & option_mino_frame_mod[(u8)current_option]) {
        banked_oam_meta_spr(option_mino_x[(u8)current_option],
                            option_mino_y[(u8)current_option],
                            metasprite_Menutaur1);
      } else {
        banked_oam_meta_spr(option_mino_x[(u8)current_option],
                            option_mino_y[(u8)current_option],
                            metasprite_Menutaur2);
      }
      break;
    case State::HowToPlay:
      if (pressed & (PAD_START | PAD_A)) {
        banked_lambda(GET_BANK(sfx_list), []() {
          GGSound::play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);
        });
        scroll(0, 0);
        banked_lambda(GET_BANK(bg_palette),
                      []() { pal_spr(sprites_player_palette); });
        state = State::Options;
        break;
      }
      if (--how_to_animation_framecount <= 0) {
        how_to_animation_framecount = 0;
        how_to_animation_step++;
      }
      switch (how_to_animation_step) {
      case 0: // show player selected
      case 2:
        if (how_to_animation_framecount == 0) {
          how_to_animation_framecount = 90;
        }
        banked_lambda(GET_BANK(bg_palette),
                      []() { pal_spr(sprites_player_palette); });
        break;
      case 1: // show polyomino selected
      case 3:
        if (how_to_animation_framecount == 0) {
          how_to_animation_framecount = 90;
        }
        banked_lambda(GET_BANK(bg_palette),
                      []() { pal_spr(sprites_polyomino_palette); });
        break;
      case 4: // rotating minos
        if (how_to_animation_framecount == 0) {
          how_to_animation_framecount = 256;
        }
        break;
      case 5: // hard drop
        if (how_to_animation_framecount == 0) {
          how_to_animation_framecount = 60;
        }
        if (how_to_animation_framecount > 48) {
          banked_oam_meta_spr(0x48, 0x70, metasprite_Menumino0);
        } else if (how_to_animation_framecount > 16) {
          banked_oam_meta_spr(
              0x48, (u8)(0xb0 - (how_to_animation_framecount - 16) * 2) & 0xf8,
              metasprite_Menumino0);
        } else {
          banked_oam_meta_spr(0x48, 0xb0, metasprite_Menumino0);
        }
        break;
      case 6: // soft drop
        if (how_to_animation_framecount == 0) {
          how_to_animation_framecount = 256;
        }
        {
          s8 delta_x = 0;
          if (how_to_animation_framecount < 200 &&
              how_to_animation_framecount > 160) {
            delta_x = -4;
          } else if (how_to_animation_framecount < 100 &&
                     how_to_animation_framecount > 90) {
            delta_x = 8;
          } else if (how_to_animation_framecount < 120 &&
                     how_to_animation_framecount > 75) {
            delta_x = 4;
          }
          if (how_to_animation_framecount < 60) {
            how_to_animation_framecount--;
          }
          banked_oam_meta_spr((u8)(0xa8 + delta_x),
                              (u8)(0xb0 - how_to_animation_framecount / 4) &
                                  0xf8,
                              metasprite_Menumino2);
        }
        break;
      case 7: // idle before loop
        if (how_to_animation_framecount == 0) {
          how_to_animation_framecount = 60;
        }
        break;
      default:
        how_to_animation_step = -1;
      }

      // always show minotaur
      if (how_to_animation_framecount & 0b10000) {
        banked_oam_meta_spr(0x48, 0x1e, metasprite_MinoRight1);
      } else {
        banked_oam_meta_spr(0x48, 0x1e, metasprite_MinoRight2);
      }

      // also show the switcheable polyomino
      banked_oam_meta_spr(0xa8, 0x1e, metasprite_Menumino0);

      // the rotating minos
      {
        if (how_to_animation_step == 4) {
          switch (how_to_animation_framecount & 0b11000000) {
          case 0b00000000:
            banked_oam_meta_spr(0x38, 0x48, metasprite_Menumino0);
            banked_oam_meta_spr(0xb8, 0x48, metasprite_Menumino0);
            break;
          case 0b01000000:
            banked_oam_meta_spr(0x38, 0x48, metasprite_MenuminoL);
            banked_oam_meta_spr(0xb8, 0x48, metasprite_MenuminoR);
            break;
          case 0b10000000:
            banked_oam_meta_spr(0x38, 0x48, metasprite_Menumino2);
            banked_oam_meta_spr(0xb8, 0x48, metasprite_Menumino2);
            break;
          case 0b11000000:
            banked_oam_meta_spr(0x38, 0x48, metasprite_MenuminoR);
            banked_oam_meta_spr(0xb8, 0x48, metasprite_MenuminoL);
            break;
          }
        } else {
          banked_oam_meta_spr(0x38, 0x48, metasprite_Menumino0);
          banked_oam_meta_spr(0xb8, 0x48, metasprite_Menumino0);
        }
      }

      // idle hard drop
      if (how_to_animation_step < 5) {
        banked_oam_meta_spr(0x48, 0x70, metasprite_Menumino0);
      } else if (how_to_animation_step > 5) {
        banked_oam_meta_spr(0x48, 0xb0, metasprite_Menumino0);
      }

      // idle soft drop
      if (how_to_animation_step < 6) {
        banked_oam_meta_spr(0xa8, 0x70, metasprite_Menumino2);
      } else if (how_to_animation_step > 6) {
        banked_oam_meta_spr(0xa8, 0xb0, metasprite_Menumino2);
      }

      break;
    case State::Credits:
      if (pressed & (PAD_START | PAD_A)) {
        banked_lambda(GET_BANK(sfx_list), []() {
          GGSound::play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);
        });
        state = State::Options;
        pal_fade_to(4, 0);
        ppu_off();

        banked_lambda(GET_BANK(credits_nam), []() {
          vram_adr(NAMETABLE_D);
          vram_write(how_to_nam, 1024);
        });
        scroll(0, 0);

        ppu_on_all();
        pal_fade_to(0, 4);
      }
      break;
    case State::Settings:
      if (pressed & PAD_UP) {
        current_setting = setting_above[(u8)current_setting];
      } else if (pressed & (PAD_DOWN | PAD_SELECT)) {
        banked_lambda(GET_BANK(sfx_list), []() {
          GGSound::play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);
        });
        current_setting = setting_below[(u8)current_setting];
      } else if (pressed & PAD_LEFT) {
        switch (current_setting) {
        case SettingsOption::LineGravity:
          banked_lambda(GET_BANK(sfx_list), []() {
            GGSound::play_sfx(SFX::Turn_left, GGSound::SFXPriority::One);
          });
          line_gravity_enabled = !line_gravity_enabled;
          break;
        case SettingsOption::Maze:
          banked_lambda(GET_BANK(sfx_list), []() {
            GGSound::play_sfx(SFX::Turn_left, GGSound::SFXPriority::One);
          });
          if (maze > 0) {
            maze--;
          }
          break;
        case SettingsOption::Return:
          break;
        }
      } else if (pressed & PAD_RIGHT) {
        switch (current_setting) {
        case SettingsOption::LineGravity:
          banked_lambda(GET_BANK(sfx_list), []() {
            GGSound::play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);
          });
          line_gravity_enabled = !line_gravity_enabled;
          break;
        case SettingsOption::Maze:
          banked_lambda(GET_BANK(sfx_list), []() {
            GGSound::play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);
          });
          if (maze < NUM_MAZES - 1) {
            maze++;
          }
          break;
        case SettingsOption::Return:
          break;
        }
      } else if (pressed & (PAD_A | PAD_START)) {
        switch (current_setting) {
        case SettingsOption::LineGravity:
          banked_lambda(GET_BANK(sfx_list), []() {
            GGSound::play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);
          });
          line_gravity_enabled = !line_gravity_enabled;
          break;
        case SettingsOption::Maze:
          banked_lambda(GET_BANK(sfx_list), []() {
            GGSound::play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);
          });
          if (maze < NUM_MAZES - 1) {
            maze++;
          }
          break;
        case SettingsOption::Return:
          banked_lambda(GET_BANK(sfx_list), []() {
            GGSound::play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);
          });
          state = State::Options;
          multi_vram_buffer_horz(menu_text, 24, NTADR_A(4, 15));
          multi_vram_buffer_horz(menu_text + 24, 24, NTADR_A(4, 16));
          multi_vram_buffer_horz(menu_text + 48, 24, NTADR_A(4, 17));
          break;
        }
      } else {
        if (line_gravity_enabled) {
          multi_vram_buffer_horz((const u8[]){0x2f, 0x4e, 0x00}, 3,
                                 NTADR_A(21, 15));
        } else {
          multi_vram_buffer_horz((const u8[]){0x2f, 0x46, 0x46}, 3,
                                 NTADR_A(21, 15));
        }
        multi_vram_buffer_horz(maze_names[(u8)maze], 10, NTADR_A(14, 16));

        u8 setting_y = setting_mino_y[(u8)current_setting];
        if (get_frame_count() & 0b1000) {
          banked_oam_meta_spr(0x24, setting_y, metasprite_Menutaur1);
        } else {
          banked_oam_meta_spr(0x24, setting_y, metasprite_Menutaur2);
        }
      }
      break;
    }
  }
}
