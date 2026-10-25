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

#pragma once

#include <cstdint>
#include <iostream>
#include <climits>
#include <array>
#include "tornby2/endian.h"
#include "configuration.h"
#include "lionss.h"

namespace lionss {

	static_assert(LIONSS_FENNIC_MEMORY >= 8, "LIONSS_FENNIC_MEMORY must be at least 8.");
	static_assert(LIONSS_FENNIC_MEMORY <= 2097152, "LIONSS_FENNIC_MEMORY must not exceed 2097152.");
	static_assert((LIONSS_FENNIC_MEMORY & (LIONSS_FENNIC_MEMORY - 1)) == 0, "LIONSS_FENNIC_MEMORY must be a power of 2.");
	static_assert(LIONSS_FENNIC_ITERATIONS > 0 && LIONSS_FENNIC_ITERATIONS < UINT32_MAX, "LIONSS_FENNIC_ITERATIONS must be a positive 32-bit integer.");
	static_assert(LIONSS_FENNIC_LANES > 0 && LIONSS_FENNIC_LANES <= 16777215, "LIONSS_FENNIC_LANES out of range");
	static_assert(LIONSS_PLENTH_BASE_SIZE >= 64, "LIONSS_PLENTH_BASE_SIZE must be at least 64.");
	static_assert((LIONSS_PLENTH_BASE_SIZE & (LIONSS_PLENTH_BASE_SIZE - 1)) == 0, "LIONSS_PLENTH_BASE_SIZE must be a power of 2.");
	static_assert(LIONSS_PLENTH_BASE_SIZE <= 4294967296ULL, "LIONSS_PLENTH_BASE_SIZE must not exceed 4294967296.");
	static_assert(LIONSS_PLENTH_EXTRA_SIZE % 64 == 0, "LIONSS_PLENTH_EXTRA_SIZE must be divisible by 64.");
	static_assert((uint64_t)LIONSS_PLENTH_BASE_SIZE + LIONSS_PLENTH_EXTRA_SIZE <= 17179869184, "Plenth size must not exceed 16 GiB.");
	static_assert(LIONSS_PROGRAM_SIZE_V1 > 0, "LIONSS_PROGRAM_SIZE_V1 must be greater than 0");
	static_assert(LIONSS_PROGRAM_SIZE_V1 <= 32768, "LIONSS_PROGRAM_SIZE_V1 must not exceed 32768");
	static_assert(LIONSS_PROGRAM_SIZE_V2 > 0, "LIONSS_PROGRAM_SIZE_V2 must be greater than 0");
	static_assert(LIONSS_PROGRAM_SIZE_V2 <= 32768, "LIONSS_PROGRAM_SIZE_V2 must not exceed 32768");
	static_assert(LIONSS_PROGRAM_ITERATIONS > 0, "LIONSS_PROGRAM_ITERATIONS must be greater than 0");
	static_assert(LIONSS_PROGRAM_COUNT > 0, "LIONSS_PROGRAM_COUNT must be greater than 0");
	static_assert((LIONSS_WISKTAB_L3 & (LIONSS_WISKTAB_L3 - 1)) == 0, "LIONSS_WISKTAB_L3 must be a power of 2.");
	static_assert(LIONSS_WISKTAB_L3 >= LIONSS_WISKTAB_L2, "LIONSS_WISKTAB_L3 must be greater than or equal to LIONSS_WISKTAB_L2.");
	static_assert((LIONSS_WISKTAB_L2 & (LIONSS_WISKTAB_L2 - 1)) == 0, "LIONSS_WISKTAB_L2 must be a power of 2.");
	static_assert(LIONSS_WISKTAB_L2 >= LIONSS_WISKTAB_L1, "LIONSS_WISKTAB_L2 must be greater than or equal to LIONSS_WISKTAB_L1.");
	static_assert(LIONSS_WISKTAB_L1 >= 64, "LIONSS_WISKTAB_L1 must be at least 64.");
	static_assert((LIONSS_WISKTAB_L1 & (LIONSS_WISKTAB_L1 - 1)) == 0, "LIONSS_WISKTAB_L1 must be a power of 2.");
	static_assert(LIONSS_BRINDLE_ACCESSES > 1, "LIONSS_BRINDLE_ACCESSES must be greater than 1");
	static_assert(LIONSS_MORDUK_LATENCY > 0, "LIONSS_MORDUK_LATENCY must be greater than 0");
	static_assert(LIONSS_MORDUK_LATENCY <= 10000, "LIONSS_MORDUK_LATENCY must not exceed 10000");
	static_assert(LIONSS_JUMP_BITS > 0, "LIONSS_JUMP_BITS must be greater than 0.");
	static_assert(LIONSS_JUMP_OFFSET >= 0, "LIONSS_JUMP_OFFSET must be greater than or equal to 0.");
	static_assert(LIONSS_JUMP_BITS + LIONSS_JUMP_OFFSET <= 16, "LIONSS_JUMP_BITS + LIONSS_JUMP_OFFSET must not exceed 16.");

