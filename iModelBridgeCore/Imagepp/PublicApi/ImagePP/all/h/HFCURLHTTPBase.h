//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCURLHTTP
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURLCommonInternet.h"

BEGIN_IMAGEPP_NAMESPACE
//:Ignore
// URL specification at this level is:
// [http|https]:[//][user[:password]@]host[:port][/[Path[?SearchPart]]]
//:End Ignore

/**

    This class is the base called for the classes that define the URL descriptor
    for the "http" and "https scheme type.

    @see HFCURLHTTP
    @see HFCURLHTTPS
    @see HFCURLCommonInternet
    @see HFCURL

*/

class HFCURLHTTPBase : public HFCURLCommonInternet
    {
public:

    HDECLARE_CLASS_ID(HFCURLId_HTTPBase, HFCURLCommonInternet);

    // Define the Scheme label
    static const Utf8String& s_SchemeName()
        {   static const Utf8String Val("http");
        return Val;
        }

    //:> constructor
    IMAGEPP_EXPORT                         HFCURLHTTPBase(const Utf8String& pi_URL,
                                                  bool    pi_IsHTTPURL);
    IMAGEPP_EXPORT                         HFCURLHTTPBase(const Utf8String& pi_User,
                                                  const Utf8String& pi_Password,
                                                  const Utf8String& pi_Host,
                                                  const Utf8String& pi_Port,
                                                  const Utf8String& pi_Path,
                                                  const Utf8String& pi_SearchPart,
                                                  bool    pi_IsHTTPURL);
    IMAGEPP_EXPORT virtual                 ~HFCURLHTTPBase();

    //:> Content access methods
    IMAGEPP_EXPORT Utf8String         GetURL() const override;
    const Utf8String&                 GetPath() const;
    const Utf8String&                 GetSearchPart() const;

    //:> Overriden methods, used in relative path management
    IMAGEPP_EXPORT bool           HasPathTo(HFCURL* pi_pURL) override;
    IMAGEPP_EXPORT Utf8String         FindPathTo(HFCURL* pi_pDest) override;
    IMAGEPP_EXPORT HFCURL*         MakeURLTo(const Utf8String& pi_Path) override;

    IMAGEPP_EXPORT void            SetUTF8EscapedURLPath(const string* pi_pURLPath = 0) override;

#ifdef __HMR_DEBUG_MEMBER
    virtual void                   PrintState() const;
#endif

protected:

    string                   m_UTF8EscapedSearchPart;

private:

    // Components of the URLPath part of the URL string.
    Utf8String                  m_Path;
    Utf8String                  m_SearchPart;
    bool                    m_IsHTTPURL; //HTTP or HTTPS

    // Disabled methods
    HFCURLHTTPBase(const HFCURLHTTPBase&);
    HFCURLHTTPBase& operator=(const HFCURLHTTPBase&);

    };

END_IMAGEPP_NAMESPACE
#include "HFCURLHTTPBase.hpp"
