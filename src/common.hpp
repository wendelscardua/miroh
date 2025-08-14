#pragma once

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

enum class GameState : u8 { TitleScreen, WorldMap, Gameplay, None = 0xff };

enum class GameMode : u8 {
  Story,
  Endless,
  TimeTrial,
};

enum class ControllerScheme : u8 {
  OnePlayer,
  TwoPlayers,
};

enum class SelectReminder : u8 {
  NeedToRemind,         // the single player game barely started
  WaitingBlockToRemind, // wait for the second block to spawn
  WaitingRowToRemind,   // wait for the second block to drop a little
  Reminding,            // remind the player by showing "select" sprite
  Reminded,             // don't show the "select" sprite anymore
};

constexpr u8 NUM_STAGES = 5;

enum class Stage : u8 {
  StarlitStables,
  RainbowRetreat,
  FairyForest,
  GlitteryGrotto,
  MarshmallowMountain
};

extern GameState current_game_state;
extern GameMode current_game_mode;
extern ControllerScheme current_controller_scheme;
extern Stage current_stage;

class Board;
extern Board board;
extern bool story_completion[];
extern u16 high_score[];
extern bool ending_triggered;
extern SelectReminder select_reminder;

// neslib/nesdoug internal stuff

extern "C" char OAM_BUF[256];
extern "C" char SPRID;

extern u8 VRAM_INDEX;
extern char VRAM_BUF[256];

typedef union {
  struct {
    s8 x;
    s8 y;
    u8 tile;
    u8 attribute;
  } spr;
  u8 terminator;
} Sprite;