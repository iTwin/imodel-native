//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLECWPS.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCURLECWPS
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURLCommonInternet.h"

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

    @see HFCURLCommonIntenret
    @see HFCURL

*/

class HFCURLECWPS : public HFCURLCommonInternet
    {
public:

    HDECLARE_CLASS_ID(HFCURLId_ECWPS, HFCURLCommonInternet);

    // Define the Scheme label
    static const Utf8String& s_SchemeName()
        {   static const Utf8String Val("ecwps");
        return Val;
        }

    //:> constructor
    IMAGEPP_EXPORT                         HFCURLECWPS(const Utf8String& pi_URL);
    IMAGEPP_EXPORT                         HFCURLECWPS(const Utf8String& pi_User,
                                               const Utf8String& pi_Password,
                                               const Utf8String& pi_Host,
                                               const Utf8String& pi_Port,
                                               const Utf8String& pi_Path,
                                               const Utf8String& pi_SearchPart);
    IMAGEPP_EXPORT virtual                 ~HFCURLECWPS();

    //:> Content access methods
    IMAGEPP_EXPORT Utf8String         GetURL() const override;
    const Utf8String&          GetPath() const;
    const Utf8String&          GetSearchPart() const;

    //:> Overriden methods, used in relative path management
    IMAGEPP_EXPORT bool           HasPathTo(HFCURL* pi_pURL) override;
    IMAGEPP_EXPORT Utf8String         FindPathTo(HFCURL* pi_pDest) override;
    IMAGEPP_EXPORT HFCURL*         MakeURLTo(const Utf8String& pi_Path) override;

#ifdef __HMR_DEBUG_MEMBER
    virtual void            PrintState() const;
#endif

protected:

private:

    friend struct URLECWPSCreator;

    // Components of the URLPath part of the URL string.
    Utf8String                  m_Path;
    Utf8String                  m_SearchPart;

    // Disabled methods
    HFCURLECWPS(const HFCURLECWPS&);
    HFCURLECWPS& operator=(const HFCURLECWPS&);

    };
END_IMAGEPP_NAMESPACE

#include "HFCURLECWPS.hpp"

