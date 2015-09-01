//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLCommonInternet.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCURLCommonInternet
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCURL.h>

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

    HFCURLCommonInternet(const WString& pi_URL);
    HFCURLCommonInternet(const WString& pi_SchemeType,
                         const WString& pi_User,
                         const WString& pi_Password,
                         const WString& pi_Host,
                         const WString& pi_Port,
                         const WString& pi_URLPath);
    HFCURLCommonInternet() { } // required for persistence
    virtual               ~HFCURLCommonInternet();

    IMAGEPP_EXPORT static void    SplitPath(const WString& pi_rURL,
                                    WString*       po_pScheme,
                                    WString*       po_pHost,
                                    WString*       po_pPort,
                                    WString*       po_pUser,
                                    WString*       po_pPassword,
                                    WString*       po_pPath);

    IMAGEPP_EXPORT static string  EscapeURLParamValue(const string& pi_rURLPart);

    //:> Content access methods

    const WString&      GetUser() const;
    const WString&      GetPassword() const;
    const WString&      GetHost() const;
    const WString&      GetPort() const;
    const WString&      GetURLPath() const;

    const string&       GetUTF8EscapedURLPath() const;
    virtual void        SetUTF8EscapedURLPath(const string* pi_pURLPath = 0);

    //:> Overriden methods, used in relative path management

    virtual bool       HasPathTo(HFCURL* pi_pURL);

protected:
    // Components of the scheme-specific part of the URL string.
    WString m_User;
    WString m_Password;
    WString m_Host;
    WString m_Port;
    WString m_URLPath;
    string  m_UTF8EscapedURLPath;

private:

    // Disabled methods
    HFCURLCommonInternet(const HFCURLCommonInternet&);
    HFCURLCommonInternet& operator=(const HFCURLCommonInternet&);
    };

END_IMAGEPP_NAMESPACE
#include "HFCURLCommonInternet.hpp"