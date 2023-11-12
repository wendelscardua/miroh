#pragma once

#include "assets.hpp"
#include "board.hpp"
#include "fruits.hpp"
#include "input-mode.hpp"
#include "player.hpp"
#include "polyomino.hpp"

class Gameplay {
  enum class PauseOption : u8 { Retry, Resume, Exit };

  enum class GameplayState : u8 {
    Playing,
    Paused,
    ConfirmExit,
    ConfirmRetry,
    ConfirmContinue,
  };

  // we level up every 50 points
  static constexpr u16 LEVEL_UP_POINTS = 50;

  static constexpr u8 MAX_LEVEL = 9;

  static constexpr u16 SPAWN_DELAY_PER_LEVEL[] = {
      150, // 0
      135, // 1
      135, // 2
      120, // 3
      120, // 4
      105, // 5
      105, // 6
      90,  // 7
      75,  // 8
      60   // 9
  };

  static constexpr u8 DROP_FRAMES_PER_LEVEL[] = {
      60, // 0
      54, // 1
      48, // 2
      42, // 3
      36, // 4
      30, // 5
      24, // 6
      18, // 7
      12, // 8
      6   // 9
  };

  u16 experience;
  u8 current_level;
  u16 spawn_timer;

public:
  static constexpr u8 BANK = 0;
  static constexpr u16 INTRO_DELAY = 900;
  static constexpr int DEFAULT_Y_SCROLL = 0x08;
  static constexpr int PAUSE_SCROLL_Y = 0x050;
  static constexpr int INTRO_SCROLL_Y = -0x100 + 0x50;
  Board &board;
  Player player;
  Polyomino polyomino;
  Fruits fruits;
  GameplayState gameplay_state;
  InputMode input_mode;
  int y_scroll;

  Gameplay(Board &board);
  ~Gameplay();
  void loop();
  void add_experience(u16 exp);

private:
  void render();
  void pause_handler(PauseOption &pause_option);
  void gameplay_handler();
};
