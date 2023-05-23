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
 * are also available at https://curl.haxx.se/docs/copyright.html.
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

#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_CRYPTO_AUTH)

#include "urldata.h"
#include "strcase.h"
#include "strdup.h"
#include "vauth/vauth.h"
#include "vauth/digest.h"
#include "http_aws_sigv4.h"
#include "curl_sha256.h"
#include "transfer.h"

#include "strcase.h"
#include "parsedate.h"
#include "sendf.h"

#include <time.h>

/* The last 3 #include files should be in this order */
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

#define HMAC_SHA256(k, kl, d, dl, o)        \
  do {                                      \
    ret = Curl_hmacit(Curl_HMAC_SHA256,     \
                      (unsigned char *)k,   \
                      (unsigned int)kl,     \
                      (unsigned char *)d,   \
                      (unsigned int)dl, o); \
    if(ret != CURLE_OK) {                   \
      goto fail;                            \
    }                                       \
  } while(0)

static void sha256_to_hex(char *dst, unsigned char *sha, size_t dst_l)
{
  int i;

  DEBUGASSERT(dst_l >= 65);
  for(i = 0; i < 32; ++i) {
    curl_msnprintf(dst + (i * 2), dst_l - (i * 2), "%02x", sha[i]);
  }
}

<<<<<<< HEAD
=======
static char *find_date_hdr(struct Curl_easy *data, const char *sig_hdr)
{
  char *tmp = Curl_checkheaders(data, sig_hdr, strlen(sig_hdr));

  if(tmp)
    return tmp;
  return Curl_checkheaders(data, STRCONST("Date"));
}

/* remove whitespace, and lowercase all headers */
static void trim_headers(struct curl_slist *head)
{
  struct curl_slist *l;
  for(l = head; l; l = l->next) {
    char *value; /* to read from */
    char *store;
    size_t colon = strcspn(l->data, ":");
    Curl_strntolower(l->data, l->data, colon);

    value = &l->data[colon];
    if(!*value)
      continue;
    ++value;
    store = value;

    /* skip leading whitespace */
    while(*value && ISBLANK(*value))
      value++;

    while(*value) {
      int space = 0;
      while(*value && ISBLANK(*value)) {
        value++;
        space++;
      }
      if(space) {
        /* replace any number of consecutive whitespace with a single space,
           unless at the end of the string, then nothing */
        if(*value)
          *store++ = ' ';
      }
      else
        *store++ = *value++;
    }
    *store = 0; /* null terminate */
  }
}

/* maximum length for the aws sivg4 parts */
#define MAX_SIGV4_LEN 64
#define MAX_SIGV4_LEN_TXT "64"

#define DATE_HDR_KEY_LEN (MAX_SIGV4_LEN + sizeof("X--Date"))

#define MAX_HOST_LEN 255
/* FQDN + host: */
#define FULL_HOST_LEN (MAX_HOST_LEN + sizeof("host:"))

/* string been x-PROVIDER-date:TIMESTAMP, I need +1 for ':' */
#define DATE_FULL_HDR_LEN (DATE_HDR_KEY_LEN + TIMESTAMP_SIZE + 1)

/* timestamp should point to a buffer of at last TIMESTAMP_SIZE bytes */
static CURLcode make_headers(struct Curl_easy *data,
                             const char *hostname,
                             char *timestamp,
                             char *provider1,
                             char **date_header,
                             char *content_sha256_header,
                             struct dynbuf *canonical_headers,
                             struct dynbuf *signed_headers)
{
  char date_hdr_key[DATE_HDR_KEY_LEN];
  char date_full_hdr[DATE_FULL_HDR_LEN];
  struct curl_slist *head = NULL;
  struct curl_slist *tmp_head = NULL;
  CURLcode ret = CURLE_OUT_OF_MEMORY;
  struct curl_slist *l;
  int again = 1;

  /* provider1 mid */
  Curl_strntolower(provider1, provider1, strlen(provider1));
  provider1[0] = Curl_raw_toupper(provider1[0]);

  msnprintf(date_hdr_key, DATE_HDR_KEY_LEN, "X-%s-Date", provider1);

  /* provider1 lowercase */
  Curl_strntolower(provider1, provider1, 1); /* first byte only */
  msnprintf(date_full_hdr, DATE_FULL_HDR_LEN,
            "x-%s-date:%s", provider1, timestamp);

