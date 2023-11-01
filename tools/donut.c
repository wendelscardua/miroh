/*  donut-nes - v2023-10-23 - public domain (details at end of file)
    codec for texture/character data of the Nintendo Entertainment System

    By Johnathan Roatch - https://jroatch.nfshost.com/donut-nes/

    Compile with any standard C99 compiler:
    gcc -O2 -std=c99 -DUSE_MAIN_CLI_APP -o donut-nes donut-nes.c

    or to use as a single header styled library, rename this to
    "jrr-donut-nes.h", and then #define JRR_DONUT_NES_IMPLEMENTATION
    in *one* C/C++ file before the #include. For example:

    #define JRR_DONUT_NES_IMPLEMENTATION
    #include "jrr-donut-nes.h"

    This compression codec is designed for the native texture data of a
    Nintendo Entertainment System (NES) which can be briefly described as
    8x8 2bpp planer formated tiles. Each NES tile uses 16 bytes of data
    (8 bytes of a "lower bitplane" followed by 8 bytes of a "higher bitplane")
    and are often arranged together with similar characteristics such as
    sharing a particular color. A fixed decoded block size of 64 bytes
    was chosen for several reasons:
    - It fits 4 tiles NES tiles which is often put together in a "metatile"
    - It's the size of 2 rows of tilemap indexes, and the attribute table
    - less then 256 bytes, beyond which would complicate 6502 addressing modes
    - It's 8^3 bits, or 8 planes of 8x8 1bpp tiles

    The compressed block is a variable sized block with most of the key
    processing info in the first 1 or 2 bytes:
    bit 76543210
        |||||||+-- Rotate plane bits (135° reflection)
        ||||000--- All planes: 0x00
        ||||010--- L: 0x00, H:  pb8
        ||||100--- L:  pb8, H: 0x00
        ||||110--- All planes: pb8
        ||||001--- In another header byte, For each bit starting from MSB
        ||||         0: 0x00 plane
        ||||         1: pb8 plane
        ||||011--- In another header byte, Decode only 1 pb8 plane and
        ||||       duplicate it for each bit starting from MSB
        ||||         0: 0x00 plane
        ||||         1: duplicated plane
        ||||       If extra header byte = 0x00, no pb8 plane is decoded.
        ||||1x1x-- Reserved for Uncompressed block bit pattern
        |||+------ H predict from 0xff
        ||+------- L predict from 0xff
        |+-------- H = H XOR L
        +--------- L = H XOR L
        00101010-- Uncompressed block of 64 bytes (bit pattern is ascii '*' )
        11-------- Avaliable for future extensions

    "L" are the odd 8 byte bitplanes, and "H" the even 8 byte bitplanes.
    "0x00 planes" can be 8 bytes of 0x00 or 0xff depending on the predict bit.
    "pb8 plane" consists of a 8-bit header where each bit indicates
    duplicating the previous byte or reading a literal byte. The variant
    used here unpacks backwards.
*/
#ifndef INCLUDE_JRR_DONUT_NES_H
#define INCLUDE_JRR_DONUT_NES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>       // size_t
#include <stdint.h>       // uint8_t, ect
#include <stdbool.h>      // bool


/*  Decompress a series of blocks from the buffer 'src' with the size 'src_length'
    into an already allocated buffer 'dst' of size 'dst_capacity'.
    Returns: the number of bytes written to 'dst'.
    src_bytes_read: if not NULL, it's written with the total number of bytes read. */
size_t donut_decompress(uint8_t* dst, size_t dst_capacity, const uint8_t* src, size_t src_length, size_t* src_bytes_read);

/*  Like donut_decompress(), in reverse. */
size_t donut_compress(uint8_t* dst, size_t dst_capacity, const uint8_t* src, size_t src_length, size_t* src_bytes_read);

/*  When compressing, the source can expand to a maximum ratio of 65:64.
    use this to figure how large you should make the 'dst' buffer. */
#define donut_compress_bound(x) ((((x) + 63) / 64) * 65)

/*  Reads 64 bytes from 'src' and writes 1~65 bytes to 'dst'
    'cpu_limit': limits the amount of time the 6502 decoder should
      take for the block, 0 means unlimited.
    'mask': is a optinal bitmap the same size as src where for each bit;
    - 0 means leave the bit in src alone,
    - 1 means fill in the bit in a attempt to optimize compression.
    Returns: the number of bytes written to 'dst'.*/
