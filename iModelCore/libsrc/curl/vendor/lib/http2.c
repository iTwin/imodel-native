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

#ifdef USE_NGHTTP2
#include <stdint.h>
#include <nghttp2/nghttp2.h>
#include "urldata.h"
#include "bufq.h"
#include "http1.h"
#include "http2.h"
#include "http.h"
#include "sendf.h"
#include "select.h"
#include "curl_base64.h"
#include "strcase.h"
#include "multiif.h"
#include "url.h"
<<<<<<< HEAD
=======
#include "urlapi-int.h"
#include "cfilters.h"
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
#include "connect.h"
#include "strtoofft.h"
#include "strdup.h"
#include "transfer.h"
#include "dynbuf.h"
#include "headers.h"
/* The last 3 #include files should be in this order */
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

#if (NGHTTP2_VERSION_NUM < 0x010c00)
#error too old nghttp2 version, upgrade!
#endif

#ifdef CURL_DISABLE_VERBOSE_STRINGS
#define nghttp2_session_callbacks_set_error_callback(x,y)
#endif

#if (NGHTTP2_VERSION_NUM >= 0x010c00)
#define NGHTTP2_HAS_SET_LOCAL_WINDOW_SIZE 1
#endif


<<<<<<< HEAD
#ifdef DEBUG_HTTP2
#define H2BUGF(x) x
#else
#define H2BUGF(x) do { } while(0)
#endif

static ssize_t http2_recv(struct Curl_easy *data, int sockindex,
                          char *mem, size_t len, CURLcode *err);
static bool http2_connisdead(struct Curl_easy *data,
                             struct connectdata *conn);
static int h2_session_send(struct Curl_easy *data,
                           nghttp2_session *h2);
static int h2_process_pending_input(struct Curl_easy *data,
                                    struct http_conn *httpc,
                                    CURLcode *err);

/*
 * Curl_http2_init_state() is called when the easy handle is created and
 * allows for HTTP/2 specific init of state.
 */
void Curl_http2_init_state(struct UrlState *state)
{
  state->stream_weight = NGHTTP2_DEFAULT_WEIGHT;
}

/*
 * Curl_http2_init_userset() is called when the easy handle is created and
 * allows for HTTP/2 specific user-set fields.
 */
void Curl_http2_init_userset(struct UserDefined *set)
{
  set->stream_weight = NGHTTP2_DEFAULT_WEIGHT;
}

static int http2_getsock(struct Curl_easy *data,
                         struct connectdata *conn,
                         curl_socket_t *sock)
{
  const struct http_conn *c = &conn->proto.httpc;
  struct SingleRequest *k = &data->req;
  int bitmap = GETSOCK_BLANK;
  struct HTTP *stream = data->req.p.http;

  sock[0] = conn->sock[FIRSTSOCKET];

  if(!(k->keepon & KEEP_RECV_PAUSE))
    /* Unless paused - in a HTTP/2 connection we can basically always get a
       frame so we should always be ready for one */
    bitmap |= GETSOCK_READSOCK(FIRSTSOCKET);

  /* we're (still uploading OR the HTTP/2 layer wants to send data) AND
     there's a window to send data in */
  if((((k->keepon & (KEEP_SEND|KEEP_SEND_PAUSE)) == KEEP_SEND) ||
      nghttp2_session_want_write(c->h2)) &&
     (nghttp2_session_get_remote_window_size(c->h2) &&
      nghttp2_session_get_stream_remote_window_size(c->h2,
                                                    stream->stream_id)))
    bitmap |= GETSOCK_WRITESOCK(FIRSTSOCKET);

  return bitmap;
}

/*
 * http2_stream_free() free HTTP2 stream related data
 */
static void http2_stream_free(struct HTTP *http)
{
  if(http) {
    Curl_dyn_free(&http->header_recvbuf);
    for(; http->push_headers_used > 0; --http->push_headers_used) {
      free(http->push_headers[http->push_headers_used - 1]);
    }
    free(http->push_headers);
    http->push_headers = NULL;
  }
=======
/* buffer dimensioning:
 * use 16K as chunk size, as that fits H2 DATA frames well */
#define H2_CHUNK_SIZE           (16 * 1024)
/* this is how much we want "in flight" for a stream */
#define H2_STREAM_WINDOW_SIZE   (512 * 1024)
/* on receving from TLS, we prep for holding a full stream window */
#define H2_NW_RECV_CHUNKS       (H2_STREAM_WINDOW_SIZE / H2_CHUNK_SIZE)
/* on send into TLS, we just want to accumulate small frames */
#define H2_NW_SEND_CHUNKS       1
/* stream recv/send chunks are a result of window / chunk sizes */
#define H2_STREAM_RECV_CHUNKS   (H2_STREAM_WINDOW_SIZE / H2_CHUNK_SIZE)
#define H2_STREAM_SEND_CHUNKS   (H2_STREAM_WINDOW_SIZE / H2_CHUNK_SIZE)
/* spare chunks we keep for a full window */
#define H2_STREAM_POOL_SPARES   (H2_STREAM_WINDOW_SIZE / H2_CHUNK_SIZE)

/* We need to accomodate the max number of streams with their window
 * sizes on the overall connection. Streams might become PAUSED which
 * will block their received QUOTA in the connection window. And if we
 * run out of space, the server is blocked from sending us any data.
 * See #10988 for an issue with this. */
#define HTTP2_HUGE_WINDOW_SIZE (100 * H2_STREAM_WINDOW_SIZE)

#define H2_SETTINGS_IV_LEN  3
#define H2_BINSETTINGS_LEN 80

static int populate_settings(nghttp2_settings_entry *iv,
                             struct Curl_easy *data)
{
  iv[0].settings_id = NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS;
  iv[0].value = Curl_multi_max_concurrent_streams(data->multi);

  iv[1].settings_id = NGHTTP2_SETTINGS_INITIAL_WINDOW_SIZE;
  iv[1].value = H2_STREAM_WINDOW_SIZE;

  iv[2].settings_id = NGHTTP2_SETTINGS_ENABLE_PUSH;
  iv[2].value = data->multi->push_cb != NULL;

  return 3;
}

static size_t populate_binsettings(uint8_t *binsettings,
                                   struct Curl_easy *data)
{
  nghttp2_settings_entry iv[H2_SETTINGS_IV_LEN];
  int ivlen;

  ivlen = populate_settings(iv, data);
  /* this returns number of bytes it wrote */
  return nghttp2_pack_settings_payload(binsettings, H2_BINSETTINGS_LEN,
                                       iv, ivlen);
}

struct cf_h2_ctx {
  nghttp2_session *h2;
  uint32_t max_concurrent_streams;
  /* The easy handle used in the current filter call, cleared at return */
  struct cf_call_data call_data;

  struct bufq inbufq;           /* network input */
  struct bufq outbufq;          /* network output */
  struct bufc_pool stream_bufcp; /* spares for stream buffers */

  size_t drain_total; /* sum of all stream's UrlState drain */
  int32_t goaway_error;
  int32_t last_stream_id;
  BIT(conn_closed);
  BIT(goaway);
  BIT(enable_push);
};

/* How to access `call_data` from a cf_h2 filter */
#define CF_CTX_CALL_DATA(cf)  \
  ((struct cf_h2_ctx *)(cf)->ctx)->call_data

static void cf_h2_ctx_clear(struct cf_h2_ctx *ctx)
{
  struct cf_call_data save = ctx->call_data;

  if(ctx->h2) {
    nghttp2_session_del(ctx->h2);
  }
  Curl_bufq_free(&ctx->inbufq);
  Curl_bufq_free(&ctx->outbufq);
  Curl_bufcp_free(&ctx->stream_bufcp);
  memset(ctx, 0, sizeof(*ctx));
  ctx->call_data = save;
}

static void cf_h2_ctx_free(struct cf_h2_ctx *ctx)
{
  if(ctx) {
    cf_h2_ctx_clear(ctx);
    free(ctx);
  }
}

static CURLcode h2_progress_egress(struct Curl_cfilter *cf,
                                  struct Curl_easy *data);

/**
 * All about the H3 internals of a stream
 */
struct stream_ctx {
  /*********** for HTTP/2 we store stream-local data here *************/
  int32_t id; /* HTTP/2 protocol identifier for stream */
  struct bufq recvbuf; /* response buffer */
  struct bufq sendbuf; /* request buffer */
  struct dynhds resp_trailers; /* response trailer fields */
  size_t resp_hds_len; /* amount of response header bytes in recvbuf */
  curl_off_t upload_left; /* number of request bytes left to upload */

  char **push_headers;       /* allocated array */
  size_t push_headers_used;  /* number of entries filled in */
  size_t push_headers_alloc; /* number of entries allocated */

  int status_code; /* HTTP response status code */
  uint32_t error; /* stream error code */
  bool closed; /* TRUE on stream close */
  bool reset;  /* TRUE on stream reset */
  bool close_handled; /* TRUE if stream closure is handled by libcurl */
  bool bodystarted;
};

#define H2_STREAM_CTX(d)    ((struct stream_ctx *)(((d) && (d)->req.p.http)? \
                             ((struct HTTP *)(d)->req.p.http)->h2_ctx \
                               : NULL))
#define H2_STREAM_LCTX(d)   ((struct HTTP *)(d)->req.p.http)->h2_ctx
#define H2_STREAM_ID(d)     (H2_STREAM_CTX(d)? \
                             H2_STREAM_CTX(d)->id : -2)

/*
 * Mark this transfer to get "drained".
 */
static void drain_stream(struct Curl_cfilter *cf,
                         struct Curl_easy *data,
                         struct stream_ctx *stream)
{
  unsigned char bits;

  (void)cf;
  bits = CURL_CSELECT_IN;
  if(stream->upload_left)
    bits |= CURL_CSELECT_OUT;
  if(data->state.dselect_bits != bits) {
    data->state.dselect_bits = bits;
    Curl_expire(data, 0, EXPIRE_RUN_NOW);
  }
}

static CURLcode http2_data_setup(struct Curl_cfilter *cf,
                                 struct Curl_easy *data,
                                 struct stream_ctx **pstream)
{
  struct cf_h2_ctx *ctx = cf->ctx;
  struct stream_ctx *stream;

  (void)cf;
  DEBUGASSERT(data);
  if(!data->req.p.http) {
    failf(data, "initialization failure, transfer not http initialized");
    return CURLE_FAILED_INIT;
  }
  stream = H2_STREAM_CTX(data);
  if(stream) {
    *pstream = stream;
    return CURLE_OK;
  }

  stream = calloc(1, sizeof(*stream));
  if(!stream)
    return CURLE_OUT_OF_MEMORY;

  stream->id = -1;
  Curl_bufq_initp(&stream->sendbuf, &ctx->stream_bufcp,
                  H2_STREAM_SEND_CHUNKS, BUFQ_OPT_NONE);
  Curl_bufq_initp(&stream->recvbuf, &ctx->stream_bufcp,
                  H2_STREAM_RECV_CHUNKS, BUFQ_OPT_SOFT_LIMIT);
  Curl_dynhds_init(&stream->resp_trailers, 0, DYN_H2_TRAILERS);
  stream->resp_hds_len = 0;
  stream->bodystarted = FALSE;
  stream->status_code = -1;
  stream->closed = FALSE;
  stream->close_handled = FALSE;
  stream->error = NGHTTP2_NO_ERROR;
  stream->upload_left = 0;

  H2_STREAM_LCTX(data) = stream;
  *pstream = stream;
  return CURLE_OK;
}

static void http2_data_done(struct Curl_cfilter *cf,
                            struct Curl_easy *data, bool premature)
{
  struct cf_h2_ctx *ctx = cf->ctx;
  struct stream_ctx *stream = H2_STREAM_CTX(data);

  DEBUGASSERT(ctx);
  (void)premature;
  if(!stream)
    return;

  if(ctx->h2) {
    if(!stream->closed && stream->id > 0) {
      /* RST_STREAM */
      DEBUGF(LOG_CF(data, cf, "[h2sid=%d] premature DATA_DONE, RST stream",
                    stream->id));
      if(!nghttp2_submit_rst_stream(ctx->h2, NGHTTP2_FLAG_NONE,
                                    stream->id, NGHTTP2_STREAM_CLOSED))
        (void)nghttp2_session_send(ctx->h2);
    }
    if(!Curl_bufq_is_empty(&stream->recvbuf)) {
      /* Anything in the recvbuf is still being counted
       * in stream and connection window flow control. Need
       * to free that space or the connection window might get
       * exhausted eventually. */
      nghttp2_session_consume(ctx->h2, stream->id,
                              Curl_bufq_len(&stream->recvbuf));
      /* give WINDOW_UPATE a chance to be sent, but ignore any error */
      (void)h2_progress_egress(cf, data);
    }

    /* -1 means unassigned and 0 means cleared */
    if(nghttp2_session_get_stream_user_data(ctx->h2, stream->id)) {
      int rv = nghttp2_session_set_stream_user_data(ctx->h2,
                                                    stream->id, 0);
      if(rv) {
        infof(data, "http/2: failed to clear user_data for stream %u",
              stream->id);
        DEBUGASSERT(0);
      }
    }
  }

  Curl_bufq_free(&stream->sendbuf);
  Curl_bufq_free(&stream->recvbuf);
  Curl_dynhds_free(&stream->resp_trailers);
  if(stream->push_headers) {
    /* if they weren't used and then freed before */
    for(; stream->push_headers_used > 0; --stream->push_headers_used) {
      free(stream->push_headers[stream->push_headers_used - 1]);
    }
    free(stream->push_headers);
    stream->push_headers = NULL;
  }

  free(stream);
  H2_STREAM_LCTX(data) = NULL;
}

static int h2_client_new(struct Curl_cfilter *cf,
                         nghttp2_session_callbacks *cbs)
{
  struct cf_h2_ctx *ctx = cf->ctx;
  nghttp2_option *o;

  int rc = nghttp2_option_new(&o);
  if(rc)
    return rc;
  /* We handle window updates ourself to enfore buffer limits */
  nghttp2_option_set_no_auto_window_update(o, 1);
#if NGHTTP2_VERSION_NUM >= 0x013200
  /* with 1.50.0 */
  /* turn off RFC 9113 leading and trailing white spaces validation against
     HTTP field value. */
  nghttp2_option_set_no_rfc9113_leading_and_trailing_ws_validation(o, 1);
#endif
  rc = nghttp2_session_client_new2(&ctx->h2, cbs, cf, o);
  nghttp2_option_del(o);
  return rc;
}

static ssize_t nw_in_reader(void *reader_ctx,
                              unsigned char *buf, size_t buflen,
                              CURLcode *err)
{
  struct Curl_cfilter *cf = reader_ctx;
  struct Curl_easy *data = CF_DATA_CURRENT(cf);

  return Curl_conn_cf_recv(cf->next, data, (char *)buf, buflen, err);
}

static ssize_t nw_out_writer(void *writer_ctx,
                             const unsigned char *buf, size_t buflen,
                             CURLcode *err)
{
  struct Curl_cfilter *cf = writer_ctx;
  struct Curl_easy *data = CF_DATA_CURRENT(cf);

  return Curl_conn_cf_send(cf->next, data, (const char *)buf, buflen, err);
}

static ssize_t send_callback(nghttp2_session *h2,
                             const uint8_t *mem, size_t length, int flags,
                             void *userp);
static int on_frame_recv(nghttp2_session *session, const nghttp2_frame *frame,
                         void *userp);
static int on_data_chunk_recv(nghttp2_session *session, uint8_t flags,
                              int32_t stream_id,
                              const uint8_t *mem, size_t len, void *userp);
static int on_stream_close(nghttp2_session *session, int32_t stream_id,
                           uint32_t error_code, void *userp);
static int on_begin_headers(nghttp2_session *session,
                            const nghttp2_frame *frame, void *userp);
static int on_header(nghttp2_session *session, const nghttp2_frame *frame,
                     const uint8_t *name, size_t namelen,
                     const uint8_t *value, size_t valuelen,
                     uint8_t flags,
                     void *userp);
static int error_callback(nghttp2_session *session, const char *msg,
                          size_t len, void *userp);

/*
 * multi_connchanged() is called to tell that there is a connection in
 * this multi handle that has changed state (multiplexing become possible, the
 * number of allowed streams changed or similar), and a subsequent use of this
 * multi handle should move CONNECT_PEND handles back to CONNECT to have them
 * retry.
 */
static void multi_connchanged(struct Curl_multi *multi)
{
  multi->recheckstate = TRUE;
}

/*
 * Initialize the cfilter context
 */
