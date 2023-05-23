#ifndef HEADER_CURL_VQUIC_NGTCP2_H
#define HEADER_CURL_VQUIC_NGTCP2_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2022, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/

#include "curl_setup.h"
#include "bufq.h"

#ifdef USE_NGTCP2

<<<<<<< HEAD:iModelCore/libsrc/curl/vendor/lib/vquic/ngtcp2.h
#ifdef HAVE_NETINET_UDP_H
#include <netinet/udp.h>
#endif

#include <ngtcp2/ngtcp2_crypto.h>
#include <nghttp3/nghttp3.h>
#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#elif defined(USE_GNUTLS)
#include <gnutls/gnutls.h>
#endif

struct blocked_pkt {
  const uint8_t *pkt;
  size_t pktlen;
  size_t gsolen;
};

struct quicsocket {
  struct connectdata *conn; /* point back to the connection */
  ngtcp2_conn *qconn;
  ngtcp2_cid dcid;
  ngtcp2_cid scid;
  uint32_t version;
  ngtcp2_settings settings;
  ngtcp2_transport_params transport_params;
  ngtcp2_connection_close_error last_error;
  ngtcp2_crypto_conn_ref conn_ref;
#ifdef USE_OPENSSL
  SSL_CTX *sslctx;
  SSL *ssl;
#elif defined(USE_GNUTLS)
  gnutls_certificate_credentials_t cred;
  gnutls_session_t ssl;
#endif
  struct sockaddr_storage local_addr;
  socklen_t local_addrlen;
  bool no_gso;
  uint8_t *pktbuf;
  size_t pktbuflen;
  /* the number of entries in blocked_pkt */
  size_t num_blocked_pkt;
  /* the number of processed entries in blocked_pkt */
  size_t num_blocked_pkt_sent;
  /* the packets blocked by sendmsg (EAGAIN or EWOULDBLOCK) */
  struct blocked_pkt blocked_pkt[2];

  nghttp3_conn *h3conn;
  nghttp3_settings h3settings;
  int qlogfd;
};

#include "urldata.h"

#endif

#endif /* HEADER_CURL_VQUIC_NGTCP2_H */
=======
#define MAX_PKT_BURST 10
#define MAX_UDP_PAYLOAD_SIZE  1452

struct cf_quic_ctx {
  curl_socket_t sockfd; /* connected UDP socket */
  struct sockaddr_storage local_addr; /* address socket is bound to */
  socklen_t local_addrlen; /* length of local address */

  struct bufq sendbuf; /* buffer for sending one or more packets */
  size_t gsolen; /* length of individual packets in send buf */
  size_t split_len; /* if != 0, buffer length after which GSO differs */
  size_t split_gsolen; /* length of individual packets after split_len */
  bool no_gso; /* do not use gso on sending */
};

CURLcode vquic_ctx_init(struct cf_quic_ctx *qctx);
void vquic_ctx_free(struct cf_quic_ctx *qctx);

CURLcode vquic_send_packets(struct Curl_cfilter *cf,
                            struct Curl_easy *data,
                            struct cf_quic_ctx *qctx,
                            const uint8_t *pkt, size_t pktlen, size_t gsolen,
                            size_t *psent);

void vquic_push_blocked_pkt(struct Curl_cfilter *cf,
                            struct cf_quic_ctx *qctx,
                            const uint8_t *pkt, size_t pktlen, size_t gsolen);

CURLcode vquic_send_blocked_pkts(struct Curl_cfilter *cf,
                                 struct Curl_easy *data,
                                 struct cf_quic_ctx *qctx);

CURLcode vquic_send(struct Curl_cfilter *cf, struct Curl_easy *data,
                        struct cf_quic_ctx *qctx, size_t gsolen);

CURLcode vquic_send_tail_split(struct Curl_cfilter *cf, struct Curl_easy *data,
                               struct cf_quic_ctx *qctx, size_t gsolen,
                               size_t tail_len, size_t tail_gsolen);

CURLcode vquic_flush(struct Curl_cfilter *cf, struct Curl_easy *data,
                     struct cf_quic_ctx *qctx);


typedef CURLcode vquic_recv_pkt_cb(const unsigned char *pkt, size_t pktlen,
                                   struct sockaddr_storage *remote_addr,
                                   socklen_t remote_addrlen, int ecn,
                                   void *userp);

CURLcode vquic_recv_packets(struct Curl_cfilter *cf,
                            struct Curl_easy *data,
                            struct cf_quic_ctx *qctx,
                            size_t max_pkts,
                            vquic_recv_pkt_cb *recv_cb, void *userp);

#endif /* !ENABLE_QUIC */

#endif /* HEADER_CURL_VQUIC_QUIC_INT_H */
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290)):iModelCore/libsrc/curl/vendor/lib/vquic/vquic_int.h
