/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/BeHttp/BeUri.h $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
        Utf8String m_uri;

    public:
        BeUri() = default;
        //! Creates URI and percent-escapes any unsafe symbols
        BEHTTP_EXPORT BeUri(Utf8String uri);

        //! Get full URI string
        Utf8StringCR GetString() const { return m_uri; }

        //! Parse and get URI scheme
        BEHTTP_EXPORT Utf8String GetScheme(bool lowerCase = true) const;
        //! Parse and get URI host
        BEHTTP_EXPORT Utf8String GetHost(bool lowerCase = true) const;

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
