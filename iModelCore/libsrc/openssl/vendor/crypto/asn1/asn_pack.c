/*
<<<<<<< HEAD
 * Copyright 1999-2016 The OpenSSL Project Authors. All Rights Reserved.
=======
 * Copyright 1999-2023 The OpenSSL Project Authors. All Rights Reserved.
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include "internal/cryptlib.h"
#include <openssl/asn1.h>

/* ASN1 packing and unpacking functions */

ASN1_STRING *ASN1_item_pack(void *obj, const ASN1_ITEM *it, ASN1_STRING **oct)
{
    ASN1_STRING *octmp;

     if (oct == NULL || *oct == NULL) {
        if ((octmp = ASN1_STRING_new()) == NULL) {
<<<<<<< HEAD
            ASN1err(ASN1_F_ASN1_ITEM_PACK, ERR_R_MALLOC_FAILURE);
=======
            ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
            return NULL;
        }
    } else {
        octmp = *oct;
    }

    OPENSSL_free(octmp->data);
    octmp->data = NULL;

<<<<<<< HEAD
    if ((octmp->length = ASN1_item_i2d(obj, &octmp->data, it)) == 0) {
        ASN1err(ASN1_F_ASN1_ITEM_PACK, ASN1_R_ENCODE_ERROR);
        goto err;
    }
    if (octmp->data == NULL) {
        ASN1err(ASN1_F_ASN1_ITEM_PACK, ERR_R_MALLOC_FAILURE);
=======
    if ((octmp->length = ASN1_item_i2d(obj, &octmp->data, it)) <= 0) {
        ERR_raise(ERR_LIB_ASN1, ASN1_R_ENCODE_ERROR);
        goto err;
    }
    if (octmp->data == NULL) {
        ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
        goto err;
    }

    if (oct != NULL && *oct == NULL)
        *oct = octmp;

    return octmp;
 err:
    if (oct == NULL || *oct == NULL)
        ASN1_STRING_free(octmp);
    return NULL;
}

/* Extract an ASN1 object from an ASN1_STRING */

void *ASN1_item_unpack(const ASN1_STRING *oct, const ASN1_ITEM *it)
{
    const unsigned char *p;
    void *ret;

    p = oct->data;
    if ((ret = ASN1_item_d2i(NULL, &p, oct->length, it)) == NULL)
<<<<<<< HEAD
        ASN1err(ASN1_F_ASN1_ITEM_UNPACK, ASN1_R_DECODE_ERROR);
=======
        ERR_raise(ERR_LIB_ASN1, ASN1_R_DECODE_ERROR);
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
    return ret;
}
