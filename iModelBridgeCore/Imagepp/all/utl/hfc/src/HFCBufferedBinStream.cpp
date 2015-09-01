//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCBufferedBinStream.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCBufferedBinStream
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCBufferedBinStream.h>
#include <Imagepp/all/h/HFCTimer.h>
#include <Imagepp/all/h/HFCURL.h>

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HFCBufferedBinStream::HFCBufferedBinStream(HFCBinStream* pi_pStream,
                                           uint32_t pi_BufferLimit,
                                           Byte pi_RatioAlloc,
                                           Byte pi_RatioDelete)
    : m_BufferLimit(pi_BufferLimit * 1024), m_pBinStream(pi_pStream),
      m_BufferFree(pi_BufferLimit * 1024), m_CurrentPos(0),
      m_RatioAlloc(pi_RatioAlloc), m_RatioDelete(pi_RatioDelete)
    {
    HPRECONDITION(m_pBinStream != 0);
    HPRECONDITION(m_RatioAlloc > 0);
    HPRECONDITION(m_RatioDelete > 0);
    HPRECONDITION(m_RatioAlloc < 100);
    HPRECONDITION(m_RatioDelete < 100);

    m_pTimer = new HFCMethodCallbackTimer<HFCBufferedBinStream>(this, &HFCBufferedBinStream::CleanBuffers);

    m_BinStreamOpen = true;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HFCBufferedBinStream::~HFCBufferedBinStream()
    {
    HPRECONDITION(m_pBinStream != 0);
    delete m_pTimer;
    CleanBuffers();
    delete m_pBinStream;
    }


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HFCBufferedBinStream::Lock(uint64_t pi_Pos, uint64_t pi_Size, bool pi_Share)
    {
    HPRECONDITION(m_pBinStream != 0);
    m_Key.ClaimKey();

    m_pBinStream->Lock(pi_Pos, pi_Size, pi_Share);

    LockInfo Info;
    Info.m_StartPos = pi_Pos;
    Info.m_Length = (size_t)pi_Size;
    m_Locks.push_back(Info);

    m_Key.ReleaseKey();
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HFCBufferedBinStream::Unlock(uint64_t pi_Pos, uint64_t pi_Size)
    {
    HPRECONDITION(m_pBinStream != 0);

    m_Key.ClaimKey();

    LockInfo InfoL;
    InfoL.m_StartPos = pi_Pos;
    InfoL.m_Length = (size_t)pi_Size;
    LockList::iterator itrL = m_Locks.begin();
    while (itrL != m_Locks.end())
        {
        if ((*itrL).IncludesStartOf(InfoL))
            break;
        ++itrL;
        }
    HASSERT(itrL != m_Locks.end());
    m_Locks.erase(itrL);

    // delete affected buffer(s)

    if (GetAccessMode().m_HasWriteAccess)
        {
        BufferInfo InfoB;
        InfoB.m_StartPos = pi_Pos;
        InfoB.m_Length = (size_t)pi_Size;
        BufferList::iterator itrB = m_Buffers.begin();
        while (itrB != m_Buffers.end())
            {
            if ((*itrB).IncludesStartOf(InfoB))
                break;
            ++itrB;
            }
        while ((itrB != m_Buffers.end()) && (InfoB.m_Length != 0))
            {
            size_t Offset = (size_t)(InfoB.m_StartPos - (*itrB).m_StartPos);
            size_t Length = (*itrB).m_Length - Offset;
            if (Length > InfoB.m_Length)
                Length = InfoB.m_Length;
            InfoB.m_StartPos += Length;
            InfoB.m_Length -= Length;
            delete[] (*itrB).m_pBuffer;
            m_BufferFree += (*itrB).m_TotalSize;
            m_Buffers.erase(itrB);
            if (InfoB.m_Length != 0)
                {
                itrB = m_Buffers.begin();
                while (itrB != m_Buffers.end())
                    {
                    if ((*itrB).IncludesStartOf(InfoB))
                        break;
                    ++itrB;
                    }
                }
            }
        }

    m_pBinStream->Unlock(pi_Pos, pi_Size);

    m_Key.ReleaseKey();
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
size_t HFCBufferedBinStream::Read(void* po_pData, size_t pi_DataSize)
    {
    HPRECONDITION(m_pBinStream != 0);

    size_t TotalRead = 0;

    if (!pi_DataSize)
        return 0;

    m_Key.ClaimKey();

    //!!!! here reset the timer
    // m_pTimer->Start(60000, true);

    BufferInfo Info;
    Info.m_StartPos = m_CurrentPos;
    Info.m_Length = pi_DataSize;

    // Using all buffers available for requested block

    BufferList::iterator itr = m_Buffers.begin();
    while (itr != m_Buffers.end())
        {
        if ((*itr).IncludesStartOf(Info))
            break;
        ++itr;
        }

    while ((itr != m_Buffers.end()) && (Info.m_Length != 0))
        {
        uint64_t Offset = Info.m_StartPos - (*itr).m_StartPos;
        HPRECONDITION(Offset < (uint64_t)(*itr).m_Length);
        size_t Length = (size_t)((uint64_t)(*itr).m_Length - Offset);
        if (Length > Info.m_Length)
            Length = Info.m_Length;
        memcpy(po_pData, (*itr).m_pBuffer + Offset, Length);
        Info.m_StartPos += Length;
        Info.m_Length -= Length;
        TotalRead += Length;
        (Byte*&)po_pData += Length;

        // if buffer was fully used, delete it and adjust free count

        if (Length == ((*itr).m_Length - Offset))
            {
            delete[] (*itr).m_pBuffer;
            m_BufferFree += (*itr).m_TotalSize;
            m_Buffers.erase(itr);
            }
        else // Otherwise keep it as active buffer
            {
            m_Buffers.push_front((*itr));    // The buffer list is also used as a MRU list
            m_Buffers.erase(itr);
            }

        if (Info.m_Length != 0)
            {
            itr = m_Buffers.begin();
            while (itr != m_Buffers.end())
                {
                if ((*itr).IncludesStartOf(Info))
                    break;
                ++itr;
                }
            }
        }

    // Is there any data still to be read? If so, load it from file,
    // loading supplementary buffer in the same move.

    if (Info.m_Length > 0)
        {
        // An old buffer is deleted if free space is under the trigger level

        if (m_BufferFree < ((m_BufferLimit / 100) * m_RatioDelete))
            {   // The buffer list is used as a MRU : last entry is the lru
            delete[] m_Buffers.back().m_pBuffer;
            m_BufferFree += m_Buffers.back().m_TotalSize;
            m_Buffers.pop_back();
            }

        // Preparing to load.

        size_t SizeToLoad = Info.m_Length;
        Byte* pDestData = (Byte*)po_pData;
        bool BufferIt = !(GetAccessMode().m_HasWriteAccess);
        if (BufferIt)
            {
            SizeToLoad = (m_BufferFree / 100) * m_RatioAlloc;
            }
        else
            {
            // Check if locked, set load size to remain of the lock if bigger
            // than data to read

            LockInfo LockToFind;
            LockToFind.m_StartPos = Info.m_StartPos;
            LockList::iterator FoundLock = m_Locks.begin();
            while (FoundLock != m_Locks.end())
                {
                if ((*FoundLock).IncludesStartOf(LockToFind))
                    break;
                ++FoundLock;
                }

            if (FoundLock != m_Locks.end())
                {
                size_t Offset = (size_t)(Info.m_StartPos - (*itr).m_StartPos);
                size_t Length = (*itr).m_Length - Offset;
                if (Length > Info.m_Length)
                    SizeToLoad = Length;
                }
            }

        // If size to load if bigger than requested data size at this moment, this means
        // that buffering has to be done.  Otherwise we directly load data.

        if (SizeToLoad > Info.m_Length)
            pDestData = new Byte[SizeToLoad];

        m_pBinStream->SeekToPos(Info.m_StartPos);

        size_t BytesRead = m_pBinStream->Read(pDestData, SizeToLoad);

        TotalRead += ((BytesRead > Info.m_Length) ? Info.m_Length : BytesRead);

        if (SizeToLoad > Info.m_Length)
            {
            BufferInfo NewBufferInfo;
            NewBufferInfo.m_StartPos = Info.m_StartPos;
            NewBufferInfo.m_Length = BytesRead;
            NewBufferInfo.m_TotalSize = SizeToLoad;  // size of the alloc, may differ from BytesRead
            NewBufferInfo.m_pBuffer = pDestData;
            m_Buffers.push_front(NewBufferInfo);
            m_BufferFree -= SizeToLoad;  // size of the alloc, may differ from BytesRead
            memcpy(po_pData, pDestData, Info.m_Length);
            }
        }

    m_CurrentPos += pi_DataSize;

    m_Key.ReleaseKey();

    return TotalRead;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
size_t HFCBufferedBinStream::Write(const void* pi_pData, size_t pi_DataSize)
    {
    HPRECONDITION(m_pBinStream != 0);

    if (!pi_DataSize)
        return 0;

    m_Key.ClaimKey();

    // delete affected buffer(s)

    BufferInfo Info;
    Info.m_StartPos = m_CurrentPos;
    Info.m_Length = pi_DataSize;
    BufferList::iterator itr = m_Buffers.begin();
    while (itr != m_Buffers.end())
        {
        if ((*itr).IncludesStartOf(Info))
            break;
        ++itr;
        }

    while ((itr != m_Buffers.end()) && (Info.m_Length != 0))
        {
        size_t Offset = (size_t)(Info.m_StartPos - (*itr).m_StartPos);
        size_t Length = (*itr).m_Length - Offset;
        if (Length > Info.m_Length)
            Length = Info.m_Length;
        Info.m_StartPos += Length;
        Info.m_Length -= Length;
        delete[] (*itr).m_pBuffer;
        m_BufferFree += (*itr).m_TotalSize;
        m_Buffers.erase(itr);
        if (Info.m_Length != 0)
            {
            itr = m_Buffers.begin();
            while (itr != m_Buffers.end())
                {
                if ((*itr).IncludesStartOf(Info))
                    break;
                ++itr;
                }
            }
        }

    // writing data

    m_pBinStream->SeekToPos(m_CurrentPos);
    size_t BytesWritten = m_pBinStream->Write(pi_pData, pi_DataSize);
    m_CurrentPos += pi_DataSize;

    m_Key.ReleaseKey();

    return BytesWritten;
    }


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HFCBufferedBinStream::CleanBuffers()
    {
    m_Key.ClaimKey();
    while (m_Buffers.size() != 0)
        {
        BufferList::iterator itr = m_Buffers.begin();
        delete[] (*itr).m_pBuffer;
        m_BufferFree += (*itr).m_TotalSize;
        m_Buffers.erase(itr);
        }
    m_Key.ReleaseKey();
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HFCPtr<HFCURL> HFCBufferedBinStream::GetURL() const
    {
    HPRECONDITION(m_pBinStream != 0);
    return m_pBinStream->GetURL();
    }