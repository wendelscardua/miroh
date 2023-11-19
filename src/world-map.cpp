#include "world-map.hpp"
#include "assets.hpp"
#include "bank-helper.hpp"
#include "banked-asset-helpers.hpp"
#include "common.hpp"
#include "metasprites.hpp"
#include "soundtrack.hpp"
#include <nesdoug.h>
#include <neslib.h>

#pragma clang section text = ".prg_rom_2.text"
#pragma clang section rodata = ".prg_rom_2.rodata"

const unsigned char stage_labels[][20] = {
    {0x15, 0x16, 0x04, 0x14, 0x0e, 0x0c, 0x16, 0x02, 0x15, 0x16,
     0x04, 0x05, 0x0e, 0x08, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x14, 0x04, 0x0c, 0x10, 0x05, 0x11, 0x19, 0x02, 0x14, 0x08,
     0x16, 0x14, 0x08, 0x04, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x09, 0x04, 0x0c, 0x14, 0x1b, 0x02, 0x09, 0x11, 0x14, 0x08,
     0x15, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x0a, 0x0e, 0x0c, 0x16, 0x16, 0x08, 0x14, 0x1b, 0x02, 0x0a,
     0x14, 0x11, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x0f, 0x04, 0x14, 0x15, 0x0b, 0x0f, 0x04, 0x0e, 0x0e, 0x11,
     0x19, 0x02, 0x0f, 0x11, 0x17, 0x10, 0x16, 0x04, 0x0c, 0x10}};

const unsigned char thanks_text[19 * 1] = {
    0x8c, 0x0b, 0x04, 0x10, 0x0d, 0x15, 0x02, 0x09, 0x11, 0x14,
    0x02, 0x12, 0x0e, 0x04, 0x1b, 0x0c, 0x10, 0x0a, 0x2f};

const unsigned char soon_text[25 * 1] = {
    0x6f, 0x17, 0x0e, 0x0e, 0x02, 0x18, 0x08, 0x14, 0x15,
    0x0c, 0x11, 0x10, 0x02, 0x06, 0x11, 0x0f, 0x0c, 0x10,
    0x0a, 0x02, 0x15, 0x11, 0x11, 0x10, 0x2f};

const unsigned char erase_text[20 * 1] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const u8 stage_label_y[] = {
    0x3b, 0x53, 0x6b, 0x83, 0x9b,
};

WorldMap::WorldMap(Board &board) : board(board) {
  banked_lambda(ASSETS_BANK, []() { load_map_assets(); });

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

  render_sprites();

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

void WorldMap::loop() {
  u8 ending_cutscene_counter = 0;
  while (current_game_state == GameState::WorldMap) {
    ppu_wait_nmi();

    if (ending_cutscene_counter > 0) {
      ending_cutscene_counter--;
      if (ending_cutscene_counter == 0) {
        current_game_state = GameState::TitleScreen;
        break;
      }
    }

    pad_poll(0);
    pad_poll(1);

    rand16();

    u8 pressed = get_pad_new(0) | get_pad_new(1);

    if (pressed & (PAD_A | PAD_START)) {
      banked_play_sfx(SFX::Uiconfirm, GGSound::SFXPriority::One);
      if (current_stage == Stage::MarshmallowMountain) {
        // TODO: change this when we have MM
        ending_triggered = true;
        ending_cutscene_counter = 240;
        banked_play_song(Song::Ending);
        multi_vram_buffer_horz(erase_text, sizeof(erase_text), NTADR_A(9, 20));
        multi_vram_buffer_horz(thanks_text, sizeof(thanks_text),
                               NTADR_A(7, 24));
        multi_vram_buffer_horz(soon_text, sizeof(soon_text), NTADR_A(4, 26));
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
