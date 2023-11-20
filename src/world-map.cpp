#include "world-map.hpp"
#include "assets.hpp"
#include "bank-helper.hpp"
#include "banked-asset-helpers.hpp"
#include "common.hpp"
#include "donut.hpp"
#include "metasprites.hpp"
#include "soundtrack.hpp"
#include "zx02.hpp"
#include <nesdoug.h>
#include <neslib.h>

#pragma clang section text = ".prg_rom_2.text"
#pragma clang section rodata = ".prg_rom_2.rodata"

const unsigned char stage_labels[][20] = {
    {0xf0, 0x16, 0x04, 0x14, 0x0e, 0x0c, 0x16, 0x02, 0xf0, 0x16,
     0x04, 0x05, 0x0e, 0x08, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xf1, 0x04, 0x0c, 0x10, 0x05, 0x11, 0x19, 0x02, 0xf1, 0x08,
     0x16, 0x14, 0x08, 0x04, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x6f, 0x04, 0x0c, 0x14, 0x1b, 0x02, 0x6f, 0x11, 0x14, 0x08,
     0x15, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xf6, 0x0e, 0x0c, 0x16, 0x16, 0x08, 0x14, 0x1b, 0x02, 0xf6,
     0x14, 0x11, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xf2, 0x04, 0x14, 0x15, 0x0b, 0x0f, 0x04, 0x0e, 0x0e, 0x11,
     0x19, 0x02, 0xf2, 0x11, 0x17, 0x10, 0x16, 0x04, 0x0c, 0x10}};

const u8 stage_label_y[] = {
    0x3b, 0x53, 0x6b, 0x83, 0x9b,
};

const u8 *showcase_sprites[] = {(const u8 *)metasprite_MirohMap,
                                (const u8 *)metasprite_BerriesHigh,
                                (const u8 *)metasprite_BlueCornHigh,
                                (const u8 *)metasprite_BananasHigh,
                                (const u8 *)metasprite_SweetPotatoHigh,
                                (const u8 *)metasprite_UniLeftIdle,
                                (const u8 *)metasprite_AppleHigh,
                                (const u8 *)metasprite_CornHigh,
                                (const u8 *)metasprite_PearHigh,
                                (const u8 *)metasprite_AvocadoHigh,
                                (const u8 *)metasprite_UniLeftCharge2,
                                (const u8 *)metasprite_EggplantHigh,
                                (const u8 *)metasprite_KiwiHigh,
                                (const u8 *)metasprite_BroccoliHigh,
                                (const u8 *)metasprite_GreenPeasHigh,
                                (const u8 *)metasprite_UniLeftBreath1,
                                (const u8 *)metasprite_StrawberryHigh,
                                (const u8 *)metasprite_CherriesHigh,
                                (const u8 *)metasprite_GrapesHigh,
                                (const u8 *)metasprite_CucumberHigh,
                                (const u8 *)metasprite_UniLeftSleep2,
                                (const u8 *)metasprite_ClementineHigh,
                                (const u8 *)metasprite_HallabongHigh,
                                (const u8 *)metasprite_CarrotHigh,
                                (const u8 *)metasprite_BerriesHigh};

WorldMap::WorldMap(Board &board) : board(board) {
  set_mirroring(MIRROR_VERTICAL);

  banked_lambda(ASSETS_BANK, []() { load_map_assets(); });

  vram_adr(NAMETABLE_A);

  pal_bright(0);

  scroll(0, 0);

  story_mode_beaten = false;

  switch (current_game_mode) {
  case GameMode::Story:
    // MM is hidden until all others are beaten
    story_mode_beaten = true;
    available_stages[NUM_STAGES - 1] = true;
    for (u8 i = 0; i < NUM_STAGES - 1; i++) {
      if ((available_stages[i] = !story_completion[i])) {
        available_stages[NUM_STAGES - 1] = story_mode_beaten = false;
      }
    }
    break;
  case GameMode::Endless:
  case GameMode::TimeTrial:
    for (u8 i = 0; i < NUM_STAGES - 1; i++) {
      available_stages[i] = true;
    }
    // TODO: change this whenever we add MM
    available_stages[NUM_STAGES - 1] = false;
    break;
  }

  u16 position = NTADR_A(9, 8);
  for (u8 i = 0; i < NUM_STAGES - 1; i++) {
    if (available_stages[i]) {
      vram_adr(position);
      vram_write(stage_labels[i], sizeof(stage_labels[i]));
    }
    position += 0x60;
  }
  vram_adr(position);
  vram_write(stage_labels[(u8)Stage::MarshmallowMountain],
             sizeof(stage_labels[(u8)Stage::MarshmallowMountain]));

  if (story_mode_beaten) {
    current_stage = Stage::MarshmallowMountain;
  } else {
    for (u8 i = (u8)current_stage; i < NUM_STAGES - 1; i++) {
      if (available_stages[i]) {
        current_stage = (Stage)i;
        break;
      }
    }

    if (!available_stages[(u8)current_stage]) {
      for (u8 i = 0; i < NUM_STAGES - 1; i++) {
        if (available_stages[i]) {
          current_stage = (Stage)i;
          break;
        }
      }
    }
  }

  oam_clear();

  if (show_intro) {
    vram_adr(NAMETABLE_B);
    zx02_decompress_to_vram((void *)intro_text_nametable, NAMETABLE_B);
    scroll(0x100, 0);
  } else {
    render_sprites();
  }
  if (story_mode_beaten) {
    vram_adr(NAMETABLE_B);
    zx02_decompress_to_vram((void *)ending_text_nametable, NAMETABLE_B);
  }
  change_uni_palette();

  ppu_on_all();

  banked_play_song(story_mode_beaten ? Song::Baby_bullhead_title
                                     : Song::Intro_music);

  ending_triggered = false;

  pal_fade_to(0, 4);
}

