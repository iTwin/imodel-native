//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSParserScope.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPSParserScope
//---------------------------------------------------------------------------


#include <ImagePPInternal/hstdcpp.h>

#include "HPSParserScope.h"
#include <Imagepp/all/h/HPAToken.h>
#include <Imagepp/all/h/HPSException.h>
#include <Imagepp/all/h/HPSParser.h>

//---------------------------------------------------------------------------
StatementDefinitionNode* HPSParserScope::FindStatement(const WString& pi_rString)
    {
    WString IdentText(pi_rString);
    BeStringUtilities::Wcsupr (&(IdentText[0]));                  //chck UNICODE
    return FindStatementInternal(IdentText);
    }

//---------------------------------------------------------------------------
VariableTokenNode* HPSParserScope::FindVariable(const WString& pi_rString)
    {
    WString IdentText(pi_rString);
    BeStringUtilities::Wcsupr (&(IdentText[0]));                  //chck UNICODE
    return FindVariableInternal(IdentText);
    }

//---------------------------------------------------------------------------
void HPSParserScope::AddParameter(HPANode* pi_pParameterNode)
    {
    HPRECONDITION(pi_pParameterNode != 0);
    HPRECONDITION(pi_pParameterNode->GetSubNodes().size() == 2);

    WString IdentText(((const HFCPtr<HPATokenNode>&)(pi_pParameterNode->GetSubNodes()[1]))->GetText());
    BeStringUtilities::Wcsupr (&(IdentText[0]));                  //chck UNICODE

    if ((FindVariable(IdentText) != 0) || (FindStatement(IdentText) != 0))
        throw HPSAlreadyDefinedException(HFCPtr<HPANode>(pi_pParameterNode), IdentText);

    VariableTokenNode* pNode = new VariableTokenNode(&m_pParser->VariableIdentifier_tk, 0,
                                                     pi_pParameterNode->GetSubNodes()[1]->GetStartPos(),
                                                     pi_pParameterNode->GetSubNodes()[1]->GetEndPos(),
                                                     m_pSession);
    m_ParameterList.push_back(pNode);
    m_VariableList.insert(VariableList::value_type(IdentText, pNode));
    }

//---------------------------------------------------------------------------
void HPSParserScope::AddVariable(const WString& pi_rString, ExpressionNode* pi_pNode)
    {
    HPRECONDITION(pi_pNode != 0);

    WString IdentText(pi_rString);
    BeStringUtilities::Wcsupr (&(IdentText[0]));              //chck UNICODE

    if ((FindVariable(IdentText) != 0) || (FindStatement(IdentText) != 0))
        throw HPSAlreadyDefinedException(HFCPtr<HPANode>(pi_pNode), IdentText);

    VariableTokenNode* pNode = new VariableTokenNode(&m_pParser->VariableIdentifier_tk,
                                                     pi_pNode,
                                                     pi_pNode->GetSubNodes().front()->GetStartPos(),
                                                     pi_pNode->GetSubNodes().front()->GetEndPos(),
                                                     m_pSession);
    m_VariableList.insert(VariableList::value_type(IdentText, pNode));
    }

//---------------------------------------------------------------------------
void HPSParserScope::AddStatement(const WString& pi_rString, StatementDefinitionNode* pi_pNode)
    {
    WString IdentText(pi_rString);
    BeStringUtilities::Wcsupr (&(IdentText[0]));                  //chck UNICODE
    if ((FindVariable(IdentText) != 0) || (FindStatement(IdentText) != 0))
        throw HPSAlreadyDefinedException(HFCPtr<HPANode>(pi_pNode), IdentText);
    m_StatementList.insert(StatementList::value_type(IdentText, pi_pNode));
    }


//---------------------------------------------------------------------------
StatementDefinitionNode* HPSParserScope::FindStatementInternal(const WString& pi_rString)
    {
    StatementList::iterator itr = m_StatementList.find(pi_rString);
    if (itr == m_StatementList.end())
        {
        if (m_pOwner)
            return m_pOwner->FindStatementInternal(pi_rString);
        else
            return 0;
        }
    else
        return (*itr).second;
    }

//---------------------------------------------------------------------------
VariableTokenNode* HPSParserScope::FindVariableInternal(const WString& pi_rString)
    {
    VariableList::iterator itr = m_VariableList.find(pi_rString);
    if (itr == m_VariableList.end())
        {
        if (m_pOwner)
            return m_pOwner->FindVariableInternal(pi_rString);
        else
            return 0;
        }
    else
        return (*itr).second;
    }