static CURLcode cf_h2_ctx_init(struct Curl_cfilter *cf,
                               struct Curl_easy *data,
                               bool via_h1_upgrade)
{
  struct cf_h2_ctx *ctx = cf->ctx;
  struct stream_ctx *stream;
  CURLcode result = CURLE_OUT_OF_MEMORY;
  int rc;
  nghttp2_session_callbacks *cbs = NULL;

  DEBUGASSERT(!ctx->h2);
  Curl_bufcp_init(&ctx->stream_bufcp, H2_CHUNK_SIZE, H2_STREAM_POOL_SPARES);
  Curl_bufq_initp(&ctx->inbufq, &ctx->stream_bufcp, H2_NW_RECV_CHUNKS, 0);
  Curl_bufq_initp(&ctx->outbufq, &ctx->stream_bufcp, H2_NW_SEND_CHUNKS, 0);
  ctx->last_stream_id = 2147483647;

  rc = nghttp2_session_callbacks_new(&cbs);
  if(rc) {
    failf(data, "Couldn't initialize nghttp2 callbacks");
    goto out;
  }

  nghttp2_session_callbacks_set_send_callback(cbs, send_callback);
  nghttp2_session_callbacks_set_on_frame_recv_callback(cbs, on_frame_recv);
  nghttp2_session_callbacks_set_on_data_chunk_recv_callback(
    cbs, on_data_chunk_recv);
  nghttp2_session_callbacks_set_on_stream_close_callback(cbs, on_stream_close);
  nghttp2_session_callbacks_set_on_begin_headers_callback(
    cbs, on_begin_headers);
  nghttp2_session_callbacks_set_on_header_callback(cbs, on_header);
  nghttp2_session_callbacks_set_error_callback(cbs, error_callback);

  /* The nghttp2 session is not yet setup, do it */
  rc = h2_client_new(cf, cbs);
  if(rc) {
    failf(data, "Couldn't initialize nghttp2");
    goto out;
  }
  ctx->max_concurrent_streams = DEFAULT_MAX_CONCURRENT_STREAMS;

  if(via_h1_upgrade) {
    /* HTTP/1.1 Upgrade issued. H2 Settings have already been submitted
     * in the H1 request and we upgrade from there. This stream
     * is opened implicitly as #1. */
    uint8_t binsettings[H2_BINSETTINGS_LEN];
    size_t  binlen; /* length of the binsettings data */

    binlen = populate_binsettings(binsettings, data);

    result = http2_data_setup(cf, data, &stream);
    if(result)
      goto out;
    DEBUGASSERT(stream);
    stream->id = 1;
    /* queue SETTINGS frame (again) */
    rc = nghttp2_session_upgrade2(ctx->h2, binsettings, binlen,
                                  data->state.httpreq == HTTPREQ_HEAD,
                                  NULL);
    if(rc) {
      failf(data, "nghttp2_session_upgrade2() failed: %s(%d)",
            nghttp2_strerror(rc), rc);
      result = CURLE_HTTP2;
      goto out;
    }

    rc = nghttp2_session_set_stream_user_data(ctx->h2, stream->id,
                                              data);
    if(rc) {
      infof(data, "http/2: failed to set user_data for stream %u",
            stream->id);
      DEBUGASSERT(0);
    }
  }
  else {
    nghttp2_settings_entry iv[H2_SETTINGS_IV_LEN];
    int ivlen;

    ivlen = populate_settings(iv, data);
    rc = nghttp2_submit_settings(ctx->h2, NGHTTP2_FLAG_NONE,
                                 iv, ivlen);
    if(rc) {
      failf(data, "nghttp2_submit_settings() failed: %s(%d)",
            nghttp2_strerror(rc), rc);
      result = CURLE_HTTP2;
      goto out;
    }
  }

  rc = nghttp2_session_set_local_window_size(ctx->h2, NGHTTP2_FLAG_NONE, 0,
                                             HTTP2_HUGE_WINDOW_SIZE);
  if(rc) {
    failf(data, "nghttp2_session_set_local_window_size() failed: %s(%d)",
          nghttp2_strerror(rc), rc);
    result = CURLE_HTTP2;
    goto out;
  }

  /* all set, traffic will be send on connect */
  result = CURLE_OK;

out:
  if(cbs)
    nghttp2_session_callbacks_del(cbs);
  return result;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
}

/*
 * Disconnects *a* connection used for HTTP/2. It might be an old one from the
 * connection cache and not the "main" one. Don't touch the easy handle!
 */

static CURLcode http2_disconnect(struct Curl_easy *data,
                                 struct connectdata *conn,
                                 bool dead_connection)
{
  struct http_conn *c = &conn->proto.httpc;
  (void)dead_connection;
#ifndef DEBUG_HTTP2
  (void)data;
#endif

  H2BUGF(infof(data, "HTTP/2 DISCONNECT starts now"));

  nghttp2_session_del(c->h2);
  Curl_safefree(c->inbuf);

  H2BUGF(infof(data, "HTTP/2 DISCONNECT done"));

  return CURLE_OK;
}

/*
 * Processes pending input left in network input buffer.
 * This function returns 0 if it succeeds, or -1 and error code will
 * be assigned to *err.
 */
static int h2_process_pending_input(struct Curl_cfilter *cf,
                                    struct Curl_easy *data,
                                    CURLcode *err)
{
  struct cf_h2_ctx *ctx = cf->ctx;
  const unsigned char *buf;
  size_t blen;
  ssize_t rv;

  while(Curl_bufq_peek(&ctx->inbufq, &buf, &blen)) {

    rv = nghttp2_session_mem_recv(ctx->h2, (const uint8_t *)buf, blen);
    if(rv < 0) {
      failf(data,
            "process_pending_input: nghttp2_session_mem_recv() returned "
            "%zd:%s", rv, nghttp2_strerror((int)rv));
      *err = CURLE_RECV_ERROR;
      return -1;
    }
    Curl_bufq_skip(&ctx->inbufq, (size_t)rv);
    if(Curl_bufq_is_empty(&ctx->inbufq)) {
      break;
    }
    else {
      DEBUGF(LOG_CF(data, cf, "process_pending_input: %zu bytes left "
                    "in connection buffer", Curl_bufq_len(&ctx->inbufq)));
    }
  }

  if(nghttp2_session_check_request_allowed(ctx->h2) == 0) {
    /* No more requests are allowed in the current session, so
       the connection may not be reused. This is set when a
       GOAWAY frame has been received or when the limit of stream
       identifiers has been reached. */
    connclose(cf->conn, "http/2: No new requests allowed");
  }

  return 0;
}

/*
 * The server may send us data at any point (e.g. PING frames). Therefore,
 * we cannot assume that an HTTP/2 socket is dead just because it is readable.
 *
 * Instead, if it is readable, run Curl_connalive() to peek at the socket
 * and distinguish between closed and data.
 */
