//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCLocalBinStream.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    IMAGEPP_EXPORT                         HFCLocalBinStream(const WString&    pi_Filename,
                                                     HFCAccessMode     pi_AccessMode,
                                                     bool              pi_AutoRemove = false,
                                                     uint64_t          pi_OriginOffset = 0,
                                                     short pi_NbRetry = 0);

    IMAGEPP_EXPORT                         HFCLocalBinStream(const WString&    pi_Filename,
                                                     HFCAccessMode     pi_AccessMode,
                                                     bool              pi_CreateFile,
                                                     bool              pi_AutoRemove,
                                                     uint64_t          pi_OriginOffset,
                                                     short pi_NbRetry=0);

    IMAGEPP_EXPORT                         HFCLocalBinStream(const WString&    pi_Filename,
                                                     bool              pi_ShareWrite = false,
                                                     bool              pi_ShareRead = true,
                                                     bool              pi_CreateFile = true,
                                                     bool              pi_AutoRemove = false,
                                                     uint64_t          pi_OriginOffset = 0,
                                                     short pi_NbRetry = 0);
    IMAGEPP_EXPORT virtual                 ~HFCLocalBinStream();

    // Information methods

    IMAGEPP_EXPORT virtual HFCPtr<HFCURL>  GetURL() const;
    IMAGEPP_EXPORT virtual uint64_t        GetSize() const;
    virtual HFCAccessMode          GetAccessMode() const;
    uint64_t                       GetOriginOffset() const;
    IMAGEPP_EXPORT uint64_t                GetCurrentFileSize() const;

    // Multiuser access

    IMAGEPP_EXPORT virtual void            Lock(uint64_t pi_Pos, uint64_t pi_Size, bool pi_Share);  
    IMAGEPP_EXPORT virtual void            Unlock(uint64_t pi_Pos, uint64_t pi_Size);               

    // File pointer management

    virtual void            Seek(int64_t pi_Delta);
    virtual void            SeekToPos(uint64_t pi_NewPos);
    virtual void            SeekToBegin();
    virtual void            SeekToEnd();
    virtual uint64_t        GetCurrentPos();
    virtual bool            EndOfFile();
    virtual bool            SetEOF();

    // Content access

    IMAGEPP_EXPORT virtual size_t    Read(void* po_pData, size_t pi_DataSize);
    IMAGEPP_EXPORT virtual size_t    Write(const void* pi_pData, size_t pi_DataSize);


    virtual bool     Flush();

    enum MaxOffsetBitsSupported
        {
        OffsetIs32Bits,
        OffsetIs64Bits
        };
    IMAGEPP_EXPORT void                  SetMaxFileSizeSupported(MaxOffsetBitsSupported pi_OffsetBits);


protected:

    IMAGEPP_EXPORT void                    Open(const WString&     pi_Filename,
                                        HFCAccessMode      pi_AccessMode,
                                        bool              pi_ShareWrite,
                                        bool              pi_ShareRead,
                                        bool              pi_CreateFile,
                                        uint64_t          pi_OriginOffset,
                                        bool              pi_IgnoreMode,
                                        bool              pi_AutoRemove,
                                        short pi_NbRetry=0);

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

    WString                m_Filename;
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