	.text
	.globl testaddfourC          
	.type  testaddfourC, @function

testaddfourC:
	pushl $2     # d
	pushl $2     # c
	pushl $3     # b
	pushl $2     # a

	call addfourC

	addl $16, %esp

	ret

