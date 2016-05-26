movb	(%rbp, %rdx)	,	%al
movq	%rbx		,	%rsp
popq	%rbp
popq	%rbx
retq
