/*
<<<<<<< HEAD
 * Copyright 1995-2021 The OpenSSL Project Authors. All Rights Reserved.
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
#include "internal/refcount.h"
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/objects.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/rsa.h>
#include <openssl/dsa.h>
#include <openssl/dh.h>
#include <openssl/cmac.h>
#include <openssl/engine.h>

#include "crypto/asn1.h"
#include "crypto/evp.h"

static void EVP_PKEY_free_it(EVP_PKEY *x);

int EVP_PKEY_bits(const EVP_PKEY *pkey)
{
    if (pkey && pkey->ameth && pkey->ameth->pkey_bits)
        return pkey->ameth->pkey_bits(pkey);
    return 0;
}

int EVP_PKEY_security_bits(const EVP_PKEY *pkey)
{
    if (pkey == NULL)
        return 0;
    if (!pkey->ameth || !pkey->ameth->pkey_security_bits)
        return -2;
    return pkey->ameth->pkey_security_bits(pkey);
}

int EVP_PKEY_size(const EVP_PKEY *pkey)
{
    if (pkey && pkey->ameth && pkey->ameth->pkey_size)
        return pkey->ameth->pkey_size(pkey);
    return 0;
}

int EVP_PKEY_save_parameters(EVP_PKEY *pkey, int mode)
{
#ifndef OPENSSL_NO_DSA
    if (pkey->type == EVP_PKEY_DSA) {
        int ret = pkey->save_parameters;

        if (mode >= 0)
            pkey->save_parameters = mode;
        return ret;
    }
#endif
#ifndef OPENSSL_NO_EC
    if (pkey->type == EVP_PKEY_EC) {
        int ret = pkey->save_parameters;

        if (mode >= 0)
            pkey->save_parameters = mode;
        return ret;
    }
#endif
    return 0;
}

int EVP_PKEY_copy_parameters(EVP_PKEY *to, const EVP_PKEY *from)
{
    if (to->type == EVP_PKEY_NONE) {
        if (EVP_PKEY_set_type(to, from->type) == 0)
            return 0;
    } else if (to->type != from->type) {
        EVPerr(EVP_F_EVP_PKEY_COPY_PARAMETERS, EVP_R_DIFFERENT_KEY_TYPES);
        goto err;
    }

    if (EVP_PKEY_missing_parameters(from)) {
        EVPerr(EVP_F_EVP_PKEY_COPY_PARAMETERS, EVP_R_MISSING_PARAMETERS);
        goto err;
    }

    if (!EVP_PKEY_missing_parameters(to)) {
        if (EVP_PKEY_cmp_parameters(to, from) == 1)
            return 1;
        EVPerr(EVP_F_EVP_PKEY_COPY_PARAMETERS, EVP_R_DIFFERENT_PARAMETERS);
        return 0;
    }

    if (from->ameth && from->ameth->param_copy)
        return from->ameth->param_copy(to, from);
 err:
    return 0;
}

int EVP_PKEY_missing_parameters(const EVP_PKEY *pkey)
{
    if (pkey != NULL && pkey->ameth && pkey->ameth->param_missing)
        return pkey->ameth->param_missing(pkey);
    return 0;
}

int EVP_PKEY_cmp_parameters(const EVP_PKEY *a, const EVP_PKEY *b)
{
    if (a->type != b->type)
        return -1;
    if (a->ameth && a->ameth->param_cmp)
        return a->ameth->param_cmp(a, b);
    return -2;
}

int EVP_PKEY_cmp(const EVP_PKEY *a, const EVP_PKEY *b)
{
    if (a->type != b->type)
        return -1;

    if (a->ameth) {
        int ret;
        /* Compare parameters if the algorithm has them */
        if (a->ameth->param_cmp) {
            ret = a->ameth->param_cmp(a, b);
            if (ret <= 0)
                return ret;
        }

        if (a->ameth->pub_cmp)
            return a->ameth->pub_cmp(a, b);
    }

    return -2;
}

<<<<<<< HEAD
=======

static EVP_PKEY *new_raw_key_int(OSSL_LIB_CTX *libctx,
                                 const char *strtype,
                                 const char *propq,
                                 int nidtype,
                                 ENGINE *e,
                                 const unsigned char *key,
                                 size_t len,
                                 int key_is_priv)
{
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    const EVP_PKEY_ASN1_METHOD *ameth = NULL;
    int result = 0;

# ifndef OPENSSL_NO_ENGINE
    /* Check if there is an Engine for this type */
    if (e == NULL) {
        ENGINE *tmpe = NULL;

        if (strtype != NULL)
            ameth = EVP_PKEY_asn1_find_str(&tmpe, strtype, -1);
        else if (nidtype != EVP_PKEY_NONE)
            ameth = EVP_PKEY_asn1_find(&tmpe, nidtype);

        /* If tmpe is NULL then no engine is claiming to support this type */
        if (tmpe == NULL)
            ameth = NULL;

        ENGINE_finish(tmpe);
    }
# endif

    if (e == NULL && ameth == NULL) {
        /*
         * No engine is claiming to support this type, so lets see if we have
         * a provider.
         */
        ctx = EVP_PKEY_CTX_new_from_name(libctx,
                                         strtype != NULL ? strtype
                                                         : OBJ_nid2sn(nidtype),
                                         propq);
        if (ctx == NULL)
            goto err;
        /* May fail if no provider available */
        ERR_set_mark();
        if (EVP_PKEY_fromdata_init(ctx) == 1) {
            OSSL_PARAM params[] = { OSSL_PARAM_END, OSSL_PARAM_END };

            ERR_clear_last_mark();
            params[0] = OSSL_PARAM_construct_octet_string(
                            key_is_priv ? OSSL_PKEY_PARAM_PRIV_KEY
                                        : OSSL_PKEY_PARAM_PUB_KEY,
                            (void *)key, len);

            if (EVP_PKEY_fromdata(ctx, &pkey, EVP_PKEY_KEYPAIR, params) != 1) {
                ERR_raise(ERR_LIB_EVP, EVP_R_KEY_SETUP_FAILED);
                goto err;
            }

            EVP_PKEY_CTX_free(ctx);

            return pkey;
        }
        ERR_pop_to_mark();
        /* else not supported so fallback to legacy */
    }

    /* Legacy code path */

    pkey = EVP_PKEY_new();
    if (pkey == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    if (!pkey_set_type(pkey, e, nidtype, strtype, -1, NULL)) {
        /* ERR_raise(ERR_LIB_EVP, ...) already called */
        goto err;
    }

    if (!ossl_assert(pkey->ameth != NULL))
        goto err;

    if (key_is_priv) {
        if (pkey->ameth->set_priv_key == NULL) {
            ERR_raise(ERR_LIB_EVP, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
            goto err;
        }

        if (!pkey->ameth->set_priv_key(pkey, key, len)) {
            ERR_raise(ERR_LIB_EVP, EVP_R_KEY_SETUP_FAILED);
            goto err;
        }
    } else {
        if (pkey->ameth->set_pub_key == NULL) {
            ERR_raise(ERR_LIB_EVP, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
            goto err;
        }

        if (!pkey->ameth->set_pub_key(pkey, key, len)) {
            ERR_raise(ERR_LIB_EVP, EVP_R_KEY_SETUP_FAILED);
            goto err;
        }
    }

    result = 1;
 err:
    if (!result) {
        EVP_PKEY_free(pkey);
        pkey = NULL;
    }
    EVP_PKEY_CTX_free(ctx);
    return pkey;
}

EVP_PKEY *EVP_PKEY_new_raw_private_key_ex(OSSL_LIB_CTX *libctx,
                                          const char *keytype,
                                          const char *propq,
                                          const unsigned char *priv, size_t len)
{
    return new_raw_key_int(libctx, keytype, propq, EVP_PKEY_NONE, NULL, priv,
                           len, 1);
}

EVP_PKEY *EVP_PKEY_new_raw_private_key(int type, ENGINE *e,
                                       const unsigned char *priv,
                                       size_t len)
{
    return new_raw_key_int(NULL, NULL, NULL, type, e, priv, len, 1);
}

EVP_PKEY *EVP_PKEY_new_raw_public_key_ex(OSSL_LIB_CTX *libctx,
                                         const char *keytype, const char *propq,
                                         const unsigned char *pub, size_t len)
{
    return new_raw_key_int(libctx, keytype, propq, EVP_PKEY_NONE, NULL, pub,
                           len, 0);
}

EVP_PKEY *EVP_PKEY_new_raw_public_key(int type, ENGINE *e,
                                      const unsigned char *pub,
                                      size_t len)
{
    return new_raw_key_int(NULL, NULL, NULL, type, e, pub, len, 0);
}

struct raw_key_details_st
{
    unsigned char **key;
    size_t *len;
    int selection;
};

static OSSL_CALLBACK get_raw_key_details;
static int get_raw_key_details(const OSSL_PARAM params[], void *arg)
{
    const OSSL_PARAM *p = NULL;
    struct raw_key_details_st *raw_key = arg;

    if (raw_key->selection == OSSL_KEYMGMT_SELECT_PRIVATE_KEY) {
        if ((p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_PRIV_KEY))
                != NULL)
            return OSSL_PARAM_get_octet_string(p, (void **)raw_key->key,
                                               raw_key->key == NULL ? 0 : *raw_key->len,
                                               raw_key->len);
    } else if (raw_key->selection == OSSL_KEYMGMT_SELECT_PUBLIC_KEY) {
        if ((p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_PUB_KEY))
                != NULL)
            return OSSL_PARAM_get_octet_string(p, (void **)raw_key->key,
                                               raw_key->key == NULL ? 0 : *raw_key->len,
                                               raw_key->len);
    }

    return 0;
}

int EVP_PKEY_get_raw_private_key(const EVP_PKEY *pkey, unsigned char *priv,
                                 size_t *len)
{
    if (pkey->keymgmt != NULL) {
        struct raw_key_details_st raw_key;

        raw_key.key = priv == NULL ? NULL : &priv;
        raw_key.len = len;
        raw_key.selection = OSSL_KEYMGMT_SELECT_PRIVATE_KEY;

        return evp_keymgmt_util_export(pkey, OSSL_KEYMGMT_SELECT_PRIVATE_KEY,
                                       get_raw_key_details, &raw_key);
    }

    if (pkey->ameth == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return 0;
    }

    if (pkey->ameth->get_priv_key == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return 0;
    }

    if (!pkey->ameth->get_priv_key(pkey, priv, len)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_GET_RAW_KEY_FAILED);
        return 0;
    }

    return 1;
}

