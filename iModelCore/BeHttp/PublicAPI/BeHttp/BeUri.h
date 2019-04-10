/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/BeHttp/BeUri.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/Http.h>
#include <Bentley/WString.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                               Travis.Cobbs          10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct BeUri
    {
    private:
        struct MatchResults;

    private:
        static Utf8String GetUriComponent(size_t group, const MatchResults& results);
        static Utf8String EscapeUriComponent(Utf8String component);

    private:
        Utf8String m_scheme;
        Utf8String m_userInfo;
        Utf8String m_host;
        Utf8String m_path;
        Utf8String m_query;
        Utf8String m_fragment;
        int32_t m_port = -1;
        bool m_authorityIsPresent = false;
        bool m_valid = false;

    public:
        //! Creates invalid URI
        BeUri() = default;

        //! Creates URI and percent-escapes any unsafe symbols in path, query, and fragment components.
        //! IMPORTANT: Make sure that the input string begins with a scheme. According to URI specification,
        //! "localhost:1234" is parsed as a URI containing scheme component "localhost" and path component "1234".
        //! Complete URI specification is available in <a href="https://tools.ietf.org/html/rfc3986#section-3">[RFC 3986] section 3</a>.
        BEHTTP_EXPORT BeUri(Utf8StringCR uri);

        //! Returns whether the uri is valid
        bool IsValid() const { return m_valid; }

        //! Returns full URI string in original letter case
        BEHTTP_EXPORT Utf8String ToString() const;

        //! Returns scheme component in lower case
        BEHTTP_EXPORT Utf8String GetScheme() const { return Utf8String(m_scheme).ToLower(); }

        //! Returns authority component with host in lower case
        BEHTTP_EXPORT Utf8String GetAuthority() const;

        //! Returns userinfo component in original letter case
        BEHTTP_EXPORT Utf8StringCR GetUserInfo() const { return m_userInfo; }

        //! Returns host component in lower case
        BEHTTP_EXPORT Utf8String GetHost() const { return Utf8String(m_host).ToLower(); }

        //! Returns port number. If port was not specified, returns -1.
        BEHTTP_EXPORT int32_t GetPort() const { return m_port; }

        //! Returns path component in original letter case
        BEHTTP_EXPORT Utf8String GetPath() const;

        //! Returns query component in original letter case
        BEHTTP_EXPORT Utf8StringCR GetQuery() const { return m_query; }

        //! Returns fragment component in original letter case
        BEHTTP_EXPORT Utf8StringCR GetFragment() const { return m_fragment; }

        //! Use percent-escape for specific URL parts
        BEHTTP_EXPORT static Utf8String EscapeString(Utf8StringCR str);
        //! Remove percent-escape to original characters
        BEHTTP_EXPORT static Utf8String UnescapeString(Utf8StringCR str);

        //! Use percent-escape for unsafe characters in URL. This is done automaticaly before using URL in Http::Request
        //! More info: https://tools.ietf.org/html/rfc1738#section-2.2 and https://tools.ietf.org/html/rfc2396
        BEHTTP_EXPORT static Utf8String& EscapeUnsafeCharactersInUrl(Utf8String& url);
    };

typedef BeUri& BeUriR;
typedef const BeUri& BeUriCR;

END_BENTLEY_HTTP_NAMESPACE
