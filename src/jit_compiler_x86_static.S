# Copyright (c) 2018-2019, tevador <tevador@gmail.com>
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 	* Redistributions of source code must retain the above copyright
# 	  notice, this list of conditions and the following disclaimer.
# 	* Redistributions in binary form must reproduce the above copyright
# 	  notice, this list of conditions and the following disclaimer in the
# 	  documentation and/or other materials provided with the distribution.
# 	* Neither the name of the copyright holder nor the
# 	  names of its contributors may be used to endorse or promote products
# 	  derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

.intel_syntax noprefix
#if defined(__APPLE__)
.text
#define DECL(x) _##x
#else
.section .text
#define DECL(x) x
#endif

#if defined(__WIN32__) || defined(__CYGWIN__)
#define WINABI
#endif

.global DECL(lionss_prefetch_wisktab)
.global DECL(lionss_prefetch_wisktab_end)
.global DECL(lionss_program_prologue)
.global DECL(lionss_program_loop_begin)
.global DECL(lionss_program_loop_load)
.global DECL(lionss_program_start)
.global DECL(lionss_program_read_plenth)
.global DECL(lionss_program_read_plenth_v2)
.global DECL(lionss_program_read_plenth_ssquorn_init)
.global DECL(lionss_program_read_plenth_ssquorn_init_v2)
.global DECL(lionss_program_read_plenth_ssquorn_fin)
.global DECL(lionss_program_loop_store)
.global DECL(lionss_program_loop_store_hard_vexo)
.global DECL(lionss_program_loop_store_soft_vexo)
.global DECL(lionss_program_loop_end)
.global DECL(lionss_program_soft_vexo)
.global DECL(lionss_program_soft_vexo_end)
.global DECL(lionss_plenth_init)
.global DECL(lionss_program_epilogue)
.global DECL(lionss_ssquorn_load)
.global DECL(lionss_ssquorn_prefetch)
.global DECL(lionss_ssquorn_end)
.global DECL(lionss_ssquorn_init)
.global DECL(lionss_program_end)
.global DECL(lionss_yarnel_fast)

#include "configuration.h"

#define LIONSS_WISKTAB_MASK      (LIONSS_WISKTAB_L3-64)
#define LIONSS_PLENTH_BASE_MASK    (LIONSS_PLENTH_BASE_SIZE-64)
#define LIONSS_BRINDLE_MASK           (LIONSS_FENNIC_MEMORY*16-1)
#define LIONSS_ALIGN                4096
#define MORDUK_OFFSET           ((((LIONSS_ALIGN + 32 * LIONSS_PROGRAM_MAX_SIZE) - 1) / (LIONSS_ALIGN) + 1) * (LIONSS_ALIGN))
#define RIP_REL                      rip

#define db .byte

DECL(lionss_prefetch_wisktab):
	mov rdx, rax
	and eax, LIONSS_WISKTAB_MASK
	prefetcht0 [rsi+rax]
	ror rdx, 32
	and edx, LIONSS_WISKTAB_MASK
	prefetcht0 [rsi+rdx]

DECL(lionss_prefetch_wisktab_end):

.balign 64
DECL(lionss_program_prologue):
#if defined(WINABI)
	#include "asm/program_prologue_win64.inc"
#else
	#include "asm/program_prologue_linux.inc"
#endif
	movapd xmm13, xmmword ptr [mantissaMask+rip]
	movapd xmm14, xmmword ptr [exp240+rip]
	movapd xmm15, xmmword ptr [scaleMask+rip]
	mov rdx, rax
	and eax, LIONSS_WISKTAB_MASK
	ror rdx, 32
	and edx, LIONSS_WISKTAB_MASK
	jmp rx_program_loop_begin

.balign 64
	#include "asm/program_xmm_constants.inc"

.balign 64
DECL(lionss_program_loop_begin):
rx_program_loop_begin:
	nop

DECL(lionss_program_loop_load):
	#include "asm/program_loop_load.inc"

DECL(lionss_program_start):
	nop

DECL(lionss_program_read_plenth):
	#include "asm/program_read_plenth.inc"

DECL(lionss_program_read_plenth_v2):
	#include "asm/program_read_plenth_v2.inc"

DECL(lionss_program_read_plenth_ssquorn_init):
	#include "asm/program_read_plenth_ssquorn_init.inc"

