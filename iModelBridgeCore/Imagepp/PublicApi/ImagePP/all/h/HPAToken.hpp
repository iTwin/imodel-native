//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HPAToken
//---------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HPATokenNode::HPATokenNode(HPAGrammarObject* pi_pToken,
                                  const Utf8String& pi_rText,
                                  const HPASourcePos& pi_rLeftPos,
                                  const HPASourcePos& pi_rRightPos,
                                  HPASession* pi_pSession)
    : HPANode(pi_pToken, pi_rLeftPos, pi_rRightPos, pi_pSession), m_Text(pi_rText)
    {
    // Nothing else to do
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPATokenNode::SetText(const Utf8String& pi_rText)
    {
    m_Text = pi_rText;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline const Utf8String& HPATokenNode::GetText() const
    {
    return m_Text;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HPANumberTokenNode::HPANumberTokenNode(HPAGrammarObject* pi_pToken,
                                              const Utf8String& pi_rText,
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