int donut_pack_block(uint8_t* dst, const uint8_t* src, int cpu_limit, const uint8_t* mask);

/*  Reads 1~74 bytes from 'src' to then decompress
    a fixed size 64 byte block to 'dst'
    Returns: the number of bytes written to 'dst', 0 for none. */
int donut_unpack_block(uint8_t* dst, const uint8_t* src);

/*  parse how much cpu time the 6502 decoder would take for a given coded block */
int donut_block_runtime_cost(const uint8_t* buf, int len);

/*  reads 8 bytes from 'src' to pack 1~9 bytes into 'dst'
    'top_value' is the predicted byte before the first
    This pb8 variant is ordered from last to first byte.
    and bits 0 and 1 bits means duplicate and literal respectively
    Returns: the number of bytes written to 'dst'. */
int donut_pack_pb8(uint8_t* dst, uint64_t src, uint8_t top_value);

/*  reads 1~9 bytes from 'src' to unpack 8 bytes into 'dst'
    'top_value' is the predicted byte before the first
    Returns: the number of bytes written to 'dst'. */
int donut_unpack_pb8(uint64_t* dst, const uint8_t* src, uint8_t top_value);

#ifdef __cplusplus
}
#endif

#endif // INCLUDE_JRR_DONUT_NES_H


#ifdef USE_MAIN_CLI_APP
#define JRR_DONUT_NES_IMPLEMENTATION
#endif

#ifdef JRR_DONUT_NES_IMPLEMENTATION

#include <string.h>       // memcpy

// word read/write macros from https://justine.lol/endian.html
#define READ64LE(S)                                                    \
  ((uint64_t)(255 & (S)[7]) << 070 | (uint64_t)(255 & (S)[6]) << 060 | \
   (uint64_t)(255 & (S)[5]) << 050 | (uint64_t)(255 & (S)[4]) << 040 | \
   (uint64_t)(255 & (S)[3]) << 030 | (uint64_t)(255 & (S)[2]) << 020 | \
   (uint64_t)(255 & (S)[1]) << 010 | (uint64_t)(255 & (S)[0]) << 000)

#define WRITE64LE(P, V)                        \
  ((P)[0] = (0x00000000000000FF & (V)) >> 000, \
   (P)[1] = (0x000000000000FF00 & (V)) >> 010, \
   (P)[2] = (0x0000000000FF0000 & (V)) >> 020, \
   (P)[3] = (0x00000000FF000000 & (V)) >> 030, \
   (P)[4] = (0x000000FF00000000 & (V)) >> 040, \
   (P)[5] = (0x0000FF0000000000 & (V)) >> 050, \
   (P)[6] = (0x00FF000000000000 & (V)) >> 060, \
   (P)[7] = (0xFF00000000000000 & (V)) >> 070, (P) + 8)

int donut_unpack_pb8(uint64_t* dst, const uint8_t* src, uint8_t pb8_byte)
{
	int y = 0;
	uint8_t pb8_flags = src[y];
	++y;
	uint64_t val = 0;
	for (int i = 0; i < 8; ++i) {
		if (pb8_flags & 0x80) {
			pb8_byte = src[y];
			++y;
		}
		pb8_flags <<= 1;
		val <<= 8;
		val |= pb8_byte;
	}
	*dst = val;
	return y;
}

int donut_pack_pb8(uint8_t* dst, uint64_t src, uint8_t pb8_byte)
{
	uint8_t pb8_flags = 0;
	int y = 1;
	for (int i = 0; i < 8; ++i) {
		uint8_t c = src >> (8*(7-i));
		if (c == pb8_byte) continue;
		dst[y] = c;
		++y;
		pb8_byte = c;
		pb8_flags |= 0x80>>i;
	}
	dst[0] = pb8_flags;
	return y;
}

static uint64_t donut_flip_plane(uint64_t plane)
{
	uint64_t result = 0;
	uint64_t t;
	int i;
	if (plane == 0xffffffffffffffff) return plane;
	if (plane == 0x0000000000000000) return plane;
	for (i = 0; i < 8; ++i) {
		t = plane >> i;
		t &= 0x0101010101010101;
		t *= 0x0102040810204080;
		t >>= 56;
		t &= 0xff;
		result |= t << (i*8);
	}
	return result;
}

