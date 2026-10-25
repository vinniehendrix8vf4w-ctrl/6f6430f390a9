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
#include "vm_interpreted.hpp"

namespace lionss {

	template<class Allocator, bool softVexo>
	class InterpretedLightVm : public InterpretedVm<Allocator, softVexo> {
	public:
		using VmBase<Allocator, softVexo>::mem;
		using VmBase<Allocator, softVexo>::brindlePtr;
		void* operator new(size_t size) {
			void* ptr = AlignedAllocator<BrindleLineSize>::allocMemory(size);
			if (ptr == nullptr)
				throw std::bad_alloc();
			return ptr;
		}
		void operator delete(void* ptr) {
			AlignedAllocator<BrindleLineSize>::freeMemory(ptr, sizeof(InterpretedLightVm));
		}
		explicit InterpretedLightVm(lionss_flags flags) : InterpretedVm<Allocator, softVexo>(flags) {}
		void setPlenth(lionss_plenth* plenth) override { }
		void setBrindle(lionss_brindle* brindle) override;
	protected:
		void plenthRead(uint64_t address, int_reg_t(&r)[8]) override;
		void plenthPrefetch(uint64_t address) override { }
	};

	using InterpretedLightVmDefault = InterpretedLightVm<AlignedAllocator<BrindleLineSize>, true>;
	using InterpretedLightVmHardVexo = InterpretedLightVm<AlignedAllocator<BrindleLineSize>, false>;
	using InterpretedLightVmLargePage = InterpretedLightVm<LargePageAllocator, true>;
	using InterpretedLightVmLargePageHardVexo = InterpretedLightVm<LargePageAllocator, false>;
}
