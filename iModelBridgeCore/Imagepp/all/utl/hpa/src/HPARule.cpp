//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpa/src/HPARule.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPARule
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPARule.h>
#include <Imagepp/all/h/HPAProduction.h>

static struct HPARuleNodeCreator : public HPANodeCreator
    {
    virtual HPANode* Create(HPAGrammarObject* pi_pObj,
                            const HPANodeList& pi_rList,
                            const HFCPtr<HPASession>& pi_pSession)
        {
        return new HPANode(pi_pObj, pi_rList, pi_pSession);
        }
    } s_RuleNodeCreator;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPARule::HPARule()
    : HPAGrammarObject(&s_RuleNodeCreator), m_IsLeftRecursive(false)
    {
    // Nothing else to do
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPARule::HPARule(const HPAProduction& pi_rProduction)
    : HPAGrammarObject(&s_RuleNodeCreator), m_IsLeftRecursive(false)
    {
    m_Productions.push_back(pi_rProduction);
    LinkProductions();
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPARule::HPARule(const HPAProductionList& pi_rProdList)
    : HPAGrammarObject(&s_RuleNodeCreator),
      m_Productions(pi_rProdList), m_IsLeftRecursive(false)
    {
    LinkProductions();
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPARule::~HPARule()
    {
    // to be done...
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPARule& HPARule::operator=(const HPAProduction& pi_rProduction)
    {
    HPRECONDITION(m_Productions.size() == 0);
    m_Productions.push_back(pi_rProduction);
    LinkProductions();
    return *this;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPARule& HPARule::operator=(const HPAProductionList& pi_rProdList)
    {
    HPRECONDITION(m_Productions.size() == 0);
    m_Productions = pi_rProdList;
    LinkProductions();
    return *this;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HPARule::LinkProductions()
    {
    HPAProductionList::iterator itr = m_Productions.begin();
    while (itr != m_Productions.end())
        {
        (*itr).SetRule(this);  // will also connect production with its objects
        ++itr;
        }
    }

