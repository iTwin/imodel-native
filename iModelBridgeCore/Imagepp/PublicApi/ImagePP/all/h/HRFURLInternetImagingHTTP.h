//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFURLInternetImagingHTTP.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFURLInternetImagingHTTP.h
//-----------------------------------------------------------------------------
// This class defines a specialized version of URL interface for the "http:"
// scheme type.
//-----------------------------------------------------------------------------
#pragma once
#include "HFCURLHTTPBase.h"

// URL specification at this level is:
// http:[//][user[:password]@]host[:port][/[Path]]?FIF=image

class HRFURLInternetImagingHTTP : public HFCURLHTTPBase
    {
public:

    HDECLARE_CLASS_ID(1305, HFCURLHTTPBase);

    //--------------------------------------
    // Primary methods
    //--------------------------------------

    HRFURLInternetImagingHTTP(const WString& pi_User,
                              const WString& pi_Password,
                              const WString& pi_Host,
                              const WString& pi_Port,
                              const WString& pi_Path,
                              const WString& pi_ImageName,
                              bool    pi_IsHTTP);
    HRFURLInternetImagingHTTP(const WString& pi_URL,
                              bool    pi_IsHTTP);
    HRFURLInternetImagingHTTP() { } // required for persistence
    virtual         ~HRFURLInternetImagingHTTP();

    //--------------------------------------
    // methods
    //--------------------------------------

    // Image name
    const WString&      GetImage() const;

    // UTF8 encoded and escaped image name
    bool               GetUTF8EscapedImage(string* po_pUTF8EscapedImage);

    // this static method indicates if a plain URL can be represented
    // by this specific URL
    _HDLLg static bool    IsURLInternetImaging(const HFCURL* pi_pURL);

#ifdef __HMR_DEBUG_MEMBER
    virtual void    PrintState() const;
#endif

private:
    //--------------------------------------
    // Friends
    //--------------------------------------

    friend struct URLIIPHTTPCreator;

    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Image name
    WString                  m_Image;

    // Disabled methods
    //--------------------------------------

    HRFURLInternetImagingHTTP(const HRFURLInternetImagingHTTP&);
    HRFURLInternetImagingHTTP& operator=(const HRFURLInternetImagingHTTP&);
    };
