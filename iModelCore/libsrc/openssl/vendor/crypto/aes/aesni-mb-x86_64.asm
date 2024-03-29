OPTION	DOTNAME
.text$	SEGMENT ALIGN(256) 'CODE'

EXTERN	OPENSSL_ia32cap_P:NEAR

PUBLIC	aesni_multi_cbc_encrypt

ALIGN	32
aesni_multi_cbc_encrypt	PROC PUBLIC
	mov	QWORD PTR[8+rsp],rdi	;WIN64 prologue
	mov	QWORD PTR[16+rsp],rsi
	mov	rax,rsp
$L$SEH_begin_aesni_multi_cbc_encrypt::
	mov	rdi,rcx
	mov	rsi,rdx
	mov	rdx,r8



	cmp	edx,2
	jb	$L$enc_non_avx
	mov	ecx,DWORD PTR[((OPENSSL_ia32cap_P+4))]
	test	ecx,268435456
	jnz	_avx_cbc_enc_shortcut
	jmp	$L$enc_non_avx
ALIGN	16
$L$enc_non_avx::
	mov	rax,rsp

	push	rbx

	push	rbp

	push	r12

	push	r13

	push	r14

	push	r15

	lea	rsp,QWORD PTR[((-168))+rsp]
	movaps	XMMWORD PTR[rsp],xmm6
	movaps	XMMWORD PTR[16+rsp],xmm7
	movaps	XMMWORD PTR[32+rsp],xmm8
	movaps	XMMWORD PTR[48+rsp],xmm9
	movaps	XMMWORD PTR[64+rsp],xmm10
	movaps	XMMWORD PTR[80+rsp],xmm11
	movaps	XMMWORD PTR[96+rsp],xmm12
	movaps	XMMWORD PTR[(-104)+rax],xmm13
	movaps	XMMWORD PTR[(-88)+rax],xmm14
	movaps	XMMWORD PTR[(-72)+rax],xmm15






	sub	rsp,48
	and	rsp,-64
	mov	QWORD PTR[16+rsp],rax


$L$enc4x_body::
	movdqu	xmm12,XMMWORD PTR[rsi]
	lea	rsi,QWORD PTR[120+rsi]
	lea	rdi,QWORD PTR[80+rdi]

