/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/WString.h>
#include <Bentley/bmap.h>
#include <Bentley/DateTime.h>
#include <BeHttp/Http.h>

#define REQUESTHEADER_ContentType_ApplicationJson "application/json"
#define REQUESTHEADER_ContentType_ApplicationXml  "application/xml"
#define REQUESTHEADER_ContentType_TextHtml        "text/html"

BEGIN_BENTLEY_HTTP_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
struct CompareInsensitiveUtfString
    {
    bool operator()(Utf8String const& lhs, Utf8String const& rhs) const { return (lhs.CompareToI(rhs) < 0); }
    };

// RFC 7230 states: "Each header field consists of a name followed by a colon (":") and the field value. Field names are case-insensitive.
typedef bmap<Utf8String, Utf8String, CompareInsensitiveUtfString> HttpHeaderMap;

DEFINE_POINTER_SUFFIX_TYPEDEFS(HttpRequestHeaders)
DEFINE_POINTER_SUFFIX_TYPEDEFS(HttpResponseHeaders)

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpHeaders
{
protected:
    HttpHeaderMap m_headers;

public:
    // Return reference to internal map
    HttpHeaderMap const& GetMap() const {return m_headers;}

    // Return referance to internal map. Always use Set/Add methods instead of directly modifying map when possible.
    HttpHeaderMap& GetMap() { return m_headers; };

    // Set header field value. Pass empty to remove previous value
    BEHTTP_EXPORT void SetValue(Utf8StringCR field, Utf8StringCR value);

    // Set header field value or comma-append it to existing value 
    // Multiple header values: http://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html#sec4.2
    BEHTTP_EXPORT void AddValue(Utf8StringCR field, Utf8StringCR value);

    // Return header field value
    BEHTTP_EXPORT Utf8CP GetValue(Utf8StringCR field) const;

    // Clear all headers fields
    void Clear() {return m_headers.clear();}
};

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpRequestHeaders : HttpHeaders
{
    // Accept header
    void SetAccept(Utf8StringCR value) {SetValue("Accept", value);}
    Utf8CP GetAccept() const {return GetValue("Accept");}

    // Accept-Language header
    void SetAcceptLanguage(Utf8StringCR langugageRanges) {SetValue("Accept-Language", langugageRanges);}
    Utf8CP GetAcceptLanguage() const {return GetValue("Accept-Language");}

    // Authorization header
    void SetAuthorization(Utf8StringCR authorization) {SetValue("Authorization", authorization);}
    Utf8CP GetAuthorization() const {return GetValue("Authorization");}

    // Content-Disposition header
    void SetContentDisposition(Utf8StringCR value) {SetValue("Content-Disposition", value);}
    Utf8CP GetContentDisposition() const {return GetValue("Content-Disposition");}

    // Content-Range header
    void SetContentRange(Utf8StringCR range) {SetValue("Content-Range", range);}
    Utf8CP GetContentRange() const {return GetValue("Content-Range");}

    // Content-Type header
    void SetContentType(Utf8StringCR type) {SetValue("Content-Type", type);}
    Utf8CP GetContentType() const {return GetValue("Content-Type");}

    // User-Agent header to identify application in the server. Example - "Bentley.SampleNavigator/2.0"
    void SetUserAgent(Utf8StringCR userAgent) {SetValue("User-Agent", userAgent);}
    Utf8CP GetUserAgent() const {return GetValue("User-Agent");}

    // If-Modified-Since header
    void SetIfModifiedSince(Utf8StringCR dateTime) {SetValue("If-Modified-Since", dateTime);}
    BEHTTP_EXPORT void SetIfModifiedSince(DateTimeCR dateTime);
    Utf8CP GetIfModifiedSince() const {return GetValue("If-Modified-Since");}

    // If-None-Match header
    void SetIfNoneMatch(Utf8StringCR etag) {SetValue("If-None-Match", etag);}
    Utf8CP GetIfNoneMatch() const {return GetValue("If-None-Match");}

    // If-Match header
    void SetIfMatch(Utf8StringCR etag) {SetValue("If-Match", etag);}
    Utf8CP GetIfMatch() const {return GetValue("If-Match");}
};

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpResponseHeaders : HttpHeaders
{
    // ETag header
    void SetETag(Utf8StringCR etag) {SetValue("ETag", etag);}
    Utf8CP GetETag() const {return GetValue("ETag");}

    // Range header
    void SetRange(Utf8StringCR range) {SetValue("Range", range);}
    Utf8CP GetRange() const {return GetValue("Range");}

    // Content-Range header
    void SetContentRange(Utf8StringCR range) {SetValue("Content-Range", range);}
    Utf8CP GetContentRange() const {return GetValue("Content-Range");}

    // Content-Type header
    void SetContentType(Utf8StringCR type) {SetValue("Content-Type", type);}
    Utf8CP GetContentType() const {return GetValue("Content-Type");}

    // Location header
    void SetLocation(Utf8StringCR type) {SetValue("Location", type);}
    Utf8CP GetLocation() const {return GetValue("Location");}

    // Server header
    void SetServer(Utf8StringCR server) {SetValue("Server", server);}
    Utf8CP GetServer() const {return GetValue("Server");}

    // Cache-Control header
    void SetCacheControl(Utf8StringCR cacheControl) {SetValue("Cache-Control", cacheControl);}
    Utf8CP GetCacheControl() const {return GetValue("Cache-Control");}

    // Proxy-Authenticate header. Use AuthenticationChallengeValue to parse it
    void SetProxyAuthenticate(Utf8StringCR value) {SetValue("Proxy-Authenticate", value);}
    Utf8CP GetProxyAuthenticate() const {return GetValue("Proxy-Authenticate");}
};

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct RangeHeaderValue
{
private:
    uint64_t m_from = 0;
    uint64_t m_to = 0;

public:
    RangeHeaderValue() {}
    RangeHeaderValue(uint64_t from, uint64_t to) : m_from(from), m_to(to) {}

    BEHTTP_EXPORT static BentleyStatus Parse(Utf8CP stringValue, RangeHeaderValue& valueOut);
    BEHTTP_EXPORT Utf8String ToString() const;
    uint64_t GetFrom() const {return m_from;}
    uint64_t GetTo() const {return m_to;}
};

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentRangeHeaderValue
{
private:
    bool m_hasRange = false;
    bool m_hasLength = false;
    uint64_t m_from = 0;
    uint64_t m_to = 0;
    uint64_t m_length = 0;

public:
    ContentRangeHeaderValue() {}
    explicit ContentRangeHeaderValue(uint64_t length) : m_length(length) {}
    ContentRangeHeaderValue(uint64_t from, uint64_t to) : m_hasRange(true), m_hasLength(false), m_from(from), m_to(to), m_length(0){}
    ContentRangeHeaderValue(uint64_t from, uint64_t to, uint64_t length) : m_hasRange(true), m_hasLength(true), m_from(from), m_to(to), m_length(length){}

    BEHTTP_EXPORT static BentleyStatus Parse(Utf8CP stringValue, ContentRangeHeaderValue& valueOut);
    BEHTTP_EXPORT Utf8String ToString() const;

    bool HasRange() const {return m_hasRange;}
    bool HasLength() const {return m_hasLength;}
    uint64_t GetFrom() const {return m_from;}
    uint64_t GetTo() const {return m_to;}
    uint64_t GetLength() const {return m_length;}
};

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct AuthenticationChallengeValue
    {
private:
    Utf8String m_type;
    Utf8String m_realm;

public:
    AuthenticationChallengeValue() = default;
    AuthenticationChallengeValue(Utf8String type, Utf8String realm) : m_type(type), m_realm(realm) {}

    BEHTTP_EXPORT static BentleyStatus Parse(Utf8CP stringValue, AuthenticationChallengeValue& valueOut);
    BEHTTP_EXPORT Utf8String ToString() const;

    Utf8StringCR GetType() const {return m_type;}
    Utf8StringCR GetRealm() const {return m_realm;}
    };

END_BENTLEY_HTTP_NAMESPACE
