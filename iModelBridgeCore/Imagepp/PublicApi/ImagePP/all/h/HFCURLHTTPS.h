//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLHTTPS.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    static const WString& s_SchemeName()
        {   static const WString Val(L"https");
        FREEZE_STL_STRING(Val);
        return Val;
        }

    //:> constructor
    IMAGEPP_EXPORT                         HFCURLHTTPS() { } //:> required for persistence
    IMAGEPP_EXPORT                         HFCURLHTTPS(const WString& pi_URL);
    IMAGEPP_EXPORT                         HFCURLHTTPS(const WString& pi_User,
                                               const WString& pi_Password,
                                               const WString& pi_Host,
                                               const WString& pi_Port,
                                               const WString& pi_Path,
                                               const WString& pi_SearchPart);
    IMAGEPP_EXPORT virtual                 ~HFCURLHTTPS();

private:

    friend struct URLHTTPSCreator;

    // Disabled methods
    HFCURLHTTPS(const HFCURLHTTPS&);
    HFCURLHTTPS& operator=(const HFCURLHTTPS&);
    };
END_IMAGEPP_NAMESPACE