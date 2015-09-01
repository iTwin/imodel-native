//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCURLFile
//-----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HFCURL.h>

BEGIN_IMAGEPP_NAMESPACE
//:> URL specification at this level is:
//:> file:{//host/[path] | [//]drive/[path] | [//]relpath}

/**

This class defines the URL descriptor for the "file" scheme type.  That
kind of URL is used to locate files that are accessible through local
access or LAN access, without the need of an internet protocol.  The
structure of the text of the URLs follows one of the following syntaxes:

@code
@i{file}://@i{host}/[@i{path}]
@i{file}:[//]@i{drive}/[@i{path}]
@end

where the text in italics has to be replaced by corresponding value, and
text between brackets is facultative.  The drive specification includes
the colon, and is different of the host specification as the host
identifies a target by its network name.

This class adds methods to access the parsed values extracted from the
URL text (the host, drive and path) and to convert the URL into a
standard filename.

@see HFCURL

*/

class HFCURLFile : public HFCURL
    {
public:

    HDECLARE_CLASS_ID(HFCURLId_File, HFCURL);

    // Define the Scheme label
    static const WString& s_SchemeName()
        {   static const WString Val(L"file");
        FREEZE_STL_STRING(Val);
        return Val;
        }

    //:> Primary methods

    IMAGEPP_EXPORT                         HFCURLFile(const WString& pi_URL);
    IMAGEPP_EXPORT                         HFCURLFile(const WString& pi_Host,
                                              const WString& pi_Path);
    HFCURLFile() { } //:> required for persistence
    IMAGEPP_EXPORT virtual                 ~HFCURLFile();

    //:> Content access methods

    IMAGEPP_EXPORT virtual WString          GetURL() const;
    IMAGEPP_EXPORT const WString&           GetHost() const;
    IMAGEPP_EXPORT const WString&           GetPath() const;
    IMAGEPP_EXPORT WString                  GetFilename() const;
    IMAGEPP_EXPORT WString                  GetAbsoluteFileName() const;
    IMAGEPP_EXPORT WString                  GetExtension() const;

    //:> Overriden methods, used in relative path management

    virtual bool            HasPathTo(HFCURL* pi_pURL);
    virtual WString         FindPathTo(HFCURL* pi_pDest);
    virtual HFCURL*         MakeURLTo(const WString& pi_Path);

    //:> Utility methods

    IMAGEPP_EXPORT bool             CreatePath();
    IMAGEPP_EXPORT void             SetFileName(const WString& pi_rFileName);


#ifdef __HMR_DEBUG_MEMBER
    virtual void PrintState() const;
#endif

protected:

private:

    friend struct URLFileCreator;

    //:> Components of the scheme-specific part of the URL string.

    WString                  m_Host;
    WString                  m_Path;

    //:> Disabled methods

    HFCURLFile(const HFCURLFile&);
    HFCURLFile& operator=(const HFCURLFile&);

    };

END_IMAGEPP_NAMESPACE
#include "HFCURLFile.hpp"
