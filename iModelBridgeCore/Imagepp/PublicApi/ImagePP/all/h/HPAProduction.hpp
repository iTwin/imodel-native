//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPAProduction.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Inline methods for class HPAProduction
//---------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HPAProduction::HPAProduction()
    {
    m_pRule = 0;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HPAProduction::HPAProduction(HPAGrammarObject& pi_rFirst)
    {
    m_pRule = 0;
    m_Syntax.push_back(&pi_rFirst);
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HPAProduction::HPAProduction(const HPAProduction& pi_rProduction)
    : m_Syntax(pi_rProduction.m_Syntax), m_pRule(pi_rProduction.m_pRule)
    {
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HPAProduction& HPAProduction::operator=(const HPAProduction& pi_rProduction)
    {
    m_Syntax = pi_rProduction.m_Syntax;
    m_pRule = 0;
    return *this;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPAProduction::AddGrammarObject(HPAGrammarObject* pi_pObj)
    {
    m_Syntax.push_back(pi_pObj);
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HPARule* HPAProduction::GetRule() const
    {
    return m_pRule;
    }

/////////////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HPAProduction operator+(HPAGrammarObject& pi_rLeft,
                               HPAGrammarObject& pi_rRight)
    {
    HPAProduction TempProd(pi_rLeft);
    TempProd.AddGrammarObject(&pi_rRight);
    return TempProd;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline const HPAProduction& operator+(const HPAProduction& pi_rLeft,
                                      HPAGrammarObject& pi_rRight)
    {
    ((HPAProduction&)pi_rLeft).AddGrammarObject(&pi_rRight);
    return pi_rLeft;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HPAProductionList operator||(const HPAProduction& pi_rLeft,
                                    const HPAProduction& pi_rRight)
    {
    HPAProductionList TempList;
    TempList.push_back(pi_rLeft);
    TempList.push_back(pi_rRight);
    return TempList;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HPAProductionList operator||(const HPAProduction& pi_rLeft,
                                    HPAGrammarObject& pi_rRight)
    {
    HPAProductionList TempList;
    TempList.push_back(pi_rLeft);
    TempList.push_back(HPAProduction(pi_rRight));
    return TempList;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline const HPAProductionList& operator||(const HPAProductionList& pi_rLeft,
                                           const HPAProduction& pi_rRight)
    {
    ((HPAProductionList&)pi_rLeft).push_back(pi_rRight);
    return pi_rLeft;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline const HPAProductionList& operator||(const HPAProductionList& pi_rLeft,
                                           HPAGrammarObject& pi_rRight)
    {
    ((HPAProductionList&)pi_rLeft).push_back(HPAProduction(pi_rRight));
    return pi_rLeft;
    }
END_IMAGEPP_NAMESPACE