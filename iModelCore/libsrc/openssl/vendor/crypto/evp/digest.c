/*
 * Copyright 1995-2023 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include "internal/cryptlib.h"
#include <openssl/objects.h>
#include <openssl/evp.h>
#include <openssl/engine.h>
#include "crypto/evp.h"
#include "evp_local.h"


static void cleanup_old_md_data(EVP_MD_CTX *ctx, int force)
{
    if (ctx->digest != NULL) {
        if (ctx->digest->cleanup != NULL
                && !EVP_MD_CTX_test_flags(ctx, EVP_MD_CTX_FLAG_CLEANED))
            ctx->digest->cleanup(ctx);
        if (ctx->md_data != NULL && ctx->digest->ctx_size > 0
                && (!EVP_MD_CTX_test_flags(ctx, EVP_MD_CTX_FLAG_REUSE)
                    || force)) {
            OPENSSL_clear_free(ctx->md_data, ctx->digest->ctx_size);
            ctx->md_data = NULL;
        }
    }
}

/* This call frees resources associated with the context */
int EVP_MD_CTX_reset(EVP_MD_CTX *ctx)
{
    if (ctx == NULL)
        return 1;

    /*
     * Don't assume ctx->md_data was cleaned in EVP_Digest_Final, because
     * sometimes only copies of the context are ever finalised.
     */
    cleanup_old_md_data(ctx, 0);

    /*
     * pctx should be freed by the user of EVP_MD_CTX
     * if EVP_MD_CTX_FLAG_KEEP_PKEY_CTX is set
     */
    if (!EVP_MD_CTX_test_flags(ctx, EVP_MD_CTX_FLAG_KEEP_PKEY_CTX))
        EVP_PKEY_CTX_free(ctx->pctx);
#ifndef OPENSSL_NO_ENGINE
    ENGINE_finish(ctx->engine);
#endif
    OPENSSL_cleanse(ctx, sizeof(*ctx));

    return 1;
}

<<<<<<< HEAD
=======
/* This call frees resources associated with the context */
int EVP_MD_CTX_reset(EVP_MD_CTX *ctx)
{
    return evp_md_ctx_reset_ex(ctx, 0);
}

