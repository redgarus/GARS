	.text
	.file	"test2.ll"
	.globl	print                           # -- Begin function print
	.p2align	4, 0x90
	.type	print,@function
print:                                  # @print
	.cfi_startproc
# %bb.0:                                # %entry
	pushq	%rax
	.cfi_def_cfa_offset 16
	movq	%rdi, %rsi
	movl	$.Lout, %edi
	xorl	%eax, %eax
	callq	printf@PLT
	xorl	%eax, %eax
	popq	%rcx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	print, .Lfunc_end0-print
	.cfi_endproc
                                        # -- End function
	.globl	main                            # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %entry
	subq	$56, %rsp
	.cfi_def_cfa_offset 64
	movq	$1, 8(%rsp)
	movq	$2, 32(%rsp)
	movq	$3, 56(%rsp)
	movq	16(%rsp), %rax
	movq	24(%rsp), %rcx
	movq	%rax, 40(%rsp)
	movq	%rcx, 48(%rsp)
	movq	$1, 32(%rsp)
	movl	$4, %edi
	callq	print@PLT
	xorl	%eax, %eax
	addq	$56, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
	.cfi_endproc
                                        # -- End function
	.type	.Lout,@object                   # @out
	.section	.rodata.str1.1,"aMS",@progbits,1
.Lout:
	.asciz	"Output: %i\n"
	.size	.Lout, 12

	.section	".note.GNU-stack","",@progbits
