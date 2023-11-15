    .section .prg_rom_1,"aR",@progbits

    .global level_bg_palettes
level_bg_palettes:
    .byte StarlitStablesBG_pal@mos16lo
    .byte RainbowRetreatBG_pal@mos16lo
    .byte FairyForestBG_pal@mos16lo
    .byte GlitteryGrottoBG_pal@mos16lo
    .byte MarshmallowMountainBG_pal@mos16lo
    .byte StarlitStablesBG_pal@mos16hi
    .byte RainbowRetreatBG_pal@mos16hi
    .byte FairyForestBG_pal@mos16hi
    .byte GlitteryGrottoBG_pal@mos16hi
    .byte MarshmallowMountainBG_pal@mos16hi

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
    .byte MarshmallowMountainBG_chr@mos16lo
    .byte StarlitStablesBG_chr@mos16hi
    .byte RainbowRetreatBG_chr@mos16hi
    .byte FairyForestBG_chr@mos16hi
    .byte GlitteryGrottoBG_chr@mos16hi
    .byte MarshmallowMountainBG_chr@mos16hi

    .global level_bg_tile_blocks
level_bg_tile_blocks:
    .byte 41
    .byte 41
    .byte 0
    .byte 37
    .byte 39

    .global level_nametables
level_nametables:
    .byte StarlitStables_nam@mos16lo
    .byte RainbowRetreat_nam@mos16lo
    .byte FairyForest_nam@mos16lo
    .byte GlitteryGrotto_nam@mos16lo
    .byte MarshmallowMountain_nam@mos16lo
    .byte StarlitStables_nam@mos16hi
    .byte RainbowRetreat_nam@mos16hi
    .byte FairyForest_nam@mos16hi
    .byte GlitteryGrotto_nam@mos16hi
    .byte MarshmallowMountain_nam@mos16hi

    ; Starlit Stables
StarlitStablesBG_pal: .incbin "StarlitStablesBG.pal"
StarlitStablesSPR_pal: .incbin "StarlitStablesSPR.pal"
StarlitStablesBG_chr: .incbin "StarlitStablesBG.suffix.chr.donut"
StarlitStables_nam: .incbin "StarlitStables.nam.zx02"

    ; Rainbow Retreat
RainbowRetreatBG_pal: .incbin "RainbowRetreatBG.pal"
RainbowRetreatSPR_pal: .incbin "RainbowRetreatSPR.pal"
RainbowRetreatBG_chr: .incbin "RainbowRetreatBG.suffix.chr.donut"
RainbowRetreat_nam: .incbin "RainbowRetreat.nam.zx02"

    ; Fairy Forest
FairyForestBG_pal: .incbin "FairyForestBG.pal"
FairyForestSPR_pal: .incbin "FairyForestSPR.pal"
FairyForestBG_chr: .incbin "FairyForestBG.chr.donut"
FairyForest_nam: .incbin "FairyForest.nam.zx02"

    ; GlitteryGrotto
GlitteryGrottoBG_pal: .incbin "GlitteryGrottoBG.pal"
GlitteryGrottoSPR_pal: .incbin "GlitteryGrottoSPR.pal"
GlitteryGrottoBG_chr: .incbin "GlitteryGrottoBG.suffix.chr.donut"
GlitteryGrotto_nam: .incbin "GlitteryGrotto.nam.zx02"

    ; Marshmallow Mountain
MarshmallowMountainBG_pal: .incbin "MarshmallowMountainBG.pal"
MarshmallowMountainSPR_pal: .incbin "MarshmallowMountainSPR.pal"
MarshmallowMountainBG_chr: .incbin "MarshmallowMountainBG.suffix.chr.donut"
MarshmallowMountain_nam: .incbin "MarshmallowMountain.nam.zx02"

    ; Title Screen
.global title_bg_palette, title_spr_palette, title_bg_tiles, title_nametable

title_bg_palette: .incbin "TitleBG.pal"
title_spr_palette: .incbin "TitleSPR.pal"
title_bg_tiles: .incbin "TitleBG.suffix.chr.donut"
title_nametable: .incbin "Title.nam.zx02"

    ; Generic
.global base_bg_tiles
base_bg_tiles = FairyForestBG_chr

.global spr_tiles
spr_tiles: .incbin "SPR.chr.donut"