  if(Curl_checkheaders(data, STRCONST("Host"))) {
    head = NULL;
  }
  else {
    char full_host[FULL_HOST_LEN + 1];

    if(data->state.aptr.host) {
      size_t pos;

      if(strlen(data->state.aptr.host) > FULL_HOST_LEN) {
        ret = CURLE_URL_MALFORMAT;
        goto fail;
      }
      strcpy(full_host, data->state.aptr.host);
      /* remove /r/n as the separator for canonical request must be '\n' */
      pos = strcspn(full_host, "\n\r");
      full_host[pos] = 0;
    }
    else {
      if(strlen(hostname) > MAX_HOST_LEN) {
        ret = CURLE_URL_MALFORMAT;
        goto fail;
      }
      msnprintf(full_host, FULL_HOST_LEN, "host:%s", hostname);
    }

    head = curl_slist_append(NULL, full_host);
    if(!head)
      goto fail;
  }


  if(*content_sha256_header) {
    tmp_head = curl_slist_append(head, content_sha256_header);
    if(!tmp_head)
      goto fail;
    head = tmp_head;
  }

  for(l = data->set.headers; l; l = l->next) {
    tmp_head = curl_slist_append(head, l->data);
    if(!tmp_head)
      goto fail;
    head = tmp_head;
  }

  trim_headers(head);

  *date_header = find_date_hdr(data, date_hdr_key);
  if(!*date_header) {
    tmp_head = curl_slist_append(head, date_full_hdr);
    if(!tmp_head)
      goto fail;
    head = tmp_head;
    *date_header = curl_maprintf("%s: %s", date_hdr_key, timestamp);
  }
  else {
    char *value;

    *date_header = strdup(*date_header);
    if(!*date_header)
      goto fail;

    value = strchr(*date_header, ':');
    if(!value)
      goto fail;
    ++value;
    while(ISBLANK(*value))
      ++value;
    strncpy(timestamp, value, TIMESTAMP_SIZE - 1);
    timestamp[TIMESTAMP_SIZE - 1] = 0;
  }

  /* alpha-sort in a case sensitive manner */
  do {
    again = 0;
    for(l = head; l; l = l->next) {
      struct curl_slist *next = l->next;

      if(next && strcmp(l->data, next->data) > 0) {
        char *tmp = l->data;

        l->data = next->data;
        next->data = tmp;
        again = 1;
      }
    }
  } while(again);

  for(l = head; l; l = l->next) {
    char *tmp;

    if(Curl_dyn_add(canonical_headers, l->data))
      goto fail;
    if(Curl_dyn_add(canonical_headers, "\n"))
      goto fail;

    tmp = strchr(l->data, ':');
    if(tmp)
      *tmp = 0;

    if(l != head) {
      if(Curl_dyn_add(signed_headers, ";"))
        goto fail;
    }
    if(Curl_dyn_add(signed_headers, l->data))
      goto fail;
  }

  ret = CURLE_OK;
fail:
  curl_slist_free_all(head);

  return ret;
}

#define CONTENT_SHA256_KEY_LEN (MAX_SIGV4_LEN + sizeof("X--Content-Sha256"))
/* add 2 for ": " between header name and value */
#define CONTENT_SHA256_HDR_LEN (CONTENT_SHA256_KEY_LEN + 2 + \
                                SHA256_HEX_LENGTH)

/* try to parse a payload hash from the content-sha256 header */
static char *parse_content_sha_hdr(struct Curl_easy *data,
                                   const char *provider1,
                                   size_t *value_len)
{
  char key[CONTENT_SHA256_KEY_LEN];
  size_t key_len;
  char *value;
  size_t len;

  key_len = msnprintf(key, sizeof(key), "x-%s-content-sha256", provider1);

  value = Curl_checkheaders(data, key, key_len);
  if(!value)
    return NULL;

  value = strchr(value, ':');
  if(!value)
    return NULL;
  ++value;

  while(*value && ISBLANK(*value))
    ++value;

  len = strlen(value);
  while(len > 0 && ISBLANK(value[len-1]))
    --len;

  *value_len = len;
  return value;
}

