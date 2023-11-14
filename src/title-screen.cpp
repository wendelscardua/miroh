#include <mapper.h>
#include <nesdoug.h>
#include <neslib.h>

#include "assets.hpp"
#include "bank-helper.hpp"

#include "banked-asset-helpers.hpp"
#include "common.hpp"
#include "donut.hpp"
#include "ggsound.hpp"
#include "metasprites.hpp"
#include "soundtrack.hpp"
#include "title-screen.hpp"

#pragma clang section text = ".prg_rom_0.text"
#pragma clang section rodata = ".prg_rom_0.rodata"

const TitleScreen::MenuOption previous_option[] = {
    TitleScreen::MenuOption::HowToPlay,  // OnePlayer
    TitleScreen::MenuOption::OnePlayer,  // TwoPlayers
    TitleScreen::MenuOption::TwoPlayers, // HowToPlay
};

const TitleScreen::MenuOption next_option[] = {
    TitleScreen::MenuOption::TwoPlayers, // OnePlayer
    TitleScreen::MenuOption::HowToPlay,  // TwoPlayers
    TitleScreen::MenuOption::OnePlayer,  // HowToPlay
};

const u8 menu_y_position[] = {
    0x96, // OnePlayer
    0xa6, // TwoPlayers
    0xb6, // HowToPlay
};

const GameMode previous_mode[] = {GameMode::TimeTrial, GameMode::Story,
                                  GameMode::Endless};

const GameMode next_mode[] = {GameMode::Endless, GameMode::TimeTrial,
                              GameMode::Story};

const unsigned char story_label[12 * 1] = {0x02, 0x02, 0x15, 0x16, 0x11, 0x14,
                                           0x1b, 0x00, 0x00, 0x00, 0x00, 0x00};
const unsigned char endless_label[12 * 1] = {
    0x00, 0x02, 0x08, 0x10, 0x07, 0x0e, 0x08, 0x15, 0x15, 0x00, 0x00, 0x00};
const unsigned char time_trial_label[12 * 1] = {
    0x00, 0x00, 0x16, 0x0c, 0x0f, 0x08, 0x00, 0x16, 0x14, 0x0c, 0x04, 0x0e};

__attribute__((noinline)) TitleScreen::TitleScreen(Board &board)
    : state(State::MainMenu), current_option(MenuOption::OnePlayer),
      current_track(Song::Baby_bullhead_title), next_track_delay(0),
      board(board), x_scroll(TITLE_SCROLL) {
  set_chr_bank(0);

  set_mirroring(MIRROR_VERTICAL);

  banked_lambda(ASSETS_BANK, []() {
    vram_adr(PPU_PATTERN_TABLE_0);
    Donut::decompress_to_ppu((void *)base_bg_tiles, 4096 / 64 - 56);
    Donut::decompress_to_ppu((void *)title_bg_tiles, 56);

    vram_adr(PPU_PATTERN_TABLE_1);
    Donut::decompress_to_ppu((void *)spr_tiles, 4096 / 64);

    vram_adr(NAMETABLE_A);
    vram_unrle(title_alt_nametable);

    vram_adr(NAMETABLE_B);
    vram_unrle(title_nametable);

    pal_bg(title_bg_palette);
    pal_spr(title_spr_palette);
  });

  pal_bright(0);

  oam_clear();

  scroll((u16)x_scroll, 0);

  ppu_on_all();

  // TODO: pick title song
  banked_play_song(current_track);

  pal_fade_to(0, 4);
}

__attribute__((noinline)) TitleScreen::~TitleScreen() {
  pal_fade_to(4, 0);
  ppu_off();
}

