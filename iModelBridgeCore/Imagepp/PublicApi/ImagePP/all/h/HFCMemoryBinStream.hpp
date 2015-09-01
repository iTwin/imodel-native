//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMemoryBinStream.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HFCMemoryBinStream
//---------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

inline uint64_t HFCMemoryBinStream::GetSize() const
    {
    return m_BinStreamBuffer->GetDataSize();
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

inline HFCAccessMode HFCMemoryBinStream::GetAccessMode() const
    {
    return m_AccessMode;
    }

//-----------------------------------------------------------------------------
// Returns the offset between the beginning of the file managed by this handle
// and the physical beginning of the file on disk.
//-----------------------------------------------------------------------------

inline uint64_t HFCMemoryBinStream::GetOriginOffset() const
    {
    return m_OriginOffset;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

inline void HFCMemoryBinStream::Seek(int64_t pi_Delta)
    {
    m_CurrentOffset += (size_t)pi_Delta;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

inline void HFCMemoryBinStream::SeekToPos(uint64_t pi_NewPos)
    {
    m_CurrentOffset = (size_t)pi_NewPos;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

inline void HFCMemoryBinStream::SeekToBegin()
    {
    m_CurrentOffset = (size_t)m_OriginOffset;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

inline void HFCMemoryBinStream::SeekToEnd()
    {
    m_CurrentOffset = m_BinStreamBuffer->GetDataSize();
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

inline uint64_t HFCMemoryBinStream::GetCurrentPos()
    {
    return m_CurrentOffset;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

inline bool HFCMemoryBinStream::EndOfFile()
    {
    return (m_CurrentOffset == m_BinStreamBuffer->GetDataSize());
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

inline size_t HFCMemoryBinStream::Read(void* po_pData, size_t pi_DataSize)
    {
    pi_DataSize = MIN(pi_DataSize, m_BinStreamBuffer->GetDataSize() - m_CurrentOffset);
    
    memcpy(po_pData, m_BinStreamBuffer->GetData() + m_CurrentOffset, pi_DataSize);
    m_CurrentOffset += pi_DataSize;

    return pi_DataSize;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

inline size_t HFCMemoryBinStream::Write(const void* pi_pData, size_t pi_DataSize)
    {
    m_BinStreamBuffer->AddData((const Byte* ) pi_pData, pi_DataSize);
    m_CurrentOffset += pi_DataSize;

    return pi_DataSize;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

inline bool HFCMemoryBinStream::Flush()
    {
    // There is nothing to do here, a memory buffer is always fush...
    // The purpose of this method is only the preservation of
    // keeping HFCLocalBinStream interface and behavior.

    return true;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

inline bool HFCMemoryBinStream::SetEOF()
    {
    // There is nothing to do here, a memory buffer is always fush...
    // The purpose of this method is only the preservation of
    // keeping HFCLocalBinStream interface and behavior.

    return false;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

inline HFCPtr<HFCBuffer > HFCMemoryBinStream::GetBuffer() const
    {
    return m_BinStreamBuffer;
    }

END_IMAGEPP_NAMESPACE