int EVP_PKEY_get_raw_public_key(const EVP_PKEY *pkey, unsigned char *pub,
                                size_t *len)
{
    if (pkey->keymgmt != NULL) {
        struct raw_key_details_st raw_key;

        raw_key.key = pub == NULL ? NULL : &pub;
        raw_key.len = len;
        raw_key.selection = OSSL_KEYMGMT_SELECT_PUBLIC_KEY;

        return evp_keymgmt_util_export(pkey, OSSL_KEYMGMT_SELECT_PUBLIC_KEY,
                                       get_raw_key_details, &raw_key);
    }

    if (pkey->ameth == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return 0;
    }

     if (pkey->ameth->get_pub_key == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return 0;
    }

    if (!pkey->ameth->get_pub_key(pkey, pub, len)) {
        ERR_raise(ERR_LIB_EVP, EVP_R_GET_RAW_KEY_FAILED);
        return 0;
    }

    return 1;
}

static EVP_PKEY *new_cmac_key_int(const unsigned char *priv, size_t len,
                                  const char *cipher_name,
                                  const EVP_CIPHER *cipher,
                                  OSSL_LIB_CTX *libctx,
                                  const char *propq, ENGINE *e)
{
# ifndef OPENSSL_NO_CMAC
#  ifndef OPENSSL_NO_ENGINE
    const char *engine_id = e != NULL ? ENGINE_get_id(e) : NULL;
#  endif
    OSSL_PARAM params[5], *p = params;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx;

    if (cipher != NULL)
        cipher_name = EVP_CIPHER_get0_name(cipher);

    if (cipher_name == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_KEY_SETUP_FAILED);
        return NULL;
    }

    ctx = EVP_PKEY_CTX_new_from_name(libctx, "CMAC", propq);
    if (ctx == NULL)
        goto err;

    if (EVP_PKEY_fromdata_init(ctx) <= 0) {
        ERR_raise(ERR_LIB_EVP, EVP_R_KEY_SETUP_FAILED);
        goto err;
    }

    *p++ = OSSL_PARAM_construct_octet_string(OSSL_PKEY_PARAM_PRIV_KEY,
                                            (void *)priv, len);
    *p++ = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_CIPHER,
                                            (char *)cipher_name, 0);
    if (propq != NULL)
        *p++ = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_PROPERTIES,
                                                (char *)propq, 0);
#  ifndef OPENSSL_NO_ENGINE
    if (engine_id != NULL)
        *p++ = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_ENGINE,
                                                (char *)engine_id, 0);
#  endif
    *p = OSSL_PARAM_construct_end();

    if (EVP_PKEY_fromdata(ctx, &pkey, EVP_PKEY_KEYPAIR, params) <= 0) {
        ERR_raise(ERR_LIB_EVP, EVP_R_KEY_SETUP_FAILED);
        goto err;
    }

 err:
    EVP_PKEY_CTX_free(ctx);

    return pkey;
# else
    ERR_raise(ERR_LIB_EVP, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
    return NULL;
# endif
}

EVP_PKEY *EVP_PKEY_new_CMAC_key(ENGINE *e, const unsigned char *priv,
                                size_t len, const EVP_CIPHER *cipher)
{
    return new_cmac_key_int(priv, len, NULL, cipher, NULL, NULL, e);
}

int EVP_PKEY_set_type(EVP_PKEY *pkey, int type)
{
    return pkey_set_type(pkey, NULL, type, NULL, -1, NULL);
}

int EVP_PKEY_set_type_str(EVP_PKEY *pkey, const char *str, int len)
{
    return pkey_set_type(pkey, NULL, EVP_PKEY_NONE, str, len, NULL);
}

# ifndef OPENSSL_NO_ENGINE
int EVP_PKEY_set1_engine(EVP_PKEY *pkey, ENGINE *e)
{
    if (e != NULL) {
        if (!ENGINE_init(e)) {
            ERR_raise(ERR_LIB_EVP, ERR_R_ENGINE_LIB);
            return 0;
        }
        if (ENGINE_get_pkey_meth(e, pkey->type) == NULL) {
            ENGINE_finish(e);
            ERR_raise(ERR_LIB_EVP, EVP_R_UNSUPPORTED_ALGORITHM);
            return 0;
        }
    }
    ENGINE_finish(pkey->pmeth_engine);
    pkey->pmeth_engine = e;
    return 1;
}

ENGINE *EVP_PKEY_get0_engine(const EVP_PKEY *pkey)
{
    return pkey->engine;
}
# endif

# ifndef OPENSSL_NO_DEPRECATED_3_0
static void detect_foreign_key(EVP_PKEY *pkey)
{
    switch (pkey->type) {
    case EVP_PKEY_RSA:
        pkey->foreign = pkey->pkey.rsa != NULL
                        && ossl_rsa_is_foreign(pkey->pkey.rsa);
        break;
#  ifndef OPENSSL_NO_EC
    case EVP_PKEY_SM2:
    case EVP_PKEY_EC:
        pkey->foreign = pkey->pkey.ec != NULL
                        && ossl_ec_key_is_foreign(pkey->pkey.ec);
        break;
#  endif
#  ifndef OPENSSL_NO_DSA
    case EVP_PKEY_DSA:
        pkey->foreign = pkey->pkey.dsa != NULL
                        && ossl_dsa_is_foreign(pkey->pkey.dsa);
        break;
#endif
#  ifndef OPENSSL_NO_DH
    case EVP_PKEY_DH:
        pkey->foreign = pkey->pkey.dh != NULL
                        && ossl_dh_is_foreign(pkey->pkey.dh);
        break;
#endif
    default:
        pkey->foreign = 0;
        break;
    }
}

