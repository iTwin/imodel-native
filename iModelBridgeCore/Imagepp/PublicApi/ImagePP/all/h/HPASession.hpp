//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPASession.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Inline methods for class HPASession
//---------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
inline HPASession::HPASession(HFCBinStream* pi_pStream)
    {
    m_pStream = pi_pStream;
    m_pParser = 0;
    }

inline HPAParser* HPASession::GetParser() const
    {
    return m_pParser;
    }

inline void HPASession::SetParser(HPAParser* pi_pParser)
    {
    m_pParser = pi_pParser;
    }

inline HFCBinStream* HPASession::GetSrcStream() const
    {
    return m_pStream;
    }
END_IMAGEPP_NAMESPACE