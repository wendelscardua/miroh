#pragma once

#include "common.hpp"
#include "soundtrack.hpp"

#define DEFAULT_TEMPO (256 * 15)
#define STREAM_ACTIVE_SET  0b00000001
#define STREAM_PAUSE_SET   0b00000100
#define STREAM_PAUSE_CLEAR 0b11111011

#define MAX_STREAMS (6)
#define MAX_MUSIC_STREAMS (4)

namespace GGSound {
  struct Track {
    u16 ntsc_tempo;
    u16 pal_tempo;
    void *square_1_stream;
    void *square_2_stream;
    void *triangle_stream;
    void *noise_stream;
    void *dpcm_stream; // unused
  };

  enum class Region : u8 {
    NTSC = 0,
    PAL = 1,
    Dandy = 2,
  };

  enum class Channel : u8 {
    Square_1 = 0,
    Square_2 = 1,
    Triangle = 2,
    Noise = 3
  };

   // doubles as a "stream" index
  enum class SFXPriority : u8 {
    One = 4,
    Two = 5,
  };

  enum class Stream : u8 {
    Square_1 = 0,
    Square_2 = 1,
    Triangle = 2,
    Noise = 3,
    SFX1 = 4,
    SFX2 = 5
  };


  // Initialize sound engine
  void init(Region region,
            Track *song_list[],
            Track *sfx_list[],
            void *instruments[],
            u8 bank);

  // Kill all active streams and halt sound
  void stop();

  // Plays a song
  void play_song(Song song);

  // Plays a sound effect with a given priority
  void play_sfx(SFX sfx, SFXPriority priority);

  // Pauses a song
  void pause();

  // Resumes a song
  void resume();
}
