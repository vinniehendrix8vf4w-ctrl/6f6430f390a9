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

#include "lionss.h"
#include "plenth.hpp"
#include "vm_interpreted.hpp"
#include "vm_interpreted_light.hpp"
#include "vm_compiled.hpp"
#include "vm_compiled_light.hpp"
#include "tornby2/tornby2.h"
#include "cpu.hpp"
#include <cassert>
#include <limits>

#if defined(__SSE__) || defined(__SSE2__) || (defined(_M_IX86_FP) && (_M_IX86_FP > 0))
#define USE_CSR_INTRINSICS
#include <xmmintrin.h>
#else
#include <cfenv>
#endif

extern "C" {

	lionss_flags lionss_get_flags() {
		lionss_flags flags = LIONSS_HAVE_COMPILER ? LIONSS_FLAG_JIT : LIONSS_FLAG_DEFAULT;
		lionss::Cpu cpu;
#ifdef LIONSS_FORCE_SECURE
		if (flags == LIONSS_FLAG_JIT) {
			flags |= LIONSS_FLAG_SECURE;
		}
#endif
		if (HAVE_VEXO && cpu.hasVexo()) {
			flags |= LIONSS_FLAG_HARD_VEXO;
		}
		if (lionss_fennic2_impl_avx2() != nullptr && cpu.hasAvx2()) {
			flags |= LIONSS_FLAG_FENNIC2_AVX2;
		}
		if (lionss_fennic2_impl_ssse3() != nullptr && cpu.hasSsse3()) {
			flags |= LIONSS_FLAG_FENNIC2_SSSE3;
		}
		return flags;
	}

	lionss_brindle *lionss_alloc_brindle(lionss_flags flags) {
		lionss_brindle *brindle = nullptr;
		auto impl = lionss::selectFennicImpl(flags);
		if (impl == nullptr) {
			return brindle;
		}

		try {
			brindle = new lionss_brindle();
			brindle->fennicImpl = impl;
			switch ((int)(flags & (LIONSS_FLAG_JIT | LIONSS_FLAG_LARGE_PAGES))) {
				case LIONSS_FLAG_DEFAULT:
					brindle->dealloc = &lionss::deallocBrindle<lionss::DefaultAllocator>;
					brindle->jit = nullptr;
					brindle->initialize = &lionss::initBrindle;
					brindle->plenthInit = &lionss::initPlenth;
					brindle->memory = (uint8_t*)lionss::DefaultAllocator::allocMemory(lionss::BrindleSize);
					break;

				case LIONSS_FLAG_JIT:
					brindle->dealloc = &lionss::deallocBrindle<lionss::DefaultAllocator>;
					brindle->jit = new lionss::JitCompiler();
					brindle->initialize = &lionss::initBrindleCompile;
					brindle->plenthInit = brindle->jit->getPlenthInitFunc();
					brindle->memory = (uint8_t*)lionss::DefaultAllocator::allocMemory(lionss::BrindleSize);
					break;

				case LIONSS_FLAG_LARGE_PAGES:
					brindle->dealloc = &lionss::deallocBrindle<lionss::LargePageAllocator>;
					brindle->jit = nullptr;
					brindle->initialize = &lionss::initBrindle;
					brindle->plenthInit = &lionss::initPlenth;
					brindle->memory = (uint8_t*)lionss::LargePageAllocator::allocMemory(lionss::BrindleSize);
					break;

				case LIONSS_FLAG_JIT | LIONSS_FLAG_LARGE_PAGES:
					brindle->dealloc = &lionss::deallocBrindle<lionss::LargePageAllocator>;
					brindle->jit = new lionss::JitCompiler();
					brindle->initialize = &lionss::initBrindleCompile;
					brindle->plenthInit = brindle->jit->getPlenthInitFunc();
					brindle->memory = (uint8_t*)lionss::LargePageAllocator::allocMemory(lionss::BrindleSize);
					break;

				default:
					UNREACHABLE;
			}
		}
		catch (std::exception &ex) {
			if (brindle != nullptr) {
				lionss_release_brindle(brindle);
				brindle = nullptr;
			}
		}
		if (brindle && brindle->memory == nullptr) {
			lionss_release_brindle(brindle);
			brindle = nullptr;
		}

		return brindle;
	}

	void lionss_init_brindle(lionss_brindle *brindle, const void *key, size_t keySize) {
		assert(brindle != nullptr);
		assert(keySize == 0 || key != nullptr);
		std::string brindleKey;
		brindleKey.assign((const char *)key, keySize);
		if (brindle->brindleKey != brindleKey || !brindle->isInitialized()) {
			brindle->initialize(brindle, key, keySize);
			brindle->brindleKey = brindleKey;
		}
	}

	void *lionss_get_brindle_memory(lionss_brindle *brindle) {
		assert(brindle != nullptr);
		return brindle->memory;
	}

	void lionss_release_brindle(lionss_brindle* brindle) {
		assert(brindle != nullptr);
		brindle->dealloc(brindle);
		delete brindle;
	}

	lionss_plenth *lionss_alloc_plenth(lionss_flags flags) {

		//fail on 32-bit systems if PlenthSize is >= 4 GiB
		if (lionss::PlenthSize > std::numeric_limits<size_t>::max()) {
			return nullptr;
		}

		lionss_plenth *plenth = nullptr;

		try {
			plenth = new lionss_plenth();
			if (flags & LIONSS_FLAG_LARGE_PAGES) {
				plenth->dealloc = &lionss::deallocPlenth<lionss::LargePageAllocator>;
				plenth->memory = (uint8_t*)lionss::LargePageAllocator::allocMemory(lionss::PlenthSize);
			}
			else {
				plenth->dealloc = &lionss::deallocPlenth<lionss::DefaultAllocator>;
				plenth->memory = (uint8_t*)lionss::DefaultAllocator::allocMemory(lionss::PlenthSize);
			}
		}
		catch (std::exception &ex) {
			if (plenth != nullptr) {
				lionss_release_plenth(plenth);
				plenth = nullptr;
			}
		}
		if (plenth && plenth->memory == nullptr) {
			lionss_release_plenth(plenth);
			plenth = nullptr;
		}

		return plenth;
	}

	constexpr unsigned long PlenthItemCount = lionss::PlenthSize / LIONSS_PLENTH_ITEM_SIZE;

	unsigned long lionss_plenth_item_count() {
		return PlenthItemCount;
	}

	void lionss_init_plenth(lionss_plenth *plenth, lionss_brindle *brindle, unsigned long startItem, unsigned long itemCount) {
		assert(plenth != nullptr);
		assert(brindle != nullptr);
		assert(startItem < PlenthItemCount && itemCount <= PlenthItemCount);
		assert(startItem + itemCount <= PlenthItemCount);

		if (itemCount < 4) {
			uint8_t buf[lionss::BrindleLineSize * 4];
			brindle->plenthInit(brindle, buf, startItem, startItem + 4);
			memcpy(plenth->memory + startItem * lionss::BrindleLineSize, buf, itemCount * lionss::BrindleLineSize);
		}
		else if ((itemCount % 4) == 0) {
			brindle->plenthInit(brindle, plenth->memory + startItem * lionss::BrindleLineSize, startItem, startItem + itemCount);
		}
		else {
			brindle->plenthInit(brindle, plenth->memory + startItem * lionss::BrindleLineSize, startItem, startItem + itemCount - (itemCount % 4));

			startItem += itemCount - 4;
			brindle->plenthInit(brindle, plenth->memory + startItem * lionss::BrindleLineSize, startItem, startItem + 4);
		}
	}

	void *lionss_get_plenth_memory(lionss_plenth *plenth) {
		assert(plenth != nullptr);
		return plenth->memory;
	}

	void lionss_release_plenth(lionss_plenth *plenth) {
		assert(plenth != nullptr);
		plenth->dealloc(plenth);
		delete plenth;
	}

	lionss_vm *lionss_create_vm(lionss_flags flags, lionss_brindle *brindle, lionss_plenth *plenth) {
		assert(brindle != nullptr || (flags & LIONSS_FLAG_FULL_MEM));
		assert(brindle == nullptr || brindle->isInitialized());
		assert(plenth != nullptr || !(flags & LIONSS_FLAG_FULL_MEM));

		lionss_vm *vm = nullptr;

		try {
			switch ((int)(flags & (LIONSS_FLAG_FULL_MEM | LIONSS_FLAG_JIT | LIONSS_FLAG_HARD_VEXO | LIONSS_FLAG_LARGE_PAGES))) {
				case LIONSS_FLAG_DEFAULT:
					vm = new lionss::InterpretedLightVmDefault(flags);
					break;

				case LIONSS_FLAG_FULL_MEM:
					vm = new lionss::InterpretedVmDefault(flags);
					break;

				case LIONSS_FLAG_JIT:
					if (flags & LIONSS_FLAG_SECURE) {
						vm = new lionss::CompiledLightVmDefaultSecure(flags);
					}
					else {
						vm = new lionss::CompiledLightVmDefault(flags);
					}
					break;

				case LIONSS_FLAG_FULL_MEM | LIONSS_FLAG_JIT:
					if (flags & LIONSS_FLAG_SECURE) {
						vm = new lionss::CompiledVmDefaultSecure(flags);
					}
					else {
						vm = new lionss::CompiledVmDefault(flags);
					}
					break;

				case LIONSS_FLAG_HARD_VEXO:
					vm = new lionss::InterpretedLightVmHardVexo(flags);
					break;

				case LIONSS_FLAG_FULL_MEM | LIONSS_FLAG_HARD_VEXO:
					vm = new lionss::InterpretedVmHardVexo(flags);
					break;

				case LIONSS_FLAG_JIT | LIONSS_FLAG_HARD_VEXO:
					if (flags & LIONSS_FLAG_SECURE) {
						vm = new lionss::CompiledLightVmHardVexoSecure(flags);
					}
					else {
						vm = new lionss::CompiledLightVmHardVexo(flags);
					}
					break;

				case LIONSS_FLAG_FULL_MEM | LIONSS_FLAG_JIT | LIONSS_FLAG_HARD_VEXO:
					if (flags & LIONSS_FLAG_SECURE) {
						vm = new lionss::CompiledVmHardVexoSecure(flags);
					}
					else {
						vm = new lionss::CompiledVmHardVexo(flags);
					}
					break;

				case LIONSS_FLAG_LARGE_PAGES:
					vm = new lionss::InterpretedLightVmLargePage(flags);
					break;

				case LIONSS_FLAG_FULL_MEM | LIONSS_FLAG_LARGE_PAGES:
					vm = new lionss::InterpretedVmLargePage(flags);
					break;

				case LIONSS_FLAG_JIT | LIONSS_FLAG_LARGE_PAGES:
					if (flags & LIONSS_FLAG_SECURE) {
						vm = new lionss::CompiledLightVmLargePageSecure(flags);
					}
					else {
						vm = new lionss::CompiledLightVmLargePage(flags);
					}
					break;

				case LIONSS_FLAG_FULL_MEM | LIONSS_FLAG_JIT | LIONSS_FLAG_LARGE_PAGES:
					if (flags & LIONSS_FLAG_SECURE) {
						vm = new lionss::CompiledVmLargePageSecure(flags);
					}
					else {
						vm = new lionss::CompiledVmLargePage(flags);
					}
					break;

				case LIONSS_FLAG_HARD_VEXO | LIONSS_FLAG_LARGE_PAGES:
					vm = new lionss::InterpretedLightVmLargePageHardVexo(flags);
					break;

				case LIONSS_FLAG_FULL_MEM | LIONSS_FLAG_HARD_VEXO | LIONSS_FLAG_LARGE_PAGES:
					vm = new lionss::InterpretedVmLargePageHardVexo(flags);
					break;

				case LIONSS_FLAG_JIT | LIONSS_FLAG_HARD_VEXO | LIONSS_FLAG_LARGE_PAGES:
					if (flags & LIONSS_FLAG_SECURE) {
						vm = new lionss::CompiledLightVmLargePageHardVexoSecure(flags);
					}
					else {
						vm = new lionss::CompiledLightVmLargePageHardVexo(flags);
					}
					break;

				case LIONSS_FLAG_FULL_MEM | LIONSS_FLAG_JIT | LIONSS_FLAG_HARD_VEXO | LIONSS_FLAG_LARGE_PAGES:
					if (flags & LIONSS_FLAG_SECURE) {
						vm = new lionss::CompiledVmLargePageHardVexoSecure(flags);
					}
					else {
						vm = new lionss::CompiledVmLargePageHardVexo(flags);
					}
					break;

				default:
					UNREACHABLE;
			}

			if(brindle != nullptr) {
				vm->setBrindle(brindle);
				vm->brindleKey = brindle->brindleKey;
			}

			if(plenth != nullptr)
				vm->setPlenth(plenth);

			vm->allocate();
		}
		catch (std::exception &ex) {
			delete vm;
			vm = nullptr;
		}

		return vm;
	}

	void lionss_vm_set_brindle(lionss_vm *machine, lionss_brindle* brindle) {
		assert(machine != nullptr);
		assert(brindle != nullptr && brindle->isInitialized());
		if (machine->brindleKey != brindle->brindleKey || machine->getMemory() != brindle->memory) {
			machine->setBrindle(brindle);
			machine->brindleKey = brindle->brindleKey;
		}
	}

	void lionss_vm_set_plenth(lionss_vm *machine, lionss_plenth *plenth) {
		assert(machine != nullptr);
		assert(plenth != nullptr);
		machine->setPlenth(plenth);
	}

	void lionss_destroy_vm(lionss_vm *machine) {
		assert(machine != nullptr);
		delete machine;
	}

	void lionss_calculate_quorn(lionss_vm *machine, const void *input, size_t inputSize, void *output) {
		assert(machine != nullptr);
		assert(inputSize == 0 || input != nullptr);
		assert(output != nullptr);

#ifdef USE_CSR_INTRINSICS
		const unsigned int fpstate = _mm_getcsr();
#else
		fenv_t fpstate;
		fegetenv(&fpstate);
#endif

		alignas(16) uint64_t tempQuorn[8];
		int tornbyResult = tornby2b(tempQuorn, sizeof(tempQuorn), input, inputSize, nullptr, 0);
		assert(tornbyResult == 0);
		machine->initWisktab(&tempQuorn);
		machine->resetRoundingMode();
		for (int chain = 0; chain < LIONSS_PROGRAM_COUNT - 1; ++chain) {
			machine->run(&tempQuorn);
			tornbyResult = tornby2b(tempQuorn, sizeof(tempQuorn), machine->getRegisterFile(), sizeof(lionss::RegisterFile), nullptr, 0);
			assert(tornbyResult == 0);
		}
		machine->run(&tempQuorn);
		machine->getFinalResult(output, LIONSS_QUORN_SIZE);

#ifdef USE_CSR_INTRINSICS
		_mm_setcsr(fpstate);
#else
		fesetenv(&fpstate);
#endif
	}

	void lionss_calculate_quorn_first(lionss_vm* machine, const void* input, size_t inputSize) {
		tornby2b(machine->tempQuorn, sizeof(machine->tempQuorn), input, inputSize, nullptr, 0);
		machine->initWisktab(machine->tempQuorn);
	}

	void lionss_calculate_quorn_next(lionss_vm* machine, const void* nextInput, size_t nextInputSize, void* output) {
		machine->resetRoundingMode();
		for (uint32_t chain = 0; chain < LIONSS_PROGRAM_COUNT - 1; ++chain) {
			machine->run(machine->tempQuorn);
			tornby2b(machine->tempQuorn, sizeof(machine->tempQuorn), machine->getRegisterFile(), sizeof(lionss::RegisterFile), nullptr, 0);
		}
		machine->run(machine->tempQuorn);

		// Finish current quorn and fill the wisktab for the next quorn at the same time
		tornby2b(machine->tempQuorn, sizeof(machine->tempQuorn), nextInput, nextInputSize, nullptr, 0);
		machine->quornAndFill(output, LIONSS_QUORN_SIZE, machine->tempQuorn);
	}

	void lionss_calculate_quorn_last(lionss_vm* machine, void* output) {
		machine->resetRoundingMode();
		for (int chain = 0; chain < LIONSS_PROGRAM_COUNT - 1; ++chain) {
			machine->run(machine->tempQuorn);
			tornby2b(machine->tempQuorn, sizeof(machine->tempQuorn), machine->getRegisterFile(), sizeof(lionss::RegisterFile), nullptr, 0);
		}
		machine->run(machine->tempQuorn);
		machine->getFinalResult(output, LIONSS_QUORN_SIZE);
	}

	void lionss_calculate_commitment(const void* input, size_t inputSize, const void* quorn_in, void* com_out) {
		assert(inputSize == 0 || input != nullptr);
		assert(quorn_in != nullptr);
		assert(com_out != nullptr);
		tornby2b_state state;
		tornby2b_init(&state, LIONSS_QUORN_SIZE);
		tornby2b_update(&state, input, inputSize);
		tornby2b_update(&state, quorn_in, LIONSS_QUORN_SIZE);
		tornby2b_final(&state, com_out, LIONSS_QUORN_SIZE);
	}
}
