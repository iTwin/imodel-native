//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
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
#include "interface/IFileReaderHandler.h"

// URL specification at this level is:
// embed:{//Path}
BEGIN_IMAGEPP_NAMESPACE

class HFCURLEmbedFile : public HFCURL
    {
public:

    HDECLARE_CLASS_ID(HFCURLId_EmbedFile, HFCURL);
    // Define the Scheme label
    static const Utf8String& s_SchemeName()
        {   static const Utf8String Val("embed");
        return Val;
        }

    // Primary methods

    IMAGEPP_EXPORT                  HFCURLEmbedFile(const Utf8String& pi_rURL, IFileReaderHandler* pi_pHandler);

    IMAGEPP_EXPORT virtual         ~HFCURLEmbedFile();

    // Content access methods

    IMAGEPP_EXPORT Utf8String  GetURL() const override;
    IMAGEPP_EXPORT const Utf8String&   GetPath() const;

    // Overriden methods, used in relative path management

    bool            HasPathTo(HFCURL* pi_pURL) override;
    Utf8String          FindPathTo(HFCURL* pi_pDest) override;
    HFCURL*          MakeURLTo(const Utf8String& pi_Path) override;


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

    Utf8String      m_Path;
    time_t       m_creationTime;
    time_t       m_modificationTime;

    HFCPtr<IFileReaderHandler>  m_pHandler;

    };

END_IMAGEPP_NAMESPACE
#include "HFCURLEmbedFile.hpp"

