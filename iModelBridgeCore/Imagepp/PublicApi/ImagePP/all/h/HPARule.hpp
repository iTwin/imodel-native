//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Inline methods for class HPARule
//---------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HPARule& HPARule::operator()(HPANodeCreator* pi_pCreator)
    {
    SetNodeCreator(pi_pCreator);
    return *this;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline bool HPARule::IsLeftRecursive() const
    {
    return m_IsLeftRecursive;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPARule::SetLeftRecursive()
    {
    m_IsLeftRecursive = true;
    }
END_IMAGEPP_NAMESPACE