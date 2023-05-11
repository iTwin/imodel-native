/*
 * Copyright 2006-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include <stdlib.h>
#include "internal/cryptlib.h"
#include <openssl/objects.h>
#include <openssl/evp.h>
#include "crypto/bn.h"
#include "crypto/asn1.h"
#include "crypto/evp.h"

int EVP_PKEY_paramgen_init(EVP_PKEY_CTX *ctx)
{
<<<<<<< HEAD
    int ret;
    if (!ctx || !ctx->pmeth || !ctx->pmeth->paramgen) {
        EVPerr(EVP_F_EVP_PKEY_PARAMGEN_INIT,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
=======
    return gen_init(ctx, EVP_PKEY_OP_PARAMGEN);
}

int EVP_PKEY_keygen_init(EVP_PKEY_CTX *ctx)
{
    return gen_init(ctx, EVP_PKEY_OP_KEYGEN);
}

static int ossl_callback_to_pkey_gencb(const OSSL_PARAM params[], void *arg)
{
    EVP_PKEY_CTX *ctx = arg;
    const OSSL_PARAM *param = NULL;
    int p = -1, n = -1;

    if (ctx->pkey_gencb == NULL)
        return 1;                /* No callback?  That's fine */

    if ((param = OSSL_PARAM_locate_const(params, OSSL_GEN_PARAM_POTENTIAL))
        == NULL
        || !OSSL_PARAM_get_int(param, &p))
        return 0;
    if ((param = OSSL_PARAM_locate_const(params, OSSL_GEN_PARAM_ITERATION))
        == NULL
        || !OSSL_PARAM_get_int(param, &n))
        return 0;

    ctx->keygen_info[0] = p;
    ctx->keygen_info[1] = n;

    return ctx->pkey_gencb(ctx);
}

int EVP_PKEY_generate(EVP_PKEY_CTX *ctx, EVP_PKEY **ppkey)
{
    int ret = 0;
    EVP_PKEY *allocated_pkey = NULL;
    /* Legacy compatible keygen callback info, only used with provider impls */
    int gentmp[2];

    if (ppkey == NULL)
        return -1;

    if (ctx == NULL)
        goto not_supported;

    if ((ctx->operation & EVP_PKEY_OP_TYPE_GEN) == 0)
        goto not_initialized;

    if (*ppkey == NULL)
        *ppkey = allocated_pkey = EVP_PKEY_new();

    if (*ppkey == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        return -1;
    }

    if (ctx->op.keymgmt.genctx == NULL)
        goto legacy;

    /*
     * Asssigning gentmp to ctx->keygen_info is something our legacy
     * implementations do.  Because the provider implementations aren't
     * allowed to reach into our EVP_PKEY_CTX, we need to provide similar
     * space for backward compatibility.  It's ok that we attach a local
     * variable, as it should only be useful in the calls down from here.
     * This is cleared as soon as it isn't useful any more, i.e. directly
     * after the evp_keymgmt_util_gen() call.
     */
    ctx->keygen_info = gentmp;
    ctx->keygen_info_count = 2;

    ret = 1;
    if (ctx->pkey != NULL) {
        EVP_KEYMGMT *tmp_keymgmt = ctx->keymgmt;
        void *keydata =
            evp_pkey_export_to_provider(ctx->pkey, ctx->libctx,
                                        &tmp_keymgmt, ctx->propquery);

        if (tmp_keymgmt == NULL)
            goto not_supported;
        /*
         * It's ok if keydata is NULL here.  The backend is expected to deal
         * with that as it sees fit.
         */
        ret = evp_keymgmt_gen_set_template(ctx->keymgmt,
                                           ctx->op.keymgmt.genctx, keydata);
    }

    /*
     * the returned value from evp_keymgmt_util_gen() is cached in *ppkey,
     * so we do not need to save it, just check it.
     */
    ret = ret
        && (evp_keymgmt_util_gen(*ppkey, ctx->keymgmt, ctx->op.keymgmt.genctx,
                                 ossl_callback_to_pkey_gencb, ctx)
            != NULL);

    ctx->keygen_info = NULL;

#ifndef FIPS_MODULE
    /* In case |*ppkey| was originally a legacy key */
    if (ret)
        evp_pkey_free_legacy(*ppkey);
#endif

    /*
     * Because we still have legacy keys
     */
    (*ppkey)->type = ctx->legacy_keytype;

    goto end;

 legacy:
#ifdef FIPS_MODULE
    goto not_supported;
#else
    /*
     * If we get here then we're using legacy paramgen/keygen. In that case
     * the pkey in ctx (if there is one) had better not be provided (because the
     * legacy methods may not know how to handle it). However we can only get
     * here if ctx->op.keymgmt.genctx == NULL, but that should never be the case
     * if ctx->pkey is provided because we don't allow this when we initialise
     * the ctx.
     */
    if (ctx->pkey != NULL && !ossl_assert(!evp_pkey_is_provided(ctx->pkey)))
        goto not_accessible;

    switch (ctx->operation) {
    case EVP_PKEY_OP_PARAMGEN:
        ret = ctx->pmeth->paramgen(ctx, *ppkey);
        break;
    case EVP_PKEY_OP_KEYGEN:
        ret = ctx->pmeth->keygen(ctx, *ppkey);
        break;
    default:
        goto not_supported;
    }
#endif

 end:
    if (ret <= 0) {
        if (allocated_pkey != NULL)
            *ppkey = NULL;
        EVP_PKEY_free(allocated_pkey);
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
    }
    ctx->operation = EVP_PKEY_OP_PARAMGEN;
    if (!ctx->pmeth->paramgen_init)
        return 1;
    ret = ctx->pmeth->paramgen_init(ctx);
    if (ret <= 0)
        ctx->operation = EVP_PKEY_OP_UNDEFINED;
    return ret;
}

