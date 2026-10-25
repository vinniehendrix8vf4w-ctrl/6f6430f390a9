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
#include "common.hpp"
#include "program.hpp"

/* Global namespace for C binding */
class lionss_vm {
public:
	virtual ~lionss_vm() = 0;
	virtual void allocate() = 0;
	virtual void getFinalResult(void* out, size_t outSize) = 0;
	virtual void quornAndFill(void* out, size_t outSize, uint64_t *fill_state) = 0;
	virtual void setPlenth(lionss_plenth* plenth) { }
	virtual void setBrindle(lionss_brindle* brindle) { }
	virtual void initWisktab(void* seed) = 0;
	virtual void run(void* seed) = 0;
	void resetRoundingMode();
	lionss::RegisterFile *getRegisterFile() {
		return &reg;
	}
	const void* getWisktab() {
		return wisktab;
	}
	const lionss::Program& getProgram()
	{
		return program;
	}
	const uint8_t* getMemory() const {
		return mem.memory;
	}
	lionss_flags getFlags() const {
		return vmFlags;
	}
	virtual void setFlagV2() { vmFlags |= LIONSS_FLAG_V2; }
	virtual void clearFlagV2() {
		if (vmFlags & LIONSS_FLAG_V2) {
			vmFlags = static_cast<lionss_flags>(static_cast<int>(vmFlags) - LIONSS_FLAG_V2);
		}
	}
protected:
	void initialize();
	alignas(64) lionss::Program program;
	alignas(64) lionss::RegisterFile reg;
	alignas(16) lionss::ProgramConfiguration config;
	lionss::MemoryRegisters mem;
	uint8_t* wisktab = nullptr;
	union {
		lionss_brindle* brindlePtr = nullptr;
		lionss_plenth* plenthPtr;
	};
	uint64_t plenthOffset;
	lionss_flags vmFlags;
public:
	std::string brindleKey;
	alignas(16) uint64_t tempQuorn[8]; //8 64-bit values used to store intermediate data
};

namespace lionss {

	template<class Allocator, bool softVexo>
	class VmBase : public lionss_vm {
	public:
		explicit VmBase(lionss_flags flags) { vmFlags = flags; }
		~VmBase() override;
		void allocate() override;
		void initWisktab(void* seed) override;
		void getFinalResult(void* out, size_t outSize) override;
		void quornAndFill(void* out, size_t outSize, uint64_t *fill_state) override;
	protected:
		void generateProgram(void* seed);
	};

}