#ifndef FIPS_MODULE
EVP_MD_CTX *evp_md_ctx_new_ex(EVP_PKEY *pkey, const ASN1_OCTET_STRING *id,
                              OSSL_LIB_CTX *libctx, const char *propq)
{
    EVP_MD_CTX *ctx;
    EVP_PKEY_CTX *pctx = NULL;

    if ((ctx = EVP_MD_CTX_new()) == NULL
        || (pctx = EVP_PKEY_CTX_new_from_pkey(libctx, pkey, propq)) == NULL) {
        ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    if (id != NULL && EVP_PKEY_CTX_set1_id(pctx, id->data, id->length) <= 0)
        goto err;

    EVP_MD_CTX_set_pkey_ctx(ctx, pctx);
    return ctx;

 err:
    EVP_PKEY_CTX_free(pctx);
    EVP_MD_CTX_free(ctx);
    return NULL;
}
#endif

>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
EVP_MD_CTX *EVP_MD_CTX_new(void)
{
    return OPENSSL_zalloc(sizeof(EVP_MD_CTX));
}

void EVP_MD_CTX_free(EVP_MD_CTX *ctx)
{
    EVP_MD_CTX_reset(ctx);
    OPENSSL_free(ctx);
}

int EVP_DigestInit(EVP_MD_CTX *ctx, const EVP_MD *type)
{
    EVP_MD_CTX_reset(ctx);
    return EVP_DigestInit_ex(ctx, type, NULL);
}

int EVP_DigestInit_ex(EVP_MD_CTX *ctx, const EVP_MD *type, ENGINE *impl)
{
    EVP_MD_CTX_clear_flags(ctx, EVP_MD_CTX_FLAG_CLEANED);
#ifndef OPENSSL_NO_ENGINE
    /*
     * Whether it's nice or not, "Inits" can be used on "Final"'d contexts so
     * this context may already have an ENGINE! Try to avoid releasing the
     * previous handle, re-querying for an ENGINE, and having a
     * reinitialisation, when it may all be unnecessary.
     */
    if (ctx->engine && ctx->digest &&
        (type == NULL || (type->type == ctx->digest->type)))
        goto skip_to_init;

<<<<<<< HEAD
=======
    /*
     * Ensure an ENGINE left lying around from last time is cleared (the
     * previous check attempted to avoid this if the same ENGINE and
     * EVP_MD could be used).
     */
    ENGINE_finish(ctx->engine);
    ctx->engine = NULL;

    if (impl == NULL)
        tmpimpl = ENGINE_get_digest_engine(type->type);
#endif

    /*
     * If there are engines involved or EVP_MD_CTX_FLAG_NO_INIT is set then we
     * should use legacy handling for now.
     */
    if (impl != NULL
#if !defined(OPENSSL_NO_ENGINE)
            || ctx->engine != NULL
# if !defined(FIPS_MODULE)
            || tmpimpl != NULL
# endif
#endif
            || (ctx->flags & EVP_MD_CTX_FLAG_NO_INIT) != 0
            || (type != NULL && type->origin == EVP_ORIG_METH)
            || (type == NULL && ctx->digest != NULL
                             && ctx->digest->origin == EVP_ORIG_METH)) {
        /* If we were using provided hash before, cleanup algctx */
        if (!evp_md_ctx_free_algctx(ctx))
            return 0;

        if (ctx->digest == ctx->fetched_digest)
            ctx->digest = NULL;
        EVP_MD_free(ctx->fetched_digest);
        ctx->fetched_digest = NULL;
        goto legacy;
    }

    cleanup_old_md_data(ctx, 1);

    /* Start of non-legacy code below */
    if (ctx->digest == type) {
        if (!ossl_assert(type->prov != NULL)) {
            ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
            return 0;
        }
    } else {
        if (!evp_md_ctx_free_algctx(ctx))
            return 0;
    }

    if (type->prov == NULL) {
#ifdef FIPS_MODULE
        /* We only do explicit fetches inside the FIPS module */
        ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
        return 0;
#else
        /* The NULL digest is a special case */
        EVP_MD *provmd = EVP_MD_fetch(NULL,
                                      type->type != NID_undef ? OBJ_nid2sn(type->type)
                                                              : "NULL", "");

        if (provmd == NULL) {
            ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
            return 0;
        }
        type = provmd;
        EVP_MD_free(ctx->fetched_digest);
        ctx->fetched_digest = provmd;
#endif
    }

    if (type->prov != NULL && ctx->fetched_digest != type) {
        if (!EVP_MD_up_ref((EVP_MD *)type)) {
            ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
            return 0;
        }
        EVP_MD_free(ctx->fetched_digest);
        ctx->fetched_digest = (EVP_MD *)type;
    }
    ctx->digest = type;
    if (ctx->algctx == NULL) {
        ctx->algctx = ctx->digest->newctx(ossl_provider_ctx(type->prov));
        if (ctx->algctx == NULL) {
            ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
            return 0;
        }
    }

    if (ctx->digest->dinit == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
        return 0;
    }

    return ctx->digest->dinit(ctx->algctx, params);

    /* Code below to be removed when legacy support is dropped. */
 legacy:

#if !defined(OPENSSL_NO_ENGINE) && !defined(FIPS_MODULE)
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
    if (type) {
        /*
         * Ensure an ENGINE left lying around from last time is cleared (the
         * previous check attempted to avoid this if the same ENGINE and
         * EVP_MD could be used).
         */
        ENGINE_finish(ctx->engine);
        if (impl != NULL) {
            if (!ENGINE_init(impl)) {
                EVPerr(EVP_F_EVP_DIGESTINIT_EX, EVP_R_INITIALIZATION_ERROR);
                return 0;
            }
        } else {
            /* Ask if an ENGINE is reserved for this job */
            impl = ENGINE_get_digest_engine(type->type);
        }
        if (impl != NULL) {
            /* There's an ENGINE for this job ... (apparently) */
            const EVP_MD *d = ENGINE_get_digest(impl, type->type);

            if (d == NULL) {
                EVPerr(EVP_F_EVP_DIGESTINIT_EX, EVP_R_INITIALIZATION_ERROR);
                ENGINE_finish(impl);
                return 0;
            }
            /* We'll use the ENGINE's private digest definition */
            type = d;
            /*
             * Store the ENGINE functional reference so we know 'type' came
             * from an ENGINE and we need to release it when done.
             */
            ctx->engine = impl;
        } else
            ctx->engine = NULL;
    } else {
        if (!ctx->digest) {
            EVPerr(EVP_F_EVP_DIGESTINIT_EX, EVP_R_NO_DIGEST_SET);
            return 0;
        }
        type = ctx->digest;
    }
#endif
    if (ctx->digest != type) {
        cleanup_old_md_data(ctx, 1);

        ctx->digest = type;
        if (!(ctx->flags & EVP_MD_CTX_FLAG_NO_INIT) && type->ctx_size) {
            ctx->update = type->update;
            ctx->md_data = OPENSSL_zalloc(type->ctx_size);
            if (ctx->md_data == NULL) {
<<<<<<< HEAD
                EVPerr(EVP_F_EVP_DIGESTINIT_EX, ERR_R_MALLOC_FAILURE);
=======
                ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
                return 0;
            }
        }
    }
#ifndef OPENSSL_NO_ENGINE
 skip_to_init:
#endif
    if (ctx->pctx) {
        int r;
        r = EVP_PKEY_CTX_ctrl(ctx->pctx, -1, EVP_PKEY_OP_TYPE_SIG,
                              EVP_PKEY_CTRL_DIGESTINIT, 0, ctx);
        if (r <= 0 && (r != -2))
            return 0;
    }
    if (ctx->flags & EVP_MD_CTX_FLAG_NO_INIT)
        return 1;
    return ctx->digest->init(ctx);
}

int EVP_DigestUpdate(EVP_MD_CTX *ctx, const void *data, size_t count)
{
    if (count == 0)
        return 1;

    return ctx->update(ctx, data, count);
}

/* The caller can assume that this removes any secret data from the context */
int EVP_DigestFinal(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *size)
{
    int ret;
    ret = EVP_DigestFinal_ex(ctx, md, size);
    EVP_MD_CTX_reset(ctx);
    return ret;
}

/* The caller can assume that this removes any secret data from the context */
int EVP_DigestFinal_ex(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *size)
{
    int ret;

<<<<<<< HEAD
    OPENSSL_assert(ctx->digest->md_size <= EVP_MAX_MD_SIZE);
=======
    if (ctx->digest == NULL)
        return 0;

    sz = EVP_MD_get_size(ctx->digest);
    if (sz < 0)
        return 0;
    mdsize = sz;
    if (ctx->digest->prov == NULL)
        goto legacy;

    if (ctx->digest->dfinal == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_FINAL_ERROR);
        return 0;
    }

    ret = ctx->digest->dfinal(ctx->algctx, md, &size, mdsize);

    if (isize != NULL) {
        if (size <= UINT_MAX) {
            *isize = (unsigned int)size;
        } else {
            ERR_raise(ERR_LIB_EVP, EVP_R_FINAL_ERROR);
            ret = 0;
        }
    }

    return ret;

    /* Code below to be removed when legacy support is dropped. */
 legacy:
    OPENSSL_assert(mdsize <= EVP_MAX_MD_SIZE);
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
    ret = ctx->digest->final(ctx, md);
    if (size != NULL)
        *size = ctx->digest->md_size;
    if (ctx->digest->cleanup) {
        ctx->digest->cleanup(ctx);
        EVP_MD_CTX_set_flags(ctx, EVP_MD_CTX_FLAG_CLEANED);
    }
    OPENSSL_cleanse(ctx->md_data, ctx->digest->ctx_size);
    return ret;
}

