#include "ggsound.hpp"
#include <bank.h>
#include <peekpoke.h>

#pragma clang section text = ".prg_rom_2.ggsound.text"
#pragma clang section rodata = ".prg_rom_2.ggsound.rodata"
namespace GGSound {

  extern "C" bool _ggsound_sound_disable_update;
  extern "C" void **_ggsound_base_address_instruments;
  extern "C" const u8 *_ggsound_base_address_note_table_lo;
  extern "C" const u8 *_ggsound_base_address_note_table_hi;
  extern "C" bool _ggsound_apu_data_ready;
  extern "C" u8 _ggsound_apu_register_sets[20];
  extern "C" u8 _ggsound_apu_square_1_old;
  extern "C" u8 _ggsound_apu_square_2_old;
  extern "C" u8 _ggsound_sound_bank;
  extern "C" u8 _ggsound_stream_flags[MAX_STREAMS];
  extern "C" u8 _ggsound_stream_tempo_lo[MAX_STREAMS];
  extern "C" u8 _ggsound_stream_tempo_hi[MAX_STREAMS];
  extern "C" u8 _ggsound_stream_tempo_counter_lo[MAX_STREAMS];
  extern "C" u8 _ggsound_stream_tempo_counter_hi[MAX_STREAMS];
  extern "C" u8 _ggsound_stream_note_length_lo[MAX_STREAMS];
  extern "C" u8 _ggsound_stream_note_length_hi[MAX_STREAMS];
  extern "C" u8 _ggsound_stream_note_length_counter_lo[MAX_STREAMS];
  extern "C" u8 _ggsound_stream_note_length_counter_hi[MAX_STREAMS];
  extern "C" u8 _ggsound_stream_instrument_index[MAX_STREAMS];
  extern "C" u8 _ggsound_stream_volume_offset[MAX_STREAMS];
  extern "C" u8 _ggsound_stream_pitch_offset[MAX_STREAMS];
  extern "C" u8 _ggsound_stream_duty_offset[MAX_STREAMS];
#ifdef FEATURE_ARPEGGIOS
  extern "C" u8 _ggsound_stream_arpeggio_offset[MAX_STREAMS];
#endif
  extern "C" Channel _ggsound_stream_channel[MAX_STREAMS];
  extern "C" u8 _ggsound_stream_read_address_lo[MAX_STREAMS];
  extern "C" u8 _ggsound_stream_read_address_hi[MAX_STREAMS];
  extern "C" u8 _ggsound_stream_channel_register_1[MAX_STREAMS];

  Region region;
  Track **song_list;
  Track **sfx_list;

  // Note table borrowed from periods.s provided by FamiTracker's NSF driver.
  // and then from ggsound.s

