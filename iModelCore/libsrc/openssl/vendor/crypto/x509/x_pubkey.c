/*
<<<<<<< HEAD
 * Copyright 1995-2020 The OpenSSL Project Authors. All Rights Reserved.
=======
 * Copyright 1995-2023 The OpenSSL Project Authors. All Rights Reserved.
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include "internal/cryptlib.h"
#include <openssl/asn1t.h>
#include <openssl/x509.h>
#include "crypto/asn1.h"
#include "crypto/evp.h"
#include "crypto/x509.h"
#include <openssl/rsa.h>
#include <openssl/dsa.h>

struct X509_pubkey_st {
    X509_ALGOR *algor;
    ASN1_BIT_STRING *public_key;
    EVP_PKEY *pkey;
};

static int x509_pubkey_decode(EVP_PKEY **pk, X509_PUBKEY *key);

/* Minor tweak to operation: free up EVP_PKEY */
static int pubkey_cb(int operation, ASN1_VALUE **pval, const ASN1_ITEM *it,
                     void *exarg)
{
    if (operation == ASN1_OP_FREE_POST) {
        X509_PUBKEY *pubkey = (X509_PUBKEY *)*pval;
        EVP_PKEY_free(pubkey->pkey);
    } else if (operation == ASN1_OP_D2I_POST) {
        /* Attempt to decode public key and cache in pubkey structure. */
        X509_PUBKEY *pubkey = (X509_PUBKEY *)*pval;
        EVP_PKEY_free(pubkey->pkey);
        pubkey->pkey = NULL;
        /*
         * Opportunistically decode the key but remove any non fatal errors
         * from the queue. Subsequent explicit attempts to decode/use the key
         * will return an appropriate error.
         */
        ERR_set_mark();
        if (x509_pubkey_decode(&pubkey->pkey, pubkey) == -1)
            return 0;
        ERR_pop_to_mark();
    }
    return 1;
}

ASN1_SEQUENCE_cb(X509_PUBKEY, pubkey_cb) = {
        ASN1_SIMPLE(X509_PUBKEY, algor, X509_ALGOR),
        ASN1_SIMPLE(X509_PUBKEY, public_key, ASN1_BIT_STRING)
} ASN1_SEQUENCE_END_cb(X509_PUBKEY, X509_PUBKEY)

<<<<<<< HEAD
IMPLEMENT_ASN1_FUNCTIONS(X509_PUBKEY)

=======
X509_PUBKEY *ossl_d2i_X509_PUBKEY_INTERNAL(const unsigned char **pp,
                                           long len, OSSL_LIB_CTX *libctx)
{
    X509_PUBKEY *xpub = OPENSSL_zalloc(sizeof(*xpub));

    if (xpub == NULL)
        return NULL;
    return (X509_PUBKEY *)ASN1_item_d2i_ex((ASN1_VALUE **)&xpub, pp, len,
                                           ASN1_ITEM_rptr(X509_PUBKEY_INTERNAL),
                                           libctx, NULL);
}

void ossl_X509_PUBKEY_INTERNAL_free(X509_PUBKEY *xpub)
{
    ASN1_item_free((ASN1_VALUE *)xpub, ASN1_ITEM_rptr(X509_PUBKEY_INTERNAL));
}

static void x509_pubkey_ex_free(ASN1_VALUE **pval, const ASN1_ITEM *it)
{
    X509_PUBKEY *pubkey;

    if (pval != NULL && (pubkey = (X509_PUBKEY *)*pval) != NULL) {
        X509_ALGOR_free(pubkey->algor);
        ASN1_BIT_STRING_free(pubkey->public_key);
        EVP_PKEY_free(pubkey->pkey);
        OPENSSL_free(pubkey->propq);
        OPENSSL_free(pubkey);
        *pval = NULL;
    }
}

static int x509_pubkey_ex_populate(ASN1_VALUE **pval, const ASN1_ITEM *it)
{
    X509_PUBKEY *pubkey = (X509_PUBKEY *)*pval;

    return (pubkey->algor != NULL
            || (pubkey->algor = X509_ALGOR_new()) != NULL)
        && (pubkey->public_key != NULL
            || (pubkey->public_key = ASN1_BIT_STRING_new()) != NULL);
}