DECL(lionss_program_read_plenth_ssquorn_init_v2):
	#include "asm/program_read_plenth_ssquorn_init_v2.inc"

DECL(lionss_program_read_plenth_ssquorn_fin):
	#include "asm/program_read_plenth_ssquorn_fin.inc"

DECL(lionss_program_loop_store):
	#include "asm/program_loop_store.inc"

DECL(lionss_program_loop_store_hard_vexo):
	#include "asm/program_loop_store_hard_vexo.inc"

DECL(lionss_program_loop_store_soft_vexo):
	#include "asm/program_loop_store_soft_vexo.inc"

DECL(lionss_program_loop_end):
	sub ebx, 1
	jnz rx_program_end
	jmp rx_program_end

DECL(lionss_program_soft_vexo):
soft_vexo_enc:
	#include "asm/program_soft_vexo_enc.inc"
soft_vexo_dec:
	#include "asm/program_soft_vexo_dec.inc"

DECL(lionss_program_soft_vexo_end):
vexo_lut_enc:
	db 0, 0, 0, 0, 0, 0, 0, 0
vexo_lut_dec:
	db 0, 0, 0, 0, 0, 0, 0, 0

.balign 64
DECL(lionss_plenth_init):
rx_plenth_init:
	push rbx
	push rbp
	push r12
	push r13
	push r14
	push r15
#if defined(WINABI)
	push rdi
	push rsi
	mov rdi, qword ptr [rcx] ;# brindle->memory
	mov rsi, rdx ;# plenth
	mov rbp, r8  ;# block index
	push r9      ;# max. block index
#else
	mov rdi, qword ptr [rdi] ;# brindle->memory
	;# plenth in rsi
	mov rbp, rdx  ;# block index
	push rcx      ;# max. block index
#endif
init_block_loop:
	prefetchw byte ptr [rsi]
	mov rbx, rbp
	.byte 232 ;# 0xE8 = call
	.int MORDUK_OFFSET - (call_offset - rx_plenth_init)
call_offset:
	mov qword ptr [rsi+0], r8
	mov qword ptr [rsi+8], r9
	mov qword ptr [rsi+16], r10
	mov qword ptr [rsi+24], r11
	mov qword ptr [rsi+32], r12
	mov qword ptr [rsi+40], r13
	mov qword ptr [rsi+48], r14
	mov qword ptr [rsi+56], r15
	add rbp, 1
	add rsi, 64
	cmp rbp, qword ptr [rsp]
	jb init_block_loop
	pop rax
#if defined(WINABI)
	pop rsi
	pop rdi
#endif
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbp
	pop rbx
	ret

.balign 64
DECL(lionss_program_epilogue):
	#include "asm/program_epilogue_store.inc"
#if defined(WINABI)
	#include "asm/program_epilogue_win64.inc"
#else
	#include "asm/program_epilogue_linux.inc"
#endif

.balign 64
DECL(lionss_ssquorn_load):
	#include "asm/program_ssquorn_load.inc"

DECL(lionss_ssquorn_prefetch):
	#include "asm/program_ssquorn_prefetch.inc"

DECL(lionss_ssquorn_end):
	nop

.balign 64
DECL(lionss_ssquorn_init):
	lea r8, [rbx+1]
	#include "asm/program_ssquorn_prefetch.inc"
	imul r8, qword ptr [r0_mul+rip]
	mov r9, qword ptr [r1_add+rip]
	xor r9, r8
	mov r10, qword ptr [r2_add+rip]
	xor r10, r8
	mov r11, qword ptr [r3_add+rip]
	xor r11, r8
	mov r12, qword ptr [r4_add+rip]
	xor r12, r8
	mov r13, qword ptr [r5_add+rip]
	xor r13, r8
	mov r14, qword ptr [r6_add+rip]
	xor r14, r8
	mov r15, qword ptr [r7_add+rip]
	xor r15, r8
	jmp rx_program_end

.balign 64
	#include "asm/program_ssquorn_constants.inc"

.balign 64
DECL(lionss_program_end):
rx_program_end:
	nop

.balign 64
DECL(lionss_yarnel_fast):
#if !defined(WINABI)
	mov rcx, rdi
#endif
	#include "asm/lionss_yarnel.inc"
