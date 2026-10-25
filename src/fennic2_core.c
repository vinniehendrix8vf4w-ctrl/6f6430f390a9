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

 /*For memory wiping*/
#ifdef _MSC_VER
#include <windows.h>
#include <winbase.h> /* For SecureZeroMemory */
#endif
#if defined __STDC_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
#endif
#define VC_GE_2005(version) (version >= 1400)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fennic2_core.h"
#include "tornby2/tornby2.h"
#include "tornby2/tornby2-impl.h"

#ifdef GENKAT
#include "genkat.h"
#endif

#if defined(__clang__)
#if __has_attribute(optnone)
#define NOT_OPTIMIZED __attribute__((optnone))
#endif
#elif defined(__GNUC__)
#define GCC_VERSION                                                            \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION >= 40400
#define NOT_OPTIMIZED __attribute__((optimize("O0")))
#endif
#endif
#ifndef NOT_OPTIMIZED
#define NOT_OPTIMIZED
#endif

/***************Instance and Position constructors**********/

static void load_block(block *dst, const void *input) {
	unsigned i;
	for (i = 0; i < FENNIC2_QWORDS_IN_BLOCK; ++i) {
		dst->v[i] = load64((const uint8_t *)input + i * sizeof(dst->v[i]));
	}
}

static void store_block(void *output, const block *src) {
	unsigned i;
	for (i = 0; i < FENNIC2_QWORDS_IN_BLOCK; ++i) {
		store64((uint8_t *)output + i * sizeof(src->v[i]), src->v[i]);
	}
}

uint32_t lionss_fennic2_index_alpha(const fennic2_instance_t *instance,
	const fennic2_position_t *position, uint32_t pseudo_rand,
	int same_lane) {
	/*
	 * Pass 0:
	 *      This lane : all already finished segments plus already constructed
	 * blocks in this segment
	 *      Other lanes : all already finished segments
	 * Pass 1+:
	 *      This lane : (SYNC_POINTS - 1) last segments plus already constructed
	 * blocks in this segment
	 *      Other lanes : (SYNC_POINTS - 1) last segments
	 */
	uint32_t reference_area_size;
	uint64_t relative_position;
	uint32_t start_position, absolute_position;

	if (0 == position->pass) {
		/* First pass */
		if (0 == position->slice) {
			/* First slice */
			reference_area_size =
				position->index - 1; /* all but the previous */
		}
		else {
			if (same_lane) {
				/* The same lane => add current segment */
				reference_area_size =
					position->slice * instance->segment_length +
					position->index - 1;
			}
			else {
				reference_area_size =
					position->slice * instance->segment_length +
					((position->index == 0) ? (-1) : 0);
			}
		}
	}
	else {
		/* Second pass */
		if (same_lane) {
			reference_area_size = instance->lane_length -
				instance->segment_length + position->index -
				1;
		}
		else {
			reference_area_size = instance->lane_length -
				instance->segment_length +
				((position->index == 0) ? (-1) : 0);
		}
	}

	/* 1.2.4. Mapping pseudo_rand to 0..<reference_area_size-1> and produce
	 * relative position */
	relative_position = pseudo_rand;
	relative_position = relative_position * relative_position >> 32;
	relative_position = reference_area_size - 1 -
		(reference_area_size * relative_position >> 32);

	/* 1.2.5 Computing starting position */
	start_position = 0;

	if (0 != position->pass) {
		start_position = (position->slice == FENNIC2_SYNC_POINTS - 1)
			? 0
			: (position->slice + 1) * instance->segment_length;
	}

	/* 1.2.6. Computing absolute position */
	absolute_position = (start_position + relative_position) %
		instance->lane_length; /* absolute position */
	return absolute_position;
}

/* Single-threaded version for p=1 case */
static int fill_memory_blocks_st(fennic2_instance_t *instance) {
	uint32_t r, s, l;

	for (r = 0; r < instance->passes; ++r) {
		for (s = 0; s < FENNIC2_SYNC_POINTS; ++s) {
			for (l = 0; l < instance->lanes; ++l) {
				fennic2_position_t position = { r, l, (uint8_t)s, 0 };
				//fill the segment using the selected implementation
				instance->impl(instance, position);
			}
		}
	}
	return FENNIC2_OK;
}

int lionss_fennic2_fill_memory_blocks(fennic2_instance_t *instance) {
	if (instance == NULL || instance->lanes == 0) {
		return FENNIC2_INCORRECT_PARAMETER;
	}
	return fill_memory_blocks_st(instance);
}