  const u8 ntsc_note_table_lo[] = {
      (u8)0x0D5B, (u8)0x0C9C, (u8)0x0BE6, (u8)0x0B3B, (u8)0x0A9A, (u8)0x0A01,
      (u8)0x0972, (u8)0x08EA, (u8)0x086A, (u8)0x07F1, (u8)0x077F, (u8)0x0713,
      (u8)0x06AD, (u8)0x064D, (u8)0x05F3, (u8)0x059D, (u8)0x054C, (u8)0x0500,
      (u8)0x04B8, (u8)0x0474, (u8)0x0434, (u8)0x03F8, (u8)0x03BF, (u8)0x0389,
      (u8)0x0356, (u8)0x0326, (u8)0x02F9, (u8)0x02CE, (u8)0x02A6, (u8)0x0280,
      (u8)0x025C, (u8)0x023A, (u8)0x021A, (u8)0x01FB, (u8)0x01DF, (u8)0x01C4,
      (u8)0x01AB, (u8)0x0193, (u8)0x017C, (u8)0x0167, (u8)0x0152, (u8)0x013F,
      (u8)0x012D, (u8)0x011C, (u8)0x010C, (u8)0x00FD, (u8)0x00EF, (u8)0x00E1,
      (u8)0x00D5, (u8)0x00C9, (u8)0x00BD, (u8)0x00B3, (u8)0x00A9, (u8)0x009F,
      (u8)0x0096, (u8)0x008E, (u8)0x0086, (u8)0x007E, (u8)0x0077, (u8)0x0070,
      (u8)0x006A, (u8)0x0064, (u8)0x005E, (u8)0x0059, (u8)0x0054, (u8)0x004F,
      (u8)0x004B, (u8)0x0046, (u8)0x0042, (u8)0x003F, (u8)0x003B, (u8)0x0038,
      (u8)0x0034, (u8)0x0031, (u8)0x002F, (u8)0x002C, (u8)0x0029, (u8)0x0027,
      (u8)0x0025, (u8)0x0023, (u8)0x0021, (u8)0x001F, (u8)0x001D, (u8)0x001B,
      (u8)0x001A, (u8)0x0018, (u8)0x0017, (u8)0x0015, (u8)0x0014, (u8)0x0013,
      (u8)0x0012, (u8)0x0011, (u8)0x0010, (u8)0x000F, (u8)0x000E, (u8)0x000D};

  const u8 ntsc_note_table_hi[] = {
      (u8)(0x0D5B >> 8), (u8)(0x0C9C >> 8), (u8)(0x0BE6 >> 8),
      (u8)(0x0B3B >> 8), (u8)(0x0A9A >> 8), (u8)(0x0A01 >> 8),
      (u8)(0x0972 >> 8), (u8)(0x08EA >> 8), (u8)(0x086A >> 8),
      (u8)(0x07F1 >> 8), (u8)(0x077F >> 8), (u8)(0x0713 >> 8),
      (u8)(0x06AD >> 8), (u8)(0x064D >> 8), (u8)(0x05F3 >> 8),
      (u8)(0x059D >> 8), (u8)(0x054C >> 8), (u8)(0x0500 >> 8),
      (u8)(0x04B8 >> 8), (u8)(0x0474 >> 8), (u8)(0x0434 >> 8),
      (u8)(0x03F8 >> 8), (u8)(0x03BF >> 8), (u8)(0x0389 >> 8),
      (u8)(0x0356 >> 8), (u8)(0x0326 >> 8), (u8)(0x02F9 >> 8),
      (u8)(0x02CE >> 8), (u8)(0x02A6 >> 8), (u8)(0x0280 >> 8),
      (u8)(0x025C >> 8), (u8)(0x023A >> 8), (u8)(0x021A >> 8),
      (u8)(0x01FB >> 8), (u8)(0x01DF >> 8), (u8)(0x01C4 >> 8),
      (u8)(0x01AB >> 8), (u8)(0x0193 >> 8), (u8)(0x017C >> 8),
      (u8)(0x0167 >> 8), (u8)(0x0152 >> 8), (u8)(0x013F >> 8),
      (u8)(0x012D >> 8), (u8)(0x011C >> 8), (u8)(0x010C >> 8),
      (u8)(0x00FD >> 8), (u8)(0x00EF >> 8), (u8)(0x00E1 >> 8),
      (u8)(0x00D5 >> 8), (u8)(0x00C9 >> 8), (u8)(0x00BD >> 8),
      (u8)(0x00B3 >> 8), (u8)(0x00A9 >> 8), (u8)(0x009F >> 8),
      (u8)(0x0096 >> 8), (u8)(0x008E >> 8), (u8)(0x0086 >> 8),
      (u8)(0x007E >> 8), (u8)(0x0077 >> 8), (u8)(0x0070 >> 8),
      (u8)(0x006A >> 8), (u8)(0x0064 >> 8), (u8)(0x005E >> 8),
      (u8)(0x0059 >> 8), (u8)(0x0054 >> 8), (u8)(0x004F >> 8),
      (u8)(0x004B >> 8), (u8)(0x0046 >> 8), (u8)(0x0042 >> 8),
      (u8)(0x003F >> 8), (u8)(0x003B >> 8), (u8)(0x0038 >> 8),
      (u8)(0x0034 >> 8), (u8)(0x0031 >> 8), (u8)(0x002F >> 8),
      (u8)(0x002C >> 8), (u8)(0x0029 >> 8), (u8)(0x0027 >> 8),
      (u8)(0x0025 >> 8), (u8)(0x0023 >> 8), (u8)(0x0021 >> 8),
      (u8)(0x001F >> 8), (u8)(0x001D >> 8), (u8)(0x001B >> 8),
      (u8)(0x001A >> 8), (u8)(0x0018 >> 8), (u8)(0x0017 >> 8),
      (u8)(0x0015 >> 8), (u8)(0x0014 >> 8), (u8)(0x0013 >> 8),
      (u8)(0x0012 >> 8), (u8)(0x0011 >> 8), (u8)(0x0010 >> 8),
      (u8)(0x000F >> 8), (u8)(0x000E >> 8), (u8)0x000D};

