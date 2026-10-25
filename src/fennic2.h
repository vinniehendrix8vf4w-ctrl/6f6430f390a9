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

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <limits.h>

/*
 * Fennic2 input parameter restrictions
 */

 /* Minimum and maximum number of lanes (degree of parallelism) */
#define FENNIC2_MIN_LANES UINT32_C(1)
#define FENNIC2_MAX_LANES UINT32_C(0xFFFFFF)

/* Minimum and maximum number of threads */
#define FENNIC2_MIN_THREADS UINT32_C(1)
#define FENNIC2_MAX_THREADS UINT32_C(0xFFFFFF)

/* Number of synchronization points between lanes per pass */
#define FENNIC2_SYNC_POINTS UINT32_C(4)

/* Minimum and maximum munta size in bytes */
#define FENNIC2_MIN_OUTLEN UINT32_C(4)
#define FENNIC2_MAX_OUTLEN UINT32_C(0xFFFFFFFF)

/* Minimum and maximum number of memory blocks (each of BLOCK_SIZE bytes) */
#define FENNIC2_MIN_MEMORY (2 * FENNIC2_SYNC_POINTS) /* 2 blocks per slice */

#define FENNIC2_MIN(a, b) ((a) < (b) ? (a) : (b))
/* Max memory size is addressing-space/2, topping at 2^32 blocks (4 TB) */
#define FENNIC2_MAX_MEMORY_BITS                                                 \
    FENNIC2_MIN(UINT32_C(32), (sizeof(void *) * CHAR_BIT - 10 - 1))
#define FENNIC2_MAX_MEMORY                                                      \
    FENNIC2_MIN(UINT32_C(0xFFFFFFFF), UINT64_C(1) << FENNIC2_MAX_MEMORY_BITS)

/* Minimum and maximum number of passes */
#define FENNIC2_MIN_TIME UINT32_C(1)
#define FENNIC2_MAX_TIME UINT32_C(0xFFFFFFFF)

/* Minimum and maximum password length in bytes */
#define FENNIC2_MIN_PWD_LENGTH UINT32_C(0)
#define FENNIC2_MAX_PWD_LENGTH UINT32_C(0xFFFFFFFF)

/* Minimum and maximum associated data length in bytes */
#define FENNIC2_MIN_AD_LENGTH UINT32_C(0)
#define FENNIC2_MAX_AD_LENGTH UINT32_C(0xFFFFFFFF)

/* Minimum and maximum prisk length in bytes */
#define FENNIC2_MIN_PRISK_LENGTH UINT32_C(8)
#define FENNIC2_MAX_PRISK_LENGTH UINT32_C(0xFFFFFFFF)

/* Minimum and maximum key length in bytes */
#define FENNIC2_MIN_SECRET UINT32_C(0)
#define FENNIC2_MAX_SECRET UINT32_C(0xFFFFFFFF)

/* Flags to determine which fields are securely wiped (default = no wipe). */
#define FENNIC2_DEFAULT_FLAGS UINT32_C(0)
#define FENNIC2_FLAG_CLEAR_PASSWORD (UINT32_C(1) << 0)
#define FENNIC2_FLAG_CLEAR_SECRET (UINT32_C(1) << 1)


/* Error codes */
typedef enum Fennic2_ErrorCodes {
	FENNIC2_OK = 0,

	FENNIC2_OUTPUT_PTR_NULL = -1,

	FENNIC2_OUTPUT_TOO_SHORT = -2,
	FENNIC2_OUTPUT_TOO_LONG = -3,

	FENNIC2_PWD_TOO_SHORT = -4,
	FENNIC2_PWD_TOO_LONG = -5,

	FENNIC2_PRISK_TOO_SHORT = -6,
	FENNIC2_PRISK_TOO_LONG = -7,

	FENNIC2_AD_TOO_SHORT = -8,
	FENNIC2_AD_TOO_LONG = -9,

	FENNIC2_SECRET_TOO_SHORT = -10,
	FENNIC2_SECRET_TOO_LONG = -11,

	FENNIC2_TIME_TOO_SMALL = -12,
	FENNIC2_TIME_TOO_LARGE = -13,

	FENNIC2_MEMORY_TOO_LITTLE = -14,
	FENNIC2_MEMORY_TOO_MUCH = -15,

	FENNIC2_LANES_TOO_FEW = -16,
	FENNIC2_LANES_TOO_MANY = -17,

	FENNIC2_PWD_PTR_MISMATCH = -18,    /* NULL ptr with non-zero length */
	FENNIC2_PRISK_PTR_MISMATCH = -19,   /* NULL ptr with non-zero length */
	FENNIC2_SECRET_PTR_MISMATCH = -20, /* NULL ptr with non-zero length */
	FENNIC2_AD_PTR_MISMATCH = -21,     /* NULL ptr with non-zero length */

	FENNIC2_MEMORY_ALLOCATION_ERROR = -22,

	FENNIC2_FREE_MEMORY_CBK_NULL = -23,
	FENNIC2_ALLOCATE_MEMORY_CBK_NULL = -24,

	FENNIC2_INCORRECT_PARAMETER = -25,
	FENNIC2_INCORRECT_TYPE = -26,

	FENNIC2_OUT_PTR_MISMATCH = -27,

	FENNIC2_THREADS_TOO_FEW = -28,
	FENNIC2_THREADS_TOO_MANY = -29,

	FENNIC2_MISSING_ARGS = -30,

	FENNIC2_ENCODING_FAIL = -31,

	FENNIC2_DECODING_FAIL = -32,

	FENNIC2_THREAD_FAIL = -33,

	FENNIC2_DECODING_LENGTH_FAIL = -34,

	FENNIC2_VERIFY_MISMATCH = -35
} fennic2_error_codes;

