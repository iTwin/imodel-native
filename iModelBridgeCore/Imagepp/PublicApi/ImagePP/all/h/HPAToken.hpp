//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPAToken.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HPAToken
//---------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HPATokenNode::HPATokenNode(HPAGrammarObject* pi_pToken,
                                  const WString& pi_rText,
                                  const HPASourcePos& pi_rLeftPos,
                                  const HPASourcePos& pi_rRightPos,
                                  HPASession* pi_pSession)
    : HPANode(pi_pToken, pi_rLeftPos, pi_rRightPos, pi_pSession), m_Text(pi_rText)
    {
    // Nothing else to do
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPATokenNode::SetText(const WString& pi_rText)
    {
    m_Text = pi_rText;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline const WString& HPATokenNode::GetText() const
    {
    return m_Text;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HPANumberTokenNode::HPANumberTokenNode(HPAGrammarObject* pi_pToken,
                                              const WString& pi_rText,
                                              const HPASourcePos& pi_rLeftPos,
                                              const HPASourcePos& pi_rRightPos,
                                              HPASession* pi_pSession,
                                              double pi_Value)
    : HPATokenNode(pi_pToken, pi_rText, pi_rLeftPos, pi_rRightPos, pi_pSession),
      m_Value(pi_Value)
    {
    // Nothing else to do
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPANumberTokenNode::SetValue(double pi_Value)
    {
    m_Value = pi_Value;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline double HPANumberTokenNode::GetValue() const
    {
    return m_Value;
    }
END_IMAGEPP_NAMESPACE