//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLFile.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCURLFile
//-----------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HFCURL.h>

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
    static const Utf8String& s_SchemeName()
        {   static const Utf8String Val("file");
        return Val;
        }

    //:> Primary methods

    IMAGEPP_EXPORT                         HFCURLFile(const Utf8String& pi_URL);
    IMAGEPP_EXPORT                         HFCURLFile(const Utf8String& pi_Host,
                                              const Utf8String& pi_Path);
    IMAGEPP_EXPORT virtual                 ~HFCURLFile();

    //:> Content access methods

    IMAGEPP_EXPORT Utf8String          GetURL() const override;
    IMAGEPP_EXPORT const Utf8String&           GetHost() const;
    IMAGEPP_EXPORT const Utf8String&           GetPath() const;
    IMAGEPP_EXPORT Utf8String                  GetFilename() const;
    IMAGEPP_EXPORT Utf8String                  GetAbsoluteFileName() const;
    IMAGEPP_EXPORT Utf8String                  GetExtension() const;

    //:> Overriden methods, used in relative path management

    bool            HasPathTo(HFCURL* pi_pURL) override;
    Utf8String         FindPathTo(HFCURL* pi_pDest) override;
    HFCURL*         MakeURLTo(const Utf8String& pi_Path) override;

    //:> Utility methods

    IMAGEPP_EXPORT bool             CreatePath();
    IMAGEPP_EXPORT void             SetFileName(const Utf8String& pi_rFileName);


#ifdef __HMR_DEBUG_MEMBER
    virtual void PrintState() const;
#endif

protected:

private:

    friend struct URLFileCreator;

    //:> Components of the scheme-specific part of the URL string.

    Utf8String                  m_Host;
    Utf8String                  m_Path;

    //:> Disabled methods

    HFCURLFile(const HFCURLFile&);
    HFCURLFile& operator=(const HFCURLFile&);

    };

END_IMAGEPP_NAMESPACE
#include "HFCURLFile.hpp"