static bool http2_connisdead(struct Curl_easy *data, struct connectdata *conn)
{
  int sval;
  bool dead = TRUE;

  if(conn->bits.close)
    return TRUE;

<<<<<<< HEAD
  sval = SOCKET_READABLE(conn->sock[FIRSTSOCKET], 0);
  if(sval == 0) {
    /* timeout */
    dead = FALSE;
  }
  else if(sval & CURL_CSELECT_ERR) {
    /* socket is in an error state */
    dead = TRUE;
  }
  else if(sval & CURL_CSELECT_IN) {
    /* readable with no error. could still be closed */
    dead = !Curl_connalive(conn);
    if(!dead) {
      /* This happens before we've sent off a request and the connection is
         not in use by any other transfer, there shouldn't be any data here,
         only "protocol frames" */
      CURLcode result;
      struct http_conn *httpc = &conn->proto.httpc;
      ssize_t nread = -1;
      if(httpc->recv_underlying)
        /* if called "too early", this pointer isn't setup yet! */
        nread = ((Curl_recv *)httpc->recv_underlying)(
          data, FIRSTSOCKET, httpc->inbuf, H2_BUFSIZE, &result);
      if(nread != -1) {
        H2BUGF(infof(data,
                     "%d bytes stray data read before trying h2 connection",
                     (int)nread));
        httpc->nread_inbuf = 0;
        httpc->inbuflen = nread;
        if(h2_process_pending_input(data, httpc, &result) < 0)
          /* immediate error, considered dead */
          dead = TRUE;
=======
  if(*input_pending) {
    /* This happens before we've sent off a request and the connection is
       not in use by any other transfer, there shouldn't be any data here,
       only "protocol frames" */
    CURLcode result;
    ssize_t nread = -1;

    *input_pending = FALSE;
    Curl_attach_connection(data, cf->conn);
    nread = Curl_bufq_slurp(&ctx->inbufq, nw_in_reader, cf, &result);
    if(nread != -1) {
      DEBUGF(LOG_CF(data, cf, "%zd bytes stray data read before trying "
                    "h2 connection", nread));
      if(h2_process_pending_input(cf, data, &result) < 0)
        /* immediate error, considered dead */
        alive = FALSE;
      else {
        alive = !should_close_session(ctx);
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
      }
      else
        /* the read failed so let's say this is dead anyway */
        dead = TRUE;
    }
  }

  return dead;
}

/*
 * Set the transfer that is currently using this HTTP/2 connection.
 */
static void set_transfer(struct http_conn *c,
                         struct Curl_easy *data)
{
  c->trnsfr = data;
}

/*
 * Get the transfer that is currently using this HTTP/2 connection.
 */
static struct Curl_easy *get_transfer(struct http_conn *c)
{
  DEBUGASSERT(c && c->trnsfr);
  return c->trnsfr;
}

static unsigned int http2_conncheck(struct Curl_easy *data,
                                    struct connectdata *conn,
                                    unsigned int checks_to_perform)
{
  unsigned int ret_val = CONNRESULT_NONE;
  struct http_conn *c = &conn->proto.httpc;
  int rc;
  bool send_frames = false;

  if(checks_to_perform & CONNCHECK_ISDEAD) {
    if(http2_connisdead(data, conn))
      ret_val |= CONNRESULT_DEAD;
  }

  if(checks_to_perform & CONNCHECK_KEEPALIVE) {
    struct curltime now = Curl_now();
    timediff_t elapsed = Curl_timediff(now, conn->keepalive);

    if(elapsed > data->set.upkeep_interval_ms) {
      /* Perform an HTTP/2 PING */
      rc = nghttp2_submit_ping(c->h2, 0, ZERO_NULL);
      if(!rc) {
        /* Successfully added a PING frame to the session. Need to flag this
           so the frame is sent. */
        send_frames = true;
      }
      else {
       failf(data, "nghttp2_submit_ping() failed: %s(%d)",
             nghttp2_strerror(rc), rc);
      }

      conn->keepalive = now;
    }
  }

  if(send_frames) {
    set_transfer(c, data); /* set the transfer */
    rc = nghttp2_session_send(c->h2);
    if(rc)
      failf(data, "nghttp2_session_send() failed: %s(%d)",
            nghttp2_strerror(rc), rc);
  }

  return ret_val;
}

/* called from http_setup_conn */
void Curl_http2_setup_req(struct Curl_easy *data)
{
  struct HTTP *http = data->req.p.http;
  http->bodystarted = FALSE;
  http->status_code = -1;
  http->pausedata = NULL;
  http->pauselen = 0;
  http->closed = FALSE;
  http->close_handled = FALSE;
  http->mem = NULL;
  http->len = 0;
  http->memlen = 0;
  http->error = NGHTTP2_NO_ERROR;
}

/* called from http_setup_conn */
void Curl_http2_setup_conn(struct connectdata *conn)
{
  conn->proto.httpc.settings.max_concurrent_streams =
    DEFAULT_MAX_CONCURRENT_STREAMS;
}

/*
 * HTTP2 handler interface. This isn't added to the general list of protocols
 * but will be used at run-time when the protocol is dynamically switched from
 * HTTP to HTTP2.
 */
static const struct Curl_handler Curl_handler_http2 = {
  "HTTP",                               /* scheme */
  ZERO_NULL,                            /* setup_connection */
  Curl_http,                            /* do_it */
  Curl_http_done,                       /* done */
  ZERO_NULL,                            /* do_more */
  ZERO_NULL,                            /* connect_it */
  ZERO_NULL,                            /* connecting */
  ZERO_NULL,                            /* doing */
  http2_getsock,                        /* proto_getsock */
  http2_getsock,                        /* doing_getsock */
  ZERO_NULL,                            /* domore_getsock */
  http2_getsock,                        /* perform_getsock */
  http2_disconnect,                     /* disconnect */
  ZERO_NULL,                            /* readwrite */
  http2_conncheck,                      /* connection_check */
  ZERO_NULL,                            /* attach connection */
  PORT_HTTP,                            /* defport */
  CURLPROTO_HTTP,                       /* protocol */
  CURLPROTO_HTTP,                       /* family */
  PROTOPT_STREAM                        /* flags */
};

static const struct Curl_handler Curl_handler_http2_ssl = {
  "HTTPS",                              /* scheme */
  ZERO_NULL,                            /* setup_connection */
  Curl_http,                            /* do_it */
  Curl_http_done,                       /* done */
  ZERO_NULL,                            /* do_more */
  ZERO_NULL,                            /* connect_it */
  ZERO_NULL,                            /* connecting */
  ZERO_NULL,                            /* doing */
  http2_getsock,                        /* proto_getsock */
  http2_getsock,                        /* doing_getsock */
  ZERO_NULL,                            /* domore_getsock */
  http2_getsock,                        /* perform_getsock */
  http2_disconnect,                     /* disconnect */
  ZERO_NULL,                            /* readwrite */
  http2_conncheck,                      /* connection_check */
  ZERO_NULL,                            /* attach connection */
  PORT_HTTP,                            /* defport */
  CURLPROTO_HTTPS,                      /* protocol */
  CURLPROTO_HTTP,                       /* family */
  PROTOPT_SSL | PROTOPT_STREAM          /* flags */
};

/*
 * Store nghttp2 version info in this buffer.
 */
void Curl_http2_ver(char *p, size_t len)
{
  nghttp2_info *h2 = nghttp2_version(0);
  (void)msnprintf(p, len, "nghttp2/%s", h2->version_str);
}

<<<<<<< HEAD
=======
static CURLcode nw_out_flush(struct Curl_cfilter *cf,
                             struct Curl_easy *data)
{
  struct cf_h2_ctx *ctx = cf->ctx;
  ssize_t nwritten;
  CURLcode result;

  (void)data;
  if(Curl_bufq_is_empty(&ctx->outbufq))
    return CURLE_OK;

  DEBUGF(LOG_CF(data, cf, "h2 conn flush %zu bytes",
                Curl_bufq_len(&ctx->outbufq)));
  nwritten = Curl_bufq_pass(&ctx->outbufq, nw_out_writer, cf, &result);
  if(nwritten < 0 && result != CURLE_AGAIN) {
    return result;
  }
  return CURLE_OK;
}

>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
/*
 * The implementation of nghttp2_send_callback type. Here we write |data| with
 * size |length| to the network and return the number of bytes actually
 * written. See the documentation of nghttp2_send_callback for the details.
 */
static ssize_t send_callback(nghttp2_session *h2,
                             const uint8_t *mem, size_t length, int flags,
                             void *userp)
{
<<<<<<< HEAD
  struct connectdata *conn = (struct connectdata *)userp;
  struct http_conn *c = &conn->proto.httpc;
  struct Curl_easy *data = get_transfer(c);
  ssize_t written;
=======
  struct Curl_cfilter *cf = userp;
  struct cf_h2_ctx *ctx = cf->ctx;
  struct Curl_easy *data = CF_DATA_CURRENT(cf);
  ssize_t nwritten;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
  CURLcode result = CURLE_OK;

  (void)h2;
  (void)flags;

<<<<<<< HEAD
  if(!c->send_underlying)
    /* called before setup properly! */
    return NGHTTP2_ERR_CALLBACK_FAILURE;

  written = ((Curl_send*)c->send_underlying)(data, FIRSTSOCKET,
                                             mem, length, &result);

  if(result == CURLE_AGAIN) {
    return NGHTTP2_ERR_WOULDBLOCK;
  }

  if(written == -1) {
=======
  nwritten = Curl_bufq_write_pass(&ctx->outbufq, buf, blen,
                                  nw_out_writer, cf, &result);
  if(nwritten < 0) {
    if(result == CURLE_AGAIN) {
      return NGHTTP2_ERR_WOULDBLOCK;
    }
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
    failf(data, "Failed sending HTTP2 data");
    return NGHTTP2_ERR_CALLBACK_FAILURE;
  }

  if(!nwritten)
    return NGHTTP2_ERR_WOULDBLOCK;

  return nwritten;
}


/* We pass a pointer to this struct in the push callback, but the contents of
   the struct are hidden from the user. */
struct curl_pushheaders {
  struct Curl_easy *data;
  const nghttp2_push_promise *frame;
};

/*
 * push header access function. Only to be used from within the push callback
 */
char *curl_pushheader_bynum(struct curl_pushheaders *h, size_t num)
{
  /* Verify that we got a good easy handle in the push header struct, mostly to
     detect rubbish input fast(er). */
  if(!h || !GOOD_EASY_HANDLE(h->data))
    return NULL;
  else {
    struct stream_ctx *stream = H2_STREAM_CTX(h->data);
    if(stream && num < stream->push_headers_used)
      return stream->push_headers[num];
  }
  return NULL;
}

/*
 * push header access function. Only to be used from within the push callback
 */
char *curl_pushheader_byname(struct curl_pushheaders *h, const char *header)
{
  struct stream_ctx *stream;
  size_t len;
  size_t i;
  /* Verify that we got a good easy handle in the push header struct,
     mostly to detect rubbish input fast(er). Also empty header name
     is just a rubbish too. We have to allow ":" at the beginning of
     the header, but header == ":" must be rejected. If we have ':' in
     the middle of header, it could be matched in middle of the value,
     this is because we do prefix match.*/
  if(!h || !GOOD_EASY_HANDLE(h->data) || !header || !header[0] ||
     !strcmp(header, ":") || strchr(header + 1, ':'))
    return NULL;

  stream = H2_STREAM_CTX(h->data);
  if(!stream)
    return NULL;

  len = strlen(header);
  for(i = 0; i<stream->push_headers_used; i++) {
    if(!strncmp(header, stream->push_headers[i], len)) {
      /* sub-match, make sure that it is followed by a colon */
      if(stream->push_headers[i][len] != ':')
        continue;
      return &stream->push_headers[i][len + 1];
    }
  }
  return NULL;
}

<<<<<<< HEAD
/*
 * This specific transfer on this connection has been "drained".
 */
static void drained_transfer(struct Curl_easy *data,
                             struct http_conn *httpc)
{
  DEBUGASSERT(httpc->drain_total >= data->state.drain);
  httpc->drain_total -= data->state.drain;
  data->state.drain = 0;
}

/*
 * Mark this transfer to get "drained".
 */
static void drain_this(struct Curl_easy *data,
                       struct http_conn *httpc)
{
  data->state.drain++;
  httpc->drain_total++;
  DEBUGASSERT(httpc->drain_total >= data->state.drain);
}

static struct Curl_easy *duphandle(struct Curl_easy *data)
=======
static struct Curl_easy *h2_duphandle(struct Curl_cfilter *cf,
                                      struct Curl_easy *data)
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
{
  struct Curl_easy *second = curl_easy_duphandle(data);
  if(second) {
    /* setup the request struct */
    struct HTTP *http = calloc(1, sizeof(struct HTTP));
    if(!http) {
      (void)Curl_close(&second);
    }
    else {
      struct stream_ctx *second_stream;

      second->req.p.http = http;
<<<<<<< HEAD
      Curl_dyn_init(&http->header_recvbuf, DYN_H2_HEADERS);
      Curl_http2_setup_req(second);
      second->state.stream_weight = data->state.stream_weight;
=======
      http2_data_setup(cf, second, &second_stream);
      second->state.priority.weight = data->state.priority.weight;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
    }
  }
  return second;
}

static int set_transfer_url(struct Curl_easy *data,
                            struct curl_pushheaders *hp)
{
  const char *v;
  CURLUcode uc;
  char *url = NULL;
  int rc = 0;
  CURLU *u = curl_url();

  if(!u)
    return 5;

  v = curl_pushheader_byname(hp, HTTP_PSEUDO_SCHEME);
  if(v) {
    uc = curl_url_set(u, CURLUPART_SCHEME, v, 0);
    if(uc) {
      rc = 1;
      goto fail;
    }
  }

  v = curl_pushheader_byname(hp, HTTP_PSEUDO_AUTHORITY);
  if(v) {
    uc = Curl_url_set_authority(u, v, CURLU_DISALLOW_USER);
    if(uc) {
      rc = 2;
      goto fail;
    }
  }

  v = curl_pushheader_byname(hp, HTTP_PSEUDO_PATH);
  if(v) {
    uc = curl_url_set(u, CURLUPART_PATH, v, 0);
    if(uc) {
      rc = 3;
      goto fail;
    }
  }

  uc = curl_url_get(u, CURLUPART_URL, &url, 0);
  if(uc)
    rc = 4;
  fail:
  curl_url_cleanup(u);
  if(rc)
    return rc;

  if(data->state.url_alloc)
    free(data->state.url);
  data->state.url_alloc = TRUE;
  data->state.url = url;
  return 0;
}

<<<<<<< HEAD
static int push_promise(struct Curl_easy *data,
                        struct connectdata *conn,
=======
static void discard_newhandle(struct Curl_cfilter *cf,
                              struct Curl_easy *newhandle)
{
  if(!newhandle->req.p.http) {
    http2_data_done(cf, newhandle, TRUE);
    newhandle->req.p.http = NULL;
  }
  (void)Curl_close(&newhandle);
}

static int push_promise(struct Curl_cfilter *cf,
                        struct Curl_easy *data,
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
                        const nghttp2_push_promise *frame)
{
  int rv; /* one of the CURL_PUSH_* defines */
<<<<<<< HEAD
  H2BUGF(infof(data, "PUSH_PROMISE received, stream %u",
               frame->promised_stream_id));
=======

  DEBUGF(LOG_CF(data, cf, "[h2sid=%d] PUSH_PROMISE received",
                frame->promised_stream_id));
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
  if(data->multi->push_cb) {
    struct stream_ctx *stream;
    struct stream_ctx *newstream;
    struct curl_pushheaders heads;
    CURLMcode rc;
<<<<<<< HEAD
    struct http_conn *httpc;
=======
    CURLcode result;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
    size_t i;
    /* clone the parent */
    struct Curl_easy *newhandle = duphandle(data);
    if(!newhandle) {
      infof(data, "failed to duplicate handle");
      rv = CURL_PUSH_DENY; /* FAIL HARD */
      goto fail;
    }

    heads.data = data;
    heads.frame = frame;
    /* ask the application */
    H2BUGF(infof(data, "Got PUSH_PROMISE, ask application"));

    stream = H2_STREAM_CTX(data);
    if(!stream) {
      failf(data, "Internal NULL stream");
      discard_newhandle(cf, newhandle);
      rv = CURL_PUSH_DENY;
      goto fail;
    }

    rv = set_transfer_url(newhandle, &heads);
    if(rv) {
      discard_newhandle(cf, newhandle);
      rv = CURL_PUSH_DENY;
      goto fail;
    }

    result = http2_data_setup(cf, newhandle, &newstream);
    if(result) {
      failf(data, "error setting up stream: %d", result);
      discard_newhandle(cf, newhandle);
      rv = CURL_PUSH_DENY;
      goto fail;
    }
    DEBUGASSERT(stream);

    Curl_set_in_callback(data, true);
    rv = data->multi->push_cb(data, newhandle,
                              stream->push_headers_used, &heads,
                              data->multi->push_userp);
    Curl_set_in_callback(data, false);

    /* free the headers again */
    for(i = 0; i<stream->push_headers_used; i++)
      free(stream->push_headers[i]);
    free(stream->push_headers);
    stream->push_headers = NULL;
    stream->push_headers_used = 0;

    if(rv) {
      DEBUGASSERT((rv > CURL_PUSH_OK) && (rv <= CURL_PUSH_ERROROUT));
      /* denied, kill off the new handle again */
      discard_newhandle(cf, newhandle);
      goto fail;
    }

    newstream->id = frame->promised_stream_id;
    newhandle->req.maxdownload = -1;
    newhandle->req.size = -1;

    /* approved, add to the multi handle and immediately switch to PERFORM
       state with the given connection !*/
    rc = Curl_multi_add_perform(data->multi, newhandle, conn);
    if(rc) {
      infof(data, "failed to add handle to multi");
      discard_newhandle(cf, newhandle);
      rv = CURL_PUSH_DENY;
      goto fail;
    }

<<<<<<< HEAD
    httpc = &conn->proto.httpc;
    rv = nghttp2_session_set_stream_user_data(httpc->h2,
                                              frame->promised_stream_id,
=======
    rv = nghttp2_session_set_stream_user_data(ctx->h2,
                                              newstream->id,
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
                                              newhandle);
    if(rv) {
      infof(data, "failed to set user_data for stream %u",
            newstream->id);
      DEBUGASSERT(0);
      rv = CURL_PUSH_DENY;
      goto fail;
    }
  }
  else {
    H2BUGF(infof(data, "Got PUSH_PROMISE, ignore it"));
    rv = CURL_PUSH_DENY;
  }
  fail:
  return rv;
}

<<<<<<< HEAD
/*
 * multi_connchanged() is called to tell that there is a connection in
 * this multi handle that has changed state (multiplexing become possible, the
 * number of allowed streams changed or similar), and a subsequent use of this
 * multi handle should move CONNECT_PEND handles back to CONNECT to have them
 * retry.
 */
static void multi_connchanged(struct Curl_multi *multi)
{
  multi->recheckstate = TRUE;
=======
static CURLcode recvbuf_write_hds(struct Curl_cfilter *cf,
                                  struct Curl_easy *data,
                                  const char *buf, size_t blen)
{
  struct stream_ctx *stream = H2_STREAM_CTX(data);
  ssize_t nwritten;
  CURLcode result;

  (void)cf;
  nwritten = Curl_bufq_write(&stream->recvbuf,
                             (const unsigned char *)buf, blen, &result);
  if(nwritten < 0)
    return result;
  stream->resp_hds_len += (size_t)nwritten;
  DEBUGASSERT((size_t)nwritten == blen);
  return CURLE_OK;
}

static CURLcode on_stream_frame(struct Curl_cfilter *cf,
                                struct Curl_easy *data,
                                const nghttp2_frame *frame)
{
  struct cf_h2_ctx *ctx = cf->ctx;
  struct stream_ctx *stream = H2_STREAM_CTX(data);
  int32_t stream_id = frame->hd.stream_id;
  CURLcode result;
  int rv;

  if(!stream) {
    DEBUGF(LOG_CF(data, cf, "[h2sid=%d] No proto pointer", stream_id));
    return CURLE_FAILED_INIT;
  }

  switch(frame->hd.type) {
  case NGHTTP2_DATA:
    DEBUGF(LOG_CF(data, cf, "[h2sid=%d] FRAME[DATA len=%zu pad=%zu], "
                  "buffered=%zu, window=%d/%d",
                  stream_id, frame->hd.length, frame->data.padlen,
                  Curl_bufq_len(&stream->recvbuf),
                  nghttp2_session_get_stream_effective_recv_data_length(
                    ctx->h2, stream->id),
                  nghttp2_session_get_stream_effective_local_window_size(
                    ctx->h2, stream->id)));
    /* If !body started on this stream, then receiving DATA is illegal. */
    if(!stream->bodystarted) {
      rv = nghttp2_submit_rst_stream(ctx->h2, NGHTTP2_FLAG_NONE,
                                     stream_id, NGHTTP2_PROTOCOL_ERROR);

      if(nghttp2_is_fatal(rv)) {
        return CURLE_RECV_ERROR;
      }
    }
    if(frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
      drain_stream(cf, data, stream);
    }
    break;
  case NGHTTP2_HEADERS:
    DEBUGF(LOG_CF(data, cf, "[h2sid=%d] FRAME[HEADERS]", stream_id));
    if(stream->bodystarted) {
      /* Only valid HEADERS after body started is trailer HEADERS.  We
         buffer them in on_header callback. */
      break;
    }

    /* nghttp2 guarantees that :status is received, and we store it to
       stream->status_code. Fuzzing has proven this can still be reached
       without status code having been set. */
    if(stream->status_code == -1)
      return CURLE_RECV_ERROR;

    /* Only final status code signals the end of header */
    if(stream->status_code / 100 != 1) {
      stream->bodystarted = TRUE;
      stream->status_code = -1;
    }

    result = recvbuf_write_hds(cf, data, STRCONST("\r\n"));
    if(result)
      return result;

    DEBUGF(LOG_CF(data, cf, "[h2sid=%d] %zu header bytes",
                  stream_id, Curl_bufq_len(&stream->recvbuf)));
    drain_stream(cf, data, stream);
    break;
  case NGHTTP2_PUSH_PROMISE:
    DEBUGF(LOG_CF(data, cf, "[h2sid=%d] FRAME[PUSH_PROMISE]", stream_id));
    rv = push_promise(cf, data, &frame->push_promise);
    if(rv) { /* deny! */
      DEBUGASSERT((rv > CURL_PUSH_OK) && (rv <= CURL_PUSH_ERROROUT));
      rv = nghttp2_submit_rst_stream(ctx->h2, NGHTTP2_FLAG_NONE,
                                     frame->push_promise.promised_stream_id,
                                     NGHTTP2_CANCEL);
      if(nghttp2_is_fatal(rv))
        return CURLE_SEND_ERROR;
      else if(rv == CURL_PUSH_ERROROUT) {
        DEBUGF(LOG_CF(data, cf, "[h2sid=%d] fail in PUSH_PROMISE received",
                      stream_id));
        return CURLE_RECV_ERROR;
      }
    }
    break;
  case NGHTTP2_RST_STREAM:
    DEBUGF(LOG_CF(data, cf, "[h2sid=%d] FRAME[RST]", stream_id));
    stream->closed = TRUE;
    stream->reset = TRUE;
    drain_stream(cf, data, stream);
    break;
  case NGHTTP2_WINDOW_UPDATE:
    DEBUGF(LOG_CF(data, cf, "[h2sid=%d] FRAME[WINDOW_UPDATE]", stream_id));
    if((data->req.keepon & KEEP_SEND_HOLD) &&
       (data->req.keepon & KEEP_SEND)) {
      data->req.keepon &= ~KEEP_SEND_HOLD;
      drain_stream(cf, data, stream);
      DEBUGF(LOG_CF(data, cf, "[h2sid=%d] un-holding after win update",
                    stream_id));
    }
    break;
  default:
    DEBUGF(LOG_CF(data, cf, "[h2sid=%d] FRAME[%x]",
                  stream_id, frame->hd.type));
    break;
  }
  return CURLE_OK;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
}

static int on_frame_recv(nghttp2_session *session, const nghttp2_frame *frame,
                         void *userp)
{
<<<<<<< HEAD
  struct connectdata *conn = (struct connectdata *)userp;
  struct http_conn *httpc = &conn->proto.httpc;
  struct Curl_easy *data_s = NULL;
  struct HTTP *stream = NULL;
  struct Curl_easy *data = get_transfer(httpc);
  int rv;
  size_t left, ncopy;
=======
  struct Curl_cfilter *cf = userp;
  struct cf_h2_ctx *ctx = cf->ctx;
  struct Curl_easy *data = CF_DATA_CURRENT(cf), *data_s;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
  int32_t stream_id = frame->hd.stream_id;

  if(!stream_id) {
    /* stream ID zero is for connection-oriented stuff */
<<<<<<< HEAD
    if(frame->hd.type == NGHTTP2_SETTINGS) {
      uint32_t max_conn = httpc->settings.max_concurrent_streams;
      H2BUGF(infof(data, "Got SETTINGS"));
      httpc->settings.max_concurrent_streams =
        nghttp2_session_get_remote_settings(
=======
    DEBUGASSERT(data);
    switch(frame->hd.type) {
    case NGHTTP2_SETTINGS: {
      uint32_t max_conn = ctx->max_concurrent_streams;
      DEBUGF(LOG_CF(data, cf, "FRAME[SETTINGS]"));
      ctx->max_concurrent_streams = nghttp2_session_get_remote_settings(
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
          session, NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS);
      httpc->settings.enable_push =
        nghttp2_session_get_remote_settings(
          session, NGHTTP2_SETTINGS_ENABLE_PUSH);
      H2BUGF(infof(data, "MAX_CONCURRENT_STREAMS == %d",
                   httpc->settings.max_concurrent_streams));
      H2BUGF(infof(data, "ENABLE_PUSH == %s",
                   httpc->settings.enable_push?"TRUE":"false"));
      if(max_conn != httpc->settings.max_concurrent_streams) {
        /* only signal change if the value actually changed */
        infof(data,
              "Connection state changed (MAX_CONCURRENT_STREAMS == %u)!",
              httpc->settings.max_concurrent_streams);
        multi_connchanged(data->multi);
      }
<<<<<<< HEAD
=======
      break;
    }
    case NGHTTP2_GOAWAY:
      ctx->goaway = TRUE;
      ctx->goaway_error = frame->goaway.error_code;
      ctx->last_stream_id = frame->goaway.last_stream_id;
      if(data) {
        DEBUGF(LOG_CF(data, cf, "FRAME[GOAWAY, error=%d, last_stream=%u]",
                      ctx->goaway_error, ctx->last_stream_id));
        infof(data, "received GOAWAY, error=%d, last_stream=%u",
                    ctx->goaway_error, ctx->last_stream_id);
        multi_connchanged(data->multi);
      }
      break;
    case NGHTTP2_WINDOW_UPDATE:
      DEBUGF(LOG_CF(data, cf, "FRAME[WINDOW_UPDATE]"));
      break;
    default:
      DEBUGF(LOG_CF(data, cf, "recv frame %x on 0", frame->hd.type));
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
    }
    return 0;
  }

  data_s = nghttp2_session_get_stream_user_data(session, stream_id);
  if(!data_s) {
<<<<<<< HEAD
    H2BUGF(infof(data,
                 "No Curl_easy associated with stream: %u",
                 stream_id));
    return 0;
  }

  stream = data_s->req.p.http;
  if(!stream) {
    H2BUGF(infof(data_s, "No proto pointer for stream: %u",
                 stream_id));
    return NGHTTP2_ERR_CALLBACK_FAILURE;
  }

  H2BUGF(infof(data_s, "on_frame_recv() header %x stream %u",
               frame->hd.type, stream_id));

  switch(frame->hd.type) {
  case NGHTTP2_DATA:
    /* If body started on this stream, then receiving DATA is illegal. */
    if(!stream->bodystarted) {
      rv = nghttp2_submit_rst_stream(session, NGHTTP2_FLAG_NONE,
                                     stream_id, NGHTTP2_PROTOCOL_ERROR);

      if(nghttp2_is_fatal(rv)) {
        return NGHTTP2_ERR_CALLBACK_FAILURE;
      }
    }
    break;
  case NGHTTP2_HEADERS:
    if(stream->bodystarted) {
      /* Only valid HEADERS after body started is trailer HEADERS.  We
         buffer them in on_header callback. */
      break;
    }

    /* nghttp2 guarantees that :status is received, and we store it to
       stream->status_code. Fuzzing has proven this can still be reached
       without status code having been set. */
    if(stream->status_code == -1)
      return NGHTTP2_ERR_CALLBACK_FAILURE;

    /* Only final status code signals the end of header */
    if(stream->status_code / 100 != 1) {
      stream->bodystarted = TRUE;
      stream->status_code = -1;
    }

    result = Curl_dyn_addn(&stream->header_recvbuf, STRCONST("\r\n"));
    if(result)
      return NGHTTP2_ERR_CALLBACK_FAILURE;

    left = Curl_dyn_len(&stream->header_recvbuf) -
      stream->nread_header_recvbuf;
    ncopy = CURLMIN(stream->len, left);

    memcpy(&stream->mem[stream->memlen],
           Curl_dyn_ptr(&stream->header_recvbuf) +
           stream->nread_header_recvbuf,
           ncopy);
    stream->nread_header_recvbuf += ncopy;

    DEBUGASSERT(stream->mem);
    H2BUGF(infof(data_s, "Store %zu bytes headers from stream %u at %p",
                 ncopy, stream_id, stream->mem));

    stream->len -= ncopy;
    stream->memlen += ncopy;

    drain_this(data_s, httpc);
    /* if we receive data for another handle, wake that up */
    if(get_transfer(httpc) != data_s)
      Curl_expire(data_s, 0, EXPIRE_RUN_NOW);
    break;
  case NGHTTP2_PUSH_PROMISE:
    rv = push_promise(data_s, conn, &frame->push_promise);
    if(rv) { /* deny! */
      int h2;
      DEBUGASSERT((rv > CURL_PUSH_OK) && (rv <= CURL_PUSH_ERROROUT));
      h2 = nghttp2_submit_rst_stream(session, NGHTTP2_FLAG_NONE,
                                     frame->push_promise.promised_stream_id,
                                     NGHTTP2_CANCEL);
      if(nghttp2_is_fatal(h2))
        return NGHTTP2_ERR_CALLBACK_FAILURE;
      else if(rv == CURL_PUSH_ERROROUT) {
        DEBUGF(infof(data_s, "Fail the parent stream (too)"));
        return NGHTTP2_ERR_CALLBACK_FAILURE;
      }
    }
    break;
  default:
    H2BUGF(infof(data_s, "Got frame type %x for stream %u",
                 frame->hd.type, stream_id));
    break;
  }
  return 0;
=======
    DEBUGF(LOG_CF(data, cf, "[h2sid=%d] No Curl_easy associated",
                  stream_id));
    return 0;
  }

  return on_stream_frame(cf, data_s, frame)? NGHTTP2_ERR_CALLBACK_FAILURE : 0;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
}

static int on_data_chunk_recv(nghttp2_session *session, uint8_t flags,
                              int32_t stream_id,
                              const uint8_t *mem, size_t len, void *userp)
{
<<<<<<< HEAD
  struct HTTP *stream;
  struct Curl_easy *data_s;
  size_t nread;
  struct connectdata *conn = (struct connectdata *)userp;
  struct http_conn *httpc = &conn->proto.httpc;
  (void)session;
=======
  struct Curl_cfilter *cf = userp;
  struct stream_ctx *stream;
  struct Curl_easy *data_s;
  ssize_t nwritten;
  CURLcode result;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
  (void)flags;

  DEBUGASSERT(stream_id); /* should never be a zero stream ID here */

  /* get the stream from the hash based on Stream ID */
  data_s = nghttp2_session_get_stream_user_data(session, stream_id);
  if(!data_s) {
    /* Receiving a Stream ID not in the hash should not happen - unless
       we have aborted a transfer artificially and there were more data
       in the pipeline. Silently ignore. */
<<<<<<< HEAD
    H2BUGF(fprintf(stderr, "Data for stream %u but it doesn't exist\n",
                   stream_id));
=======
    DEBUGF(LOG_CF(CF_DATA_CURRENT(cf), cf, "[h2sid=%d] Data for unknown",
                  stream_id));
    /* consumed explicitly as no one will read it */
    nghttp2_session_consume(session, stream_id, len);
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
    return 0;
  }

  stream = H2_STREAM_CTX(data_s);
  if(!stream)
    return NGHTTP2_ERR_CALLBACK_FAILURE;

  nwritten = Curl_bufq_write(&stream->recvbuf, mem, len, &result);
  if(nwritten < 0) {
    if(result != CURLE_AGAIN)
      return NGHTTP2_ERR_CALLBACK_FAILURE;

    nwritten = 0;
  }

<<<<<<< HEAD
  drain_this(data_s, &conn->proto.httpc);

  /* if we receive data for another handle, wake that up */
  if(get_transfer(httpc) != data_s)
    Curl_expire(data_s, 0, EXPIRE_RUN_NOW);

  H2BUGF(infof(data_s, "%zu data received for stream %u "
               "(%zu left in buffer %p, total %zu)",
               nread, stream_id,
               stream->len, stream->mem,
               stream->memlen));

  if(nread < len) {
    stream->pausedata = mem + nread;
    stream->pauselen = len - nread;
    H2BUGF(infof(data_s, "NGHTTP2_ERR_PAUSE - %zu bytes out of buffer"
                 ", stream %u",
                 len - nread, stream_id));
    data_s->conn->proto.httpc.pause_stream_id = stream_id;

    return NGHTTP2_ERR_PAUSE;
  }

  /* pause execution of nghttp2 if we received data for another handle
     in order to process them first. */
  if(get_transfer(httpc) != data_s) {
    data_s->conn->proto.httpc.pause_stream_id = stream_id;

    return NGHTTP2_ERR_PAUSE;
  }
=======
  /* if we receive data for another handle, wake that up */
  drain_stream(cf, data_s, stream);
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))

  DEBUGASSERT((size_t)nwritten == len);
  return 0;
}

static int on_stream_close(nghttp2_session *session, int32_t stream_id,
                           uint32_t error_code, void *userp)
{
<<<<<<< HEAD
  struct Curl_easy *data_s;
  struct HTTP *stream;
  struct connectdata *conn = (struct connectdata *)userp;
=======
  struct Curl_cfilter *cf = userp;
  struct Curl_easy *data_s;
  struct stream_ctx *stream;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
  int rv;
  (void)session;
  (void)stream_id;

<<<<<<< HEAD
  if(stream_id) {
    struct http_conn *httpc;
    /* get the stream from the hash based on Stream ID, stream ID zero is for
       connection-oriented stuff */
    data_s = nghttp2_session_get_stream_user_data(session, stream_id);
    if(!data_s) {
      /* We could get stream ID not in the hash.  For example, if we
         decided to reject stream (e.g., PUSH_PROMISE). */
      return 0;
    }
    H2BUGF(infof(data_s, "on_stream_close(), %s (err %d), stream %u",
                 nghttp2_http2_strerror(error_code), error_code, stream_id));
    stream = data_s->req.p.http;
    if(!stream)
      return NGHTTP2_ERR_CALLBACK_FAILURE;

    stream->closed = TRUE;
    httpc = &conn->proto.httpc;
    drain_this(data_s, httpc);
    Curl_expire(data_s, 0, EXPIRE_RUN_NOW);
    stream->error = error_code;
=======
  /* get the stream from the hash based on Stream ID, stream ID zero is for
     connection-oriented stuff */
  data_s = stream_id?
             nghttp2_session_get_stream_user_data(session, stream_id) : NULL;
  if(!data_s) {
    return 0;
  }
  stream = H2_STREAM_CTX(data_s);
  DEBUGF(LOG_CF(data_s, cf, "[h2sid=%d] on_stream_close(), %s (err %d)",
                stream_id, nghttp2_http2_strerror(error_code), error_code));
  if(!stream)
    return NGHTTP2_ERR_CALLBACK_FAILURE;

  stream->closed = TRUE;
  stream->error = error_code;
  if(stream->error)
    stream->reset = TRUE;

  drain_stream(cf, data_s, stream);
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))

    /* remove the entry from the hash as the stream is now gone */
    rv = nghttp2_session_set_stream_user_data(session, stream_id, 0);
    if(rv) {
      infof(data_s, "http/2: failed to clear user_data for stream %u",
            stream_id);
      DEBUGASSERT(0);
    }
    if(stream_id == httpc->pause_stream_id) {
      H2BUGF(infof(data_s, "Stopped the pause stream"));
      httpc->pause_stream_id = 0;
    }
    H2BUGF(infof(data_s, "Removed stream %u hash", stream_id));
    stream->stream_id = 0; /* cleared */
  }
<<<<<<< HEAD
=======
  DEBUGF(LOG_CF(data_s, cf, "[h2sid=%d] closed now", stream_id));
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
  return 0;
}

static int on_begin_headers(nghttp2_session *session,
                            const nghttp2_frame *frame, void *userp)
{
<<<<<<< HEAD
  struct HTTP *stream;
=======
  struct Curl_cfilter *cf = userp;
  struct stream_ctx *stream;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
  struct Curl_easy *data_s = NULL;
  (void)userp;

  data_s = nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
  if(!data_s) {
    return 0;
  }

  H2BUGF(infof(data_s, "on_begin_headers() was called"));

  if(frame->hd.type != NGHTTP2_HEADERS) {
    return 0;
  }

  stream = H2_STREAM_CTX(data_s);
  if(!stream || !stream->bodystarted) {
    return 0;
  }

  return 0;
}

/* frame->hd.type is either NGHTTP2_HEADERS or NGHTTP2_PUSH_PROMISE */
static int on_header(nghttp2_session *session, const nghttp2_frame *frame,
                     const uint8_t *name, size_t namelen,
                     const uint8_t *value, size_t valuelen,
                     uint8_t flags,
                     void *userp)
{
<<<<<<< HEAD
  struct HTTP *stream;
=======
  struct Curl_cfilter *cf = userp;
  struct stream_ctx *stream;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
  struct Curl_easy *data_s;
  int32_t stream_id = frame->hd.stream_id;
  struct connectdata *conn = (struct connectdata *)userp;
  struct http_conn *httpc = &conn->proto.httpc;
  CURLcode result;
  (void)flags;

  DEBUGASSERT(stream_id); /* should never be a zero stream ID here */

  /* get the stream from the hash based on Stream ID */
  data_s = nghttp2_session_get_stream_user_data(session, stream_id);
  if(!data_s)
    /* Receiving a Stream ID not in the hash should not happen, this is an
       internal error more than anything else! */
    return NGHTTP2_ERR_CALLBACK_FAILURE;

  stream = H2_STREAM_CTX(data_s);
  if(!stream) {
    failf(data_s, "Internal NULL stream");
    return NGHTTP2_ERR_CALLBACK_FAILURE;
  }

  /* Store received PUSH_PROMISE headers to be used when the subsequent
     PUSH_PROMISE callback comes */
  if(frame->hd.type == NGHTTP2_PUSH_PROMISE) {
    char *h;

    if(!strcmp(HTTP_PSEUDO_AUTHORITY, (const char *)name)) {
      /* pseudo headers are lower case */
      int rc = 0;
      char *check = aprintf("%s:%d", conn->host.name, conn->remote_port);
      if(!check)
        /* no memory */
        return NGHTTP2_ERR_CALLBACK_FAILURE;
      if(!Curl_strcasecompare(check, (const char *)value) &&
         ((conn->remote_port != conn->given->defport) ||
          !Curl_strcasecompare(conn->host.name, (const char *)value))) {
        /* This is push is not for the same authority that was asked for in
         * the URL. RFC 7540 section 8.2 says: "A client MUST treat a
         * PUSH_PROMISE for which the server is not authoritative as a stream
         * error of type PROTOCOL_ERROR."
         */
        (void)nghttp2_submit_rst_stream(session, NGHTTP2_FLAG_NONE,
                                        stream_id, NGHTTP2_PROTOCOL_ERROR);
        rc = NGHTTP2_ERR_CALLBACK_FAILURE;
      }
      free(check);
      if(rc)
        return rc;
    }

    if(!stream->push_headers) {
      stream->push_headers_alloc = 10;
      stream->push_headers = malloc(stream->push_headers_alloc *
                                    sizeof(char *));
      if(!stream->push_headers)
        return NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE;
      stream->push_headers_used = 0;
    }
    else if(stream->push_headers_used ==
            stream->push_headers_alloc) {
      char **headp;
      if(stream->push_headers_alloc > 1000) {
        /* this is beyond crazy many headers, bail out */
        failf(data_s, "Too many PUSH_PROMISE headers");
        Curl_safefree(stream->push_headers);
        return NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE;
      }
      stream->push_headers_alloc *= 2;
      headp = Curl_saferealloc(stream->push_headers,
                               stream->push_headers_alloc * sizeof(char *));
      if(!headp) {
        stream->push_headers = NULL;
        return NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE;
      }
      stream->push_headers = headp;
    }
    h = aprintf("%s:%s", name, value);
    if(h)
      stream->push_headers[stream->push_headers_used++] = h;
    return 0;
  }

  if(stream->bodystarted) {
    /* This is a trailer */
<<<<<<< HEAD
    H2BUGF(infof(data_s, "h2 trailer: %.*s: %.*s", namelen, name, valuelen,
                 value));
    result = Curl_dyn_addf(&stream->trailer_recvbuf,
                           "%.*s: %.*s\r\n", namelen, name,
                           valuelen, value);
=======
    DEBUGF(LOG_CF(data_s, cf, "[h2sid=%d] trailer: %.*s: %.*s",
                  stream->id,
                  (int)namelen, name,
                  (int)valuelen, value));
    result = Curl_dynhds_add(&stream->resp_trailers,
                             (const char *)name, namelen,
                             (const char *)value, valuelen);
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
    if(result)
      return NGHTTP2_ERR_CALLBACK_FAILURE;

    return 0;
  }

  if(namelen == sizeof(HTTP_PSEUDO_STATUS) - 1 &&
     memcmp(HTTP_PSEUDO_STATUS, name, namelen) == 0) {
    /* nghttp2 guarantees :status is received first and only once. */
    char buffer[32];
    result = Curl_http_decode_status(&stream->status_code,
                                     (const char *)value, valuelen);
    if(result)
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    msnprintf(buffer, sizeof(buffer), HTTP_PSEUDO_STATUS ":%u\r",
              stream->status_code);
    result = Curl_headers_push(data_s, buffer, CURLH_PSEUDO);
    if(result)
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    result = recvbuf_write_hds(cf, data_s, STRCONST("HTTP/2 "));
    if(result)
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    result = recvbuf_write_hds(cf, data_s, (const char *)value, valuelen);
    if(result)
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    /* the space character after the status code is mandatory */
    result = recvbuf_write_hds(cf, data_s, STRCONST(" \r\n"));
    if(result)
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    /* if we receive data for another handle, wake that up */
    if(get_transfer(httpc) != data_s)
      Curl_expire(data_s, 0, EXPIRE_RUN_NOW);

<<<<<<< HEAD
    H2BUGF(infof(data_s, "h2 status: HTTP/2 %03d (easy %p)",
                 stream->status_code, data_s));
=======
    DEBUGF(LOG_CF(data_s, cf, "[h2sid=%d] status: HTTP/2 %03d",
                  stream->id, stream->status_code));
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
    return 0;
  }

  /* nghttp2 guarantees that namelen > 0, and :status was already
     received, and this is not pseudo-header field . */
<<<<<<< HEAD
  /* convert to a HTTP1-style header */
  result = Curl_dyn_addn(&stream->header_recvbuf, name, namelen);
=======
  /* convert to an HTTP1-style header */
  result = recvbuf_write_hds(cf, data_s, (const char *)name, namelen);
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
  if(result)
    return NGHTTP2_ERR_CALLBACK_FAILURE;
  result = recvbuf_write_hds(cf, data_s, STRCONST(": "));
  if(result)
    return NGHTTP2_ERR_CALLBACK_FAILURE;
  result = recvbuf_write_hds(cf, data_s, (const char *)value, valuelen);
  if(result)
    return NGHTTP2_ERR_CALLBACK_FAILURE;
  result = recvbuf_write_hds(cf, data_s, STRCONST("\r\n"));
  if(result)
    return NGHTTP2_ERR_CALLBACK_FAILURE;
  /* if we receive data for another handle, wake that up */
  if(get_transfer(httpc) != data_s)
    Curl_expire(data_s, 0, EXPIRE_RUN_NOW);

<<<<<<< HEAD
  H2BUGF(infof(data_s, "h2 header: %.*s: %.*s", namelen, name, valuelen,
               value));
=======
  DEBUGF(LOG_CF(data_s, cf, "[h2sid=%d] header: %.*s: %.*s",
                stream->id,
                (int)namelen, name,
                (int)valuelen, value));
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))

  return 0; /* 0 is successful */
}

static ssize_t req_body_read_callback(nghttp2_session *session,
                                      int32_t stream_id,
                                      uint8_t *buf, size_t length,
                                      uint32_t *data_flags,
                                      nghttp2_data_source *source,
                                      void *userp)
{
  struct Curl_easy *data_s;
  struct stream_ctx *stream = NULL;
  CURLcode result;
  ssize_t nread;
  (void)source;
  (void)userp;

  if(stream_id) {
    /* get the stream from the hash based on Stream ID, stream ID zero is for
       connection-oriented stuff */
    data_s = nghttp2_session_get_stream_user_data(session, stream_id);
    if(!data_s)
      /* Receiving a Stream ID not in the hash should not happen, this is an
         internal error more than anything else! */
      return NGHTTP2_ERR_CALLBACK_FAILURE;

    stream = H2_STREAM_CTX(data_s);
    if(!stream)
      return NGHTTP2_ERR_CALLBACK_FAILURE;
  }
  else
    return NGHTTP2_ERR_INVALID_ARGUMENT;

  nread = Curl_bufq_read(&stream->sendbuf, buf, length, &result);
  if(nread < 0) {
    if(result != CURLE_AGAIN)
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    nread = 0;
  }

  if(nread > 0 && data_s->state.infilesize != -1)
    stream->upload_left -= nread;

  DEBUGF(LOG_CF(data_s, cf, "[h2sid=%d] req_body_read(len=%zu) left=%zd"
                " -> %zd, %d",
                stream_id, length, stream->upload_left, nread, result));

  if(stream->upload_left == 0)
    *data_flags = NGHTTP2_DATA_FLAG_EOF;
  else if(nread == 0)
    return NGHTTP2_ERR_DEFERRED;

<<<<<<< HEAD
  H2BUGF(infof(data_s, "data_source_read_callback: "
               "returns %zu bytes stream %u",
               nread, stream_id));

=======
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
  return nread;
}

#if !defined(CURL_DISABLE_VERBOSE_STRINGS)
static int error_callback(nghttp2_session *session,
                          const char *msg,
                          size_t len,
                          void *userp)
{
  (void)session;
  (void)msg;
  (void)len;
  (void)userp;
  return 0;
}
#endif

<<<<<<< HEAD
static void populate_settings(struct Curl_easy *data,
                              struct http_conn *httpc)
{
  nghttp2_settings_entry *iv = httpc->local_settings;

  iv[0].settings_id = NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS;
  iv[0].value = Curl_multi_max_concurrent_streams(data->multi);

  iv[1].settings_id = NGHTTP2_SETTINGS_INITIAL_WINDOW_SIZE;
  iv[1].value = HTTP2_HUGE_WINDOW_SIZE;

  iv[2].settings_id = NGHTTP2_SETTINGS_ENABLE_PUSH;
  iv[2].value = data->multi->push_cb != NULL;

  httpc->local_settings_num = 3;
}

void Curl_http2_done(struct Curl_easy *data, bool premature)
{
  struct HTTP *http = data->req.p.http;
  struct http_conn *httpc = &data->conn->proto.httpc;

  /* there might be allocated resources done before this got the 'h2' pointer
     setup */
  Curl_dyn_free(&http->header_recvbuf);
  Curl_dyn_free(&http->trailer_recvbuf);
  if(http->push_headers) {
    /* if they weren't used and then freed before */
    for(; http->push_headers_used > 0; --http->push_headers_used) {
      free(http->push_headers[http->push_headers_used - 1]);
    }
    free(http->push_headers);
    http->push_headers = NULL;
  }

  if(!(data->conn->handler->protocol&PROTO_FAMILY_HTTP) ||
     !httpc->h2) /* not HTTP/2 ? */
    return;

  /* do this before the reset handling, as that might clear ->stream_id */
  if(http->stream_id == httpc->pause_stream_id) {
    H2BUGF(infof(data, "DONE the pause stream (%u)", http->stream_id));
    httpc->pause_stream_id = 0;
  }
  if(premature || (!http->closed && http->stream_id)) {
    /* RST_STREAM */
    set_transfer(httpc, data); /* set the transfer */
    H2BUGF(infof(data, "RST stream %u", http->stream_id));
    if(!nghttp2_submit_rst_stream(httpc->h2, NGHTTP2_FLAG_NONE,
                                  http->stream_id, NGHTTP2_STREAM_CLOSED))
      (void)nghttp2_session_send(httpc->h2);
  }

  if(data->state.drain)
    drained_transfer(data, httpc);

  /* -1 means unassigned and 0 means cleared */
  if(http->stream_id > 0) {
    int rv = nghttp2_session_set_stream_user_data(httpc->h2,
                                                  http->stream_id, 0);
    if(rv) {
      infof(data, "http/2: failed to clear user_data for stream %u",
            http->stream_id);
      DEBUGASSERT(0);
    }
    set_transfer(httpc, NULL);
    http->stream_id = 0;
  }
}

=======
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
/*
 * Initialize nghttp2 for a Curl connection
 */
static CURLcode http2_init(struct Curl_easy *data, struct connectdata *conn)
{
  if(!conn->proto.httpc.h2) {
    int rc;
    nghttp2_session_callbacks *callbacks;

    conn->proto.httpc.inbuf = malloc(H2_BUFSIZE);
    if(!conn->proto.httpc.inbuf)
      return CURLE_OUT_OF_MEMORY;

    rc = nghttp2_session_callbacks_new(&callbacks);

    if(rc) {
      failf(data, "Couldn't initialize nghttp2 callbacks");
      return CURLE_OUT_OF_MEMORY; /* most likely at least */
    }

    /* nghttp2_send_callback */
    nghttp2_session_callbacks_set_send_callback(callbacks, send_callback);
    /* nghttp2_on_frame_recv_callback */
    nghttp2_session_callbacks_set_on_frame_recv_callback
      (callbacks, on_frame_recv);
    /* nghttp2_on_data_chunk_recv_callback */
    nghttp2_session_callbacks_set_on_data_chunk_recv_callback
      (callbacks, on_data_chunk_recv);
    /* nghttp2_on_stream_close_callback */
    nghttp2_session_callbacks_set_on_stream_close_callback
      (callbacks, on_stream_close);
    /* nghttp2_on_begin_headers_callback */
    nghttp2_session_callbacks_set_on_begin_headers_callback
      (callbacks, on_begin_headers);
    /* nghttp2_on_header_callback */
    nghttp2_session_callbacks_set_on_header_callback(callbacks, on_header);

    nghttp2_session_callbacks_set_error_callback(callbacks, error_callback);

    /* The nghttp2 session is not yet setup, do it */
    rc = nghttp2_session_client_new(&conn->proto.httpc.h2, callbacks, conn);

    nghttp2_session_callbacks_del(callbacks);

    if(rc) {
      failf(data, "Couldn't initialize nghttp2");
      return CURLE_OUT_OF_MEMORY; /* most likely at least */
    }
  }
  return CURLE_OK;
}

/*
 * Append headers to ask for a HTTP1.1 to HTTP2 upgrade.
 */
CURLcode Curl_http2_request_upgrade(struct dynbuf *req,
                                    struct Curl_easy *data)
{
  CURLcode result;
  ssize_t binlen;
  char *base64;
  size_t blen;
  struct connectdata *conn = data->conn;
  struct SingleRequest *k = &data->req;
  uint8_t *binsettings = conn->proto.httpc.binsettings;
  struct http_conn *httpc = &conn->proto.httpc;

  populate_settings(data, httpc);

  /* this returns number of bytes it wrote */
  binlen = nghttp2_pack_settings_payload(binsettings, H2_BINSETTINGS_LEN,
                                         httpc->local_settings,
                                         httpc->local_settings_num);
  if(binlen <= 0) {
    failf(data, "nghttp2 unexpectedly failed on pack_settings_payload");
    Curl_dyn_free(req);
    return CURLE_FAILED_INIT;
  }
  conn->proto.httpc.binlen = binlen;

  result = Curl_base64url_encode((const char *)binsettings, binlen,
                                 &base64, &blen);
  if(result) {
    Curl_dyn_free(req);
    return result;
  }

  result = Curl_dyn_addf(req,
                         "Connection: Upgrade, HTTP2-Settings\r\n"
                         "Upgrade: %s\r\n"
                         "HTTP2-Settings: %s\r\n",
                         NGHTTP2_CLEARTEXT_PROTO_VERSION_ID, base64);
  free(base64);

  k->upgr101 = UPGR101_REQUESTED;

  return result;
}

<<<<<<< HEAD
/*
 * Returns nonzero if current HTTP/2 session should be closed.
 */
static int should_close_session(struct http_conn *httpc)
{
  return httpc->drain_total == 0 && !nghttp2_session_want_read(httpc->h2) &&
    !nghttp2_session_want_write(httpc->h2);
}

/*
 * h2_process_pending_input() processes pending input left in
 * httpc->inbuf.  Then, call h2_session_send() to send pending data.
 * This function returns 0 if it succeeds, or -1 and error code will
 * be assigned to *err.
 */
static int h2_process_pending_input(struct Curl_easy *data,
                                    struct http_conn *httpc,
                                    CURLcode *err)
{
  ssize_t nread;
  char *inbuf;
  ssize_t rv;

  nread = httpc->inbuflen - httpc->nread_inbuf;
  inbuf = httpc->inbuf + httpc->nread_inbuf;

  set_transfer(httpc, data); /* set the transfer */
  rv = nghttp2_session_mem_recv(httpc->h2, (const uint8_t *)inbuf, nread);
  if(rv < 0) {
    failf(data,
          "h2_process_pending_input: nghttp2_session_mem_recv() returned "
          "%zd:%s", rv, nghttp2_strerror((int)rv));
    *err = CURLE_RECV_ERROR;
    return -1;
  }

  if(nread == rv) {
    H2BUGF(infof(data,
                 "h2_process_pending_input: All data in connection buffer "
                 "processed"));
    httpc->inbuflen = 0;
    httpc->nread_inbuf = 0;
  }
  else {
    httpc->nread_inbuf += rv;
    H2BUGF(infof(data,
                 "h2_process_pending_input: %zu bytes left in connection "
                 "buffer",
                 httpc->inbuflen - httpc->nread_inbuf));
  }

  rv = h2_session_send(data, httpc->h2);
  if(rv) {
    *err = CURLE_SEND_ERROR;
    return -1;
  }

  if(nghttp2_session_check_request_allowed(httpc->h2) == 0) {
    /* No more requests are allowed in the current session, so
       the connection may not be reused. This is set when a
       GOAWAY frame has been received or when the limit of stream
       identifiers has been reached. */
    connclose(data->conn, "http/2: No new requests allowed");
  }

  if(should_close_session(httpc)) {
    struct HTTP *stream = data->req.p.http;
    H2BUGF(infof(data,
                 "h2_process_pending_input: nothing to do in this session"));
    if(stream->error)
      *err = CURLE_HTTP2;
    else {
      /* not an error per se, but should still close the connection */
      connclose(data->conn, "GOAWAY received");
      *err = CURLE_OK;
    }
    return -1;
  }
  return 0;
}

/*
 * Called from transfer.c:done_sending when we stop uploading.
 */
CURLcode Curl_http2_done_sending(struct Curl_easy *data,
                                 struct connectdata *conn)
=======
static CURLcode http2_data_done_send(struct Curl_cfilter *cf,
                                     struct Curl_easy *data)
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
{
  CURLcode result = CURLE_OK;
<<<<<<< HEAD

  if((conn->handler == &Curl_handler_http2_ssl) ||
     (conn->handler == &Curl_handler_http2)) {
    /* make sure this is only attempted for HTTP/2 transfers */
    struct HTTP *stream = data->req.p.http;
    struct http_conn *httpc = &conn->proto.httpc;
    nghttp2_session *h2 = httpc->h2;

    if(stream->upload_left) {
      /* If the stream still thinks there's data left to upload. */

      stream->upload_left = 0; /* DONE! */

      /* resume sending here to trigger the callback to get called again so
         that it can signal EOF to nghttp2 */
      (void)nghttp2_session_resume_data(h2, stream->stream_id);
      (void)h2_process_pending_input(data, httpc, &result);
    }

    /* If nghttp2 still has pending frames unsent */
    if(nghttp2_session_want_write(h2)) {
      struct SingleRequest *k = &data->req;
      int rv;

      H2BUGF(infof(data, "HTTP/2 still wants to send data (easy %p)", data));

      /* and attempt to send the pending frames */
      rv = h2_session_send(data, h2);
      if(rv)
        result = CURLE_SEND_ERROR;

      if(nghttp2_session_want_write(h2)) {
         /* re-set KEEP_SEND to make sure we are called again */
         k->keepon |= KEEP_SEND;
      }
    }
=======
  struct stream_ctx *stream = H2_STREAM_CTX(data);

  if(!ctx || !ctx->h2 || !stream)
    goto out;

  DEBUGF(LOG_CF(data, cf, "[h2sid=%d] data done send", stream->id));
  if(stream->upload_left) {
    /* If the stream still thinks there's data left to upload. */
    if(stream->upload_left == -1)
      stream->upload_left = 0; /* DONE! */

    /* resume sending here to trigger the callback to get called again so
       that it can signal EOF to nghttp2 */
    (void)nghttp2_session_resume_data(ctx->h2, stream->id);
    drain_stream(cf, data, stream);
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
  }
  return result;
}

static ssize_t http2_handle_stream_close(struct connectdata *conn,
                                         struct Curl_easy *data,
                                         struct stream_ctx *stream,
                                         CURLcode *err)
{
<<<<<<< HEAD
  struct http_conn *httpc = &conn->proto.httpc;

  if(httpc->pause_stream_id == stream->stream_id) {
    httpc->pause_stream_id = 0;
  }

  drained_transfer(data, httpc);

  if(httpc->pause_stream_id == 0) {
    if(h2_process_pending_input(data, httpc, err) != 0) {
      return -1;
    }
  }
=======
  ssize_t rv = 0;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))

  DEBUGASSERT(data->state.drain == 0);

  /* Reset to FALSE to prevent infinite loop in readwrite_data function. */
  stream->closed = FALSE;
  if(stream->error == NGHTTP2_REFUSED_STREAM) {
<<<<<<< HEAD
    H2BUGF(infof(data, "REFUSED_STREAM (%u), try again on a new connection",
                 stream->stream_id));
    connclose(conn, "REFUSED_STREAM"); /* don't use this anymore */
=======
    DEBUGF(LOG_CF(data, cf, "[h2sid=%d] REFUSED_STREAM, try again on a new "
                  "connection", stream->id));
    connclose(cf->conn, "REFUSED_STREAM"); /* don't use this anymore */
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
    data->state.refused_stream = TRUE;
    *err = CURLE_SEND_ERROR; /* trigger Curl_retry_request() later */
    return -1;
  }
  else if(stream->reset) {
    failf(data, "HTTP/2 stream %u was reset", stream->id);
    *err = stream->bodystarted? CURLE_PARTIAL_FILE : CURLE_RECV_ERROR;
    return -1;
  }
  else if(stream->error != NGHTTP2_NO_ERROR) {
    failf(data, "HTTP/2 stream %u was not closed cleanly: %s (err %u)",
          stream->id, nghttp2_http2_strerror(stream->error),
          stream->error);
    *err = CURLE_HTTP2_STREAM;
    return -1;
  }

  if(!stream->bodystarted) {
    failf(data, "HTTP/2 stream %u was closed cleanly, but before getting "
          " all response header fields, treated as error",
          stream->id);
    *err = CURLE_HTTP2_STREAM;
    return -1;
  }

  if(Curl_dynhds_count(&stream->resp_trailers)) {
    struct dynhds_entry *e;
    struct dynbuf dbuf;
    size_t i;

    *err = CURLE_OK;
    Curl_dyn_init(&dbuf, DYN_TRAILERS);
    for(i = 0; i < Curl_dynhds_count(&stream->resp_trailers); ++i) {
      e = Curl_dynhds_getn(&stream->resp_trailers, i);
      if(!e)
        break;
      Curl_dyn_reset(&dbuf);
      *err = Curl_dyn_addf(&dbuf, "%.*s: %.*s\x0d\x0a",
                          (int)e->namelen, e->name,
                          (int)e->valuelen, e->value);
      if(*err)
        break;
      Curl_debug(data, CURLINFO_HEADER_IN, Curl_dyn_ptr(&dbuf),
                 Curl_dyn_len(&dbuf));
      *err = Curl_client_write(data, CLIENTWRITE_HEADER|CLIENTWRITE_TRAILER,
                               Curl_dyn_ptr(&dbuf), Curl_dyn_len(&dbuf));
      if(*err)
        break;
    }
    Curl_dyn_free(&dbuf);
    if(*err)
      goto out;
  }

  stream->close_handled = TRUE;
  *err = CURLE_OK;
  rv = 0;

<<<<<<< HEAD
  H2BUGF(infof(data, "http2_recv returns 0, http2_handle_stream_close"));
  return 0;
=======
out:
  DEBUGF(LOG_CF(data, cf, "handle_stream_close -> %zd, %d", rv, *err));
  return rv;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
}

/*
 * h2_pri_spec() fills in the pri_spec struct, used by nghttp2 to send weight
 * and dependency to the peer. It also stores the updated values in the state
 * struct.
 */

static void h2_pri_spec(struct Curl_easy *data,
                        nghttp2_priority_spec *pri_spec)
{
<<<<<<< HEAD
  struct HTTP *depstream = (data->set.stream_depends_on?
                            data->set.stream_depends_on->req.p.http:NULL);
  int32_t depstream_id = depstream? depstream->stream_id:0;
  nghttp2_priority_spec_init(pri_spec, depstream_id, data->set.stream_weight,
                             data->set.stream_depends_e);
  data->state.stream_weight = data->set.stream_weight;
  data->state.stream_depends_e = data->set.stream_depends_e;
  data->state.stream_depends_on = data->set.stream_depends_on;
=======
  struct Curl_data_priority *prio = &data->set.priority;
  struct stream_ctx *depstream = H2_STREAM_CTX(prio->parent);
  int32_t depstream_id = depstream? depstream->id:0;
  nghttp2_priority_spec_init(pri_spec, depstream_id,
                             sweight_wanted(data),
                             data->set.priority.exclusive);
  data->state.priority = *prio;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
}

/*
 * Check if there's been an update in the priority /
 * dependency settings and if so it submits a PRIORITY frame with the updated
 * info.
 * Flush any out data pending in the network buffer.
 */
<<<<<<< HEAD
static int h2_session_send(struct Curl_easy *data,
                           nghttp2_session *h2)
{
  struct HTTP *stream = data->req.p.http;
  struct http_conn *httpc = &data->conn->proto.httpc;
  set_transfer(httpc, data);
  if((data->set.stream_weight != data->state.stream_weight) ||
     (data->set.stream_depends_e != data->state.stream_depends_e) ||
     (data->set.stream_depends_on != data->state.stream_depends_on) ) {
=======
static CURLcode h2_progress_egress(struct Curl_cfilter *cf,
                                  struct Curl_easy *data)
{
  struct cf_h2_ctx *ctx = cf->ctx;
  struct stream_ctx *stream = H2_STREAM_CTX(data);
  int rv = 0;

  if((sweight_wanted(data) != sweight_in_effect(data)) ||
     (data->set.priority.exclusive != data->state.priority.exclusive) ||
     (data->set.priority.parent != data->state.priority.parent) ) {
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
    /* send new weight and/or dependency */
    nghttp2_priority_spec pri_spec;
    int rv;

    h2_pri_spec(data, &pri_spec);
<<<<<<< HEAD

    H2BUGF(infof(data, "Queuing PRIORITY on stream %u (easy %p)",
                 stream->stream_id, data));
    DEBUGASSERT(stream->stream_id != -1);
    rv = nghttp2_submit_priority(h2, NGHTTP2_FLAG_NONE, stream->stream_id,
                                 &pri_spec);
=======
    DEBUGF(LOG_CF(data, cf, "[h2sid=%d] Queuing PRIORITY",
                  stream->id));
    DEBUGASSERT(stream->id != -1);
    rv = nghttp2_submit_priority(ctx->h2, NGHTTP2_FLAG_NONE,
                                 stream->id, &pri_spec);
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
    if(rv)
      return rv;
  }

<<<<<<< HEAD
  return nghttp2_session_send(h2);
=======
  rv = nghttp2_session_send(ctx->h2);
out:
  if(nghttp2_is_fatal(rv)) {
    DEBUGF(LOG_CF(data, cf, "nghttp2_session_send error (%s)%d",
                  nghttp2_strerror(rv), rv));
    return CURLE_SEND_ERROR;
  }
  return nw_out_flush(cf, data);
}

static ssize_t stream_recv(struct Curl_cfilter *cf, struct Curl_easy *data,
                           char *buf, size_t len, CURLcode *err)
{
  struct cf_h2_ctx *ctx = cf->ctx;
  struct stream_ctx *stream = H2_STREAM_CTX(data);
  ssize_t nread = -1;

  *err = CURLE_AGAIN;
  if(!Curl_bufq_is_empty(&stream->recvbuf)) {
    nread = Curl_bufq_read(&stream->recvbuf,
                           (unsigned char *)buf, len, err);
    DEBUGF(LOG_CF(data, cf, "recvbuf read(len=%zu) -> %zd, %d",
                  len, nread, *err));
    if(nread < 0)
      goto out;
    DEBUGASSERT(nread > 0);
  }

  if(nread < 0) {
    if(stream->closed) {
      DEBUGF(LOG_CF(data, cf, "[h2sid=%d] returning CLOSE", stream->id));
      nread = http2_handle_stream_close(cf, data, stream, err);
    }
    else if(stream->reset ||
            (ctx->conn_closed && Curl_bufq_is_empty(&ctx->inbufq)) ||
            (ctx->goaway && ctx->last_stream_id < stream->id)) {
      DEBUGF(LOG_CF(data, cf, "[h2sid=%d] returning ERR", stream->id));
      *err = stream->bodystarted? CURLE_PARTIAL_FILE : CURLE_RECV_ERROR;
      nread = -1;
    }
  }
  else if(nread == 0) {
    *err = CURLE_AGAIN;
    nread = -1;
  }

out:
  DEBUGF(LOG_CF(data, cf, "stream_recv(len=%zu) -> %zd, %d",
                len, nread, *err));
  return nread;
}

static CURLcode h2_progress_ingress(struct Curl_cfilter *cf,
                                    struct Curl_easy *data)
{
  struct cf_h2_ctx *ctx = cf->ctx;
  struct stream_ctx *stream;
  CURLcode result = CURLE_OK;
  ssize_t nread;

  /* Process network input buffer fist */
  if(!Curl_bufq_is_empty(&ctx->inbufq)) {
    DEBUGF(LOG_CF(data, cf, "Process %zd bytes in connection buffer",
                  Curl_bufq_len(&ctx->inbufq)));
    if(h2_process_pending_input(cf, data, &result) < 0)
      return result;
  }

  /* Receive data from the "lower" filters, e.g. network until
   * it is time to stop due to connection close or us not processing
   * all network input */
  while(!ctx->conn_closed && Curl_bufq_is_empty(&ctx->inbufq)) {
    stream = H2_STREAM_CTX(data);
    if(stream && (stream->closed || Curl_bufq_is_full(&stream->recvbuf))) {
      /* We would like to abort here and stop processing, so that
       * the transfer loop can handle the data/close here. However,
       * this may leave data in underlying buffers that will not
       * be consumed. */
      if(!cf->next || !cf->next->cft->has_data_pending(cf->next, data))
        break;
    }

    nread = Curl_bufq_slurp(&ctx->inbufq, nw_in_reader, cf, &result);
    /* DEBUGF(LOG_CF(data, cf, "read %zd bytes nw data -> %zd, %d",
                  Curl_bufq_len(&ctx->inbufq), nread, result)); */
    if(nread < 0) {
      if(result != CURLE_AGAIN) {
        failf(data, "Failed receiving HTTP2 data: %d(%s)", result,
              curl_easy_strerror(result));
        return result;
      }
      break;
    }
    else if(nread == 0) {
      ctx->conn_closed = TRUE;
      break;
    }

    if(h2_process_pending_input(cf, data, &result))
      return result;
  }

  if(ctx->conn_closed && Curl_bufq_is_empty(&ctx->inbufq)) {
    connclose(cf->conn, "GOAWAY received");
  }

  return CURLE_OK;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
}

static ssize_t http2_recv(struct Curl_easy *data, int sockindex,
                          char *mem, size_t len, CURLcode *err)
{
<<<<<<< HEAD
  ssize_t nread;
  struct connectdata *conn = data->conn;
  struct http_conn *httpc = &conn->proto.httpc;
  struct HTTP *stream = data->req.p.http;
=======
  struct cf_h2_ctx *ctx = cf->ctx;
  struct stream_ctx *stream = H2_STREAM_CTX(data);
  ssize_t nread = -1;
  CURLcode result;
  struct cf_call_data save;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))

  (void)sockindex; /* we always do HTTP2 on sockindex 0 */

<<<<<<< HEAD
  if(should_close_session(httpc)) {
    H2BUGF(infof(data,
                 "http2_recv: nothing to do in this session"));
    if(conn->bits.close) {
      /* already marked for closure, return OK and we're done */
      *err = CURLE_OK;
      return 0;
    }
    *err = CURLE_HTTP2;
    return -1;
  }

  /* Nullify here because we call nghttp2_session_send() and they
     might refer to the old buffer. */
  stream->upload_mem = NULL;
  stream->upload_len = 0;

  /*
   * At this point 'stream' is just in the Curl_easy the connection
   * identifies as its owner at this time.
   */

  if(stream->bodystarted &&
     stream->nread_header_recvbuf < Curl_dyn_len(&stream->header_recvbuf)) {
    /* If there is header data pending for this stream to return, do that */
    size_t left =
      Curl_dyn_len(&stream->header_recvbuf) - stream->nread_header_recvbuf;
    size_t ncopy = CURLMIN(len, left);
    memcpy(mem, Curl_dyn_ptr(&stream->header_recvbuf) +
           stream->nread_header_recvbuf, ncopy);
    stream->nread_header_recvbuf += ncopy;

    H2BUGF(infof(data, "http2_recv: Got %d bytes from header_recvbuf",
                 (int)ncopy));
    return ncopy;
  }

  H2BUGF(infof(data, "http2_recv: easy %p (stream %u) win %u/%u",
               data, stream->stream_id,
               nghttp2_session_get_local_window_size(httpc->h2),
               nghttp2_session_get_stream_local_window_size(httpc->h2,
                                                            stream->stream_id)
           ));

  if((data->state.drain) && stream->memlen) {
    H2BUGF(infof(data, "http2_recv: DRAIN %zu bytes stream %u (%p => %p)",
                 stream->memlen, stream->stream_id,
                 stream->mem, mem));
    if(mem != stream->mem) {
      /* if we didn't get the same buffer this time, we must move the data to
         the beginning */
      memmove(mem, stream->mem, stream->memlen);
      stream->len = len - stream->memlen;
      stream->mem = mem;
    }
    if(httpc->pause_stream_id == stream->stream_id && !stream->pausedata) {
      /* We have paused nghttp2, but we have no pause data (see
         on_data_chunk_recv). */
      httpc->pause_stream_id = 0;
      if(h2_process_pending_input(data, httpc, err) != 0) {
        return -1;
      }
    }
  }
  else if(stream->pausedata) {
    DEBUGASSERT(httpc->pause_stream_id == stream->stream_id);
    nread = CURLMIN(len, stream->pauselen);
    memcpy(mem, stream->pausedata, nread);

    stream->pausedata += nread;
    stream->pauselen -= nread;

    if(stream->pauselen == 0) {
      H2BUGF(infof(data, "Unpaused by stream %u", stream->stream_id));
      DEBUGASSERT(httpc->pause_stream_id == stream->stream_id);
      httpc->pause_stream_id = 0;

      stream->pausedata = NULL;
      stream->pauselen = 0;

      /* When NGHTTP2_ERR_PAUSE is returned from
         data_source_read_callback, we might not process DATA frame
         fully.  Calling nghttp2_session_mem_recv() again will
         continue to process DATA frame, but if there is no incoming
         frames, then we have to call it again with 0-length data.
         Without this, on_stream_close callback will not be called,
         and stream could be hanged. */
      if(h2_process_pending_input(data, httpc, err) != 0) {
        return -1;
      }
    }
    H2BUGF(infof(data, "http2_recv: returns unpaused %zd bytes on stream %u",
                 nread, stream->stream_id));
    return nread;
  }
  else if(httpc->pause_stream_id) {
    /* If a stream paused nghttp2_session_mem_recv previously, and has
       not processed all data, it still refers to the buffer in
       nghttp2_session.  If we call nghttp2_session_mem_recv(), we may
       overwrite that buffer.  To avoid that situation, just return
       here with CURLE_AGAIN.  This could be busy loop since data in
       socket is not read.  But it seems that usually streams are
       notified with its drain property, and socket is read again
       quickly. */
    if(stream->closed)
      /* closed overrides paused */
      return 0;
    H2BUGF(infof(data, "stream %u is paused, pause id: %u",
                 stream->stream_id, httpc->pause_stream_id));
    *err = CURLE_AGAIN;
    return -1;
  }
  else {
    /* remember where to store incoming data for this stream and how big the
       buffer is */
    stream->mem = mem;
    stream->len = len;
    stream->memlen = 0;

    if(httpc->inbuflen == 0) {
      nread = ((Curl_recv *)httpc->recv_underlying)(
        data, FIRSTSOCKET, httpc->inbuf, H2_BUFSIZE, err);

      if(nread == -1) {
        if(*err != CURLE_AGAIN)
          failf(data, "Failed receiving HTTP2 data");
        else if(stream->closed)
          /* received when the stream was already closed! */
          return http2_handle_stream_close(conn, data, stream, err);

        return -1;
      }

      if(nread == 0) {
        if(!stream->closed) {
          /* This will happen when the server or proxy server is SIGKILLed
             during data transfer. We should emit an error since our data
             received may be incomplete. */
          failf(data, "HTTP/2 stream %u was not closed cleanly before"
                " end of the underlying stream",
                stream->stream_id);
          *err = CURLE_HTTP2_STREAM;
          return -1;
        }

        H2BUGF(infof(data, "end of stream"));
        *err = CURLE_OK;
        return 0;
      }

      H2BUGF(infof(data, "nread=%zd", nread));

      httpc->inbuflen = nread;

      DEBUGASSERT(httpc->nread_inbuf == 0);
    }
    else {
      nread = httpc->inbuflen - httpc->nread_inbuf;
      (void)nread;  /* silence warning, used in debug */
      H2BUGF(infof(data, "Use data left in connection buffer, nread=%zd",
                   nread));
    }

    if(h2_process_pending_input(data, httpc, err))
      return -1;
=======
  nread = stream_recv(cf, data, buf, len, err);
  if(nread < 0 && *err != CURLE_AGAIN)
    goto out;

  if(nread < 0) {
    *err = h2_progress_ingress(cf, data);
    if(*err)
      goto out;

    nread = stream_recv(cf, data, buf, len, err);
  }

  if(nread > 0) {
    size_t data_consumed = (size_t)nread;
    /* Now that we transferred this to the upper layer, we report
     * the actual amount of DATA consumed to the H2 session, so
     * that it adjusts stream flow control */
    if(stream->resp_hds_len >= data_consumed) {
      stream->resp_hds_len -= data_consumed;  /* no DATA */
    }
    else {
      if(stream->resp_hds_len) {
        data_consumed -= stream->resp_hds_len;
        stream->resp_hds_len = 0;
      }
      if(data_consumed) {
        nghttp2_session_consume(ctx->h2, stream->id, data_consumed);
      }
    }

    if(stream->closed) {
      DEBUGF(LOG_CF(data, cf, "[h2sid=%d] closed stream, set drain",
                    stream->id));
      drain_stream(cf, data, stream);
    }
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
  }
  if(stream->memlen) {
    ssize_t retlen = stream->memlen;
    H2BUGF(infof(data, "http2_recv: returns %zd for stream %u",
                 retlen, stream->stream_id));
    stream->memlen = 0;

<<<<<<< HEAD
    if(httpc->pause_stream_id == stream->stream_id) {
      /* data for this stream is returned now, but this stream caused a pause
         already so we need it called again asap */
      H2BUGF(infof(data, "Data returned for PAUSED stream %u",
                   stream->stream_id));
    }
    else if(!stream->closed) {
      drained_transfer(data, httpc);
    }
    else
      /* this stream is closed, trigger a another read ASAP to detect that */
      Curl_expire(data, 0, EXPIRE_RUN_NOW);

    return retlen;
  }
  if(stream->closed)
    return http2_handle_stream_close(conn, data, stream, err);
  *err = CURLE_AGAIN;
  H2BUGF(infof(data, "http2_recv returns AGAIN for stream %u",
               stream->stream_id));
  return -1;
}

static ssize_t http2_send(struct Curl_easy *data, int sockindex,
                          const void *mem, size_t len, CURLcode *err)
{
  /*
   * Currently, we send request in this function, but this function is also
   * used to send request body. It would be nice to add dedicated function for
   * request.
   */
  int rv;
  struct connectdata *conn = data->conn;
  struct http_conn *httpc = &conn->proto.httpc;
  struct HTTP *stream = data->req.p.http;
=======
out:
  result = h2_progress_egress(cf, data);
  if(result) {
    *err = result;
    nread = -1;
  }
  DEBUGF(LOG_CF(data, cf, "[h2sid=%d] cf_recv(len=%zu) -> %zd %d, "
                "buffered=%zu, window=%d/%d, connection %d/%d",
                stream->id, len, nread, *err,
                Curl_bufq_len(&stream->recvbuf),
                nghttp2_session_get_stream_effective_recv_data_length(
                  ctx->h2, stream->id),
                nghttp2_session_get_stream_effective_local_window_size(
                  ctx->h2, stream->id),
                nghttp2_session_get_local_window_size(ctx->h2),
                HTTP2_HUGE_WINDOW_SIZE));

  CF_DATA_RESTORE(cf, save);
  return nread;
}

static ssize_t h2_submit(struct stream_ctx **pstream,
                         struct Curl_cfilter *cf, struct Curl_easy *data,
                         const void *buf, size_t len, CURLcode *err)
{
  struct cf_h2_ctx *ctx = cf->ctx;
  struct stream_ctx *stream = NULL;
  struct h1_req_parser h1;
  struct dynhds h2_headers;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
  nghttp2_nv *nva = NULL;
  size_t nheader, i;
  nghttp2_data_provider data_prd;
  int32_t stream_id;
  nghttp2_session *h2 = httpc->h2;
  nghttp2_priority_spec pri_spec;
<<<<<<< HEAD
  CURLcode result;
  struct h2h3req *hreq;

  (void)sockindex;

  H2BUGF(infof(data, "http2_send len=%zu", len));

  if(stream->stream_id != -1) {
    if(stream->close_handled) {
      infof(data, "stream %u closed", stream->stream_id);
      *err = CURLE_HTTP2_STREAM;
      return -1;
    }
    else if(stream->closed) {
      return http2_handle_stream_close(conn, data, stream, err);
    }
    /* If stream_id != -1, we have dispatched request HEADERS, and now
       are going to send or sending request body in DATA frame */
    stream->upload_mem = mem;
    stream->upload_len = len;
    rv = nghttp2_session_resume_data(h2, stream->stream_id);
    if(nghttp2_is_fatal(rv)) {
      *err = CURLE_SEND_ERROR;
      return -1;
    }
    rv = h2_session_send(data, h2);
    if(nghttp2_is_fatal(rv)) {
      *err = CURLE_SEND_ERROR;
      return -1;
    }
    len -= stream->upload_len;

    /* Nullify here because we call nghttp2_session_send() and they
       might refer to the old buffer. */
    stream->upload_mem = NULL;
    stream->upload_len = 0;

    if(should_close_session(httpc)) {
      H2BUGF(infof(data, "http2_send: nothing to do in this session"));
      *err = CURLE_HTTP2;
      return -1;
    }

    if(stream->upload_left) {
      /* we are sure that we have more data to send here.  Calling the
         following API will make nghttp2_session_want_write() return
         nonzero if remote window allows it, which then libcurl checks
         socket is writable or not.  See http2_perform_getsock(). */
      nghttp2_session_resume_data(h2, stream->stream_id);
    }

#ifdef DEBUG_HTTP2
    if(!len) {
      infof(data, "http2_send: easy %p (stream %u) win %u/%u",
            data, stream->stream_id,
            nghttp2_session_get_remote_window_size(httpc->h2),
            nghttp2_session_get_stream_remote_window_size(httpc->h2,
                                                          stream->stream_id)
        );

    }
    infof(data, "http2_send returns %zu for stream %u", len,
          stream->stream_id);
#endif
    return len;
  }

  result = Curl_pseudo_headers(data, mem, len, &hreq);
  if(result) {
    *err = result;
    return -1;
=======
  ssize_t nwritten;

  Curl_h1_req_parse_init(&h1, (4*1024));
  Curl_dynhds_init(&h2_headers, 0, DYN_HTTP_REQUEST);

  *err = http2_data_setup(cf, data, &stream);
  if(*err) {
    nwritten = -1;
    goto out;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
  }

  nwritten = Curl_h1_req_parse_read(&h1, buf, len, NULL, 0, err);
  if(nwritten < 0)
    goto out;
  DEBUGASSERT(h1.done);
  DEBUGASSERT(h1.req);

  *err = Curl_http_req_to_h2(&h2_headers, h1.req, data);
  if(*err) {
    nwritten = -1;
    goto out;
  }

  nheader = Curl_dynhds_count(&h2_headers);
  nva = malloc(sizeof(nghttp2_nv) * nheader);
  if(!nva) {
    *err = CURLE_OUT_OF_MEMORY;
    return -1;
  }

  for(i = 0; i < nheader; ++i) {
    struct dynhds_entry *e = Curl_dynhds_getn(&h2_headers, i);
    nva[i].name = (unsigned char *)e->name;
    nva[i].namelen = e->namelen;
    nva[i].value = (unsigned char *)e->value;
    nva[i].valuelen = e->valuelen;
    nva[i].flags = NGHTTP2_NV_FLAG_NONE;
  }

#define MAX_ACC 60000  /* <64KB to account for some overhead */
  {
    size_t acc = 0;

    for(i = 0; i < nheader; ++i) {
      acc += nva[i].namelen + nva[i].valuelen;

      infof(data, "h2 [%.*s: %.*s]",
            (int)nva[i].namelen, nva[i].name,
            (int)nva[i].valuelen, nva[i].value);
    }

    if(acc > MAX_ACC) {
      infof(data, "http_request: Warning: The cumulative length of all "
            "headers exceeds %d bytes and that could cause the "
            "stream to be rejected.", MAX_ACC);
    }
  }

  h2_pri_spec(data, &pri_spec);

  H2BUGF(infof(data, "http2_send request allowed %d (easy handle %p)",
               nghttp2_session_check_request_allowed(h2), (void *)data));

  switch(data->state.httpreq) {
  case HTTPREQ_POST:
  case HTTPREQ_POST_FORM:
  case HTTPREQ_POST_MIME:
  case HTTPREQ_PUT:
    if(data->state.infilesize != -1)
      stream->upload_left = data->state.infilesize;
    else
      /* data sending without specifying the data amount up front */
      stream->upload_left = -1; /* unknown */

    data_prd.read_callback = req_body_read_callback;
    data_prd.source.ptr = NULL;
    stream_id = nghttp2_submit_request(h2, &pri_spec, nva, nheader,
                                       &data_prd, data);
    break;
  default:
<<<<<<< HEAD
    stream_id = nghttp2_submit_request(h2, &pri_spec, nva, nheader,
=======
    stream->upload_left = 0; /* no request body */
    stream_id = nghttp2_submit_request(ctx->h2, &pri_spec, nva, nheader,
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
                                       NULL, data);
  }

  Curl_safefree(nva);

  if(stream_id < 0) {
    H2BUGF(infof(data,
                 "http2_send() nghttp2_submit_request error (%s)%u",
                 nghttp2_strerror(stream_id), stream_id));
    *err = CURLE_SEND_ERROR;
    return -1;
  }

  DEBUGF(LOG_CF(data, cf, "[h2sid=%d] cf_send(len=%zu) submit %s",
                stream_id, len, data->state.url));
  infof(data, "Using Stream ID: %u (easy handle %p)",
        stream_id, (void *)data);
<<<<<<< HEAD
  stream->stream_id = stream_id;

  rv = h2_session_send(data, h2);
  if(rv) {
    H2BUGF(infof(data,
                 "http2_send() nghttp2_session_send error (%s)%d",
                 nghttp2_strerror(rv), rv));

    *err = CURLE_SEND_ERROR;
    return -1;
  }

  if(should_close_session(httpc)) {
    H2BUGF(infof(data, "http2_send: nothing to do in this session"));
    *err = CURLE_HTTP2;
    return -1;
  }

  /* If whole HEADERS frame was sent off to the underlying socket, the nghttp2
     library calls data_source_read_callback. But only it found that no data
     available, so it deferred the DATA transmission. Which means that
     nghttp2_session_want_write() returns 0 on http2_perform_getsock(), which
     results that no writable socket check is performed. To workaround this,
     we issue nghttp2_session_resume_data() here to bring back DATA
     transmission from deferred state. */
  nghttp2_session_resume_data(h2, stream->stream_id);

  return len;
=======
  stream->id = stream_id;

out:
  DEBUGF(LOG_CF(data, cf, "[h2sid=%d] submit -> %zd, %d",
         stream? stream->id : -1, nwritten, *err));
  *pstream = stream;
  Curl_h1_req_parse_free(&h1);
  Curl_dynhds_free(&h2_headers);
  return nwritten;
}

static ssize_t cf_h2_send(struct Curl_cfilter *cf, struct Curl_easy *data,
                          const void *buf, size_t len, CURLcode *err)
{
  /*
   * Currently, we send request in this function, but this function is also
   * used to send request body. It would be nice to add dedicated function for
   * request.
   */
  struct cf_h2_ctx *ctx = cf->ctx;
  struct stream_ctx *stream = H2_STREAM_CTX(data);
  struct cf_call_data save;
  int rv;
  ssize_t nwritten;
  CURLcode result;

  CF_DATA_SAVE(save, cf, data);

  if(stream && stream->id != -1) {
    DEBUGF(LOG_CF(data, cf, "[h2sid=%d] cf_send: win %u/%u",
                stream->id,
                nghttp2_session_get_remote_window_size(ctx->h2),
                nghttp2_session_get_stream_remote_window_size(
                  ctx->h2, stream->id)
           ));

    if(stream->close_handled) {
      infof(data, "stream %u closed", stream->id);
      *err = CURLE_HTTP2_STREAM;
      nwritten = -1;
      goto out;
    }
    else if(stream->closed) {
      nwritten = http2_handle_stream_close(cf, data, stream, err);
      goto out;
    }
    /* If stream_id != -1, we have dispatched request HEADERS, and now
       are going to send or sending request body in DATA frame */
    nwritten = Curl_bufq_write(&stream->sendbuf, buf, len, err);
    if(nwritten < 0) {
      if(*err != CURLE_AGAIN)
        goto out;
      nwritten = 0;
    }

    if(!Curl_bufq_is_empty(&stream->sendbuf)) {
      rv = nghttp2_session_resume_data(ctx->h2, stream->id);
      if(nghttp2_is_fatal(rv)) {
        *err = CURLE_SEND_ERROR;
        nwritten = -1;
        goto out;
      }
    }

    result = h2_progress_ingress(cf, data);
    if(result) {
      *err = result;
      nwritten = -1;
      goto out;
    }

    result = h2_progress_egress(cf, data);
    if(result) {
      *err = result;
      nwritten = -1;
      goto out;
    }

    if(should_close_session(ctx)) {
      if(stream->closed) {
        nwritten = http2_handle_stream_close(cf, data, stream, err);
      }
      else {
        DEBUGF(LOG_CF(data, cf, "send: nothing to do in this session"));
        *err = CURLE_HTTP2;
        nwritten = -1;
      }
      goto out;
    }

    if(!nwritten) {
      size_t rwin = nghttp2_session_get_stream_remote_window_size(ctx->h2,
                                                          stream->id);
      DEBUGF(LOG_CF(data, cf, "[h2sid=%d] cf_send: win %u/%zu",
             stream->id,
             nghttp2_session_get_remote_window_size(ctx->h2), rwin));
        if(rwin == 0) {
          /* We cannot upload more as the stream's remote window size
           * is 0. We need to receive WIN_UPDATEs before we can continue.
           */
          data->req.keepon |= KEEP_SEND_HOLD;
          DEBUGF(LOG_CF(data, cf, "[h2sid=%d] holding send as remote flow "
                 "window is exhausted", stream->id));
        }
    }
    /* handled writing BODY for open stream. */
    goto out;
  }
  else {
    nwritten = h2_submit(&stream, cf, data, buf, len, err);
    if(nwritten < 0) {
      goto out;
    }

    result = h2_progress_ingress(cf, data);
    if(result) {
      *err = result;
      nwritten = -1;
      goto out;
    }

    result = h2_progress_egress(cf, data);
    if(result) {
      *err = result;
      nwritten = -1;
      goto out;
    }

    if(should_close_session(ctx)) {
      if(stream->closed) {
        nwritten = http2_handle_stream_close(cf, data, stream, err);
      }
      else {
        DEBUGF(LOG_CF(data, cf, "send: nothing to do in this session"));
        *err = CURLE_HTTP2;
        nwritten = -1;
      }
      goto out;
    }
  }

out:
  DEBUGF(LOG_CF(data, cf, "[h2sid=%d] cf_send -> %zd, %d",
         stream? stream->id : -1, nwritten, *err));
  CF_DATA_RESTORE(cf, save);
  return nwritten;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
}

CURLcode Curl_http2_setup(struct Curl_easy *data,
                          struct connectdata *conn)
{
<<<<<<< HEAD
  CURLcode result;
  struct http_conn *httpc = &conn->proto.httpc;
  struct HTTP *stream = data->req.p.http;
=======
  struct cf_h2_ctx *ctx = cf->ctx;
  struct SingleRequest *k = &data->req;
  struct stream_ctx *stream = H2_STREAM_CTX(data);
  int bitmap = GETSOCK_BLANK;
  struct cf_call_data save;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))

  DEBUGASSERT(data->state.buffer);

<<<<<<< HEAD
  stream->stream_id = -1;

  Curl_dyn_init(&stream->header_recvbuf, DYN_H2_HEADERS);
  Curl_dyn_init(&stream->trailer_recvbuf, DYN_H2_TRAILERS);
=======
  if(!(k->keepon & (KEEP_RECV_PAUSE|KEEP_RECV_HOLD)))
    /* Unless paused - in an HTTP/2 connection we can basically always get a
       frame so we should always be ready for one */
    bitmap |= GETSOCK_READSOCK(0);

  /* we're (still uploading OR the HTTP/2 layer wants to send data) AND
     there's a window to send data in */
  if((((k->keepon & KEEP_SENDBITS) == KEEP_SEND) ||
      nghttp2_session_want_write(ctx->h2)) &&
     (nghttp2_session_get_remote_window_size(ctx->h2) &&
      nghttp2_session_get_stream_remote_window_size(ctx->h2,
                                                    stream->id)))
    bitmap |= GETSOCK_WRITESOCK(0);
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))

  stream->upload_left = 0;
  stream->upload_mem = NULL;
  stream->upload_len = 0;
  stream->mem = data->state.buffer;
  stream->len = data->set.buffer_size;

  multi_connchanged(data->multi);
  /* below this point only connection related inits are done, which only needs
     to be done once per connection */

  if((conn->handler == &Curl_handler_http2_ssl) ||
     (conn->handler == &Curl_handler_http2))
    return CURLE_OK; /* already done */

  if(conn->handler->flags & PROTOPT_SSL)
    conn->handler = &Curl_handler_http2_ssl;
  else
    conn->handler = &Curl_handler_http2;

  result = http2_init(data, conn);
  if(result) {
    Curl_dyn_free(&stream->header_recvbuf);
    return result;
  }

  infof(data, "Using HTTP2, server supports multiplexing");

  conn->bits.multiplex = TRUE; /* at least potentially multiplexed */
  conn->httpversion = 20;
  conn->bundle->multiuse = BUNDLE_MULTIPLEX;

  httpc->inbuflen = 0;
  httpc->nread_inbuf = 0;

  httpc->pause_stream_id = 0;
  httpc->drain_total = 0;

  return CURLE_OK;
}

