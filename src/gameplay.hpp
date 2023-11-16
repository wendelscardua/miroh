#pragma once

#include "board.hpp"
#include "fruits.hpp"
#include "polyomino.hpp"
#include "unicorn.hpp"

class Gameplay {
  enum class PauseOption : u8 { Retry, Resume, Exit };

  enum class GameplayState : u8 {
    Playing,
    Swapping,
    Paused,
    ConfirmExit,
    ConfirmRetry,
    ConfirmContinue,
    RetryOrExit,
    Retrying
  };

  // the semantic applies to player 1 (whichever it controls, p2 will control
  // the other)
  enum class InputMode { Unicorn, Polyomino };

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
  static const u8 BANK = 0;
  static const u16 INTRO_DELAY = 900;
  static const int DEFAULT_Y_SCROLL = 0x08;
  static const int PAUSE_SCROLL_Y = 0x050;
  static const int INTRO_SCROLL_Y = -0x100 + 0x50;
  static const int PAUSE_MENU_POSITION = NTADR_C(0, 3);
  static const int PAUSE_MENU_OPTIONS_POSITION = NTADR_C(0, 5);
  static const constexpr struct {
    bool display_unicorn : 1;
    bool display_polyomino : 1;
    u8 duration : 6;
  } swap_frames[] = {
      {false, true, 4}, {true, false, 4}, {false, true, 4},
      {true, false, 4}, {true, true, 1},
  };
  static const u8 TIME_TRIAL_DURATION = 90;
  static const u8 LINES_GOAL = 12;
  static const u8 SNACKS_GOAL = 24;
  static const u8 BLOCKS_GOAL = 60;
  static const u16 SCORE_GOAL = 200;
  Board &board;
  Unicorn unicorn;
  Polyomino polyomino;
  Fruits fruits;

  // sub-state for the gameplay state
  GameplayState gameplay_state;

  // it's actually the input mode for player 1; whatever
  // p1 controls, p2 controls the other
  InputMode input_mode;

  // buttons pressed by whoever controls the unicorn
  u8 unicorn_pressed;
  // buttons held by whoever controls the unicorn
  u8 unicorn_held;
  // buttons pressed by whoever controls the polyomino
  u8 polyomino_pressed;
  // buttons held by whoever controls the polyomino
  u8 polyomino_held;
  // buttons pressed by anyone
  u8 any_pressed;
  // buttons held by anyone
  u8 any_held;

  // countdown of frames for the swap flashing and sfx
  u8 swap_index : 4;
  u8 swap_frame_counter : 4;

  // Track current answer for a yes-or-no prompt
  bool yes_no_option;

  // Track current pause option
  PauseOption pause_option;

  int y_scroll;
  union {
    u16 goal_counter;
    struct {
      u8 time_trial_seconds;
      u8 time_trial_frames;
    };
    u8 lines_left;
    u8 snacks_left;
    u8 blocks_left;
    u16 points_left;
  };
  bool blocks_were_placed;
  bool failed_to_place;
  u8 lines_cleared;
  bool snack_was_eaten;

  Gameplay(Board &board);
  ~Gameplay();
  void loop();
  void add_experience(u16 exp);

private:
  void render();
  void pause_game();
  void end_game();
  void yes_no_cursor();
  void pause_handler();
  void gameplay_handler();
  void confirm_exit_handler();
  void confirm_retry_handler();
  void retry_exit_handler();
  void confirm_continue_handler();
  void initialize_goal();
  void game_mode_upkeep(bool stuff_in_progress);
  void swap_inputs();
};