static int x509_pubkey_ex_new_ex(ASN1_VALUE **pval, const ASN1_ITEM *it,
                                 OSSL_LIB_CTX *libctx, const char *propq)
{
    X509_PUBKEY *ret;

    if ((ret = OPENSSL_zalloc(sizeof(*ret))) == NULL
        || !x509_pubkey_ex_populate((ASN1_VALUE **)&ret, NULL)
        || !x509_pubkey_set0_libctx(ret, libctx, propq)) {
        x509_pubkey_ex_free((ASN1_VALUE **)&ret, NULL);
        ret = NULL;
        ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
    } else {
        *pval = (ASN1_VALUE *)ret;
    }

    return ret != NULL;
}

static int x509_pubkey_ex_d2i_ex(ASN1_VALUE **pval,
                                 const unsigned char **in, long len,
                                 const ASN1_ITEM *it, int tag, int aclass,
                                 char opt, ASN1_TLC *ctx, OSSL_LIB_CTX *libctx,
                                 const char *propq)
{
    const unsigned char *in_saved = *in;
    size_t publen;
    X509_PUBKEY *pubkey;
    int ret;
    OSSL_DECODER_CTX *dctx = NULL;
    unsigned char *tmpbuf = NULL;

    if (*pval == NULL && !x509_pubkey_ex_new_ex(pval, it, libctx, propq))
        return 0;
    if (!x509_pubkey_ex_populate(pval, NULL)) {
        ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
        return 0;
    }

    /* This ensures that |*in| advances properly no matter what */
    if ((ret = ASN1_item_ex_d2i(pval, in, len,
                                ASN1_ITEM_rptr(X509_PUBKEY_INTERNAL),
                                tag, aclass, opt, ctx)) <= 0)
        return ret;

    publen = *in - in_saved;
    if (!ossl_assert(publen > 0)) {
        ERR_raise(ERR_LIB_ASN1, ERR_R_INTERNAL_ERROR);
        return 0;
    }

    pubkey = (X509_PUBKEY *)*pval;
    EVP_PKEY_free(pubkey->pkey);
    pubkey->pkey = NULL;

    /*
     * Opportunistically decode the key but remove any non fatal errors
     * from the queue. Subsequent explicit attempts to decode/use the key
     * will return an appropriate error.
     */
    ERR_set_mark();

    /*
     * Try to decode with legacy method first.  This ensures that engines
     * aren't overridden by providers.
     */
    if ((ret = x509_pubkey_decode(&pubkey->pkey, pubkey)) == -1) {
        /* -1 indicates a fatal error, like malloc failure */
        ERR_clear_last_mark();
        goto end;
    }

    /* Try to decode it into an EVP_PKEY with OSSL_DECODER */
    if (ret <= 0 && !pubkey->flag_force_legacy) {
        const unsigned char *p;
        char txtoidname[OSSL_MAX_NAME_SIZE];
        size_t slen = publen;

        /*
        * The decoders don't know how to handle anything other than Universal
        * class so we modify the data accordingly.
        */
        if (aclass != V_ASN1_UNIVERSAL) {
            tmpbuf = OPENSSL_memdup(in_saved, publen);
            if (tmpbuf == NULL) {
                ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
                return 0;
            }
            in_saved = tmpbuf;
            *tmpbuf = V_ASN1_CONSTRUCTED | V_ASN1_SEQUENCE;
        }
        p = in_saved;

        if (OBJ_obj2txt(txtoidname, sizeof(txtoidname),
                        pubkey->algor->algorithm, 0) <= 0) {
            ERR_clear_last_mark();
            goto end;
        }
        if ((dctx =
             OSSL_DECODER_CTX_new_for_pkey(&pubkey->pkey,
                                           "DER", "SubjectPublicKeyInfo",
                                           txtoidname, EVP_PKEY_PUBLIC_KEY,
                                           pubkey->libctx,
                                           pubkey->propq)) != NULL)
            /*
             * As said higher up, we're being opportunistic.  In other words,
             * we don't care if we fail.
             */
            if (OSSL_DECODER_from_data(dctx, &p, &slen)) {
                if (slen != 0) {
                    /*
                     * If we successfully decoded then we *must* consume all the
                     * bytes.
                     */
                    ERR_clear_last_mark();
                    ERR_raise(ERR_LIB_ASN1, EVP_R_DECODE_ERROR);
                    goto end;
                }
            }
    }

    ERR_pop_to_mark();
    ret = 1;
 end:
    OSSL_DECODER_CTX_free(dctx);
    OPENSSL_free(tmpbuf);
    return ret;
}

