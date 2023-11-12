#pragma once

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

enum class GameState : u8 { TitleScreen, Gameplay, None = 0xff };

enum class GameMode : u8 {
  Story,
  Endless,
  TimeTrial,
};

enum class ControllerScheme : u8 {
  OnePlayer,
  TwoPlayers,
};

extern GameState current_game_state;
extern GameMode current_game_mode;
extern ControllerScheme current_controller_scheme;