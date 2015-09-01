//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLEmbedFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCURLStorageFile
//-----------------------------------------------------------------------------
// This class defines a specialized version of URL interface for the "ss:"
// scheme type.
//-----------------------------------------------------------------------------

#pragma once

#include "HFCURL.h"
#include "interface\IFileReaderHandler.h"

// URL specification at this level is:
// embed:{//Path}
BEGIN_IMAGEPP_NAMESPACE

class HFCURLEmbedFile : public HFCURL
    {
public:

    HDECLARE_CLASS_ID(HFCURLId_EmbedFile, HFCURL);
    // Define the Scheme label
    static const WString& s_SchemeName()
        {   static const WString Val(L"embed");
        FREEZE_STL_STRING(Val);
        return Val;
        }

    // Primary methods

    IMAGEPP_EXPORT                  HFCURLEmbedFile(const WString& pi_rURL, IFileReaderHandler* pi_pHandler);

    HFCURLEmbedFile() { } // required for persistence
    IMAGEPP_EXPORT virtual         ~HFCURLEmbedFile();

    // Content access methods

    IMAGEPP_EXPORT virtual WString  GetURL() const;
    IMAGEPP_EXPORT const WString&   GetPath() const;

    // Overriden methods, used in relative path management

    virtual bool            HasPathTo(HFCURL* pi_pURL);
    virtual WString          FindPathTo(HFCURL* pi_pDest);
    virtual HFCURL*          MakeURLTo(const WString& pi_Path);


    IMAGEPP_EXPORT  void             RegisterFileReaderHandler(HFCPtr<IFileReaderHandler> pi_pHandler);
    IMAGEPP_EXPORT  HFCPtr<IFileReaderHandler> GetFileReaderHandler() const;

    //:> HFCStat utilities methods
    void                        SetCreationTime(time_t   pi_NewTime);
    time_t                      GetCreationTime() const;

    void                        SetModificationTime(time_t   pi_NewTime);
    time_t                      GetModificationTime() const;

#ifdef __HMR_DEBUG_MEMBER
    virtual void PrintState() const;
#endif

protected:

private:

    friend struct URLEmbedFileCreator;

    // Disabled methods

    HFCURLEmbedFile(const HFCURLEmbedFile&);
    HFCURLEmbedFile& operator=(const HFCURLEmbedFile&);

    // Components of the scheme-specific part of the URL string.

    WString      m_Path;
    time_t       m_creationTime;
    time_t       m_modificationTime;

    HFCPtr<IFileReaderHandler>  m_pHandler;

    };

END_IMAGEPP_NAMESPACE
#include "HFCURLEmbedFile.hpp"

