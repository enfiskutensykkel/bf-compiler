movb	(%rbp, %rdx)	,	%al
cmpb	$0		,	%al
je	. + 0x1000
