//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCMemoryBinStream
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

#pragma once

#include "HFCBinStream.h"
#include "HFCBuffer.h"
#include "ImagePP/all/h/interface/IFileReaderHandler.h"

BEGIN_IMAGEPP_NAMESPACE

class HFCEmbedBinStream : public HFCBinStream
    {
public:
    HDECLARE_CLASS_ID (HFCBinStreamId_Embed, HFCBinStream);

    // Primary methods
    IMAGEPP_EXPORT                  HFCEmbedBinStream();

    IMAGEPP_EXPORT                  HFCEmbedBinStream(const Utf8String&      pi_Filename,
                                              IFileReaderHandler* pi_pHandler);

    IMAGEPP_EXPORT virtual          ~HFCEmbedBinStream();

    // Information methods

    HFCPtr<HFCURL>          GetURL() const override;
    uint64_t       GetSize() const override;
    HFCAccessMode   GetAccessMode() const override;
    uint64_t               GetOriginOffset() const;

    // File pointer management

    void            Seek(int64_t pi_Delta) override;
    void            SeekToPos(uint64_t pi_NewPos) override;
    void            SeekToBegin() override;
    void            SeekToEnd() override;
    uint64_t        GetCurrentPos() override;
    bool            EndOfFile() override;
    bool            SetEOF() override;

    // Content access
    size_t          Read(void* po_pData, size_t pi_DataSize) override;
    size_t          Write(const void* pi_pData, size_t pi_DataSize) override;
    bool           Flush() override;

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
    Utf8String             m_Filename;

    };

END_IMAGEPP_NAMESPACE
