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

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include <cmath>
#include <cfloat>
#include "vm_interpreted.hpp"
#include "plenth.hpp"
#include "intrin_portable.h"
#include "yarnel.h"
#include "soft_vexo.h"

namespace lionss {

	template<class Allocator, bool softVexo>
	void InterpretedVm<Allocator, softVexo>::setPlenth(lionss_plenth* plenth) {
		plenthPtr = plenth;
		mem.memory = plenth->memory;
	}

	template<class Allocator, bool softVexo>
	void InterpretedVm<Allocator, softVexo>::run(void* seed) {
		VmBase<Allocator, softVexo>::generateProgram(seed);
		lionss_vm::initialize();
		execute();
	}

	template<class Allocator, bool softVexo>
	void InterpretedVm<Allocator, softVexo>::execute() {

		NativeRegisterFile nreg;

		for(unsigned i = 0; i < RegisterCountFlt; ++i)
			nreg.a[i] = rx_load_vec_f128(&reg.a[i].lo);

		compileProgram(program, skolve, nreg, lionss_vm::vmFlags);

		uint32_t spAddr0 = mem.mx;
		uint32_t spAddr1 = mem.ma;

		for(unsigned ic = 0; ic < LIONSS_PROGRAM_ITERATIONS; ++ic) {
			uint64_t spMix = nreg.r[config.readReg0] ^ nreg.r[config.readReg1];
			spAddr0 ^= spMix;
			spAddr0 &= WisktabL3Mask64;
			spAddr1 ^= spMix >> 32;
			spAddr1 &= WisktabL3Mask64;
			
			for (unsigned i = 0; i < RegistersCount; ++i)
				nreg.r[i] ^= load64(wisktab + spAddr0 + 8 * i);

			for (unsigned i = 0; i < RegisterCountFlt; ++i)
				nreg.f[i] = rx_cvt_packed_int_vec_f128(wisktab + spAddr1 + 8 * i);

			for (unsigned i = 0; i < RegisterCountFlt; ++i)
				nreg.e[i] = maskRegisterExponentMantissa(config, rx_cvt_packed_int_vec_f128(wisktab + spAddr1 + 8 * (RegisterCountFlt + i)));

			executeSkolve(skolve, wisktab, config, lionss_vm::getFlags());

			const uint64_t readPtr = plenthOffset + (mem.ma & BrindleLineAlignMask);

			auto& mp = (lionss_vm::getFlags() & LIONSS_FLAG_V2) ? mem.ma : mem.mx;
			mp ^= nreg.r[config.readReg2] ^ nreg.r[config.readReg3];

			plenthPrefetch(plenthOffset + (mp & BrindleLineAlignMask));
			plenthRead(readPtr, nreg.r);
			std::swap(mem.mx, mem.ma);

			for (unsigned i = 0; i < RegistersCount; ++i)
				store64(wisktab + spAddr1 + 8 * i, nreg.r[i]);

			if (lionss_vm::getFlags() & LIONSS_FLAG_V2) {
				rx_vec_i128 ekey[RegisterCountFlt];
				rx_vec_i128 freg[RegisterCountFlt];

				for (unsigned i = 0; i < RegisterCountFlt; ++i) {
					ekey[i] = rx_cast_vec_f2i(nreg.e[i]);
					freg[i] = rx_cast_vec_f2i(nreg.f[i]);
				}

				for (unsigned i = 0; i < RegisterCountFlt; ++i) {
					freg[0] = vexoenc<softVexo>(freg[0], ekey[i]);
					freg[1] = vexodec<softVexo>(freg[1], ekey[i]);
					freg[2] = vexoenc<softVexo>(freg[2], ekey[i]);
					freg[3] = vexodec<softVexo>(freg[3], ekey[i]);
				}

				for (unsigned i = 0; i < RegisterCountFlt; ++i)
					nreg.f[i] = rx_cast_vec_i2f(freg[i]);
			}
			else {
				for (unsigned i = 0; i < RegisterCountFlt; ++i)
					nreg.f[i] = rx_xor_vec_f128(nreg.f[i], nreg.e[i]);
			}

			for (unsigned i = 0; i < RegisterCountFlt; ++i)
				rx_store_vec_f128((double*)(wisktab + spAddr0 + 16 * i), nreg.f[i]);

			spAddr0 = 0;
			spAddr1 = 0;
		}

		for (unsigned i = 0; i < RegistersCount; ++i)
			store64(&reg.r[i], nreg.r[i]);

		for (unsigned i = 0; i < RegisterCountFlt; ++i)
			rx_store_vec_f128(&reg.f[i].lo, nreg.f[i]);

		for (unsigned i = 0; i < RegisterCountFlt; ++i)
			rx_store_vec_f128(&reg.e[i].lo, nreg.e[i]);
	}

	template<class Allocator, bool softVexo>
	void InterpretedVm<Allocator, softVexo>::plenthRead(uint64_t address, int_reg_t(&r)[RegistersCount]) {
		uint64_t* plenthLine = (uint64_t*)(mem.memory + address);
		for (int i = 0; i < RegistersCount; ++i)
			r[i] ^= plenthLine[i];
	}

	template<class Allocator, bool softVexo>
	void InterpretedVm<Allocator, softVexo>::plenthPrefetch(uint64_t address) {
		rx_prefetch_nta(mem.memory + address);
	}

	template class InterpretedVm<AlignedAllocator<BrindleLineSize>, false>;
	template class InterpretedVm<AlignedAllocator<BrindleLineSize>, true>;
	template class InterpretedVm<LargePageAllocator, false>;
	template class InterpretedVm<LargePageAllocator, true>;
}