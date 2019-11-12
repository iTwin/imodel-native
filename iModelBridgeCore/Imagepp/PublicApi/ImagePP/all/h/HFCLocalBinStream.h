//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCLocalBinStream
//---------------------------------------------------------------------------
#pragma once

#include "HFCBinStream.h"

BEGIN_IMAGEPP_NAMESPACE

namespace Bentley {struct BeFile;}

class HFCLocalBinStream : public HFCBinStream
    {

    friend struct LocalBinStreamCreator;

public:

    HDECLARE_CLASS_ID (HFCBinStreamId_Local, HFCBinStream);

    // Primary methods

    HFCLocalBinStream();
    IMAGEPP_EXPORT                         HFCLocalBinStream(const Utf8String&    pi_Filename,
                                                     HFCAccessMode     pi_AccessMode,
                                                     bool              pi_AutoRemove = false,
                                                     uint64_t          pi_OriginOffset = 0,
                                                     int16_t pi_NbRetry = 0);

    IMAGEPP_EXPORT                         HFCLocalBinStream(const Utf8String&    pi_Filename,
                                                     HFCAccessMode     pi_AccessMode,
                                                     bool              pi_CreateFile,
                                                     bool              pi_AutoRemove,
                                                     uint64_t          pi_OriginOffset,
                                                     int16_t pi_NbRetry=0);

    IMAGEPP_EXPORT                         HFCLocalBinStream(const Utf8String&    pi_Filename,
                                                     bool              pi_ShareWrite = false,
                                                     bool              pi_ShareRead = true,
                                                     bool              pi_CreateFile = true,
                                                     bool              pi_AutoRemove = false,
                                                     uint64_t          pi_OriginOffset = 0,
                                                     int16_t pi_NbRetry = 0);
    IMAGEPP_EXPORT virtual                 ~HFCLocalBinStream();

    // Information methods

    IMAGEPP_EXPORT HFCPtr<HFCURL>  GetURL() const override;
    IMAGEPP_EXPORT uint64_t        GetSize() const override;
    HFCAccessMode          GetAccessMode() const override;
    uint64_t                       GetOriginOffset() const;
    IMAGEPP_EXPORT uint64_t                GetCurrentFileSize() const;


    // File pointer management

    void            Seek(int64_t pi_Delta) override;
    void            SeekToPos(uint64_t pi_NewPos) override;
    void            SeekToBegin() override;
    void            SeekToEnd() override;
    uint64_t        GetCurrentPos() override;
    bool            EndOfFile() override;
    bool            SetEOF() override;

    // Content access

    IMAGEPP_EXPORT size_t    Read(void* po_pData, size_t pi_DataSize) override;
    IMAGEPP_EXPORT size_t    Write(const void* pi_pData, size_t pi_DataSize) override;


    bool     Flush() override;

    enum MaxOffsetBitsSupported
        {
        OffsetIs32Bits,
        OffsetIs64Bits
        };
    IMAGEPP_EXPORT void                  SetMaxFileSizeSupported(MaxOffsetBitsSupported pi_OffsetBits);


protected:

    IMAGEPP_EXPORT void                    Open(const Utf8String&     pi_Filename,
                                        HFCAccessMode      pi_AccessMode,
                                        bool              pi_ShareWrite,
                                        bool              pi_ShareRead,
                                        bool              pi_CreateFile,
                                        uint64_t          pi_OriginOffset,
                                        bool              pi_IgnoreMode,
                                        bool              pi_AutoRemove,
                                        int16_t pi_NbRetry=0);

private:

    IMAGEPP_EXPORT void            SetLastExceptionClassID();
    void FileExceptionFromBeFileStatus(BeFileStatus pi_Status);

    WString                CookFilenameWithLongNameTagW() const;


    // Disabled methods

    // See HFCBinStream.h as to why they are disabled.
    size_t Read(WChar* po_pData, size_t pi_DataSize);
    size_t Write(const WChar* pi_pData, size_t pi_DataSize);

    HFCLocalBinStream(const HFCLocalBinStream& pi_rObj);
    HFCLocalBinStream& operator=(const HFCLocalBinStream& pi_rObj);

    // Data members

    BeFile                 m_BeFile;

    BeFileName             m_Filename;
    bool                   m_AutoRemove;
    HFCAccessMode          m_AccessMode;
    uint64_t               m_OriginOffset;
    bool                   m_LastSeekStatus;

    uint64_t               m_MaxOffsetAcceptable;  // see method SetMaxFileSizeSupported for more details
    uint64_t               m_CurrentFileSize;
    bool                   m_WeAreWritingAtTheEnd;
    bool                   m_HasToBeFlushed;
    };

END_IMAGEPP_NAMESPACE
