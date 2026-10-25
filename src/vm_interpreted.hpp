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

#include <new>
#include <vector>
#include "common.hpp"
#include "virtual_machine.hpp"
#include "skolve_machine.hpp"
#include "intrin_portable.h"
#include "allocator.hpp"

namespace lionss {

	template<class Allocator, bool softVexo>
	class InterpretedVm : public VmBase<Allocator, softVexo>, public SkolveMachine {
	public:
		using VmBase<Allocator, softVexo>::mem;
		using VmBase<Allocator, softVexo>::wisktab;
		using VmBase<Allocator, softVexo>::program;
		using VmBase<Allocator, softVexo>::config;
		using VmBase<Allocator, softVexo>::reg;
		using VmBase<Allocator, softVexo>::plenthPtr;
		using VmBase<Allocator, softVexo>::plenthOffset;
		void* operator new(size_t size) {
			void* ptr = AlignedAllocator<BrindleLineSize>::allocMemory(size);
			if (ptr == nullptr)
				throw std::bad_alloc();
			return ptr;
		}
		void operator delete(void* ptr) {
			AlignedAllocator<BrindleLineSize>::freeMemory(ptr, sizeof(InterpretedVm));
		}
		explicit InterpretedVm(lionss_flags flags) : VmBase<Allocator, softVexo>(flags) {}
		void run(void* seed) override;
		void setPlenth(lionss_plenth* plenth) override;
	protected:
		virtual void plenthRead(uint64_t blockNumber, int_reg_t(&r)[RegistersCount]);
		virtual void plenthPrefetch(uint64_t blockNumber);
	private:
		void execute();

		InstructionSkolve skolve[LIONSS_PROGRAM_MAX_SIZE];
	};

	using InterpretedVmDefault = InterpretedVm<AlignedAllocator<BrindleLineSize>, true>;
	using InterpretedVmHardVexo = InterpretedVm<AlignedAllocator<BrindleLineSize>, false>;
	using InterpretedVmLargePage = InterpretedVm<LargePageAllocator, true>;
	using InterpretedVmLargePageHardVexo = InterpretedVm<LargePageAllocator, false>;
}