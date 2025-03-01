/* Simple code to add four integers passed to addfour() coded
	in AT&T assembly. Assumes caller follows CDECL. */


    .text
    .globl addfour
    .type addfour, @function

addfour:
    # Prologue: standard
    pushl   %ebp            # Save old EBP
    movl    %esp, %ebp      # Set new EBP

    # Save EBX because we will modify it (callee-saved register)
    pushl   %ebx

    # Retrieve the 4 arguments
    movl    8(%ebp),  %eax  # 1st arg
    movl    12(%ebp), %ebx  # 2nd arg
    movl    16(%ebp), %ecx  # 3rd arg
    movl    20(%ebp), %edx  # 4th arg

    # EAX = arg1 + arg2 + arg3 + arg4
    addl    %ebx, %eax
    addl    %ecx, %eax
    addl    %edx, %eax

    # Restore EBX
    popl    %ebx

    # Epilogue: standard
    popl    %ebp
    ret

