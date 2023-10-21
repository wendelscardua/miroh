#include <bank.h>
#include <nesdoug.h>
#include <neslib.h>

#include "assets.hpp"
#include "bank-helper.hpp"

#include "banked-asset-helpers.hpp"
#include "common.hpp"
#include "donut.hpp"
#include "ggsound.hpp"
#include "maze-defs.hpp"
#include "metasprites.hpp"
#include "soundtrack.hpp"
#include "title-screen.hpp"

#pragma clang section text = ".prg_rom_0.text"
#pragma clang section rodata = ".prg_rom_0.rodata"

const unsigned char menu_text[24 * 3] = {
    0x00, 0x00, 0x00, 0x00, 0x30, 0x4c, 0x41, 0x59, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x2f, 0x50, 0x54, 0x49, 0x4f, 0x4e, 0x53, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x28, 0x45, 0x4c, 0x50, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x23, 0x52, 0x45, 0x44, 0x49, 0x54, 0x53, 0x00, 0x00, 0x00, 0x00};

const unsigned char settings_text[24 * 3] = {
    0x00, 0x00, 0x00, 0x2c, 0x49, 0x4e, 0x45, 0x00, 0x47, 0x52, 0x41, 0x56,
    0x49, 0x54, 0x59, 0x1a, 0x1c, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x2d, 0x41, 0x5a, 0x45, 0x1a, 0x00, 0x1c, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x32, 0x45, 0x54, 0x55, 0x52, 0x4e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const TitleScreen::MenuOption left_of[] = {
    TitleScreen::MenuOption::Start,    // Start
    TitleScreen::MenuOption::Controls, // Controls
    TitleScreen::MenuOption::Start,    // Settings
    TitleScreen::MenuOption::Controls, // Credits
};

const TitleScreen::MenuOption right_of[] = {
    TitleScreen::MenuOption::Settings, // Start
    TitleScreen::MenuOption::Credits,  // Controls
    TitleScreen::MenuOption::Settings, // Settings
    TitleScreen::MenuOption::Credits,  // Credits
};

const TitleScreen::MenuOption above_of[] = {
    TitleScreen::MenuOption::Start,    // Start
    TitleScreen::MenuOption::Start,    // Controls
    TitleScreen::MenuOption::Settings, // Settings
    TitleScreen::MenuOption::Settings, // Credits
};

const TitleScreen::MenuOption below_of[] = {
    TitleScreen::MenuOption::Controls, // Start
    TitleScreen::MenuOption::Controls, // Controls
    TitleScreen::MenuOption::Credits,  // Settings
    TitleScreen::MenuOption::Credits,  // Credits
};

const TitleScreen::MenuOption next[] = {
    TitleScreen::MenuOption::Controls, // Start
    TitleScreen::MenuOption::Settings, // Controls
    TitleScreen::MenuOption::Credits,  // Settings
    TitleScreen::MenuOption::Start,    // Credits
};

const u8 option_mino_x[] = {
    0x30, // Start
    0x30, // Controls
    0x78, // Settings
    0x78, // Credits
};

const u8 option_mino_y[] = {
    0x70, // Start
    0x80, // Controls
    0x70, // Settings
    0x80, // Credits
};

const u8 option_mino_frame_mod[] = {
    0x08, // Start
    0x10, // Controls
    0x20, // Settings
    0x10, // Credits
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
    : state(State::PressStart), current_option(MenuOption::Start) {
  set_chr_bank(0);

  banked_lambda(ASSETS_BANK, []() {
    vram_adr(PPU_PATTERN_TABLE_0);
    Donut::decompress_to_ppu(level_bg_tiles[0], PPU_PATTERN_TABLE_SIZE / 64);

    vram_adr(PPU_PATTERN_TABLE_1);
    Donut::decompress_to_ppu(level_spr_tiles[0], PPU_PATTERN_TABLE_SIZE / 64);

    vram_adr(NAMETABLE_D);
    vram_write(how_to_nam, 1024);

    vram_adr(NAMETABLE_A);
    vram_write(title_nam, 1024);

    pal_bg(level_bg_palettes[0]);
    pal_spr(level_spr_palettes[0]);
  });

  pal_bright(0);

  oam_clear();

  scroll(0, 0);

  ppu_on_all();

  banked_play_song(Song::Miroh);

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

        banked_play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);

        state = State::Options;
        current_option = MenuOption::Start;
      }
      break;
    case State::Options:
      if (pressed & (PAD_START | PAD_A)) {
        switch (current_option) {
        case MenuOption::Controls:
          banked_play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);

          state = State::HowToPlay;
          scroll(0, 240);
          break;
        case MenuOption::Credits:
          banked_play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);

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
          banked_play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);

          state = State::PressStart;
          current_mode = GameMode::Gameplay;
          break;
        case MenuOption::Settings:
          banked_play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);

          state = State::Settings;
          current_setting = SettingsOption::LineGravity;
          multi_vram_buffer_horz(settings_text, 24, NTADR_A(4, 15));
          multi_vram_buffer_horz(settings_text + 24, 24, NTADR_A(4, 16));
          multi_vram_buffer_horz(settings_text + 48, 24, NTADR_A(4, 17));
          break;
        }
        break;
      } else if (pressed & PAD_UP) {
        banked_play_sfx(SFX::Turn_left, GGSound::SFXPriority::One);

        current_option = above_of[(u8)current_option];
      } else if (pressed & PAD_DOWN) {
        banked_play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);

        current_option = below_of[(u8)current_option];
      } else if (pressed & PAD_LEFT) {
        banked_play_sfx(SFX::Turn_left, GGSound::SFXPriority::One);

        current_option = left_of[(u8)current_option];
      } else if (pressed & PAD_RIGHT) {
        banked_play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);

        current_option = right_of[(u8)current_option];
      } else if (pressed & (PAD_SELECT | PAD_B)) {
        banked_play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);

        current_option = next[(u8)current_option];
      }

      if (get_frame_count() & option_mino_frame_mod[(u8)current_option]) {
        banked_oam_meta_spr(option_mino_x[(u8)current_option],
                            option_mino_y[(u8)current_option],
                            metasprite_UniRightWalk1);
      } else {
        banked_oam_meta_spr(option_mino_x[(u8)current_option],
                            option_mino_y[(u8)current_option],
                            metasprite_UniRightWalk2);
      }
      break;
    case State::HowToPlay:
      if (pressed & (PAD_START | PAD_A)) {
        banked_play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);

        scroll(0, 0);
        state = State::Options;
        break;
      }
      break;
    case State::Credits:
      if (pressed & (PAD_START | PAD_A)) {
        banked_play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);

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
        banked_play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);
        current_setting = setting_below[(u8)current_setting];
      } else if (pressed & PAD_LEFT) {
        switch (current_setting) {
        case SettingsOption::LineGravity:
          banked_play_sfx(SFX::Turn_left, GGSound::SFXPriority::One);
          line_gravity_enabled = !line_gravity_enabled;
          break;
        case SettingsOption::Maze:
          banked_play_sfx(SFX::Turn_left, GGSound::SFXPriority::One);
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
          banked_play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);
          line_gravity_enabled = !line_gravity_enabled;
          break;
        case SettingsOption::Maze:
          banked_play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);
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
          banked_play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);
          line_gravity_enabled = !line_gravity_enabled;
          break;
        case SettingsOption::Maze:
          banked_play_sfx(SFX::Turn_right, GGSound::SFXPriority::One);
          if (maze < NUM_MAZES - 1) {
            maze++;
          }
          break;
        case SettingsOption::Return:
          banked_play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);
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
          banked_oam_meta_spr(0x24, setting_y, metasprite_UniRightWalk1);
        } else {
          banked_oam_meta_spr(0x24, setting_y, metasprite_UniRightWalk2);
        }
      }
      break;
    }
  }
}