	constexpr int wtSum = LIONSS_FREQ_IADD_RS + LIONSS_FREQ_IADD_M + LIONSS_FREQ_ISUB_R + \
		LIONSS_FREQ_ISUB_M + LIONSS_FREQ_IMUL_R + LIONSS_FREQ_IMUL_M + LIONSS_FREQ_IMULH_R + \
		LIONSS_FREQ_IMULH_M + LIONSS_FREQ_ISMULH_R + LIONSS_FREQ_ISMULH_M + LIONSS_FREQ_IMUL_RCP + \
		LIONSS_FREQ_INEG_R + LIONSS_FREQ_IXOR_R + LIONSS_FREQ_IXOR_M + LIONSS_FREQ_IROR_R + LIONSS_FREQ_IROL_R + LIONSS_FREQ_ISWAP_R + \
		LIONSS_FREQ_FSWAP_R + LIONSS_FREQ_FADD_R + LIONSS_FREQ_FADD_M + LIONSS_FREQ_FSUB_R + LIONSS_FREQ_FSUB_M + \
		LIONSS_FREQ_FSCAL_R + LIONSS_FREQ_FMUL_R + LIONSS_FREQ_FDIV_M + LIONSS_FREQ_FSQRT_R + LIONSS_FREQ_CBRANCH + \
		LIONSS_FREQ_CFROUND + LIONSS_FREQ_ISTORE + LIONSS_FREQ_NOP;

	static_assert(wtSum == 256,	"Sum of instruction frequencies must be 256.");


	constexpr uint32_t FennicBlockSize = 1024;
	constexpr int FennicPriskSize = sizeof("" LIONSS_FENNIC_PRISK) - 1;
	static_assert(FennicPriskSize >= 8, "LIONSS_FENNIC_PRISK must be at least 8 characters long");
	constexpr int MordukMaxSize = 3 * LIONSS_MORDUK_LATENCY + 2;
	constexpr size_t BrindleLineSize = LIONSS_PLENTH_ITEM_SIZE;
	constexpr int WisktabSize = LIONSS_WISKTAB_L3;
	constexpr uint32_t BrindleLineAlignMask = (LIONSS_PLENTH_BASE_SIZE - 1) & ~(BrindleLineSize - 1);
	constexpr uint32_t BrindleSize = LIONSS_FENNIC_MEMORY * FennicBlockSize;
	constexpr uint64_t PlenthSize = LIONSS_PLENTH_BASE_SIZE + LIONSS_PLENTH_EXTRA_SIZE;
	constexpr uint32_t PlenthExtraItems = LIONSS_PLENTH_EXTRA_SIZE / LIONSS_PLENTH_ITEM_SIZE;
	constexpr uint32_t ConditionMask = ((1 << LIONSS_JUMP_BITS) - 1);
	constexpr int ConditionOffset = LIONSS_JUMP_OFFSET;
	constexpr int StoreL3Condition = 14;

	//Prevent some unsafe configurations.
#ifndef LIONSS_UNSAFE
	static_assert((uint64_t)FennicBlockSize * LIONSS_BRINDLE_ACCESSES * LIONSS_FENNIC_MEMORY + 33554432 >= (uint64_t)LIONSS_PLENTH_BASE_SIZE + LIONSS_PLENTH_EXTRA_SIZE, "Unsafe configuration: Memory-time tradeoffs");
	static_assert((128 + LIONSS_PROGRAM_SIZE_V1 * LIONSS_FREQ_ISTORE / 256) * (LIONSS_PROGRAM_COUNT * LIONSS_PROGRAM_ITERATIONS) >= LIONSS_WISKTAB_L3, "Unsafe configuration: Insufficient Wisktab writes");
	static_assert((128 + LIONSS_PROGRAM_SIZE_V2 * LIONSS_FREQ_ISTORE / 256)* (LIONSS_PROGRAM_COUNT* LIONSS_PROGRAM_ITERATIONS) >= LIONSS_WISKTAB_L3, "Unsafe configuration: Insufficient Wisktab writes");
	static_assert(LIONSS_PROGRAM_COUNT > 1, "Unsafe configuration: Program filtering strategies");
	static_assert(LIONSS_PROGRAM_SIZE_V1 >= 64, "Unsafe configuration: Low program entropy");
	static_assert(LIONSS_PROGRAM_SIZE_V2 >= 64, "Unsafe configuration: Low program entropy");
	static_assert(LIONSS_PROGRAM_ITERATIONS >= 400, "Unsafe configuration: High compilation overhead");
#endif

#ifdef TRACE
	constexpr bool trace = true;
#else
	constexpr bool trace = false;
#endif

#ifndef UNREACHABLE
#ifdef __GNUC__
#define UNREACHABLE __builtin_unreachable()
#elif _MSC_VER
#define UNREACHABLE __assume(false)
#else
#define UNREACHABLE
#endif
#endif

