#define STACK_SIZE 0x1000
#define HEAP_SIZE  0x1000

#define __ASM
#include <multiboot.h>


	.section	.text
	.globl	_start
_start:	

	/* Save the multiboot info, the multiboot magic and the heap address */
	movl	%eax, multiboot_magic
	movl	%ebx, multiboot_info

	/* Set up a stack so we can push */
	movl	$(stack + STACK_SIZE), %ebp
	movl	%ebp, %esp

	/* Go to the C entry */
	pushl	$HEAP_SIZE
	pushl	$heap
	call	main
	
	/* We should never be there but infinite loop just in case */
1:	jmp	1b


/*
 * The multiboot header. You can find the specs at:
 * http://www.gnu.org/software/grub/manual/multiboot/
 */

	/* Align the multiboot header on 4-bytes */
.align	4
	/* Magic number */
.long	MULTIBOOT_HEADER_MAGIC
	/* Flags */
.long	MULTIBOOT_HEADER_FLAGS
	/* Checksum is -(magic_number + flags) */
.long	- (MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)


	/* BSS uninitialized memory section for the stack and the heap */
	.bss
	.align	16
	.lcomm	stack, STACK_SIZE
	.lcomm	heap, HEAP_SIZE
