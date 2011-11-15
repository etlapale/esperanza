#define KEYBOARD_PORT 0x21
	
	.text
	
	.globl int_handler_keyboard

int_handler_keyboard:

#ifdef CONFIG_CPU_IA32
	/* Disable the interrupts */
	cli

	/* Save the scratch register */
	push	%eax

	/* End of interrupt */
	movb	$0x20, %al
	outb	%al, $0x20

	/* Read the pressed key makecode */
	xorl	%eax, %eax
	inb	$0x60, %al

	/* Save the scratch registers */
	pushl	%ecx
	push	%edx

	/* Call send(msg,port) */
	pushl	%eax
	pushl	$KEYBOARD_PORT
	call	send
	add	$8, %esp

	/* Restore the scratch registers */
	popl	%edx
	popl	%ecx

	/* Restore %eax */
	popl	%eax

	/* Return */
	iret
#elif defined CONFIG_CPU_AMD64
	cli
	
	pushq	%rax
	pushq	%rdx
	pushq	%rsi
	pushq	%rdi

	movb	$0x20, %al
	outb	%al, $0x20

	xorl	%esi, %esi
	xorl	%eax, %eax
	inb	$0x60, %al
	movl	%eax, %esi
	movq	$KEYBOARD_PORT, %rdi

	
	pushq	%rcx
	pushq	%r9
	pushq	%r10
	pushq	%r11
	call	send
	popq	%r11
	popq	%r10
	popq	%r9
	popq	%rcx

	popq	%rdi
	popq	%rsi
	popq	%rdx
	popq	%rax

	iretq
#endif
