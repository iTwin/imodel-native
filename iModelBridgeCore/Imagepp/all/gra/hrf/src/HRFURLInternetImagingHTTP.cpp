/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRFURLInternetImagingHTTP
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HFCURLHTTP.h>
#include <ImagePP/all/h/HFCURLHTTPS.h>
#include <ImagePP/all/h/HRFURLInternetImagingHTTP.h>

//-----------------------------------------------------------------------------
// This constructor configures the object from the detached parts of the URL
// specification.
//-----------------------------------------------------------------------------
HRFURLInternetImagingHTTP::HRFURLInternetImagingHTTP(const Utf8String& pi_User,
                                                     const Utf8String& pi_Password,
                                                     const Utf8String& pi_Host,
                                                     const Utf8String& pi_Port,
                                                     const Utf8String& pi_Path,
                                                     const Utf8String& pi_ImageName,
                                                     bool    pi_IsHTTP)
    : HFCURLHTTPBase(pi_User,
                     pi_Password,
                     pi_Host,
                     pi_Port,
                     pi_Path,
                     Utf8String("fif=") + pi_ImageName,
                     pi_IsHTTP)
    {
    m_Image = pi_ImageName;
    }

//-----------------------------------------------------------------------------
// This constructor configures the object from the detached parts of the URL
// specification.
//-----------------------------------------------------------------------------
HRFURLInternetImagingHTTP::HRFURLInternetImagingHTTP(const Utf8String& pi_rURL,
                                                     bool    pi_IsHTTP)
    : HFCURLHTTPBase(pi_rURL, pi_IsHTTP)
    {
    HPRECONDITION(IsURLInternetImaging(this));

    // extract the image name from the URL search path
    m_Image = GetSearchPart().substr(4);
    }

//-----------------------------------------------------------------------------
// The destructor for this class.
//-----------------------------------------------------------------------------
HRFURLInternetImagingHTTP::~HRFURLInternetImagingHTTP()
    {
    }

//-----------------------------------------------------------------------------
// Returns a reference to a string that contains the name of the image
//-----------------------------------------------------------------------------
bool HRFURLInternetImagingHTTP::GetUTF8EscapedImage(string* po_pUTF8EscapedImage)
    {
    HPRECONDITION(po_pUTF8EscapedImage != 0);

    bool         HasUT8EscapedImage;

    if (m_UTF8EscapedSearchPart.empty() == false)
        {
        // extract the image name from the URL search path
        *po_pUTF8EscapedImage = m_UTF8EscapedSearchPart.substr(4);
        HasUT8EscapedImage = true;
        }
    else
        {
        HasUT8EscapedImage = false;
        }

    return HasUT8EscapedImage;
    }

//-----------------------------------------------------------------------------
// Indicates if the given URL is a HTTP-IIP url
//-----------------------------------------------------------------------------
bool HRFURLInternetImagingHTTP::IsURLInternetImaging(const HFCURL* pi_pURL)
    {
    HPRECONDITION(pi_pURL != 0);
    bool Result = false;

    if ((pi_pURL->GetSchemeType() == HFCURLHTTP::s_SchemeName()) ||
        (pi_pURL->GetSchemeType() == HFCURLHTTPS::s_SchemeName()))
        {
        // get the first 4 bytes of the search part and lower case them
        Utf8String SearchPart(((const HFCURLHTTPBase*)pi_pURL)->GetSearchPart(), 0, 4);

        // verify that it starts with "fif="
        Result = SearchPart.EqualsI("fif=");
        }

    return (Result);
    }

//-----------------------------------------------------------------------------
// Returns a reference to a string that contains the name of the image
//-----------------------------------------------------------------------------
const Utf8String& HRFURLInternetImagingHTTP::GetImage() const
    {
    return m_Image;
    }

#ifdef __HMR_DEBUG_MEMBER
void HRFURLInternetImagingHTTP::PrintState() const
    {
    }
#endif