static CURLcode calc_payload_hash(struct Curl_easy *data,
                                  unsigned char *sha_hash, char *sha_hex)
{
  const char *post_data = data->set.postfields;
  size_t post_data_len = 0;
  CURLcode result;

  if(post_data) {
    if(data->set.postfieldsize < 0)
      post_data_len = strlen(post_data);
    else
      post_data_len = (size_t)data->set.postfieldsize;
  }
  result = Curl_sha256it(sha_hash, (const unsigned char *) post_data,
                         post_data_len);
  if(!result)
    sha256_to_hex(sha_hex, sha_hash);
  return result;
}

#define S3_UNSIGNED_PAYLOAD "UNSIGNED-PAYLOAD"

static CURLcode calc_s3_payload_hash(struct Curl_easy *data,
                                     Curl_HttpReq httpreq, char *provider1,
                                     unsigned char *sha_hash,
                                     char *sha_hex, char *header)
{
  bool empty_method = (httpreq == HTTPREQ_GET || httpreq == HTTPREQ_HEAD);
  /* The request method or filesize indicate no request payload */
  bool empty_payload = (empty_method || data->set.filesize == 0);
  /* The POST payload is in memory */
  bool post_payload = (httpreq == HTTPREQ_POST && data->set.postfields);
  CURLcode ret = CURLE_OUT_OF_MEMORY;

  if(empty_payload || post_payload) {
    /* Calculate a real hash when we know the request payload */
    ret = calc_payload_hash(data, sha_hash, sha_hex);
    if(ret)
      goto fail;
  }
  else {
    /* Fall back to s3's UNSIGNED-PAYLOAD */
    size_t len = sizeof(S3_UNSIGNED_PAYLOAD) - 1;
    DEBUGASSERT(len < SHA256_HEX_LENGTH); /* 16 < 65 */
    memcpy(sha_hex, S3_UNSIGNED_PAYLOAD, len);
    sha_hex[len] = 0;
  }

  /* format the required content-sha256 header */
  msnprintf(header, CONTENT_SHA256_HDR_LEN,
            "x-%s-content-sha256: %s", provider1, sha_hex);

  ret = CURLE_OK;
fail:
  return ret;
}

