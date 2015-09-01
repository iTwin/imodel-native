//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLInternetImagingSocket.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCURLInternetImagingSocket.h
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURLCommonInternet.h"

BEGIN_IMAGEPP_NAMESPACE
//:Ignore
// URL specification at this level is:
// iip[//][user[:password]@]host[:port][/[SearchPart]]
//:End Ignore

/**

    This class defines the URL descriptor for the "iip" scheme type.  That
    kind of URL is used to locate files that are accessible through IIP
    server (image servers).  The structure of the text of the URLs follows
    the syntax of the common internet scheme types.  The resulting syntax is
    as follows:

    @code
    iip://[@i{user}[:@i{password}]@]@i{host}[:@i{port}]/@i{imagepath}
    @end

    where the text in italics has to be replaced by corresponding value, and
    text between brackets is facultative.

    This class adds, to inherited interface, a method to access the image
    path that is the same of inherited method @k{GetURLPath}.

    @see HFCURLCommonInternet
    @see HFCURL

*/

class HFCURLInternetImagingSocket : public HFCURLCommonInternet
    {
public:

    HDECLARE_CLASS_ID(HFCURLId_InternetImagingSocket, HFCURLCommonInternet);

    // Define the Scheme label

    static const WString& s_SchemeName()
        {   static const WString Val(L"iip");
        FREEZE_STL_STRING(Val);
        return Val;
        }

    //:> Primary methods

    IMAGEPP_EXPORT                HFCURLInternetImagingSocket(const WString& pi_rURL);
    IMAGEPP_EXPORT                HFCURLInternetImagingSocket(const WString& pi_rUser,
                                                      const WString& pi_rPassword,
                                                      const WString& pi_rHost,
                                                      const WString& pi_rPort,
                                                      const WString& pi_rImage);
    IMAGEPP_EXPORT                 HFCURLInternetImagingSocket() { } //:> required for persistence
    IMAGEPP_EXPORT virtual         ~HFCURLInternetImagingSocket();

    //:> Content access methods

    virtual WString  GetURL() const;
    const WString&   GetImage () const {return GetURLPath();}

    //:>  Overriden methods, used in relative path management

    virtual bool   HasPathTo(HFCURL* pi_pURL);
    virtual WString FindPathTo(HFCURL* pi_pDest);
    virtual HFCURL* MakeURLTo(const WString& pi_rPath);

#ifdef __HMR_DEBUG_MEMBER
    virtual void    PrintState() const;
#endif

protected:
private:

    // Friends

    friend struct URLIIPSocketCreator;

    // Disabled methods

    HFCURLInternetImagingSocket(const HFCURLInternetImagingSocket&);
    HFCURLInternetImagingSocket& operator=(const HFCURLInternetImagingSocket&);
    };

END_IMAGEPP_NAMESPACE