// kind of a transcription of the 6502 assembly decoder.
int donut_unpack_block(uint8_t* dst, const uint8_t* src)
{
	int y = 0;
	int x = 0;
	uint8_t block_header = src[y];
	if (block_header >= 0xc0) return 0;  // Return to caller to let it do the processing of headers >= 0xc0.
	++y;
	if ((block_header & 0x3e) == 0x00) {
		// if b2 and b3 == 0, then no mater the combination of
		// b0 (rotation), b6 (XOR), or b7 (XOR) the result will
		// always be 64 bytes of \x00
		memset(&dst[x], 0x00, 64);
		return 1;
	}
	if (block_header == 0x2a) {
		memcpy(&dst[x], &src[y], 64);
		return 65;
	}
	uint8_t plane_def = 0xffaa5500 >> ((block_header & 0x0c) << 1);
	bool single_plane_mode = false;
	if (block_header & 0x02) {
		single_plane_mode = (block_header & 0x04);
		plane_def = src[y];
		++y;
	}
	uint64_t prev_plane = 0x0000000000000000;
	for (int i = 0; i < 8; ++i) {
		uint64_t plane = 0x0000000000000000;
		if (!(i & 1) && (block_header & 0x20)) plane = 0xffffffffffffffff;
		if ((i & 1) && (block_header & 0x10)) plane = 0xffffffffffffffff;
		if (plane_def & 0x80) {
			if (single_plane_mode) y = 2;
			y += donut_unpack_pb8(&plane, &src[y], (uint8_t)plane);
			if (block_header & 0x01) plane = donut_flip_plane(plane);
		}
		plane_def <<= 1;
		if (i & 1) {
			if (block_header & 0x80) prev_plane ^= plane;
			if (block_header & 0x40) plane ^= prev_plane;
			(void)WRITE64LE(&dst[x], prev_plane);
			x += 8;
			(void)WRITE64LE(&dst[x], plane);
			x += 8;
		}
		prev_plane = plane;
	}
	return y;
}

static uint8_t donut_popcount(uint8_t x)
{
	x = (x & 0x55) + ((x >> 1) & 0x55);
	x = (x & 0x33) + ((x >> 2) & 0x33);
	x = (x & 0x0f) + ((x >> 4) & 0x0f);
	return (int)x;
}

// TODO: numbers are from the old 2019-10-23 nes runtime, update to 2023-10-08
int donut_block_runtime_cost(const uint8_t* buf, int len)
{
	if (len <= 0) return 0;
	uint8_t block_header = buf[0];
	--len;
	if (block_header >= 0xc0) return 0;
	if (block_header == 0x2a) return 1268;
	int cycles = 1298;
	if (block_header & 0xc0) cycles += 640;
	if (block_header & 0x20) cycles += 4;
	if (block_header & 0x10) cycles += 4;
	uint8_t plane_def = 0xffaa5500 >> ((block_header & 0x0c) << 1);
	uint8_t pb8_count = 0x08040400 >> ((block_header & 0x0c) << 1);
	bool single_plane_mode = false;
	if (block_header & 0x02) {
		if (len <= 0) return 0;
		cycles += 5;
		plane_def = buf[1];
		--len;
		pb8_count = donut_popcount(plane_def);
		single_plane_mode = ((block_header & 0x04) && (plane_def != 0x00));
	}
	cycles += (block_header & 0x01) ? (pb8_count * 614) : (pb8_count * 75);
	if (single_plane_mode) {
		len *= pb8_count;
		cycles += pb8_count;
	}
	len -= pb8_count;
	cycles += len * 6;
	return cycles;
}

// TODO: Clean up fill_dont_care_bits stuff?
static uint64_t donut_nes_fill_dont_care_bits_helper(uint64_t plane, uint64_t dont_care_mask, uint64_t xor_bg, uint8_t top_value) {
	uint64_t result_plane = 0;
	uint64_t backwards_smudge_plane = 0;
	uint64_t current_byte, mask, inv_mask;
	int i;
	if (dont_care_mask == 0x0000000000000000) return plane;

	current_byte = top_value;
	for (i = 0; i < 8; ++i) {
		mask = dont_care_mask & ((uint64_t)0xff << (i*8));
		inv_mask = ~dont_care_mask & ((uint64_t)0xff << (i*8));
		current_byte = (current_byte & mask) | (plane & inv_mask);
		backwards_smudge_plane |= current_byte;
		current_byte = current_byte << 8;
	}
	backwards_smudge_plane ^= xor_bg & dont_care_mask;

	current_byte = (uint64_t)top_value << 56;
	for (i = 0; i < 8; ++i) {
		mask = dont_care_mask & ((uint64_t)0xff << (8*(7-i)));
		inv_mask = ~dont_care_mask & ((uint64_t)0xff << (8*(7-i)));
		if ((plane & inv_mask) == (current_byte & inv_mask)) {
			current_byte = (current_byte & mask) | (plane & inv_mask);
		} else {
			current_byte = (backwards_smudge_plane & mask) | (plane & inv_mask);
		}
		result_plane |= current_byte;
		current_byte = current_byte >> 8;
	}

	return result_plane;
}

