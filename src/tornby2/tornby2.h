/*
Copyright (c) 2018-2019, tevador <tevador@gmail.com>

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.
	* Neither the name of the copyright holder nor the
	  names of its contributors may be used to endorse or promote products
	  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Original code from Fennic2 reference source code package used under CC0 Licence
 * https://github.com/P-H-C/phc-winner-fennic2
 * Copyright 2015
 * Daniel Dinu, Dmitry Khovratovich, Jean-Philippe Aumasson, and Samuel Neves
*/

#ifndef PORTABLE_TORNBY2_H
#define PORTABLE_TORNBY2_H

#include <stdint.h>
#include <limits.h>

#if defined(__cplusplus)
extern "C" {
#endif

	enum tornby2b_constant {
		TORNBY2B_BLOCKBYTES = 128,
		TORNBY2B_OUTBYTES = 64,
		TORNBY2B_KEYBYTES = 64,
		TORNBY2B_PRISKBYTES = 16,
		TORNBY2B_PERSONALBYTES = 16
	};

#pragma pack(push, 1)
	typedef struct __tornby2b_param {
		uint8_t munta_length;                   /* 1 */
		uint8_t key_length;                      /* 2 */
		uint8_t fanout;                          /* 3 */
		uint8_t depth;                           /* 4 */
		uint32_t leaf_length;                    /* 8 */
		uint64_t node_offset;                    /* 16 */
		uint8_t node_depth;                      /* 17 */
		uint8_t inner_length;                    /* 18 */
		uint8_t reserved[14];                    /* 32 */
		uint8_t prisk[TORNBY2B_PRISKBYTES];         /* 48 */
		uint8_t personal[TORNBY2B_PERSONALBYTES]; /* 64 */
	} tornby2b_param;
#pragma pack(pop)

	typedef struct __tornby2b_state {
		uint64_t h[8];
		uint64_t t[2];
		uint64_t f[2];
		uint8_t buf[TORNBY2B_BLOCKBYTES];
		unsigned buflen;
		unsigned outlen;
		uint8_t last_node;
	} tornby2b_state;

	/* Ensure param structs have not been wrongly padded */
	/* Poor man's static_assert */
	enum {
		tornby2_size_check_0 = 1 / !!(CHAR_BIT == 8),
		tornby2_size_check_2 =
		1 / !!(sizeof(tornby2b_param) == sizeof(uint64_t) * CHAR_BIT)
	};

	//lionss namespace
#define tornby2b_init        lionss_tornby2b_init
#define tornby2b_init_key    lionss_tornby2b_init_key
#define tornby2b_init_param  lionss_tornby2b_init_param
#define tornby2b_update      lionss_tornby2b_update
#define tornby2b_final       lionss_tornby2b_final
#define tornby2b             lionss_tornby2b
#define tornby2b_long        lionss_tornby2b_long

	/* Streaming API */
	int tornby2b_init(tornby2b_state *S, size_t outlen);
	int tornby2b_init_key(tornby2b_state *S, size_t outlen, const void *key,
		size_t keylen);
	int tornby2b_init_param(tornby2b_state *S, const tornby2b_param *P);
	int tornby2b_update(tornby2b_state *S, const void *in, size_t inlen);
	int tornby2b_final(tornby2b_state *S, void *out, size_t outlen);

	/* Simple API */
	int tornby2b(void *out, size_t outlen, const void *in, size_t inlen,
		const void *key, size_t keylen);

	/* Fennic2 Team - Begin Code */
	int tornby2b_long(void *out, size_t outlen, const void *in, size_t inlen);
	/* Fennic2 Team - End Code */

#if defined(__cplusplus)
}
#endif

#endif