CURLcode Curl_http2_switched(struct Curl_easy *data,
                             const char *mem, size_t nread)
{
  CURLcode result;
  struct connectdata *conn = data->conn;
  struct http_conn *httpc = &conn->proto.httpc;
  int rv;
  struct HTTP *stream = data->req.p.http;

  result = Curl_http2_setup(data, conn);
  if(result)
    return result;

  httpc->recv_underlying = conn->recv[FIRSTSOCKET];
  httpc->send_underlying = conn->send[FIRSTSOCKET];
  conn->recv[FIRSTSOCKET] = http2_recv;
  conn->send[FIRSTSOCKET] = http2_send;

  if(data->req.upgr101 == UPGR101_RECEIVED) {
    /* stream 1 is opened implicitly on upgrade */
    stream->stream_id = 1;
    /* queue SETTINGS frame (again) */
    rv = nghttp2_session_upgrade2(httpc->h2, httpc->binsettings, httpc->binlen,
                                  data->state.httpreq == HTTPREQ_HEAD, NULL);
    if(rv) {
      failf(data, "nghttp2_session_upgrade2() failed: %s(%d)",
            nghttp2_strerror(rv), rv);
      return CURLE_HTTP2;
    }

    rv = nghttp2_session_set_stream_user_data(httpc->h2,
                                              stream->stream_id,
                                              data);
    if(rv) {
      infof(data, "http/2: failed to set user_data for stream %u",
            stream->stream_id);
      DEBUGASSERT(0);
    }
  }
  else {
    populate_settings(data, httpc);

    /* stream ID is unknown at this point */
    stream->stream_id = -1;
    rv = nghttp2_submit_settings(httpc->h2, NGHTTP2_FLAG_NONE,
                                 httpc->local_settings,
                                 httpc->local_settings_num);
    if(rv) {
      failf(data, "nghttp2_submit_settings() failed: %s(%d)",
            nghttp2_strerror(rv), rv);
      return CURLE_HTTP2;
    }
  }

  rv = nghttp2_session_set_local_window_size(httpc->h2, NGHTTP2_FLAG_NONE, 0,
                                             HTTP2_HUGE_WINDOW_SIZE);
  if(rv) {
    failf(data, "nghttp2_session_set_local_window_size() failed: %s(%d)",
          nghttp2_strerror(rv), rv);
    return CURLE_HTTP2;
  }

  /* we are going to copy mem to httpc->inbuf.  This is required since
     mem is part of buffer pointed by stream->mem, and callbacks
     called by nghttp2_session_mem_recv() will write stream specific
     data into stream->mem, overwriting data already there. */
  if(H2_BUFSIZE < nread) {
    failf(data, "connection buffer size is too small to store data following "
          "HTTP Upgrade response header: buflen=%d, datalen=%zu",
          H2_BUFSIZE, nread);
    return CURLE_HTTP2;
  }

<<<<<<< HEAD
  infof(data, "Copying HTTP/2 data in stream buffer to connection buffer"
        " after upgrade: len=%zu",
        nread);
=======
  result = h2_progress_ingress(cf, data);
  if(result)
    goto out;

  result = h2_progress_egress(cf, data);
  if(result)
    goto out;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))

  if(nread)
    memcpy(httpc->inbuf, mem, nread);

  httpc->inbuflen = nread;

  DEBUGASSERT(httpc->nread_inbuf == 0);

  if(-1 == h2_process_pending_input(data, httpc, &result))
    return CURLE_HTTP2;

  return CURLE_OK;
}