static int x509_pubkey_ex_i2d(const ASN1_VALUE **pval, unsigned char **out,
                              const ASN1_ITEM *it, int tag, int aclass)
{
    return ASN1_item_ex_i2d(pval, out, ASN1_ITEM_rptr(X509_PUBKEY_INTERNAL),
                            tag, aclass);
}

static int x509_pubkey_ex_print(BIO *out, const ASN1_VALUE **pval, int indent,
                                const char *fname, const ASN1_PCTX *pctx)
{
    return ASN1_item_print(out, *pval, indent,
                           ASN1_ITEM_rptr(X509_PUBKEY_INTERNAL), pctx);
}

static const ASN1_EXTERN_FUNCS x509_pubkey_ff = {
    NULL,
    NULL,
    x509_pubkey_ex_free,
    0,                          /* Default clear behaviour is OK */
    NULL,
    x509_pubkey_ex_i2d,
    x509_pubkey_ex_print,
    x509_pubkey_ex_new_ex,
    x509_pubkey_ex_d2i_ex,
};

IMPLEMENT_EXTERN_ASN1(X509_PUBKEY, V_ASN1_SEQUENCE, x509_pubkey_ff)
IMPLEMENT_ASN1_FUNCTIONS(X509_PUBKEY)

X509_PUBKEY *X509_PUBKEY_new_ex(OSSL_LIB_CTX *libctx, const char *propq)
{
    X509_PUBKEY *pubkey = NULL;

    pubkey = (X509_PUBKEY *)ASN1_item_new_ex(X509_PUBKEY_it(), libctx, propq);
    if (!x509_pubkey_set0_libctx(pubkey, libctx, propq)) {
        X509_PUBKEY_free(pubkey);
        pubkey = NULL;
    }
    return pubkey;
}

/*
 * X509_PUBKEY_dup() must be implemented manually, because there is no
 * support for it in ASN1_EXTERN_FUNCS.
 */
