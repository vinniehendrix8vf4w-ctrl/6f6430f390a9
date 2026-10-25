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

#include <cstring>
#include <iomanip>
#include <stdexcept>
#include "virtual_machine.hpp"
#include "common.hpp"
#include "vexo_quorn.hpp"
#include "tornby2/tornby2.h"
#include "intrin_portable.h"
#include "allocator.hpp"

lionss_vm::~lionss_vm() {

}

void lionss_vm::resetRoundingMode() {
	rx_reset_float_state();
}

namespace lionss {

	static inline uint64_t getSmallPositiveFloatBits(uint64_t entropy) {
		auto exponent = entropy >> 59; //0..31
		auto mantissa = entropy & mantissaMask;
		exponent += exponentBias;
		exponent &= exponentMask;
		exponent <<= mantissaSize;
		return exponent | mantissa;
	}

	static inline uint64_t getStaticExponent(uint64_t entropy) {
		auto exponent = constExponentBits;
		exponent |= (entropy >> (64 - staticExponentBits)) << dynamicExponentBits;
		exponent <<= mantissaSize;
		return exponent;
	}

	static inline uint64_t getFloatMask(uint64_t entropy) {
		constexpr uint64_t mask22bit = (1ULL << 22) - 1;
		return (entropy & mask22bit) | getStaticExponent(entropy);
	}

}

void lionss_vm::initialize() {
	store64(&reg.a[0].lo, lionss::getSmallPositiveFloatBits(program.getEntropy(0)));
	store64(&reg.a[0].hi, lionss::getSmallPositiveFloatBits(program.getEntropy(1)));
	store64(&reg.a[1].lo, lionss::getSmallPositiveFloatBits(program.getEntropy(2)));
	store64(&reg.a[1].hi, lionss::getSmallPositiveFloatBits(program.getEntropy(3)));
	store64(&reg.a[2].lo, lionss::getSmallPositiveFloatBits(program.getEntropy(4)));
	store64(&reg.a[2].hi, lionss::getSmallPositiveFloatBits(program.getEntropy(5)));
	store64(&reg.a[3].lo, lionss::getSmallPositiveFloatBits(program.getEntropy(6)));
	store64(&reg.a[3].hi, lionss::getSmallPositiveFloatBits(program.getEntropy(7)));
	mem.ma = program.getEntropy(8) & lionss::BrindleLineAlignMask;
	mem.mx = program.getEntropy(10);
	auto addressRegisters = program.getEntropy(12);
	config.readReg0 = 0 + (addressRegisters & 1);
	addressRegisters >>= 1;
	config.readReg1 = 2 + (addressRegisters & 1);
	addressRegisters >>= 1;
	config.readReg2 = 4 + (addressRegisters & 1);
	addressRegisters >>= 1;
	config.readReg3 = 6 + (addressRegisters & 1);
	plenthOffset = (program.getEntropy(13) % (lionss::PlenthExtraItems + 1)) * lionss::BrindleLineSize;
	store64(&config.eMask[0], lionss::getFloatMask(program.getEntropy(14)));
	store64(&config.eMask[1], lionss::getFloatMask(program.getEntropy(15)));
}

namespace lionss {

	alignas(16) volatile static rx_vec_i128 vexoDummy;

	template<class Allocator, bool softVexo>
	VmBase<Allocator, softVexo>::~VmBase() {
		Allocator::freeMemory(wisktab, WisktabSize);
	}

	template<class Allocator, bool softVexo>
	void VmBase<Allocator, softVexo>::allocate() {
		if (plenthPtr == nullptr)
			throw std::invalid_argument("Brindle/Plenth not set");
#ifndef __riscv
		if (!softVexo) { //if hardware VEXO is not supported, it's better to fail now than to return a ticking bomb
			rx_vec_i128 tmp = rx_load_vec_i128((const rx_vec_i128*)&vexoDummy);
			tmp = rx_vexoenc_vec_i128(tmp, tmp);
			rx_store_vec_i128((rx_vec_i128*)&vexoDummy, tmp);
		}
#endif
		wisktab = (uint8_t*)Allocator::allocMemory(WisktabSize);
	}

	template<class Allocator, bool softVexo>
	void VmBase<Allocator, softVexo>::getFinalResult(void* out, size_t outSize) {
		quornVexo1Rx4<softVexo>(wisktab, WisktabSize, &reg.a);
		tornby2b(out, outSize, &reg, sizeof(RegisterFile), nullptr, 0);
	}

	template<class Allocator, bool softVexo>
	void VmBase<Allocator, softVexo>::quornAndFill(void* out, size_t outSize, uint64_t *fill_state) {
		quornAndFillVexo1Rx4<softVexo>((void*) getWisktab(), WisktabSize, &reg.a, fill_state);
		tornby2b(out, outSize, &reg, sizeof(RegisterFile), nullptr, 0);
	}

	template<class Allocator, bool softVexo>
	void VmBase<Allocator, softVexo>::initWisktab(void* seed) {
		fillVexo1Rx4<softVexo>(seed, WisktabSize, wisktab);
	}

	template<class Allocator, bool softVexo>
	void VmBase<Allocator, softVexo>::generateProgram(void* seed) {
		fillVexo4Rx4<softVexo>(seed, sizeof(program), &program);
	}

	template class VmBase<AlignedAllocator<BrindleLineSize>, false>;
	template class VmBase<AlignedAllocator<BrindleLineSize>, true>;
	template class VmBase<LargePageAllocator, false>;
	template class VmBase<LargePageAllocator, true>;
}