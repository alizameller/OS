	.text					# Declare we are in text region
	.globl _tas				# Declare tas to be a global symbol
_tas:						# Label marking start of tas function
	pushq	%rbp			# Save base pointer on stack
	movq	%rsp, %rbp		# Establish new base pointer
	movq	$1, %rax		# Prepare a constant 1
	lock/xchgb	%al, (%rdi)	# The actual atomic instruction, arg1 is in the rdi register
	movsbq	%al,%rax		# sign-extend result into rax
	pop		%rbp			# Restore base pointer
	ret						# Return rax contains the return value
.Lfe1:
	.set tas_size, .Lfe1 - _tas	# Declare the size of function tas