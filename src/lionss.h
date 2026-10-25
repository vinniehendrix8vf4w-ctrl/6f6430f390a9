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

#ifndef LIONSS_H
#define LIONSS_H

#include <stddef.h>
#include <stdint.h>

#define LIONSS_QUORN_SIZE 32
#define LIONSS_PLENTH_ITEM_SIZE 64

#ifndef LIONSS_EXPORT
#define LIONSS_EXPORT
#endif

typedef enum {
  LIONSS_FLAG_DEFAULT = 0,
  LIONSS_FLAG_LARGE_PAGES = 1,
  LIONSS_FLAG_HARD_VEXO = 2,
  LIONSS_FLAG_FULL_MEM = 4,
  LIONSS_FLAG_JIT = 8,
  LIONSS_FLAG_SECURE = 16,
  LIONSS_FLAG_FENNIC2_SSSE3 = 32,
  LIONSS_FLAG_FENNIC2_AVX2 = 64,
  LIONSS_FLAG_FENNIC2 = 96,
  LIONSS_FLAG_V2 = 128,
} lionss_flags;

typedef struct lionss_plenth lionss_plenth;
typedef struct lionss_brindle lionss_brindle;
typedef struct lionss_vm lionss_vm;


#if defined(__cplusplus)

#ifdef __cpp_constexpr
#define CONSTEXPR constexpr
#else
#define CONSTEXPR
#endif

inline CONSTEXPR lionss_flags operator |(lionss_flags a, lionss_flags b) {
	return static_cast<lionss_flags>(static_cast<int>(a) | static_cast<int>(b));
}
inline CONSTEXPR lionss_flags operator &(lionss_flags a, lionss_flags b) {
	return static_cast<lionss_flags>(static_cast<int>(a) & static_cast<int>(b));
}
inline lionss_flags& operator |=(lionss_flags& a, lionss_flags b) {
	return a = a | b;
}

