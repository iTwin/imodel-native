/*
<<<<<<< HEAD
 * Copyright 1995-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
=======
 * Copyright 1995-2023 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "../ssl_local.h"
#include "record_local.h"

void SSL3_BUFFER_set_data(SSL3_BUFFER *b, const unsigned char *d, size_t n)
{
    if (d != NULL)
        memcpy(b->buf, d, n);
    b->left = n;
    b->offset = 0;
}

/*
 * Clear the contents of an SSL3_BUFFER but retain any memory allocated. Also
 * retains the default_len setting
 */
void SSL3_BUFFER_clear(SSL3_BUFFER *b)
{
    b->offset = 0;
    b->left = 0;
}

void SSL3_BUFFER_release(SSL3_BUFFER *b)
{
    OPENSSL_free(b->buf);
    b->buf = NULL;
}

int ssl3_setup_read_buffer(SSL *s)
{
    unsigned char *p;
    size_t len, align = 0, headerlen;
    SSL3_BUFFER *b;

    b = RECORD_LAYER_get_rbuf(&s->rlayer);

    if (SSL_IS_DTLS(s))
        headerlen = DTLS1_RT_HEADER_LENGTH;
    else
        headerlen = SSL3_RT_HEADER_LENGTH;

#if defined(SSL3_ALIGN_PAYLOAD) && SSL3_ALIGN_PAYLOAD!=0
    align = (-SSL3_RT_HEADER_LENGTH) & (SSL3_ALIGN_PAYLOAD - 1);
#endif

    if (b->buf == NULL) {
        len = SSL3_RT_MAX_PLAIN_LENGTH
            + SSL3_RT_MAX_ENCRYPTED_OVERHEAD + headerlen + align;
#ifndef OPENSSL_NO_COMP
        if (ssl_allow_compression(s))
            len += SSL3_RT_MAX_COMPRESSED_OVERHEAD;
#endif
<<<<<<< HEAD
=======

        /* Ensure our buffer is large enough to support all our pipelines */
        if (s->max_pipelines > 1)
            len *= s->max_pipelines;

>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
        if (b->default_len > len)
            len = b->default_len;
        if ((p = OPENSSL_malloc(len)) == NULL) {
            /*
             * We've got a malloc failure, and we're still initialising buffers.
             * We assume we're so doomed that we won't even be able to send an
             * alert.
             */
<<<<<<< HEAD
            SSLfatal(s, SSL_AD_NO_ALERT, SSL_F_SSL3_SETUP_READ_BUFFER,
                     ERR_R_MALLOC_FAILURE);
=======
            SSLfatal(s, SSL_AD_NO_ALERT, ERR_R_MALLOC_FAILURE);
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
            return 0;
        }
        b->buf = p;
        b->len = len;
    }

    return 1;
}

int ssl3_setup_write_buffer(SSL *s, size_t numwpipes, size_t len)
{
    unsigned char *p;
    size_t align = 0, headerlen;
    SSL3_BUFFER *wb;
    size_t currpipe;

    s->rlayer.numwpipes = numwpipes;

    if (len == 0) {
        if (SSL_IS_DTLS(s))
            headerlen = DTLS1_RT_HEADER_LENGTH + 1;
        else
            headerlen = SSL3_RT_HEADER_LENGTH;

#if defined(SSL3_ALIGN_PAYLOAD) && SSL3_ALIGN_PAYLOAD!=0
        align = SSL3_ALIGN_PAYLOAD - 1;
#endif

        len = ssl_get_max_send_fragment(s)
<<<<<<< HEAD
            + SSL3_RT_SEND_MAX_ENCRYPTED_OVERHEAD + headerlen + align;
=======
            + SSL3_RT_SEND_MAX_ENCRYPTED_OVERHEAD + headerlen + align
            + SSL_RT_MAX_CIPHER_BLOCK_SIZE /* Explicit IV allowance */;
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
#ifndef OPENSSL_NO_COMP
        if (ssl_allow_compression(s))
            len += SSL3_RT_MAX_COMPRESSED_OVERHEAD;
#endif
<<<<<<< HEAD
=======
        /*
         * We don't need to add an allowance for eivlen here since empty
         * fragments only occur when we don't have an explicit IV
         */
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
        if (!(s->options & SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS))
            len += headerlen + align + SSL3_RT_SEND_MAX_ENCRYPTED_OVERHEAD;
    }

    wb = RECORD_LAYER_get_wbuf(&s->rlayer);
    for (currpipe = 0; currpipe < numwpipes; currpipe++) {
        SSL3_BUFFER *thiswb = &wb[currpipe];

<<<<<<< HEAD
        if (thiswb->buf != NULL && thiswb->len != len) {
=======
        if (thiswb->len != len) {
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
            OPENSSL_free(thiswb->buf);
            thiswb->buf = NULL;         /* force reallocation */
        }

        if (thiswb->buf == NULL) {
<<<<<<< HEAD
            p = OPENSSL_malloc(len);
            if (p == NULL) {
                s->rlayer.numwpipes = currpipe;
                /*
                 * We've got a malloc failure, and we're still initialising
                 * buffers. We assume we're so doomed that we won't even be able
                 * to send an alert.
                 */
                SSLfatal(s, SSL_AD_NO_ALERT,
                         SSL_F_SSL3_SETUP_WRITE_BUFFER, ERR_R_MALLOC_FAILURE);
                return 0;
=======
            if (s->wbio == NULL || !BIO_get_ktls_send(s->wbio)) {
                p = OPENSSL_malloc(len);
                if (p == NULL) {
                    s->rlayer.numwpipes = currpipe;
                    /*
                     * We've got a malloc failure, and we're still initialising
                     * buffers. We assume we're so doomed that we won't even be able
                     * to send an alert.
                     */
                    SSLfatal(s, SSL_AD_NO_ALERT, ERR_R_MALLOC_FAILURE);
                    return 0;
                }
            } else {
                p = NULL;
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
            }
            memset(thiswb, 0, sizeof(SSL3_BUFFER));
            thiswb->buf = p;
            thiswb->len = len;
        }
    }

    return 1;
}

int ssl3_setup_buffers(SSL *s)
{
    if (!ssl3_setup_read_buffer(s)) {
        /* SSLfatal() already called */
        return 0;
    }
    if (!ssl3_setup_write_buffer(s, 1, 0)) {
        /* SSLfatal() already called */
        return 0;
    }
    return 1;
}

int ssl3_release_write_buffer(SSL *s)
{
    SSL3_BUFFER *wb;
    size_t pipes;

    pipes = s->rlayer.numwpipes;
    while (pipes > 0) {
        wb = &RECORD_LAYER_get_wbuf(&s->rlayer)[pipes - 1];

<<<<<<< HEAD
        OPENSSL_free(wb->buf);
=======
        if (SSL3_BUFFER_is_app_buffer(wb))
            SSL3_BUFFER_set_app_buffer(wb, 0);
        else
            OPENSSL_free(wb->buf);
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
        wb->buf = NULL;
        pipes--;
    }
    s->rlayer.numwpipes = 0;
    return 1;
}

int ssl3_release_read_buffer(SSL *s)
{
    SSL3_BUFFER *b;

    b = RECORD_LAYER_get_rbuf(&s->rlayer);
<<<<<<< HEAD
=======
    if (s->options & SSL_OP_CLEANSE_PLAINTEXT)
        OPENSSL_cleanse(b->buf, b->len);
>>>>>>> 56ac539c (copy over openssl 3.1 (#276))
    OPENSSL_free(b->buf);
    b->buf = NULL;
    return 1;
}
