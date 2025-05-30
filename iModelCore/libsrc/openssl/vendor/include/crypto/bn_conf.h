/* WARNING: do not edit! */
/* Generated by makefile from include\crypto\bn_conf.h.in */
/*
 * Copyright 2016-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OSSL_CRYPTO_BN_CONF_H
# define OSSL_CRYPTO_BN_CONF_H
# pragma once

/*
 * The contents of this file are not used in the UEFI build, as
 * both 32-bit and 64-bit builds are supported from a single run
 * of the Configure script.
 */

/* Should we define BN_DIV2W here? */

/* Only one for the following should be defined */
#if defined (BENTLEY_WIN32)
    #  undef SIXTY_FOUR_BIT_LONG
    #  define SIXTY_FOUR_BIT
#else
    #  define SIXTY_FOUR_BIT_LONG
    #  undef SIXTY_FOUR_BIT
#endif
#undef THIRTY_TWO_BIT

#endif