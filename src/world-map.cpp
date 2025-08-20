#include "world-map.hpp"
#include "assets.hpp"
#include "bank-helper.hpp"
#include "banked-asset-helpers.hpp"
#include "board.hpp"
#include "charset.hpp"
#include "common.hpp"
#include "ggsound.hpp"
#include "metasprites.hpp"
#include "soundtrack.hpp"
#include "zx02.hpp"
#include <nesdoug.h>
#include <neslib.h>

#pragma clang section text = ".prg_rom_3.text.world"
#pragma clang section rodata = ".prg_rom_3.rodata.world"

// NOTE: this screen uses different tiles to capital letters require literal hex
const char *stage_labels[] = {
    "\\\x{f0}tarlit_\\\x{f0}tables_____"_ts,
    "\\\x{f1}ainbow_\\\x{f1}etreat_____"_ts,
    "\\\x{6f}airy_\\\x{6f}orest________"_ts,
    "\\\x{f6}littery_\\\x{f6}rove______"_ts,
    "\\\x{f2}arshmallow_\\\x{f2}ountain"_ts,
};

const u8 stage_label_y[] = {
    0x3b, 0x53, 0x6b, 0x83, 0x9b,
};

const u8 *showcase_sprites[] = {(const u8 *)Metasprites::MirohMap,
                                (const u8 *)Metasprites::BerriesHigh,
                                (const u8 *)Metasprites::BlueCornHigh,
                                (const u8 *)Metasprites::BananasHigh,
                                (const u8 *)Metasprites::SweetPotatoHigh,
                                (const u8 *)Metasprites::UniLeftIdle,
                                (const u8 *)Metasprites::AppleHigh,
                                (const u8 *)Metasprites::CornHigh,
                                (const u8 *)Metasprites::PearHigh,
                                (const u8 *)Metasprites::AvocadoHigh,
                                (const u8 *)Metasprites::UniLeftCharge2,
                                (const u8 *)Metasprites::EggplantHigh,
                                (const u8 *)Metasprites::KiwiHigh,
                                (const u8 *)Metasprites::BroccoliHigh,
                                (const u8 *)Metasprites::GreenPeasHigh,
                                (const u8 *)Metasprites::UniLeftBreath1,
                                (const u8 *)Metasprites::StrawberryHigh,
                                (const u8 *)Metasprites::CherriesHigh,
                                (const u8 *)Metasprites::GrapesHigh,
                                (const u8 *)Metasprites::CucumberHigh,
                                (const u8 *)Metasprites::UniLeftSleep2,
                                (const u8 *)Metasprites::ClementineHigh,
                                (const u8 *)Metasprites::HallabongHigh,
                                (const u8 *)Metasprites::CarrotHigh,
                                (const u8 *)Metasprites::BerriesHigh};

WorldMap::WorldMap() {
  load_map_assets();

  vram_adr(NAMETABLE_A);

  pal_bright(0);

  scroll(0, 0);

  story_mode_beaten = false;

  switch (current_game_mode) {
  case GameMode::Story:
    // MM is disabled until all others are beaten
    available_stages[NUM_STAGES - 1] = true;
    for (u8 i = 0; i < NUM_STAGES - 1; i++) {
      available_stages[i] = !story_completion[i];
      if (available_stages[i]) {
        available_stages[NUM_STAGES - 1] = false;
      }
    }

    story_mode_beaten = story_completion[NUM_STAGES - 1];

    break;
  case GameMode::Endless:
  case GameMode::TimeTrial:
    for (u8 i = 0; i < NUM_STAGES - 1; i++) {
      available_stages[i] = true;
    }
    // MM is disabled until it's beaten on story mode
    available_stages[NUM_STAGES - 1] = story_completion[NUM_STAGES - 1];
    break;
  }

  u16 position = NTADR_A(9, 8);
  for (u8 i = 0; i < NUM_STAGES; i++) {
    if (available_stages[i]) {
      vram_adr(position);
      vram_write(stage_labels[i], 20);
    }
    position += 0x60;
  }

  if (!story_mode_beaten) {
    for (u8 i = (u8)current_stage; i < NUM_STAGES; i++) {
      if (available_stages[i]) {
        current_stage = (Stage)i;
        break;
      }
    }

    if (!available_stages[(u8)current_stage]) {
      for (u8 i = 0; i < NUM_STAGES; i++) {
        if (available_stages[i]) {
          current_stage = (Stage)i;
          break;
        }
      }
    }
  }

  oam_clear();

  show_intro = (current_game_mode == GameMode::Story) && (!story_mode_beaten);

  if (show_intro) {
    vram_adr(NAMETABLE_C);
    zx02_decompress_to_vram((void *)intro_text_nametable, NAMETABLE_C);
    scroll(0, 0xf0);
  } else if (story_mode_beaten) {
    vram_adr(NAMETABLE_C);
    zx02_decompress_to_vram((void *)ending_text_nametable, NAMETABLE_C);
    scroll(0, 0xf0);
  } else {
    render_sprites();
  }

  change_uni_palette();

  ppu_on_all();

  GGSound::play_song(story_mode_beaten ? Song::Baby_bullhead_title
                                       : Song::Intro_music);

  pal_fade_to(0, 4);
}

