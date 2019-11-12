//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HPATokenizer
//---------------------------------------------------------------------------

#include <ImagePP/all/h/HPAToken.h>

BEGIN_IMAGEPP_NAMESPACE

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPADefaultTokenizer::SetCommentMarker(Utf8Char pi_Marker)
    {
    m_CommentMarker = pi_Marker;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPADefaultTokenizer::SetNumberToken(HPAToken& pi_rToken)
    {
    m_pNumberToken = &pi_rToken;
    pi_rToken.SetName("Numeric token");
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPADefaultTokenizer::SetStringToken(HPAToken& pi_rToken)
    {
    m_pStringToken = &pi_rToken;
    pi_rToken.SetName("String token");
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPADefaultTokenizer::SetIdentifierToken(HPAToken& pi_rToken)
    {
    m_pIdentifierToken = &pi_rToken;
    pi_rToken.SetName("Identifier token");
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPADefaultTokenizer::SetErrorToken(HPAToken& pi_rToken)
    {
    m_pErrorToken = &pi_rToken;
    pi_rToken.SetName("Error token");
    }
END_IMAGEPP_NAMESPACE