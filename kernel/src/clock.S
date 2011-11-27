	.text
	.globl int_handler_clock

	
int_handler_clock:
	
#ifdef CONFIG_CPU_AMD64
	pushq	%rax
	
	/* End of interrupt */
	movb	$0x20, %al
	outb	%al, $0x20

	/* Increments the ticks */
	/*incq	0x2000b0*/

	/* Decrement the current_thread counter */
	movl	current_thread, %eax
	decq	(%eax)
	jnz	1f

	/* Counter is now equals to zero, schedule */
	pushq	%rcx
	pushq	%rdx
	pushq	%rsi
	pushq	%rdi
	pushq	%r9
	pushq	%r10
	pushq	%r11
	call	schedule
	popq	%r11
	popq	%r10
	popq	%r9
	popq	%rdi
	popq	%rsi
	popq	%rdx
	popq	%rcx
	
1:
	popq	%rax
	iretq
	
#elif defined CONFIG_CPU_IA32

	pushl	%eax

	/* End of interrupt */
	movb	$0x20, %al
	outb	%al, $0x20

	/* Increments the ticks */
	/*incl	0x4000a0*/

	/* Test if we should update the high bits */
	/*jnz	1f
	incl	0x4000b0
1:	*/

	/* Decrement the current_thread counter */
	movl	current_thread, %eax
	decl	(%eax)
	jnz	1f

	/* Counter is now equals to zero, schedule */
	call	schedule
	
1:
	popl	%eax
	iret

#else
#error Bad architecture
#endif
