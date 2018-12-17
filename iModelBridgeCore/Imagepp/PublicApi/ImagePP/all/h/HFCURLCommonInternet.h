//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLCommonInternet.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCURLCommonInternet
//-----------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HFCURL.h>

BEGIN_IMAGEPP_NAMESPACE
//:Ignore
// URL specification at this level is:
// <scheme-type>[//][user[:password]@]host[:port][/[URLPath]]

// The URLPath part of the string will have to be analyzed by derived classes.
//:End Ignore


/**

    This class defines the common attributes for a set of classes that
    correspond to a group of scheme types used in URL specifications, which
    are those used to access resouces on the internet.  Since all URLs used
    for internet have common features, an intermediary abstract class has
    been defined tu support these common features.

    At this level, the text of URLs are considered to respect the following
    structure:

    @code
    @i{schemetype}:[//][@i{user}[:@i{password}]@]@i{host}[:@i{port}][/[@i{urlpath}]]
    @end

    where the text in italics has to be replaced by corresponding value, and
    text between brackets is facultative.

    This class adds, to inherited interface, methods to access the parsed
    values extracted from the URL text.

    @see HFCURL

*/


class HFCURLCommonInternet : public HFCURL
    {
public:

    HDECLARE_CLASS_ID(HFCURLId_CommonInternet, HFCURL);

    //:> Primary methods

    IMAGEPP_EXPORT HFCURLCommonInternet(const Utf8String& pi_URL);
    IMAGEPP_EXPORT HFCURLCommonInternet(const Utf8String& pi_SchemeType,
                                        const Utf8String& pi_User,
                                        const Utf8String& pi_Password,
                                        const Utf8String& pi_Host,
                                        const Utf8String& pi_Port,
                                        const Utf8String& pi_URLPath);

    IMAGEPP_EXPORT virtual               ~HFCURLCommonInternet();

    IMAGEPP_EXPORT static void    SplitPath(const Utf8String& pi_rURL,
                                    Utf8String*       po_pScheme,
                                    Utf8String*       po_pHost,
                                    Utf8String*       po_pPort,
                                    Utf8String*       po_pUser,
                                    Utf8String*       po_pPassword,
                                    Utf8String*       po_pPath);

    IMAGEPP_EXPORT static string  EscapeURLParamValue(const string& pi_rURLPart);

    //:> Content access methods

    const Utf8String&      GetUser() const;
    const Utf8String&      GetPassword() const;
    const Utf8String&      GetHost() const;
    const Utf8String&      GetPort() const;
    const Utf8String&      GetURLPath() const;

    const string&       GetUTF8EscapedURLPath() const;
    IMAGEPP_EXPORT virtual void        SetUTF8EscapedURLPath(const string* pi_pURLPath = 0);

    //:> Overriden methods, used in relative path management

    IMAGEPP_EXPORT bool       HasPathTo(HFCURL* pi_pURL) override;

protected:
    // Components of the scheme-specific part of the URL string.
    Utf8String m_User;
    Utf8String m_Password;
    Utf8String m_Host;
    Utf8String m_Port;
    Utf8String m_URLPath;
    string  m_UTF8EscapedURLPath;

private:

    // Disabled methods
    HFCURLCommonInternet(const HFCURLCommonInternet&);
    HFCURLCommonInternet& operator=(const HFCURLCommonInternet&);
    };

END_IMAGEPP_NAMESPACE
#include "HFCURLCommonInternet.hpp"
