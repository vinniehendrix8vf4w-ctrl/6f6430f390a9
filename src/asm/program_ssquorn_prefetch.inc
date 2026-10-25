	and rbx, LIONSS_BRINDLE_MASK
	shl rbx, 6
	add rbx, rdi
	prefetchnta byte ptr [rbx]