    .section .prg_rom_1,"a"

    .global level_bg_palettes
level_bg_palettes:
    .byte StarlitStablesBG_pal@mos16lo
    .byte StarlitStablesBG_pal@mos16hi

    .global level_spr_palettes
level_spr_palettes:
    .byte StarlitStablesSPR_pal@mos16lo
    .byte StarlitStablesSPR_pal@mos16hi

    .global level_bg_tiles
level_bg_tiles:
    .byte StarlitStablesBG_chr@mos16lo
    .byte StarlitStablesBG_chr@mos16hi

    .global level_spr_tiles
level_spr_tiles:
    .byte SPR_chr@mos16lo
    .byte SPR_chr@mos16hi

    .global level_nametables
level_nametables:
    .byte StarlitStables_nam@mos16lo
    .byte StarlitStables_nam@mos16hi

    .global level_alt_nametables
level_alt_nametables:
    .byte StarlitStablesAlt_nam@mos16lo
    .byte StarlitStablesAlt_nam@mos16hi

    ; Generic
SPR_chr: .incbin "SPR.chr.donut"
 
    ; Starlit Stables
StarlitStablesBG_pal: .incbin "StarlitStablesBG.pal"
StarlitStablesSPR_pal: .incbin "StarlitStablesSPR.pal"
StarlitStablesBG_chr: .incbin "StarlitStablesBG.chr.donut"
StarlitStables_nam: .incbin "StarlitStables.nam.rle"
StarlitStablesAlt_nam: .incbin "StarlitStablesAlt.nam.rle"

    ; non-level stuff
.global title_nam
title_nam: .incbin "Title.nam.rle"
.global how_to_nam
how_to_nam: .incbin "how-to.nam.rle"
.global credits_nam
credits_nam: .incbin "credits.nam.rle"