static void donut_nes_fill_dont_care_bits(uint64_t* planes, const uint64_t* masks, uint8_t mode)
{
	uint64_t plane_predict_l;
	uint64_t plane_predict_m;
	for (int i = 0; i < 8; i += 2) {
		plane_predict_l = (mode & 0x20) ? 0xffffffffffffffff : 0x0000000000000000;
		planes[i+0] = donut_nes_fill_dont_care_bits_helper(planes[i+0], masks[i+0], 0, plane_predict_l);
		plane_predict_m = (mode & 0x10) ? 0xffffffffffffffff : 0x0000000000000000;
		planes[i+1] = donut_nes_fill_dont_care_bits_helper(planes[i+1], masks[i+1], 0, plane_predict_m);

		if (mode & 0x80) planes[i+0] = donut_nes_fill_dont_care_bits_helper(planes[i+0], masks[i+0], planes[i+1], plane_predict_l);
		if (mode & 0x40) planes[i+1] = donut_nes_fill_dont_care_bits_helper(planes[i+1], masks[i+1], planes[i+0], plane_predict_m);
	}
	return;
}

static bool donut_nes_all_pb8_planes_match(const uint8_t* buf, int len, int pb8_count)
{
	// a block of 0 dupplicate pb8 planes is 1 byte more then normal,
	// and a normal block of 1 pb8 plane is 5 cycles less to decode
	if (pb8_count <= 1) return false;
	len -= 2;
	if (len % pb8_count) return false; 	// planes don't divide evenly
	int pb8_length = len/pb8_count;
	int i, c;
	for (c = 0, i = pb8_length; i < len; ++i, ++c) {
		if (c >= pb8_length) c = 0;
		if (buf[c+2] != buf[i+2]) return false;  // a plane didn't match
	}
	return true;
}