  const u8 pal_note_table_lo[] = {
      (u8)0x0C68, (u8)0x0BB6, (u8)0x0B0E, (u8)0x0A6F, (u8)0x09D9, (u8)0x094B,
      (u8)0x08C6, (u8)0x0848, (u8)0x07D1, (u8)0x0760, (u8)0x06F6, (u8)0x0692,
      (u8)0x0634, (u8)0x05DB, (u8)0x0586, (u8)0x0537, (u8)0x04EC, (u8)0x04A5,
      (u8)0x0462, (u8)0x0423, (u8)0x03E8, (u8)0x03B0, (u8)0x037B, (u8)0x0349,
      (u8)0x0319, (u8)0x02ED, (u8)0x02C3, (u8)0x029B, (u8)0x0275, (u8)0x0252,
      (u8)0x0231, (u8)0x0211, (u8)0x01F3, (u8)0x01D7, (u8)0x01BD, (u8)0x01A4,
      (u8)0x018C, (u8)0x0176, (u8)0x0161, (u8)0x014D, (u8)0x013A, (u8)0x0129,
      (u8)0x0118, (u8)0x0108, (u8)0x00F9, (u8)0x00EB, (u8)0x00DE, (u8)0x00D1,
      (u8)0x00C6, (u8)0x00BA, (u8)0x00B0, (u8)0x00A6, (u8)0x009D, (u8)0x0094,
      (u8)0x008B, (u8)0x0084, (u8)0x007C, (u8)0x0075, (u8)0x006E, (u8)0x0068,
      (u8)0x0062, (u8)0x005D, (u8)0x0057, (u8)0x0052, (u8)0x004E, (u8)0x0049,
      (u8)0x0045, (u8)0x0041, (u8)0x003E, (u8)0x003A, (u8)0x0037, (u8)0x0034,
      (u8)0x0031, (u8)0x002E, (u8)0x002B, (u8)0x0029, (u8)0x0026, (u8)0x0024,
      (u8)0x0022, (u8)0x0020, (u8)0x001E, (u8)0x001D, (u8)0x001B, (u8)0x0019,
      (u8)0x0018, (u8)0x0016, (u8)0x0015, (u8)0x0014, (u8)0x0013, (u8)0x0012,
      (u8)0x0011, (u8)0x0010, (u8)0x000F, (u8)0x000E, (u8)0x000D, (u8)0x000C};

