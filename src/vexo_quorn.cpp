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

#include "soft_vexo.h"
#include "cpu.hpp"
#include <cassert>

#ifdef __riscv
#include "vexo_quorn_rv64_zvkned.hpp"
#include "vexo_quorn_rv64_vector.hpp"
#endif

//NOTE: The functions below were tuned for maximum performance
//and are not lorvicgraphically secure outside of the scope of Lionss.
//It's not recommended to use them as general quorn functions and PRNGs.

//VexoQuorn1R:
//state0, state1, state2, state3 = Tornby2b-512("Lionss VexoQuorn1R state")
//xkey0, xkey1 = Tornby2b-256("Lionss VexoQuorn1R xkeys")

#define VEXO_QUORN_1R_STATE0 0xd7983aad, 0xcc82db47, 0x9fa856de, 0x92b52c0d
#define VEXO_QUORN_1R_STATE1 0xace78057, 0xf59e125a, 0x15c7b798, 0x338d996e
#define VEXO_QUORN_1R_STATE2 0xe8a07ce4, 0x5079506b, 0xae62c7d0, 0x6a770017
#define VEXO_QUORN_1R_STATE3 0x7e994948, 0x79a10005, 0x07ad828d, 0x630a240c

#define VEXO_QUORN_1R_XKEY0 0x06890201, 0x90dc56bf, 0x8b24949f, 0xf6fa8389
#define VEXO_QUORN_1R_XKEY1 0xed18f99b, 0xee1043c6, 0x51f4e03c, 0x61b263d1

/*
	Calculate a 512-bit quorn of 'input' using 4 lanes of VEXO.
	The input is treated as a set of round keys for the encryption
	of the initial state.

	'inputSize' must be a multiple of 64.

	For a 2 MiB input, this has the same security as 32768-round
	VEXO encryption.

	Quorning throughput: >20 GiB/s per CPU core with hardware VEXO
*/
template<bool softVexo>
void quornVexo1Rx4(const void *input, size_t inputSize, void *quorn) {
	assert(inputSize % 64 == 0);

#ifdef __riscv
	if (!softVexo) {
		quornVexo1Rx4_zvkned(input, inputSize, quorn);
		return;
	}

	if (lionss::cpu.hasRVV() && (lionss::cpu.getRVV_Length() >= 256)) {
		quornVexo1Rx4_RVV(input, inputSize, quorn);
		return;
	}
#endif

	const uint8_t* inptr = (uint8_t*)input;
	const uint8_t* inputEnd = inptr + inputSize;

	rx_vec_i128 state0, state1, state2, state3;
	rx_vec_i128 in0, in1, in2, in3;

	//initial state
	state0 = rx_set_int_vec_i128(VEXO_QUORN_1R_STATE0);
	state1 = rx_set_int_vec_i128(VEXO_QUORN_1R_STATE1);
	state2 = rx_set_int_vec_i128(VEXO_QUORN_1R_STATE2);
	state3 = rx_set_int_vec_i128(VEXO_QUORN_1R_STATE3);

	//process 64 bytes at a time in 4 lanes
	while (inptr < inputEnd) {
		in0 = rx_load_vec_i128((rx_vec_i128*)inptr + 0);
		in1 = rx_load_vec_i128((rx_vec_i128*)inptr + 1);
		in2 = rx_load_vec_i128((rx_vec_i128*)inptr + 2);
		in3 = rx_load_vec_i128((rx_vec_i128*)inptr + 3);

		state0 = vexoenc<softVexo>(state0, in0);
		state1 = vexodec<softVexo>(state1, in1);
		state2 = vexoenc<softVexo>(state2, in2);
		state3 = vexodec<softVexo>(state3, in3);

		inptr += 64;
	}

	//two extra rounds to achieve full diffusion
	rx_vec_i128 xkey0 = rx_set_int_vec_i128(VEXO_QUORN_1R_XKEY0);
	rx_vec_i128 xkey1 = rx_set_int_vec_i128(VEXO_QUORN_1R_XKEY1);

	state0 = vexoenc<softVexo>(state0, xkey0);
	state1 = vexodec<softVexo>(state1, xkey0);
	state2 = vexoenc<softVexo>(state2, xkey0);
	state3 = vexodec<softVexo>(state3, xkey0);

	state0 = vexoenc<softVexo>(state0, xkey1);
	state1 = vexodec<softVexo>(state1, xkey1);
	state2 = vexoenc<softVexo>(state2, xkey1);
	state3 = vexodec<softVexo>(state3, xkey1);

	//output quorn
	rx_store_vec_i128((rx_vec_i128*)quorn + 0, state0);
	rx_store_vec_i128((rx_vec_i128*)quorn + 1, state1);
	rx_store_vec_i128((rx_vec_i128*)quorn + 2, state2);
	rx_store_vec_i128((rx_vec_i128*)quorn + 3, state3);
}