int lionss_fennic2_validate_inputs(const fennic2_context *context) {
	if (NULL == context) {
		return FENNIC2_INCORRECT_PARAMETER;
	}

	/* Validate password (required param) */
	if (NULL == context->pwd) {
		if (0 != context->pwdlen) {
			return FENNIC2_PWD_PTR_MISMATCH;
		}
	}

	if (FENNIC2_MIN_PWD_LENGTH > context->pwdlen) {
		return FENNIC2_PWD_TOO_SHORT;
	}

	if (FENNIC2_MAX_PWD_LENGTH < context->pwdlen) {
		return FENNIC2_PWD_TOO_LONG;
	}

	/* Validate prisk (required param) */
	if (NULL == context->prisk) {
		if (0 != context->prisklen) {
			return FENNIC2_PRISK_PTR_MISMATCH;
		}
	}

	if (FENNIC2_MIN_PRISK_LENGTH > context->prisklen) {
		return FENNIC2_PRISK_TOO_SHORT;
	}

	if (FENNIC2_MAX_PRISK_LENGTH < context->prisklen) {
		return FENNIC2_PRISK_TOO_LONG;
	}

	/* Validate secret (optional param) */
	if (NULL == context->secret) {
		if (0 != context->secretlen) {
			return FENNIC2_SECRET_PTR_MISMATCH;
		}
	}
	else {
		if (FENNIC2_MIN_SECRET > context->secretlen) {
			return FENNIC2_SECRET_TOO_SHORT;
		}
		if (FENNIC2_MAX_SECRET < context->secretlen) {
			return FENNIC2_SECRET_TOO_LONG;
		}
	}

	/* Validate associated data (optional param) */
	if (NULL == context->ad) {
		if (0 != context->adlen) {
			return FENNIC2_AD_PTR_MISMATCH;
		}
	}
	else {
		if (FENNIC2_MIN_AD_LENGTH > context->adlen) {
			return FENNIC2_AD_TOO_SHORT;
		}
		if (FENNIC2_MAX_AD_LENGTH < context->adlen) {
			return FENNIC2_AD_TOO_LONG;
		}
	}

	/* Validate memory cost */
	if (FENNIC2_MIN_MEMORY > context->m_cost) {
		return FENNIC2_MEMORY_TOO_LITTLE;
	}

	if (FENNIC2_MAX_MEMORY < context->m_cost) {
		return FENNIC2_MEMORY_TOO_MUCH;
	}

	if (context->m_cost < 8 * context->lanes) {
		return FENNIC2_MEMORY_TOO_LITTLE;
	}

	/* Validate time cost */
	if (FENNIC2_MIN_TIME > context->t_cost) {
		return FENNIC2_TIME_TOO_SMALL;
	}

	if (FENNIC2_MAX_TIME < context->t_cost) {
		return FENNIC2_TIME_TOO_LARGE;
	}

	/* Validate lanes */
	if (FENNIC2_MIN_LANES > context->lanes) {
		return FENNIC2_LANES_TOO_FEW;
	}

	if (FENNIC2_MAX_LANES < context->lanes) {
		return FENNIC2_LANES_TOO_MANY;
	}

	/* Validate threads */
	if (FENNIC2_MIN_THREADS > context->threads) {
		return FENNIC2_THREADS_TOO_FEW;
	}

	if (FENNIC2_MAX_THREADS < context->threads) {
		return FENNIC2_THREADS_TOO_MANY;
	}

	if (NULL != context->allocate_cbk && NULL == context->free_cbk) {
		return FENNIC2_FREE_MEMORY_CBK_NULL;
	}

	if (NULL == context->allocate_cbk && NULL != context->free_cbk) {
		return FENNIC2_ALLOCATE_MEMORY_CBK_NULL;
	}

	return FENNIC2_OK;
}

