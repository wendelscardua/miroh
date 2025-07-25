#include <mapper.h>
#include <nesdoug.h>
#include <neslib.h>

#include "assets.hpp"
#include "bank-helper.hpp"

#include "banked-asset-helpers.hpp"
#include "common.hpp"
#include "ggsound.hpp"
#include "metasprites.hpp"
#include "soundtrack.hpp"
#include "title-screen.hpp"

#pragma clang section text = ".prg_rom_2.text"
#pragma clang section rodata = ".prg_rom_2.rodata"

__attribute__((section(".prg_rom_2.rodata")))
const TitleScreen::MenuOption previous_option[] = {
    TitleScreen::MenuOption::HowToPlay,  // OnePlayer
    TitleScreen::MenuOption::OnePlayer,  // TwoPlayers
    TitleScreen::MenuOption::TwoPlayers, // HowToPlay
};

__attribute__((section(".prg_rom_2.rodata")))
const TitleScreen::MenuOption next_option[] = {
    TitleScreen::MenuOption::TwoPlayers, // OnePlayer
    TitleScreen::MenuOption::HowToPlay,  // TwoPlayers
    TitleScreen::MenuOption::OnePlayer,  // HowToPlay
};

__attribute__((section(".prg_rom_2.rodata"))) const u8 menu_y_position[] = {
    0x96, // OnePlayer
    0xa6, // TwoPlayers
    0xb6, // HowToPlay
};

__attribute__((section(".prg_rom_2.rodata"))) const GameMode previous_mode[] = {
    GameMode::TimeTrial, GameMode::Story, GameMode::Endless};

__attribute__((section(".prg_rom_2.rodata"))) const GameMode next_mode[] = {
    GameMode::Endless, GameMode::TimeTrial, GameMode::Story};

__attribute__((section(".prg_rom_2.rodata")))
const unsigned char story_label[12 * 1] = {0x02, 0x02, 0x15, 0x16, 0x11, 0x14,
                                           0x1b, 0x00, 0x00, 0x00, 0x00, 0x00};
__attribute__((section(".prg_rom_2.rodata")))
const unsigned char endless_label[12 * 1] = {
    0x00, 0x02, 0x08, 0x10, 0x07, 0x0e, 0x08, 0x15, 0x15, 0x00, 0x00, 0x00};
__attribute__((section(".prg_rom_2.rodata")))
const unsigned char time_trial_label[12 * 1] = {
    0x00, 0x00, 0x16, 0x0c, 0x0f, 0x08, 0x00, 0x16, 0x14, 0x0c, 0x04, 0x0e};

__attribute__((section(".prg_rom_2.rodata")))
const unsigned char bgm_test_labels[11 * 1] = {
    0x15, 0x12, 0x04, 0x06, 0x08, 0x09, 0x0e, 0x0c, 0x0a, 0x0b, 0x16};

__attribute__((
    section(".prg_rom_2.rodata"))) constexpr Song bgm_test_songs[] = {
    Song::Marshmallow_mountain,
    Song::Sting_plus_drums,
    Song::Intro_music,
    Song::Starlit_stables,
    Song::Rainbow_retreat,
    Song::Fairy_flight,
    Song::Glitter_grotto,
    Song::Baby_bullhead_title,
    Song::Ending,
    Song::Failure,
    Song::Victory};

static_assert(bgm_test_songs[0] == Song::Marshmallow_mountain);

__attribute__((noinline)) TitleScreen::TitleScreen()
    : state(State::MainMenu), current_option(MenuOption::OnePlayer),
      current_track(Song::Marshmallow_mountain), next_track_delay(0),
      y_scroll(TITLE_SCROLL) {
  banked_lambda(ASSETS_BANK, []() { load_title_assets(); });

  pal_bright(0);

  oam_clear();

  render_sprites();

  scroll(0, (u16)y_scroll);

  ppu_on_all();

  if (ending_triggered) {
    current_track = Song::Ending;
    bgm_test_index = 8;
  } else {
    bgm_test_index = 0;
    banked_play_song(current_track);
  }
  one_vram_buffer(bgm_test_labels[bgm_test_index], TRACK_ID_POSITION);

  pal_fade_to(0, 4);
}

__attribute__((noinline)) TitleScreen::~TitleScreen() {
  pal_fade_to(4, 0);
  ppu_off();
}

