//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLHTTPS.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCURLHTTPS
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURLHTTPBase.h"

BEGIN_IMAGEPP_NAMESPACE
//:Ignore
// URL specification at this level is:
// http:[//][user[:password]@]host[:port][/[Path[?SearchPart]]]
//:End Ignore

/**

    This class defines the URL descriptor for the "http" scheme type.  That
    kind of URL is used to locate files that are accessible through HTTP
    server (web servers).  The structure of the text of the URLs follows the
    syntax of the common internet scheme types, with one particularity: the
    url-path section is divided into two components, one being the path
    itself, the second being a facultative search expression.  The resulting
    syntax is as follows:

    @code
    http://[@i{user}[:@i{password}]@]@i{host}[:@i{port}][/[@i{path}[?@i{search}]]]
    @end

    where the text in italics has to be replaced by corresponding value, and
    text between brackets is facultative.

    This class adds, to inherited interface, new methods to access the
    parsed values extracted from the URL text (the path and search
    expression).  The inherited method GetURLPath is still available, in
    this case it returns the combination of the path, the question mark and
    the search expression.

    @see HFCURLHTTPBase
    @see HFCURL

*/

class HFCURLHTTPS : public HFCURLHTTPBase
    {
public:

    HDECLARE_CLASS_ID(HFCURLId_HTTPS, HFCURLHTTPBase);

    // Define the Scheme label
    static const Utf8String& s_SchemeName()
        {   static const Utf8String Val("https");
        return Val;
        }

    //:> constructor
    IMAGEPP_EXPORT                         HFCURLHTTPS(const Utf8String& pi_URL);
    IMAGEPP_EXPORT                         HFCURLHTTPS(const Utf8String& pi_User,
                                               const Utf8String& pi_Password,
                                               const Utf8String& pi_Host,
                                               const Utf8String& pi_Port,
                                               const Utf8String& pi_Path,
                                               const Utf8String& pi_SearchPart);
    IMAGEPP_EXPORT virtual                 ~HFCURLHTTPS();

private:

    friend struct URLHTTPSCreator;

    // Disabled methods
    HFCURLHTTPS(const HFCURLHTTPS&);
    HFCURLHTTPS& operator=(const HFCURLHTTPS&);
    };
END_IMAGEPP_NAMESPACE