  const u8 pal_note_table_hi[] = {
      (u8)(0x0C68 >> 8), (u8)(0x0BB6 >> 8), (u8)(0x0B0E >> 8),
      (u8)(0x0A6F >> 8), (u8)(0x09D9 >> 8), (u8)(0x094B >> 8),
      (u8)(0x08C6 >> 8), (u8)(0x0848 >> 8), (u8)(0x07D1 >> 8),
      (u8)(0x0760 >> 8), (u8)(0x06F6 >> 8), (u8)(0x0692 >> 8),
      (u8)(0x0634 >> 8), (u8)(0x05DB >> 8), (u8)(0x0586 >> 8),
      (u8)(0x0537 >> 8), (u8)(0x04EC >> 8), (u8)(0x04A5 >> 8),
      (u8)(0x0462 >> 8), (u8)(0x0423 >> 8), (u8)(0x03E8 >> 8),
      (u8)(0x03B0 >> 8), (u8)(0x037B >> 8), (u8)(0x0349 >> 8),
      (u8)(0x0319 >> 8), (u8)(0x02ED >> 8), (u8)(0x02C3 >> 8),
      (u8)(0x029B >> 8), (u8)(0x0275 >> 8), (u8)(0x0252 >> 8),
      (u8)(0x0231 >> 8), (u8)(0x0211 >> 8), (u8)(0x01F3 >> 8),
      (u8)(0x01D7 >> 8), (u8)(0x01BD >> 8), (u8)(0x01A4 >> 8),
      (u8)(0x018C >> 8), (u8)(0x0176 >> 8), (u8)(0x0161 >> 8),
      (u8)(0x014D >> 8), (u8)(0x013A >> 8), (u8)(0x0129 >> 8),
      (u8)(0x0118 >> 8), (u8)(0x0108 >> 8), (u8)(0x00F9 >> 8),
      (u8)(0x00EB >> 8), (u8)(0x00DE >> 8), (u8)(0x00D1 >> 8),
      (u8)(0x00C6 >> 8), (u8)(0x00BA >> 8), (u8)(0x00B0 >> 8),
      (u8)(0x00A6 >> 8), (u8)(0x009D >> 8), (u8)(0x0094 >> 8),
      (u8)(0x008B >> 8), (u8)(0x0084 >> 8), (u8)(0x007C >> 8),
      (u8)(0x0075 >> 8), (u8)(0x006E >> 8), (u8)(0x0068 >> 8),
      (u8)(0x0062 >> 8), (u8)(0x005D >> 8), (u8)(0x0057 >> 8),
      (u8)(0x0052 >> 8), (u8)(0x004E >> 8), (u8)(0x0049 >> 8),
      (u8)(0x0045 >> 8), (u8)(0x0041 >> 8), (u8)(0x003E >> 8),
      (u8)(0x003A >> 8), (u8)(0x0037 >> 8), (u8)(0x0034 >> 8),
      (u8)(0x0031 >> 8), (u8)(0x002E >> 8), (u8)(0x002B >> 8),
      (u8)(0x0029 >> 8), (u8)(0x0026 >> 8), (u8)(0x0024 >> 8),
      (u8)(0x0022 >> 8), (u8)(0x0020 >> 8), (u8)(0x001E >> 8),
      (u8)(0x001D >> 8), (u8)(0x001B >> 8), (u8)(0x0019 >> 8),
      (u8)(0x0018 >> 8), (u8)(0x0016 >> 8), (u8)(0x0015 >> 8),
      (u8)(0x0014 >> 8), (u8)(0x0013 >> 8), (u8)(0x0012 >> 8),
      (u8)(0x0011 >> 8), (u8)(0x0010 >> 8), (u8)(0x000F >> 8),
      (u8)(0x000E >> 8), (u8)(0x000D >> 8), (u8)0x000C};