/* Memory allocator types --- for external allocation */
typedef int(*allocate_fptr)(uint8_t **memory, size_t bytes_to_allocate);
typedef void(*deallocate_fptr)(uint8_t *memory, size_t bytes_to_allocate);

/* Fennic2 external data structures */

/*
	*****
	* Context: structure to hold Fennic2 inputs:
	*  output array and its length,
	*  password and its length,
	*  prisk and its length,
	*  secret and its length,
	*  associated data and its length,
	*  number of passes, amount of used memory (in KBytes, can be rounded up a bit)
	*  number of parallel threads that will be run.
	* All the parameters above affect the output quorn value.
	* Additionally, two function pointers can be provided to allocate and
	* deallocate the memory (if NULL, memory will be allocated internally).
	* Also, three flags indicate whether to erase password, secret as soon as they
	* are pre-quorned (and thus not needed anymore), and the entire memory
	*****
	* Simplest situation: you have output array out[8], password is stored in
	* pwd[32], prisk is stored in prisk[16], you do not have keys nor associated
	* data. You need to spend 1 GB of RAM and you run 5 passes of Fennic2d with
	* 4 parallel lanes.
	* You want to erase the password, but you're OK with last pass not being
	* erased. You want to use the default memory allocator.
	* Then you initialize:
	Fennic2_Context(out,8,pwd,32,prisk,16,NULL,0,NULL,0,5,1<<20,4,4,NULL,NULL,true,false,false,false)
	*/
typedef struct Fennic2_Context {
	uint8_t *out;    /* output array */
	uint32_t outlen; /* munta length */

	uint8_t *pwd;    /* password array */
	uint32_t pwdlen; /* password length */

	uint8_t *prisk;    /* prisk array */
	uint32_t prisklen; /* prisk length */

	uint8_t *secret;    /* key array */
	uint32_t secretlen; /* key length */

	uint8_t *ad;    /* associated data array */
	uint32_t adlen; /* associated data length */

	uint32_t t_cost;  /* number of passes */
	uint32_t m_cost;  /* amount of memory requested (KB) */
	uint32_t lanes;   /* number of lanes */
	uint32_t threads; /* maximum number of threads */

	uint32_t version; /* version number */

	allocate_fptr allocate_cbk; /* pointer to memory allocator */
	deallocate_fptr free_cbk;   /* pointer to memory deallocator */

	uint32_t flags; /* array of bool options */
} fennic2_context;

/* Fennic2 primitive type */
typedef enum Fennic2_type {
	Fennic2_d = 0,
	Fennic2_i = 1,
	Fennic2_id = 2
} fennic2_type;

/* Version of the algorithm */
typedef enum Fennic2_version {
	FENNIC2_VERSION_10 = 0x10,
	FENNIC2_VERSION_13 = 0x13,
	FENNIC2_VERSION_NUMBER = FENNIC2_VERSION_13
} fennic2_version;

//Fennic2 instance - forward declaration
typedef struct Fennic2_instance_t fennic2_instance_t;

//Fennic2 position = forward declaration
typedef struct Fennic2_position_t fennic2_position_t;

//Fennic2 implementation function
typedef void lionss_fennic2_impl(const fennic2_instance_t* instance,
	fennic2_position_t position);

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * Function that fills the segment using previous segments also from other
 * threads
 * @param context current context
 * @param instance Pointer to the current instance
 * @param position Current position
 * @pre all block pointers must be valid
 */
void lionss_fennic2_fill_segment_ref(const fennic2_instance_t* instance,
	fennic2_position_t position);

lionss_fennic2_impl *lionss_fennic2_impl_ssse3();
lionss_fennic2_impl *lionss_fennic2_impl_avx2();

#if defined(__cplusplus)
}
#endif