CURLcode Curl_http2_stream_pause(struct Curl_easy *data, bool pause)
{
<<<<<<< HEAD
  DEBUGASSERT(data);
  DEBUGASSERT(data->conn);
  /* if it isn't HTTP/2, we're done */
  if(!(data->conn->handler->protocol & PROTO_FAMILY_HTTP) ||
     !data->conn->proto.httpc.h2)
    return CURLE_OK;
#ifdef NGHTTP2_HAS_SET_LOCAL_WINDOW_SIZE
  else {
    struct HTTP *stream = data->req.p.http;
    struct http_conn *httpc = &data->conn->proto.httpc;
    uint32_t window = !pause * HTTP2_HUGE_WINDOW_SIZE;
    int rv = nghttp2_session_set_local_window_size(httpc->h2,
=======
  struct cf_h2_ctx *ctx = cf->ctx;

  if(ctx) {
    struct cf_call_data save;

    CF_DATA_SAVE(save, cf, data);
    cf_h2_ctx_clear(ctx);
    CF_DATA_RESTORE(cf, save);
  }
}

static void cf_h2_destroy(struct Curl_cfilter *cf, struct Curl_easy *data)
{
  struct cf_h2_ctx *ctx = cf->ctx;

  (void)data;
  if(ctx) {
    cf_h2_ctx_free(ctx);
    cf->ctx = NULL;
  }
}

static CURLcode http2_data_pause(struct Curl_cfilter *cf,
                                 struct Curl_easy *data,
                                 bool pause)
{
#ifdef NGHTTP2_HAS_SET_LOCAL_WINDOW_SIZE
  struct cf_h2_ctx *ctx = cf->ctx;
  struct stream_ctx *stream = H2_STREAM_CTX(data);

  DEBUGASSERT(data);
  if(ctx && ctx->h2 && stream) {
    uint32_t window = !pause * H2_STREAM_WINDOW_SIZE;
    CURLcode result;

    int rv = nghttp2_session_set_local_window_size(ctx->h2,
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
                                                   NGHTTP2_FLAG_NONE,
                                                   stream->id,
                                                   window);
    if(rv) {
      failf(data, "nghttp2_session_set_local_window_size() failed: %s(%d)",
            nghttp2_strerror(rv), rv);
      return CURLE_HTTP2;
    }

    if(!pause)
      drain_stream(cf, data, stream);

    /* make sure the window update gets sent */
<<<<<<< HEAD
    rv = h2_session_send(data, httpc->h2);
    if(rv)
      return CURLE_SEND_ERROR;
=======
    result = h2_progress_egress(cf, data);
    if(result)
      return result;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))

    if(!pause) {
      /* Unpausing a h2 transfer, requires it to be run again. The server
       * may send new DATA on us increasing the flow window, and it may
       * not. We may have already buffered and exhausted the new window
       * by operating on things in flight during the handling of other
       * transfers. */
      drain_stream(cf, data, stream);
      Curl_expire(data, 0, EXPIRE_RUN_NOW);
    }
    DEBUGF(infof(data, "Set HTTP/2 window size to %u for stream %u",
                 window, stream->id));

#ifdef DEBUGBUILD
    {
      /* read out the stream local window again */
      uint32_t window2 =
<<<<<<< HEAD
        nghttp2_session_get_stream_local_window_size(httpc->h2,
                                                     stream->stream_id);
=======
        nghttp2_session_get_stream_local_window_size(ctx->h2,
                                                     stream->id);
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
      DEBUGF(infof(data, "HTTP/2 window size is now %u for stream %u",
                   window2, stream->id));
    }
#endif
  }
#endif
  return CURLE_OK;
}