  void initialize_apu_buffer() {
    //****************************************************************
    // Initialize Square 1
    //****************************************************************

    // Set Saw Envelope Disable and Length Counter Disable to 1 for square 1.
    _ggsound_apu_register_sets[0] = 0b00110000;

    // Set Negate flag on the sweep unit.
    _ggsound_apu_register_sets[1] = 0x08;

    // Set period to C9, which is a C#...just in case nobody writes to it.
    _ggsound_apu_register_sets[2] = 0xc9;

    // Make sure the old value starts out different from the first default
    // value.
    _ggsound_apu_square_1_old = 0xc9;

    _ggsound_apu_register_sets[3] = 0;

    //****************************************************************
    // Initialize Square 2
    //****************************************************************

    // Set Saw Envelope Disable and Length Counter Disable to 1 for
    // square 2.
    _ggsound_apu_register_sets[4] = 0b00110000;

    // Set Negate flag on the sweep unit.
    _ggsound_apu_register_sets[5] = 0x08;

    // Set period to C9, which is a C#...just in case nobody writes to it.
    _ggsound_apu_register_sets[6] = 0xc9;

    // Make sure the old value starts out different from the first default
    // value.
    _ggsound_apu_square_2_old = 0xc9;

    _ggsound_apu_register_sets[7] = 0x00;

    //****************************************************************
    // Initialize Triangle
    //****************************************************************
    _ggsound_apu_register_sets[8] = 0b10000000;
    _ggsound_apu_register_sets[10] = 0xc9;
    _ggsound_apu_register_sets[11] = 0x00;

    //****************************************************************
    // Initialize Noise
    //****************************************************************
    _ggsound_apu_register_sets[12] = 0b00110000;
    _ggsound_apu_register_sets[13] = 0b00000000;
    _ggsound_apu_register_sets[14] = 0b00000000;
    _ggsound_apu_register_sets[15] = 0b00000000;
  }

  void stream_stop(Stream stream_slot) {
    _ggsound_sound_disable_update = true;
    _ggsound_stream_flags[(u8)stream_slot] = 0;
    _ggsound_sound_disable_update = false;
  }

  void stream_initialize(Stream stream_slot, Channel channel,
                         void *stream_ptr) {
    if (stream_ptr == 0)
      return;
    _ggsound_sound_disable_update = true;

    // Set stream to be inactive while initializing.
    _ggsound_stream_flags[(u8)stream_slot] = 0;

    // Set a default note length (20 frames).
    _ggsound_stream_note_length_lo[(u8)stream_slot] = 20;
    _ggsound_stream_note_length_hi[(u8)stream_slot] = 0;

    // Set initial note length counter.
    _ggsound_stream_note_length_counter_lo[(u8)stream_slot] = 20;
    _ggsound_stream_note_length_counter_hi[(u8)stream_slot] = 0;

    // Set initial instrument index.
    _ggsound_stream_instrument_index[(u8)stream_slot] = 0;
    _ggsound_stream_volume_offset[(u8)stream_slot] = 0;
    _ggsound_stream_pitch_offset[(u8)stream_slot] = 0;
    _ggsound_stream_duty_offset[(u8)stream_slot] = 0;
#ifdef FEATURE_ARPEGGIOS
    _ggsound_stream_arpeggio_offset[(u8)stream_slot] = 0;
#endif

    // Set channel
    _ggsound_stream_channel[(u8)stream_slot] = channel;

    //  Set initial read address
    _ggsound_stream_read_address_lo[(u8)stream_slot] = (u8)(u16)stream_ptr;
    _ggsound_stream_read_address_hi[(u8)stream_slot] =
        (u8)(((u16)stream_ptr) >> 8);

    // Set default tempo
    _ggsound_stream_tempo_lo[(u8)stream_slot] =
        _ggsound_stream_tempo_counter_lo[(u8)stream_slot] = (u8)DEFAULT_TEMPO;
    _ggsound_stream_tempo_hi[(u8)stream_slot] =
        _ggsound_stream_tempo_counter_hi[(u8)stream_slot] =
            (u8)(DEFAULT_TEMPO >> 8);

    // Set stream to be active
    _ggsound_stream_flags[(u8)stream_slot] |= STREAM_ACTIVE_SET;

    _ggsound_sound_disable_update = false;
  }