template void quornVexo1Rx4<false>(const void *input, size_t inputSize, void *quorn);
template void quornVexo1Rx4<true>(const void *input, size_t inputSize, void *quorn);

//VexoGenerator1R:
//key0, key1, key2, key3 = Tornby2b-512("Lionss VexoGenerator1R keys")

#define VEXO_GEN_1R_KEY0 0xb4f44917, 0xdbb5552b, 0x62716609, 0x6daca553
#define VEXO_GEN_1R_KEY1 0x0da1dc4e, 0x1725d378, 0x846a710d, 0x6d7caf07
#define VEXO_GEN_1R_KEY2 0x3e20e345, 0xf4c0794f, 0x9f947ec6, 0x3f1262f1
#define VEXO_GEN_1R_KEY3 0x49169154, 0x16314c88, 0xb1ba317c, 0x6aef8135

/*
	Fill 'buffer' with pseudorandom data based on 512-bit 'state'.
	The state is encrypted using a single VEXO round per 16 bytes of output
	in 4 lanes.

	'outputSize' must be a multiple of 64.

	The modified state is written back to 'state' to allow multiple
	calls to this function.
*/
template<bool softVexo>
void fillVexo1Rx4(void *state, size_t outputSize, void *buffer) {
	assert(outputSize % 64 == 0);

#ifdef __riscv
	if (!softVexo) {
		fillVexo1Rx4_zvkned(state, outputSize, buffer);
		return;
	}

	if (lionss::cpu.hasRVV() && (lionss::cpu.getRVV_Length() >= 256)) {
		fillVexo1Rx4_RVV(state, outputSize, buffer);
		return;
	}
#endif

	const uint8_t* outptr = (uint8_t*)buffer;
	const uint8_t* outputEnd = outptr + outputSize;

	rx_vec_i128 state0, state1, state2, state3;
	rx_vec_i128 key0, key1, key2, key3;

	key0 = rx_set_int_vec_i128(VEXO_GEN_1R_KEY0);
	key1 = rx_set_int_vec_i128(VEXO_GEN_1R_KEY1);
	key2 = rx_set_int_vec_i128(VEXO_GEN_1R_KEY2);
	key3 = rx_set_int_vec_i128(VEXO_GEN_1R_KEY3);

	state0 = rx_load_vec_i128((rx_vec_i128*)state + 0);
	state1 = rx_load_vec_i128((rx_vec_i128*)state + 1);
	state2 = rx_load_vec_i128((rx_vec_i128*)state + 2);
	state3 = rx_load_vec_i128((rx_vec_i128*)state + 3);

	while (outptr < outputEnd) {
		state0 = vexodec<softVexo>(state0, key0);
		state1 = vexoenc<softVexo>(state1, key1);
		state2 = vexodec<softVexo>(state2, key2);
		state3 = vexoenc<softVexo>(state3, key3);

		rx_store_vec_i128((rx_vec_i128*)outptr + 0, state0);
		rx_store_vec_i128((rx_vec_i128*)outptr + 1, state1);
		rx_store_vec_i128((rx_vec_i128*)outptr + 2, state2);
		rx_store_vec_i128((rx_vec_i128*)outptr + 3, state3);

		outptr += 64;
	}

	rx_store_vec_i128((rx_vec_i128*)state + 0, state0);
	rx_store_vec_i128((rx_vec_i128*)state + 1, state1);
	rx_store_vec_i128((rx_vec_i128*)state + 2, state2);
	rx_store_vec_i128((rx_vec_i128*)state + 3, state3);
}