int EVP_PKEY_assign(EVP_PKEY *pkey, int type, void *key)
{
#  ifndef OPENSSL_NO_EC
    int pktype;

    pktype = EVP_PKEY_type(type);
    if ((key != NULL) && (pktype == EVP_PKEY_EC || pktype == EVP_PKEY_SM2)) {
        const EC_GROUP *group = EC_KEY_get0_group(key);

        if (group != NULL) {
            int curve = EC_GROUP_get_curve_name(group);

            /*
             * Regardless of what is requested the SM2 curve must be SM2 type,
             * and non SM2 curves are EC type.
             */
            if (curve == NID_sm2 && pktype == EVP_PKEY_EC)
                type = EVP_PKEY_SM2;
            else if(curve != NID_sm2 && pktype == EVP_PKEY_SM2)
                type = EVP_PKEY_EC;
        }
    }
#  endif

    if (pkey == NULL || !EVP_PKEY_set_type(pkey, type))
        return 0;

    pkey->pkey.ptr = key;
    detect_foreign_key(pkey);

    return (key != NULL);
}
# endif

void *EVP_PKEY_get0(const EVP_PKEY *pkey)
{
    if (pkey == NULL)
        return NULL;

    if (!evp_pkey_is_provided(pkey))
        return pkey->pkey.ptr;

    return NULL;
}

const unsigned char *EVP_PKEY_get0_hmac(const EVP_PKEY *pkey, size_t *len)
{
    const ASN1_OCTET_STRING *os = NULL;
    if (pkey->type != EVP_PKEY_HMAC) {
        ERR_raise(ERR_LIB_EVP, EVP_R_EXPECTING_AN_HMAC_KEY);
        return NULL;
    }
    os = evp_pkey_get_legacy((EVP_PKEY *)pkey);
    if (os != NULL) {
        *len = os->length;
        return os->data;
    }
    return NULL;
}

# ifndef OPENSSL_NO_POLY1305
const unsigned char *EVP_PKEY_get0_poly1305(const EVP_PKEY *pkey, size_t *len)
{
    const ASN1_OCTET_STRING *os = NULL;
    if (pkey->type != EVP_PKEY_POLY1305) {
        ERR_raise(ERR_LIB_EVP, EVP_R_EXPECTING_A_POLY1305_KEY);
        return NULL;
    }
    os = evp_pkey_get_legacy((EVP_PKEY *)pkey);
    if (os != NULL) {
        *len = os->length;
        return os->data;
    }
    return NULL;
}
# endif

# ifndef OPENSSL_NO_SIPHASH
const unsigned char *EVP_PKEY_get0_siphash(const EVP_PKEY *pkey, size_t *len)
{
    const ASN1_OCTET_STRING *os = NULL;

    if (pkey->type != EVP_PKEY_SIPHASH) {
        ERR_raise(ERR_LIB_EVP, EVP_R_EXPECTING_A_SIPHASH_KEY);
        return NULL;
    }
    os = evp_pkey_get_legacy((EVP_PKEY *)pkey);
    if (os != NULL) {
        *len = os->length;
        return os->data;
    }
    return NULL;
}
# endif

# ifndef OPENSSL_NO_DSA
static DSA *evp_pkey_get0_DSA_int(const EVP_PKEY *pkey)
{
    if (pkey->type != EVP_PKEY_DSA) {
        ERR_raise(ERR_LIB_EVP, EVP_R_EXPECTING_A_DSA_KEY);
        return NULL;
    }
    return evp_pkey_get_legacy((EVP_PKEY *)pkey);
}

const DSA *EVP_PKEY_get0_DSA(const EVP_PKEY *pkey)
{
    return evp_pkey_get0_DSA_int(pkey);
}

int EVP_PKEY_set1_DSA(EVP_PKEY *pkey, DSA *key)
{
    int ret = EVP_PKEY_assign_DSA(pkey, key);
    if (ret)
        DSA_up_ref(key);
    return ret;
}
DSA *EVP_PKEY_get1_DSA(EVP_PKEY *pkey)
{
    DSA *ret = evp_pkey_get0_DSA_int(pkey);

    if (ret != NULL)
        DSA_up_ref(ret);
    return ret;
}
# endif /*  OPENSSL_NO_DSA */

# ifndef OPENSSL_NO_EC
static const ECX_KEY *evp_pkey_get0_ECX_KEY(const EVP_PKEY *pkey, int type)
{
    if (EVP_PKEY_get_base_id(pkey) != type) {
        ERR_raise(ERR_LIB_EVP, EVP_R_EXPECTING_A_ECX_KEY);
        return NULL;
    }
    return evp_pkey_get_legacy((EVP_PKEY *)pkey);
}

static ECX_KEY *evp_pkey_get1_ECX_KEY(EVP_PKEY *pkey, int type)
{
    ECX_KEY *ret = (ECX_KEY *)evp_pkey_get0_ECX_KEY(pkey, type);

    if (ret != NULL && !ossl_ecx_key_up_ref(ret))
        ret = NULL;
    return ret;
}

