#pragma once

#include "common.hpp"
#include "soundtrack.hpp"

// #define FEATURE_DPCM

namespace GGSound {
  constexpr u8 BANK = 2;

  struct Track {
    u16 ntsc_tempo;
    u16 pal_tempo;
    void *square_1_stream;
    void *square_2_stream;
    void *triangle_stream;
    void *noise_stream;
    void *dpcm_stream;
  };

  enum class Region : u8 {
    NTSC = 0,
    PAL = 1,
    Dandy = 2,
  };

  enum class Channel : u8 {
    Square_1,
    Square_2,
    Triangle,
    Noise,
#ifdef FEATURE_DPCM
    DPCM,
#endif
  };

  enum class Stream : u8 {
    Square_1,
    Square_2,
    Triangle,
    Noise,
#ifdef FEATURE_DPCM
    DPCM,
#endif
    SFX1,
    SFX2
  };

  // doubles as a "stream" index
  enum class SFXPriority : u8 {
    One = (u8)Stream::SFX1,
    Two = (u8)Stream::SFX2,
  };

  // Initialize sound engine
  __attribute__((noinline)) void init(Region region, const Track *song_list[],
                                      const Track *sfx_list[],
                                      const void *instruments[]
#ifdef FEATURE_DPCM
                                      ,
                                      const void *dpcm_pointers[]
#endif
  );

  // Kill all active streams and halt sound
  __attribute__((noinline)) void stop();

  // Plays a song
  __attribute__((noinline)) void play_song(Song song);

  // Plays a sound effect with a given priority
  __attribute__((noinline)) void play_sfx(SFX sfx, SFXPriority priority);

  // Pauses a song
  __attribute__((noinline)) void pause();

  // Resumes a song
  __attribute__((noinline)) void resume();
} // namespace GGSound
