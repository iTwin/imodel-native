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
        Utf8String m_content;

    public:
        BeUri() = default;
        BeUri(Utf8StringCR content) : m_content(content) {}
        Utf8StringCR GetContent() const { return m_content; }
        BEHTTP_EXPORT Utf8String GetScheme(bool lowerCase = true) const;
        BEHTTP_EXPORT Utf8String GetHost(bool lowerCase = true) const;
    };

typedef BeUri& BeUriR;
typedef const BeUri& BeUriCR;

END_BENTLEY_HTTP_NAMESPACE