int EVP_PKEY_paramgen(EVP_PKEY_CTX *ctx, EVP_PKEY **ppkey)
{
    int ret;
    if (!ctx || !ctx->pmeth || !ctx->pmeth->paramgen) {
        EVPerr(EVP_F_EVP_PKEY_PARAMGEN,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }

    if (ctx->operation != EVP_PKEY_OP_PARAMGEN) {
        EVPerr(EVP_F_EVP_PKEY_PARAMGEN, EVP_R_OPERATON_NOT_INITIALIZED);
        return -1;
    }

    if (ppkey == NULL)
        return -1;

    if (*ppkey == NULL)
        *ppkey = EVP_PKEY_new();

    if (*ppkey == NULL) {
        EVPerr(EVP_F_EVP_PKEY_PARAMGEN, ERR_R_MALLOC_FAILURE);
        return -1;
    }

    ret = ctx->pmeth->paramgen(ctx, *ppkey);
    if (ret <= 0) {
        EVP_PKEY_free(*ppkey);
        *ppkey = NULL;
    }
    return ret;
}

int EVP_PKEY_keygen_init(EVP_PKEY_CTX *ctx)
{
    int ret;
    if (!ctx || !ctx->pmeth || !ctx->pmeth->keygen) {
        EVPerr(EVP_F_EVP_PKEY_KEYGEN_INIT,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }
    ctx->operation = EVP_PKEY_OP_KEYGEN;
    if (!ctx->pmeth->keygen_init)
        return 1;
    ret = ctx->pmeth->keygen_init(ctx);
    if (ret <= 0)
        ctx->operation = EVP_PKEY_OP_UNDEFINED;
    return ret;
}

int EVP_PKEY_keygen(EVP_PKEY_CTX *ctx, EVP_PKEY **ppkey)
{
    int ret;

    if (!ctx || !ctx->pmeth || !ctx->pmeth->keygen) {
        EVPerr(EVP_F_EVP_PKEY_KEYGEN,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }
    if (ctx->operation != EVP_PKEY_OP_KEYGEN) {
        EVPerr(EVP_F_EVP_PKEY_KEYGEN, EVP_R_OPERATON_NOT_INITIALIZED);
        return -1;
    }

    if (ppkey == NULL)
        return -1;

    if (*ppkey == NULL)
        *ppkey = EVP_PKEY_new();
    if (*ppkey == NULL)
        return -1;

    ret = ctx->pmeth->keygen(ctx, *ppkey);
    if (ret <= 0) {
        EVP_PKEY_free(*ppkey);
        *ppkey = NULL;
    }
    return ret;
}

void EVP_PKEY_CTX_set_cb(EVP_PKEY_CTX *ctx, EVP_PKEY_gen_cb *cb)
{
    ctx->pkey_gencb = cb;
}

EVP_PKEY_gen_cb *EVP_PKEY_CTX_get_cb(EVP_PKEY_CTX *ctx)
{
    return ctx->pkey_gencb;
}

/*
 * "translation callback" to call EVP_PKEY_CTX callbacks using BN_GENCB style
 * callbacks.
 */

static int trans_cb(int a, int b, BN_GENCB *gcb)
{
    EVP_PKEY_CTX *ctx = BN_GENCB_get_arg(gcb);
    ctx->keygen_info[0] = a;
    ctx->keygen_info[1] = b;
    return ctx->pkey_gencb(ctx);
}

void evp_pkey_set_cb_translate(BN_GENCB *cb, EVP_PKEY_CTX *ctx)
{
    BN_GENCB_set(cb, trans_cb, ctx);
}

int EVP_PKEY_CTX_get_keygen_info(EVP_PKEY_CTX *ctx, int idx)
{
    if (idx == -1)
        return ctx->keygen_info_count;
    if (idx < 0 || idx > ctx->keygen_info_count)
        return 0;
    return ctx->keygen_info[idx];
}

EVP_PKEY *EVP_PKEY_new_mac_key(int type, ENGINE *e,
                               const unsigned char *key, int keylen)
{
    EVP_PKEY_CTX *mac_ctx = NULL;
    EVP_PKEY *mac_key = NULL;
    mac_ctx = EVP_PKEY_CTX_new_id(type, e);
    if (!mac_ctx)
        return NULL;
    if (EVP_PKEY_keygen_init(mac_ctx) <= 0)
        goto merr;
    if (EVP_PKEY_CTX_set_mac_key(mac_ctx, key, keylen) <= 0)
        goto merr;
    if (EVP_PKEY_keygen(mac_ctx, &mac_key) <= 0)
        goto merr;
 merr:
    EVP_PKEY_CTX_free(mac_ctx);
    return mac_key;
}

int EVP_PKEY_check(EVP_PKEY_CTX *ctx)
{
    EVP_PKEY *pkey = ctx->pkey;

    if (pkey == NULL) {
        EVPerr(EVP_F_EVP_PKEY_CHECK, EVP_R_NO_KEY_SET);
        return 0;
    }

    /* call customized check function first */
    if (ctx->pmeth->check != NULL)
        return ctx->pmeth->check(pkey);

    /* use default check function in ameth */
    if (pkey->ameth == NULL || pkey->ameth->pkey_check == NULL) {
        EVPerr(EVP_F_EVP_PKEY_CHECK,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }

<<<<<<< HEAD
    return pkey->ameth->pkey_check(pkey);
=======
    if (ppkey == NULL)
        return -1;

    if (*ppkey == NULL)
        allocated_pkey = *ppkey = EVP_PKEY_new();

    if (*ppkey == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        return -1;
    }

    keydata = evp_keymgmt_util_fromdata(*ppkey, ctx->keymgmt, selection, params);
    if (keydata == NULL) {
        if (allocated_pkey != NULL) {
            *ppkey = NULL;
            EVP_PKEY_free(allocated_pkey);
        }
        return 0;
    }
    /* keydata is cached in *ppkey, so we need not bother with it further */
    return 1;
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
}

int EVP_PKEY_public_check(EVP_PKEY_CTX *ctx)
{
    EVP_PKEY *pkey = ctx->pkey;

    if (pkey == NULL) {
        EVPerr(EVP_F_EVP_PKEY_PUBLIC_CHECK, EVP_R_NO_KEY_SET);
        return 0;
    }

    /* call customized public key check function first */
    if (ctx->pmeth->public_check != NULL)
        return ctx->pmeth->public_check(pkey);

    /* use default public key check function in ameth */
    if (pkey->ameth == NULL || pkey->ameth->pkey_public_check == NULL) {
        EVPerr(EVP_F_EVP_PKEY_PUBLIC_CHECK,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }

    return pkey->ameth->pkey_public_check(pkey);
}

int EVP_PKEY_param_check(EVP_PKEY_CTX *ctx)
{
    EVP_PKEY *pkey = ctx->pkey;

    if (pkey == NULL) {
        EVPerr(EVP_F_EVP_PKEY_PARAM_CHECK, EVP_R_NO_KEY_SET);
        return 0;
    }

    /* call customized param check function first */
    if (ctx->pmeth->param_check != NULL)
        return ctx->pmeth->param_check(pkey);

    /* use default param check function in ameth */
    if (pkey->ameth == NULL || pkey->ameth->pkey_param_check == NULL) {
        EVPerr(EVP_F_EVP_PKEY_PARAM_CHECK,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }

    return pkey->ameth->pkey_param_check(pkey);
}
