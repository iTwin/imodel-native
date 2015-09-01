//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLMemFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCURLMemFile
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCURL.h>

BEGIN_IMAGEPP_NAMESPACE
//:> URL specification at this level is:
//:> memory:{//Filename[.extension]}

/**

This class defines the URL descriptor for the "memory" scheme type.  That
kind of URL is used to simulate files through memory buffer wich are accessible
on local access. The structure of the text of the URLs follows one of the following syntaxes:

@code
@i{memory}://@iFilename[.extension]
@end

where the text in italics has to be replaced by corresponding value, and
text between brackets is facultative.

This class adds methods to access the parsed values extracted from the
URL text (the host and path) and to convert the memory buffer into a
standard filename like.

@see HFCURL

*/

class HFCBuffer;

class HFCURLMemFile : public HFCURL
    {
public:

    HDECLARE_CLASS_ID(HFCURLId_MemFile, HFCURL);

    // Define the Scheme label
    static const WString& s_SchemeName()
        {   static const WString Val(L"memory");
        FREEZE_STL_STRING(Val);
        return Val;
        }

    //:> Primary methods

    IMAGEPP_EXPORT                      HFCURLMemFile(WStringCR pi_URL, const HFCPtr<HFCBuffer>& pi_rpBuffer = HFCPtr<HFCBuffer>());
    IMAGEPP_EXPORT virtual              ~HFCURLMemFile();

    //:> Content access methods
    virtual WString GetURL() const override;
    
    IMAGEPP_EXPORT WStringCR GetFilename() const;
    
    IMAGEPP_EXPORT WString GetExtension() const;

    //:> Overriden methods, used in relative path management

    virtual bool                HasPathTo(HFCURL* pi_pURL) override;
    virtual WString             FindPathTo(HFCURL* pi_pDest) override;
    virtual HFCURL*             MakeURLTo(const WString& pi_Path) override;

    IMAGEPP_EXPORT HFCPtr<HFCBuffer>& GetBuffer();
    IMAGEPP_EXPORT void               SetBuffer(HFCPtr<HFCBuffer>& pi_rpBuffer);

    //:> HFCStat utilities methods
    IMAGEPP_EXPORT void     SetCreationTime(time_t   pi_NewTime);
    IMAGEPP_EXPORT time_t   GetCreationTime() const;

    IMAGEPP_EXPORT void     SetModificationTime(time_t   pi_NewTime);
    IMAGEPP_EXPORT time_t   GetModificationTime() const;


#ifdef __HMR_DEBUG_MEMBER
    virtual void PrintState() const;
#endif

protected:

private:
    friend struct URLMemoryCreator;

    //:> Disabled methods
    HFCURLMemFile(const HFCURLMemFile&);
    HFCURLMemFile& operator=(const HFCURLMemFile&);


    //:> Components of the scheme-specific part of the URL string.
    WString                 m_Filename;
    HFCPtr<HFCBuffer>       m_pBuffer;
    time_t                  m_creationTime;
    time_t                  m_modificationTime;
    };

END_IMAGEPP_NAMESPACE