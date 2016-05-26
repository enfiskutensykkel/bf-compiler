movb	(%rbp, %rdx)	,	%al
movq	%rbx		    ,	%rsp
popq    %rdi
popq	%rsi
popq	%rbp
popq	%rbx
retq
