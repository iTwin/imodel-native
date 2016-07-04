/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/BeHttp/HttpHeaders.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/WString.h>
#include <Bentley/bmap.h>
#include <Bentley/DateTime.h>

#include <BeHttp/Http.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

typedef bmap<Utf8String, Utf8String> HttpHeaderMap;
typedef HttpHeaderMap& HttpHeaderMapR;
typedef const HttpHeaderMap& HttpHeaderMapCR;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE HttpHeaders
    {
protected:
    HttpHeaderMap m_headers;

public:
    BEHTTP_EXPORT virtual ~HttpHeaders ();

    // Return referance to internal map
    BEHTTP_EXPORT HttpHeaderMapCR GetMap () const;
    // Set header field value. Pass empty to remove previous value
    BEHTTP_EXPORT void   SetValue (Utf8StringCR field, Utf8StringCR value);
    // Set header field value or comma-append it to existing value 
    // Multiple header values: http://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html#sec4.2
    BEHTTP_EXPORT void   AddValue (Utf8StringCR field, Utf8StringCR value);
    // Return header field value
    BEHTTP_EXPORT Utf8CP GetValue (Utf8StringCR field) const;
    // Clear all headers fields
    BEHTTP_EXPORT void Clear ();
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpRequestHeaders : HttpHeaders
    {
    // Accept header
    BEHTTP_EXPORT void SetAccept (Utf8StringCR contentType);
    BEHTTP_EXPORT Utf8CP GetAccept () const;

    // Accept-Language header
    BEHTTP_EXPORT void SetAcceptLanguage (Utf8StringCR langugageRanges);
    BEHTTP_EXPORT Utf8CP GetAcceptLanguage () const;

    // Authorization header
    BEHTTP_EXPORT void SetAuthorization (Utf8StringCR authorization);
    BEHTTP_EXPORT Utf8CP GetAuthorization () const;

    // Content-Disposition header
    BEHTTP_EXPORT void SetContentDisposition (Utf8StringCR value);
    BEHTTP_EXPORT Utf8CP GetContentDisposition () const;

    // Content-Range header
    BEHTTP_EXPORT void SetContentRange (Utf8StringCR range);
    BEHTTP_EXPORT Utf8CP GetContentRange () const;

    // Content-Type header
    BEHTTP_EXPORT void SetContentType (Utf8StringCR type);
    BEHTTP_EXPORT Utf8CP GetContentType () const;

    // User-Agent header to identify application in the server. Example - "Bentley.SampleNavigator/2.0"
    BEHTTP_EXPORT void SetUserAgent (Utf8StringCR userAgent);
    BEHTTP_EXPORT Utf8CP GetUserAgent () const;

    // If-Modified-Since header
    BEHTTP_EXPORT void SetIfModifiedSince(Utf8StringCR dateTime);
    BEHTTP_EXPORT void SetIfModifiedSince(DateTimeCR dateTime);
    BEHTTP_EXPORT Utf8CP GetIfModifiedSince() const;

    // If-None-Match header
    BEHTTP_EXPORT void SetIfNoneMatch (Utf8StringCR etag);
    BEHTTP_EXPORT Utf8CP GetIfNoneMatch () const;

    // If-Match header
    BEHTTP_EXPORT void SetIfMatch (Utf8StringCR etag);
    BEHTTP_EXPORT Utf8CP GetIfMatch () const;
    };

typedef HttpRequestHeaders& HttpRequestHeadersR;
typedef const HttpRequestHeaders& HttpRequestHeadersCR;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpResponseHeaders : HttpHeaders
    {
    // ETag header
    BEHTTP_EXPORT void SetETag (Utf8StringCR etag);
    BEHTTP_EXPORT Utf8CP GetETag () const;

    // Range header
    BEHTTP_EXPORT void SetRange (Utf8StringCR range);
    BEHTTP_EXPORT Utf8CP GetRange () const;

    // Content-Range header
    BEHTTP_EXPORT void SetContentRange (Utf8StringCR range);
    BEHTTP_EXPORT Utf8CP GetContentRange () const;

    // Content-Type header
    BEHTTP_EXPORT void SetContentType (Utf8StringCR type);
    BEHTTP_EXPORT Utf8CP GetContentType () const;

    // Location header
    BEHTTP_EXPORT void SetLocation (Utf8StringCR type);
    BEHTTP_EXPORT Utf8CP GetLocation () const;

    // Server header
    BEHTTP_EXPORT void SetServer (Utf8StringCR server);
    BEHTTP_EXPORT Utf8CP GetServer () const;
    
    // Cache-Control header
    BEHTTP_EXPORT void SetCacheControl(Utf8StringCR cacheControl);
    BEHTTP_EXPORT Utf8CP GetCacheControl() const;
    };

typedef HttpResponseHeaders& HttpResponseHeadersR;
typedef const HttpResponseHeaders& HttpResponseHeadersCR;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct RangeHeaderValue
    {
    BEHTTP_EXPORT RangeHeaderValue ();
    BEHTTP_EXPORT RangeHeaderValue (uint64_t from, uint64_t to);

    BEHTTP_EXPORT static BentleyStatus Parse (Utf8CP stringValue, RangeHeaderValue& valueOut);
    BEHTTP_EXPORT Utf8String ToString () const;

    uint64_t from;
    uint64_t to;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentRangeHeaderValue
    {
private:
    bool m_hasRange;
    bool m_hasLength;

public:
    BEHTTP_EXPORT ContentRangeHeaderValue ();
    BEHTTP_EXPORT explicit ContentRangeHeaderValue (uint64_t length);
    BEHTTP_EXPORT ContentRangeHeaderValue (uint64_t from, uint64_t to);
    BEHTTP_EXPORT ContentRangeHeaderValue (uint64_t from, uint64_t to, uint64_t length);

    BEHTTP_EXPORT static BentleyStatus Parse (Utf8CP stringValue, ContentRangeHeaderValue& valueOut);
    BEHTTP_EXPORT Utf8String ToString () const;

    BEHTTP_EXPORT bool HasRange () const;
    BEHTTP_EXPORT bool HasLength () const;

    uint64_t from;
    uint64_t to;
    uint64_t length;
    };

END_BENTLEY_HTTP_NAMESPACE
