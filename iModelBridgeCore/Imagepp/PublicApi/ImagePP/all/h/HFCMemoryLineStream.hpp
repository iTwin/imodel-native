//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMemoryLineStream.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HFCMemoryLineStream
//---------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
inline uint32_t HFCMemoryLineStream::GetNbLines() const
    {
    return (uint32_t)m_pLineStartingAddresses->size();
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
inline uint64_t HFCMemoryLineStream::GetLinePos(uint32_t pi_LineNb) const
    {
    uint64_t LinePos = UINT64_MAX;

    if (pi_LineNb < m_pLineStartingAddresses->size())
        {
        LinePos = (*m_pLineStartingAddresses)[pi_LineNb];
        }

    return LinePos;
    }

END_IMAGEPP_NAMESPACE