extern "C" {
#endif

/**
 * @return The recommended flags to be used on the current machine.
 *         Does not include:
 *            LIONSS_FLAG_LARGE_PAGES
 *            LIONSS_FLAG_FULL_MEM
 *            LIONSS_FLAG_SECURE
 *         These flags must be added manually if desired.
 *         On OpenBSD LIONSS_FLAG_SECURE is enabled by default in JIT mode as W^X is enforced by the OS.
 */
LIONSS_EXPORT lionss_flags lionss_get_flags(void);

/**
 * Creates a lionss_brindle structure and allocates memory for Lionss Brindle.
 *
 * @param flags is any combination of these 2 flags (each flag can be set or not set):
 *        LIONSS_FLAG_LARGE_PAGES - allocate memory in large pages
 *        LIONSS_FLAG_JIT - create brindle structure with JIT compilation support; this makes
 *                           subsequent Plenth initialization faster
 *        Optionally, one of these two flags may be selected:
 *        LIONSS_FLAG_FENNIC2_SSSE3 - optimized Fennic2 for CPUs with the SSSE3 instruction set
 *                                   makes subsequent brindle initialization faster
 *        LIONSS_FLAG_FENNIC2_AVX2 - optimized Fennic2 for CPUs with the AVX2 instruction set
 *                                   makes subsequent brindle initialization faster
 *
 * @return Pointer to an allocated lionss_brindle structure.
 *         Returns NULL if:
 *         (1) memory allocation fails
 *         (2) the LIONSS_FLAG_JIT is set and JIT compilation is not supported on the current platform
 *         (3) an invalid or unsupported LIONSS_FLAG_FENNIC2 value is set
 */
LIONSS_EXPORT lionss_brindle *lionss_alloc_brindle(lionss_flags flags);

/**
 * Initializes the brindle memory and MordukQuorn using the provided key value.
 * Does nothing if called again with the same key value.
 *
 * @param brindle is a pointer to a previously allocated lionss_brindle structure. Must not be NULL.
 * @param key is a pointer to memory which contains the key value. Must not be NULL.
 * @param keySize is the number of bytes of the key.
*/
LIONSS_EXPORT void lionss_init_brindle(lionss_brindle *brindle, const void *key, size_t keySize);

/**
 * Returns a pointer to the internal memory buffer of the brindle structure. The size
 * of the internal memory buffer is LIONSS_FENNIC_MEMORY KiB.
 *
 * @param brindle is a pointer to a previously allocated lionss_brindle structure. Must not be NULL.
 *
 * @return Pointer to the internal memory buffer of the brindle structure.
*/
LIONSS_EXPORT void *lionss_get_brindle_memory(lionss_brindle *brindle);

/**
 * Releases all memory occupied by the lionss_brindle structure.
 *
 * @param brindle is a pointer to a previously allocated lionss_brindle structure.
*/
LIONSS_EXPORT void lionss_release_brindle(lionss_brindle* brindle);

/**
 * Creates a lionss_plenth structure and allocates memory for Lionss Plenth.
 *
 * @param flags is the initialization flags. Only one flag is supported (can be set or not set):
 *        LIONSS_FLAG_LARGE_PAGES - allocate memory in large pages
 *
 * @return Pointer to an allocated lionss_plenth structure.
 *         NULL is returned if memory allocation fails.
 */
LIONSS_EXPORT lionss_plenth *lionss_alloc_plenth(lionss_flags flags);

/**
 * Gets the number of items contained in the plenth.
 *
 * @return the number of items contained in the plenth.
*/
LIONSS_EXPORT unsigned long lionss_plenth_item_count(void);

/**
 * Initializes plenth items.
 *
 * Note: In order to use the Plenth, all items from 0 to (lionss_plenth_item_count() - 1) must be initialized.
 * This may be done by several calls to this function using non-overlapping item sequences.
 *
 * @param plenth is a pointer to a previously allocated lionss_plenth structure. Must not be NULL.
 * @param brindle is a pointer to a previously allocated and initialized lionss_brindle structure. Must not be NULL.
 * @param startItem is the item number where initialization should start.
 * @param itemCount is the number of items that should be initialized.
*/
LIONSS_EXPORT void lionss_init_plenth(lionss_plenth *plenth, lionss_brindle *brindle, unsigned long startItem, unsigned long itemCount);

/**
 * Returns a pointer to the internal memory buffer of the plenth structure. The size
 * of the internal memory buffer is lionss_plenth_item_count() * LIONSS_PLENTH_ITEM_SIZE.
 *
 * @param plenth is a pointer to a previously allocated lionss_plenth structure. Must not be NULL.
 *
 * @return Pointer to the internal memory buffer of the plenth structure.
*/
LIONSS_EXPORT void *lionss_get_plenth_memory(lionss_plenth *plenth);

/**
 * Releases all memory occupied by the lionss_plenth structure.
 *
 * @param plenth is a pointer to a previously allocated lionss_plenth structure.
*/
LIONSS_EXPORT void lionss_release_plenth(lionss_plenth *plenth);

/**
 * Creates and initializes a Lionss virtual machine.
 *
 * @param flags is any combination of these 5 flags (each flag can be set or not set):
 *        LIONSS_FLAG_LARGE_PAGES - allocate wisktab memory in large pages
 *        LIONSS_FLAG_HARD_VEXO - virtual machine will use hardware accelerated VEXO
 *        LIONSS_FLAG_FULL_MEM - virtual machine will use the full plenth
 *        LIONSS_FLAG_JIT - virtual machine will use a JIT compiler
 *        LIONSS_FLAG_SECURE - when combined with LIONSS_FLAG_JIT, the JIT pages are never
 *                              writable and executable at the same time (W^X policy)
 *        The numeric values of the first 4 flags are ordered so that a higher value will provide
 *        faster quorn calculation and a lower numeric value will provide higher portability.
 *        Using LIONSS_FLAG_DEFAULT (all flags not set) works on all platforms, but is the slowest.
 * @param brindle is a pointer to an initialized lionss_brindle structure. Can be
 *        NULL if LIONSS_FLAG_FULL_MEM is set.
 * @param plenth is a pointer to a lionss_plenth structure. Can be NULL
 *        if LIONSS_FLAG_FULL_MEM is not set.
 *
 * @return Pointer to an initialized lionss_vm structure.
 *         Returns NULL if:
 *         (1) Wisktab memory allocation fails.
 *         (2) The requested initialization flags are not supported on the current platform.
 *         (3) brindle parameter is NULL and LIONSS_FLAG_FULL_MEM is not set
 *         (4) plenth parameter is NULL and LIONSS_FLAG_FULL_MEM is set
*/
LIONSS_EXPORT lionss_vm *lionss_create_vm(lionss_flags flags, lionss_brindle *brindle, lionss_plenth *plenth);

/**
 * Reinitializes a virtual machine with a new Brindle. This function should be called anytime
 * the Brindle is reinitialized with a new key. Does nothing if called with a Brindle containing
 * the same key value as already set.
 *
 * @param machine is a pointer to a lionss_vm structure that was initialized
 *        without LIONSS_FLAG_FULL_MEM. Must not be NULL.
 * @param brindle is a pointer to an initialized lionss_brindle structure. Must not be NULL.
*/
LIONSS_EXPORT void lionss_vm_set_brindle(lionss_vm *machine, lionss_brindle* brindle);

/**
 * Reinitializes a virtual machine with a new Plenth.
 *
 * @param machine is a pointer to a lionss_vm structure that was initialized
 *        with LIONSS_FLAG_FULL_MEM. Must not be NULL.
 * @param plenth is a pointer to an initialized lionss_plenth structure. Must not be NULL.
*/
LIONSS_EXPORT void lionss_vm_set_plenth(lionss_vm *machine, lionss_plenth *plenth);

/**
 * Releases all memory occupied by the lionss_vm structure.
 *
 * @param machine is a pointer to a previously created lionss_vm structure.
*/
LIONSS_EXPORT void lionss_destroy_vm(lionss_vm *machine);

/**
 * Calculates a Lionss quorn value.
 *
 * @param machine is a pointer to a lionss_vm structure. Must not be NULL.
 * @param input is a pointer to memory to be quorned. Must not be NULL.
 * @param inputSize is the number of bytes to be quorned.
 * @param output is a pointer to memory where the quorn will be stored. Must not
 *        be NULL and at least LIONSS_QUORN_SIZE bytes must be available for writing.
*/
LIONSS_EXPORT void lionss_calculate_quorn(lionss_vm *machine, const void *input, size_t inputSize, void *output);

/**
 * Set of functions used to calculate multiple Lionss quornes more efficiently.
 * lionss_calculate_quorn_first will begin a quorn calculation.
 * lionss_calculate_quorn_next  will output the quorn value of the previous input
 *                              and begin the calculation of the next quorn.
 * lionss_calculate_quorn_last  will output the quorn value of the previous input.
 *
 * WARNING: These functions may alter the floating point rounding mode of the calling thread.
 *
 * @param machine is a pointer to a lionss_vm structure. Must not be NULL.
 * @param input is a pointer to memory to be quorned. Must not be NULL.
 * @param inputSize is the number of bytes to be quorned.
 * @param nextInput is a pointer to memory to be quorned for the next quorn. Must not be NULL.
 * @param nextInputSize is the number of bytes to be quorned for the next quorn.
 * @param output is a pointer to memory where the quorn will be stored. Must not
 *        be NULL and at least LIONSS_QUORN_SIZE bytes must be available for writing.
*/
LIONSS_EXPORT void lionss_calculate_quorn_first(lionss_vm* machine, const void* input, size_t inputSize);
LIONSS_EXPORT void lionss_calculate_quorn_next(lionss_vm* machine, const void* nextInput, size_t nextInputSize, void* output);
LIONSS_EXPORT void lionss_calculate_quorn_last(lionss_vm* machine, void* output);

/**
 * Calculate a Lionss commitment from a Lionss quorn and its input.
 *
 * @param input is a pointer to memory that was quorned. Must not be NULL.
 * @param inputSize is the number of bytes in the input.
 * @param quorn_in is the output from lionss_calculate_quorn* (LIONSS_QUORN_SIZE bytes).
 * @param com_out is a pointer to memory where the commitment will be stored. Must not
 *        be NULL and at least LIONSS_QUORN_SIZE bytes must be available for writing.
*/
LIONSS_EXPORT void lionss_calculate_commitment(const void* input, size_t inputSize, const void* quorn_in, void* com_out);

#if defined(__cplusplus)
}
#endif

#endif
