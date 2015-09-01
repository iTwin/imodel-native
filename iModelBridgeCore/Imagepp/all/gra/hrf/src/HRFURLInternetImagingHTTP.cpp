/*--------------------------------------------------------------------------------------+
|
|     $Source: all/gra/hrf/src/HRFURLInternetImagingHTTP.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRFURLInternetImagingHTTP
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCURLHttp.h>
#include <Imagepp/all/h/HFCURLHttps.h>
#include <Imagepp/all/h/HRFURLInternetImagingHTTP.h>

//-----------------------------------------------------------------------------
// This constructor configures the object from the detached parts of the URL
// specification.
//-----------------------------------------------------------------------------
HRFURLInternetImagingHTTP::HRFURLInternetImagingHTTP(const WString& pi_User,
                                                     const WString& pi_Password,
                                                     const WString& pi_Host,
                                                     const WString& pi_Port,
                                                     const WString& pi_Path,
                                                     const WString& pi_ImageName,
                                                     bool    pi_IsHTTP)
    : HFCURLHTTPBase(pi_User,
                     pi_Password,
                     pi_Host,
                     pi_Port,
                     pi_Path,
                     WString(L"fif=") + pi_ImageName,
                     pi_IsHTTP)
    {
    m_Image = pi_ImageName;
    }

//-----------------------------------------------------------------------------
// This constructor configures the object from the detached parts of the URL
// specification.
//-----------------------------------------------------------------------------
HRFURLInternetImagingHTTP::HRFURLInternetImagingHTTP(const WString& pi_rURL,
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
        WString SearchPart(((const HFCURLHTTPBase*)pi_pURL)->GetSearchPart(), 0, 4);

        // verify that it starts with "fif="
        Result = CaseInsensitiveStringTools().AreEqual(SearchPart, L"fif=");
        }

    return (Result);
    }

//-----------------------------------------------------------------------------
// Returns a reference to a string that contains the name of the image
//-----------------------------------------------------------------------------
const WString& HRFURLInternetImagingHTTP::GetImage() const
    {
    return m_Image;
    }

#ifdef __HMR_DEBUG_MEMBER
void HRFURLInternetImagingHTTP::PrintState() const
    {
    }
#endif
