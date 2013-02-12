	.section	__TEXT,__text,regular,pure_instructions
	.globl	_sigintHandler
	.align	4, 0x90
_sigintHandler:
Leh_func_begin1:
	pushq	%rbp
Ltmp0:
	movq	%rsp, %rbp
Ltmp1:
	subq	$16, %rsp
Ltmp2:
	movl	$1, -4(%rbp)
	movl	-4(%rbp), %eax
	leaq	L_.str(%rip), %rcx
	movabsq	$12, %rdx
	movl	%eax, %edi
	movq	%rcx, %rsi
	callq	_write
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	cmpq	$12, %rax
	je	LBB1_2
	movl	$4294966297, %eax
	movl	%eax, %edi
	callq	_exit
LBB1_2:
	addq	$16, %rsp
	popq	%rbp
	ret
Leh_func_end1:

	.globl	_sigusr1Handler
	.align	4, 0x90
_sigusr1Handler:
Leh_func_begin2:
	pushq	%rbp
Ltmp3:
	movq	%rsp, %rbp
Ltmp4:
	subq	$16, %rsp
Ltmp5:
	movl	$1, -12(%rbp)
	movl	-12(%rbp), %eax
	leaq	L_.str1(%rip), %rcx
	movabsq	$9, %rdx
	movl	%eax, %edi
	movq	%rcx, %rsi
	callq	_write
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	cmpq	$9, %rax
	je	LBB2_2
	movl	$4294966297, %eax
	movl	%eax, %edi
	callq	_exit
LBB2_2:
	movl	$1, %eax
	movl	%eax, %edi
	callq	_exit
Leh_func_end2:

	.globl	_main
	.align	4, 0x90
_main:
Leh_func_begin3:
	pushq	%rbp
Ltmp6:
	movq	%rsp, %rbp
Ltmp7:
	subq	$80, %rsp
Ltmp8:
	movl	%edi, -4(%rbp)
	movq	%rsi, -16(%rbp)
	movq	$1, -40(%rbp)
	movq	$0, -32(%rbp)
	movl	$2, %eax
	leaq	_sigintHandler(%rip), %rcx
	movl	%eax, %edi
	movq	%rcx, %rsi
	callq	_signal
	movl	$30, %eax
	leaq	_sigusr1Handler(%rip), %rcx
	movl	%eax, %edi
	movq	%rcx, %rsi
	callq	_signal
	callq	_getpid
	movl	%eax, %ecx
	xorb	%dl, %dl
	leaq	L_.str2(%rip), %rdi
	movl	%ecx, %esi
	movb	%dl, %al
	callq	_printf
LBB3_1:
	leaq	-40(%rbp), %rax
	leaq	-56(%rbp), %rcx
	movq	%rax, %rdi
	movq	%rcx, %rsi
	callq	_nanosleep
	movl	%eax, %ecx
	cmpl	$-1, %ecx
	jne	LBB3_3
	movq	-56(%rbp), %rax
	movq	%rax, -40(%rbp)
	movq	-48(%rbp), %rax
	movq	%rax, -32(%rbp)
	jmp	LBB3_6
LBB3_3:
	movl	$1, -68(%rbp)
	movl	-68(%rbp), %eax
	leaq	L_.str3(%rip), %rcx
	movabsq	$14, %rdx
	movl	%eax, %edi
	movq	%rcx, %rsi
	callq	_write
	movq	%rax, -64(%rbp)
	movq	-64(%rbp), %rax
	cmpq	$14, %rax
	je	LBB3_5
	movl	$4294966297, %eax
	movl	%eax, %edi
	callq	_exit
LBB3_5:
	movq	$1, -40(%rbp)
	movq	$0, -32(%rbp)
LBB3_6:
	jmp	LBB3_1
Leh_func_end3:

	.section	__TEXT,__cstring,cstring_literals
L_.str:
	.asciz	 "\nNice try.\n"

L_.str1:
	.asciz	 "Exiting.\n"

L_.str2:
	.asciz	 "%d\n"

L_.str3:
	.asciz	 "Still here...\n"

	.section	__TEXT,__eh_frame,coalesced,no_toc+strip_static_syms+live_support
EH_frame0:
Lsection_eh_frame:
Leh_frame_common:
Lset0 = Leh_frame_common_end-Leh_frame_common_begin
	.long	Lset0
Leh_frame_common_begin:
	.long	0
	.byte	1
	.asciz	 "zR"
	.byte	1
	.byte	120
	.byte	16
	.byte	1
	.byte	16
	.byte	12
	.byte	7
	.byte	8
	.byte	144
	.byte	1
	.align	3
Leh_frame_common_end:
	.globl	_sigintHandler.eh
_sigintHandler.eh:
Lset1 = Leh_frame_end1-Leh_frame_begin1
	.long	Lset1
Leh_frame_begin1:
Lset2 = Leh_frame_begin1-Leh_frame_common
	.long	Lset2
Ltmp9:
	.quad	Leh_func_begin1-Ltmp9
Lset3 = Leh_func_end1-Leh_func_begin1
	.quad	Lset3
	.byte	0
	.byte	4
Lset4 = Ltmp0-Leh_func_begin1
	.long	Lset4
	.byte	14
	.byte	16
	.byte	134
	.byte	2
	.byte	4
Lset5 = Ltmp1-Ltmp0
	.long	Lset5
	.byte	13
	.byte	6
	.align	3
Leh_frame_end1:

	.globl	_sigusr1Handler.eh
_sigusr1Handler.eh:
Lset6 = Leh_frame_end2-Leh_frame_begin2
	.long	Lset6
Leh_frame_begin2:
Lset7 = Leh_frame_begin2-Leh_frame_common
	.long	Lset7
Ltmp10:
	.quad	Leh_func_begin2-Ltmp10
Lset8 = Leh_func_end2-Leh_func_begin2
	.quad	Lset8
	.byte	0
	.byte	4
Lset9 = Ltmp3-Leh_func_begin2
	.long	Lset9
	.byte	14
	.byte	16
	.byte	134
	.byte	2
	.byte	4
Lset10 = Ltmp4-Ltmp3
	.long	Lset10
	.byte	13
	.byte	6
	.align	3
Leh_frame_end2:

	.globl	_main.eh
_main.eh:
Lset11 = Leh_frame_end3-Leh_frame_begin3
	.long	Lset11
Leh_frame_begin3:
Lset12 = Leh_frame_begin3-Leh_frame_common
	.long	Lset12
Ltmp11:
	.quad	Leh_func_begin3-Ltmp11
Lset13 = Leh_func_end3-Leh_func_begin3
	.quad	Lset13
	.byte	0
	.byte	4
Lset14 = Ltmp6-Leh_func_begin3
	.long	Lset14
	.byte	14
	.byte	16
	.byte	134
	.byte	2
	.byte	4
Lset15 = Ltmp7-Ltmp6
	.long	Lset15
	.byte	13
	.byte	6
	.align	3
Leh_frame_end3:


.subsections_via_symbols