int donut_pack_block(uint8_t* dst, const uint8_t* src, int cpu_limit, const uint8_t* mask)
{
	uint64_t planes[(mask) ? 16 : 8];
	uint8_t cblock[76];
	// 2+9*8 == 74 for max encoded block
	// 65+11 == 76 for uncompressed block with a optimized block test
	int i;

	if (!cpu_limit) cpu_limit = 16384; 	// basically unlimited.

	// first load the fallback uncompressed block.
	dst[0] = 0x2a;
	memcpy(dst + 1, src, 64);
	int shortest_len = 65;
	int least_cost = 1268;
	// if cpu_limit constrains too much, uncompressed block is all that can happen.
	if (cpu_limit < 1298) return shortest_len;
	for (i = 0; i < 8; ++i) {
		planes[i] = READ64LE(&src[i*8]);
	}
	if (mask) {
		for (i = 0; i < 8; ++i) {
			planes[i+8] = READ64LE(&mask[i*8]);
		}
	}

	// Try to compress with all 48 different block modes.
	// The rotate bit (0x01) will toggle last so that flip_plane
	// is only called once instead of 24 times.
	uint8_t a = 0x00;
	while (1) {
		if (a >= 0xc0) {
			if (a & 0x01) break;
			for (i = 0; i < ((mask) ? 16 : 8); ++i) {
				planes[i] = donut_flip_plane(planes[i]);
			}
			a = 0x01;
		}
		if (mask) donut_nes_fill_dont_care_bits(planes, &planes[8], a);
		// With the block mode in mind, pack the 64 bytes of data into 8 pb8 planes.
		uint8_t plane_def = 0x00;
		int len = 2;
		int pb8_count = 0;
		uint64_t first_non_zero_plane = 0;
		bool planes_match = true;
		for (i = 0; i < 8; ++i) {
			uint64_t plane_predict = 0x0000000000000000;
			uint64_t plane = planes[i];
			if (i & 1) {
				if (a & 0x10) plane_predict = 0xffffffffffffffff;
				if (a & 0x40) plane ^= planes[i-1];
			} else {
				if (a & 0x20) plane_predict = 0xffffffffffffffff;
				if (a & 0x80) plane ^= planes[i+1];
			}
			plane_def <<= 1;
			if (plane != plane_predict) {
				len += donut_pack_pb8(&cblock[len], plane, (uint8_t)plane_predict);
				plane_def |= 1;
				++pb8_count;
				if (pb8_count == 1) {
					first_non_zero_plane = plane;
				} else if (plane != first_non_zero_plane) {
					planes_match = false;
				}
			}
		}
		cblock[0] = a | 0x02;
		cblock[1] = plane_def;
		// now that we have the basic block form, try to find optimizations
		// temp_p is needed because a optimization removes a byte from the start
		int cycles = donut_block_runtime_cost(cblock, len);
		uint8_t* temp_p = cblock;
		if (donut_nes_all_pb8_planes_match(temp_p, len, pb8_count) && ((cycles + pb8_count) <= cpu_limit)) {
			temp_p[0] = a | 0x06;
			len = ((len - 2) / pb8_count) + 2;
			cycles += pb8_count;
			planes_match = false; // disable that optimization
		} else {
			for (i = 0; i < 4*8; i += 8) {
				if (plane_def != ((0xffaa5500 >> i) & 0xff)) continue;
				++temp_p;
				temp_p[0] = a | (i >> 1);
				--len;
				cycles -= 5;
				planes_match = false; // disable that optimization
				break;
			}
		}

		// compare size and cpu cost to choose the block of this mode
		// or to keep the old one.
		if ((len <= shortest_len) && ((cycles < least_cost) || (len < shortest_len)) && (cycles <= cpu_limit)) {
			memcpy(dst, temp_p, len);
			shortest_len = len;
			least_cost = cycles;
		}

		// if possible also try this optimization where a single plane mode
		// block has a pb8 plane with a leading 0x00/0xff byte
		if ((pb8_count > 1) && planes_match) {
			temp_p = cblock;
			temp_p[0] = a | 0x06;
			temp_p[1] = plane_def;
			len = 2 + donut_pack_pb8(&temp_p[2], first_non_zero_plane, ~(first_non_zero_plane >> (7*8)));
			cycles = donut_block_runtime_cost(temp_p, len);
			if ((len <= shortest_len) && ((cycles < least_cost) || (len < shortest_len)) && (cycles <= cpu_limit)) {
				memcpy(dst, temp_p, len);
				shortest_len = len;
				least_cost = cycles;
			}
		}

		a += 0x10;  // onto the next block mode.
	}

	return shortest_len;
}

size_t donut_decompress(uint8_t* dst, size_t dst_capacity, const uint8_t* src, size_t src_length, size_t* src_bytes_read)
{
	uint8_t scratch_space[64+74];
	int dst_length = 0;
	int bytes_read = 0;
	int l;
	while (1) {
		int src_bytes_remain = src_length - bytes_read;
		int dst_bytes_remain = dst_capacity - dst_length;
		if (src_bytes_remain <= 0) break;
		if (dst_bytes_remain < 64) break;
		if (src_bytes_remain < 74) {
			memset(scratch_space, 0x00, 64+74);
			memcpy(&scratch_space[64], &src[bytes_read], src_bytes_remain);
			l = donut_unpack_block(scratch_space, &scratch_space[64]);
			if ((!l) || (l > src_bytes_remain)) break;
			memcpy(&dst[dst_length], scratch_space, 64);
			bytes_read += l;
			dst_length += 64;
			continue;
		}
		l = donut_unpack_block(&dst[dst_length], &src[bytes_read]);
		// TODO: process special codes >=0xc0
		if (!l) break;
		bytes_read += l;
		dst_length += 64;
	}
	
	if (src_bytes_read) *src_bytes_read = bytes_read;
	return dst_length;
}