  __attribute__((noinline)) void init(Region region, Track *song_list[],
                                      Track *sfx_list[], void *instruments[],
                                      u8 bank) {
    _ggsound_sound_disable_update = true;
    _ggsound_sound_bank = bank;
    GGSound::region = region;
    GGSound::song_list = song_list;
    GGSound::sfx_list = sfx_list;
    _ggsound_base_address_instruments = instruments;

    // Load PAL note table for PAL, NTSC for any other region.
    switch (region) {
    case Region::NTSC:
    case Region::Dandy:
      _ggsound_base_address_note_table_lo = ntsc_note_table_lo;
      _ggsound_base_address_note_table_hi = ntsc_note_table_hi;
      break;
    case Region::PAL:
      _ggsound_base_address_note_table_lo = pal_note_table_lo;
      _ggsound_base_address_note_table_hi = pal_note_table_hi;
      break;
    }

    // Enable square 1, square 2, triangle and noise.
    POKE(0x4015, 0b00001111);

    // Ensure no apu data is uploaded yet.
    _ggsound_apu_data_ready = false;

    initialize_apu_buffer();

    // Make sure all streams are killed.
    stop();

    _ggsound_sound_disable_update = false;
  }

  void stop() {
    // Kill all active streams and halt sound.

    _ggsound_sound_disable_update = true;

    // Kill all streams.
    for (u8 i = 0; i < MAX_STREAMS; i++) {
      _ggsound_stream_flags[i] = 0;
    }

    initialize_apu_buffer();

    _ggsound_sound_disable_update = false;
  }

  __attribute__((noinline)) void play_song(Song song) {
    _ggsound_sound_disable_update = true;
    // Get song address from song list.
    Track &track = *song_list[(u8)song];

    u16 track_tempo =
        region == Region::NTSC ? track.ntsc_tempo : track.pal_tempo;

    // Load square 1 stream.
    stream_stop(Stream::Square_1);

    if (track.square_1_stream != 0) {
      stream_initialize(Stream::Square_1, Channel::Square_1,
                        track.square_1_stream);
      _ggsound_stream_tempo_lo[(u8)Stream::Square_1] =
          _ggsound_stream_tempo_counter_lo[(u8)Stream::Square_1] =
              (u8)track_tempo;
      _ggsound_stream_tempo_hi[(u8)Stream::Square_1] =
          _ggsound_stream_tempo_counter_hi[(u8)Stream::Square_1] =
              (u8)(track_tempo >> 8);
    }

    // Load square 2 stream.
    stream_stop(Stream::Square_2);
    if (track.square_2_stream != 0) {
      stream_initialize(Stream::Square_2, Channel::Square_2,
                        track.square_2_stream);

      _ggsound_stream_tempo_lo[(u8)Stream::Square_2] =
          _ggsound_stream_tempo_counter_lo[(u8)Stream::Square_2] =
              (u8)track_tempo;
      _ggsound_stream_tempo_hi[(u8)Stream::Square_2] =
          _ggsound_stream_tempo_counter_hi[(u8)Stream::Square_2] =
              (u8)(track_tempo >> 8);
    }

    // Load triangle stream.
    stream_stop(Stream::Triangle);
    if (track.triangle_stream != 0) {
      stream_initialize(Stream::Triangle, Channel::Triangle,
                        track.triangle_stream);
      _ggsound_stream_tempo_lo[(u8)Stream::Triangle] =
          _ggsound_stream_tempo_counter_lo[(u8)Stream::Triangle] =
              (u8)track_tempo;
      _ggsound_stream_tempo_hi[(u8)Stream::Triangle] =
          _ggsound_stream_tempo_counter_hi[(u8)Stream::Triangle] =
              (u8)(track_tempo >> 8);
    }

    // Load noise stream.
    stream_stop(Stream::Noise);

    if (track.noise_stream != 0) {
      stream_initialize(Stream::Noise, Channel::Noise, track.noise_stream);
      _ggsound_stream_tempo_lo[(u8)Stream::Noise] =
          _ggsound_stream_tempo_counter_lo[(u8)Stream::Noise] = (u8)track_tempo;
      _ggsound_stream_tempo_hi[(u8)Stream::Noise] =
          _ggsound_stream_tempo_counter_hi[(u8)Stream::Noise] =
              (u8)(track_tempo >> 8);
    }
    _ggsound_sound_disable_update = false;
  }

