//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCLocalBinStream.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCLocalBinStream
//---------------------------------------------------------------------------
#pragma once

#include "HFCBinStream.h"

BEGIN_BENTLEY_NAMESPACE
struct BeFile;
END_BENTLEY_NAMESPACE

class HFCLocalBinStream : public HFCBinStream
    {
public:

    HDECLARE_CLASS_ID (1291, HFCBinStream);

    // Primary methods

    HFCLocalBinStream();
    _HDLLu                         HFCLocalBinStream(const WString&    pi_Filename,
                                                     HFCAccessMode     pi_AccessMode,
                                                     bool              pi_AutoRemove = false,
                                                     uint64_t          pi_OriginOffset = 0,
                                                     short pi_NbRetry = 0);

    _HDLLu                         HFCLocalBinStream(const WString&    pi_Filename,
                                                     HFCAccessMode     pi_AccessMode,
                                                     bool              pi_CreateFile,
                                                     bool              pi_AutoRemove,
                                                     uint64_t          pi_OriginOffset,
                                                     short pi_NbRetry=0);

    _HDLLu                         HFCLocalBinStream(const WString&    pi_Filename,
                                                     bool              pi_ShareWrite = false,
                                                     bool              pi_ShareRead = true,
                                                     bool              pi_CreateFile = true,
                                                     bool              pi_AutoRemove = false,
                                                     uint64_t          pi_OriginOffset = 0,
                                                     short pi_NbRetry = 0);
    _HDLLu virtual                 ~HFCLocalBinStream();

    // Information methods

    _HDLLu virtual HFCPtr<HFCURL>  GetURL() const;
    _HDLLu virtual uint64_t        GetSize() const;
    virtual HFCAccessMode          GetAccessMode() const;
    uint64_t                       GetOriginOffset() const;
    _HDLLu uint64_t                GetCurrentFileSize() const;

    // Multiuser access

    _HDLLu virtual void            Lock(uint64_t pi_Pos, uint64_t pi_Size, bool pi_Share);  //DM-Android
    _HDLLu virtual void            Unlock(uint64_t pi_Pos, uint64_t pi_Size);               //DM-Android

    // File pointer management

    virtual void            Seek(int64_t pi_Delta);
    virtual void            SeekToPos(uint64_t pi_NewPos);
    virtual void            SeekToBegin();
    virtual void            SeekToEnd();
    virtual uint64_t        GetCurrentPos();
    virtual bool            EndOfFile();
    virtual bool            SetEOF();

    // Content access

    _HDLLu virtual size_t    Read(void* po_pData, size_t pi_DataSize);
    _HDLLu virtual size_t    Write(const void* pi_pData, size_t pi_DataSize);

    // Unicode support, these method will read UTF-8 to UTF-16 and
    // write from UTF-16 to UTF-8 in the file.
    _HDLLu virtual size_t    Read(WChar* po_pData, size_t pi_DataSize);
    _HDLLu virtual size_t    Write(const WChar* pi_pData, size_t pi_DataSize);

    virtual bool     Flush();

    enum MaxOffsetBitsSupported
        {
        OffsetIs32Bits,
        OffsetIs64Bits
        };
    _HDLLu void                  SetMaxFileSizeSupported(MaxOffsetBitsSupported pi_OffsetBits);


protected:

    _HDLLu void                    Open(const WString&     pi_Filename,
                                        HFCAccessMode      pi_AccessMode,
                                        bool              pi_ShareWrite,
                                        bool              pi_ShareRead,
                                        bool              pi_CreateFile,
                                        uint64_t          pi_OriginOffset,
                                        bool              pi_IgnoreMode,
                                        bool              pi_AutoRemove,
                                        short pi_NbRetry=0);

private:

    friend struct LocalBinStreamCreator;

    _HDLLu void            SetLastExceptionClassID();

    WString                CookFilenameWithLongNameTagW() const;


    // Disabled methods

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

    bool                   m_FileIsUTF8Encoded;
    HArrayAutoPtr<WChar>   m_StackChar;
    size_t                 m_StackSize;
    size_t                 m_StackCharIndex;
    HArrayAutoPtr<char>    m_BufferChar;
    size_t                 m_BufferCharIndex;
    };

