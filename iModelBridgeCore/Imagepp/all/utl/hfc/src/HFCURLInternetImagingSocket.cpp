//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCURLInternetImagingSocket.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCURLInternetImagingSocket
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCURLInternetImagingSocket.h>

//:Ignore

//------------------------------------------------------------------------------
// This is the creator that registers itself in the scheme list.
//------------------------------------------------------------------------------
struct URLIIPSocketCreator : public HFCURL::Creator
    {
    URLIIPSocketCreator()
        {
        HFCURLInternetImagingSocket::GetSchemeList().
        insert(HFCURLInternetImagingSocket::SchemeList::value_type(HFCURLInternetImagingSocket::s_SchemeName(), this));
        }
    virtual HFCURL* Create(const WString& pi_URL) const
        {
        return new HFCURLInternetImagingSocket(pi_URL);
        }
    } g_URLIIPCreator;

//:End Ignore

/**----------------------------------------------------------------------------
 This constructor configures the object from the detached parts of the URL
 specification.

 Copy constructor and assignment operators are disabled for this class.

 @param pi_User       Constant reference to a string that contains the name
                      of the user, if a username has to be specified in this
                      URL, or an empty string.

 @param pi_Password   Constant reference to a string that contains the
                      password needed for the user specified in this URL,
                      if any, or an empty string.

 @param pi_Host       Constant reference to a string that contains the
                      specification of the host (IP address or domain name).

 @param pi_Port       Constant reference to a string that contains the port
                      number to use onto the host, if required for this URL,
                      or an empty string.

 @param pi_Image      Constant reference to a string that contains the path
                      to the image resource on the host.  Must not begin by
                      a slash or blackslash.  If no path, the string must be
                      empty.

 @inheritance This class is an instanciable one that correspond to one
              precise kind of URL and no child of it are expected to be
              defined.
-----------------------------------------------------------------------------*/
HFCURLInternetImagingSocket::HFCURLInternetImagingSocket(const WString& pi_rUser,
                                                         const WString& pi_rPassword,
                                                         const WString& pi_rHost,
                                                         const WString& pi_rPort,
                                                         const WString& pi_rImage)
    : HFCURLCommonInternet(s_SchemeName(), pi_rUser, pi_rPassword, pi_rHost, pi_rPort, pi_rImage)
    {
    }


/**----------------------------------------------------------------------------
 This constructor configure the object from the complete URL specification.

 Syntax: @c{iip://[user[:password]@]host:port/imagename}

 @param pi_rURL Constant reference to a string that contains a complete URL
                specification.

 @inheritance This class is an instanciable one that correspond to one
              precise kind of URL and no child of it are expected to be
              defined.
-----------------------------------------------------------------------------*/
HFCURLInternetImagingSocket::HFCURLInternetImagingSocket(const WString& pi_pURL)
    : HFCURLCommonInternet(pi_pURL)
    {
    }


/**----------------------------------------------------------------------------
 The destructor for this class.
-----------------------------------------------------------------------------*/
HFCURLInternetImagingSocket::~HFCURLInternetImagingSocket()
    {
    // Nothing to do here.
    }


/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 @see HFCURL
-----------------------------------------------------------------------------*/
WString HFCURLInternetImagingSocket::GetURL() const
    {
    WString Result(s_SchemeName() + L"://");
    if (!GetUser().empty())
        {
        Result += GetUser();
        if (!GetPassword().empty())
            {
            Result += L":";
            Result += GetPassword();
            }
        Result += L"@";
        }
    Result += GetHost();
    if (!GetPort().empty())
        {
        Result += L":";
        Result += GetPort();
        }
    Result += L"/";
    Result += GetImage();
    return Result;
    }


/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 @see HFCURL
-----------------------------------------------------------------------------*/
bool HFCURLInternetImagingSocket::HasPathTo(HFCURL* pi_pURL)
    {
    return HFCURLCommonInternet::HasPathTo(pi_pURL);
    }


/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 @see HFCURL
-----------------------------------------------------------------------------*/
WString HFCURLInternetImagingSocket::FindPathTo(HFCURL* pi_pDest)
    {
    HPRECONDITION(pi_pDest != 0);
    HPRECONDITION(HasPathTo(pi_pDest));

    HASSERT(pi_pDest->GetSchemeType() == s_SchemeName());
    return FindPath(GetImage(),
                    static_cast<HFCURLInternetImagingSocket*>(pi_pDest)->GetImage());
    }


/**----------------------------------------------------------------------------
 Overriden from HFCURL.

 @see HFCURL
-----------------------------------------------------------------------------*/
HFCURL* HFCURLInternetImagingSocket::MakeURLTo(const WString& pi_rPath)
    {
    return new HFCURLInternetImagingSocket(GetUser(),
                                           GetPassword(),
                                           GetHost(),
                                           GetPort(),
                                           AddPath(GetImage(),
                                                   pi_rPath));
    }


#ifdef __HMR_DEBUG_MEMBER
void HFCURLInternetImagingSocket::PrintState() const
    {
    }
#endif
