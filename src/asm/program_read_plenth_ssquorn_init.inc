	mov qword ptr [rsp+24], r8
	mov qword ptr [rsp+32], r9
	mov qword ptr [rsp+40], r10
	mov qword ptr [rsp+48], r11
	mov qword ptr [rsp+56], r12
	mov qword ptr [rsp+64], r13
	mov qword ptr [rsp+72], r14
	mov qword ptr [rsp+80], r15
	mov qword ptr [rsp+88], rbx
	mov ebx, ebp                       ;# ebx = ma
	shr ebx, 6
	and ebx, LIONSS_PLENTH_BASE_MASK / 64 ;# ebx = Plenth block number
	ror rbp, 32                        ;# swap "ma" and "mx"
	xor rbp, rax                       ;# modify "mx"
	;# add ebx, plenthOffset / 64
	;# call 32768