X509_PUBKEY *X509_PUBKEY_dup(const X509_PUBKEY *a)
{
    X509_PUBKEY *pubkey = OPENSSL_zalloc(sizeof(*pubkey));

    if (pubkey == NULL
            || !x509_pubkey_set0_libctx(pubkey, a->libctx, a->propq)
            || (pubkey->algor = X509_ALGOR_dup(a->algor)) == NULL
            || (pubkey->public_key = ASN1_BIT_STRING_new()) == NULL
            || !ASN1_BIT_STRING_set(pubkey->public_key,
                                    a->public_key->data,
                                    a->public_key->length)) {
        x509_pubkey_ex_free((ASN1_VALUE **)&pubkey,
                            ASN1_ITEM_rptr(X509_PUBKEY_INTERNAL));
        ERR_raise(ERR_LIB_X509, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    if (a->pkey != NULL) {
        ERR_set_mark();
        pubkey->pkey = EVP_PKEY_dup(a->pkey);
        if (pubkey->pkey == NULL) {
            pubkey->flag_force_legacy = 1;
            if (x509_pubkey_decode(&pubkey->pkey, pubkey) <= 0) {
                x509_pubkey_ex_free((ASN1_VALUE **)&pubkey,
                                    ASN1_ITEM_rptr(X509_PUBKEY_INTERNAL));
                ERR_clear_last_mark();
                return NULL;
            }
        }
        ERR_pop_to_mark();
    }
    return pubkey;
}

>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
int X509_PUBKEY_set(X509_PUBKEY **x, EVP_PKEY *pkey)
{
    X509_PUBKEY *pk = NULL;

    if (x == NULL)
        return 0;

<<<<<<< HEAD
    if ((pk = X509_PUBKEY_new()) == NULL)
        goto error;

    if (pkey->ameth) {
        if (pkey->ameth->pub_encode) {
=======
    if (pkey->ameth != NULL) {
        if ((pk = X509_PUBKEY_new()) == NULL) {
            ERR_raise(ERR_LIB_X509, ERR_R_MALLOC_FAILURE);
            goto error;
        }
        if (pkey->ameth->pub_encode != NULL) {
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
            if (!pkey->ameth->pub_encode(pk, pkey)) {
                X509err(X509_F_X509_PUBKEY_SET,
                        X509_R_PUBLIC_KEY_ENCODE_ERROR);
                goto error;
            }
        } else {
            X509err(X509_F_X509_PUBKEY_SET, X509_R_METHOD_NOT_SUPPORTED);
            goto error;
        }
    } else {
        X509err(X509_F_X509_PUBKEY_SET, X509_R_UNSUPPORTED_ALGORITHM);
        goto error;
    }

    X509_PUBKEY_free(*x);
    *x = pk;
    pk->pkey = pkey;
    EVP_PKEY_up_ref(pkey);
    return 1;

 error:
    X509_PUBKEY_free(pk);
    return 0;
}

/*
 * Attempt to decode a public key.
 * Returns 1 on success, 0 for a decode failure and -1 for a fatal
 * error e.g. malloc failure.
 */


static int x509_pubkey_decode(EVP_PKEY **ppkey, X509_PUBKEY *key)
{
    EVP_PKEY *pkey = EVP_PKEY_new();

    if (pkey == NULL) {
<<<<<<< HEAD
        X509err(X509_F_X509_PUBKEY_DECODE, ERR_R_MALLOC_FAILURE);
=======
        ERR_raise(ERR_LIB_X509, ERR_R_MALLOC_FAILURE);
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
        return -1;
    }

    if (!EVP_PKEY_set_type(pkey, OBJ_obj2nid(key->algor->algorithm))) {
        X509err(X509_F_X509_PUBKEY_DECODE, X509_R_UNSUPPORTED_ALGORITHM);
        goto error;
    }

    if (pkey->ameth->pub_decode) {
        /*
         * Treat any failure of pub_decode as a decode error. In
         * future we could have different return codes for decode
         * errors and fatal errors such as malloc failure.
         */
        if (!pkey->ameth->pub_decode(pkey, key)) {
            X509err(X509_F_X509_PUBKEY_DECODE, X509_R_PUBLIC_KEY_DECODE_ERROR);
            goto error;
        }
    } else {
        X509err(X509_F_X509_PUBKEY_DECODE, X509_R_METHOD_NOT_SUPPORTED);
        goto error;
    }

    *ppkey = pkey;
    return 1;

 error:
    EVP_PKEY_free(pkey);
    return 0;
}

EVP_PKEY *X509_PUBKEY_get0(X509_PUBKEY *key)
{
    EVP_PKEY *ret = NULL;

    if (key == NULL || key->public_key == NULL)
        return NULL;

    if (key->pkey != NULL)
        return key->pkey;

    /*
     * When the key ASN.1 is initially parsed an attempt is made to
     * decode the public key and cache the EVP_PKEY structure. If this
     * operation fails the cached value will be NULL. Parsing continues
     * to allow parsing of unknown key types or unsupported forms.
     * We repeat the decode operation so the appropriate errors are left
     * in the queue.
     */
    x509_pubkey_decode(&ret, key);
    /* If decode doesn't fail something bad happened */
    if (ret != NULL) {
        X509err(X509_F_X509_PUBKEY_GET0, ERR_R_INTERNAL_ERROR);
        EVP_PKEY_free(ret);
    }

    return NULL;
}

EVP_PKEY *X509_PUBKEY_get(X509_PUBKEY *key)
{
    EVP_PKEY *ret = X509_PUBKEY_get0(key);

    if (ret != NULL && !EVP_PKEY_up_ref(ret)) {
        X509err(X509_F_X509_PUBKEY_GET, ERR_R_INTERNAL_ERROR);
        ret = NULL;
    }
    return ret;
}

/*
 * Now two pseudo ASN1 routines that take an EVP_PKEY structure and encode or
 * decode as X509_PUBKEY
 */
<<<<<<< HEAD
=======
static EVP_PKEY *d2i_PUBKEY_int(EVP_PKEY **a,
                                const unsigned char **pp, long length,
                                OSSL_LIB_CTX *libctx, const char *propq,
                                unsigned int force_legacy,
                                X509_PUBKEY *
                                (*d2i_x509_pubkey)(X509_PUBKEY **a,
                                                   const unsigned char **in,
                                                   long len))
{
    X509_PUBKEY *xpk, *xpk2 = NULL, **pxpk = NULL;
    EVP_PKEY *pktmp = NULL;
    const unsigned char *q;

    q = *pp;

    /*
     * If libctx or propq are non-NULL, we take advantage of the reuse
     * feature.  It's not generally recommended, but is safe enough for
     * newly created structures.
     */
    if (libctx != NULL || propq != NULL || force_legacy) {
        xpk2 = OPENSSL_zalloc(sizeof(*xpk2));
        if (xpk2 == NULL) {
            ERR_raise(ERR_LIB_X509, ERR_R_MALLOC_FAILURE);
            return NULL;
        }
        if (!x509_pubkey_set0_libctx(xpk2, libctx, propq))
            goto end;
        xpk2->flag_force_legacy = !!force_legacy;
        pxpk = &xpk2;
    }
    xpk = d2i_x509_pubkey(pxpk, &q, length);
    if (xpk == NULL)
        goto end;
    pktmp = X509_PUBKEY_get(xpk);
    X509_PUBKEY_free(xpk);
    xpk2 = NULL;                 /* We know that xpk == xpk2 */
    if (pktmp == NULL)
        goto end;
    *pp = q;
    if (a != NULL) {
        EVP_PKEY_free(*a);
        *a = pktmp;
    }
 end:
    X509_PUBKEY_free(xpk2);
    return pktmp;
}

/* For the algorithm specific d2i functions further down */
EVP_PKEY *ossl_d2i_PUBKEY_legacy(EVP_PKEY **a, const unsigned char **pp,
                                 long length)
{
    return d2i_PUBKEY_int(a, pp, length, NULL, NULL, 1, d2i_X509_PUBKEY);
}

EVP_PKEY *d2i_PUBKEY_ex(EVP_PKEY **a, const unsigned char **pp, long length,
                        OSSL_LIB_CTX *libctx, const char *propq)
{
    return d2i_PUBKEY_int(a, pp, length, libctx, propq, 0, d2i_X509_PUBKEY);
}
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))

EVP_PKEY *d2i_PUBKEY(EVP_PKEY **a, const unsigned char **pp, long length)
{
    X509_PUBKEY *xpk;
    EVP_PKEY *pktmp;
    const unsigned char *q;
    q = *pp;
    xpk = d2i_X509_PUBKEY(NULL, &q, length);
    if (!xpk)
        return NULL;
    pktmp = X509_PUBKEY_get(xpk);
    X509_PUBKEY_free(xpk);
    if (!pktmp)
        return NULL;
    *pp = q;
    if (a) {
        EVP_PKEY_free(*a);
        *a = pktmp;
    }
    return pktmp;
}

int i2d_PUBKEY(EVP_PKEY *a, unsigned char **pp)
{
    X509_PUBKEY *xpk = NULL;
    int ret;
    if (!a)
        return 0;
    if (!X509_PUBKEY_set(&xpk, a))
        return -1;
    ret = i2d_X509_PUBKEY(xpk, pp);
    X509_PUBKEY_free(xpk);
    return ret;
}

/*
 * The following are equivalents but which return RSA and DSA keys
 */
#ifndef OPENSSL_NO_RSA
RSA *d2i_RSA_PUBKEY(RSA **a, const unsigned char **pp, long length)
{
    EVP_PKEY *pkey;
    RSA *key;
    const unsigned char *q;
    q = *pp;
    pkey = d2i_PUBKEY(NULL, &q, length);
    if (!pkey)
        return NULL;
    key = EVP_PKEY_get1_RSA(pkey);
    EVP_PKEY_free(pkey);
    if (!key)
        return NULL;
    *pp = q;
    if (a) {
        RSA_free(*a);
        *a = key;
    }
    return key;
}

int i2d_RSA_PUBKEY(RSA *a, unsigned char **pp)
{
    EVP_PKEY *pktmp;
    int ret;
    if (!a)
        return 0;
    pktmp = EVP_PKEY_new();
    if (pktmp == NULL) {
<<<<<<< HEAD
        ASN1err(ASN1_F_I2D_RSA_PUBKEY, ERR_R_MALLOC_FAILURE);
=======
        ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
        return -1;
    }
    EVP_PKEY_set1_RSA(pktmp, a);
    ret = i2d_PUBKEY(pktmp, pp);
<<<<<<< HEAD
=======
    pktmp->pkey.ptr = NULL;
    EVP_PKEY_free(pktmp);
    return ret;
}

#ifndef OPENSSL_NO_DH
DH *ossl_d2i_DH_PUBKEY(DH **a, const unsigned char **pp, long length)
{
    EVP_PKEY *pkey;
    DH *key = NULL;
    const unsigned char *q;

    q = *pp;
    pkey = ossl_d2i_PUBKEY_legacy(NULL, &q, length);
    if (pkey == NULL)
        return NULL;
    if (EVP_PKEY_get_id(pkey) == EVP_PKEY_DH)
        key = EVP_PKEY_get1_DH(pkey);
    EVP_PKEY_free(pkey);
    if (key == NULL)
        return NULL;
    *pp = q;
    if (a != NULL) {
        DH_free(*a);
        *a = key;
    }
    return key;
}

int ossl_i2d_DH_PUBKEY(const DH *a, unsigned char **pp)
{
    EVP_PKEY *pktmp;
    int ret;
    if (!a)
        return 0;
    pktmp = EVP_PKEY_new();
    if (pktmp == NULL) {
        ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
        return -1;
    }
    (void)EVP_PKEY_assign_DH(pktmp, (DH *)a);
    ret = i2d_PUBKEY(pktmp, pp);
    pktmp->pkey.ptr = NULL;
    EVP_PKEY_free(pktmp);
    return ret;
}

DH *ossl_d2i_DHx_PUBKEY(DH **a, const unsigned char **pp, long length)
{
    EVP_PKEY *pkey;
    DH *key = NULL;
    const unsigned char *q;

    q = *pp;
    pkey = ossl_d2i_PUBKEY_legacy(NULL, &q, length);
    if (pkey == NULL)
        return NULL;
    if (EVP_PKEY_get_id(pkey) == EVP_PKEY_DHX)
        key = EVP_PKEY_get1_DH(pkey);
    EVP_PKEY_free(pkey);
    if (key == NULL)
        return NULL;
    *pp = q;
    if (a != NULL) {
        DH_free(*a);
        *a = key;
    }
    return key;
}

int ossl_i2d_DHx_PUBKEY(const DH *a, unsigned char **pp)
{
    EVP_PKEY *pktmp;
    int ret;
    if (!a)
        return 0;
    pktmp = EVP_PKEY_new();
    if (pktmp == NULL) {
        ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
        return -1;
    }
    (void)EVP_PKEY_assign(pktmp, EVP_PKEY_DHX, (DH *)a);
    ret = i2d_PUBKEY(pktmp, pp);
    pktmp->pkey.ptr = NULL;
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
    EVP_PKEY_free(pktmp);
    return ret;
}
#endif

#ifndef OPENSSL_NO_DSA
DSA *d2i_DSA_PUBKEY(DSA **a, const unsigned char **pp, long length)
{
    EVP_PKEY *pkey;
    DSA *key;
    const unsigned char *q;
    q = *pp;
    pkey = d2i_PUBKEY(NULL, &q, length);
    if (!pkey)
        return NULL;
    key = EVP_PKEY_get1_DSA(pkey);
    EVP_PKEY_free(pkey);
    if (!key)
        return NULL;
    *pp = q;
    if (a) {
        DSA_free(*a);
        *a = key;
    }
    return key;
}

int i2d_DSA_PUBKEY(DSA *a, unsigned char **pp)
{
    EVP_PKEY *pktmp;
    int ret;
    if (!a)
        return 0;
    pktmp = EVP_PKEY_new();
    if (pktmp == NULL) {
<<<<<<< HEAD
        ASN1err(ASN1_F_I2D_DSA_PUBKEY, ERR_R_MALLOC_FAILURE);
=======
        ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
        return -1;
    }
    EVP_PKEY_set1_DSA(pktmp, a);
    ret = i2d_PUBKEY(pktmp, pp);
    EVP_PKEY_free(pktmp);
    return ret;
}
#endif

#ifndef OPENSSL_NO_EC
EC_KEY *d2i_EC_PUBKEY(EC_KEY **a, const unsigned char **pp, long length)
{
    EVP_PKEY *pkey;
    EC_KEY *key;
    const unsigned char *q;
    q = *pp;
    pkey = d2i_PUBKEY(NULL, &q, length);
    if (!pkey)
        return NULL;
    key = EVP_PKEY_get1_EC_KEY(pkey);
    EVP_PKEY_free(pkey);
    if (!key)
        return NULL;
    *pp = q;
    if (a) {
        EC_KEY_free(*a);
        *a = key;
    }
    return key;
}

int i2d_EC_PUBKEY(EC_KEY *a, unsigned char **pp)
{
    EVP_PKEY *pktmp;
    int ret;
    if (!a)
        return 0;
    if ((pktmp = EVP_PKEY_new()) == NULL) {
<<<<<<< HEAD
        ASN1err(ASN1_F_I2D_EC_PUBKEY, ERR_R_MALLOC_FAILURE);
=======
        ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
        return -1;
    }
    EVP_PKEY_set1_EC_KEY(pktmp, a);
    ret = i2d_PUBKEY(pktmp, pp);
    EVP_PKEY_free(pktmp);
    return ret;
}
<<<<<<< HEAD
=======

ECX_KEY *ossl_d2i_ED25519_PUBKEY(ECX_KEY **a,
                                 const unsigned char **pp, long length)
{
    EVP_PKEY *pkey;
    ECX_KEY *key = NULL;
    const unsigned char *q;

    q = *pp;
    pkey = ossl_d2i_PUBKEY_legacy(NULL, &q, length);
    if (pkey == NULL)
        return NULL;
    key = ossl_evp_pkey_get1_ED25519(pkey);
    EVP_PKEY_free(pkey);
    if (key == NULL)
        return NULL;
    *pp = q;
    if (a != NULL) {
        ossl_ecx_key_free(*a);
        *a = key;
    }
    return key;
}

int ossl_i2d_ED25519_PUBKEY(const ECX_KEY *a, unsigned char **pp)
{
    EVP_PKEY *pktmp;
    int ret;

    if (a == NULL)
        return 0;
    if ((pktmp = EVP_PKEY_new()) == NULL) {
        ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
        return -1;
    }
    (void)EVP_PKEY_assign(pktmp, EVP_PKEY_ED25519, (ECX_KEY *)a);
    ret = i2d_PUBKEY(pktmp, pp);
    pktmp->pkey.ptr = NULL;
    EVP_PKEY_free(pktmp);
    return ret;
}

ECX_KEY *ossl_d2i_ED448_PUBKEY(ECX_KEY **a,
                               const unsigned char **pp, long length)
{
    EVP_PKEY *pkey;
    ECX_KEY *key = NULL;
    const unsigned char *q;

    q = *pp;
    pkey = ossl_d2i_PUBKEY_legacy(NULL, &q, length);
    if (pkey == NULL)
        return NULL;
    if (EVP_PKEY_get_id(pkey) == EVP_PKEY_ED448)
        key = ossl_evp_pkey_get1_ED448(pkey);
    EVP_PKEY_free(pkey);
    if (key == NULL)
        return NULL;
    *pp = q;
    if (a != NULL) {
        ossl_ecx_key_free(*a);
        *a = key;
    }
    return key;
}

int ossl_i2d_ED448_PUBKEY(const ECX_KEY *a, unsigned char **pp)
{
    EVP_PKEY *pktmp;
    int ret;

    if (a == NULL)
        return 0;
    if ((pktmp = EVP_PKEY_new()) == NULL) {
        ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
        return -1;
    }
    (void)EVP_PKEY_assign(pktmp, EVP_PKEY_ED448, (ECX_KEY *)a);
    ret = i2d_PUBKEY(pktmp, pp);
    pktmp->pkey.ptr = NULL;
    EVP_PKEY_free(pktmp);
    return ret;
}

ECX_KEY *ossl_d2i_X25519_PUBKEY(ECX_KEY **a,
                                const unsigned char **pp, long length)
{
    EVP_PKEY *pkey;
    ECX_KEY *key = NULL;
    const unsigned char *q;

    q = *pp;
    pkey = ossl_d2i_PUBKEY_legacy(NULL, &q, length);
    if (pkey == NULL)
        return NULL;
    if (EVP_PKEY_get_id(pkey) == EVP_PKEY_X25519)
        key = ossl_evp_pkey_get1_X25519(pkey);
    EVP_PKEY_free(pkey);
    if (key == NULL)
        return NULL;
    *pp = q;
    if (a != NULL) {
        ossl_ecx_key_free(*a);
        *a = key;
    }
    return key;
}

int ossl_i2d_X25519_PUBKEY(const ECX_KEY *a, unsigned char **pp)
{
    EVP_PKEY *pktmp;
    int ret;

    if (a == NULL)
        return 0;
    if ((pktmp = EVP_PKEY_new()) == NULL) {
        ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
        return -1;
    }
    (void)EVP_PKEY_assign(pktmp, EVP_PKEY_X25519, (ECX_KEY *)a);
    ret = i2d_PUBKEY(pktmp, pp);
    pktmp->pkey.ptr = NULL;
    EVP_PKEY_free(pktmp);
    return ret;
}

ECX_KEY *ossl_d2i_X448_PUBKEY(ECX_KEY **a,
                              const unsigned char **pp, long length)
{
    EVP_PKEY *pkey;
    ECX_KEY *key = NULL;
    const unsigned char *q;

    q = *pp;
    pkey = ossl_d2i_PUBKEY_legacy(NULL, &q, length);
    if (pkey == NULL)
        return NULL;
    if (EVP_PKEY_get_id(pkey) == EVP_PKEY_X448)
        key = ossl_evp_pkey_get1_X448(pkey);
    EVP_PKEY_free(pkey);
    if (key == NULL)
        return NULL;
    *pp = q;
    if (a != NULL) {
        ossl_ecx_key_free(*a);
        *a = key;
    }
    return key;
}

int ossl_i2d_X448_PUBKEY(const ECX_KEY *a, unsigned char **pp)
{
    EVP_PKEY *pktmp;
    int ret;

    if (a == NULL)
        return 0;
    if ((pktmp = EVP_PKEY_new()) == NULL) {
        ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
        return -1;
    }
    (void)EVP_PKEY_assign(pktmp, EVP_PKEY_X448, (ECX_KEY *)a);
    ret = i2d_PUBKEY(pktmp, pp);
    pktmp->pkey.ptr = NULL;
    EVP_PKEY_free(pktmp);
    return ret;
}

>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
#endif

int X509_PUBKEY_set0_param(X509_PUBKEY *pub, ASN1_OBJECT *aobj,
                           int ptype, void *pval,
                           unsigned char *penc, int penclen)
{
    if (!X509_ALGOR_set0(pub->algor, aobj, ptype, pval))
        return 0;
    if (penc) {
        OPENSSL_free(pub->public_key->data);
        pub->public_key->data = penc;
        pub->public_key->length = penclen;
        /* Set number of unused bits to zero */
        pub->public_key->flags &= ~(ASN1_STRING_FLAG_BITS_LEFT | 0x07);
        pub->public_key->flags |= ASN1_STRING_FLAG_BITS_LEFT;
    }
    return 1;
}

int X509_PUBKEY_get0_param(ASN1_OBJECT **ppkalg,
                           const unsigned char **pk, int *ppklen,
                           X509_ALGOR **pa, X509_PUBKEY *pub)
{
    if (ppkalg)
        *ppkalg = pub->algor->algorithm;
    if (pk) {
        *pk = pub->public_key->data;
        *ppklen = pub->public_key->length;
    }
    if (pa)
        *pa = pub->algor;
    return 1;
}

ASN1_BIT_STRING *X509_get0_pubkey_bitstr(const X509 *x)
{
    if (x == NULL)
        return NULL;
    return x->cert_info.key->public_key;
}