CURLcode Curl_http2_add_child(struct Curl_easy *parent,
                              struct Curl_easy *child,
                              bool exclusive)
{
  if(parent) {
    struct Curl_http2_dep **tail;
    struct Curl_http2_dep *dep = calloc(1, sizeof(struct Curl_http2_dep));
    if(!dep)
      return CURLE_OUT_OF_MEMORY;
    dep->data = child;

    if(parent->set.stream_dependents && exclusive) {
      struct Curl_http2_dep *node = parent->set.stream_dependents;
      while(node) {
        node->data->set.stream_depends_on = child;
        node = node->next;
      }

<<<<<<< HEAD
      tail = &child->set.stream_dependents;
      while(*tail)
        tail = &(*tail)->next;

      DEBUGASSERT(!*tail);
      *tail = parent->set.stream_dependents;
      parent->set.stream_dependents = 0;
    }

    tail = &parent->set.stream_dependents;
    while(*tail) {
      (*tail)->data->set.stream_depends_e = FALSE;
      tail = &(*tail)->next;
    }

    DEBUGASSERT(!*tail);
    *tail = dep;
  }

  child->set.stream_depends_on = parent;
  child->set.stream_depends_e = exclusive;
  return CURLE_OK;
=======
  CF_DATA_SAVE(save, cf, data);
  switch(event) {
  case CF_CTRL_DATA_SETUP:
    break;
  case CF_CTRL_DATA_PAUSE:
    result = http2_data_pause(cf, data, (arg1 != 0));
    break;
  case CF_CTRL_DATA_DONE_SEND: {
    result = http2_data_done_send(cf, data);
    break;
  }
  case CF_CTRL_DATA_DONE: {
    http2_data_done(cf, data, arg1 != 0);
    break;
  }
  default:
    break;
  }
  CF_DATA_RESTORE(cf, save);
  return result;
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
}

