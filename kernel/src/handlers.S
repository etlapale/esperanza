	/* handlers.s - Assembly interrupt handlers */
	
	.text

	.globl int_handler_gp, int_handler_page_fault, \
		int_handler_not_present, int_handler_stack, \
		int_handler_opcode, int_handler_default, \
		int_handler_tss, int_handler_double, int_handler_zero


	/* Default interrupt handler */
int_handler_default:
#ifdef CONFIG_CPU_IA32

	pushl	$default_str
	
#elif defined CONFIG_CPU_AMD64

	movq	$default_str, %rdi
	movq	(%esp), %rsi
	movq	8(%esp), %rdx
	movq	16(%esp), %rcx
	movq	24(%esp), %r8
#endif
	call panic

	/* Segment not present */
int_handler_not_present:
#ifdef CONFIG_CPU_IA32

	pushl	$not_present_str
	
#elif defined CONFIG_CPU_AMD64

	movq	$not_present_str, %rdi
	movq	(%esp), %rsi
	movq	8(%esp), %rdx
	movq	16(%esp), %rcx
	movq	24(%esp), %r8
#endif
	call panic

	/* Double fault */
int_handler_double:
#ifdef CONFIG_CPU_IA32

	pushl	$double_str
	
#elif defined CONFIG_CPU_AMD64

	movq	$double_str, %rdi
	movq	(%esp), %rsi
	movq	8(%esp), %rdx
	movq	16(%esp), %rcx
	movq	24(%esp), %r8
#endif
	call panic

	/* Invalid TSS */
int_handler_tss:
#ifdef CONFIG_CPU_IA32

	pushl	$tss_str
	
#elif defined CONFIG_CPU_AMD64

	movq	$tss_str, %rdi
	movq	(%esp), %rsi
	movq	8(%esp), %rdx
	movq	16(%esp), %rcx
	movq	24(%esp), %r8
#endif
	call panic

	/* Stack exception */
int_handler_stack:
#ifdef CONFIG_CPU_IA32

	pushl	$stack_str
	
#elif defined CONFIG_CPU_AMD64

	movq	$stack_str, %rdi
	movq	(%esp), %rsi
	movq	8(%esp), %rdx
	movq	16(%esp), %rcx
	movq	24(%esp), %r8
#endif
	call panic

	/* Divide by zero */
int_handler_zero:
#ifdef CONFIG_CPU_IA32

	pushl	$zero_str
	
#elif defined CONFIG_CPU_AMD64

	movq	$zero_str, %rdi
	movq	(%esp), %rsi
	movq	8(%esp), %rdx
	movq	16(%esp), %rcx
	movq	24(%esp), %r8
#endif
	call panic
	

	/* Throwed on error due to a page access.
	 * Maye either indicate that the page is not present, that the
	 * access specified is forbidden, ...
	 */
int_handler_page_fault:
#ifdef CONFIG_CPU_IA32
	movl	%cr2, %eax
	pushl	%eax
	pushl	4(%esp)
	pushl	$page_fault_str
#else /* CONFIG_CPU_AMD64 */
	movq	$page_fault_str, %rdi
	movq	(%rsp), %rsi
	movq	%cr2, %rdx
#endif
	
	call	panic

	/*
	 * General protection fault
	 */
int_handler_gp:
#ifdef CONFIG_CPU_IA32
	pushl	$gp_fault_str
	call	panic
#else /* CONFIG_CPU_AMD64 */
	movq	$gp_fault_str, %rdi
	movq	(%rsp), %rsi
	movq	8(%rsp), %rdx
#endif
	call	panic

	/* Data section */
	.data

default_str:
	.asciz "Unhandled interrupt (Stack: %p %p %p %p)"
zero_str:
	.asciz "Divide by zero (Stack: %p %p %p %p)"
stack_str:
	.asciz "Stack exception (Stack: %p %p %p %p)"
tss_str:
	.asciz "Invalid TSS (Stack: %p %p %p %p)"
not_present_str:
	.asciz "Segment not present (Stack: %p %p %p %p)"
double_str:
	.asciz "Double fault (Stack: %p %p %p %p)"
invalid_op_str:
	.asciz "Invalid opcode at 0x%lx"
page_fault_str:
	.asciz "Page fault 0x%x on 0x%lx"
gp_fault_str:
	.asciz "General protection fault 0x%x at 0x%lx"
