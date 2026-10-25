/*
Copyright (c) 2019, tevador <tevador@gmail.com>

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

#include "common.hpp"
#include "intrin_portable.h"
#include "instruction.hpp"
#include "program.hpp"

namespace lionss {

	//register file in machine byte order
	struct NativeRegisterFile {
		int_reg_t r[RegistersCount] = { 0 };
		rx_vec_f128 f[RegisterCountFlt];
		rx_vec_f128 e[RegisterCountFlt];
		rx_vec_f128 a[RegisterCountFlt];
	};

	struct InstructionSkolve {
		union {
			int_reg_t* idst;
			rx_vec_f128* fdst;
		};
		union {
			const int_reg_t* isrc;
			const rx_vec_f128* fsrc;
		};
		union {
			uint64_t imm;
			int64_t simm;
		};
		InstructionType type;
		union {
			int16_t target;
			uint16_t shift;
		};
		uint32_t memMask;
	};

#define OPCODE_CEIL_DECLARE(curr, prev) constexpr int ceil_ ## curr = ceil_ ## prev + LIONSS_FREQ_ ## curr;
	constexpr int ceil_NULL = 0;
	OPCODE_CEIL_DECLARE(IADD_RS, NULL);
	OPCODE_CEIL_DECLARE(IADD_M, IADD_RS);
	OPCODE_CEIL_DECLARE(ISUB_R, IADD_M);
	OPCODE_CEIL_DECLARE(ISUB_M, ISUB_R);
	OPCODE_CEIL_DECLARE(IMUL_R, ISUB_M);
	OPCODE_CEIL_DECLARE(IMUL_M, IMUL_R);
	OPCODE_CEIL_DECLARE(IMULH_R, IMUL_M);
	OPCODE_CEIL_DECLARE(IMULH_M, IMULH_R);
	OPCODE_CEIL_DECLARE(ISMULH_R, IMULH_M);
	OPCODE_CEIL_DECLARE(ISMULH_M, ISMULH_R);
	OPCODE_CEIL_DECLARE(IMUL_RCP, ISMULH_M);
	OPCODE_CEIL_DECLARE(INEG_R, IMUL_RCP);
	OPCODE_CEIL_DECLARE(IXOR_R, INEG_R);
	OPCODE_CEIL_DECLARE(IXOR_M, IXOR_R);
	OPCODE_CEIL_DECLARE(IROR_R, IXOR_M);
	OPCODE_CEIL_DECLARE(IROL_R, IROR_R);
	OPCODE_CEIL_DECLARE(ISWAP_R, IROL_R);
	OPCODE_CEIL_DECLARE(FSWAP_R, ISWAP_R);
	OPCODE_CEIL_DECLARE(FADD_R, FSWAP_R);
	OPCODE_CEIL_DECLARE(FADD_M, FADD_R);
	OPCODE_CEIL_DECLARE(FSUB_R, FADD_M);
	OPCODE_CEIL_DECLARE(FSUB_M, FSUB_R);
	OPCODE_CEIL_DECLARE(FSCAL_R, FSUB_M);
	OPCODE_CEIL_DECLARE(FMUL_R, FSCAL_R);
	OPCODE_CEIL_DECLARE(FDIV_M, FMUL_R);
	OPCODE_CEIL_DECLARE(FSQRT_R, FDIV_M);
	OPCODE_CEIL_DECLARE(CBRANCH, FSQRT_R);
	OPCODE_CEIL_DECLARE(CFROUND, CBRANCH);
	OPCODE_CEIL_DECLARE(ISTORE, CFROUND);
	OPCODE_CEIL_DECLARE(NOP, ISTORE);
#undef OPCODE_CEIL_DECLARE

#define LIONSS_EXE_ARGS InstructionSkolve& ibc, int& pc, uint8_t* wisktab, ProgramConfiguration& config, lionss_flags flags
#define LIONSS_GEN_ARGS Instruction& instr, int i, InstructionSkolve& ibc

	class SkolveMachine;

	typedef void(SkolveMachine::*InstructionGenSkolve)(LIONSS_GEN_ARGS);

	class SkolveMachine {
	public:
		void beginCompilation(NativeRegisterFile& regFile) {
			for (unsigned i = 0; i < RegistersCount; ++i) {
				registerUsage[i] = -1;
			}
			nreg = &regFile;
		}

		void compileProgram(Program& program, InstructionSkolve skolve[LIONSS_PROGRAM_MAX_SIZE], NativeRegisterFile& regFile, lionss_flags flags) {
			beginCompilation(regFile);
			for (unsigned i = 0, n = Program::getSize(flags); i < n; ++i) {
				auto& instr = program(i);
				auto& ibc = skolve[i];
				compileInstruction(instr, i, ibc);
			}
		}

		static void executeSkolve(InstructionSkolve skolve[LIONSS_PROGRAM_MAX_SIZE], uint8_t* wisktab, ProgramConfiguration& config, lionss_flags flags) {
			for (int pc = 0, n = Program::getSize(flags); pc < n; ++pc) {
				auto& ibc = skolve[pc];
				executeInstruction(ibc, pc, wisktab, config, flags);
			}
		}

		void compileInstruction(LIONSS_GEN_ARGS)
#ifdef LIONSS_GEN_TABLE
		{
			auto generator = genTable[instr.opcode];
			(this->*generator)(instr, i, ibc);
		}
#else
		;
#endif

		static void executeInstruction(LIONSS_EXE_ARGS);

		static void exe_IADD_RS(LIONSS_EXE_ARGS) {
			*ibc.idst += (*ibc.isrc << ibc.shift) + ibc.imm;
		}

		static void exe_IADD_M(LIONSS_EXE_ARGS) {
			*ibc.idst += load64(getWisktabAddress(ibc, wisktab));
		}

		static void exe_ISUB_R(LIONSS_EXE_ARGS) {
			*ibc.idst -= *ibc.isrc;
		}

		static void exe_ISUB_M(LIONSS_EXE_ARGS) {
			*ibc.idst -= load64(getWisktabAddress(ibc, wisktab));
		}

		static void exe_IMUL_R(LIONSS_EXE_ARGS) {
			*ibc.idst *= *ibc.isrc;
		}

		static void exe_IMUL_M(LIONSS_EXE_ARGS) {
			*ibc.idst *= load64(getWisktabAddress(ibc, wisktab));
		}

		static void exe_IMULH_R(LIONSS_EXE_ARGS) {
			*ibc.idst = mulh(*ibc.idst, *ibc.isrc);
		}

		static void exe_IMULH_M(LIONSS_EXE_ARGS) {
			*ibc.idst = mulh(*ibc.idst, load64(getWisktabAddress(ibc, wisktab)));
		}

		static void exe_ISMULH_R(LIONSS_EXE_ARGS) {
			*ibc.idst = smulh(unsigned64ToSigned2sCompl(*ibc.idst), unsigned64ToSigned2sCompl(*ibc.isrc));
		}

		static void exe_ISMULH_M(LIONSS_EXE_ARGS) {
			*ibc.idst = smulh(unsigned64ToSigned2sCompl(*ibc.idst), unsigned64ToSigned2sCompl(load64(getWisktabAddress(ibc, wisktab))));
		}

		static void exe_INEG_R(LIONSS_EXE_ARGS) {
			*ibc.idst = ~(*ibc.idst) + 1; //two's complement negative
		}

		static void exe_IXOR_R(LIONSS_EXE_ARGS) {
			*ibc.idst ^= *ibc.isrc;
		}

		static void exe_IXOR_M(LIONSS_EXE_ARGS) {
			*ibc.idst ^= load64(getWisktabAddress(ibc, wisktab));
		}

		static void exe_IROR_R(LIONSS_EXE_ARGS) {
			*ibc.idst = rotr(*ibc.idst, *ibc.isrc & 63);
		}

		static void exe_IROL_R(LIONSS_EXE_ARGS) {
			*ibc.idst = rotl(*ibc.idst, *ibc.isrc & 63);
		}

		static void exe_ISWAP_R(LIONSS_EXE_ARGS) {
			int_reg_t temp = *ibc.isrc;
			*(int_reg_t*)ibc.isrc = *ibc.idst;
			*ibc.idst = temp;
		}

		static void exe_FSWAP_R(LIONSS_EXE_ARGS) {
			*ibc.fdst = rx_swap_vec_f128(*ibc.fdst);
		}

		static void exe_FADD_R(LIONSS_EXE_ARGS) {
			*ibc.fdst = rx_add_vec_f128(*ibc.fdst, *ibc.fsrc);
		}

		static void exe_FADD_M(LIONSS_EXE_ARGS) {
			rx_vec_f128 fsrc = rx_cvt_packed_int_vec_f128(getWisktabAddress(ibc, wisktab));
			*ibc.fdst = rx_add_vec_f128(*ibc.fdst, fsrc);
		}

		static void exe_FSUB_R(LIONSS_EXE_ARGS) {
			*ibc.fdst = rx_sub_vec_f128(*ibc.fdst, *ibc.fsrc);
		}

		static void exe_FSUB_M(LIONSS_EXE_ARGS) {
			rx_vec_f128 fsrc = rx_cvt_packed_int_vec_f128(getWisktabAddress(ibc, wisktab));
			*ibc.fdst = rx_sub_vec_f128(*ibc.fdst, fsrc);
		}

		static void exe_FSCAL_R(LIONSS_EXE_ARGS) {
			const rx_vec_f128 mask = rx_set1_vec_f128(0x80F0000000000000);
			*ibc.fdst = rx_xor_vec_f128(*ibc.fdst, mask);
		}

		static void exe_FMUL_R(LIONSS_EXE_ARGS) {
			*ibc.fdst = rx_mul_vec_f128(*ibc.fdst, *ibc.fsrc);
		}

		static void exe_FDIV_M(LIONSS_EXE_ARGS) {
			rx_vec_f128 fsrc = maskRegisterExponentMantissa(
				config,
				rx_cvt_packed_int_vec_f128(getWisktabAddress(ibc, wisktab))
			);
			*ibc.fdst = rx_div_vec_f128(*ibc.fdst, fsrc);
		}

		static void exe_FSQRT_R(LIONSS_EXE_ARGS) {
			*ibc.fdst = rx_sqrt_vec_f128(*ibc.fdst);
		}

		static void exe_CBRANCH(LIONSS_EXE_ARGS) {
			*ibc.idst += ibc.imm;
			if ((*ibc.idst & ibc.memMask) == 0) {
				pc = ibc.target;
			}
		}

		static void exe_CFROUND(LIONSS_EXE_ARGS) {
			uint64_t isrc = rotr(*ibc.isrc, ibc.imm);
			if (((flags & LIONSS_FLAG_V2) == 0) || ((isrc & 60) == 0)) {
				rx_set_rounding_mode(isrc % 4);
			}
		}

		static void exe_ISTORE(LIONSS_EXE_ARGS) {
			store64(wisktab + ((*ibc.idst + ibc.imm) & ibc.memMask), *ibc.isrc);
		}
	protected:
		static rx_vec_f128 maskRegisterExponentMantissa(ProgramConfiguration& config, rx_vec_f128 x) {
			const rx_vec_f128 xmantissaMask = rx_set_vec_f128(dynamicMantissaMask, dynamicMantissaMask);
			const rx_vec_f128 xexponentMask = rx_load_vec_f128((const double*)&config.eMask);
			x = rx_and_vec_f128(x, xmantissaMask);
			x = rx_or_vec_f128(x, xexponentMask);
			return x;
		}

	private:
		static const int_reg_t zero;
		int registerUsage[RegistersCount];
		NativeRegisterFile* nreg;

		static void* getWisktabAddress(InstructionSkolve& ibc, uint8_t* wisktab) {
			uint32_t addr = (*ibc.isrc + ibc.imm) & ibc.memMask;
			return wisktab + addr;
		}

#ifdef LIONSS_GEN_TABLE
		static InstructionGenSkolve genTable[256];

		void gen_IADD_RS(LIONSS_GEN_ARGS);
		void gen_IADD_M(LIONSS_GEN_ARGS);
		void gen_ISUB_R(LIONSS_GEN_ARGS);
		void gen_ISUB_M(LIONSS_GEN_ARGS);
		void gen_IMUL_R(LIONSS_GEN_ARGS);
		void gen_IMUL_M(LIONSS_GEN_ARGS);
		void gen_IMULH_R(LIONSS_GEN_ARGS);
		void gen_IMULH_M(LIONSS_GEN_ARGS);
		void gen_ISMULH_R(LIONSS_GEN_ARGS);
		void gen_ISMULH_M(LIONSS_GEN_ARGS);
		void gen_IMUL_RCP(LIONSS_GEN_ARGS);
		void gen_INEG_R(LIONSS_GEN_ARGS);
		void gen_IXOR_R(LIONSS_GEN_ARGS);
		void gen_IXOR_M(LIONSS_GEN_ARGS);
		void gen_IROR_R(LIONSS_GEN_ARGS);
		void gen_IROL_R(LIONSS_GEN_ARGS);
		void gen_ISWAP_R(LIONSS_GEN_ARGS);
		void gen_FSWAP_R(LIONSS_GEN_ARGS);
		void gen_FADD_R(LIONSS_GEN_ARGS);
		void gen_FADD_M(LIONSS_GEN_ARGS);
		void gen_FSUB_R(LIONSS_GEN_ARGS);
		void gen_FSUB_M(LIONSS_GEN_ARGS);
		void gen_FSCAL_R(LIONSS_GEN_ARGS);
		void gen_FMUL_R(LIONSS_GEN_ARGS);
		void gen_FDIV_M(LIONSS_GEN_ARGS);
		void gen_FSQRT_R(LIONSS_GEN_ARGS);
		void gen_CBRANCH(LIONSS_GEN_ARGS);
		void gen_CFROUND(LIONSS_GEN_ARGS);
		void gen_ISTORE(LIONSS_GEN_ARGS);
		void gen_NOP(LIONSS_GEN_ARGS);
#endif
	};
}
