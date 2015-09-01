//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPATokenizer.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HPATokenizer
//---------------------------------------------------------------------------

#include <ImagePP/all/h/HPAToken.h>

BEGIN_IMAGEPP_NAMESPACE

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPADefaultTokenizer::SetCommentMarker(WChar pi_Marker)
    {
    m_CommentMarker = pi_Marker;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPADefaultTokenizer::SetNumberToken(HPAToken& pi_rToken)
    {
    m_pNumberToken = &pi_rToken;
    pi_rToken.SetName(L"Numeric token");
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPADefaultTokenizer::SetStringToken(HPAToken& pi_rToken)
    {
    m_pStringToken = &pi_rToken;
    pi_rToken.SetName(L"String token");
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPADefaultTokenizer::SetIdentifierToken(HPAToken& pi_rToken)
    {
    m_pIdentifierToken = &pi_rToken;
    pi_rToken.SetName(L"Identifier token");
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPADefaultTokenizer::SetErrorToken(HPAToken& pi_rToken)
    {
    m_pErrorToken = &pi_rToken;
    pi_rToken.SetName(L"Error token");
    }
END_IMAGEPP_NAMESPACE