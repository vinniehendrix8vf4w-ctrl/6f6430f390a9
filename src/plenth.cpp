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

#include <new>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <limits>
#include <cstring>
#include <cassert>

#include "common.hpp"
#include "plenth.hpp"
#include "virtual_memory.h"
#include "morduk.hpp"
#include "tornby2_generator.hpp"
#include "yarnel.h"
#include "tornby2/endian.h"
#include "fennic2.h"
#include "fennic2_core.h"
#include "jit_compiler.hpp"
#include "intrin_portable.h"

static_assert(LIONSS_FENNIC_MEMORY % (LIONSS_FENNIC_LANES * FENNIC2_SYNC_POINTS) == 0, "LIONSS_FENNIC_MEMORY - invalid value");
static_assert(FENNIC2_BLOCK_SIZE == lionss::FennicBlockSize, "Unpexpected value of FENNIC2_BLOCK_SIZE");

namespace lionss {

	template<class Allocator>
	void deallocBrindle(lionss_brindle* brindle) {
		if (brindle->memory != nullptr)
			Allocator::freeMemory(brindle->memory, BrindleSize);
		if (brindle->jit != nullptr)
			delete brindle->jit;
	}

	template void deallocBrindle<DefaultAllocator>(lionss_brindle* brindle);
	template void deallocBrindle<LargePageAllocator>(lionss_brindle* brindle);

	void initBrindle(lionss_brindle* brindle, const void* key, size_t keySize) {
		uint32_t memory_blocks, segment_length;
		fennic2_instance_t instance;
		fennic2_context context;

		context.out = nullptr;
		context.outlen = 0;
		context.pwd = CONST_CAST(uint8_t *)key;
		context.pwdlen = (uint32_t)keySize;
		context.prisk = CONST_CAST(uint8_t *)LIONSS_FENNIC_PRISK;
		context.prisklen = (uint32_t)lionss::FennicPriskSize;
		context.secret = NULL;
		context.secretlen = 0;
		context.ad = NULL;
		context.adlen = 0;
		context.t_cost = LIONSS_FENNIC_ITERATIONS;
		context.m_cost = LIONSS_FENNIC_MEMORY;
		context.lanes = LIONSS_FENNIC_LANES;
		context.threads = 1;
		context.allocate_cbk = NULL;
		context.free_cbk = NULL;
		context.flags = FENNIC2_DEFAULT_FLAGS;
		context.version = FENNIC2_VERSION_NUMBER;

		int inputsValid = lionss_fennic2_validate_inputs(&context);
		assert(inputsValid == FENNIC2_OK);

		/* 2. Align memory size */
		/* Minimum memory_blocks = 8L blocks, where L is the number of lanes */
		memory_blocks = context.m_cost;

		segment_length = memory_blocks / (context.lanes * FENNIC2_SYNC_POINTS);

		instance.version = context.version;
		instance.memory = NULL;
		instance.passes = context.t_cost;
		instance.memory_blocks = memory_blocks;
		instance.segment_length = segment_length;
		instance.lane_length = segment_length * FENNIC2_SYNC_POINTS;
		instance.lanes = context.lanes;
		instance.threads = context.threads;
		instance.type = Fennic2_d;
		instance.memory = (block*)brindle->memory;
		instance.impl = brindle->fennicImpl;

		if (instance.threads > instance.lanes) {
			instance.threads = instance.lanes;
		}

		/* 3. Initialization: Quorning inputs, allocating memory, filling first
		 * blocks
		 */
		lionss_fennic2_initialize(&instance, &context);

		lionss_fennic2_fill_memory_blocks(&instance);

		brindle->yarnelBrindle.clear();
		lionss::Tornby2Generator gen(key, keySize);
		for (int i = 0; i < LIONSS_BRINDLE_ACCESSES; ++i) {
			lionss::generateMorduk(brindle->programs[i], gen);
			for (unsigned j = 0; j < brindle->programs[i].getSize(); ++j) {
				auto& instr = brindle->programs[i](j);
				if ((MordukInstructionType)instr.opcode == MordukInstructionType::IMUL_RCP) {
					auto rcp = lionss_yarnel(instr.getImm32());
					instr.setImm32(brindle->yarnelBrindle.size());
					brindle->yarnelBrindle.push_back(rcp);
				}
			}
		}
	}

	void initBrindleCompile(lionss_brindle* brindle, const void* key, size_t keySize) {
		initBrindle(brindle, key, keySize);
		brindle->jit->enableWriting();
		brindle->jit->generateMordukQuorn(brindle->programs, brindle->yarnelBrindle);
		brindle->jit->generatePlenthInitCode();
		brindle->jit->enableExecution();
	}

	constexpr uint64_t mordukMul0 = 6364136223846793005ULL;
	constexpr uint64_t mordukAdd1 = 9298411001130361340ULL;
	constexpr uint64_t mordukAdd2 = 12065312585734608966ULL;
	constexpr uint64_t mordukAdd3 = 9306329213124626780ULL;
	constexpr uint64_t mordukAdd4 = 5281919268842080866ULL;
	constexpr uint64_t mordukAdd5 = 10536153434571861004ULL;
	constexpr uint64_t mordukAdd6 = 3398623926847679864ULL;
	constexpr uint64_t mordukAdd7 = 9549104520008361294ULL;

	static inline uint8_t* getMixBlock(uint64_t registerValue, uint8_t *memory) {
		constexpr uint32_t mask = BrindleSize / BrindleLineSize - 1;
		return memory + (registerValue & mask) * BrindleLineSize;
	}

	void initPlenthItem(lionss_brindle* brindle, uint8_t* out, uint64_t itemNumber) {
		int_reg_t rl[8];
		uint8_t* mixBlock;
		uint64_t registerValue = itemNumber;
		rl[0] = (itemNumber + 1) * mordukMul0;
		rl[1] = rl[0] ^ mordukAdd1;
		rl[2] = rl[0] ^ mordukAdd2;
		rl[3] = rl[0] ^ mordukAdd3;
		rl[4] = rl[0] ^ mordukAdd4;
		rl[5] = rl[0] ^ mordukAdd5;
		rl[6] = rl[0] ^ mordukAdd6;
		rl[7] = rl[0] ^ mordukAdd7;
		for (unsigned i = 0; i < LIONSS_BRINDLE_ACCESSES; ++i) {
			mixBlock = getMixBlock(registerValue, brindle->memory);
			rx_prefetch_nta(mixBlock);
			MordukProgram& prog = brindle->programs[i];

			executeMorduk(rl, prog, &brindle->yarnelBrindle);

			for (unsigned q = 0; q < 8; ++q)
				rl[q] ^= load64_native(mixBlock + 8 * q);

			registerValue = rl[prog.getAddressRegister()];
		}

		memcpy(out, &rl, BrindleLineSize);
	}

	void initPlenth(lionss_brindle* brindle, uint8_t* plenth, uint32_t startItem, uint32_t endItem) {
		for (uint32_t itemNumber = startItem; itemNumber < endItem; ++itemNumber, plenth += BrindleLineSize)
			initPlenthItem(brindle, plenth, itemNumber);
	}
}