#  define IMPLEMENT_ECX_VARIANT(NAME)                                   \
    ECX_KEY *ossl_evp_pkey_get1_##NAME(EVP_PKEY *pkey)                  \
    {                                                                   \
        return evp_pkey_get1_ECX_KEY(pkey, EVP_PKEY_##NAME);            \
    }
IMPLEMENT_ECX_VARIANT(X25519)
IMPLEMENT_ECX_VARIANT(X448)
IMPLEMENT_ECX_VARIANT(ED25519)
IMPLEMENT_ECX_VARIANT(ED448)

# endif

# if !defined(OPENSSL_NO_DH) && !defined(OPENSSL_NO_DEPRECATED_3_0)

int EVP_PKEY_set1_DH(EVP_PKEY *pkey, DH *dhkey)
{
    int ret, type;

    /*
     * ossl_dh_is_named_safe_prime_group() returns 1 for named safe prime groups
     * related to ffdhe and modp (which cache q = (p - 1) / 2),
     * and returns 0 for all other dh parameter generation types including
     * RFC5114 named groups.
     *
     * The EVP_PKEY_DH type is used for dh parameter generation types:
     *  - named safe prime groups related to ffdhe and modp
     *  - safe prime generator
     *
     * The type EVP_PKEY_DHX is used for dh parameter generation types
     *  - fips186-4 and fips186-2
     *  - rfc5114 named groups.
     *
     * The EVP_PKEY_DH type is used to save PKCS#3 data than can be stored
     * without a q value.
     * The EVP_PKEY_DHX type is used to save X9.42 data that requires the
     * q value to be stored.
     */
    if (ossl_dh_is_named_safe_prime_group(dhkey))
        type = EVP_PKEY_DH;
    else
        type = DH_get0_q(dhkey) == NULL ? EVP_PKEY_DH : EVP_PKEY_DHX;

    ret = EVP_PKEY_assign(pkey, type, dhkey);

    if (ret)
        DH_up_ref(dhkey);
    return ret;
}

DH *evp_pkey_get0_DH_int(const EVP_PKEY *pkey)
{
    if (pkey->type != EVP_PKEY_DH && pkey->type != EVP_PKEY_DHX) {
        ERR_raise(ERR_LIB_EVP, EVP_R_EXPECTING_A_DH_KEY);
        return NULL;
    }
    return evp_pkey_get_legacy((EVP_PKEY *)pkey);
}

const DH *EVP_PKEY_get0_DH(const EVP_PKEY *pkey)
{
    return evp_pkey_get0_DH_int(pkey);
}

DH *EVP_PKEY_get1_DH(EVP_PKEY *pkey)
{
    DH *ret = evp_pkey_get0_DH_int(pkey);

    if (ret != NULL)
        DH_up_ref(ret);
    return ret;
}
# endif

int EVP_PKEY_type(int type)
{
    int ret;
    const EVP_PKEY_ASN1_METHOD *ameth;
    ENGINE *e;
    ameth = EVP_PKEY_asn1_find(&e, type);
    if (ameth)
        ret = ameth->pkey_id;
    else
        ret = NID_undef;
# ifndef OPENSSL_NO_ENGINE
    ENGINE_finish(e);
# endif
    return ret;
}

int EVP_PKEY_get_id(const EVP_PKEY *pkey)
{
    return pkey->type;
}

int EVP_PKEY_get_base_id(const EVP_PKEY *pkey)
{
    return EVP_PKEY_type(pkey->type);
}

/*
 * These hard coded cases are pure hackery to get around the fact
 * that names in crypto/objects/objects.txt are a mess.  There is
 * no "EC", and "RSA" leads to the NID for 2.5.8.1.1, an OID that's
 * fallen out in favor of { pkcs-1 1 }, i.e. 1.2.840.113549.1.1.1,
 * the NID of which is used for EVP_PKEY_RSA.  Strangely enough,
 * "DSA" is accurate...  but still, better be safe and hard-code
 * names that we know.
 * On a similar topic, EVP_PKEY_type(EVP_PKEY_SM2) will result in
 * EVP_PKEY_EC, because of aliasing.
 * This should be cleaned away along with all other #legacy support.
 */
static const OSSL_ITEM standard_name2type[] = {
    { EVP_PKEY_RSA,     "RSA" },
    { EVP_PKEY_RSA_PSS, "RSA-PSS" },
    { EVP_PKEY_EC,      "EC" },
    { EVP_PKEY_ED25519, "ED25519" },
    { EVP_PKEY_ED448,   "ED448" },
    { EVP_PKEY_X25519,  "X25519" },
    { EVP_PKEY_X448,    "X448" },
    { EVP_PKEY_SM2,     "SM2" },
    { EVP_PKEY_DH,      "DH" },
    { EVP_PKEY_DHX,     "X9.42 DH" },
    { EVP_PKEY_DHX,     "DHX" },
    { EVP_PKEY_DSA,     "DSA" },
};

int evp_pkey_name2type(const char *name)
{
    int type;
    size_t i;

    for (i = 0; i < OSSL_NELEM(standard_name2type); i++) {
        if (OPENSSL_strcasecmp(name, standard_name2type[i].ptr) == 0)
            return (int)standard_name2type[i].id;
    }

    if ((type = EVP_PKEY_type(OBJ_sn2nid(name))) != NID_undef)
        return type;
    return EVP_PKEY_type(OBJ_ln2nid(name));
}

const char *evp_pkey_type2name(int type)
{
    size_t i;

    for (i = 0; i < OSSL_NELEM(standard_name2type); i++) {
        if (type == (int)standard_name2type[i].id)
            return standard_name2type[i].ptr;
    }

    return OBJ_nid2sn(type);
}

int EVP_PKEY_is_a(const EVP_PKEY *pkey, const char *name)
{
    if (pkey == NULL)
        return 0;
    if (pkey->keymgmt == NULL)
        return pkey->type == evp_pkey_name2type(name);
    return EVP_KEYMGMT_is_a(pkey->keymgmt, name);
}

int EVP_PKEY_type_names_do_all(const EVP_PKEY *pkey,
                               void (*fn)(const char *name, void *data),
                               void *data)
{
    if (!evp_pkey_is_typed(pkey))
        return 0;

    if (!evp_pkey_is_provided(pkey)) {
        const char *name = OBJ_nid2sn(EVP_PKEY_get_id(pkey));

        fn(name, data);
        return 1;
    }
    return EVP_KEYMGMT_names_do_all(pkey->keymgmt, fn, data);
}

int EVP_PKEY_can_sign(const EVP_PKEY *pkey)
{
    if (pkey->keymgmt == NULL) {
        switch (EVP_PKEY_get_base_id(pkey)) {
        case EVP_PKEY_RSA:
            return 1;
# ifndef OPENSSL_NO_DSA
        case EVP_PKEY_DSA:
            return 1;
# endif
# ifndef OPENSSL_NO_EC
        case EVP_PKEY_ED25519:
        case EVP_PKEY_ED448:
            return 1;
        case EVP_PKEY_EC:        /* Including SM2 */
            return EC_KEY_can_sign(pkey->pkey.ec);
# endif
        default:
            break;
        }
    } else {
        const OSSL_PROVIDER *prov = EVP_KEYMGMT_get0_provider(pkey->keymgmt);
        OSSL_LIB_CTX *libctx = ossl_provider_libctx(prov);
        const char *supported_sig =
            pkey->keymgmt->query_operation_name != NULL
            ? pkey->keymgmt->query_operation_name(OSSL_OP_SIGNATURE)
            : EVP_KEYMGMT_get0_name(pkey->keymgmt);
        EVP_SIGNATURE *signature = NULL;

        signature = EVP_SIGNATURE_fetch(libctx, supported_sig, NULL);
        if (signature != NULL) {
            EVP_SIGNATURE_free(signature);
            return 1;
        }
    }
    return 0;
}

static int print_reset_indent(BIO **out, int pop_f_prefix, long saved_indent)
{
    BIO_set_indent(*out, saved_indent);
    if (pop_f_prefix) {
        BIO *next = BIO_pop(*out);

        BIO_free(*out);
        *out = next;
    }
    return 1;
}

static int print_set_indent(BIO **out, int *pop_f_prefix, long *saved_indent,
                            long indent)
{
    *pop_f_prefix = 0;
    *saved_indent = 0;
    if (indent > 0) {
        long i = BIO_get_indent(*out);

        *saved_indent =  (i < 0 ? 0 : i);
        if (BIO_set_indent(*out, indent) <= 0) {
            BIO *prefbio = BIO_new(BIO_f_prefix());

            if (prefbio == NULL)
                return 0;
            *out = BIO_push(prefbio, *out);
            *pop_f_prefix = 1;
        }
        if (BIO_set_indent(*out, indent) <= 0) {
            print_reset_indent(out, *pop_f_prefix, *saved_indent);
            return 0;
        }
    }
    return 1;
}

static int unsup_alg(BIO *out, const EVP_PKEY *pkey, int indent,
                     const char *kstr)
{
    return BIO_indent(out, indent, 128)
        && BIO_printf(out, "%s algorithm \"%s\" unsupported\n",
                      kstr, OBJ_nid2ln(pkey->type)) > 0;
}

static int print_pkey(const EVP_PKEY *pkey, BIO *out, int indent,
                      int selection /* For provided encoding */,
                      const char *propquery /* For provided encoding */,
                      int (*legacy_print)(BIO *out, const EVP_PKEY *pkey,
                                          int indent, ASN1_PCTX *pctx),
                      ASN1_PCTX *legacy_pctx /* For legacy print */)
{
    int pop_f_prefix;
    long saved_indent;
    OSSL_ENCODER_CTX *ctx = NULL;
    int ret = -2;                /* default to unsupported */

    if (!print_set_indent(&out, &pop_f_prefix, &saved_indent, indent))
        return 0;

    ctx = OSSL_ENCODER_CTX_new_for_pkey(pkey, selection, "TEXT", NULL,
                                        propquery);
    if (OSSL_ENCODER_CTX_get_num_encoders(ctx) != 0)
        ret = OSSL_ENCODER_to_bio(ctx, out);
    OSSL_ENCODER_CTX_free(ctx);

    if (ret != -2)
        goto end;

    /* legacy fallback */
    if (legacy_print != NULL)
        ret = legacy_print(out, pkey, 0, legacy_pctx);
    else
        ret = unsup_alg(out, pkey, 0, "Public Key");

 end:
    print_reset_indent(&out, pop_f_prefix, saved_indent);
    return ret;
}

int EVP_PKEY_print_public(BIO *out, const EVP_PKEY *pkey,
                          int indent, ASN1_PCTX *pctx)
{
    return print_pkey(pkey, out, indent, EVP_PKEY_PUBLIC_KEY, NULL,
                      (pkey->ameth != NULL ? pkey->ameth->pub_print : NULL),
                      pctx);
}

int EVP_PKEY_print_private(BIO *out, const EVP_PKEY *pkey,
                           int indent, ASN1_PCTX *pctx)
{
    return print_pkey(pkey, out, indent, EVP_PKEY_KEYPAIR, NULL,
                      (pkey->ameth != NULL ? pkey->ameth->priv_print : NULL),
                      pctx);
}

int EVP_PKEY_print_params(BIO *out, const EVP_PKEY *pkey,
                          int indent, ASN1_PCTX *pctx)
{
    return print_pkey(pkey, out, indent, EVP_PKEY_KEY_PARAMETERS, NULL,
                      (pkey->ameth != NULL ? pkey->ameth->param_print : NULL),
                      pctx);
}

# ifndef OPENSSL_NO_STDIO
int EVP_PKEY_print_public_fp(FILE *fp, const EVP_PKEY *pkey,
                             int indent, ASN1_PCTX *pctx)
{
    int ret;
    BIO *b = BIO_new_fp(fp, BIO_NOCLOSE);

    if (b == NULL)
        return 0;
    ret = EVP_PKEY_print_public(b, pkey, indent, pctx);
    BIO_free(b);
    return ret;
}

int EVP_PKEY_print_private_fp(FILE *fp, const EVP_PKEY *pkey,
                              int indent, ASN1_PCTX *pctx)
{
    int ret;
    BIO *b = BIO_new_fp(fp, BIO_NOCLOSE);

    if (b == NULL)
        return 0;
    ret = EVP_PKEY_print_private(b, pkey, indent, pctx);
    BIO_free(b);
    return ret;
}

int EVP_PKEY_print_params_fp(FILE *fp, const EVP_PKEY *pkey,
                             int indent, ASN1_PCTX *pctx)
{
    int ret;
    BIO *b = BIO_new_fp(fp, BIO_NOCLOSE);

    if (b == NULL)
        return 0;
    ret = EVP_PKEY_print_params(b, pkey, indent, pctx);
    BIO_free(b);
    return ret;
}
# endif

static void mdname2nid(const char *mdname, void *data)
{
    int *nid = (int *)data;

    if (*nid != NID_undef)
        return;

    *nid = OBJ_sn2nid(mdname);
    if (*nid == NID_undef)
        *nid = OBJ_ln2nid(mdname);
}

static int legacy_asn1_ctrl_to_param(EVP_PKEY *pkey, int op,
                                     int arg1, void *arg2)
{
    if (pkey->keymgmt == NULL)
        return 0;
    switch (op) {
    case ASN1_PKEY_CTRL_DEFAULT_MD_NID:
        {
            char mdname[80] = "";
            int rv = EVP_PKEY_get_default_digest_name(pkey, mdname,
                                                      sizeof(mdname));

            if (rv > 0) {
                int mdnum;
                OSSL_LIB_CTX *libctx = ossl_provider_libctx(pkey->keymgmt->prov);
                /* Make sure the MD is in the namemap if available */
                EVP_MD *md;
                OSSL_NAMEMAP *namemap;
                int nid = NID_undef;

                (void)ERR_set_mark();
                md = EVP_MD_fetch(libctx, mdname, NULL);
                (void)ERR_pop_to_mark();
                namemap = ossl_namemap_stored(libctx);

                /*
                 * The only reason to fetch the MD was to make sure it is in the
                 * namemap. We can immediately free it.
                 */
                EVP_MD_free(md);
                mdnum = ossl_namemap_name2num(namemap, mdname);
                if (mdnum == 0)
                    return 0;

                /*
                 * We have the namemap number - now we need to find the
                 * associated nid
                 */
                if (!ossl_namemap_doall_names(namemap, mdnum, mdname2nid, &nid))
                    return 0;
                *(int *)arg2 = nid;
            }
            return rv;
        }
    default:
        return -2;
    }
}

static int evp_pkey_asn1_ctrl(EVP_PKEY *pkey, int op, int arg1, void *arg2)
{
    if (pkey->ameth == NULL)
        return legacy_asn1_ctrl_to_param(pkey, op, arg1, arg2);
    if (pkey->ameth->pkey_ctrl == NULL)
        return -2;
    return pkey->ameth->pkey_ctrl(pkey, op, arg1, arg2);
}

int EVP_PKEY_get_default_digest_nid(EVP_PKEY *pkey, int *pnid)
{
    if (pkey == NULL)
        return 0;
    return evp_pkey_asn1_ctrl(pkey, ASN1_PKEY_CTRL_DEFAULT_MD_NID, 0, pnid);
}

int EVP_PKEY_get_default_digest_name(EVP_PKEY *pkey,
                                     char *mdname, size_t mdname_sz)
{
    if (pkey->ameth == NULL)
        return evp_keymgmt_util_get_deflt_digest_name(pkey->keymgmt,
                                                      pkey->keydata,
                                                      mdname, mdname_sz);

    {
        int nid = NID_undef;
        int rv = EVP_PKEY_get_default_digest_nid(pkey, &nid);
        const char *name = rv > 0 ? OBJ_nid2sn(nid) : NULL;

        if (rv > 0)
            OPENSSL_strlcpy(mdname, name, mdname_sz);
        return rv;
    }
}

int EVP_PKEY_get_group_name(const EVP_PKEY *pkey, char *gname, size_t gname_sz,
                            size_t *gname_len)
{
    return EVP_PKEY_get_utf8_string_param(pkey, OSSL_PKEY_PARAM_GROUP_NAME,
                                          gname, gname_sz, gname_len);
}

int EVP_PKEY_digestsign_supports_digest(EVP_PKEY *pkey, OSSL_LIB_CTX *libctx,
                                        const char *name, const char *propq)
{
    int rv;
    EVP_MD_CTX *ctx = NULL;

    if ((ctx = EVP_MD_CTX_new()) == NULL)
        return -1;

    ERR_set_mark();
    rv = EVP_DigestSignInit_ex(ctx, NULL, name, libctx,
                               propq, pkey, NULL);
    ERR_pop_to_mark();

    EVP_MD_CTX_free(ctx);
    return rv;
}

int EVP_PKEY_set1_encoded_public_key(EVP_PKEY *pkey, const unsigned char *pub,
                                     size_t publen)
{
    if (pkey == NULL)
        return 0;
    if (evp_pkey_is_provided(pkey))
        return
            EVP_PKEY_set_octet_string_param(pkey,
                                            OSSL_PKEY_PARAM_ENCODED_PUBLIC_KEY,
                                            (unsigned char *)pub, publen);

    if (publen > INT_MAX)
        return 0;
    /* Historically this function was EVP_PKEY_set1_tls_encodedpoint */
    if (evp_pkey_asn1_ctrl(pkey, ASN1_PKEY_CTRL_SET1_TLS_ENCPT, publen,
                           (void *)pub) <= 0)
        return 0;
    return 1;
}

size_t EVP_PKEY_get1_encoded_public_key(EVP_PKEY *pkey, unsigned char **ppub)
{
    int rv;

    if (pkey == NULL)
        return 0;
    if (evp_pkey_is_provided(pkey)) {
        size_t return_size = OSSL_PARAM_UNMODIFIED;
        unsigned char *buf;

        /*
         * We know that this is going to fail, but it will give us a size
         * to allocate.
         */
        EVP_PKEY_get_octet_string_param(pkey,
                                        OSSL_PKEY_PARAM_ENCODED_PUBLIC_KEY,
                                        NULL, 0, &return_size);
        if (return_size == OSSL_PARAM_UNMODIFIED)
            return 0;

        *ppub = NULL;
        buf = OPENSSL_malloc(return_size);
        if (buf == NULL)
            return 0;

        if (!EVP_PKEY_get_octet_string_param(pkey,
                                             OSSL_PKEY_PARAM_ENCODED_PUBLIC_KEY,
                                             buf, return_size, NULL)) {
            OPENSSL_free(buf);
            return 0;
        }
        *ppub = buf;
        return return_size;
    }


    rv = evp_pkey_asn1_ctrl(pkey, ASN1_PKEY_CTRL_GET1_TLS_ENCPT, 0, ppub);
    if (rv <= 0)
        return 0;
    return rv;
}

#endif /* FIPS_MODULE */

/*- All methods below can also be used in FIPS_MODULE */

>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
EVP_PKEY *EVP_PKEY_new(void)
{
    EVP_PKEY *ret = OPENSSL_zalloc(sizeof(*ret));

    if (ret == NULL) {
<<<<<<< HEAD
        EVPerr(EVP_F_EVP_PKEY_NEW, ERR_R_MALLOC_FAILURE);
        return NULL;
    }
=======
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
    ret->type = EVP_PKEY_NONE;
    ret->save_type = EVP_PKEY_NONE;
    ret->references = 1;
    ret->save_parameters = 1;
    ret->lock = CRYPTO_THREAD_lock_new();
    if (ret->lock == NULL) {
<<<<<<< HEAD
        EVPerr(EVP_F_EVP_PKEY_NEW, ERR_R_MALLOC_FAILURE);
        OPENSSL_free(ret);
        return NULL;
    }
=======
        EVPerr(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        goto err;
    }

#ifndef FIPS_MODULE
    ret->save_parameters = 1;
    if (!CRYPTO_new_ex_data(CRYPTO_EX_INDEX_EVP_PKEY, ret, &ret->ex_data)) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        goto err;
    }
#endif
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
    return ret;
}

