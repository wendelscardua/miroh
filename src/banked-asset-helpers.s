.section .prg_rom_fixed,"axR",@progbits

.global banked_oam_meta_spr

; void banked_oam_meta_spr(char x, int y, const void *data);
; A/rc6: x
; X/rc7: y.l
; rc2: y.h 
; rc4: data.l
; rc5: data.h
; rc8: extend signal for sprite delta y

banked_oam_meta_spr:
  sta __rc6
  stx __rc7

  jsr get_prg_bank
  pha
  lda #mos24bank(_ZN11Metasprites5blockE)
  jsr set_prg_bank

  ldx SPRID
  ldy #0
1:
  lda (__rc4),y  ;x offset
  cmp #$80
  beq 2f
  iny
  clc
  adc __rc6
  sta OAM_BUF+3,x
  
  lda #0
  sta __rc8
  
  lda (__rc4),y  ;y offset
  
  ; extend signal to 16 bits
  bpl 3f
  beq 3f
  
  lda #$ff
  sta __rc8
  lda (__rc4),y  ;restore y offset
3:
  iny
  clc
  adc __rc7
  sta OAM_BUF+0,x
  
  lda __rc2
  adc __rc8

  ; high byte zero = on screen (?)
  beq 4f

  ; not on screen, go to next sprite
  lda #$ff
  sta OAM_BUF+0,x
  iny
  iny
  jmp 1b

4:
  
  lda (__rc4),y  ;tile
  iny
  sta OAM_BUF+1,x
  lda (__rc4),y  ;attribute
  iny
  sta OAM_BUF+2,x
  inx
  inx
  inx
  inx
  jmp 1b
2:
  stx SPRID
  pla
  jsr set_prg_bank
  rts


.global banked_oam_meta_spr_horizontal

; void banked_oam_meta_spr_horizontal(int x, char y, const void *data);
; A/rc6: x.l
; X/rc7: x.h
; rc2: y
; rc4: data.l
; rc5: data.h
; rc8: extend signal for sprite delta x

banked_oam_meta_spr_horizontal:
  sta __rc6
  stx __rc7

  jsr get_prg_bank
  pha
  lda #mos24bank(_ZN11Metasprites5blockE)
  jsr set_prg_bank

  ldx SPRID
  ldy #0
1:
  lda (__rc4),y  ;x offset
  cmp #$80
  beq 2f
  
  bcs 5f ; go to extend negative

  lda #0
  sta __rc8 ; extend positive
  jmp 6f
5: 
  lda #$ff
  sta __rc8
6: ; proceed with x offset
  lda (__rc4),y ; restore x offset
  iny
  clc
  adc __rc6
  sta OAM_BUF+3,x

  lda __rc7
  adc __rc8

  ; high byte zero = on screen (?)
  beq 4f

  ; not on screen, go to next sprite
  iny
  iny
  iny
  jmp 1b

4:
  
  lda (__rc4),y  ; y offset
  iny
  clc
  adc __rc2
  sta OAM_BUF+0,x
    
  lda (__rc4),y  ;tile
  iny
  sta OAM_BUF+1,x
  lda (__rc4),y  ;attribute
  iny
  sta OAM_BUF+2,x
  inx
  inx
  inx
  inx
  jmp 1b
2:
  stx SPRID
  pla
  jsr set_prg_bank
  rts