WorldMap::~WorldMap() {
  pal_fade_to(4, 0);
  ppu_off();
}

void WorldMap::stage_change(Stage new_stage) {
  if (new_stage == current_stage) {
    banked_play_sfx(SFX::Uiabort, GGSound::SFXPriority::Two);
  } else {
    current_stage = new_stage;
    banked_play_sfx(SFX::Uioptionscycle, GGSound::SFXPriority::Two);
    change_uni_palette();
  }
}

void WorldMap::tick_ending() {
  ending_frame_counter++;
  if (ending_frame_counter >= 96) {
    ending_frame_counter = 0;
  }
}

void WorldMap::loop() {
  u8 ending_sprite = 0;
  u8 ending_palette_counter = 0;
  u8 ending_palette = 4;
  while (current_game_state == GameState::WorldMap) {
    ppu_wait_nmi();

    pad_poll(0);
    pad_poll(1);

    rand16();

    u8 pressed = get_pad_new(0) | get_pad_new(1);

    if (show_intro) {
      if (pressed) {
        show_intro = false;
        scroll(0x0, 0);
      }
      continue;
    }

    if (pressed & (PAD_A | PAD_START)) {
      banked_play_sfx(SFX::Uiconfirm, GGSound::SFXPriority::One);
      if (current_stage == Stage::MarshmallowMountain) {
        // TODO: change this when we have MM
        oam_hide_rest();

        scroll(0x100, 0);
        do {
          ppu_wait_nmi();
          pad_poll(0);
          pad_poll(1);
        } while (get_pad_new(0) == 0 && get_pad_new(1) == 0);

        for (u8 i = 0; i < 32; i++) {
          donut_block_buffer[i] = 0;
        }
        for (u8 y = 9; y <= 18; y++) {
          ppu_wait_nmi();
          multi_vram_buffer_horz(donut_block_buffer, 32, NTADR_B(0, y));
        }

        ending_triggered = true;
        banked_play_song(Song::Ending);

        do {
          ppu_wait_nmi();
          pad_poll(0);
          pad_poll(1);

          tick_ending();

          banked_oam_meta_spr_horizontal(0x78, 0x80,
                                         showcase_sprites[ending_sprite]);
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
                        i == 9 ? 0x22
                               : level_spr_palettes[(u8)ending_palette][i]);
              }
            }
          }

        } while (get_pad_new(0) == 0 && get_pad_new(1) == 0);

        current_game_state = GameState::TitleScreen;
        return;
      } else {
        current_game_state = GameState::Gameplay;
        banked_lambda(Board::MAZE_BANK, [this]() { board.generate_maze(); });
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

      // TODO: fix these offsets when MM stage comes back
      if ((u8)current_stage < NUM_STAGES - 2) {
        for (s8 i = (s8)current_stage + 1; i < NUM_STAGES - 1; i++) {
          if (available_stages[i]) {
            new_stage = (Stage)i;
            break;
          }
        }
      }
      stage_change(new_stage);
    }

    render_sprites();
  }
}

void WorldMap::render_sprites() {
  if (ending_triggered) {
    // TODO: maybe showcase sprites?
  } else {
    banked_oam_meta_spr(0x35, stage_label_y[(u8)current_stage],
                        story_mode_beaten ? metasprite_MirohMap
                                          : metasprite_UniMap);
  }
  oam_hide_rest();
}
