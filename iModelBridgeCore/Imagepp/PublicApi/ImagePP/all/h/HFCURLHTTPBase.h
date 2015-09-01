//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLHTTPBase.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    static const WString& s_SchemeName()
        {   static const WString Val(L"http");
        return Val;
        }

    //:> constructor
    IMAGEPP_EXPORT                         HFCURLHTTPBase() { } //:> required for persistence
    IMAGEPP_EXPORT                         HFCURLHTTPBase(const WString& pi_URL,
                                                  bool    pi_IsHTTPURL);
    IMAGEPP_EXPORT                         HFCURLHTTPBase(const WString& pi_User,
                                                  const WString& pi_Password,
                                                  const WString& pi_Host,
                                                  const WString& pi_Port,
                                                  const WString& pi_Path,
                                                  const WString& pi_SearchPart,
                                                  bool    pi_IsHTTPURL);
    IMAGEPP_EXPORT virtual                 ~HFCURLHTTPBase();

    //:> Content access methods
    IMAGEPP_EXPORT virtual WString         GetURL() const;
    const WString&                 GetPath() const;
    const WString&                 GetSearchPart() const;

    //:> Overriden methods, used in relative path management
    IMAGEPP_EXPORT virtual bool           HasPathTo(HFCURL* pi_pURL);
    IMAGEPP_EXPORT virtual WString         FindPathTo(HFCURL* pi_pDest);
    IMAGEPP_EXPORT virtual HFCURL*         MakeURLTo(const WString& pi_Path);

    IMAGEPP_EXPORT virtual void            SetUTF8EscapedURLPath(const string* pi_pURLPath = 0);

#ifdef __HMR_DEBUG_MEMBER
    virtual void                   PrintState() const;
#endif

protected:

    string                   m_UTF8EscapedSearchPart;

private:

    // Components of the URLPath part of the URL string.
    WString                  m_Path;
    WString                  m_SearchPart;
    bool                    m_IsHTTPURL; //HTTP or HTTPS

    // Disabled methods
    HFCURLHTTPBase(const HFCURLHTTPBase&);
    HFCURLHTTPBase& operator=(const HFCURLHTTPBase&);

    };

END_IMAGEPP_NAMESPACE
#include "HFCURLHTTPBase.hpp"