int EVP_DigestFinalXOF(EVP_MD_CTX *ctx, unsigned char *md, size_t size)
{
    int ret = 0;

    if (ctx->digest->flags & EVP_MD_FLAG_XOF
        && size <= INT_MAX
        && ctx->digest->md_ctrl(ctx, EVP_MD_CTRL_XOF_LEN, (int)size, NULL)) {
        ret = ctx->digest->final(ctx, md);

        if (ctx->digest->cleanup != NULL) {
            ctx->digest->cleanup(ctx);
            EVP_MD_CTX_set_flags(ctx, EVP_MD_CTX_FLAG_CLEANED);
        }
        OPENSSL_cleanse(ctx->md_data, ctx->digest->ctx_size);
    } else {
        EVPerr(EVP_F_EVP_DIGESTFINALXOF, EVP_R_NOT_XOF_OR_INVALID_LENGTH);
    }

    return ret;
}

int EVP_MD_CTX_copy(EVP_MD_CTX *out, const EVP_MD_CTX *in)
{
    EVP_MD_CTX_reset(out);
    return EVP_MD_CTX_copy_ex(out, in);
}

int EVP_MD_CTX_copy_ex(EVP_MD_CTX *out, const EVP_MD_CTX *in)
{
    unsigned char *tmp_buf;
    if ((in == NULL) || (in->digest == NULL)) {
        EVPerr(EVP_F_EVP_MD_CTX_COPY_EX, EVP_R_INPUT_NOT_INITIALIZED);
        return 0;
    }
#ifndef OPENSSL_NO_ENGINE
    /* Make sure it's safe to copy a digest context using an ENGINE */
    if (in->engine && !ENGINE_init(in->engine)) {
        EVPerr(EVP_F_EVP_MD_CTX_COPY_EX, ERR_R_ENGINE_LIB);
        return 0;
    }
#endif

    if (out->digest == in->digest) {
        tmp_buf = out->md_data;
        EVP_MD_CTX_set_flags(out, EVP_MD_CTX_FLAG_REUSE);
    } else
        tmp_buf = NULL;
    EVP_MD_CTX_reset(out);
    memcpy(out, in, sizeof(*out));

    /* copied EVP_MD_CTX should free the copied EVP_PKEY_CTX */
    EVP_MD_CTX_clear_flags(out, EVP_MD_CTX_FLAG_KEEP_PKEY_CTX);

    /* Null these variables, since they are getting fixed up
     * properly below.  Anything else may cause a memleak and/or
     * double free if any of the memory allocations below fail
     */
    out->md_data = NULL;
    out->pctx = NULL;

    if (in->md_data && out->digest->ctx_size) {
        if (tmp_buf)
            out->md_data = tmp_buf;
        else {
            out->md_data = OPENSSL_malloc(out->digest->ctx_size);
            if (out->md_data == NULL) {
<<<<<<< HEAD
                EVPerr(EVP_F_EVP_MD_CTX_COPY_EX, ERR_R_MALLOC_FAILURE);
=======
                ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
                return 0;
            }
        }
        memcpy(out->md_data, in->md_data, out->digest->ctx_size);
    }

    out->update = in->update;

    if (in->pctx) {
        out->pctx = EVP_PKEY_CTX_dup(in->pctx);
        if (!out->pctx) {
            EVP_MD_CTX_reset(out);
            return 0;
        }
    }

    if (out->digest->copy)
        return out->digest->copy(out, in);

    return 1;
}