int EVP_PKEY_up_ref(EVP_PKEY *pkey)
{
    int i;

    if (CRYPTO_UP_REF(&pkey->references, &i, pkey->lock) <= 0)
        return 0;

    REF_PRINT_COUNT("EVP_PKEY", pkey);
    REF_ASSERT_ISNT(i < 2);
    return ((i > 1) ? 1 : 0);
}

/*
 * Setup a public key ASN1 method and ENGINE from a NID or a string. If pkey
 * is NULL just return 1 or 0 if the algorithm exists.
 */

static int pkey_set_type(EVP_PKEY *pkey, ENGINE *e, int type, const char *str,
                         int len)
{
    const EVP_PKEY_ASN1_METHOD *ameth;
    ENGINE **eptr = (e == NULL) ? &e :  NULL;

    if (pkey) {
        if (pkey->pkey.ptr)
            EVP_PKEY_free_it(pkey);
        /*
         * If key type matches and a method exists then this lookup has
         * succeeded once so just indicate success.
         */
        if ((type == pkey->save_type) && pkey->ameth)
            return 1;
#ifndef OPENSSL_NO_ENGINE
        /* If we have ENGINEs release them */
        ENGINE_finish(pkey->engine);
        pkey->engine = NULL;
        ENGINE_finish(pkey->pmeth_engine);
        pkey->pmeth_engine = NULL;
#endif
    }
    if (str)
        ameth = EVP_PKEY_asn1_find_str(eptr, str, len);
    else
        ameth = EVP_PKEY_asn1_find(eptr, type);
#ifndef OPENSSL_NO_ENGINE
    if (pkey == NULL && eptr != NULL)
        ENGINE_finish(e);
#endif
    if (ameth == NULL) {
        EVPerr(EVP_F_PKEY_SET_TYPE, EVP_R_UNSUPPORTED_ALGORITHM);
        return 0;
    }
    if (pkey) {
        pkey->ameth = ameth;
        pkey->type = pkey->ameth->pkey_id;
        pkey->save_type = type;
# ifndef OPENSSL_NO_ENGINE
        if (eptr == NULL && e != NULL && !ENGINE_init(e)) {
            EVPerr(EVP_F_PKEY_SET_TYPE, EVP_R_INITIALIZATION_ERROR);
            return 0;
        }
# endif
        pkey->engine = e;
    }
    return 1;
}

