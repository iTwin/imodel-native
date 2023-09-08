OPTION	DOTNAME
.text$	SEGMENT ALIGN(256) 'CODE'

PUBLIC	ossl_rsaz_avx512ifma_eligible

ossl_rsaz_avx512ifma_eligible	PROC PUBLIC
	xor	eax,eax
	DB	0F3h,0C3h		;repret
ossl_rsaz_avx512ifma_eligible	ENDP

PUBLIC	ossl_rsaz_amm52x20_x1_ifma256
PUBLIC	ossl_rsaz_amm52x20_x2_ifma256
PUBLIC	ossl_extract_multiplier_2x20_win5

ossl_rsaz_amm52x20_x1_ifma256	PROC PUBLIC
ossl_rsaz_amm52x20_x2_ifma256::
ossl_extract_multiplier_2x20_win5::
DB	00fh,00bh
	DB	0F3h,0C3h		;repret
ossl_rsaz_amm52x20_x1_ifma256	ENDP

.text$	ENDS
END