__attribute__((noinline)) void TitleScreen::loop() {
  bool how_to_players_switched = false;
  u8 how_to_select_timer = 0;

  while (current_game_state == GameState::TitleScreen) {
    ppu_wait_nmi();

    pad_poll(0);
    pad_poll(1);

    rand16();

    u8 pressed = get_pad_new(0) | get_pad_new(1);
    bool bobbing_flag = get_frame_count() & 0b10000;

    if (next_track_delay > 0) {
      next_track_delay--;
      if (next_track_delay == 0) {
        banked_play_song(current_track);
      }
    }

    switch (state) {
    case State::MainMenu:
      if (x_scroll != TITLE_SCROLL) {
        // TODO: easing
        x_scroll += 16;
        set_scroll_x((u16)x_scroll);
        if (x_scroll == PALETTE_SWAP_POINT) {
          pal_col(0x11, 0x15);
          pal_col(0x13, 0x35);
        }
      }
      if (pressed & (PAD_UP | PAD_LEFT)) {
        current_option = previous_option[(u8)current_option];
        banked_play_sfx(SFX::Uioptionscycle, GGSound::SFXPriority::One);
      } else if (pressed & (PAD_DOWN | PAD_RIGHT | PAD_SELECT | PAD_B)) {
        current_option = next_option[(u8)current_option];
        banked_play_sfx(SFX::Uioptionscycle, GGSound::SFXPriority::One);
      } else if (pressed & (PAD_START | PAD_A)) {
        banked_play_sfx(SFX::Uiconfirm, GGSound::SFXPriority::One);
        switch (current_option) {
        case MenuOption::OnePlayer:
        case MenuOption::TwoPlayers:
          current_controller_scheme = current_option == MenuOption::OnePlayer
                                          ? ControllerScheme::OnePlayer
                                          : ControllerScheme::TwoPlayers;
          current_game_mode = GameMode::Story;
          state = State::ModeMenu;
          multi_vram_buffer_horz(story_label, 12, NTADR_B(11, 19));
          multi_vram_buffer_horz(endless_label, 12, NTADR_B(11, 21));
          multi_vram_buffer_horz(time_trial_label, 12, NTADR_B(11, 23));
          break;
        case MenuOption::HowToPlay:
          state = State::HowToPlay;
          break;
        }
      }
      break;
    case State::ModeMenu:
      if (pressed & (PAD_UP | PAD_LEFT)) {
        current_game_mode = previous_mode[(u8)current_game_mode];
        banked_play_sfx(SFX::Uioptionscycle, GGSound::SFXPriority::One);
      } else if (pressed & (PAD_DOWN | PAD_RIGHT | PAD_SELECT | PAD_B)) {
        current_game_mode = next_mode[(u8)current_game_mode];
        banked_play_sfx(SFX::Uioptionscycle, GGSound::SFXPriority::One);
      } else if (pressed & (PAD_START | PAD_A)) {
        banked_play_sfx(SFX::Uiconfirm, GGSound::SFXPriority::One);
        current_game_state = GameState::Gameplay;
        // TODO: select stage on world map
        current_stage = Stage::StarlitStables;
        banked_lambda(Board::MAZE_BANK, [this]() { board.generate_maze(); });
      }
      break;
    case State::HowToPlay:
      if (pressed & (PAD_B)) {
        state = State::MainMenu;
        banked_play_sfx(SFX::Uiabort, GGSound::SFXPriority::One);
      } else if (pressed & (PAD_LEFT | PAD_UP)) {
        if ((u8)current_track == 0) {
          current_track = (Song)(NUM_SONGS - 1);
        } else {
          current_track = (Song)((u8)current_track - 1);
        }
        next_track_delay = NEXT_TRACK_DELAY;
        GGSound::stop();
        banked_play_sfx(SFX::Uioptionscycle, GGSound::SFXPriority::One);
        one_vram_buffer(0x04 + (u8)current_track, TRACK_ID_POSITION);
      } else if (pressed & (PAD_RIGHT | PAD_DOWN | PAD_SELECT | PAD_A)) {
        if ((u8)current_track == NUM_SONGS - 1) {
          current_track = (Song)0;
        } else {
          current_track = (Song)((u8)current_track + 1);
        }
        next_track_delay = NEXT_TRACK_DELAY;
        GGSound::stop();
        banked_play_sfx(SFX::Uioptionscycle, GGSound::SFXPriority::One);
        one_vram_buffer(0x04 + (u8)current_track, TRACK_ID_POSITION);
      }
      how_to_select_timer++;
      if (how_to_select_timer == 90) {
        how_to_players_switched = !how_to_players_switched;
        const u8 pressed_button[] = {SELECT_BUTTON_BASE_TILE + 2,
                                     SELECT_BUTTON_BASE_TILE + 3};
        multi_vram_buffer_horz(pressed_button, 2, NTADR_A(15, 5));
        if (how_to_players_switched) {
          one_vram_buffer(HOW_TO_PLAYER_LABELS_BASE_TILE + 1, NTADR_A(10, 6));
          one_vram_buffer(HOW_TO_PLAYER_LABELS_BASE_TILE, NTADR_A(28, 6));
        } else {
          one_vram_buffer(HOW_TO_PLAYER_LABELS_BASE_TILE, NTADR_A(10, 6));
          one_vram_buffer(HOW_TO_PLAYER_LABELS_BASE_TILE + 1, NTADR_A(28, 6));
        }
      } else if (how_to_select_timer == 100) {
        how_to_select_timer = 0;
        const u8 released_button[] = {SELECT_BUTTON_BASE_TILE,
                                      SELECT_BUTTON_BASE_TILE + 1};
        multi_vram_buffer_horz(released_button, 2, NTADR_A(15, 5));
      }
      if (x_scroll != HOW_TO_SCROLL) {
        // TODO: easing
        x_scroll -= 16;
        set_scroll_x((u16)x_scroll);
        if (x_scroll == PALETTE_SWAP_POINT) {
          pal_col(0x11, 0x13);
          pal_col(0x13, 0x20);
        }
      }
      break;
    }

    s16 cursor_x;
    u8 cursor_y;
    if (state == State::ModeMenu) {
      cursor_x = MODE_MENU_CURSOR_X_POSITION;
      cursor_y = menu_y_position[(u8)current_game_mode];
    } else {
      cursor_x = MAIN_MENU_CURSOR_X_POSITION;
      cursor_y = menu_y_position[(u8)current_option];
    }
    banked_oam_meta_spr_horizontal(cursor_x - x_scroll, cursor_y,
                                   bobbing_flag ? metasprite_AvocadoHigh
                                                : metasprite_AvocadoLow);

    banked_oam_meta_spr_horizontal(JR_X_POSITION - x_scroll, JR_Y_POSITION,
                                   metasprite_TitleJR);

    banked_oam_meta_spr_horizontal(HOW_TO_LEFT_X_POSITION - x_scroll,
                                   HOW_TO_LEFT_Y_POSITION,
                                   metasprite_HowtoLeft);
    banked_oam_meta_spr_horizontal(HOW_TO_RIGHT_X_POSITION - x_scroll,
                                   HOW_TO_RIGHT_Y_POSITION,
                                   metasprite_HowtoRight);
    oam_hide_rest();
  }
}