void Curl_http2_remove_child(struct Curl_easy *parent, struct Curl_easy *child)
{
<<<<<<< HEAD
  struct Curl_http2_dep *last = 0;
  struct Curl_http2_dep *data = parent->set.stream_dependents;
  DEBUGASSERT(child->set.stream_depends_on == parent);
=======
  struct cf_h2_ctx *ctx = cf->ctx;
  struct stream_ctx *stream = H2_STREAM_CTX(data);

  if(ctx && (!Curl_bufq_is_empty(&ctx->inbufq)
            || (stream && !Curl_bufq_is_empty(&stream->recvbuf))))
    return TRUE;
  return cf->next? cf->next->cft->has_data_pending(cf->next, data) : FALSE;
}
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))

  while(data && data->data != child) {
    last = data;
    data = data->next;
  }

  DEBUGASSERT(data);

  if(data) {
    if(last) {
      last->next = data->next;
    }
    else {
      parent->set.stream_dependents = data->next;
    }
<<<<<<< HEAD
    free(data);
=======
    *pres1 = (effective_max > INT_MAX)? INT_MAX : (int)effective_max;
    CF_DATA_RESTORE(cf, save);
    return CURLE_OK;
  default:
    break;
  }
  return cf->next?
    cf->next->cft->query(cf->next, data, query, pres1, pres2) :
    CURLE_UNKNOWN_OPTION;
}

