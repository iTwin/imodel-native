//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCBufferedBinStream.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCBufferedBinStream
//---------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HFCExclusiveKey.h>

BEGIN_IMAGEPP_NAMESPACE

class HFCTimer;

class HFCBufferedBinStream : public HFCBinStream
    {
public:

    HDECLARE_CLASS_ID(HFCBinStreamId_Buffered, HFCBinStream);

    // Primary methods

    HFCBufferedBinStream(HFCBinStream* pi_pStream,
                         uint32_t pi_BufferLimit = 64,
                         Byte pi_RatioAlloc = 25,
                         Byte pi_RatioDelete = 10);
    virtual                 ~HFCBufferedBinStream();

    // Information methods

    virtual HFCPtr<HFCURL>  GetURL() const;
    virtual uint64_t       GetSize() const;
    virtual HFCAccessMode   GetAccessMode() const;

    // Multiuser access

    virtual void            Lock(uint64_t pi_Pos, uint64_t pi_Size, bool pi_Share);
    virtual void            Unlock(uint64_t pi_Pos, uint64_t pi_Size);

    // File pointer management

    virtual void            Seek(int64_t pi_Delta);
    virtual void            SeekToPos(uint64_t pi_NewPos);
    virtual void            SeekToBegin();
    virtual void            SeekToEnd();
    virtual uint64_t        GetCurrentPos();

    // Content access

    virtual size_t          Read(void* po_pData, size_t pi_DataSize);
    virtual size_t          Write(const void* pi_pData, size_t pi_DataSize);
    virtual bool           Flush()    {
        return true;
        }

protected:

    void                    CleanBuffers();

private:

    // Disabled methods

    HFCBufferedBinStream(const HFCBufferedBinStream& pi_rObj);
    HFCBufferedBinStream& operator=(const HFCBufferedBinStream& pi_rObj);

    // Internal types

    struct BufferInfo
        {
        uint64_t m_StartPos;
        size_t m_Length;
        size_t m_TotalSize;
        bool m_Modified;
        Byte* m_pBuffer;

        bool IncludesStartOf(const BufferInfo& pi_rObj) const
            {
            return (pi_rObj.m_StartPos >= m_StartPos) && (pi_rObj.m_StartPos < (m_StartPos + m_Length));
            }
        };

    typedef list<BufferInfo> BufferList;

    struct LockInfo
        {
        uint64_t m_StartPos;
        size_t m_Length;

        bool IncludesStartOf(const LockInfo& pi_rObj) const
            {
            return (pi_rObj.m_StartPos >= m_StartPos) && (pi_rObj.m_StartPos < (m_StartPos + m_Length));
            }
        };

    typedef list<LockInfo> LockList;

    // Data members

    HFCExclusiveKey         m_Key;
    HFCBinStream*           m_pBinStream;
    size_t                  m_BufferLimit;
    size_t                  m_BufferFree;
    Byte                  m_RatioAlloc;    // in percent
    Byte                  m_RatioDelete;   // in percent
    uint64_t               m_CurrentPos;
    BufferList              m_Buffers;
    LockList                m_Locks;
    HFCTimer*               m_pTimer;

    };

END_IMAGEPP_NAMESPACE

#include "HFCBufferedBinStream.hpp"
