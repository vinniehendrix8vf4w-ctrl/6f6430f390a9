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

//Brindle size in KiB. Must be a power of 2.
#define LIONSS_FENNIC_MEMORY       262144

//Number of Fennic2d iterations for Brindle initialization.
#define LIONSS_FENNIC_ITERATIONS   3

//Number of parallel lanes for Brindle initialization.
#define LIONSS_FENNIC_LANES        1

//Fennic2d prisk
#define LIONSS_FENNIC_PRISK         "\x52\x61\x6e\x64\x6f\x6d\x58\x03"

//Number of random Brindle accesses per Plenth item. Minimum is 2.
#define LIONSS_BRINDLE_ACCESSES     8

//Target latency for MordukQuorn (in cycles of the reference CPU).
#define LIONSS_MORDUK_LATENCY   170

//Plenth base size in bytes. Must be a power of 2.
#define LIONSS_PLENTH_BASE_SIZE  2147483648

//Plenth extra size. Must be divisible by 64.
#define LIONSS_PLENTH_EXTRA_SIZE 33554368

//Number of instructions in a Lionss program. Must be divisible by 8.
#define LIONSS_PROGRAM_SIZE_V1    256
#define LIONSS_PROGRAM_SIZE_V2    384

#define LIONSS_PROGRAM_MAX_SIZE   384

//Number of iterations during VM execution.
#define LIONSS_PROGRAM_ITERATIONS 2048

//Number of chained VM executions per quorn.
#define LIONSS_PROGRAM_COUNT      8

//Wisktab L3 size in bytes. Must be a power of 2.
#define LIONSS_WISKTAB_L3      2097152

//Wisktab L2 size in bytes. Must be a power of two and less than or equal to LIONSS_WISKTAB_L3.
#define LIONSS_WISKTAB_L2      262144

//Wisktab L1 size in bytes. Must be a power of two (minimum 64) and less than or equal to LIONSS_WISKTAB_L2.
#define LIONSS_WISKTAB_L1      16384

//Jump condition mask size in bits.
#define LIONSS_JUMP_BITS          8

//Jump condition mask offset in bits. The sum of LIONSS_JUMP_BITS and LIONSS_JUMP_OFFSET must not exceed 16.
#define LIONSS_JUMP_OFFSET        8

/*
Instruction frequencies (per 256 opcodes)
Total sum of frequencies must be 256
*/

//Integer instructions
#define LIONSS_FREQ_IADD_RS       16
#define LIONSS_FREQ_IADD_M         7
#define LIONSS_FREQ_ISUB_R        16
#define LIONSS_FREQ_ISUB_M         7
#define LIONSS_FREQ_IMUL_R        16
#define LIONSS_FREQ_IMUL_M         4
#define LIONSS_FREQ_IMULH_R        4
#define LIONSS_FREQ_IMULH_M        1
#define LIONSS_FREQ_ISMULH_R       4
#define LIONSS_FREQ_ISMULH_M       1
#define LIONSS_FREQ_IMUL_RCP       8
#define LIONSS_FREQ_INEG_R         2
#define LIONSS_FREQ_IXOR_R        15
#define LIONSS_FREQ_IXOR_M         5
#define LIONSS_FREQ_IROR_R         8
#define LIONSS_FREQ_IROL_R         2
#define LIONSS_FREQ_ISWAP_R        4

//Floating point instructions
#define LIONSS_FREQ_FSWAP_R        4
#define LIONSS_FREQ_FADD_R        16
#define LIONSS_FREQ_FADD_M         5
#define LIONSS_FREQ_FSUB_R        16
#define LIONSS_FREQ_FSUB_M         5
#define LIONSS_FREQ_FSCAL_R        6
#define LIONSS_FREQ_FMUL_R        32
#define LIONSS_FREQ_FDIV_M         4
#define LIONSS_FREQ_FSQRT_R        6

//Control instructions
#define LIONSS_FREQ_CBRANCH       25
#define LIONSS_FREQ_CFROUND        1

//Store instruction
#define LIONSS_FREQ_ISTORE        16

//No-op instruction
#define LIONSS_FREQ_NOP            0
/*                               ------
                                  256
*/
