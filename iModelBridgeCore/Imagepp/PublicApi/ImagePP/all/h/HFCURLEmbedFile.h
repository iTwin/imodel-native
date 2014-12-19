//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLEmbedFile.h $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
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


class HFCURLEmbedFile : public HFCURL
    {
public:

    HDECLARE_CLASS_ID(1312, HFCURL);
    // Define the Scheme label
    static const WString& s_SchemeName()
        {   static const WString Val(L"embed");
        FREEZE_STL_STRING(Val);
        return Val;
        }

    // Primary methods

    _HDLLu                  HFCURLEmbedFile(const WString& pi_rURL, IFileReaderHandler* pi_pHandler);

    HFCURLEmbedFile() { } // required for persistence
    _HDLLu virtual         ~HFCURLEmbedFile();

    // Content access methods

    _HDLLu virtual WString  GetURL() const;
    _HDLLu const WString&   GetPath() const;

    // Overriden methods, used in relative path management

    virtual bool            HasPathTo(HFCURL* pi_pURL);
    virtual WString          FindPathTo(HFCURL* pi_pDest);
    virtual HFCURL*          MakeURLTo(const WString& pi_Path);


    _HDLLu  void             RegisterFileReaderHandler(HFCPtr<IFileReaderHandler> pi_pHandler);
    _HDLLu  HFCPtr<IFileReaderHandler> GetFileReaderHandler() const;

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


#include "HFCURLEmbedFile.hpp"

