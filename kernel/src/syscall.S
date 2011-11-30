	.text
	
#include <thread-bits.h>
#ifdef CONFIG_CPU_IA32

	.globl	syscall_0x80

syscall_0x80:

	cmpl	$0x02, %ebp
	je	_listen_on

	/* Unknown syscall */
	pushl	(%esp)
	pushl	%ebp
	pushl	$bad_sys_str
	call panic

_listen_on:
	pushl	%ebx
	pushl	%edx

	/* Modify the listener */
	shll	$2, %edx
	addl	$listeners, %edx
	movl	current_thread, %ebx
	movl	%ebx, (%edx)
	/* Set state to THREAD_RECEIVING */
	movl	current_thread, %eax
	movl	$3, 28(%eax)

	/* Switch to another thread */
	pushl	%eax
	pushl	%ecx
	call 	schedule
	popl	%ecx
	popl	%eax

	/* Put the received data in %eax */
	movl	24(%eax), %eax
	
	popl	%edx
	popl	%ebx
	
	iret

	
#elif defined CONFIG_CPU_AMD64
	
	.globl	syscall_handler
	
syscall_handler:
	/**
	 * Only the two first KB of the UTCB are used by L4.
	 * We use +2048-+3192 for syscall kernel stack, and data.
	 * TODO: Do not use %r15.
	 */
	movq	$UTCB_VIRTUAL_ADDRESS, %r15
	movq	%rsp, 8(%r15)
	movq	%rcx, 16(%r15)
	addq	$3192, %r15
	movq	%r15, %rsp

	/*pushfq*/
	
	cmpq	$0x02, %rbp
	je	_listen_on
	cmpq	$0x03, %rbp
	je	_ipc
	
	/* Unknown syscall */
	movq	$bad_sys_str, %rdi
	movq	%rbp, %rsi
	movq	(%rsp), %rdx
	call	panic

_listen_on:
	pushq	%rbx
	pushq	%rdx
	
	/* Modify the listener */
	shl	$3, %rdx
	addq	$listeners, %rdx
	movq	current_thread, %rbx
	movq	%rbx, (%rdx)
	/* Set state to THREAD_RECEIVING */
	movq	current_thread, %rax
	movl	$3, 56(%rax)
	
	/* Switch to another thread */
	pushq	%rax
	pushq	%rcx
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
	popq	%rcx
	popq	%rax
	
	/* Put the received data in %rax */
	movq	48(%rax), %rax

	popq	%rdx
	popq	%rbx

	jmp	_syscall_exit

_ipc:
	/*
	 * Input:
	 *     rsi	to
	 *     rdx	from_specifier
	 *     r8	timeout
	 *
	 * Output:
	 *     rsi	from
	 */

	/* Check if a send phase is included */
	cmpq	$NIL_THREAD, %rsi
	je	_ipc_recv
_ipc_send:
	/* Search for the destination */
	
_ipc_recv:
	/* Check if we receive something */
	cmpq	$NIL_THREAD, %rdx
	je	_syscall_exit
	/* Set state to THREAD_IPC_RCV */
	movq	current_thread, %rax
	movl	$5, 56(%rax)
	/* Switch to another thread */
	pushq	%rax
	pushq	%rcx
	pushq	%rdi
	pushq	%r9
	pushq	%r10
	pushq	%r11
	call	schedule
	popq	%r11
	popq	%r10
	popq	%r9
	popq	%rdi
	popq	%rcx
	popq	%rax
	/* Store sender id in %rsi */
	movq	92(%rax), %rsi

_syscall_exit:
	movq	$UTCB_VIRTUAL_ADDRESS, %r15
	movq	8(%r15), %rsp
	movq	16(%r15), %rcx
	/*popfq*/
	sysretq
#endif

	
	.data
	
bad_sys_str:	.asciz "Bad syscall: 0x%x @0x%lx"
