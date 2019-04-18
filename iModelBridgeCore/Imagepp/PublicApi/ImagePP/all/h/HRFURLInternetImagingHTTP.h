//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

BEGIN_IMAGEPP_NAMESPACE
class HRFURLInternetImagingHTTP : public HFCURLHTTPBase
    {
public:

    HDECLARE_CLASS_ID(HRFURLInternetImagingHTTPId, HFCURLHTTPBase);

    //--------------------------------------
    // Primary methods
    //--------------------------------------

    HRFURLInternetImagingHTTP(const Utf8String& pi_User,
                              const Utf8String& pi_Password,
                              const Utf8String& pi_Host,
                              const Utf8String& pi_Port,
                              const Utf8String& pi_Path,
                              const Utf8String& pi_ImageName,
                              bool    pi_IsHTTP);
    IMAGEPP_EXPORT HRFURLInternetImagingHTTP(const Utf8String& pi_URL, bool pi_IsHTTP);

    virtual         ~HRFURLInternetImagingHTTP();

    //--------------------------------------
    // methods
    //--------------------------------------

    // Image name
    IMAGEPP_EXPORT const Utf8String&      GetImage() const;

    // UTF8 encoded and escaped image name
    IMAGEPP_EXPORT bool               GetUTF8EscapedImage(string* po_pUTF8EscapedImage);

    // this static method indicates if a plain URL can be represented
    // by this specific URL
    IMAGEPP_EXPORT static bool    IsURLInternetImaging(const HFCURL* pi_pURL);

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
    Utf8String                  m_Image;

    // Disabled methods
    //--------------------------------------

    HRFURLInternetImagingHTTP(const HRFURLInternetImagingHTTP&);
    HRFURLInternetImagingHTTP& operator=(const HRFURLInternetImagingHTTP&);
    };
END_IMAGEPP_NAMESPACE
