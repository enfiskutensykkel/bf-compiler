movq	$0x2000004	,	%rax
movq	$1		,	%rdi
leaq	(%rbp,	%rdx)	,	%rsi
pushq	%rbp
pushq	%rdx
movq	$1		,	%rdx
syscall
popq	%rdx
popq	%rbp