int EVP_Digest(const void *data, size_t count,
               unsigned char *md, unsigned int *size, const EVP_MD *type,
               ENGINE *impl)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    int ret;

    if (ctx == NULL)
        return 0;
    EVP_MD_CTX_set_flags(ctx, EVP_MD_CTX_FLAG_ONESHOT);
    ret = EVP_DigestInit_ex(ctx, type, impl)
        && EVP_DigestUpdate(ctx, data, count)
        && EVP_DigestFinal_ex(ctx, md, size);
    EVP_MD_CTX_free(ctx);

    return ret;
}

int EVP_MD_CTX_ctrl(EVP_MD_CTX *ctx, int cmd, int p1, void *p2)
{
    if (ctx->digest && ctx->digest->md_ctrl) {
        int ret = ctx->digest->md_ctrl(ctx, cmd, p1, p2);
        if (ret <= 0)
            return 0;
        return 1;
    }
<<<<<<< HEAD
    return 0;
=======

    if (ctx->digest != NULL && ctx->digest->prov == NULL)
        goto legacy;

    switch (cmd) {
    case EVP_MD_CTRL_XOF_LEN:
        sz = (size_t)p1;
        params[0] = OSSL_PARAM_construct_size_t(OSSL_DIGEST_PARAM_XOFLEN, &sz);
        break;
    case EVP_MD_CTRL_MICALG:
        set_params = 0;
        params[0] = OSSL_PARAM_construct_utf8_string(OSSL_DIGEST_PARAM_MICALG,
                                                     p2, p1 ? p1 : 9999);
        break;
    case EVP_CTRL_SSL3_MASTER_SECRET:
        params[0] = OSSL_PARAM_construct_octet_string(OSSL_DIGEST_PARAM_SSL3_MS,
                                                      p2, p1);
        break;
    default:
        goto conclude;
    }

    if (set_params)
        ret = EVP_MD_CTX_set_params(ctx, params);
    else
        ret = EVP_MD_CTX_get_params(ctx, params);
    goto conclude;


    /* Code below to be removed when legacy support is dropped. */
 legacy:
    if (ctx->digest->md_ctrl == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_CTRL_NOT_IMPLEMENTED);
        return 0;
    }

    ret = ctx->digest->md_ctrl(ctx, cmd, p1, p2);
 conclude:
    if (ret <= 0)
        return 0;
    return ret;
}