$L$enc4x_loop_grande::
	mov	DWORD PTR[24+rsp],edx
	xor	edx,edx

	mov	ecx,DWORD PTR[((-64))+rdi]
	mov	r8,QWORD PTR[((-80))+rdi]
	cmp	ecx,edx
	mov	r12,QWORD PTR[((-72))+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	movdqu	xmm2,XMMWORD PTR[((-56))+rdi]
	mov	DWORD PTR[32+rsp],ecx
	cmovle	r8,rsp

	mov	ecx,DWORD PTR[((-24))+rdi]
	mov	r9,QWORD PTR[((-40))+rdi]
	cmp	ecx,edx
	mov	r13,QWORD PTR[((-32))+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	movdqu	xmm3,XMMWORD PTR[((-16))+rdi]
	mov	DWORD PTR[36+rsp],ecx
	cmovle	r9,rsp

	mov	ecx,DWORD PTR[16+rdi]
	mov	r10,QWORD PTR[rdi]
	cmp	ecx,edx
	mov	r14,QWORD PTR[8+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	movdqu	xmm4,XMMWORD PTR[24+rdi]
	mov	DWORD PTR[40+rsp],ecx
	cmovle	r10,rsp

	mov	ecx,DWORD PTR[56+rdi]
	mov	r11,QWORD PTR[40+rdi]
	cmp	ecx,edx
	mov	r15,QWORD PTR[48+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	movdqu	xmm5,XMMWORD PTR[64+rdi]
	mov	DWORD PTR[44+rsp],ecx
	cmovle	r11,rsp
	test	edx,edx
	jz	$L$enc4x_done

	movups	xmm1,XMMWORD PTR[((16-120))+rsi]
	pxor	xmm2,xmm12
	movups	xmm0,XMMWORD PTR[((32-120))+rsi]
	pxor	xmm3,xmm12
	mov	eax,DWORD PTR[((240-120))+rsi]
	pxor	xmm4,xmm12
	movdqu	xmm6,XMMWORD PTR[r8]
	pxor	xmm5,xmm12
	movdqu	xmm7,XMMWORD PTR[r9]
	pxor	xmm2,xmm6
	movdqu	xmm8,XMMWORD PTR[r10]
	pxor	xmm3,xmm7
	movdqu	xmm9,XMMWORD PTR[r11]
	pxor	xmm4,xmm8
	pxor	xmm5,xmm9
	movdqa	xmm10,XMMWORD PTR[32+rsp]
	xor	rbx,rbx
	jmp	$L$oop_enc4x

ALIGN	32
$L$oop_enc4x::
	add	rbx,16
	lea	rbp,QWORD PTR[16+rsp]
	mov	ecx,1
	sub	rbp,rbx

DB	102,15,56,220,209
	prefetcht0	[31+rbx*1+r8]
	prefetcht0	[31+rbx*1+r9]
DB	102,15,56,220,217
	prefetcht0	[31+rbx*1+r10]
	prefetcht0	[31+rbx*1+r10]
DB	102,15,56,220,225
DB	102,15,56,220,233
	movups	xmm1,XMMWORD PTR[((48-120))+rsi]
	cmp	ecx,DWORD PTR[32+rsp]
DB	102,15,56,220,208
DB	102,15,56,220,216
DB	102,15,56,220,224
	cmovge	r8,rbp
	cmovg	r12,rbp
DB	102,15,56,220,232
	movups	xmm0,XMMWORD PTR[((-56))+rsi]
	cmp	ecx,DWORD PTR[36+rsp]
DB	102,15,56,220,209
DB	102,15,56,220,217
DB	102,15,56,220,225
	cmovge	r9,rbp
	cmovg	r13,rbp
DB	102,15,56,220,233
	movups	xmm1,XMMWORD PTR[((-40))+rsi]
	cmp	ecx,DWORD PTR[40+rsp]
DB	102,15,56,220,208
DB	102,15,56,220,216
DB	102,15,56,220,224
	cmovge	r10,rbp
	cmovg	r14,rbp
DB	102,15,56,220,232
	movups	xmm0,XMMWORD PTR[((-24))+rsi]
	cmp	ecx,DWORD PTR[44+rsp]
DB	102,15,56,220,209
DB	102,15,56,220,217
DB	102,15,56,220,225
	cmovge	r11,rbp
	cmovg	r15,rbp
DB	102,15,56,220,233
	movups	xmm1,XMMWORD PTR[((-8))+rsi]
	movdqa	xmm11,xmm10
DB	102,15,56,220,208
	prefetcht0	[15+rbx*1+r12]
	prefetcht0	[15+rbx*1+r13]
DB	102,15,56,220,216
	prefetcht0	[15+rbx*1+r14]
	prefetcht0	[15+rbx*1+r15]
DB	102,15,56,220,224
DB	102,15,56,220,232
	movups	xmm0,XMMWORD PTR[((128-120))+rsi]
	pxor	xmm12,xmm12

DB	102,15,56,220,209
	pcmpgtd	xmm11,xmm12
	movdqu	xmm12,XMMWORD PTR[((-120))+rsi]
DB	102,15,56,220,217
	paddd	xmm10,xmm11
	movdqa	XMMWORD PTR[32+rsp],xmm10
DB	102,15,56,220,225
DB	102,15,56,220,233
	movups	xmm1,XMMWORD PTR[((144-120))+rsi]

	cmp	eax,11

DB	102,15,56,220,208
DB	102,15,56,220,216
DB	102,15,56,220,224
DB	102,15,56,220,232
	movups	xmm0,XMMWORD PTR[((160-120))+rsi]

	jb	$L$enc4x_tail

DB	102,15,56,220,209
DB	102,15,56,220,217
DB	102,15,56,220,225
DB	102,15,56,220,233
	movups	xmm1,XMMWORD PTR[((176-120))+rsi]

DB	102,15,56,220,208
DB	102,15,56,220,216
DB	102,15,56,220,224
DB	102,15,56,220,232
	movups	xmm0,XMMWORD PTR[((192-120))+rsi]

	je	$L$enc4x_tail

DB	102,15,56,220,209
DB	102,15,56,220,217
DB	102,15,56,220,225
DB	102,15,56,220,233
	movups	xmm1,XMMWORD PTR[((208-120))+rsi]

DB	102,15,56,220,208
DB	102,15,56,220,216
DB	102,15,56,220,224
DB	102,15,56,220,232
	movups	xmm0,XMMWORD PTR[((224-120))+rsi]
	jmp	$L$enc4x_tail

ALIGN	32
$L$enc4x_tail::
DB	102,15,56,220,209
DB	102,15,56,220,217
DB	102,15,56,220,225
DB	102,15,56,220,233
	movdqu	xmm6,XMMWORD PTR[rbx*1+r8]
	movdqu	xmm1,XMMWORD PTR[((16-120))+rsi]

DB	102,15,56,221,208
	movdqu	xmm7,XMMWORD PTR[rbx*1+r9]
	pxor	xmm6,xmm12
DB	102,15,56,221,216
	movdqu	xmm8,XMMWORD PTR[rbx*1+r10]
	pxor	xmm7,xmm12
DB	102,15,56,221,224
	movdqu	xmm9,XMMWORD PTR[rbx*1+r11]
	pxor	xmm8,xmm12
DB	102,15,56,221,232
	movdqu	xmm0,XMMWORD PTR[((32-120))+rsi]
	pxor	xmm9,xmm12

	movups	XMMWORD PTR[(-16)+rbx*1+r12],xmm2
	pxor	xmm2,xmm6
	movups	XMMWORD PTR[(-16)+rbx*1+r13],xmm3
	pxor	xmm3,xmm7
	movups	XMMWORD PTR[(-16)+rbx*1+r14],xmm4
	pxor	xmm4,xmm8
	movups	XMMWORD PTR[(-16)+rbx*1+r15],xmm5
	pxor	xmm5,xmm9

	dec	edx
	jnz	$L$oop_enc4x

	mov	rax,QWORD PTR[16+rsp]

	mov	edx,DWORD PTR[24+rsp]











	lea	rdi,QWORD PTR[160+rdi]
	dec	edx
	jnz	$L$enc4x_loop_grande

$L$enc4x_done::
	movaps	xmm6,XMMWORD PTR[((-216))+rax]
	movaps	xmm7,XMMWORD PTR[((-200))+rax]
	movaps	xmm8,XMMWORD PTR[((-184))+rax]
	movaps	xmm9,XMMWORD PTR[((-168))+rax]
	movaps	xmm10,XMMWORD PTR[((-152))+rax]
	movaps	xmm11,XMMWORD PTR[((-136))+rax]
	movaps	xmm12,XMMWORD PTR[((-120))+rax]



	mov	r15,QWORD PTR[((-48))+rax]

	mov	r14,QWORD PTR[((-40))+rax]

	mov	r13,QWORD PTR[((-32))+rax]

	mov	r12,QWORD PTR[((-24))+rax]

	mov	rbp,QWORD PTR[((-16))+rax]

	mov	rbx,QWORD PTR[((-8))+rax]

	lea	rsp,QWORD PTR[rax]

$L$enc4x_epilogue::
	mov	rdi,QWORD PTR[8+rsp]	;WIN64 epilogue
	mov	rsi,QWORD PTR[16+rsp]
	DB	0F3h,0C3h		;repret

$L$SEH_end_aesni_multi_cbc_encrypt::
aesni_multi_cbc_encrypt	ENDP

PUBLIC	aesni_multi_cbc_decrypt

ALIGN	32
aesni_multi_cbc_decrypt	PROC PUBLIC
	mov	QWORD PTR[8+rsp],rdi	;WIN64 prologue
	mov	QWORD PTR[16+rsp],rsi
	mov	rax,rsp
$L$SEH_begin_aesni_multi_cbc_decrypt::
	mov	rdi,rcx
	mov	rsi,rdx
	mov	rdx,r8



	cmp	edx,2
	jb	$L$dec_non_avx
	mov	ecx,DWORD PTR[((OPENSSL_ia32cap_P+4))]
	test	ecx,268435456
	jnz	_avx_cbc_dec_shortcut
	jmp	$L$dec_non_avx
ALIGN	16
$L$dec_non_avx::
	mov	rax,rsp

	push	rbx

	push	rbp

	push	r12

	push	r13

	push	r14

	push	r15

	lea	rsp,QWORD PTR[((-168))+rsp]
	movaps	XMMWORD PTR[rsp],xmm6
	movaps	XMMWORD PTR[16+rsp],xmm7
	movaps	XMMWORD PTR[32+rsp],xmm8
	movaps	XMMWORD PTR[48+rsp],xmm9
	movaps	XMMWORD PTR[64+rsp],xmm10
	movaps	XMMWORD PTR[80+rsp],xmm11
	movaps	XMMWORD PTR[96+rsp],xmm12
	movaps	XMMWORD PTR[(-104)+rax],xmm13
	movaps	XMMWORD PTR[(-88)+rax],xmm14
	movaps	XMMWORD PTR[(-72)+rax],xmm15






	sub	rsp,48
	and	rsp,-64
	mov	QWORD PTR[16+rsp],rax


$L$dec4x_body::
	movdqu	xmm12,XMMWORD PTR[rsi]
	lea	rsi,QWORD PTR[120+rsi]
	lea	rdi,QWORD PTR[80+rdi]

$L$dec4x_loop_grande::
	mov	DWORD PTR[24+rsp],edx
	xor	edx,edx

	mov	ecx,DWORD PTR[((-64))+rdi]
	mov	r8,QWORD PTR[((-80))+rdi]
	cmp	ecx,edx
	mov	r12,QWORD PTR[((-72))+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	movdqu	xmm6,XMMWORD PTR[((-56))+rdi]
	mov	DWORD PTR[32+rsp],ecx
	cmovle	r8,rsp

	mov	ecx,DWORD PTR[((-24))+rdi]
	mov	r9,QWORD PTR[((-40))+rdi]
	cmp	ecx,edx
	mov	r13,QWORD PTR[((-32))+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	movdqu	xmm7,XMMWORD PTR[((-16))+rdi]
	mov	DWORD PTR[36+rsp],ecx
	cmovle	r9,rsp

	mov	ecx,DWORD PTR[16+rdi]
	mov	r10,QWORD PTR[rdi]
	cmp	ecx,edx
	mov	r14,QWORD PTR[8+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	movdqu	xmm8,XMMWORD PTR[24+rdi]
	mov	DWORD PTR[40+rsp],ecx
	cmovle	r10,rsp

	mov	ecx,DWORD PTR[56+rdi]
	mov	r11,QWORD PTR[40+rdi]
	cmp	ecx,edx
	mov	r15,QWORD PTR[48+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	movdqu	xmm9,XMMWORD PTR[64+rdi]
	mov	DWORD PTR[44+rsp],ecx
	cmovle	r11,rsp
	test	edx,edx
	jz	$L$dec4x_done

	movups	xmm1,XMMWORD PTR[((16-120))+rsi]
	movups	xmm0,XMMWORD PTR[((32-120))+rsi]
	mov	eax,DWORD PTR[((240-120))+rsi]
	movdqu	xmm2,XMMWORD PTR[r8]
	movdqu	xmm3,XMMWORD PTR[r9]
	pxor	xmm2,xmm12
	movdqu	xmm4,XMMWORD PTR[r10]
	pxor	xmm3,xmm12
	movdqu	xmm5,XMMWORD PTR[r11]
	pxor	xmm4,xmm12
	pxor	xmm5,xmm12
	movdqa	xmm10,XMMWORD PTR[32+rsp]
	xor	rbx,rbx
	jmp	$L$oop_dec4x

ALIGN	32
$L$oop_dec4x::
	add	rbx,16
	lea	rbp,QWORD PTR[16+rsp]
	mov	ecx,1
	sub	rbp,rbx

DB	102,15,56,222,209
	prefetcht0	[31+rbx*1+r8]
	prefetcht0	[31+rbx*1+r9]
DB	102,15,56,222,217
	prefetcht0	[31+rbx*1+r10]
	prefetcht0	[31+rbx*1+r11]
DB	102,15,56,222,225
DB	102,15,56,222,233
	movups	xmm1,XMMWORD PTR[((48-120))+rsi]
	cmp	ecx,DWORD PTR[32+rsp]
DB	102,15,56,222,208
DB	102,15,56,222,216
DB	102,15,56,222,224
	cmovge	r8,rbp
	cmovg	r12,rbp
DB	102,15,56,222,232
	movups	xmm0,XMMWORD PTR[((-56))+rsi]
	cmp	ecx,DWORD PTR[36+rsp]
DB	102,15,56,222,209
DB	102,15,56,222,217
DB	102,15,56,222,225
	cmovge	r9,rbp
	cmovg	r13,rbp
DB	102,15,56,222,233
	movups	xmm1,XMMWORD PTR[((-40))+rsi]
	cmp	ecx,DWORD PTR[40+rsp]
DB	102,15,56,222,208
DB	102,15,56,222,216
DB	102,15,56,222,224
	cmovge	r10,rbp
	cmovg	r14,rbp
DB	102,15,56,222,232
	movups	xmm0,XMMWORD PTR[((-24))+rsi]
	cmp	ecx,DWORD PTR[44+rsp]
DB	102,15,56,222,209
DB	102,15,56,222,217
DB	102,15,56,222,225
	cmovge	r11,rbp
	cmovg	r15,rbp
DB	102,15,56,222,233
	movups	xmm1,XMMWORD PTR[((-8))+rsi]
	movdqa	xmm11,xmm10
DB	102,15,56,222,208
	prefetcht0	[15+rbx*1+r12]
	prefetcht0	[15+rbx*1+r13]
DB	102,15,56,222,216
	prefetcht0	[15+rbx*1+r14]
	prefetcht0	[15+rbx*1+r15]
DB	102,15,56,222,224
DB	102,15,56,222,232
	movups	xmm0,XMMWORD PTR[((128-120))+rsi]
	pxor	xmm12,xmm12

DB	102,15,56,222,209
	pcmpgtd	xmm11,xmm12
	movdqu	xmm12,XMMWORD PTR[((-120))+rsi]
DB	102,15,56,222,217
	paddd	xmm10,xmm11
	movdqa	XMMWORD PTR[32+rsp],xmm10
DB	102,15,56,222,225
DB	102,15,56,222,233
	movups	xmm1,XMMWORD PTR[((144-120))+rsi]

	cmp	eax,11

DB	102,15,56,222,208
DB	102,15,56,222,216
DB	102,15,56,222,224
DB	102,15,56,222,232
	movups	xmm0,XMMWORD PTR[((160-120))+rsi]

	jb	$L$dec4x_tail

DB	102,15,56,222,209
DB	102,15,56,222,217
DB	102,15,56,222,225
DB	102,15,56,222,233
	movups	xmm1,XMMWORD PTR[((176-120))+rsi]

DB	102,15,56,222,208
DB	102,15,56,222,216
DB	102,15,56,222,224
DB	102,15,56,222,232
	movups	xmm0,XMMWORD PTR[((192-120))+rsi]

	je	$L$dec4x_tail

DB	102,15,56,222,209
DB	102,15,56,222,217
DB	102,15,56,222,225
DB	102,15,56,222,233
	movups	xmm1,XMMWORD PTR[((208-120))+rsi]

DB	102,15,56,222,208
DB	102,15,56,222,216
DB	102,15,56,222,224
DB	102,15,56,222,232
	movups	xmm0,XMMWORD PTR[((224-120))+rsi]
	jmp	$L$dec4x_tail

ALIGN	32
$L$dec4x_tail::
DB	102,15,56,222,209
DB	102,15,56,222,217
DB	102,15,56,222,225
	pxor	xmm6,xmm0
	pxor	xmm7,xmm0
DB	102,15,56,222,233
	movdqu	xmm1,XMMWORD PTR[((16-120))+rsi]
	pxor	xmm8,xmm0
	pxor	xmm9,xmm0
	movdqu	xmm0,XMMWORD PTR[((32-120))+rsi]

DB	102,15,56,223,214
DB	102,15,56,223,223
	movdqu	xmm6,XMMWORD PTR[((-16))+rbx*1+r8]
	movdqu	xmm7,XMMWORD PTR[((-16))+rbx*1+r9]
DB	102,65,15,56,223,224
DB	102,65,15,56,223,233
	movdqu	xmm8,XMMWORD PTR[((-16))+rbx*1+r10]
	movdqu	xmm9,XMMWORD PTR[((-16))+rbx*1+r11]

	movups	XMMWORD PTR[(-16)+rbx*1+r12],xmm2
	movdqu	xmm2,XMMWORD PTR[rbx*1+r8]
	movups	XMMWORD PTR[(-16)+rbx*1+r13],xmm3
	movdqu	xmm3,XMMWORD PTR[rbx*1+r9]
	pxor	xmm2,xmm12
	movups	XMMWORD PTR[(-16)+rbx*1+r14],xmm4
	movdqu	xmm4,XMMWORD PTR[rbx*1+r10]
	pxor	xmm3,xmm12
	movups	XMMWORD PTR[(-16)+rbx*1+r15],xmm5
	movdqu	xmm5,XMMWORD PTR[rbx*1+r11]
	pxor	xmm4,xmm12
	pxor	xmm5,xmm12

	dec	edx
	jnz	$L$oop_dec4x

	mov	rax,QWORD PTR[16+rsp]

	mov	edx,DWORD PTR[24+rsp]

	lea	rdi,QWORD PTR[160+rdi]
	dec	edx
	jnz	$L$dec4x_loop_grande

$L$dec4x_done::
	movaps	xmm6,XMMWORD PTR[((-216))+rax]
	movaps	xmm7,XMMWORD PTR[((-200))+rax]
	movaps	xmm8,XMMWORD PTR[((-184))+rax]
	movaps	xmm9,XMMWORD PTR[((-168))+rax]
	movaps	xmm10,XMMWORD PTR[((-152))+rax]
	movaps	xmm11,XMMWORD PTR[((-136))+rax]
	movaps	xmm12,XMMWORD PTR[((-120))+rax]



	mov	r15,QWORD PTR[((-48))+rax]

	mov	r14,QWORD PTR[((-40))+rax]

	mov	r13,QWORD PTR[((-32))+rax]

	mov	r12,QWORD PTR[((-24))+rax]

	mov	rbp,QWORD PTR[((-16))+rax]

	mov	rbx,QWORD PTR[((-8))+rax]

	lea	rsp,QWORD PTR[rax]

$L$dec4x_epilogue::
	mov	rdi,QWORD PTR[8+rsp]	;WIN64 epilogue
	mov	rsi,QWORD PTR[16+rsp]
	DB	0F3h,0C3h		;repret

$L$SEH_end_aesni_multi_cbc_decrypt::
aesni_multi_cbc_decrypt	ENDP

ALIGN	32
aesni_multi_cbc_encrypt_avx	PROC PRIVATE
	mov	QWORD PTR[8+rsp],rdi	;WIN64 prologue
	mov	QWORD PTR[16+rsp],rsi
	mov	rax,rsp
$L$SEH_begin_aesni_multi_cbc_encrypt_avx::
	mov	rdi,rcx
	mov	rsi,rdx
	mov	rdx,r8



_avx_cbc_enc_shortcut::
	mov	rax,rsp

	push	rbx

	push	rbp

	push	r12

	push	r13

	push	r14

	push	r15

	lea	rsp,QWORD PTR[((-168))+rsp]
	movaps	XMMWORD PTR[rsp],xmm6
	movaps	XMMWORD PTR[16+rsp],xmm7
	movaps	XMMWORD PTR[32+rsp],xmm8
	movaps	XMMWORD PTR[48+rsp],xmm9
	movaps	XMMWORD PTR[64+rsp],xmm10
	movaps	XMMWORD PTR[80+rsp],xmm11
	movaps	XMMWORD PTR[(-120)+rax],xmm12
	movaps	XMMWORD PTR[(-104)+rax],xmm13
	movaps	XMMWORD PTR[(-88)+rax],xmm14
	movaps	XMMWORD PTR[(-72)+rax],xmm15








	sub	rsp,192
	and	rsp,-128
	mov	QWORD PTR[16+rsp],rax


$L$enc8x_body::
	vzeroupper
	vmovdqu	xmm15,XMMWORD PTR[rsi]
	lea	rsi,QWORD PTR[120+rsi]
	lea	rdi,QWORD PTR[160+rdi]
	shr	edx,1

$L$enc8x_loop_grande::

	xor	edx,edx

	mov	ecx,DWORD PTR[((-144))+rdi]

	mov	r8,QWORD PTR[((-160))+rdi]
	cmp	ecx,edx

	mov	rbx,QWORD PTR[((-152))+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	vmovdqu	xmm2,XMMWORD PTR[((-136))+rdi]
	mov	DWORD PTR[32+rsp],ecx
	cmovle	r8,rsp
	sub	rbx,r8
	mov	QWORD PTR[64+rsp],rbx

	mov	ecx,DWORD PTR[((-104))+rdi]

	mov	r9,QWORD PTR[((-120))+rdi]
	cmp	ecx,edx

	mov	rbp,QWORD PTR[((-112))+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	vmovdqu	xmm3,XMMWORD PTR[((-96))+rdi]
	mov	DWORD PTR[36+rsp],ecx
	cmovle	r9,rsp
	sub	rbp,r9
	mov	QWORD PTR[72+rsp],rbp

	mov	ecx,DWORD PTR[((-64))+rdi]

	mov	r10,QWORD PTR[((-80))+rdi]
	cmp	ecx,edx

	mov	rbp,QWORD PTR[((-72))+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	vmovdqu	xmm4,XMMWORD PTR[((-56))+rdi]
	mov	DWORD PTR[40+rsp],ecx
	cmovle	r10,rsp
	sub	rbp,r10
	mov	QWORD PTR[80+rsp],rbp

	mov	ecx,DWORD PTR[((-24))+rdi]

	mov	r11,QWORD PTR[((-40))+rdi]
	cmp	ecx,edx

	mov	rbp,QWORD PTR[((-32))+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	vmovdqu	xmm5,XMMWORD PTR[((-16))+rdi]
	mov	DWORD PTR[44+rsp],ecx
	cmovle	r11,rsp
	sub	rbp,r11
	mov	QWORD PTR[88+rsp],rbp

	mov	ecx,DWORD PTR[16+rdi]

	mov	r12,QWORD PTR[rdi]
	cmp	ecx,edx

	mov	rbp,QWORD PTR[8+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	vmovdqu	xmm6,XMMWORD PTR[24+rdi]
	mov	DWORD PTR[48+rsp],ecx
	cmovle	r12,rsp
	sub	rbp,r12
	mov	QWORD PTR[96+rsp],rbp

	mov	ecx,DWORD PTR[56+rdi]

	mov	r13,QWORD PTR[40+rdi]
	cmp	ecx,edx

	mov	rbp,QWORD PTR[48+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	vmovdqu	xmm7,XMMWORD PTR[64+rdi]
	mov	DWORD PTR[52+rsp],ecx
	cmovle	r13,rsp
	sub	rbp,r13
	mov	QWORD PTR[104+rsp],rbp

	mov	ecx,DWORD PTR[96+rdi]

	mov	r14,QWORD PTR[80+rdi]
	cmp	ecx,edx

	mov	rbp,QWORD PTR[88+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	vmovdqu	xmm8,XMMWORD PTR[104+rdi]
	mov	DWORD PTR[56+rsp],ecx
	cmovle	r14,rsp
	sub	rbp,r14
	mov	QWORD PTR[112+rsp],rbp

	mov	ecx,DWORD PTR[136+rdi]

	mov	r15,QWORD PTR[120+rdi]
	cmp	ecx,edx

	mov	rbp,QWORD PTR[128+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	vmovdqu	xmm9,XMMWORD PTR[144+rdi]
	mov	DWORD PTR[60+rsp],ecx
	cmovle	r15,rsp
	sub	rbp,r15
	mov	QWORD PTR[120+rsp],rbp
	test	edx,edx
	jz	$L$enc8x_done

	vmovups	xmm1,XMMWORD PTR[((16-120))+rsi]
	vmovups	xmm0,XMMWORD PTR[((32-120))+rsi]
	mov	eax,DWORD PTR[((240-120))+rsi]

	vpxor	xmm10,xmm15,XMMWORD PTR[r8]
	lea	rbp,QWORD PTR[128+rsp]
	vpxor	xmm11,xmm15,XMMWORD PTR[r9]
	vpxor	xmm12,xmm15,XMMWORD PTR[r10]
	vpxor	xmm13,xmm15,XMMWORD PTR[r11]
	vpxor	xmm2,xmm2,xmm10
	vpxor	xmm10,xmm15,XMMWORD PTR[r12]
	vpxor	xmm3,xmm3,xmm11
	vpxor	xmm11,xmm15,XMMWORD PTR[r13]
	vpxor	xmm4,xmm4,xmm12
	vpxor	xmm12,xmm15,XMMWORD PTR[r14]
	vpxor	xmm5,xmm5,xmm13
	vpxor	xmm13,xmm15,XMMWORD PTR[r15]
	vpxor	xmm6,xmm6,xmm10
	mov	ecx,1
	vpxor	xmm7,xmm7,xmm11
	vpxor	xmm8,xmm8,xmm12
	vpxor	xmm9,xmm9,xmm13
	jmp	$L$oop_enc8x

ALIGN	32
$L$oop_enc8x::
	vaesenc	xmm2,xmm2,xmm1
	cmp	ecx,DWORD PTR[((32+0))+rsp]
	vaesenc	xmm3,xmm3,xmm1
	prefetcht0	[31+r8]
	vaesenc	xmm4,xmm4,xmm1
	vaesenc	xmm5,xmm5,xmm1
	lea	rbx,QWORD PTR[rbx*1+r8]
	cmovge	r8,rsp
	vaesenc	xmm6,xmm6,xmm1
	cmovg	rbx,rsp
	vaesenc	xmm7,xmm7,xmm1
	sub	rbx,r8
	vaesenc	xmm8,xmm8,xmm1
	vpxor	xmm10,xmm15,XMMWORD PTR[16+r8]
	mov	QWORD PTR[((64+0))+rsp],rbx
	vaesenc	xmm9,xmm9,xmm1
	vmovups	xmm1,XMMWORD PTR[((-72))+rsi]
	lea	r8,QWORD PTR[16+rbx*1+r8]
	vmovdqu	XMMWORD PTR[rbp],xmm10
	vaesenc	xmm2,xmm2,xmm0
	cmp	ecx,DWORD PTR[((32+4))+rsp]
	mov	rbx,QWORD PTR[((64+8))+rsp]
	vaesenc	xmm3,xmm3,xmm0
	prefetcht0	[31+r9]
	vaesenc	xmm4,xmm4,xmm0
	vaesenc	xmm5,xmm5,xmm0
	lea	rbx,QWORD PTR[rbx*1+r9]
	cmovge	r9,rsp
	vaesenc	xmm6,xmm6,xmm0
	cmovg	rbx,rsp
	vaesenc	xmm7,xmm7,xmm0
	sub	rbx,r9
	vaesenc	xmm8,xmm8,xmm0
	vpxor	xmm11,xmm15,XMMWORD PTR[16+r9]
	mov	QWORD PTR[((64+8))+rsp],rbx
	vaesenc	xmm9,xmm9,xmm0
	vmovups	xmm0,XMMWORD PTR[((-56))+rsi]
	lea	r9,QWORD PTR[16+rbx*1+r9]
	vmovdqu	XMMWORD PTR[16+rbp],xmm11
	vaesenc	xmm2,xmm2,xmm1
	cmp	ecx,DWORD PTR[((32+8))+rsp]
	mov	rbx,QWORD PTR[((64+16))+rsp]
	vaesenc	xmm3,xmm3,xmm1
	prefetcht0	[31+r10]
	vaesenc	xmm4,xmm4,xmm1
	prefetcht0	[15+r8]
	vaesenc	xmm5,xmm5,xmm1
	lea	rbx,QWORD PTR[rbx*1+r10]
	cmovge	r10,rsp
	vaesenc	xmm6,xmm6,xmm1
	cmovg	rbx,rsp
	vaesenc	xmm7,xmm7,xmm1
	sub	rbx,r10
	vaesenc	xmm8,xmm8,xmm1
	vpxor	xmm12,xmm15,XMMWORD PTR[16+r10]
	mov	QWORD PTR[((64+16))+rsp],rbx
	vaesenc	xmm9,xmm9,xmm1
	vmovups	xmm1,XMMWORD PTR[((-40))+rsi]
	lea	r10,QWORD PTR[16+rbx*1+r10]
	vmovdqu	XMMWORD PTR[32+rbp],xmm12
	vaesenc	xmm2,xmm2,xmm0
	cmp	ecx,DWORD PTR[((32+12))+rsp]
	mov	rbx,QWORD PTR[((64+24))+rsp]
	vaesenc	xmm3,xmm3,xmm0
	prefetcht0	[31+r11]
	vaesenc	xmm4,xmm4,xmm0
	prefetcht0	[15+r9]
	vaesenc	xmm5,xmm5,xmm0
	lea	rbx,QWORD PTR[rbx*1+r11]
	cmovge	r11,rsp
	vaesenc	xmm6,xmm6,xmm0
	cmovg	rbx,rsp
	vaesenc	xmm7,xmm7,xmm0
	sub	rbx,r11
	vaesenc	xmm8,xmm8,xmm0
	vpxor	xmm13,xmm15,XMMWORD PTR[16+r11]
	mov	QWORD PTR[((64+24))+rsp],rbx
	vaesenc	xmm9,xmm9,xmm0
	vmovups	xmm0,XMMWORD PTR[((-24))+rsi]
	lea	r11,QWORD PTR[16+rbx*1+r11]
	vmovdqu	XMMWORD PTR[48+rbp],xmm13
	vaesenc	xmm2,xmm2,xmm1
	cmp	ecx,DWORD PTR[((32+16))+rsp]
	mov	rbx,QWORD PTR[((64+32))+rsp]
	vaesenc	xmm3,xmm3,xmm1
	prefetcht0	[31+r12]
	vaesenc	xmm4,xmm4,xmm1
	prefetcht0	[15+r10]
	vaesenc	xmm5,xmm5,xmm1
	lea	rbx,QWORD PTR[rbx*1+r12]
	cmovge	r12,rsp
	vaesenc	xmm6,xmm6,xmm1
	cmovg	rbx,rsp
	vaesenc	xmm7,xmm7,xmm1
	sub	rbx,r12
	vaesenc	xmm8,xmm8,xmm1
	vpxor	xmm10,xmm15,XMMWORD PTR[16+r12]
	mov	QWORD PTR[((64+32))+rsp],rbx
	vaesenc	xmm9,xmm9,xmm1
	vmovups	xmm1,XMMWORD PTR[((-8))+rsi]
	lea	r12,QWORD PTR[16+rbx*1+r12]
	vaesenc	xmm2,xmm2,xmm0
	cmp	ecx,DWORD PTR[((32+20))+rsp]
	mov	rbx,QWORD PTR[((64+40))+rsp]
	vaesenc	xmm3,xmm3,xmm0
	prefetcht0	[31+r13]
	vaesenc	xmm4,xmm4,xmm0
	prefetcht0	[15+r11]
	vaesenc	xmm5,xmm5,xmm0
	lea	rbx,QWORD PTR[r13*1+rbx]
	cmovge	r13,rsp
	vaesenc	xmm6,xmm6,xmm0
	cmovg	rbx,rsp
	vaesenc	xmm7,xmm7,xmm0
	sub	rbx,r13
	vaesenc	xmm8,xmm8,xmm0
	vpxor	xmm11,xmm15,XMMWORD PTR[16+r13]
	mov	QWORD PTR[((64+40))+rsp],rbx
	vaesenc	xmm9,xmm9,xmm0
	vmovups	xmm0,XMMWORD PTR[8+rsi]
	lea	r13,QWORD PTR[16+rbx*1+r13]
	vaesenc	xmm2,xmm2,xmm1
	cmp	ecx,DWORD PTR[((32+24))+rsp]
	mov	rbx,QWORD PTR[((64+48))+rsp]
	vaesenc	xmm3,xmm3,xmm1
	prefetcht0	[31+r14]
	vaesenc	xmm4,xmm4,xmm1
	prefetcht0	[15+r12]
	vaesenc	xmm5,xmm5,xmm1
	lea	rbx,QWORD PTR[rbx*1+r14]
	cmovge	r14,rsp
	vaesenc	xmm6,xmm6,xmm1
	cmovg	rbx,rsp
	vaesenc	xmm7,xmm7,xmm1
	sub	rbx,r14
	vaesenc	xmm8,xmm8,xmm1
	vpxor	xmm12,xmm15,XMMWORD PTR[16+r14]
	mov	QWORD PTR[((64+48))+rsp],rbx
	vaesenc	xmm9,xmm9,xmm1
	vmovups	xmm1,XMMWORD PTR[24+rsi]
	lea	r14,QWORD PTR[16+rbx*1+r14]
	vaesenc	xmm2,xmm2,xmm0
	cmp	ecx,DWORD PTR[((32+28))+rsp]
	mov	rbx,QWORD PTR[((64+56))+rsp]
	vaesenc	xmm3,xmm3,xmm0
	prefetcht0	[31+r15]
	vaesenc	xmm4,xmm4,xmm0
	prefetcht0	[15+r13]
	vaesenc	xmm5,xmm5,xmm0
	lea	rbx,QWORD PTR[rbx*1+r15]
	cmovge	r15,rsp
	vaesenc	xmm6,xmm6,xmm0
	cmovg	rbx,rsp
	vaesenc	xmm7,xmm7,xmm0
	sub	rbx,r15
	vaesenc	xmm8,xmm8,xmm0
	vpxor	xmm13,xmm15,XMMWORD PTR[16+r15]
	mov	QWORD PTR[((64+56))+rsp],rbx
	vaesenc	xmm9,xmm9,xmm0
	vmovups	xmm0,XMMWORD PTR[40+rsi]
	lea	r15,QWORD PTR[16+rbx*1+r15]
	vmovdqu	xmm14,XMMWORD PTR[32+rsp]
	prefetcht0	[15+r14]
	prefetcht0	[15+r15]
	cmp	eax,11
	jb	$L$enc8x_tail

	vaesenc	xmm2,xmm2,xmm1
	vaesenc	xmm3,xmm3,xmm1
	vaesenc	xmm4,xmm4,xmm1
	vaesenc	xmm5,xmm5,xmm1
	vaesenc	xmm6,xmm6,xmm1
	vaesenc	xmm7,xmm7,xmm1
	vaesenc	xmm8,xmm8,xmm1
	vaesenc	xmm9,xmm9,xmm1
	vmovups	xmm1,XMMWORD PTR[((176-120))+rsi]

	vaesenc	xmm2,xmm2,xmm0
	vaesenc	xmm3,xmm3,xmm0
	vaesenc	xmm4,xmm4,xmm0
	vaesenc	xmm5,xmm5,xmm0
	vaesenc	xmm6,xmm6,xmm0
	vaesenc	xmm7,xmm7,xmm0
	vaesenc	xmm8,xmm8,xmm0
	vaesenc	xmm9,xmm9,xmm0
	vmovups	xmm0,XMMWORD PTR[((192-120))+rsi]
	je	$L$enc8x_tail

	vaesenc	xmm2,xmm2,xmm1
	vaesenc	xmm3,xmm3,xmm1
	vaesenc	xmm4,xmm4,xmm1
	vaesenc	xmm5,xmm5,xmm1
	vaesenc	xmm6,xmm6,xmm1
	vaesenc	xmm7,xmm7,xmm1
	vaesenc	xmm8,xmm8,xmm1
	vaesenc	xmm9,xmm9,xmm1
	vmovups	xmm1,XMMWORD PTR[((208-120))+rsi]

	vaesenc	xmm2,xmm2,xmm0
	vaesenc	xmm3,xmm3,xmm0
	vaesenc	xmm4,xmm4,xmm0
	vaesenc	xmm5,xmm5,xmm0
	vaesenc	xmm6,xmm6,xmm0
	vaesenc	xmm7,xmm7,xmm0
	vaesenc	xmm8,xmm8,xmm0
	vaesenc	xmm9,xmm9,xmm0
	vmovups	xmm0,XMMWORD PTR[((224-120))+rsi]

$L$enc8x_tail::
	vaesenc	xmm2,xmm2,xmm1
	vpxor	xmm15,xmm15,xmm15
	vaesenc	xmm3,xmm3,xmm1
	vaesenc	xmm4,xmm4,xmm1
	vpcmpgtd	xmm15,xmm14,xmm15
	vaesenc	xmm5,xmm5,xmm1
	vaesenc	xmm6,xmm6,xmm1
	vpaddd	xmm15,xmm15,xmm14
	vmovdqu	xmm14,XMMWORD PTR[48+rsp]
	vaesenc	xmm7,xmm7,xmm1
	mov	rbx,QWORD PTR[64+rsp]
	vaesenc	xmm8,xmm8,xmm1
	vaesenc	xmm9,xmm9,xmm1
	vmovups	xmm1,XMMWORD PTR[((16-120))+rsi]

	vaesenclast	xmm2,xmm2,xmm0
	vmovdqa	XMMWORD PTR[32+rsp],xmm15
	vpxor	xmm15,xmm15,xmm15
	vaesenclast	xmm3,xmm3,xmm0
	vaesenclast	xmm4,xmm4,xmm0
	vpcmpgtd	xmm15,xmm14,xmm15
	vaesenclast	xmm5,xmm5,xmm0
	vaesenclast	xmm6,xmm6,xmm0
	vpaddd	xmm14,xmm14,xmm15
	vmovdqu	xmm15,XMMWORD PTR[((-120))+rsi]
	vaesenclast	xmm7,xmm7,xmm0
	vaesenclast	xmm8,xmm8,xmm0
	vmovdqa	XMMWORD PTR[48+rsp],xmm14
	vaesenclast	xmm9,xmm9,xmm0
	vmovups	xmm0,XMMWORD PTR[((32-120))+rsi]

	vmovups	XMMWORD PTR[(-16)+r8],xmm2
	sub	r8,rbx
	vpxor	xmm2,xmm2,XMMWORD PTR[rbp]
	vmovups	XMMWORD PTR[(-16)+r9],xmm3
	sub	r9,QWORD PTR[72+rsp]
	vpxor	xmm3,xmm3,XMMWORD PTR[16+rbp]
	vmovups	XMMWORD PTR[(-16)+r10],xmm4
	sub	r10,QWORD PTR[80+rsp]
	vpxor	xmm4,xmm4,XMMWORD PTR[32+rbp]
	vmovups	XMMWORD PTR[(-16)+r11],xmm5
	sub	r11,QWORD PTR[88+rsp]
	vpxor	xmm5,xmm5,XMMWORD PTR[48+rbp]
	vmovups	XMMWORD PTR[(-16)+r12],xmm6
	sub	r12,QWORD PTR[96+rsp]
	vpxor	xmm6,xmm6,xmm10
	vmovups	XMMWORD PTR[(-16)+r13],xmm7
	sub	r13,QWORD PTR[104+rsp]
	vpxor	xmm7,xmm7,xmm11
	vmovups	XMMWORD PTR[(-16)+r14],xmm8
	sub	r14,QWORD PTR[112+rsp]
	vpxor	xmm8,xmm8,xmm12
	vmovups	XMMWORD PTR[(-16)+r15],xmm9
	sub	r15,QWORD PTR[120+rsp]
	vpxor	xmm9,xmm9,xmm13

	dec	edx
	jnz	$L$oop_enc8x

	mov	rax,QWORD PTR[16+rsp]






$L$enc8x_done::
	vzeroupper
	movaps	xmm6,XMMWORD PTR[((-216))+rax]
	movaps	xmm7,XMMWORD PTR[((-200))+rax]
	movaps	xmm8,XMMWORD PTR[((-184))+rax]
	movaps	xmm9,XMMWORD PTR[((-168))+rax]
	movaps	xmm10,XMMWORD PTR[((-152))+rax]
	movaps	xmm11,XMMWORD PTR[((-136))+rax]
	movaps	xmm12,XMMWORD PTR[((-120))+rax]
	movaps	xmm13,XMMWORD PTR[((-104))+rax]
	movaps	xmm14,XMMWORD PTR[((-88))+rax]
	movaps	xmm15,XMMWORD PTR[((-72))+rax]
	mov	r15,QWORD PTR[((-48))+rax]

	mov	r14,QWORD PTR[((-40))+rax]

	mov	r13,QWORD PTR[((-32))+rax]

	mov	r12,QWORD PTR[((-24))+rax]

	mov	rbp,QWORD PTR[((-16))+rax]

	mov	rbx,QWORD PTR[((-8))+rax]

	lea	rsp,QWORD PTR[rax]

$L$enc8x_epilogue::
	mov	rdi,QWORD PTR[8+rsp]	;WIN64 epilogue
	mov	rsi,QWORD PTR[16+rsp]
	DB	0F3h,0C3h		;repret

$L$SEH_end_aesni_multi_cbc_encrypt_avx::
aesni_multi_cbc_encrypt_avx	ENDP


ALIGN	32
aesni_multi_cbc_decrypt_avx	PROC PRIVATE
	mov	QWORD PTR[8+rsp],rdi	;WIN64 prologue
	mov	QWORD PTR[16+rsp],rsi
	mov	rax,rsp
$L$SEH_begin_aesni_multi_cbc_decrypt_avx::
	mov	rdi,rcx
	mov	rsi,rdx
	mov	rdx,r8



_avx_cbc_dec_shortcut::
	mov	rax,rsp

	push	rbx

	push	rbp

	push	r12

	push	r13

	push	r14

	push	r15

	lea	rsp,QWORD PTR[((-168))+rsp]
	movaps	XMMWORD PTR[rsp],xmm6
	movaps	XMMWORD PTR[16+rsp],xmm7
	movaps	XMMWORD PTR[32+rsp],xmm8
	movaps	XMMWORD PTR[48+rsp],xmm9
	movaps	XMMWORD PTR[64+rsp],xmm10
	movaps	XMMWORD PTR[80+rsp],xmm11
	movaps	XMMWORD PTR[(-120)+rax],xmm12
	movaps	XMMWORD PTR[(-104)+rax],xmm13
	movaps	XMMWORD PTR[(-88)+rax],xmm14
	movaps	XMMWORD PTR[(-72)+rax],xmm15









	sub	rsp,256
	and	rsp,-256
	sub	rsp,192
	mov	QWORD PTR[16+rsp],rax


$L$dec8x_body::
	vzeroupper
	vmovdqu	xmm15,XMMWORD PTR[rsi]
	lea	rsi,QWORD PTR[120+rsi]
	lea	rdi,QWORD PTR[160+rdi]
	shr	edx,1

$L$dec8x_loop_grande::

	xor	edx,edx

	mov	ecx,DWORD PTR[((-144))+rdi]

	mov	r8,QWORD PTR[((-160))+rdi]
	cmp	ecx,edx

	mov	rbx,QWORD PTR[((-152))+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	vmovdqu	xmm2,XMMWORD PTR[((-136))+rdi]
	mov	DWORD PTR[32+rsp],ecx
	cmovle	r8,rsp
	sub	rbx,r8
	mov	QWORD PTR[64+rsp],rbx
	vmovdqu	XMMWORD PTR[192+rsp],xmm2

	mov	ecx,DWORD PTR[((-104))+rdi]

	mov	r9,QWORD PTR[((-120))+rdi]
	cmp	ecx,edx

	mov	rbp,QWORD PTR[((-112))+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	vmovdqu	xmm3,XMMWORD PTR[((-96))+rdi]
	mov	DWORD PTR[36+rsp],ecx
	cmovle	r9,rsp
	sub	rbp,r9
	mov	QWORD PTR[72+rsp],rbp
	vmovdqu	XMMWORD PTR[208+rsp],xmm3

	mov	ecx,DWORD PTR[((-64))+rdi]

	mov	r10,QWORD PTR[((-80))+rdi]
	cmp	ecx,edx

	mov	rbp,QWORD PTR[((-72))+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	vmovdqu	xmm4,XMMWORD PTR[((-56))+rdi]
	mov	DWORD PTR[40+rsp],ecx
	cmovle	r10,rsp
	sub	rbp,r10
	mov	QWORD PTR[80+rsp],rbp
	vmovdqu	XMMWORD PTR[224+rsp],xmm4

	mov	ecx,DWORD PTR[((-24))+rdi]

	mov	r11,QWORD PTR[((-40))+rdi]
	cmp	ecx,edx

	mov	rbp,QWORD PTR[((-32))+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	vmovdqu	xmm5,XMMWORD PTR[((-16))+rdi]
	mov	DWORD PTR[44+rsp],ecx
	cmovle	r11,rsp
	sub	rbp,r11
	mov	QWORD PTR[88+rsp],rbp
	vmovdqu	XMMWORD PTR[240+rsp],xmm5

	mov	ecx,DWORD PTR[16+rdi]

	mov	r12,QWORD PTR[rdi]
	cmp	ecx,edx

	mov	rbp,QWORD PTR[8+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	vmovdqu	xmm6,XMMWORD PTR[24+rdi]
	mov	DWORD PTR[48+rsp],ecx
	cmovle	r12,rsp
	sub	rbp,r12
	mov	QWORD PTR[96+rsp],rbp
	vmovdqu	XMMWORD PTR[256+rsp],xmm6

	mov	ecx,DWORD PTR[56+rdi]

	mov	r13,QWORD PTR[40+rdi]
	cmp	ecx,edx

	mov	rbp,QWORD PTR[48+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	vmovdqu	xmm7,XMMWORD PTR[64+rdi]
	mov	DWORD PTR[52+rsp],ecx
	cmovle	r13,rsp
	sub	rbp,r13
	mov	QWORD PTR[104+rsp],rbp
	vmovdqu	XMMWORD PTR[272+rsp],xmm7

	mov	ecx,DWORD PTR[96+rdi]

	mov	r14,QWORD PTR[80+rdi]
	cmp	ecx,edx

	mov	rbp,QWORD PTR[88+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	vmovdqu	xmm8,XMMWORD PTR[104+rdi]
	mov	DWORD PTR[56+rsp],ecx
	cmovle	r14,rsp
	sub	rbp,r14
	mov	QWORD PTR[112+rsp],rbp
	vmovdqu	XMMWORD PTR[288+rsp],xmm8

	mov	ecx,DWORD PTR[136+rdi]

	mov	r15,QWORD PTR[120+rdi]
	cmp	ecx,edx

	mov	rbp,QWORD PTR[128+rdi]
	cmovg	edx,ecx
	test	ecx,ecx

	vmovdqu	xmm9,XMMWORD PTR[144+rdi]
	mov	DWORD PTR[60+rsp],ecx
	cmovle	r15,rsp
	sub	rbp,r15
	mov	QWORD PTR[120+rsp],rbp
	vmovdqu	XMMWORD PTR[304+rsp],xmm9
	test	edx,edx
	jz	$L$dec8x_done

	vmovups	xmm1,XMMWORD PTR[((16-120))+rsi]
	vmovups	xmm0,XMMWORD PTR[((32-120))+rsi]
	mov	eax,DWORD PTR[((240-120))+rsi]
	lea	rbp,QWORD PTR[((192+128))+rsp]

	vmovdqu	xmm2,XMMWORD PTR[r8]
	vmovdqu	xmm3,XMMWORD PTR[r9]
	vmovdqu	xmm4,XMMWORD PTR[r10]
	vmovdqu	xmm5,XMMWORD PTR[r11]
	vmovdqu	xmm6,XMMWORD PTR[r12]
	vmovdqu	xmm7,XMMWORD PTR[r13]
	vmovdqu	xmm8,XMMWORD PTR[r14]
	vmovdqu	xmm9,XMMWORD PTR[r15]
	vmovdqu	XMMWORD PTR[rbp],xmm2
	vpxor	xmm2,xmm2,xmm15
	vmovdqu	XMMWORD PTR[16+rbp],xmm3
	vpxor	xmm3,xmm3,xmm15
	vmovdqu	XMMWORD PTR[32+rbp],xmm4
	vpxor	xmm4,xmm4,xmm15
	vmovdqu	XMMWORD PTR[48+rbp],xmm5
	vpxor	xmm5,xmm5,xmm15
	vmovdqu	XMMWORD PTR[64+rbp],xmm6
	vpxor	xmm6,xmm6,xmm15
	vmovdqu	XMMWORD PTR[80+rbp],xmm7
	vpxor	xmm7,xmm7,xmm15
	vmovdqu	XMMWORD PTR[96+rbp],xmm8
	vpxor	xmm8,xmm8,xmm15
	vmovdqu	XMMWORD PTR[112+rbp],xmm9
	vpxor	xmm9,xmm9,xmm15
	xor	rbp,080h
	mov	ecx,1
	jmp	$L$oop_dec8x

ALIGN	32
$L$oop_dec8x::
	vaesdec	xmm2,xmm2,xmm1
	cmp	ecx,DWORD PTR[((32+0))+rsp]
	vaesdec	xmm3,xmm3,xmm1
	prefetcht0	[31+r8]
	vaesdec	xmm4,xmm4,xmm1
	vaesdec	xmm5,xmm5,xmm1
	lea	rbx,QWORD PTR[rbx*1+r8]
	cmovge	r8,rsp
	vaesdec	xmm6,xmm6,xmm1
	cmovg	rbx,rsp
	vaesdec	xmm7,xmm7,xmm1
	sub	rbx,r8
	vaesdec	xmm8,xmm8,xmm1
	vmovdqu	xmm10,XMMWORD PTR[16+r8]
	mov	QWORD PTR[((64+0))+rsp],rbx
	vaesdec	xmm9,xmm9,xmm1
	vmovups	xmm1,XMMWORD PTR[((-72))+rsi]
	lea	r8,QWORD PTR[16+rbx*1+r8]
	vmovdqu	XMMWORD PTR[128+rsp],xmm10
	vaesdec	xmm2,xmm2,xmm0
	cmp	ecx,DWORD PTR[((32+4))+rsp]
	mov	rbx,QWORD PTR[((64+8))+rsp]
	vaesdec	xmm3,xmm3,xmm0
	prefetcht0	[31+r9]
	vaesdec	xmm4,xmm4,xmm0
	vaesdec	xmm5,xmm5,xmm0
	lea	rbx,QWORD PTR[rbx*1+r9]
	cmovge	r9,rsp
	vaesdec	xmm6,xmm6,xmm0
	cmovg	rbx,rsp
	vaesdec	xmm7,xmm7,xmm0
	sub	rbx,r9
	vaesdec	xmm8,xmm8,xmm0
	vmovdqu	xmm11,XMMWORD PTR[16+r9]
	mov	QWORD PTR[((64+8))+rsp],rbx
	vaesdec	xmm9,xmm9,xmm0
	vmovups	xmm0,XMMWORD PTR[((-56))+rsi]
	lea	r9,QWORD PTR[16+rbx*1+r9]
	vmovdqu	XMMWORD PTR[144+rsp],xmm11
	vaesdec	xmm2,xmm2,xmm1
	cmp	ecx,DWORD PTR[((32+8))+rsp]
	mov	rbx,QWORD PTR[((64+16))+rsp]
	vaesdec	xmm3,xmm3,xmm1
	prefetcht0	[31+r10]
	vaesdec	xmm4,xmm4,xmm1
	prefetcht0	[15+r8]
	vaesdec	xmm5,xmm5,xmm1
	lea	rbx,QWORD PTR[rbx*1+r10]
	cmovge	r10,rsp
	vaesdec	xmm6,xmm6,xmm1
	cmovg	rbx,rsp
	vaesdec	xmm7,xmm7,xmm1
	sub	rbx,r10
	vaesdec	xmm8,xmm8,xmm1
	vmovdqu	xmm12,XMMWORD PTR[16+r10]
	mov	QWORD PTR[((64+16))+rsp],rbx
	vaesdec	xmm9,xmm9,xmm1
	vmovups	xmm1,XMMWORD PTR[((-40))+rsi]
	lea	r10,QWORD PTR[16+rbx*1+r10]
	vmovdqu	XMMWORD PTR[160+rsp],xmm12
	vaesdec	xmm2,xmm2,xmm0
	cmp	ecx,DWORD PTR[((32+12))+rsp]
	mov	rbx,QWORD PTR[((64+24))+rsp]
	vaesdec	xmm3,xmm3,xmm0
	prefetcht0	[31+r11]
	vaesdec	xmm4,xmm4,xmm0
	prefetcht0	[15+r9]
	vaesdec	xmm5,xmm5,xmm0
	lea	rbx,QWORD PTR[rbx*1+r11]
	cmovge	r11,rsp
	vaesdec	xmm6,xmm6,xmm0
	cmovg	rbx,rsp
	vaesdec	xmm7,xmm7,xmm0
	sub	rbx,r11
	vaesdec	xmm8,xmm8,xmm0
	vmovdqu	xmm13,XMMWORD PTR[16+r11]
	mov	QWORD PTR[((64+24))+rsp],rbx
	vaesdec	xmm9,xmm9,xmm0
	vmovups	xmm0,XMMWORD PTR[((-24))+rsi]
	lea	r11,QWORD PTR[16+rbx*1+r11]
	vmovdqu	XMMWORD PTR[176+rsp],xmm13
	vaesdec	xmm2,xmm2,xmm1
	cmp	ecx,DWORD PTR[((32+16))+rsp]
	mov	rbx,QWORD PTR[((64+32))+rsp]
	vaesdec	xmm3,xmm3,xmm1
	prefetcht0	[31+r12]
	vaesdec	xmm4,xmm4,xmm1
	prefetcht0	[15+r10]
	vaesdec	xmm5,xmm5,xmm1
	lea	rbx,QWORD PTR[rbx*1+r12]
	cmovge	r12,rsp
	vaesdec	xmm6,xmm6,xmm1
	cmovg	rbx,rsp
	vaesdec	xmm7,xmm7,xmm1
	sub	rbx,r12
	vaesdec	xmm8,xmm8,xmm1
	vmovdqu	xmm10,XMMWORD PTR[16+r12]
	mov	QWORD PTR[((64+32))+rsp],rbx
	vaesdec	xmm9,xmm9,xmm1
	vmovups	xmm1,XMMWORD PTR[((-8))+rsi]
	lea	r12,QWORD PTR[16+rbx*1+r12]
	vaesdec	xmm2,xmm2,xmm0
	cmp	ecx,DWORD PTR[((32+20))+rsp]
	mov	rbx,QWORD PTR[((64+40))+rsp]
	vaesdec	xmm3,xmm3,xmm0
	prefetcht0	[31+r13]
	vaesdec	xmm4,xmm4,xmm0
	prefetcht0	[15+r11]
	vaesdec	xmm5,xmm5,xmm0
	lea	rbx,QWORD PTR[r13*1+rbx]
	cmovge	r13,rsp
	vaesdec	xmm6,xmm6,xmm0
	cmovg	rbx,rsp
	vaesdec	xmm7,xmm7,xmm0
	sub	rbx,r13
	vaesdec	xmm8,xmm8,xmm0
	vmovdqu	xmm11,XMMWORD PTR[16+r13]
	mov	QWORD PTR[((64+40))+rsp],rbx
	vaesdec	xmm9,xmm9,xmm0
	vmovups	xmm0,XMMWORD PTR[8+rsi]
	lea	r13,QWORD PTR[16+rbx*1+r13]
	vaesdec	xmm2,xmm2,xmm1
	cmp	ecx,DWORD PTR[((32+24))+rsp]
	mov	rbx,QWORD PTR[((64+48))+rsp]
	vaesdec	xmm3,xmm3,xmm1
	prefetcht0	[31+r14]
	vaesdec	xmm4,xmm4,xmm1
	prefetcht0	[15+r12]
	vaesdec	xmm5,xmm5,xmm1
	lea	rbx,QWORD PTR[rbx*1+r14]
	cmovge	r14,rsp
	vaesdec	xmm6,xmm6,xmm1
	cmovg	rbx,rsp
	vaesdec	xmm7,xmm7,xmm1
	sub	rbx,r14
	vaesdec	xmm8,xmm8,xmm1
	vmovdqu	xmm12,XMMWORD PTR[16+r14]
	mov	QWORD PTR[((64+48))+rsp],rbx
	vaesdec	xmm9,xmm9,xmm1
	vmovups	xmm1,XMMWORD PTR[24+rsi]
	lea	r14,QWORD PTR[16+rbx*1+r14]
	vaesdec	xmm2,xmm2,xmm0
	cmp	ecx,DWORD PTR[((32+28))+rsp]
	mov	rbx,QWORD PTR[((64+56))+rsp]
	vaesdec	xmm3,xmm3,xmm0
	prefetcht0	[31+r15]
	vaesdec	xmm4,xmm4,xmm0
	prefetcht0	[15+r13]
	vaesdec	xmm5,xmm5,xmm0
	lea	rbx,QWORD PTR[rbx*1+r15]
	cmovge	r15,rsp
	vaesdec	xmm6,xmm6,xmm0
	cmovg	rbx,rsp
	vaesdec	xmm7,xmm7,xmm0
	sub	rbx,r15
	vaesdec	xmm8,xmm8,xmm0
	vmovdqu	xmm13,XMMWORD PTR[16+r15]
	mov	QWORD PTR[((64+56))+rsp],rbx
	vaesdec	xmm9,xmm9,xmm0
	vmovups	xmm0,XMMWORD PTR[40+rsi]
	lea	r15,QWORD PTR[16+rbx*1+r15]
	vmovdqu	xmm14,XMMWORD PTR[32+rsp]
	prefetcht0	[15+r14]
	prefetcht0	[15+r15]
	cmp	eax,11
	jb	$L$dec8x_tail

	vaesdec	xmm2,xmm2,xmm1
	vaesdec	xmm3,xmm3,xmm1
	vaesdec	xmm4,xmm4,xmm1
	vaesdec	xmm5,xmm5,xmm1
	vaesdec	xmm6,xmm6,xmm1
	vaesdec	xmm7,xmm7,xmm1
	vaesdec	xmm8,xmm8,xmm1
	vaesdec	xmm9,xmm9,xmm1
	vmovups	xmm1,XMMWORD PTR[((176-120))+rsi]

	vaesdec	xmm2,xmm2,xmm0
	vaesdec	xmm3,xmm3,xmm0
	vaesdec	xmm4,xmm4,xmm0
	vaesdec	xmm5,xmm5,xmm0
	vaesdec	xmm6,xmm6,xmm0
	vaesdec	xmm7,xmm7,xmm0
	vaesdec	xmm8,xmm8,xmm0
	vaesdec	xmm9,xmm9,xmm0
	vmovups	xmm0,XMMWORD PTR[((192-120))+rsi]
	je	$L$dec8x_tail

	vaesdec	xmm2,xmm2,xmm1
	vaesdec	xmm3,xmm3,xmm1
	vaesdec	xmm4,xmm4,xmm1
	vaesdec	xmm5,xmm5,xmm1
	vaesdec	xmm6,xmm6,xmm1
	vaesdec	xmm7,xmm7,xmm1
	vaesdec	xmm8,xmm8,xmm1
	vaesdec	xmm9,xmm9,xmm1
	vmovups	xmm1,XMMWORD PTR[((208-120))+rsi]

	vaesdec	xmm2,xmm2,xmm0
	vaesdec	xmm3,xmm3,xmm0
	vaesdec	xmm4,xmm4,xmm0
	vaesdec	xmm5,xmm5,xmm0
	vaesdec	xmm6,xmm6,xmm0
	vaesdec	xmm7,xmm7,xmm0
	vaesdec	xmm8,xmm8,xmm0
	vaesdec	xmm9,xmm9,xmm0
	vmovups	xmm0,XMMWORD PTR[((224-120))+rsi]

$L$dec8x_tail::
	vaesdec	xmm2,xmm2,xmm1
	vpxor	xmm15,xmm15,xmm15
	vaesdec	xmm3,xmm3,xmm1
	vaesdec	xmm4,xmm4,xmm1
	vpcmpgtd	xmm15,xmm14,xmm15
	vaesdec	xmm5,xmm5,xmm1
	vaesdec	xmm6,xmm6,xmm1
	vpaddd	xmm15,xmm15,xmm14
	vmovdqu	xmm14,XMMWORD PTR[48+rsp]
	vaesdec	xmm7,xmm7,xmm1
	mov	rbx,QWORD PTR[64+rsp]
	vaesdec	xmm8,xmm8,xmm1
	vaesdec	xmm9,xmm9,xmm1
	vmovups	xmm1,XMMWORD PTR[((16-120))+rsi]

	vaesdeclast	xmm2,xmm2,xmm0
	vmovdqa	XMMWORD PTR[32+rsp],xmm15
	vpxor	xmm15,xmm15,xmm15
	vaesdeclast	xmm3,xmm3,xmm0
	vpxor	xmm2,xmm2,XMMWORD PTR[rbp]
	vaesdeclast	xmm4,xmm4,xmm0
	vpxor	xmm3,xmm3,XMMWORD PTR[16+rbp]
	vpcmpgtd	xmm15,xmm14,xmm15
	vaesdeclast	xmm5,xmm5,xmm0
	vpxor	xmm4,xmm4,XMMWORD PTR[32+rbp]
	vaesdeclast	xmm6,xmm6,xmm0
	vpxor	xmm5,xmm5,XMMWORD PTR[48+rbp]
	vpaddd	xmm14,xmm14,xmm15
	vmovdqu	xmm15,XMMWORD PTR[((-120))+rsi]
	vaesdeclast	xmm7,xmm7,xmm0
	vpxor	xmm6,xmm6,XMMWORD PTR[64+rbp]
	vaesdeclast	xmm8,xmm8,xmm0
	vpxor	xmm7,xmm7,XMMWORD PTR[80+rbp]
	vmovdqa	XMMWORD PTR[48+rsp],xmm14
	vaesdeclast	xmm9,xmm9,xmm0
	vpxor	xmm8,xmm8,XMMWORD PTR[96+rbp]
	vmovups	xmm0,XMMWORD PTR[((32-120))+rsi]

	vmovups	XMMWORD PTR[(-16)+r8],xmm2
	sub	r8,rbx
	vmovdqu	xmm2,XMMWORD PTR[((128+0))+rsp]
	vpxor	xmm9,xmm9,XMMWORD PTR[112+rbp]
	vmovups	XMMWORD PTR[(-16)+r9],xmm3
	sub	r9,QWORD PTR[72+rsp]
	vmovdqu	XMMWORD PTR[rbp],xmm2
	vpxor	xmm2,xmm2,xmm15
	vmovdqu	xmm3,XMMWORD PTR[((128+16))+rsp]
	vmovups	XMMWORD PTR[(-16)+r10],xmm4
	sub	r10,QWORD PTR[80+rsp]
	vmovdqu	XMMWORD PTR[16+rbp],xmm3
	vpxor	xmm3,xmm3,xmm15
	vmovdqu	xmm4,XMMWORD PTR[((128+32))+rsp]
	vmovups	XMMWORD PTR[(-16)+r11],xmm5
	sub	r11,QWORD PTR[88+rsp]
	vmovdqu	XMMWORD PTR[32+rbp],xmm4
	vpxor	xmm4,xmm4,xmm15
	vmovdqu	xmm5,XMMWORD PTR[((128+48))+rsp]
	vmovups	XMMWORD PTR[(-16)+r12],xmm6
	sub	r12,QWORD PTR[96+rsp]
	vmovdqu	XMMWORD PTR[48+rbp],xmm5
	vpxor	xmm5,xmm5,xmm15
	vmovdqu	XMMWORD PTR[64+rbp],xmm10
	vpxor	xmm6,xmm15,xmm10
	vmovups	XMMWORD PTR[(-16)+r13],xmm7
	sub	r13,QWORD PTR[104+rsp]
	vmovdqu	XMMWORD PTR[80+rbp],xmm11
	vpxor	xmm7,xmm15,xmm11
	vmovups	XMMWORD PTR[(-16)+r14],xmm8
	sub	r14,QWORD PTR[112+rsp]
	vmovdqu	XMMWORD PTR[96+rbp],xmm12
	vpxor	xmm8,xmm15,xmm12
	vmovups	XMMWORD PTR[(-16)+r15],xmm9
	sub	r15,QWORD PTR[120+rsp]
	vmovdqu	XMMWORD PTR[112+rbp],xmm13
	vpxor	xmm9,xmm15,xmm13

	xor	rbp,128
	dec	edx
	jnz	$L$oop_dec8x

	mov	rax,QWORD PTR[16+rsp]






$L$dec8x_done::
	vzeroupper
	movaps	xmm6,XMMWORD PTR[((-216))+rax]
	movaps	xmm7,XMMWORD PTR[((-200))+rax]
	movaps	xmm8,XMMWORD PTR[((-184))+rax]
	movaps	xmm9,XMMWORD PTR[((-168))+rax]
	movaps	xmm10,XMMWORD PTR[((-152))+rax]
	movaps	xmm11,XMMWORD PTR[((-136))+rax]
	movaps	xmm12,XMMWORD PTR[((-120))+rax]
	movaps	xmm13,XMMWORD PTR[((-104))+rax]
	movaps	xmm14,XMMWORD PTR[((-88))+rax]
	movaps	xmm15,XMMWORD PTR[((-72))+rax]
	mov	r15,QWORD PTR[((-48))+rax]

	mov	r14,QWORD PTR[((-40))+rax]

	mov	r13,QWORD PTR[((-32))+rax]

	mov	r12,QWORD PTR[((-24))+rax]

	mov	rbp,QWORD PTR[((-16))+rax]

	mov	rbx,QWORD PTR[((-8))+rax]

	lea	rsp,QWORD PTR[rax]

$L$dec8x_epilogue::
	mov	rdi,QWORD PTR[8+rsp]	;WIN64 epilogue
	mov	rsi,QWORD PTR[16+rsp]
	DB	0F3h,0C3h		;repret

$L$SEH_end_aesni_multi_cbc_decrypt_avx::
aesni_multi_cbc_decrypt_avx	ENDP
EXTERN	__imp_RtlVirtualUnwind:NEAR

ALIGN	16
se_handler	PROC PRIVATE
	push	rsi
	push	rdi
	push	rbx
	push	rbp
	push	r12
	push	r13
	push	r14
	push	r15
	pushfq
	sub	rsp,64

	mov	rax,QWORD PTR[120+r8]
	mov	rbx,QWORD PTR[248+r8]

	mov	rsi,QWORD PTR[8+r9]
	mov	r11,QWORD PTR[56+r9]

	mov	r10d,DWORD PTR[r11]
	lea	r10,QWORD PTR[r10*1+rsi]
	cmp	rbx,r10
	jb	$L$in_prologue

	mov	rax,QWORD PTR[152+r8]

	mov	r10d,DWORD PTR[4+r11]
	lea	r10,QWORD PTR[r10*1+rsi]
	cmp	rbx,r10
	jae	$L$in_prologue

	mov	rax,QWORD PTR[16+rax]

	mov	rbx,QWORD PTR[((-8))+rax]
	mov	rbp,QWORD PTR[((-16))+rax]
	mov	r12,QWORD PTR[((-24))+rax]
	mov	r13,QWORD PTR[((-32))+rax]
	mov	r14,QWORD PTR[((-40))+rax]
	mov	r15,QWORD PTR[((-48))+rax]
	mov	QWORD PTR[144+r8],rbx
	mov	QWORD PTR[160+r8],rbp
	mov	QWORD PTR[216+r8],r12
	mov	QWORD PTR[224+r8],r13
	mov	QWORD PTR[232+r8],r14
	mov	QWORD PTR[240+r8],r15

	lea	rsi,QWORD PTR[((-56-160))+rax]
	lea	rdi,QWORD PTR[512+r8]
	mov	ecx,20
	DD	0a548f3fch

$L$in_prologue::
	mov	rdi,QWORD PTR[8+rax]
	mov	rsi,QWORD PTR[16+rax]
	mov	QWORD PTR[152+r8],rax
	mov	QWORD PTR[168+r8],rsi
	mov	QWORD PTR[176+r8],rdi

	mov	rdi,QWORD PTR[40+r9]
	mov	rsi,r8
	mov	ecx,154
	DD	0a548f3fch

	mov	rsi,r9
	xor	rcx,rcx
	mov	rdx,QWORD PTR[8+rsi]
	mov	r8,QWORD PTR[rsi]
	mov	r9,QWORD PTR[16+rsi]
	mov	r10,QWORD PTR[40+rsi]
	lea	r11,QWORD PTR[56+rsi]
	lea	r12,QWORD PTR[24+rsi]
	mov	QWORD PTR[32+rsp],r10
	mov	QWORD PTR[40+rsp],r11
	mov	QWORD PTR[48+rsp],r12
	mov	QWORD PTR[56+rsp],rcx
	call	QWORD PTR[__imp_RtlVirtualUnwind]

	mov	eax,1
	add	rsp,64
	popfq
	pop	r15
	pop	r14
	pop	r13
	pop	r12
	pop	rbp
	pop	rbx
	pop	rdi
	pop	rsi
	DB	0F3h,0C3h		;repret
se_handler	ENDP

.text$	ENDS
.pdata	SEGMENT READONLY ALIGN(4)
ALIGN	4
	DD	imagerel $L$SEH_begin_aesni_multi_cbc_encrypt
	DD	imagerel $L$SEH_end_aesni_multi_cbc_encrypt
	DD	imagerel $L$SEH_info_aesni_multi_cbc_encrypt
	DD	imagerel $L$SEH_begin_aesni_multi_cbc_decrypt
	DD	imagerel $L$SEH_end_aesni_multi_cbc_decrypt
	DD	imagerel $L$SEH_info_aesni_multi_cbc_decrypt
	DD	imagerel $L$SEH_begin_aesni_multi_cbc_encrypt_avx
	DD	imagerel $L$SEH_end_aesni_multi_cbc_encrypt_avx
	DD	imagerel $L$SEH_info_aesni_multi_cbc_encrypt_avx
	DD	imagerel $L$SEH_begin_aesni_multi_cbc_decrypt_avx
	DD	imagerel $L$SEH_end_aesni_multi_cbc_decrypt_avx
	DD	imagerel $L$SEH_info_aesni_multi_cbc_decrypt_avx
.pdata	ENDS
.xdata	SEGMENT READONLY ALIGN(8)
ALIGN	8
$L$SEH_info_aesni_multi_cbc_encrypt::
DB	9,0,0,0
	DD	imagerel se_handler
	DD	imagerel $L$enc4x_body,imagerel $L$enc4x_epilogue
$L$SEH_info_aesni_multi_cbc_decrypt::
DB	9,0,0,0
	DD	imagerel se_handler
	DD	imagerel $L$dec4x_body,imagerel $L$dec4x_epilogue
$L$SEH_info_aesni_multi_cbc_encrypt_avx::
DB	9,0,0,0
	DD	imagerel se_handler
	DD	imagerel $L$enc8x_body,imagerel $L$enc8x_epilogue
$L$SEH_info_aesni_multi_cbc_decrypt_avx::
DB	9,0,0,0
	DD	imagerel se_handler
	DD	imagerel $L$dec8x_body,imagerel $L$dec8x_epilogue

.xdata	ENDS
END