WorldMap::~WorldMap() {
  pal_fade_to(4, 0);
  ppu_off();
}

void WorldMap::stage_change(Stage new_stage) {
  if (new_stage == current_stage) {
    GGSound::play_sfx(SFX::Uiabort, GGSound::SFXPriority::Two);
  } else {
    current_stage = new_stage;
    GGSound::play_sfx(SFX::Uioptionscycle, GGSound::SFXPriority::Two);
    change_uni_palette();
  }
}

void WorldMap::tick_ending() {
  ending_frame_counter++;
  if (ending_frame_counter >= 96) {
    ending_frame_counter = 0;
  }
}

void WorldMap::ending_cutscene() {
  u8 ending_sprite = 0;
  u8 ending_palette_counter = 0;
  u8 ending_palette = 4;

  oam_hide_rest();

  u8 temp[32];

  for (u8 i = 0; i < 32; i++) {
    temp[i] = 0;
  }
  for (u8 y = 9; y <= 18; y++) {
    ppu_wait_nmi();
    multi_vram_buffer_horz(temp, 32, NTADR_C(0, y));
  }

  GGSound::play_song(Song::Ending);

  do {
    ppu_wait_nmi();
    pad_poll(0);
    pad_poll(1);

    tick_ending();

    banked_oam_meta_spr_horizontal(0x78, 0x80, showcase_sprites[ending_sprite]);
    oam_hide_rest();

    if (ending_frame_counter == 0) {
      ending_sprite++;
      if (ending_sprite == 25) {
        ending_sprite = 0;
      }
      ending_palette_counter++;
      if (ending_palette_counter == 5) {
        ending_palette_counter = 0;
        ending_palette++;
        if (ending_palette == 5) {
          ending_palette = 0;
        }
        for (u8 i = 0; i < 16; i++) {
          pal_col(0x10 | i,
                  i == 9 ? 0x22 : level_spr_palettes[(u8)ending_palette][i]);
        }
      }
    }

  } while (get_pad_new(0) == 0 && get_pad_new(1) == 0);
}

void WorldMap::loop() {
  while (current_game_state == GameState::WorldMap) {
    ppu_wait_nmi();

    pad_poll(0);
    pad_poll(1);

    rand16();

    u8 pressed = get_pad_new(0) | get_pad_new(1);

    if (pressed & (PAD_A | PAD_START)) {
      GGSound::play_sfx(SFX::Uiconfirm, GGSound::SFXPriority::One);
      if (show_intro) {
        show_intro = false;
        scroll(0, 0);
        continue;
      } else if (story_mode_beaten) {
        ending_cutscene();
        current_game_state = GameState::TitleScreen;
        return;
      } else {
        current_game_state = GameState::Gameplay;
        banked_lambda(Board::BANK, []() { board.generate_maze(); });
      }
    } else if (pressed & (PAD_B)) {
      current_game_state = GameState::TitleScreen;
    } else if (!story_mode_beaten && (pressed & (PAD_UP | PAD_LEFT))) {
      Stage new_stage = current_stage;

      if ((u8)current_stage > 0) {
        for (s8 i = (s8)current_stage - 1; i >= 0; i--) {
          if (available_stages[i]) {
            new_stage = (Stage)i;
            break;
          }
        }
      }
      stage_change(new_stage);
    } else if (!story_mode_beaten &&
               (pressed & (PAD_DOWN | PAD_RIGHT | PAD_SELECT))) {
      Stage new_stage = current_stage;

      if ((u8)current_stage < NUM_STAGES - 1) {
        for (s8 i = (s8)current_stage + 1; i < NUM_STAGES; i++) {
          if (available_stages[i]) {
            new_stage = (Stage)i;
            break;
          }
        }
      }
      stage_change(new_stage);
    }

    if (!show_intro && !story_mode_beaten) {
      render_sprites();
    }
  }
}

void WorldMap::render_sprites() {
  banked_oam_meta_spr(METASPRITES_BANK, 0x35, stage_label_y[(u8)current_stage],
                      current_game_mode == GameMode::Story &&
                              available_stages[NUM_STAGES - 1]
                          ? Metasprites::MirohMap
                          : Metasprites::UniMap);
  oam_hide_rest();
}
