.text
    .global _start
_start: 
    movl $4, %eax
    movl $1, %ebx
    movl $string, %ecx 
    movl $4, %edx
    int $0x80
    # exit
    movl $1, %eax
    movl $42, %ebx
    int $0x80
# More code below here, not shown
string: .string "123\n"
