//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMemoryBinStream.h $
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

BEGIN_IMAGEPP_NAMESPACE

class HFCMemoryBinStream : public HFCBinStream
    {
public:
    HDECLARE_CLASS_ID (HFCBinStreamId_Memory, HFCBinStream);

    // Primary methods
    IMAGEPP_EXPORT                  HFCMemoryBinStream();

    IMAGEPP_EXPORT                  HFCMemoryBinStream(const WString&       pi_Filename,
                                               HFCAccessMode        pi_AccessMode,
                                               bool                 pi_AutoRemove = false,
                                               uint64_t             pi_OriginOffset = 0,
                                               short pi_NbRetryBeforeThrow = 0,
                                               const HFCPtr<HFCBuffer>&   pi_rpBuffer = HFCPtr<HFCBuffer>());

    IMAGEPP_EXPORT                  HFCMemoryBinStream(const WString&       pi_Filename,
                                               HFCAccessMode        pi_AccessMode,
                                               bool                 pi_CreateFile,
                                               bool                 pi_AutoRemove,
                                               uint64_t             pi_OriginOffset,
                                               short pi_NbRetryBeforeThrow = 0,
                                               const HFCPtr<HFCBuffer>&   pi_rpBuffer = HFCPtr<HFCBuffer>());


    IMAGEPP_EXPORT                  HFCMemoryBinStream(const WString&       pi_Filename,
                                               bool                 pi_ShareWrite = false,
                                               bool                 pi_ShareRead = true,
                                               bool                 pi_CreateFile = true,
                                               bool                 pi_AutoRemove = false,
                                               uint64_t             pi_OriginOffset = 0,
                                               short pi_NbRetryBeforeThrow = 0,
                                               const HFCPtr<HFCBuffer>&   pi_rpBuffer = HFCPtr<HFCBuffer>());

    IMAGEPP_EXPORT virtual          ~HFCMemoryBinStream();

    // Information methods

    virtual HFCPtr<HFCURL>  GetURL() const override;
    virtual uint64_t        GetSize() const override;
    virtual HFCAccessMode   GetAccessMode() const override;
    uint64_t                GetOriginOffset() const;

    // Multiuser access

    virtual void            Lock(uint64_t pi_Pos, uint64_t pi_Size, bool pi_Share) override;
    virtual void            Unlock(uint64_t pi_Pos, uint64_t pi_Size) override;

    // File pointer management

    virtual void            Seek(int64_t pi_Delta) override;
    virtual void            SeekToPos(uint64_t pi_NewPos) override;
    virtual void            SeekToBegin() override;
    virtual void            SeekToEnd() override;
    virtual uint64_t        GetCurrentPos() override;
    virtual bool            EndOfFile() override;
    virtual bool            SetEOF() override;

    // Content access
    virtual size_t          Read(void* po_pData, size_t pi_DataSize) override;
    virtual size_t          Write(const void* pi_pData, size_t pi_DataSize) override;
    virtual bool            Flush() override;

    HFCPtr<HFCBuffer >      GetBuffer() const;

#ifdef __HMR_DEBUG_MEMBER
    void DumpToFile() const;
#endif


private:
    // Disabled methods
    HFCMemoryBinStream(const HFCMemoryBinStream& pi_rObj);
    HFCMemoryBinStream& operator=(const HFCMemoryBinStream& pi_rObj);

    friend struct MemoryBinStreamCreator;

    // Data members
    bool                   m_AutoRemove;
    bool                   m_LastSeekStatus;

    HFCPtr<HFCBuffer >     m_BinStreamBuffer;
    size_t                 m_CurrentOffset;

    WString                m_Filename;

    HFCAccessMode          m_AccessMode;
    uint64_t               m_OriginOffset;
    };

END_IMAGEPP_NAMESPACE

#include "HFCMemoryBinStream.hpp"