EVP_PKEY *EVP_PKEY_new_raw_private_key(int type, ENGINE *e,
                                       const unsigned char *priv,
                                       size_t len)
{
    EVP_PKEY *ret = EVP_PKEY_new();

    if (ret == NULL
            || !pkey_set_type(ret, e, type, NULL, -1)) {
        /* EVPerr already called */
        goto err;
    }

    if (ret->ameth->set_priv_key == NULL) {
        EVPerr(EVP_F_EVP_PKEY_NEW_RAW_PRIVATE_KEY,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        goto err;
    }

    if (!ret->ameth->set_priv_key(ret, priv, len)) {
        EVPerr(EVP_F_EVP_PKEY_NEW_RAW_PRIVATE_KEY, EVP_R_KEY_SETUP_FAILED);
        goto err;
    }

    return ret;

 err:
    EVP_PKEY_free(ret);
    return NULL;
}

EVP_PKEY *EVP_PKEY_new_raw_public_key(int type, ENGINE *e,
                                      const unsigned char *pub,
                                      size_t len)
{
    EVP_PKEY *ret = EVP_PKEY_new();

    if (ret == NULL
            || !pkey_set_type(ret, e, type, NULL, -1)) {
        /* EVPerr already called */
        goto err;
    }

    if (ret->ameth->set_pub_key == NULL) {
        EVPerr(EVP_F_EVP_PKEY_NEW_RAW_PUBLIC_KEY,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        goto err;
    }

    if (!ret->ameth->set_pub_key(ret, pub, len)) {
        EVPerr(EVP_F_EVP_PKEY_NEW_RAW_PUBLIC_KEY, EVP_R_KEY_SETUP_FAILED);
        goto err;
    }

    return ret;

 err:
    EVP_PKEY_free(ret);
    return NULL;
}

int EVP_PKEY_get_raw_private_key(const EVP_PKEY *pkey, unsigned char *priv,
                                 size_t *len)
{
     if (pkey->ameth->get_priv_key == NULL) {
        EVPerr(EVP_F_EVP_PKEY_GET_RAW_PRIVATE_KEY,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return 0;
    }

    if (!pkey->ameth->get_priv_key(pkey, priv, len)) {
        EVPerr(EVP_F_EVP_PKEY_GET_RAW_PRIVATE_KEY, EVP_R_GET_RAW_KEY_FAILED);
        return 0;
    }

    return 1;
}

int EVP_PKEY_get_raw_public_key(const EVP_PKEY *pkey, unsigned char *pub,
                                size_t *len)
{
     if (pkey->ameth->get_pub_key == NULL) {
        EVPerr(EVP_F_EVP_PKEY_GET_RAW_PUBLIC_KEY,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return 0;
    }

    if (!pkey->ameth->get_pub_key(pkey, pub, len)) {
        EVPerr(EVP_F_EVP_PKEY_GET_RAW_PUBLIC_KEY, EVP_R_GET_RAW_KEY_FAILED);
        return 0;
    }

    return 1;
}

EVP_PKEY *EVP_PKEY_new_CMAC_key(ENGINE *e, const unsigned char *priv,
                                size_t len, const EVP_CIPHER *cipher)
{
#ifndef OPENSSL_NO_CMAC
    EVP_PKEY *ret = EVP_PKEY_new();
    CMAC_CTX *cmctx = CMAC_CTX_new();

    if (ret == NULL
            || cmctx == NULL
            || !pkey_set_type(ret, e, EVP_PKEY_CMAC, NULL, -1)) {
        /* EVPerr already called */
        goto err;
    }

    if (!CMAC_Init(cmctx, priv, len, cipher, e)) {
        EVPerr(EVP_F_EVP_PKEY_NEW_CMAC_KEY, EVP_R_KEY_SETUP_FAILED);
        goto err;
    }

    ret->pkey.ptr = cmctx;
    return ret;

 err:
    EVP_PKEY_free(ret);
    CMAC_CTX_free(cmctx);
    return NULL;
#else
    EVPerr(EVP_F_EVP_PKEY_NEW_CMAC_KEY,
           EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
    return NULL;
#endif
}

int EVP_PKEY_set_type(EVP_PKEY *pkey, int type)
{
    return pkey_set_type(pkey, NULL, type, NULL, -1);
}

int EVP_PKEY_set_type_str(EVP_PKEY *pkey, const char *str, int len)
{
    return pkey_set_type(pkey, NULL, EVP_PKEY_NONE, str, len);
}

int EVP_PKEY_set_alias_type(EVP_PKEY *pkey, int type)
{
    if (pkey->type == type) {
        return 1; /* it already is that type */
    }

    /*
     * The application is requesting to alias this to a different pkey type,
     * but not one that resolves to the base type.
     */
    if (EVP_PKEY_type(type) != EVP_PKEY_base_id(pkey)) {
        EVPerr(EVP_F_EVP_PKEY_SET_ALIAS_TYPE, EVP_R_UNSUPPORTED_ALGORITHM);
        return 0;
    }

    pkey->type = type;
    return 1;
}

#ifndef OPENSSL_NO_ENGINE
int EVP_PKEY_set1_engine(EVP_PKEY *pkey, ENGINE *e)
{
    if (e != NULL) {
        if (!ENGINE_init(e)) {
            EVPerr(EVP_F_EVP_PKEY_SET1_ENGINE, ERR_R_ENGINE_LIB);
            return 0;
        }
        if (ENGINE_get_pkey_meth(e, pkey->type) == NULL) {
            ENGINE_finish(e);
            EVPerr(EVP_F_EVP_PKEY_SET1_ENGINE, EVP_R_UNSUPPORTED_ALGORITHM);
            return 0;
        }
    }
    ENGINE_finish(pkey->pmeth_engine);
    pkey->pmeth_engine = e;
    return 1;
}

ENGINE *EVP_PKEY_get0_engine(const EVP_PKEY *pkey)
{
    return pkey->engine;
}
#endif
int EVP_PKEY_assign(EVP_PKEY *pkey, int type, void *key)
{
    if (pkey == NULL || !EVP_PKEY_set_type(pkey, type))
        return 0;
    pkey->pkey.ptr = key;
    return (key != NULL);
}

void *EVP_PKEY_get0(const EVP_PKEY *pkey)
{
    return pkey->pkey.ptr;
}

const unsigned char *EVP_PKEY_get0_hmac(const EVP_PKEY *pkey, size_t *len)
{
    ASN1_OCTET_STRING *os = NULL;
    if (pkey->type != EVP_PKEY_HMAC) {
        EVPerr(EVP_F_EVP_PKEY_GET0_HMAC, EVP_R_EXPECTING_AN_HMAC_KEY);
        return NULL;
    }
    os = EVP_PKEY_get0(pkey);
    *len = os->length;
    return os->data;
}

#ifndef OPENSSL_NO_POLY1305
const unsigned char *EVP_PKEY_get0_poly1305(const EVP_PKEY *pkey, size_t *len)
{
    ASN1_OCTET_STRING *os = NULL;
    if (pkey->type != EVP_PKEY_POLY1305) {
        EVPerr(EVP_F_EVP_PKEY_GET0_POLY1305, EVP_R_EXPECTING_A_POLY1305_KEY);
        return NULL;
    }
    os = EVP_PKEY_get0(pkey);
    *len = os->length;
    return os->data;
}
#endif

#ifndef OPENSSL_NO_SIPHASH
const unsigned char *EVP_PKEY_get0_siphash(const EVP_PKEY *pkey, size_t *len)
{
    ASN1_OCTET_STRING *os = NULL;

    if (pkey->type != EVP_PKEY_SIPHASH) {
        EVPerr(EVP_F_EVP_PKEY_GET0_SIPHASH, EVP_R_EXPECTING_A_SIPHASH_KEY);
        return NULL;
    }
    os = EVP_PKEY_get0(pkey);
    *len = os->length;
    return os->data;
}
#endif

#ifndef OPENSSL_NO_RSA
int EVP_PKEY_set1_RSA(EVP_PKEY *pkey, RSA *key)
{
    int ret = EVP_PKEY_assign_RSA(pkey, key);
    if (ret)
        RSA_up_ref(key);
    return ret;
}

RSA *EVP_PKEY_get0_RSA(EVP_PKEY *pkey)
{
    if (pkey->type != EVP_PKEY_RSA && pkey->type != EVP_PKEY_RSA_PSS) {
        EVPerr(EVP_F_EVP_PKEY_GET0_RSA, EVP_R_EXPECTING_AN_RSA_KEY);
        return NULL;
    }
    return pkey->pkey.rsa;
}

RSA *EVP_PKEY_get1_RSA(EVP_PKEY *pkey)
{
    RSA *ret = EVP_PKEY_get0_RSA(pkey);
    if (ret != NULL)
        RSA_up_ref(ret);
    return ret;
}
#endif

#ifndef OPENSSL_NO_DSA
int EVP_PKEY_set1_DSA(EVP_PKEY *pkey, DSA *key)
{
    int ret = EVP_PKEY_assign_DSA(pkey, key);
    if (ret)
        DSA_up_ref(key);
    return ret;
}

DSA *EVP_PKEY_get0_DSA(EVP_PKEY *pkey)
{
    if (pkey->type != EVP_PKEY_DSA) {
        EVPerr(EVP_F_EVP_PKEY_GET0_DSA, EVP_R_EXPECTING_A_DSA_KEY);
        return NULL;
    }
    return pkey->pkey.dsa;
}

DSA *EVP_PKEY_get1_DSA(EVP_PKEY *pkey)
{
    DSA *ret = EVP_PKEY_get0_DSA(pkey);
    if (ret != NULL)
        DSA_up_ref(ret);
    return ret;
}
#endif

#ifndef OPENSSL_NO_EC

int EVP_PKEY_set1_EC_KEY(EVP_PKEY *pkey, EC_KEY *key)
{
    int ret = EVP_PKEY_assign_EC_KEY(pkey, key);
    if (ret)
        EC_KEY_up_ref(key);
    return ret;
}

EC_KEY *EVP_PKEY_get0_EC_KEY(EVP_PKEY *pkey)
{
    if (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC) {
        EVPerr(EVP_F_EVP_PKEY_GET0_EC_KEY, EVP_R_EXPECTING_A_EC_KEY);
        return NULL;
    }
    return pkey->pkey.ec;
}

EC_KEY *EVP_PKEY_get1_EC_KEY(EVP_PKEY *pkey)
{
    EC_KEY *ret = EVP_PKEY_get0_EC_KEY(pkey);
    if (ret != NULL)
        EC_KEY_up_ref(ret);
    return ret;
}
#endif

#ifndef OPENSSL_NO_DH

int EVP_PKEY_set1_DH(EVP_PKEY *pkey, DH *key)
{
    int type = DH_get0_q(key) == NULL ? EVP_PKEY_DH : EVP_PKEY_DHX;
    int ret = EVP_PKEY_assign(pkey, type, key);

    if (ret)
        DH_up_ref(key);
    return ret;
}

DH *EVP_PKEY_get0_DH(EVP_PKEY *pkey)
{
    if (pkey->type != EVP_PKEY_DH && pkey->type != EVP_PKEY_DHX) {
        EVPerr(EVP_F_EVP_PKEY_GET0_DH, EVP_R_EXPECTING_A_DH_KEY);
        return NULL;
    }
    return pkey->pkey.dh;
}

DH *EVP_PKEY_get1_DH(EVP_PKEY *pkey)
{
    DH *ret = EVP_PKEY_get0_DH(pkey);
    if (ret != NULL)
        DH_up_ref(ret);
    return ret;
}
#endif

int EVP_PKEY_type(int type)
{
    int ret;
    const EVP_PKEY_ASN1_METHOD *ameth;
    ENGINE *e;
    ameth = EVP_PKEY_asn1_find(&e, type);
    if (ameth)
        ret = ameth->pkey_id;
    else
        ret = NID_undef;
#ifndef OPENSSL_NO_ENGINE
    ENGINE_finish(e);
#endif
    return ret;
}

int EVP_PKEY_id(const EVP_PKEY *pkey)
{
    return pkey->type;
}

int EVP_PKEY_base_id(const EVP_PKEY *pkey)
{
    return EVP_PKEY_type(pkey->type);
}

void EVP_PKEY_free(EVP_PKEY *x)
{
    int i;

    if (x == NULL)
        return;

    CRYPTO_DOWN_REF(&x->references, &i, x->lock);
    REF_PRINT_COUNT("EVP_PKEY", x);
    if (i > 0)
        return;
    REF_ASSERT_ISNT(i < 0);
    EVP_PKEY_free_it(x);
    CRYPTO_THREAD_lock_free(x->lock);
    sk_X509_ATTRIBUTE_pop_free(x->attributes, X509_ATTRIBUTE_free);
    OPENSSL_free(x);
}

static void EVP_PKEY_free_it(EVP_PKEY *x)
{
    /* internal function; x is never NULL */
    if (x->ameth && x->ameth->pkey_free) {
        x->ameth->pkey_free(x);
        x->pkey.ptr = NULL;
    }
#ifndef OPENSSL_NO_ENGINE
    ENGINE_finish(x->engine);
    x->engine = NULL;
    ENGINE_finish(x->pmeth_engine);
    x->pmeth_engine = NULL;
#endif
}

static int unsup_alg(BIO *out, const EVP_PKEY *pkey, int indent,
                     const char *kstr)
{
    BIO_indent(out, indent, 128);
    BIO_printf(out, "%s algorithm \"%s\" unsupported\n",
               kstr, OBJ_nid2ln(pkey->type));
    return 1;
}

int EVP_PKEY_print_public(BIO *out, const EVP_PKEY *pkey,
                          int indent, ASN1_PCTX *pctx)
{
    if (pkey->ameth && pkey->ameth->pub_print)
        return pkey->ameth->pub_print(out, pkey, indent, pctx);

    return unsup_alg(out, pkey, indent, "Public Key");
}

int EVP_PKEY_print_private(BIO *out, const EVP_PKEY *pkey,
                           int indent, ASN1_PCTX *pctx)
{
    if (pkey->ameth && pkey->ameth->priv_print)
        return pkey->ameth->priv_print(out, pkey, indent, pctx);

<<<<<<< HEAD
    return unsup_alg(out, pkey, indent, "Private Key");
=======
    if (!ossl_assert(dest != NULL))
        return 0;

    if (evp_pkey_is_assigned(src) && evp_pkey_is_provided(src)) {
        EVP_KEYMGMT *keymgmt = src->keymgmt;
        void *keydata = src->keydata;
        int type = src->type;
        const char *keytype = NULL;

        keytype = EVP_KEYMGMT_get0_name(keymgmt);

        /*
         * If the type is EVP_PKEY_NONE, then we have a problem somewhere
         * else in our code.  If it's not one of the well known EVP_PKEY_xxx
         * values, it should at least be EVP_PKEY_KEYMGMT at this point.
         * The check is kept as a safety measure.
         */
        if (!ossl_assert(type != EVP_PKEY_NONE)) {
            ERR_raise_data(ERR_LIB_EVP, ERR_R_INTERNAL_ERROR,
                           "keymgmt key type = %s but legacy type = EVP_PKEY_NONE",
                           keytype);
            return 0;
        }

        /* Prefer the legacy key type name for error reporting */
        if (type != EVP_PKEY_KEYMGMT)
            keytype = OBJ_nid2sn(type);

        /* Make sure we have a clean slate to copy into */
        if (*dest == NULL) {
            allocpkey = *dest = EVP_PKEY_new();
            if (*dest == NULL) {
                ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
                return 0;
            }
        } else {
            evp_pkey_free_it(*dest);
        }

        if (EVP_PKEY_set_type(*dest, type)) {
            /* If the key is typed but empty, we're done */
            if (keydata == NULL)
                return 1;

            if ((*dest)->ameth->import_from == NULL) {
                ERR_raise_data(ERR_LIB_EVP, EVP_R_NO_IMPORT_FUNCTION,
                               "key type = %s", keytype);
            } else {
                /*
                 * We perform the export in the same libctx as the keymgmt
                 * that we are using.
                 */
                OSSL_LIB_CTX *libctx =
                    ossl_provider_libctx(keymgmt->prov);
                EVP_PKEY_CTX *pctx =
                    EVP_PKEY_CTX_new_from_pkey(libctx, *dest, NULL);

                if (pctx == NULL)
                    ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);

                if (pctx != NULL
                    && evp_keymgmt_export(keymgmt, keydata,
                                          OSSL_KEYMGMT_SELECT_ALL,
                                          (*dest)->ameth->import_from,
                                          pctx)) {
                    /* Synchronize the dirty count */
                    (*dest)->dirty_cnt_copy = (*dest)->ameth->dirty_cnt(*dest);

                    EVP_PKEY_CTX_free(pctx);
                    return 1;
                }
                EVP_PKEY_CTX_free(pctx);
            }

            ERR_raise_data(ERR_LIB_EVP, EVP_R_KEYMGMT_EXPORT_FAILURE,
                           "key type = %s", keytype);
        }
    }

    if (allocpkey != NULL) {
        EVP_PKEY_free(allocpkey);
        *dest = NULL;
    }
    return 0;
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
}

int EVP_PKEY_print_params(BIO *out, const EVP_PKEY *pkey,
                          int indent, ASN1_PCTX *pctx)
{
    if (pkey->ameth && pkey->ameth->param_print)
        return pkey->ameth->param_print(out, pkey, indent, pctx);
    return unsup_alg(out, pkey, indent, "Parameters");
}

static int evp_pkey_asn1_ctrl(EVP_PKEY *pkey, int op, int arg1, void *arg2)
{
    if (pkey->ameth == NULL || pkey->ameth->pkey_ctrl == NULL)
        return -2;
    return pkey->ameth->pkey_ctrl(pkey, op, arg1, arg2);
}

int EVP_PKEY_get_default_digest_nid(EVP_PKEY *pkey, int *pnid)
{
    return evp_pkey_asn1_ctrl(pkey, ASN1_PKEY_CTRL_DEFAULT_MD_NID, 0, pnid);
}

int EVP_PKEY_set1_tls_encodedpoint(EVP_PKEY *pkey,
                               const unsigned char *pt, size_t ptlen)
{
    if (ptlen > INT_MAX)
        return 0;
    if (evp_pkey_asn1_ctrl(pkey, ASN1_PKEY_CTRL_SET1_TLS_ENCPT, ptlen,
                           (void *)pt) <= 0)
        return 0;
    return 1;
}

size_t EVP_PKEY_get1_tls_encodedpoint(EVP_PKEY *pkey, unsigned char **ppt)
{
    int rv;
    rv = evp_pkey_asn1_ctrl(pkey, ASN1_PKEY_CTRL_GET1_TLS_ENCPT, 0, ppt);
    if (rv <= 0)
        return 0;
    return rv;
}
