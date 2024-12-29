        ; .segment "_pdpcm"
        ; .include "soundtrack_dpcm.asm"

        .segment "_pprg__rom__2"
        .include "ggsound.inc"

        .global song_list
        .global sfx_list
        .global instrument_list
        .global dpcm_list
        
        .include "soundtrack.asm"