//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMemoryBinStream.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCMemoryBinStream
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

#pragma once

#include "HFCBinStream.h"
#include "HFCBuffer.h"

class HFCMemoryBinStream : public HFCBinStream
    {
public:
    HDECLARE_CLASS_ID (1293, HFCBinStream);

    // Primary methods
    _HDLLu                  HFCMemoryBinStream();

    _HDLLu                  HFCMemoryBinStream(const WString&       pi_Filename,
                                               HFCAccessMode        pi_AccessMode,
                                               bool                 pi_AutoRemove = false,
                                               uint64_t             pi_OriginOffset = 0,
                                               short pi_NbRetryBeforeThrow = 0,
                                               const HFCPtr<HFCBuffer>&   pi_rpBuffer = HFCPtr<HFCBuffer>());

    _HDLLu                  HFCMemoryBinStream(const WString&       pi_Filename,
                                               HFCAccessMode        pi_AccessMode,
                                               bool                 pi_CreateFile,
                                               bool                 pi_AutoRemove,
                                               uint64_t             pi_OriginOffset,
                                               short pi_NbRetryBeforeThrow = 0,
                                               const HFCPtr<HFCBuffer>&   pi_rpBuffer = HFCPtr<HFCBuffer>());


    _HDLLu                  HFCMemoryBinStream(const WString&       pi_Filename,
                                               bool                 pi_ShareWrite = false,
                                               bool                 pi_ShareRead = true,
                                               bool                 pi_CreateFile = true,
                                               bool                 pi_AutoRemove = false,
                                               uint64_t             pi_OriginOffset = 0,
                                               short pi_NbRetryBeforeThrow = 0,
                                               const HFCPtr<HFCBuffer>&   pi_rpBuffer = HFCPtr<HFCBuffer>());

    _HDLLu virtual          ~HFCMemoryBinStream();

    // Information methods

    HFCPtr<HFCURL>          GetURL() const;
    virtual uint64_t        GetSize() const;
    virtual HFCAccessMode   GetAccessMode() const;
    uint64_t                GetOriginOffset() const;

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
    virtual size_t          Read(WChar* po_pData, size_t pi_DataSize);
    virtual size_t          Write(const WChar* pi_pData, size_t pi_DataSize);
    virtual bool            Flush();

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

#include "HFCMemoryBinStream.hpp"


