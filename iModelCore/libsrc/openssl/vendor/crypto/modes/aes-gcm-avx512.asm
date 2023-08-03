OPTION	DOTNAME
.text$	SEGMENT ALIGN(256) 'CODE'
PUBLIC	ossl_vaes_vpclmulqdq_capable

ossl_vaes_vpclmulqdq_capable	PROC PUBLIC
	xor	eax,eax
	DB	0F3h,0C3h		;repret
ossl_vaes_vpclmulqdq_capable	ENDP

PUBLIC	ossl_aes_gcm_init_avx512
PUBLIC	ossl_aes_gcm_setiv_avx512
PUBLIC	ossl_aes_gcm_update_aad_avx512
PUBLIC	ossl_aes_gcm_encrypt_avx512
PUBLIC	ossl_aes_gcm_decrypt_avx512
PUBLIC	ossl_aes_gcm_finalize_avx512
PUBLIC	ossl_gcm_gmult_avx512


ossl_aes_gcm_init_avx512	PROC PUBLIC
ossl_aes_gcm_setiv_avx512::
ossl_aes_gcm_update_aad_avx512::
ossl_aes_gcm_encrypt_avx512::
ossl_aes_gcm_decrypt_avx512::
ossl_aes_gcm_finalize_avx512::
ossl_gcm_gmult_avx512::
DB	00fh,00bh
	DB	0F3h,0C3h		;repret
ossl_aes_gcm_init_avx512	ENDP

.text$	ENDS
END
