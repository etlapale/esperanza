define(`forloop',
        `pushdef(`$1', `$2')_forloop(`$1', `$2', `$3', `$4')popdef(`$1')')
define(`_forloop',
        `$4`'ifelse($1, `$3', ,
            `define(`$1', incr($1))_forloop(`$1', `$2', `$3', `$4')')')

define(`NUM_HANDLER',
`	.globl int_num_handler_$1
int_num_handler_$1:
	movq	$num_handler_str, %rdi
	movl	$$1, %esi
	callq	panic
')

	.text

forloop(`i', 0, 255, `NUM_HANDLER('`i'`)
')

	.data
num_handler_str:
	.asciz "Unhandled [0x%lx] interrupt"