void TitleScreen::render_sprites() {
  bool bobbing_flag = get_frame_count() & 0b10000;

  u8 cursor_x;
  u8 cursor_y;
  if (state == State::ModeMenu) {
    cursor_x = MODE_MENU_CURSOR_X_POSITION;
    cursor_y = menu_y_position[(u8)current_game_mode];
  } else {
    cursor_x = MAIN_MENU_CURSOR_X_POSITION;
    cursor_y = menu_y_position[(u8)current_option];
  }
  banked_oam_meta_spr(METASPRITES_BANK, cursor_x, cursor_y - y_scroll,
                      bobbing_flag ? Metasprites::AvocadoHigh
                                   : Metasprites::AvocadoLow);

#ifdef NDEBUG
  banked_oam_meta_spr(METASPRITES_BANK, JR_X_POSITION, JR_Y_POSITION - y_scroll,
                      Metasprites::TitleJR);
#endif

  banked_oam_meta_spr(METASPRITES_BANK, HOW_TO_LEFT_X_POSITION,
                      HOW_TO_LEFT_Y_POSITION - y_scroll,
                      Metasprites::HowtoLeft);
  banked_oam_meta_spr(METASPRITES_BANK, HOW_TO_RIGHT_X_POSITION,
                      HOW_TO_RIGHT_Y_POSITION - y_scroll,
                      Metasprites::HowtoRight);
  oam_hide_rest();
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

    if (next_track_delay > 0) {
      next_track_delay--;
      if (next_track_delay == 0) {
        banked_play_song(current_track);
      }
    }

    switch (state) {
    case State::MainMenu:
      if (y_scroll != TITLE_SCROLL) {
        // TODO: easing
        y_scroll -= 16;
        if (y_scroll == 240)
          y_scroll = 224;
        set_scroll_y((u16)y_scroll);
        if (y_scroll == PALETTE_SWAP_POINT) {
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
        current_game_state = GameState::WorldMap;
        current_stage = Stage::StarlitStables;
        show_intro =
            (current_game_mode == GameMode::Story) && (!story_mode_beaten);
      }
      break;
    case State::HowToPlay:
      if (pressed & (PAD_B)) {
        state = State::MainMenu;
        banked_play_sfx(SFX::Uiabort, GGSound::SFXPriority::One);
      } else if (pressed & (PAD_LEFT | PAD_UP)) {
        if ((u8)bgm_test_index == 0) {
          bgm_test_index = sizeof(bgm_test_songs) - 1;
        } else {
          bgm_test_index--;
        }
        current_track = bgm_test_songs[bgm_test_index];
        next_track_delay = NEXT_TRACK_DELAY;
        GGSound::stop();
        banked_play_sfx(SFX::Uioptionscycle, GGSound::SFXPriority::One);
        one_vram_buffer(bgm_test_labels[bgm_test_index], TRACK_ID_POSITION);
      } else if (pressed & (PAD_RIGHT | PAD_DOWN | PAD_SELECT | PAD_A)) {
        if ((u8)bgm_test_index == sizeof(bgm_test_songs) - 1) {
          bgm_test_index = 0;
        } else {
          bgm_test_index++;
        }
        current_track = bgm_test_songs[bgm_test_index];
        next_track_delay = NEXT_TRACK_DELAY;
        GGSound::stop();
        banked_play_sfx(SFX::Uioptionscycle, GGSound::SFXPriority::One);
        one_vram_buffer(bgm_test_labels[bgm_test_index], TRACK_ID_POSITION);
      }
      how_to_select_timer++;
      if (how_to_select_timer == 90) {
        how_to_players_switched = !how_to_players_switched;
        static const u8 pressed_button[] = {SELECT_BUTTON_BASE_TILE + 2,
                                            SELECT_BUTTON_BASE_TILE + 3};
        multi_vram_buffer_horz(pressed_button, 2, NTADR_D(15, 5));
        if (how_to_players_switched) {
          one_vram_buffer(HOW_TO_PLAYER_LABELS_BASE_TILE + 1, NTADR_D(10, 6));
          one_vram_buffer(HOW_TO_PLAYER_LABELS_BASE_TILE, NTADR_D(28, 6));
        } else {
          one_vram_buffer(HOW_TO_PLAYER_LABELS_BASE_TILE, NTADR_D(10, 6));
          one_vram_buffer(HOW_TO_PLAYER_LABELS_BASE_TILE + 1, NTADR_D(28, 6));
        }
      } else if (how_to_select_timer == 100) {
        how_to_select_timer = 0;
        static const u8 released_button[] = {SELECT_BUTTON_BASE_TILE,
                                             SELECT_BUTTON_BASE_TILE + 1};
        multi_vram_buffer_horz(released_button, 2, NTADR_D(15, 5));
      }
      if (y_scroll != HOW_TO_SCROLL) {
        // TODO: easing
        y_scroll += 16;
        if (y_scroll == 240)
          y_scroll = 256;
        set_scroll_y((u16)y_scroll);
        if (y_scroll == PALETTE_SWAP_POINT) {
          pal_col(0x11, 0x13);
          pal_col(0x13, 0x20);
        }
      }
      break;
    }

    render_sprites();
  }
}