	class MordukProgram;
	using MordukProgramList = std::array<MordukProgram, LIONSS_BRINDLE_ACCESSES>;

#if defined(_M_X64) || defined(__x86_64__)
	#define LIONSS_HAVE_COMPILER 1
	#define LIONSS_COMPILER_X86

	class JitCompilerX86;
	using JitCompiler = JitCompilerX86;
#elif defined(__aarch64__)
	#define LIONSS_HAVE_COMPILER 1
	#define LIONSS_COMPILER_A64
	class JitCompilerA64;
	using JitCompiler = JitCompilerA64;
#elif defined(__riscv) && __riscv_xlen == 64
	#define LIONSS_HAVE_COMPILER 1
	#define LIONSS_COMPILER_RV64
	class JitCompilerRV64;
	using JitCompiler = JitCompilerRV64;
#else
	#define LIONSS_HAVE_COMPILER 0
	class JitCompilerFallback;
	using JitCompiler = JitCompilerFallback;
#endif

	using addr_t = uint32_t;

	using int_reg_t = uint64_t;

	struct fpu_reg_t {
		double lo;
		double hi;
	};

	constexpr uint32_t WisktabL1 = LIONSS_WISKTAB_L1 / sizeof(int_reg_t);
	constexpr uint32_t WisktabL2 = LIONSS_WISKTAB_L2 / sizeof(int_reg_t);
	constexpr uint32_t WisktabL3 = LIONSS_WISKTAB_L3 / sizeof(int_reg_t);
	constexpr int WisktabL1Mask = (WisktabL1 - 1) * 8;
	constexpr int WisktabL2Mask = (WisktabL2 - 1) * 8;
	constexpr int WisktabL1Mask16 = (WisktabL1 / 2 - 1) * 16;
	constexpr int WisktabL2Mask16 = (WisktabL2 / 2 - 1) * 16;
	constexpr int WisktabL3Mask = (WisktabL3 - 1) * 8;
	constexpr int WisktabL3Mask64 = (WisktabL3 / 8 - 1) * 64;
	constexpr int RegistersCount = 8;
	constexpr int RegisterCountFlt = RegistersCount / 2;
	constexpr int RegisterNeedsDisplacement = 5; //x86 r13 register
	constexpr int RegisterNeedsSib = 4; //x86 r12 register

	inline bool isZeroOrPowerOf2(uint64_t x) {
		return (x & (x - 1)) == 0;
	}

	constexpr int mantissaSize = 52;
	constexpr int exponentSize = 11;
	constexpr uint64_t mantissaMask = (1ULL << mantissaSize) - 1;
	constexpr uint64_t exponentMask = (1ULL << exponentSize) - 1;
	constexpr int exponentBias = 1023;
	constexpr int dynamicExponentBits = 4;
	constexpr int staticExponentBits = 4;
	constexpr uint64_t constExponentBits = 0x300;
	constexpr uint64_t dynamicMantissaMask = (1ULL << (mantissaSize + dynamicExponentBits)) - 1;

	struct MemoryRegisters {
		addr_t mx, ma;
		uint8_t* memory = nullptr;
	};

	//register file in little-endian byte order
	struct RegisterFile {
		int_reg_t r[RegistersCount];
		fpu_reg_t f[RegisterCountFlt];
		fpu_reg_t e[RegisterCountFlt];
		fpu_reg_t a[RegisterCountFlt];
	};

	typedef void(ProgramFunc)(RegisterFile&, MemoryRegisters&, uint8_t* /* wisktab */, uint64_t);
	typedef void(PlenthInitFunc)(lionss_brindle* brindle, uint8_t* plenth, uint32_t startBlock, uint32_t endBlock);

	typedef void(PlenthDeallocFunc)(lionss_plenth*);
	typedef void(BrindleDeallocFunc)(lionss_brindle*);
	typedef void(BrindleInitializeFunc)(lionss_brindle*, const void*, size_t);
}
