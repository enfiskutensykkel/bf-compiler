movb	(%rbp, %rdx)	,	%al
decb	%al
movb	%al		, 	(%rbp, %rdx)