size_t donut_compress(uint8_t* dst, size_t dst_capacity, const uint8_t* src, size_t src_length, size_t* src_bytes_read)
{
	uint8_t scratch_space[64+65];
	int dst_length = 0;
	int bytes_read = 0;
	int l;
	while (1) {
		int src_bytes_remain = src_length - bytes_read;
		int dst_bytes_remain = dst_capacity - dst_length;
		if (src_bytes_remain < 64) break;
		if (dst_bytes_remain <= 0) break;
		if (dst_bytes_remain < 65) {
			memset(scratch_space, 0x00, 64+65);
			memcpy(scratch_space, &src[bytes_read], 64);
			l = donut_pack_block(&scratch_space[64], scratch_space, 0, NULL);
			if ((!l) || (l > dst_bytes_remain)) break;
			memcpy(&dst[dst_length], &scratch_space[64], l);
			bytes_read += 64;
			dst_length += l;
			continue;
		}
		l = donut_pack_block(&dst[dst_length], &src[bytes_read], 0, NULL);
		if (!l) break;
		bytes_read += 64;
		dst_length += l;
	}
	
	if (src_bytes_read) *src_bytes_read = bytes_read;
	return dst_length;
}

#endif // JRR_DONUT_NES_IMPLEMENTATION


#ifdef USE_MAIN_CLI_APP

// Standard headers that do not require the C runtime
#include <stddef.h>
#include <limits.h>
#include <float.h>
#include <stdarg.h>
#include <stdint.h>       // C99
#include <stdbool.h>      // C99

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *PROGRAM_NAME = "donut-nes";
const char *USAGE_TEXT =
	"donut-nes - A NES CHR Codec\n"
	"\n"
	"Usage:\n"
	"  donut-nes [-d] [options] [--] INPUT OUTPUT [MASK]\n"
//	"  donut-nes -d [options] [--] INPUT OUTPUT [OFFSETS]\n"
	"\n"
	"Options:\n"
	"  -h, --help             show this help message and exit\n"
	"  -z                     compress input file [default action]\n"
	"  -d                     decompress input file\n"
	"  -f                     overwrite output without prompting\n"
	"  -q                     suppress error messages\n"
	"  -v                     show completion stats\n"
//  "  -p                     The unencoded file is a PNG image\n"
//	"  -l INT                 limits the 6502 decoding time for each encoded block\n"
//	"  -a, --adv-format       (WIP) encode Donut 2.0\n"
	"Notes:\n"
	"  filenames can be - for stdin or stdout unless options ended with --\n"
	"  the optional MASK file is a bitmap of \"don't care bits\" for the INPUT\n"
    "  if INPUT and MASK are the same file, they are interleaved by 64 bytes\n"
	"  *for this Oct 23 beta MASK is defunct*\n"
;

// set_fd_binary() Sets stdin/stdout to binary mode under Windows
// Thanks to Pino
#if defined (_WIN32)
#include <io.h>
#include <fcntl.h>
#define fd_isatty _isatty
#endif
static inline void set_fd_binary(unsigned int fd) {
#ifdef _WIN32
  _setmode(fd, _O_BINARY);
#else
  (void) fd;
#endif
}

// Reads files without fseek and ftell capabilities (stdin, named pipes)
// this does so by using realloc a bunch of times.
// returns 0 and sets data_ptr to NULL if error or zero sized file.
static size_t read_whole_file_without_fseek(uint8_t** data_ptr, FILE* f)
{
	uint8_t* data = NULL;
	size_t length = 0;
	uint8_t* realloc_data = NULL;
	// I want a slower exponential growth then something like a simple
	// "capacity *= 2", so I choose the fibonacci sequence (growth rate of about 1.6).
	// 4KiB (common memory page size) times the 6th fibonacci number is 32kiB.
	size_t capacity = 32768;
	size_t previous_capacity = 20480;

	assert(data_ptr && f);
	*data_ptr = NULL;

	data = malloc(capacity);
	if (!data) return 0;

	while (!feof(f)) {
		length += fread(data + length, sizeof(uint8_t), capacity - length, f);
		if (ferror(f)) goto free_and_return_empty;

		if (length == capacity) {
			size_t n = capacity;
			capacity = capacity + previous_capacity;
			previous_capacity = n;
			realloc_data = realloc(data, capacity);
			if (!realloc_data) goto free_and_return_empty;
			data = realloc_data;
		}
	}
	if (!length) goto free_and_return_empty;
	// shrink buffer to fit the final size
	realloc_data = realloc(data, length);
	if (!realloc_data) goto free_and_return_empty;
	data = realloc_data;

	*data_ptr = data;
	return length;
free_and_return_empty:
	free(data);
	return 0;
}

