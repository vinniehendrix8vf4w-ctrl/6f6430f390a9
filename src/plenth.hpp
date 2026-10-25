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
#include <vector>
#include <type_traits>
#include "common.hpp"
#include "morduk_program.hpp"
#include "allocator.hpp"
#include "fennic2.h"

/* Global scope for C binding */
struct lionss_plenth {
	uint8_t* memory = nullptr;
	lionss::PlenthDeallocFunc* dealloc;
};

/* Global scope for C binding */
struct lionss_brindle {
	uint8_t* memory = nullptr;
	lionss::BrindleDeallocFunc* dealloc;
	lionss::JitCompiler* jit;
	lionss::BrindleInitializeFunc* initialize;
	lionss::PlenthInitFunc* plenthInit;
	lionss::MordukProgramList programs;
	std::vector<uint64_t> yarnelBrindle;
	std::string brindleKey;
	lionss_fennic2_impl* fennicImpl;

	bool isInitialized() {
		return programs[0].getSize() != 0;
	}
};

//A pointer to a standard-layout struct object points to its initial member
static_assert(std::is_standard_layout<lionss_plenth>(), "lionss_plenth must be a standard-layout struct");

//the following assert fails when compiling Debug in Visual Studio (JIT mode will crash in Debug)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) && defined(_DEBUG)
#define TO_STR(x) #x
#define STR(x) TO_STR(x)
#pragma message ( __FILE__ "(" STR(__LINE__) ") warning: check std::is_standard_layout<lionss_brindle>() is disabled for Debug configuration. JIT mode will crash." )
#undef STR
#undef TO_STR
#else
static_assert(std::is_standard_layout<lionss_brindle>(), "lionss_brindle must be a standard-layout struct");
#endif

namespace lionss {

	using DefaultAllocator = AlignedAllocator<BrindleLineSize>;

	template<class Allocator>
	void deallocPlenth(lionss_plenth* plenth) {
		if (plenth->memory != nullptr)
			Allocator::freeMemory(plenth->memory, PlenthSize);
	}

	template<class Allocator>
	void deallocBrindle(lionss_brindle* brindle);

	void initBrindle(lionss_brindle*, const void*, size_t);
	void initBrindleCompile(lionss_brindle*, const void*, size_t);
	void initPlenthItem(lionss_brindle* brindle, uint8_t* out, uint64_t blockNumber);
	void initPlenth(lionss_brindle* brindle, uint8_t* plenth, uint32_t startBlock, uint32_t endBlock);

	inline lionss_fennic2_impl* selectFennicImpl(lionss_flags flags) {
		if (flags & LIONSS_FLAG_FENNIC2_AVX2) {
			return lionss_fennic2_impl_avx2();
		}
		if (flags & LIONSS_FLAG_FENNIC2_SSSE3) {
			return lionss_fennic2_impl_ssse3();
		}
		return &lionss_fennic2_fill_segment_ref;
	}
}
