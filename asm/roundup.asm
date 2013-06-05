section	.data
	x:	dd	0	

section	.text

        global roundup
roundup:	
        push 	ebp	
        mov 	ebp, esp

	xor	eax, eax
        fstcw 	[ebp-16h]
        mov   	ax, word [ebp - 16h]
        xor    	ah, ah
	or	ah, 8
        mov	[ebp - 16h], ax
        fldcw  	[ebp - 16h]
	mov	eax, [ebp + 8]
	fld	qword [eax]
	fistp	dword [x]

        mov	esp, ebp	
        pop	ebp	
	
	xor	eax, eax
	mov	eax, [x]

	ret	