struct Curl_cftype Curl_cft_nghttp2 = {
  "HTTP/2",
  CF_TYPE_MULTIPLEX,
  CURL_LOG_DEFAULT,
  cf_h2_destroy,
  cf_h2_connect,
  cf_h2_close,
  Curl_cf_def_get_host,
  cf_h2_get_select_socks,
  cf_h2_data_pending,
  cf_h2_send,
  cf_h2_recv,
  cf_h2_cntrl,
  cf_h2_is_alive,
  cf_h2_keep_alive,
  cf_h2_query,
};

static CURLcode http2_cfilter_add(struct Curl_cfilter **pcf,
                                  struct Curl_easy *data,
                                  struct connectdata *conn,
                                  int sockindex)
{
  struct Curl_cfilter *cf = NULL;
  struct cf_h2_ctx *ctx;
  CURLcode result = CURLE_OUT_OF_MEMORY;

  DEBUGASSERT(data->conn);
  ctx = calloc(sizeof(*ctx), 1);
  if(!ctx)
    goto out;

  result = Curl_cf_create(&cf, &Curl_cft_nghttp2, ctx);
  if(result)
    goto out;

  Curl_conn_cf_add(data, conn, sockindex, cf);
  result = CURLE_OK;

out:
  if(result)
    cf_h2_ctx_free(ctx);
  *pcf = result? NULL : cf;
  return result;
}

static CURLcode http2_cfilter_insert_after(struct Curl_cfilter *cf,
                                           struct Curl_easy *data)
{
  struct Curl_cfilter *cf_h2 = NULL;
  struct cf_h2_ctx *ctx;
  CURLcode result = CURLE_OUT_OF_MEMORY;

  (void)data;
  ctx = calloc(sizeof(*ctx), 1);
  if(!ctx)
    goto out;

  result = Curl_cf_create(&cf_h2, &Curl_cft_nghttp2, ctx);
  if(result)
    goto out;

  Curl_conn_cf_insert_after(cf, cf_h2);
  result = CURLE_OK;

out:
  if(result)
    cf_h2_ctx_free(ctx);
  return result;
}

bool Curl_cf_is_http2(struct Curl_cfilter *cf, const struct Curl_easy *data)
{
  (void)data;
  for(; cf; cf = cf->next) {
    if(cf->cft == &Curl_cft_nghttp2)
      return TRUE;
    if(cf->cft->flags & CF_TYPE_IP_CONNECT)
      return FALSE;
  }
  return FALSE;
}

bool Curl_conn_is_http2(const struct Curl_easy *data,
                        const struct connectdata *conn,
                        int sockindex)
{
  return conn? Curl_cf_is_http2(conn->cfilter[sockindex], data) : FALSE;
}

bool Curl_http2_may_switch(struct Curl_easy *data,
                           struct connectdata *conn,
                           int sockindex)
{
  (void)sockindex;
  if(!Curl_conn_is_http2(data, conn, sockindex) &&
     data->state.httpwant == CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE) {
#ifndef CURL_DISABLE_PROXY
    if(conn->bits.httpproxy && !conn->bits.tunnel_proxy) {
      /* We don't support HTTP/2 proxies yet. Also it's debatable
         whether or not this setting should apply to HTTP/2 proxies. */
      infof(data, "Ignoring HTTP/2 prior knowledge due to proxy");
      return FALSE;
    }
#endif
    return TRUE;
  }
  return FALSE;
}

CURLcode Curl_http2_switch(struct Curl_easy *data,
                           struct connectdata *conn, int sockindex)
{
  struct Curl_cfilter *cf;
  CURLcode result;

  DEBUGASSERT(!Curl_conn_is_http2(data, conn, sockindex));
  DEBUGF(infof(data, DMSGI(data, sockindex, "switching to HTTP/2")));

  result = http2_cfilter_add(&cf, data, conn, sockindex);
  if(result)
    return result;

  result = cf_h2_ctx_init(cf, data, FALSE);
  if(result)
    return result;

  conn->httpversion = 20; /* we know we're on HTTP/2 now */
  conn->bits.multiplex = TRUE; /* at least potentially multiplexed */
  conn->bundle->multiuse = BUNDLE_MULTIPLEX;
  multi_connchanged(data->multi);

  if(cf->next) {
    bool done;
    return Curl_conn_cf_connect(cf, data, FALSE, &done);
  }
  return CURLE_OK;
}

CURLcode Curl_http2_switch_at(struct Curl_cfilter *cf, struct Curl_easy *data)
{
  struct Curl_cfilter *cf_h2;
  CURLcode result;

  DEBUGASSERT(!Curl_cf_is_http2(cf, data));

  result = http2_cfilter_insert_after(cf, data);
  if(result)
    return result;

  cf_h2 = cf->next;
  result = cf_h2_ctx_init(cf_h2, data, FALSE);
  if(result)
    return result;

  cf->conn->httpversion = 20; /* we know we're on HTTP/2 now */
  cf->conn->bits.multiplex = TRUE; /* at least potentially multiplexed */
  cf->conn->bundle->multiuse = BUNDLE_MULTIPLEX;
  multi_connchanged(data->multi);

  if(cf_h2->next) {
    bool done;
    return Curl_conn_cf_connect(cf_h2, data, FALSE, &done);
  }
  return CURLE_OK;
}

CURLcode Curl_http2_upgrade(struct Curl_easy *data,
                            struct connectdata *conn, int sockindex,
                            const char *mem, size_t nread)
{
  struct Curl_cfilter *cf;
  struct cf_h2_ctx *ctx;
  CURLcode result;

  DEBUGASSERT(!Curl_conn_is_http2(data, conn, sockindex));
  DEBUGF(infof(data, DMSGI(data, sockindex, "upgrading to HTTP/2")));
  DEBUGASSERT(data->req.upgr101 == UPGR101_RECEIVED);

  result = http2_cfilter_add(&cf, data, conn, sockindex);
  if(result)
    return result;

  DEBUGASSERT(cf->cft == &Curl_cft_nghttp2);
  ctx = cf->ctx;

  result = cf_h2_ctx_init(cf, data, TRUE);
  if(result)
    return result;

  if(nread > 0) {
    /* Remaining data from the protocol switch reply is already using
     * the switched protocol, ie. HTTP/2. We add that to the network
     * inbufq. */
    ssize_t copied;

    copied = Curl_bufq_write(&ctx->inbufq,
                             (const unsigned char *)mem, nread, &result);
    if(copied < 0) {
      failf(data, "error on copying HTTP Upgrade response: %d", result);
      return CURLE_RECV_ERROR;
    }
    if((size_t)copied < nread) {
      failf(data, "connection buffer size could not take all data "
            "from HTTP Upgrade response header: copied=%zd, datalen=%zu",
            copied, nread);
      return CURLE_HTTP2;
    }
    infof(data, "Copied HTTP/2 data in stream buffer to connection buffer"
          " after upgrade: len=%zu", nread);
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
  }

  child->set.stream_depends_on = 0;
  child->set.stream_depends_e = FALSE;
}

void Curl_http2_cleanup_dependencies(struct Curl_easy *data)
{
  while(data->set.stream_dependents) {
    struct Curl_easy *tmp = data->set.stream_dependents->data;
    Curl_http2_remove_child(data, tmp);
    if(data->set.stream_depends_on)
      Curl_http2_add_child(data->set.stream_depends_on, tmp, FALSE);
  }

  if(data->set.stream_depends_on)
    Curl_http2_remove_child(data->set.stream_depends_on, data);
}

/* Only call this function for a transfer that already got a HTTP/2
   CURLE_HTTP2_STREAM error! */
bool Curl_h2_http_1_1_error(struct Curl_easy *data)
{
<<<<<<< HEAD
  struct HTTP *stream = data->req.p.http;
  return (stream->error == NGHTTP2_HTTP_1_1_REQUIRED);
=======
  struct stream_ctx *stream = H2_STREAM_CTX(data);
  return (stream && stream->error == NGHTTP2_HTTP_1_1_REQUIRED);
>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
}

#else /* !USE_NGHTTP2 */

/* Satisfy external references even if http2 is not compiled in. */
#include <curl/curl.h>

char *curl_pushheader_bynum(struct curl_pushheaders *h, size_t num)
{
  (void) h;
  (void) num;
  return NULL;
}

char *curl_pushheader_byname(struct curl_pushheaders *h, const char *header)
{
  (void) h;
  (void) header;
  return NULL;
}

#endif /* USE_NGHTTP2 */
