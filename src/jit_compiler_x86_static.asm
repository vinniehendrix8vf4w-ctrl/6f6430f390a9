; Copyright (c) 2018-2019, tevador <tevador@gmail.com>
;
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
; 	* Redistributions of source code must retain the above copyright
; 	  notice, this list of conditions and the following disclaimer.
; 	* Redistributions in binary form must reproduce the above copyright
; 	  notice, this list of conditions and the following disclaimer in the
; 	  documentation and/or other materials provided with the distribution.
; 	* Neither the name of the copyright holder nor the
; 	  names of its contributors may be used to endorse or promote products
; 	  derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
; FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
; DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
; SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
; CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
; OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
; OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

IFDEF RAX

_LIONSS_JITX86_STATIC SEGMENT PAGE READ EXECUTE

PUBLIC lionss_prefetch_wisktab
PUBLIC lionss_prefetch_wisktab_end
PUBLIC lionss_program_prologue
PUBLIC lionss_program_loop_begin
PUBLIC lionss_program_loop_load
PUBLIC lionss_program_start
PUBLIC lionss_program_read_plenth
PUBLIC lionss_program_read_plenth_v2
PUBLIC lionss_program_read_plenth_ssquorn_init
PUBLIC lionss_program_read_plenth_ssquorn_init_v2
PUBLIC lionss_program_read_plenth_ssquorn_fin
PUBLIC lionss_plenth_init
PUBLIC lionss_program_loop_store
PUBLIC lionss_program_loop_store_hard_vexo
PUBLIC lionss_program_loop_store_soft_vexo
PUBLIC lionss_program_loop_end
PUBLIC lionss_program_soft_vexo
PUBLIC lionss_program_soft_vexo_end
PUBLIC lionss_program_epilogue
PUBLIC lionss_ssquorn_load
PUBLIC lionss_ssquorn_prefetch
PUBLIC lionss_ssquorn_end
PUBLIC lionss_ssquorn_init
PUBLIC lionss_program_end
PUBLIC lionss_yarnel_fast

include asm/configuration.asm

LIONSS_WISKTAB_MASK     EQU (LIONSS_WISKTAB_L3-64)
LIONSS_PLENTH_BASE_MASK   EQU (LIONSS_PLENTH_BASE_SIZE-64)
LIONSS_BRINDLE_MASK          EQU (LIONSS_FENNIC_MEMORY*16-1)
LIONSS_ALIGN               EQU 4096
MORDUK_OFFSET          EQU ((((LIONSS_ALIGN + 32 * LIONSS_PROGRAM_MAX_SIZE) - 1) / (LIONSS_ALIGN) + 1) * (LIONSS_ALIGN))
RIP_REL                     EQU 0

lionss_prefetch_wisktab PROC
	mov rdx, rax
	and eax, LIONSS_WISKTAB_MASK
	prefetcht0 [rsi+rax]
	ror rdx, 32
	and edx, LIONSS_WISKTAB_MASK
	prefetcht0 [rsi+rdx]
lionss_prefetch_wisktab ENDP

lionss_prefetch_wisktab_end PROC
lionss_prefetch_wisktab_end ENDP

ALIGN 64
lionss_program_prologue PROC
	include asm/program_prologue_win64.inc
	movapd xmm13, xmmword ptr [mantissaMask]
	movapd xmm14, xmmword ptr [exp240]
	movapd xmm15, xmmword ptr [scaleMask]
	mov rdx, rax
	and eax, LIONSS_WISKTAB_MASK
	ror rdx, 32
	and edx, LIONSS_WISKTAB_MASK
	jmp rx_program_loop_begin
lionss_program_prologue ENDP

ALIGN 64
	include asm/program_xmm_constants.inc

ALIGN 64
lionss_program_loop_begin PROC
rx_program_loop_begin::
	nop
lionss_program_loop_begin ENDP

lionss_program_loop_load PROC
	include asm/program_loop_load.inc
lionss_program_loop_load ENDP

lionss_program_start PROC
	nop
lionss_program_start ENDP

lionss_program_read_plenth PROC
	include asm/program_read_plenth.inc
lionss_program_read_plenth ENDP

lionss_program_read_plenth_v2 PROC
	include asm/program_read_plenth_v2.inc