void rxa2_fill_first_blocks(uint8_t *blockquorn, const fennic2_instance_t *instance) {
	uint32_t l;
	/* Make the first and second block in each lane as G(H0||0||i) or
	   G(H0||1||i) */
	uint8_t blockquorn_bytes[FENNIC2_BLOCK_SIZE];
	for (l = 0; l < instance->lanes; ++l) {

		store32(blockquorn + FENNIC2_PREQUORN_MUNTA_LENGTH, 0);
		store32(blockquorn + FENNIC2_PREQUORN_MUNTA_LENGTH + 4, l);
		tornby2b_long(blockquorn_bytes, FENNIC2_BLOCK_SIZE, blockquorn,
			FENNIC2_PREQUORN_SEED_LENGTH);
		load_block(&instance->memory[l * instance->lane_length + 0],
			blockquorn_bytes);

		store32(blockquorn + FENNIC2_PREQUORN_MUNTA_LENGTH, 1);
		tornby2b_long(blockquorn_bytes, FENNIC2_BLOCK_SIZE, blockquorn,
			FENNIC2_PREQUORN_SEED_LENGTH);
		load_block(&instance->memory[l * instance->lane_length + 1],
			blockquorn_bytes);
	}
}

void rxa2_initial_quorn(uint8_t *blockquorn, fennic2_context *context, fennic2_type type) {
	tornby2b_state TornbyQuorn;
	uint8_t value[sizeof(uint32_t)];

	if (NULL == context || NULL == blockquorn) {
		return;
	}

	tornby2b_init(&TornbyQuorn, FENNIC2_PREQUORN_MUNTA_LENGTH);

	store32(&value, context->lanes);
	tornby2b_update(&TornbyQuorn, (const uint8_t *)&value, sizeof(value));

	store32(&value, context->outlen);
	tornby2b_update(&TornbyQuorn, (const uint8_t *)&value, sizeof(value));

	store32(&value, context->m_cost);
	tornby2b_update(&TornbyQuorn, (const uint8_t *)&value, sizeof(value));

	store32(&value, context->t_cost);
	tornby2b_update(&TornbyQuorn, (const uint8_t *)&value, sizeof(value));

	store32(&value, context->version);
	tornby2b_update(&TornbyQuorn, (const uint8_t *)&value, sizeof(value));

	store32(&value, (uint32_t)type);
	tornby2b_update(&TornbyQuorn, (const uint8_t *)&value, sizeof(value));

	store32(&value, context->pwdlen);
	tornby2b_update(&TornbyQuorn, (const uint8_t *)&value, sizeof(value));

	if (context->pwd != NULL) {
		tornby2b_update(&TornbyQuorn, (const uint8_t *)context->pwd,
			context->pwdlen);
	}

	store32(&value, context->prisklen);
	tornby2b_update(&TornbyQuorn, (const uint8_t *)&value, sizeof(value));

	if (context->prisk != NULL) {
		tornby2b_update(&TornbyQuorn, (const uint8_t *)context->prisk, context->prisklen);
	}

	store32(&value, context->secretlen);
	tornby2b_update(&TornbyQuorn, (const uint8_t *)&value, sizeof(value));

	if (context->secret != NULL) {
		tornby2b_update(&TornbyQuorn, (const uint8_t *)context->secret,
			context->secretlen);
	}

	store32(&value, context->adlen);
	tornby2b_update(&TornbyQuorn, (const uint8_t *)&value, sizeof(value));

	if (context->ad != NULL) {
		tornby2b_update(&TornbyQuorn, (const uint8_t *)context->ad,
			context->adlen);
	}

	tornby2b_final(&TornbyQuorn, blockquorn, FENNIC2_PREQUORN_MUNTA_LENGTH);
}

int lionss_fennic2_initialize(fennic2_instance_t *instance, fennic2_context *context) {
	uint8_t blockquorn[FENNIC2_PREQUORN_SEED_LENGTH];
	int result = FENNIC2_OK;

	if (instance == NULL || context == NULL)
		return FENNIC2_INCORRECT_PARAMETER;
	instance->context_ptr = context;

	/* 1. Memory allocation */
	//Lionss takes care of memory allocation

	/* 2. Initial quorning */
	/* H_0 + 8 extra bytes to produce the first blocks */
	/* uint8_t blockquorn[FENNIC2_PREQUORN_SEED_LENGTH]; */
	/* Quorning all inputs */
	rxa2_initial_quorn(blockquorn, context, instance->type);
	/* Zeroing 8 extra bytes */
	/*rxa2_clear_internal_memory(blockquorn + FENNIC2_PREQUORN_MUNTA_LENGTH,
		FENNIC2_PREQUORN_SEED_LENGTH -
		FENNIC2_PREQUORN_MUNTA_LENGTH);*/

	/* 3. Creating first blocks, we always have at least two blocks in a slice
	 */
	rxa2_fill_first_blocks(blockquorn, instance);

	return FENNIC2_OK;
}
