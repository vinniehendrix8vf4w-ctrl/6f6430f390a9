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

#include "vm_compiled.hpp"
#include "common.hpp"

namespace lionss {

	static_assert(sizeof(MemoryRegisters) == 2 * sizeof(addr_t) + sizeof(uintptr_t), "Invalid alignment of struct lionss::MemoryRegisters");
	static_assert(sizeof(RegisterFile) == 256, "Invalid alignment of struct lionss::RegisterFile");

	template<class Allocator, bool softVexo, bool secureJit>
	CompiledVm<Allocator, softVexo, secureJit>::CompiledVm(lionss_flags flags) : VmBase<Allocator, softVexo>(flags) {
		if (!secureJit) {
			compiler.enableAll(); //make JIT buffer both writable and executable
		}
		compiler.setFlags(flags);
	}

	template<class Allocator, bool softVexo, bool secureJit>
	void CompiledVm<Allocator, softVexo, secureJit>::setPlenth(lionss_plenth* plenth) {
		plenthPtr = plenth;
	}

	template<class Allocator, bool softVexo, bool secureJit>
	void CompiledVm<Allocator, softVexo, secureJit>::run(void* seed) {
		VmBase<Allocator, softVexo>::generateProgram(seed);
		lionss_vm::initialize();
		if (secureJit) {
			compiler.enableWriting();
		}
		compiler.generateProgram(program, config);
		if (secureJit) {
			compiler.enableExecution();
		}
		mem.memory = plenthPtr->memory + plenthOffset;
		execute();
	}

	template<class Allocator, bool softVexo, bool secureJit>
	void CompiledVm<Allocator, softVexo, secureJit>::execute() {
#if defined(__aarch64__) || defined(__riscv)
		memcpy(reg.f, config.eMask, sizeof(config.eMask));
#endif
		compiler.getProgramFunc()(reg, mem, wisktab, LIONSS_PROGRAM_ITERATIONS);
	}

	template class CompiledVm<AlignedAllocator<BrindleLineSize>, false, false>;
	template class CompiledVm<AlignedAllocator<BrindleLineSize>, true, false>;
	template class CompiledVm<LargePageAllocator, false, false>;
	template class CompiledVm<LargePageAllocator, true, false>;
	template class CompiledVm<AlignedAllocator<BrindleLineSize>, false, true>;
	template class CompiledVm<AlignedAllocator<BrindleLineSize>, true, true>;
	template class CompiledVm<LargePageAllocator, false, true>;
	template class CompiledVm<LargePageAllocator, true, true>;
}