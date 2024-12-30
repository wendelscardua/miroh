#include <mapper.h>
#include <nesdoug.h>
#include <neslib.h>

#include "bank-helper.hpp"
#include "common.hpp"
#include "gameplay.hpp"
#include "ggsound.hpp"
#include "maze-defs.hpp"
#include "title-screen.hpp"

#include "soundtrack-ptr.hpp"
#include "world-map.hpp"

GameState current_game_state;
GameState previous_game_state;

GameMode current_game_mode;
ControllerScheme current_controller_scheme;
Stage current_stage;

Board board;

u16 high_score[NUM_STAGES];
bool story_completion[NUM_STAGES];
bool ending_triggered;
bool story_mode_beaten;
bool show_intro;

static void main_init() {
  previous_game_state = GameState::None;
  current_game_state = GameState::TitleScreen;
  current_game_mode = GameMode::Story;
  current_controller_scheme = ControllerScheme::OnePlayer;

  set_prg_bank(0);

  for (u8 i = 0; i < NUM_MAZES; i++) {
    high_score[i] = 0;
  }
  for (u8 i = 0; i < NUM_STAGES; i++) {
    story_completion[i] = false;
  }

  ending_triggered = false;

  show_intro = false;
  story_mode_beaten = false;

  ppu_off();

  // set 8x8 sprite mode
  oam_size(0);

  // Use lower half of PPU memory for background tiles
  bank_bg(0);
  // ... and upper half for sprites
  bank_spr(1);

  set_vram_buffer();

  {
    ScopedBank scopedBank(GET_BANK(song_list));
    GGSound::init(GGSound::Region::NTSC, song_list, sfx_list, instrument_list,
                  GET_BANK(song_list));
  }
}

int main() {
  main_init();

  while (true) {
    switch (current_game_state) {
    case GameState::TitleScreen: {
      ScopedBank bank(TitleScreen::BANK);
      TitleScreen title_screen;
      title_screen.loop();
    }; break;
    case GameState::Gameplay: {
      ScopedBank bank(Gameplay::BANK);
      Gameplay gameplay;
      gameplay.loop();
    }; break;
    case GameState::WorldMap: {
      ScopedBank bank(WorldMap::BANK);
      WorldMap world_map;
      world_map.loop();
    }; break;
    case GameState::None:
      break;
    }
  }
}