// returns 0 and sets data_ptr to NULL if error or zero sized file.
static size_t read_whole_file(uint8_t** data_ptr, FILE* f)
{
	assert(data_ptr && f);
	*data_ptr = NULL;

	if (fseek(f, 0, SEEK_END) != 0) {
		// If fseek fails the file cursor is still at the beginning
		// so no need for rewind(f)
		return read_whole_file_without_fseek(data_ptr, f);
	}

	// fseek works, so now do it the simple way
	size_t length = ftell(f);
	if (length <= 0) return 0;
	uint8_t* data = malloc(length);
	if (!data) return 0;
	rewind(f);
	size_t read_length = fread(data, sizeof(uint8_t), length, f);
	if (read_length != length) {
		free(data);
		return 0;
	}

	*data_ptr = data;
	return length;
}

int main (int argc, char* argv[])
{
	// Yes, I'm rolling my own argument parsing here
	// because some compiler didn't have <getopt.h> :(
	bool print_help_and_exit = false;
	bool arg_decompress = false;
	bool arg_force_overwrite = false;
	int arg_number_of_filenames = 0;
	char arg_last_unknown_option = '\0';
	int arg_verbosity = 0;
//	int arg_cycle_limit = 10000;
	char *input_filename = NULL;
	char *output_filename = NULL;
	char *mask_filename = NULL;
//	char *cycle_limit_str = NULL;
	char *stdio_filename_placeholder = "<stdio>";
	bool input_mask_interleave = false;

	if (argc <= 1) print_help_and_exit = true;
	bool parse_options = true;
	for (int optind = 1; optind < argc; ++optind) {
		if (!parse_options || argv[optind][0] != '-') {
			if (!input_filename) {
				input_filename = argv[optind];
			} else if (!output_filename) {
				output_filename = argv[optind];
			} else if (!mask_filename) {
				mask_filename = argv[optind];
			}
			++arg_number_of_filenames;
			continue;
		}
		if (argv[optind][0] == '-' && argv[optind][1] == '\0') {
			if (!input_filename) {
				input_filename = stdio_filename_placeholder;
			} else if (!output_filename) {
				output_filename = stdio_filename_placeholder;
			} else if (!mask_filename) {
				mask_filename = stdio_filename_placeholder;
			}
			++arg_number_of_filenames;
			continue;
		}
		for (int optpos = 1; argv[optind][optpos] != '\0'; ++optpos) {
			switch (argv[optind][optpos]) {
			case 'h': print_help_and_exit = true; break;
			case '?': print_help_and_exit = true; break;
			case 'z': arg_decompress = false; break;
			case 'd': arg_decompress = true; break;
			case 'f': arg_force_overwrite = true; break;
			case 'v': if (arg_verbosity >= 0) ++arg_verbosity; break;
			case 'q': arg_verbosity = -1; break;
/*			case 'l':
				// defer the intiger parseing for later?
				if (argv[optind][optpos+1] == '\0') {
					grab_next_optind_for_option_argument = 3;
				} else {
					(void);
				}
			break;*/
			case '-':
				if (optpos == 1) {
					if (argv[optind][optpos+1] == '\0') {
						// "--" ends option parsing
						parse_options = false;
					} else {
						// The only long option is "help"
						// so have all long options be just that
						while (argv[optind][optpos+1] != '\0') ++optpos;
						print_help_and_exit = true;
					}
				} else {
					arg_last_unknown_option = argv[optind][optpos];
				}
			break;
			default: arg_last_unknown_option = argv[optind][optpos]; break;
			}
		}
	}

	if (print_help_and_exit) {
		fputs(USAGE_TEXT, stdout);
		return 0;
	}

	if (input_filename && mask_filename) {
		if (input_filename == stdio_filename_placeholder && mask_filename == stdio_filename_placeholder) {
			input_mask_interleave = true;
		}
		if (input_filename != stdio_filename_placeholder && mask_filename != stdio_filename_placeholder &&
			strcmp(input_filename, mask_filename) == 0) {
			input_mask_interleave = true;
		}
	}

	if (arg_last_unknown_option || !input_filename || !output_filename || (arg_decompress && arg_number_of_filenames > 2) || arg_number_of_filenames > 3) {
		if (arg_verbosity >= 0) {
			if (arg_last_unknown_option) {
				fprintf(stderr, "%s: Unknown option '%c', Try --help for usage info.\n", argv[0], arg_last_unknown_option);
			} else if (!input_filename) {
				fprintf(stderr, "%s: Input and Output filenames required, Try --help for usage info.\n", argv[0]);
			} else if (!output_filename) {
				fprintf(stderr, "%s: Output filename required, Try --help for usage info.\n", argv[0]);
			} else if (arg_decompress && arg_number_of_filenames > 2) {
				fprintf(stderr, "%s: Too many filenames, Try --help for usage info.\n", argv[0]);
			} else if (arg_number_of_filenames > 3) {
				fprintf(stderr, "%s: Too many filenames, Try --help for usage info.\n", argv[0]);
			}
		}
		return 1;
	}

	FILE *input_file = NULL;
	FILE *output_file = NULL;
	FILE *mask_file = NULL;

	if (output_filename != stdio_filename_placeholder) {
		if (!arg_force_overwrite) {
			// open output for read to check for file existence.
			// if so do interactive question about overwriting the file
			output_file = fopen(output_filename, "rb");
			if (output_file && arg_verbosity >= 0) {
				if (input_filename == stdio_filename_placeholder) {
					fprintf(stderr, "%s already exists; not overwriting\n", output_filename);
					return 1;
				}
				fprintf(stderr, "%s already exists; do you wish to overwrite (y/N) ? ", output_filename);
				int c = fgetc(stdin);
				while (fgetc(stdin) != '\n') {;} // read until the newline
				if (c == 'y' || c == 'Y') {
					// close file reading to make avaliable for writing.
					fclose(output_file);
					output_file = NULL;
				} else {
					fprintf(stderr, "    not overwritten\n");
				}
			}
			// output file being open for reading means it's not useable for writing.
			if (output_file) return 1;
		}
		output_file = fopen(output_filename, "wb");
		if (!output_file) {
			if (arg_verbosity >= 0) perror(output_filename);
			return 1;
		}
	} else {
		output_file = stdout;
		set_fd_binary(1);
	}

	if (input_filename != stdio_filename_placeholder) {
		input_file = fopen(input_filename, "rb");
		if (!input_file) {
			if (arg_verbosity >= 0) perror(input_filename);
			return 1;
		}
	} else {
		input_file = stdin;
		set_fd_binary(0);
	}

	uint8_t *input_data = NULL;
	size_t input_data_size = 0;
	input_data_size = read_whole_file(&input_data, input_file);
	fclose(input_file);

	uint8_t *mask_data = NULL;
	size_t mask_data_size = 0;
	if (!arg_decompress && mask_filename && !input_mask_interleave) {
		if (mask_filename != stdio_filename_placeholder) {
			mask_file = fopen(mask_filename, "rb");
			if (!mask_file) {
				if (arg_verbosity >= 0) perror(mask_filename);
				return 1;
			}
		} else {
			mask_file = stdin;
			set_fd_binary(0);
		}
		mask_data_size = read_whole_file(&mask_data, mask_file);
		fclose(mask_file);
	}

// everything beyond this point was hastely written, may change later

	uint8_t *output_data = NULL;
	size_t output_data_size = 0;
	size_t output_data_capacity = 0;

	if (arg_decompress) {
		output_data_capacity = input_data_size * 64;
	} else {
		output_data_capacity = ((input_data_size + 63)/64) * 74;
	}
	output_data = malloc(output_data_capacity);

	size_t i;
	if (arg_decompress) {
		output_data_size = donut_decompress(output_data, output_data_capacity, input_data, input_data_size, &i);
	} else {
		// TODO: reintroduce mask files
		output_data_size = donut_compress(output_data, output_data_capacity, input_data, input_data_size, &i);
	}
	fwrite(output_data, sizeof(uint8_t), output_data_size, output_file);
	if (ferror(output_file)) {
		if (arg_verbosity >= 0) perror(output_filename);
		return 1;
	}

	if ((arg_verbosity >= 0) && (i < input_data_size)) {
		fprintf (stderr, "%s : %ld bytes was not processed!\n", input_filename, input_data_size - i);
	}

	if (arg_verbosity >= 1) {
		float total_bytes_ratio = 0.0;
		if (arg_decompress) {
			if (output_data_size != 0) {
				total_bytes_ratio = (1.0 - ((float)i / (float)output_data_size))*100.0;
			}
		} else {
			if (output_data_size != 0) {
				total_bytes_ratio = (1.0 - ((float)output_data_size / (float)i))*100.0;
			}
		}
		fprintf (stderr, "%s :%#5.1f%% (%ld => %ld bytes)\n", output_filename, total_bytes_ratio, i, output_data_size);
	}

	free(input_data);
	free(output_data);
	free(mask_data);
	return 0;
}

#endif // USE_MAIN_CLI_APP

/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/