EVP_MD *evp_md_new(void)
{
    EVP_MD *md = OPENSSL_zalloc(sizeof(*md));

    if (md != NULL) {
        md->lock = CRYPTO_THREAD_lock_new();
        if (md->lock == NULL) {
            OPENSSL_free(md);
            return NULL;
        }
        md->refcnt = 1;
    }
    return md;
}

/*
 * FIPS module note: since internal fetches will be entirely
 * provider based, we know that none of its code depends on legacy
 * NIDs or any functionality that use them.
 */
#ifndef FIPS_MODULE
static void set_legacy_nid(const char *name, void *vlegacy_nid)
{
    int nid;
    int *legacy_nid = vlegacy_nid;
    /*
     * We use lowest level function to get the associated method, because
     * higher level functions such as EVP_get_digestbyname() have changed
     * to look at providers too.
     */
    const void *legacy_method = OBJ_NAME_get(name, OBJ_NAME_TYPE_MD_METH);

    if (*legacy_nid == -1)       /* We found a clash already */
        return;

    if (legacy_method == NULL)
        return;
    nid = EVP_MD_nid(legacy_method);
    if (*legacy_nid != NID_undef && *legacy_nid != nid) {
        *legacy_nid = -1;
        return;
    }
    *legacy_nid = nid;
}
#endif

static int evp_md_cache_constants(EVP_MD *md)
{
    int ok, xof = 0, algid_absent = 0;
    size_t blksz = 0;
    size_t mdsize = 0;
    OSSL_PARAM params[5];

    params[0] = OSSL_PARAM_construct_size_t(OSSL_DIGEST_PARAM_BLOCK_SIZE, &blksz);
    params[1] = OSSL_PARAM_construct_size_t(OSSL_DIGEST_PARAM_SIZE, &mdsize);
    params[2] = OSSL_PARAM_construct_int(OSSL_DIGEST_PARAM_XOF, &xof);
    params[3] = OSSL_PARAM_construct_int(OSSL_DIGEST_PARAM_ALGID_ABSENT,
                                         &algid_absent);
    params[4] = OSSL_PARAM_construct_end();
    ok = evp_do_md_getparams(md, params) > 0;
    if (mdsize > INT_MAX || blksz > INT_MAX)
        ok = 0;
    if (ok) {
        md->block_size = (int)blksz;
        md->md_size = (int)mdsize;
        if (xof)
            md->flags |= EVP_MD_FLAG_XOF;
        if (algid_absent)
            md->flags |= EVP_MD_FLAG_DIGALGID_ABSENT;
    }
    return ok;
}

