#pragma once

#include "ggsound.hpp"

extern "C" const GGSound::Track *song_list[];
extern "C" const GGSound::Track *sfx_list[];
extern "C" const void *instrument_list[];
#ifdef FEATURE_DPCM
extern "C" const void *dpcm_list[];
#endif