>>>>>>> 9f82eed7 (Updated Curl to 8.1.0 (#290))
CURLcode Curl_output_aws_sigv4(struct Curl_easy *data, bool proxy)
{
  CURLcode ret = CURLE_OUT_OF_MEMORY;
  struct connectdata *conn = data->conn;
  size_t len;
  const char *tmp0;
  const char *tmp1;
  char *provider0_low = NULL;
  char *provider0_up = NULL;
  char *provider1_low = NULL;
  char *provider1_mid = NULL;
  char *region = NULL;
  char *service = NULL;
  const char *hostname = conn->host.name;
#ifdef DEBUGBUILD
  char *force_timestamp;
#endif
  time_t clock;
  struct tm tm;
  char timestamp[17];
  char date[9];
  const char *content_type = Curl_checkheaders(data, STRCONST("Content-Type"));
  char *canonical_headers = NULL;
  char *signed_headers = NULL;
  Curl_HttpReq httpreq;
  const char *method;
  size_t post_data_len;
  const char *post_data = data->set.postfields ? data->set.postfields : "";
  unsigned char sha_hash[32];
  char sha_hex[65];
  char *canonical_request = NULL;
  char *request_type = NULL;
  char *credential_scope = NULL;
  char *str_to_sign = NULL;
  const char *user = data->state.aptr.user ? data->state.aptr.user : "";
  const char *passwd = data->state.aptr.passwd ? data->state.aptr.passwd : "";
  char *secret = NULL;
  unsigned char tmp_sign0[32] = {0};
  unsigned char tmp_sign1[32] = {0};
  char *auth_headers = NULL;

  DEBUGASSERT(!proxy);
  (void)proxy;

  if(Curl_checkheaders(data, STRCONST("Authorization"))) {
    /* Authorization already present, Bailing out */
    return CURLE_OK;
  }

  /*
   * Parameters parsing
   * Google and Outscale use the same OSC or GOOG,
   * but Amazon uses AWS and AMZ for header arguments.
   * AWS is the default because most of non-amazon providers
   * are still using aws:amz as a prefix.
   */
  tmp0 = data->set.str[STRING_AWS_SIGV4] ?
    data->set.str[STRING_AWS_SIGV4] : "aws:amz";
  tmp1 = strchr(tmp0, ':');
  len = tmp1 ? (size_t)(tmp1 - tmp0) : strlen(tmp0);
  if(len < 1) {
    infof(data, "first provider can't be empty");
    ret = CURLE_BAD_FUNCTION_ARGUMENT;
    goto fail;
  }
  provider0_low = malloc(len + 1);
  provider0_up = malloc(len + 1);
  if(!provider0_low || !provider0_up) {
    goto fail;
  }
  Curl_strntolower(provider0_low, tmp0, len);
  provider0_low[len] = '\0';
  Curl_strntoupper(provider0_up, tmp0, len);
  provider0_up[len] = '\0';

  if(tmp1) {
    tmp0 = tmp1 + 1;
    tmp1 = strchr(tmp0, ':');
    len = tmp1 ? (size_t)(tmp1 - tmp0) : strlen(tmp0);
    if(len < 1) {
      infof(data, "second provider can't be empty");
      ret = CURLE_BAD_FUNCTION_ARGUMENT;
      goto fail;
    }
    provider1_low = malloc(len + 1);
    provider1_mid = malloc(len + 1);
    if(!provider1_low || !provider1_mid) {
      goto fail;
    }
    Curl_strntolower(provider1_low, tmp0, len);
    provider1_low[len] = '\0';
    Curl_strntolower(provider1_mid, tmp0, len);
    provider1_mid[0] = Curl_raw_toupper(provider1_mid[0]);
    provider1_mid[len] = '\0';

    if(tmp1) {
      tmp0 = tmp1 + 1;
      tmp1 = strchr(tmp0, ':');
      len = tmp1 ? (size_t)(tmp1 - tmp0) : strlen(tmp0);
      if(len < 1) {
        infof(data, "region can't be empty");
        ret = CURLE_BAD_FUNCTION_ARGUMENT;
        goto fail;
      }
      region = Curl_memdup(tmp0, len + 1);
      if(!region) {
        goto fail;
      }
      region[len] = '\0';

      if(tmp1) {
        tmp0 = tmp1 + 1;
        service = strdup(tmp0);
        if(!service) {
          goto fail;
        }
        if(strlen(service) < 1) {
          infof(data, "service can't be empty");
          ret = CURLE_BAD_FUNCTION_ARGUMENT;
          goto fail;
        }
      }
    }
  }
  else {
    provider1_low = Curl_memdup(provider0_low, len + 1);
    provider1_mid = Curl_memdup(provider0_low, len + 1);
    if(!provider1_low || !provider1_mid) {
      goto fail;
    }
    provider1_mid[0] = Curl_raw_toupper(provider1_mid[0]);
  }

  if(!service) {
    tmp0 = hostname;
    tmp1 = strchr(tmp0, '.');
    if(!tmp1) {
      infof(data, "service missing in parameters or hostname");
      ret = CURLE_URL_MALFORMAT;
      goto fail;
    }
    len = tmp1 - tmp0;
    service = Curl_memdup(tmp0, len + 1);
    if(!service) {
      goto fail;
    }
    service[len] = '\0';

    if(!region) {
      tmp0 = tmp1 + 1;
      tmp1 = strchr(tmp0, '.');
      if(!tmp1) {
        infof(data, "region missing in parameters or hostname");
        ret = CURLE_URL_MALFORMAT;
        goto fail;
      }
      len = tmp1 - tmp0;
      region = Curl_memdup(tmp0, len + 1);
      if(!region) {
        goto fail;
      }
      region[len] = '\0';
    }
  }

#ifdef DEBUGBUILD
  force_timestamp = getenv("CURL_FORCETIME");
  if(force_timestamp)
    clock = 0;
  else
    time(&clock);
#else
  time(&clock);
#endif
  ret = Curl_gmtime(clock, &tm);
  if(ret != CURLE_OK) {
    goto fail;
  }
  if(!strftime(timestamp, sizeof(timestamp), "%Y%m%dT%H%M%SZ", &tm)) {
    goto fail;
  }
  memcpy(date, timestamp, sizeof(date));
  date[sizeof(date) - 1] = 0;

  if(content_type) {
    content_type = strchr(content_type, ':');
    if(!content_type) {
      ret = CURLE_FAILED_INIT;
      goto fail;
    }
    content_type++;
    /* Skip whitespace now */
    while(*content_type == ' ' || *content_type == '\t')
      ++content_type;

    canonical_headers = curl_maprintf("content-type:%s\n"
                                      "host:%s\n"
                                      "x-%s-date:%s\n",
                                      content_type,
                                      hostname,
                                      provider1_low, timestamp);
    signed_headers = curl_maprintf("content-type;host;x-%s-date",
                                   provider1_low);
  }
  else {
    canonical_headers = curl_maprintf("host:%s\n"
                                      "x-%s-date:%s\n",
                                      hostname,
                                      provider1_low, timestamp);
    signed_headers = curl_maprintf("host;x-%s-date", provider1_low);
  }

  if(!canonical_headers || !signed_headers) {
    goto fail;
  }

  if(data->set.postfieldsize < 0)
    post_data_len = strlen(post_data);
  else
    post_data_len = (size_t)data->set.postfieldsize;
  if(Curl_sha256it(sha_hash, (const unsigned char *) post_data,
                   post_data_len)) {
    goto fail;
  }

  sha256_to_hex(sha_hex, sha_hash, sizeof(sha_hex));

  Curl_http_method(data, conn, &method, &httpreq);

  canonical_request =
    curl_maprintf("%s\n" /* HTTPRequestMethod */
                  "%s\n" /* CanonicalURI */
                  "%s\n" /* CanonicalQueryString */
                  "%s\n" /* CanonicalHeaders */
                  "%s\n" /* SignedHeaders */
                  "%s",  /* HashedRequestPayload in hex */
                  method,
                  data->state.up.path,
                  data->state.up.query ? data->state.up.query : "",
                  canonical_headers,
                  signed_headers,
                  sha_hex);
  if(!canonical_request) {
    goto fail;
  }

  request_type = curl_maprintf("%s4_request", provider0_low);
  if(!request_type) {
    goto fail;
  }

  credential_scope = curl_maprintf("%s/%s/%s/%s",
                                   date, region, service, request_type);
  if(!credential_scope) {
    goto fail;
  }

  if(Curl_sha256it(sha_hash, (unsigned char *) canonical_request,
                   strlen(canonical_request))) {
    goto fail;
  }

  sha256_to_hex(sha_hex, sha_hash, sizeof(sha_hex));

  /*
   * Google allow to use rsa key instead of HMAC, so this code might change
   * In the future, but for now we support only HMAC version
   */
  str_to_sign = curl_maprintf("%s4-HMAC-SHA256\n" /* Algorithm */
                              "%s\n" /* RequestDateTime */
                              "%s\n" /* CredentialScope */
                              "%s",  /* HashedCanonicalRequest in hex */
                              provider0_up,
                              timestamp,
                              credential_scope,
                              sha_hex);
  if(!str_to_sign) {
    goto fail;
  }

  secret = curl_maprintf("%s4%s", provider0_up, passwd);
  if(!secret) {
    goto fail;
  }

  HMAC_SHA256(secret, strlen(secret),
              date, strlen(date), tmp_sign0);
  HMAC_SHA256(tmp_sign0, sizeof(tmp_sign0),
              region, strlen(region), tmp_sign1);
  HMAC_SHA256(tmp_sign1, sizeof(tmp_sign1),
              service, strlen(service), tmp_sign0);
  HMAC_SHA256(tmp_sign0, sizeof(tmp_sign0),
              request_type, strlen(request_type), tmp_sign1);
  HMAC_SHA256(tmp_sign1, sizeof(tmp_sign1),
              str_to_sign, strlen(str_to_sign), tmp_sign0);

  sha256_to_hex(sha_hex, tmp_sign0, sizeof(sha_hex));

  auth_headers = curl_maprintf("Authorization: %s4-HMAC-SHA256 "
                               "Credential=%s/%s, "
                               "SignedHeaders=%s, "
                               "Signature=%s\r\n"
                               "X-%s-Date: %s\r\n",
                               provider0_up,
                               user,
                               credential_scope,
                               signed_headers,
                               sha_hex,
                               provider1_mid,
                               timestamp);
  if(!auth_headers) {
    goto fail;
  }

  Curl_safefree(data->state.aptr.userpwd);
  data->state.aptr.userpwd = auth_headers;
  data->state.authhost.done = TRUE;
  ret = CURLE_OK;

fail:
  free(provider0_low);
  free(provider0_up);
  free(provider1_low);
  free(provider1_mid);
  free(region);
  free(service);
  free(canonical_headers);
  free(signed_headers);
  free(canonical_request);
  free(request_type);
  free(credential_scope);
  free(str_to_sign);
  free(secret);
  return ret;
}

#endif /* !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_CRYPTO_AUTH) */
