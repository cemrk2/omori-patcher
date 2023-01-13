<$chowdren.2BEBFEF>
	@codecave:
		sub rsp, 0x28								; instruction overwritten by "call codecave"
		; nop
		; jmp 0x00007FF6FEB3BFF3					; infinite loop for debugging, no longer needed

		push rax									; push the values of the registers on to the stack
		push rbx
		push rcx
		push rdx
		push rsp

		push 0x00									; push two nullbytes to make sure nothing important gets overwritten
		push 0x00
		
		mov rcx, 0x11								; allocate 0x11 (17) bytes
		call qword ptr ds:[0x00000001432C3AB8]		; malloc(17);

		mov qword ptr ds:[rax], 0x6F				; 'o'
		mov qword ptr ds:[rax+0x1], 0x6D			; 'm'
		mov qword ptr ds:[rax+0x2], 0x6F			; 'o'
		mov qword ptr ds:[rax+0x3], 0x72			; 'r'
		mov qword ptr ds:[rax+0x4], 0x69			; 'i'
		mov qword ptr ds:[rax+0x5], 0x2D			; '-'
		mov qword ptr ds:[rax+0x6], 0x70			; 'p'
		mov qword ptr ds:[rax+0x7], 0x61			; 'a'
		mov qword ptr ds:[rax+0x8], 0x74			; 't'
		mov qword ptr ds:[rax+0x9], 0x63			; 'c'
		mov qword ptr ds:[rax+0xA], 0x68			; 'h'
		mov qword ptr ds:[rax+0xB], 0x65			; 'e'
		mov qword ptr ds:[rax+0xC], 0x72			; 'r'
		mov qword ptr ds:[rax+0xD], 0x2E			; '.'
		mov qword ptr ds:[rax+0xE], 0x64			; 'd'
		mov qword ptr ds:[rax+0xF], 0x6C			; 'l'
		mov qword ptr ds:[rax+0x10], 0x6C			; 'l'
		mov qword ptr ds:[rax+0x11], 0x00			; terminating nullbyte

		mov rcx, rax
		call qword ptr ds:[0x00000001432C2598]		; LoadLibraryA("omori-patcher.dll");

		pop rax										; pop the first two zeros from stack that we have added
		pop rax

		pop rdx										; restore the values of the registers from the stack
		pop rcx
		pop rbx
		pop rax

		pop rbx										; pop the 6 nulls from stack
		pop rbx
		pop rbx
		pop rbx
		pop rbx
		pop rbx
		ret