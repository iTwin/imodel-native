//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCEmbedBinStream.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCMemoryBinStream
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

#pragma once

#include "HFCBinStream.h"
#include "HFCBuffer.h"
#include "Imagepp/all/h/interface/IFileReaderHandler.h"

BEGIN_IMAGEPP_NAMESPACE

class HFCEmbedBinStream : public HFCBinStream
    {
public:
    HDECLARE_CLASS_ID (HFCBinStreamId_Embed, HFCBinStream);

    // Primary methods
    IMAGEPP_EXPORT                  HFCEmbedBinStream();

    IMAGEPP_EXPORT                  HFCEmbedBinStream(const WString&      pi_Filename,
                                              IFileReaderHandler* pi_pHandler);

    IMAGEPP_EXPORT virtual          ~HFCEmbedBinStream();

    // Information methods

    HFCPtr<HFCURL>          GetURL() const;
    virtual uint64_t       GetSize() const;
    virtual HFCAccessMode   GetAccessMode() const;
    uint64_t               GetOriginOffset() const;

    // Multiuser access

    virtual void            Lock(uint64_t pi_Pos, uint64_t pi_Size, bool pi_Share);
    virtual void            Unlock(uint64_t pi_Pos, uint64_t pi_Size);

    // File pointer management

    virtual void            Seek(int64_t pi_Delta);
    virtual void            SeekToPos(uint64_t pi_NewPos);
    virtual void            SeekToBegin();
    virtual void            SeekToEnd();
    virtual uint64_t        GetCurrentPos();
    virtual bool            EndOfFile();
    virtual bool            SetEOF();

    // Content access
    virtual size_t          Read(void* po_pData, size_t pi_DataSize);
    virtual size_t          Write(const void* pi_pData, size_t pi_DataSize);
    virtual bool           Flush();

#ifdef __HMR_DEBUG_MEMBER
    void DumpToFile() const;
#endif


private:
    // Disabled methods
    HFCEmbedBinStream(const HFCEmbedBinStream& pi_rObj);
    HFCEmbedBinStream& operator=(const HFCEmbedBinStream& pi_rObj);

    friend struct EmbedBinStreamCreator;

    // Data members
    uint64_t           m_CurrentPos;
    IFileReaderHandler* m_pHandler;
    HFCAccessMode       m_AccessMode;
    WString             m_Filename;

    };

END_IMAGEPP_NAMESPACE