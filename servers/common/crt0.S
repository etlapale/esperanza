#define DEFAULT_STACK_SIZE 0x1000

	.text
	.globl	_start, exit

	
/* Program entry point */
_start:
#ifdef CONFIG_CPU_IA32

	/* Setup the stack */
	movl	$stack, %esp
	
#else /* CONFIG_CPU_AMD64 */
	
	/* Setup the stack */
	movq	$stack, %rsp

#endif
	
	/* Call the main() function */
	call	main


/* Terminate the current thread */
exit:
#ifdef CONFIG_CPU_IA32
	xorl	%eax, %eax	/* MySelf */
	xorl	%esi, %esi	/* Deletion */
	int	$0x81
#elif defined CONFIG_CPU_AMD64
	movl	$0x01, %ebp	/* exit () */
	syscall
#endif

1:	jmp	1b		/* We should never be there */

	
	.data

	/* The stack */
	.align	32
	.space	DEFAULT_STACK_SIZE
stack:
