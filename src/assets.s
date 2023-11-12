    .section .prg_rom_1,"aR",@progbits

    .global level_bg_palettes
level_bg_palettes:
    .byte StarlitStablesBG_pal@mos16lo
    .byte RainbowRetreatBG_pal@mos16lo
    .byte FairyForestBG_pal@mos16lo
    .byte GlitteryGrottoBG_pal@mos16lo
    .zero 1
    .byte StarlitStablesBG_pal@mos16hi
    .byte RainbowRetreatBG_pal@mos16hi
    .byte FairyForestBG_pal@mos16hi
    .byte GlitteryGrottoBG_pal@mos16hi
    .zero 1

    .global level_spr_palettes
level_spr_palettes:
    .byte StarlitStablesSPR_pal@mos16lo
    .byte RainbowRetreatSPR_pal@mos16lo
    .byte FairyForestSPR_pal@mos16lo
    .byte GlitteryGrottoSPR_pal@mos16lo
    .byte MarshmallowMountainSPR_pal@mos16lo
    .byte StarlitStablesSPR_pal@mos16hi
    .byte RainbowRetreatSPR_pal@mos16hi
    .byte FairyForestSPR_pal@mos16hi
    .byte GlitteryGrottoSPR_pal@mos16hi
    .byte MarshmallowMountainSPR_pal@mos16hi
    .global level_bg_tiles
level_bg_tiles:
    .byte StarlitStablesBG_chr@mos16lo
    .byte RainbowRetreatBG_chr@mos16lo
    .byte FairyForestBG_chr@mos16lo
    .byte GlitteryGrottoBG_chr@mos16lo
    .zero 1
    .byte StarlitStablesBG_chr@mos16hi
    .byte RainbowRetreatBG_chr@mos16hi
    .byte FairyForestBG_chr@mos16hi
    .byte GlitteryGrottoBG_chr@mos16hi
    .zero 1

    .global level_nametables
level_nametables:
    .byte StarlitStables_nam@mos16lo
    .byte RainbowRetreat_nam@mos16lo
    .byte FairyForest_nam@mos16lo
    .byte GlitteryGrotto_nam@mos16lo
    .zero 1
    .byte StarlitStables_nam@mos16hi
    .byte RainbowRetreat_nam@mos16hi
    .byte FairyForest_nam@mos16hi
    .byte GlitteryGrotto_nam@mos16hi
    .zero 1

    .global level_alt_nametables
level_alt_nametables:
    .byte StarlitStablesAlt_nam@mos16lo
    .byte RainbowRetreatAlt_nam@mos16lo
    .byte FairyForestAlt_nam@mos16lo
    .byte GlitteryGrottoAlt_nam@mos16lo
    .zero 1
    .byte StarlitStablesAlt_nam@mos16hi
    .byte RainbowRetreatAlt_nam@mos16hi
    .byte FairyForestAlt_nam@mos16hi
    .byte GlitteryGrottoAlt_nam@mos16hi
    .zero 1
 
    ; Starlit Stables
StarlitStablesBG_pal: .incbin "StarlitStablesBG.pal"
StarlitStablesSPR_pal: .incbin "StarlitStablesSPR.pal"
StarlitStablesBG_chr: .incbin "StarlitStablesBG.chr.donut"
StarlitStables_nam: .incbin "StarlitStables.nam.rle"
StarlitStablesAlt_nam: .incbin "StarlitStablesAlt.nam.rle"

    ; Rainbow Retreat
RainbowRetreatBG_pal: .incbin "RainbowRetreatBG.pal"
RainbowRetreatSPR_pal: .incbin "RainbowRetreatSPR.pal"
RainbowRetreatBG_chr: .incbin "RainbowRetreatBG.chr.donut"
RainbowRetreat_nam: .incbin "RainbowRetreat.nam.rle"
RainbowRetreatAlt_nam: .incbin "RainbowRetreatAlt.nam.rle"

    ; Fairy Forest
FairyForestBG_pal: .incbin "FairyForestBG.pal"
FairyForestSPR_pal: .incbin "FairyForestSPR.pal"
FairyForestBG_chr: .incbin "FairyForestBG.chr.donut"
FairyForest_nam: .incbin "FairyForest.nam.rle"
FairyForestAlt_nam: .incbin "FairyForestAlt.nam.rle"

    ; GlitteryGrotto
GlitteryGrottoBG_pal: .incbin "GlitteryGrottoBG.pal"
GlitteryGrottoSPR_pal: .incbin "GlitteryGrottoSPR.pal"
GlitteryGrottoBG_chr: .incbin "GlitteryGrottoBG.chr.donut"
GlitteryGrotto_nam: .incbin "GlitteryGrotto.nam.rle"
GlitteryGrottoAlt_nam: .incbin "GlitteryGrottoAlt.nam.rle"

    ; Marshmallow Mountain
;MarshmallowMountainBG_pal: .incbin "MarshmallowMountainBG.pal"
MarshmallowMountainSPR_pal: .incbin "MarshmallowMountainSPR.pal"
;MarshmallowMountainBG_chr: .incbin "MarshmallowMountainBG.chr.donut"
;MarshmallowMountain_nam: .incbin "MarshmallowMountain.nam.rle"
;MarshmallowMountainAlt_nam: .incbin "MarshmallowMountainAlt.nam.rle"

    ; Title Screen
.global title_bg_palette, title_spr_palette, title_bg_tiles, title_nametable, title_alt_nametable

title_bg_palette: .incbin "TitleBG.pal"
title_spr_palette: .incbin "TitleSPR.pal"
title_bg_tiles: .incbin "TitleBG.chr.donut"
title_nametable: .incbin "Title.nam.rle"
title_alt_nametable: .incbin "TitleAlt.nam.rle"

    ; Generic
.global spr_tiles
spr_tiles: .incbin "SPR.chr.donut"