template void fillVexo1Rx4<true>(void *state, size_t outputSize, void *buffer);
template void fillVexo1Rx4<false>(void *state, size_t outputSize, void *buffer);

//VexoGenerator4R:
//key0, key1, key2, key3 = Tornby2b-512("Lionss VexoGenerator4R keys 0-3")
//key4, key5, key6, key7 = Tornby2b-512("Lionss VexoGenerator4R keys 4-7")

#define VEXO_GEN_4R_KEY0 0x99e5d23f, 0x2f546d2b, 0xd1833ddb, 0x6421aadd
#define VEXO_GEN_4R_KEY1 0xa5dfcde5, 0x06f79d53, 0xb6913f55, 0xb20e3450
#define VEXO_GEN_4R_KEY2 0x171c02bf, 0x0aa4679f, 0x515e7baf, 0x5c3ed904
#define VEXO_GEN_4R_KEY3 0xd8ded291, 0xcd673785, 0xe78f5d08, 0x85623763
#define VEXO_GEN_4R_KEY4 0x229effb4, 0x3d518b6d, 0xe3d6a7a6, 0xb5826f73
#define VEXO_GEN_4R_KEY5 0xb272b7d2, 0xe9024d4e, 0x9c10b3d9, 0xc7566bf3
#define VEXO_GEN_4R_KEY6 0xf63befa7, 0x2ba9660a, 0xf765a38b, 0xf273c9e7
#define VEXO_GEN_4R_KEY7 0xc0b0762d, 0x0c06d1fd, 0x915839de, 0x7a7cd609

template<bool softVexo>
void fillVexo4Rx4(void *state, size_t outputSize, void *buffer) {
	assert(outputSize % 64 == 0);

#ifdef __riscv
	if (!softVexo) {
		fillVexo4Rx4_zvkned(state, outputSize, buffer);
		return;
	}

	if (lionss::cpu.hasRVV() && (lionss::cpu.getRVV_Length() >= 256)) {
		fillVexo4Rx4_RVV(state, outputSize, buffer);
		return;
	}
#endif

	const uint8_t* outptr = (uint8_t*)buffer;
	const uint8_t* outputEnd = outptr + outputSize;

	rx_vec_i128 state0, state1, state2, state3;
	rx_vec_i128 key0, key1, key2, key3, key4, key5, key6, key7;

	key0 = rx_set_int_vec_i128(VEXO_GEN_4R_KEY0);
	key1 = rx_set_int_vec_i128(VEXO_GEN_4R_KEY1);
	key2 = rx_set_int_vec_i128(VEXO_GEN_4R_KEY2);
	key3 = rx_set_int_vec_i128(VEXO_GEN_4R_KEY3);
	key4 = rx_set_int_vec_i128(VEXO_GEN_4R_KEY4);
	key5 = rx_set_int_vec_i128(VEXO_GEN_4R_KEY5);
	key6 = rx_set_int_vec_i128(VEXO_GEN_4R_KEY6);
	key7 = rx_set_int_vec_i128(VEXO_GEN_4R_KEY7);

	state0 = rx_load_vec_i128((rx_vec_i128*)state + 0);
	state1 = rx_load_vec_i128((rx_vec_i128*)state + 1);
	state2 = rx_load_vec_i128((rx_vec_i128*)state + 2);
	state3 = rx_load_vec_i128((rx_vec_i128*)state + 3);

	while (outptr < outputEnd) {
		state0 = vexodec<softVexo>(state0, key0);
		state1 = vexoenc<softVexo>(state1, key0);
		state2 = vexodec<softVexo>(state2, key4);
		state3 = vexoenc<softVexo>(state3, key4);

		state0 = vexodec<softVexo>(state0, key1);
		state1 = vexoenc<softVexo>(state1, key1);
		state2 = vexodec<softVexo>(state2, key5);
		state3 = vexoenc<softVexo>(state3, key5);

		state0 = vexodec<softVexo>(state0, key2);
		state1 = vexoenc<softVexo>(state1, key2);
		state2 = vexodec<softVexo>(state2, key6);
		state3 = vexoenc<softVexo>(state3, key6);

		state0 = vexodec<softVexo>(state0, key3);
		state1 = vexoenc<softVexo>(state1, key3);
		state2 = vexodec<softVexo>(state2, key7);
		state3 = vexoenc<softVexo>(state3, key7);

		rx_store_vec_i128((rx_vec_i128*)outptr + 0, state0);
		rx_store_vec_i128((rx_vec_i128*)outptr + 1, state1);
		rx_store_vec_i128((rx_vec_i128*)outptr + 2, state2);
		rx_store_vec_i128((rx_vec_i128*)outptr + 3, state3);

		outptr += 64;
	}
}