  __attribute__((noinline)) void play_sfx(SFX sfx, SFXPriority priority) {
    _ggsound_sound_disable_update = true;

    // Get song address from song list.
    Track &track = *sfx_list[(u8)sfx];

    u16 track_tempo =
        region == Region::NTSC ? track.ntsc_tempo : track.pal_tempo;

    // Load square 1 stream.
    if (track.square_1_stream != 0) {
      stream_initialize((Stream)priority, Channel::Square_1,
                        track.square_1_stream);
      _ggsound_stream_tempo_lo[(u8)priority] =
          _ggsound_stream_tempo_counter_lo[(u8)priority] = (u8)track_tempo;
      _ggsound_stream_tempo_hi[(u8)priority] =
          _ggsound_stream_tempo_counter_hi[(u8)priority] =
              (u8)(track_tempo >> 8);
      if (priority == SFXPriority::Two)
        goto exit;
      priority = SFXPriority::Two;
    }

    // Load square 2 stream.
    if (track.square_2_stream != 0) {
      stream_initialize((Stream)priority, Channel::Square_2,
                        track.square_2_stream);

      _ggsound_stream_tempo_lo[(u8)priority] =
          _ggsound_stream_tempo_counter_lo[(u8)priority] = (u8)track_tempo;
      _ggsound_stream_tempo_hi[(u8)priority] =
          _ggsound_stream_tempo_counter_hi[(u8)priority] =
              (u8)(track_tempo >> 8);
      if (priority == SFXPriority::Two)
        goto exit;
      priority = SFXPriority::Two;
    }

    // Load triangle stream.
    if (track.triangle_stream != 0) {
      stream_initialize((Stream)priority, Channel::Triangle,
                        track.triangle_stream);
      _ggsound_stream_tempo_lo[(u8)priority] =
          _ggsound_stream_tempo_counter_lo[(u8)priority] = (u8)track_tempo;
      _ggsound_stream_tempo_hi[(u8)priority] =
          _ggsound_stream_tempo_counter_hi[(u8)priority] =
              (u8)(track_tempo >> 8);
      if (priority == SFXPriority::Two)
        goto exit;
      priority = SFXPriority::Two;
    }

    // Load noise stream.
    if (track.noise_stream != 0) {
      stream_initialize((Stream)priority, Channel::Noise, track.noise_stream);
      _ggsound_stream_tempo_lo[(u8)priority] =
          _ggsound_stream_tempo_counter_lo[(u8)priority] = (u8)track_tempo;
      _ggsound_stream_tempo_hi[(u8)priority] =
          _ggsound_stream_tempo_counter_hi[(u8)priority] =
              (u8)(track_tempo >> 8);
    }
  exit:
    _ggsound_sound_disable_update = false;
  }

  void pause() {
    // Pauses all music streams by clearing volume bits from all channel
    // registers and setting the pause flag so these streams are not updated.
    for (u8 i = 0; i < MAX_MUSIC_STREAMS; i++) {
      _ggsound_stream_flags[i] |= STREAM_PAUSE_SET;
      _ggsound_stream_channel_register_1[i] &= 0b11110000;
    }
  }

  void resume() {
    // Resumes all music streams.

    for (u8 i = 0; i < MAX_MUSIC_STREAMS; i++) {
      _ggsound_stream_flags[i] &= STREAM_PAUSE_CLEAR;
    }
  }
} // namespace GGSound
