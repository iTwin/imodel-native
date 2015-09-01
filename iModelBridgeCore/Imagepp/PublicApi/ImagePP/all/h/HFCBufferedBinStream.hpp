//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCBufferedBinStream.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Inline methods for class HFCBufferedBinStream
//---------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline uint64_t HFCBufferedBinStream::GetSize() const
    {
    HPRECONDITION(m_pBinStream != 0);
    return m_pBinStream->GetSize();
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HFCAccessMode HFCBufferedBinStream::GetAccessMode() const
    {
    HPRECONDITION(m_pBinStream != 0);
    return m_pBinStream->GetAccessMode();
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HFCBufferedBinStream::Seek(int64_t pi_Delta)
    {
    m_CurrentPos += pi_Delta;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HFCBufferedBinStream::SeekToPos(uint64_t pi_NewPos)
    {
    m_CurrentPos = pi_NewPos;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HFCBufferedBinStream::SeekToBegin()
    {
    m_CurrentPos = 0;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HFCBufferedBinStream::SeekToEnd()
    {
    m_CurrentPos = GetSize();
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline uint64_t HFCBufferedBinStream::GetCurrentPos()
    {
    return m_CurrentPos;
    }
END_IMAGEPP_NAMESPACE