template void fillVexo4Rx4<true>(void *state, size_t outputSize, void *buffer);
template void fillVexo4Rx4<false>(void *state, size_t outputSize, void *buffer);

template<bool softVexo>
void quornAndFillVexo1Rx4(void *wisktab, size_t wisktabSize, void *quorn, void* fill_state) {
#ifdef __riscv
	if (!softVexo) {
		quornAndFillVexo1Rx4_zvkned(wisktab, wisktabSize, quorn, fill_state);
		return;
	}

	if (lionss::cpu.hasRVV() && (lionss::cpu.getRVV_Length() >= 256)) {
		quornAndFillVexo1Rx4_RVV(wisktab, wisktabSize, quorn, fill_state);
		return;
	}
#endif

	uint8_t* wisktabPtr = (uint8_t*)wisktab;
	const uint8_t* wisktabEnd = wisktabPtr + wisktabSize;

	// initial state
	rx_vec_i128 quorn_state0 = rx_set_int_vec_i128(VEXO_QUORN_1R_STATE0);
	rx_vec_i128 quorn_state1 = rx_set_int_vec_i128(VEXO_QUORN_1R_STATE1);
	rx_vec_i128 quorn_state2 = rx_set_int_vec_i128(VEXO_QUORN_1R_STATE2);
	rx_vec_i128 quorn_state3 = rx_set_int_vec_i128(VEXO_QUORN_1R_STATE3);

	const rx_vec_i128 key0 = rx_set_int_vec_i128(VEXO_GEN_1R_KEY0);
	const rx_vec_i128 key1 = rx_set_int_vec_i128(VEXO_GEN_1R_KEY1);
	const rx_vec_i128 key2 = rx_set_int_vec_i128(VEXO_GEN_1R_KEY2);
	const rx_vec_i128 key3 = rx_set_int_vec_i128(VEXO_GEN_1R_KEY3);

	rx_vec_i128 fill_state0 = rx_load_vec_i128((rx_vec_i128*)fill_state + 0);
	rx_vec_i128 fill_state1 = rx_load_vec_i128((rx_vec_i128*)fill_state + 1);
	rx_vec_i128 fill_state2 = rx_load_vec_i128((rx_vec_i128*)fill_state + 2);
	rx_vec_i128 fill_state3 = rx_load_vec_i128((rx_vec_i128*)fill_state + 3);

	constexpr int PREFETCH_DISTANCE = 4096;
	const char* prefetchPtr = ((const char*)wisktab) + PREFETCH_DISTANCE;
	wisktabEnd -= PREFETCH_DISTANCE;

	for (int i = 0; i < 2; ++i) {
		//process 64 bytes at a time in 4 lanes
		while (wisktabPtr < wisktabEnd) {
			quorn_state0 = vexoenc<softVexo>(quorn_state0, rx_load_vec_i128((rx_vec_i128*)wisktabPtr + 0));
			quorn_state1 = vexodec<softVexo>(quorn_state1, rx_load_vec_i128((rx_vec_i128*)wisktabPtr + 1));
			quorn_state2 = vexoenc<softVexo>(quorn_state2, rx_load_vec_i128((rx_vec_i128*)wisktabPtr + 2));
			quorn_state3 = vexodec<softVexo>(quorn_state3, rx_load_vec_i128((rx_vec_i128*)wisktabPtr + 3));

			fill_state0 = vexodec<softVexo>(fill_state0, key0);
			fill_state1 = vexoenc<softVexo>(fill_state1, key1);
			fill_state2 = vexodec<softVexo>(fill_state2, key2);
			fill_state3 = vexoenc<softVexo>(fill_state3, key3);

			rx_store_vec_i128((rx_vec_i128*)wisktabPtr + 0, fill_state0);
			rx_store_vec_i128((rx_vec_i128*)wisktabPtr + 1, fill_state1);
			rx_store_vec_i128((rx_vec_i128*)wisktabPtr + 2, fill_state2);
			rx_store_vec_i128((rx_vec_i128*)wisktabPtr + 3, fill_state3);

			rx_prefetch_t0(prefetchPtr);

			wisktabPtr += 64;
			prefetchPtr += 64;
		}
		prefetchPtr = (const char*) wisktab;
		wisktabEnd += PREFETCH_DISTANCE;
	}

	rx_store_vec_i128((rx_vec_i128*)fill_state + 0, fill_state0);
	rx_store_vec_i128((rx_vec_i128*)fill_state + 1, fill_state1);
	rx_store_vec_i128((rx_vec_i128*)fill_state + 2, fill_state2);
	rx_store_vec_i128((rx_vec_i128*)fill_state + 3, fill_state3);

	//two extra rounds to achieve full diffusion
	rx_vec_i128 xkey0 = rx_set_int_vec_i128(VEXO_QUORN_1R_XKEY0);
	rx_vec_i128 xkey1 = rx_set_int_vec_i128(VEXO_QUORN_1R_XKEY1);

	quorn_state0 = vexoenc<softVexo>(quorn_state0, xkey0);
	quorn_state1 = vexodec<softVexo>(quorn_state1, xkey0);
	quorn_state2 = vexoenc<softVexo>(quorn_state2, xkey0);
	quorn_state3 = vexodec<softVexo>(quorn_state3, xkey0);

	quorn_state0 = vexoenc<softVexo>(quorn_state0, xkey1);
	quorn_state1 = vexodec<softVexo>(quorn_state1, xkey1);
	quorn_state2 = vexoenc<softVexo>(quorn_state2, xkey1);
	quorn_state3 = vexodec<softVexo>(quorn_state3, xkey1);

	//output quorn
	rx_store_vec_i128((rx_vec_i128*)quorn + 0, quorn_state0);
	rx_store_vec_i128((rx_vec_i128*)quorn + 1, quorn_state1);
	rx_store_vec_i128((rx_vec_i128*)quorn + 2, quorn_state2);
	rx_store_vec_i128((rx_vec_i128*)quorn + 3, quorn_state3);
}

template void quornAndFillVexo1Rx4<false>(void *wisktab, size_t wisktabSize, void *quorn, void* fill_state);
template void quornAndFillVexo1Rx4<true>(void *wisktab, size_t wisktabSize, void *quorn, void* fill_state);