static void *evp_md_from_algorithm(int name_id,
                                   const OSSL_ALGORITHM *algodef,
                                   OSSL_PROVIDER *prov)
{
    const OSSL_DISPATCH *fns = algodef->implementation;
    EVP_MD *md = NULL;
    int fncnt = 0;

    /* EVP_MD_fetch() will set the legacy NID if available */
    if ((md = evp_md_new()) == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

#ifndef FIPS_MODULE
    md->type = NID_undef;
    if (!evp_names_do_all(prov, name_id, set_legacy_nid, &md->type)
            || md->type == -1) {
        ERR_raise(ERR_LIB_EVP, ERR_R_INTERNAL_ERROR);
        EVP_MD_free(md);
        return NULL;
    }
#endif

    md->name_id = name_id;
    if ((md->type_name = ossl_algorithm_get1_first_name(algodef)) == NULL) {
        EVP_MD_free(md);
        return NULL;
    }
    md->description = algodef->algorithm_description;

    for (; fns->function_id != 0; fns++) {
        switch (fns->function_id) {
        case OSSL_FUNC_DIGEST_NEWCTX:
            if (md->newctx == NULL) {
                md->newctx = OSSL_FUNC_digest_newctx(fns);
                fncnt++;
            }
            break;
        case OSSL_FUNC_DIGEST_INIT:
            if (md->dinit == NULL) {
                md->dinit = OSSL_FUNC_digest_init(fns);
                fncnt++;
            }
            break;
        case OSSL_FUNC_DIGEST_UPDATE:
            if (md->dupdate == NULL) {
                md->dupdate = OSSL_FUNC_digest_update(fns);
                fncnt++;
            }
            break;
        case OSSL_FUNC_DIGEST_FINAL:
            if (md->dfinal == NULL) {
                md->dfinal = OSSL_FUNC_digest_final(fns);
                fncnt++;
            }
            break;
        case OSSL_FUNC_DIGEST_DIGEST:
            if (md->digest == NULL)
                md->digest = OSSL_FUNC_digest_digest(fns);
            /* We don't increment fnct for this as it is stand alone */
            break;
        case OSSL_FUNC_DIGEST_FREECTX:
            if (md->freectx == NULL) {
                md->freectx = OSSL_FUNC_digest_freectx(fns);
                fncnt++;
            }
            break;
        case OSSL_FUNC_DIGEST_DUPCTX:
            if (md->dupctx == NULL)
                md->dupctx = OSSL_FUNC_digest_dupctx(fns);
            break;
        case OSSL_FUNC_DIGEST_GET_PARAMS:
            if (md->get_params == NULL)
                md->get_params = OSSL_FUNC_digest_get_params(fns);
            break;
        case OSSL_FUNC_DIGEST_SET_CTX_PARAMS:
            if (md->set_ctx_params == NULL)
                md->set_ctx_params = OSSL_FUNC_digest_set_ctx_params(fns);
            break;
        case OSSL_FUNC_DIGEST_GET_CTX_PARAMS:
            if (md->get_ctx_params == NULL)
                md->get_ctx_params = OSSL_FUNC_digest_get_ctx_params(fns);
            break;
        case OSSL_FUNC_DIGEST_GETTABLE_PARAMS:
            if (md->gettable_params == NULL)
                md->gettable_params = OSSL_FUNC_digest_gettable_params(fns);
            break;
        case OSSL_FUNC_DIGEST_SETTABLE_CTX_PARAMS:
            if (md->settable_ctx_params == NULL)
                md->settable_ctx_params =
                    OSSL_FUNC_digest_settable_ctx_params(fns);
            break;
        case OSSL_FUNC_DIGEST_GETTABLE_CTX_PARAMS:
            if (md->gettable_ctx_params == NULL)
                md->gettable_ctx_params =
                    OSSL_FUNC_digest_gettable_ctx_params(fns);
            break;
        }
    }
    if ((fncnt != 0 && fncnt != 5)
        || (fncnt == 0 && md->digest == NULL)) {
        /*
         * In order to be a consistent set of functions we either need the
         * whole set of init/update/final etc functions or none of them.
         * The "digest" function can standalone. We at least need one way to
         * generate digests.
         */
        EVP_MD_free(md);
        ERR_raise(ERR_LIB_EVP, EVP_R_INVALID_PROVIDER_FUNCTIONS);
        return NULL;
    }
    md->prov = prov;
    if (prov != NULL)
        ossl_provider_up_ref(prov);

    if (!evp_md_cache_constants(md)) {
        EVP_MD_free(md);
        ERR_raise(ERR_LIB_EVP, EVP_R_CACHE_CONSTANTS_FAILED);
        md = NULL;
    }

    return md;
}

static int evp_md_up_ref(void *md)
{
    return EVP_MD_up_ref(md);
}

static void evp_md_free(void *md)
{
    EVP_MD_free(md);
}

EVP_MD *EVP_MD_fetch(OSSL_LIB_CTX *ctx, const char *algorithm,
                     const char *properties)
{
    EVP_MD *md =
        evp_generic_fetch(ctx, OSSL_OP_DIGEST, algorithm, properties,
                          evp_md_from_algorithm, evp_md_up_ref, evp_md_free);

    return md;
}

int EVP_MD_up_ref(EVP_MD *md)
{
    int ref = 0;

    if (md->origin == EVP_ORIG_DYNAMIC)
        CRYPTO_UP_REF(&md->refcnt, &ref, md->lock);
    return 1;
}

void EVP_MD_free(EVP_MD *md)
{
    int i;

    if (md == NULL || md->origin != EVP_ORIG_DYNAMIC)
        return;

    CRYPTO_DOWN_REF(&md->refcnt, &i, md->lock);
    if (i > 0)
        return;
    evp_md_free_int(md);
}

void EVP_MD_do_all_provided(OSSL_LIB_CTX *libctx,
                            void (*fn)(EVP_MD *mac, void *arg),
                            void *arg)
{
    evp_generic_do_all(libctx, OSSL_OP_DIGEST,
                       (void (*)(void *, void *))fn, arg,
                       evp_md_from_algorithm, evp_md_up_ref, evp_md_free);
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
}
