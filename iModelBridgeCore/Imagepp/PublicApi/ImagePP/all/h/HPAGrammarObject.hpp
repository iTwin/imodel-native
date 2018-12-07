//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPAGrammarObject.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Inline methods for class HPAGrammarObject
//---------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HPAGrammarObject::HPAGrammarObject(HPANodeCreator* pi_pCreator)
    : m_pNodeCreator(pi_pCreator)
    {
    // Nothing else to do
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HPANode* HPAGrammarObject::Resolve(const HPANodeList& pi_rList,
                                          HPASession* pi_pSession)
    {
    HPRECONDITION(m_pNodeCreator != 0);
    return m_pNodeCreator->Create(this, pi_rList, pi_pSession);
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPAGrammarObject::AddReferingProduction(HPAProduction* pi_pProd)
    {
    m_ReferingProductions.push_back(pi_pProd);
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline const HPAReferingProdList& HPAGrammarObject::GetReferingProdList() const
    {
    return m_ReferingProductions;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline void HPAGrammarObject::SetNodeCreator(HPANodeCreator* pi_pCreator)
    {
    m_pNodeCreator = pi_pCreator;
    }




END_IMAGEPP_NAMESPACE