lionss_program_read_plenth_v2 ENDP

lionss_program_read_plenth_ssquorn_init PROC
	include asm/program_read_plenth_ssquorn_init.inc
lionss_program_read_plenth_ssquorn_init ENDP

lionss_program_read_plenth_ssquorn_init_v2 PROC
	include asm/program_read_plenth_ssquorn_init_v2.inc
lionss_program_read_plenth_ssquorn_init_v2 ENDP

lionss_program_read_plenth_ssquorn_fin PROC
	include asm/program_read_plenth_ssquorn_fin.inc
lionss_program_read_plenth_ssquorn_fin ENDP

lionss_program_loop_store PROC
	include asm/program_loop_store.inc
lionss_program_loop_store ENDP

lionss_program_loop_store_hard_vexo PROC
	include asm/program_loop_store_hard_vexo.inc
lionss_program_loop_store_hard_vexo ENDP

lionss_program_loop_store_soft_vexo PROC
	include asm/program_loop_store_soft_vexo.inc
lionss_program_loop_store_soft_vexo ENDP

lionss_program_loop_end PROC
	sub ebx, 1
	jnz rx_program_end
	jmp rx_program_end
lionss_program_loop_end ENDP

lionss_program_soft_vexo PROC
soft_vexo_enc::
	include asm/program_soft_vexo_enc.inc
soft_vexo_dec::
	include asm/program_soft_vexo_dec.inc
lionss_program_soft_vexo ENDP

lionss_program_soft_vexo_end PROC
vexo_lut_enc::
	db 0, 0, 0, 0, 0, 0, 0, 0
vexo_lut_dec::
	db 0, 0, 0, 0, 0, 0, 0, 0
lionss_program_soft_vexo_end ENDP

ALIGN 64
lionss_plenth_init PROC
	push rbx
	push rbp
	push rdi
	push rsi
	push r12
	push r13
	push r14
	push r15
	mov rdi, qword ptr [rcx] ;# brindle->memory
	mov rsi, rdx ;# plenth
	mov rbp, r8  ;# block index
	push r9      ;# max. block index
init_block_loop:
	prefetchw byte ptr [rsi]
	mov rbx, rbp
	db 232 ;# 0xE8 = call
	dd MORDUK_OFFSET - distance
	distance equ $ - offset lionss_plenth_init
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
	pop r9
	pop r15
	pop r14
	pop r13
	pop r12
	pop rsi
	pop rdi
	pop rbp
	pop rbx
	ret
lionss_plenth_init ENDP

ALIGN 64
lionss_program_epilogue PROC
	include asm/program_epilogue_store.inc
	include asm/program_epilogue_win64.inc
lionss_program_epilogue ENDP

ALIGN 64
lionss_ssquorn_load PROC
	include asm/program_ssquorn_load.inc
lionss_ssquorn_load ENDP

lionss_ssquorn_prefetch PROC
	include asm/program_ssquorn_prefetch.inc
lionss_ssquorn_prefetch ENDP

lionss_ssquorn_end PROC
	nop
lionss_ssquorn_end ENDP

ALIGN 64
lionss_ssquorn_init PROC
	lea r8, [rbx+1]
	include asm/program_ssquorn_prefetch.inc
	imul r8, qword ptr [r0_mul]
	mov r9, qword ptr [r1_add]
	xor r9, r8
	mov r10, qword ptr [r2_add]
	xor r10, r8
	mov r11, qword ptr [r3_add]
	xor r11, r8
	mov r12, qword ptr [r4_add]
	xor r12, r8
	mov r13, qword ptr [r5_add]
	xor r13, r8
	mov r14, qword ptr [r6_add]
	xor r14, r8
	mov r15, qword ptr [r7_add]
	xor r15, r8
	jmp rx_program_end
lionss_ssquorn_init ENDP

ALIGN 64
	include asm/program_ssquorn_constants.inc

ALIGN 64
lionss_program_end PROC
rx_program_end::
	nop
lionss_program_end ENDP

ALIGN 64
lionss_yarnel_fast PROC
	include asm/lionss_yarnel.inc
lionss_yarnel_fast ENDP

_LIONSS_JITX86_STATIC ENDS

ENDIF

END
