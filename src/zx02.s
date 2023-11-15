; De-compressor for ZX02 files
; ----------------------------
;
; Decompress ZX02 data (6502 optimized format), optimized for minimal size:
;  130 bytes code, 72.6 cycles/byte in test file.
;
; Compress with:
;    zx02 input.bin output.zx0
;
; (c) 2022 DMSC
; Code under MIT license, see LICENSE.zx02 file.
;
; (adapted for llvm-mos + chr ram buffer by Wendel Scardua)

.section .zp,"z",@nobits
offset: .zero 2
ZX0_src: .zero 2
ZX0_dst: .zero 2
bitr: .zero 1
pntr: .zero 2

.section .prg_rom_last,"axR",@progbits

PPU_ADDR = $2006
PPU_DATA = $2007

; void zx02_decompress_to_chr_ram(void *src)
.global zx02_decompress_to_chr_ram
zx02_decompress_to_chr_ram:
; TODO maybe use __rc2 directly instead of ZX0_src
    sta ZX0_dst
    stx ZX0_dst+1
    lda mos8(__rc2)
    sta ZX0_src
    lda mos8(__rc3)
    sta ZX0_src+1

    lda #$00
    sta offset
    sta offset+1
    lda #$80
    sta bitr

    ldy #0

;--------------------------------------------------
; Decompress ZX0 data (6502 optimized format)

; Decode literal: Copy next N bytes from compressed file
;    Elias(length)  byte[1]  byte[2]  ...  byte[N]
decode_literal:
              jsr   get_elias

cop0:         jsr   get_byte
              jsr   put_byte
              bne   cop0

              asl   bitr
              bcs   dzx0s_new_offset

; Copy from last offset (repeat N bytes from last offset)
;    Elias(length)
              jsr   get_elias
dzx0s_copy:
              lda   ZX0_dst
              sbc   offset  ; C=0 from get_elias
              sta   pntr
              lda   ZX0_dst+1
              sbc   offset+1
              sta   pntr+1

cop1: ; copy from CHRRAM
              lda pntr+1
              sta PPU_ADDR
              lda pntr
              sta PPU_ADDR
              
              lda PPU_DATA
              lda PPU_DATA ; double-read needed
              pha

              ; restore target address
              lda ZX0_dst+1
              sta PPU_ADDR
              lda ZX0_dst
              sta PPU_ADDR
              
              pla ; here, the read byte is on A the same as it was when we were reading from RAM

              inc   pntr
              bne   1f
              inc   pntr+1
1:            jsr   put_byte
              bne   cop1

              asl   bitr
              bcc   decode_literal

; Copy from new offset (repeat N bytes from new offset)
;    Elias(MSB(offset))  LSB(offset)  Elias(length-1)
dzx0s_new_offset:
              ; Read elias code for high part of offset
              jsr   get_elias
              beq   exit  ; Read a 0, signals the end
              ; Decrease and divide by 2
              dex
              txa
              lsr
              sta   offset+1

              ; Get low part of offset, a literal 7 bits
              jsr   get_byte

              ; Divide by 2
              ror
              sta   offset

              ; And get the copy length.
              ; Start elias reading with the bit already in carry:
              ldx   #1
              jsr   elias_skip1

              inx
              bcc   dzx0s_copy

; Read an elias-gamma interlaced code.
; ------------------------------------
get_elias:
              ; Initialize return value to #1
              ldx   #1
              bne   elias_start

elias_get:     ; Read next data bit to result
              asl   bitr
              rol
              tax

elias_start:
              ; Get one bit
              asl   bitr
              bne   elias_skip1

              ; Read new bit from stream
              jsr   get_byte
              ;sec   ; not needed, C=1 guaranteed from last bit
              rol
              sta   bitr

elias_skip1:
              txa
              bcs   elias_get
              ; Got ending bit, stop reading
              rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
get_byte:
              lda   (ZX0_src), y
              inc   ZX0_src
              bne   2f
              inc   ZX0_src+1
exit:
2:            rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
put_byte:
              sta PPU_DATA
              inc   ZX0_dst
              bne   3f
              inc   ZX0_dst+1
3:            dex
              rts

