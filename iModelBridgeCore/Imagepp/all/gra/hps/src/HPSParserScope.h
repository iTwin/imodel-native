//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Header for class : HPSParserScope
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------


#pragma once

#include <ImagePP/all/h/HFCPtr.h>
#include "HPSInternalNodes.h"

BEGIN_IMAGEPP_NAMESPACE

class HPSParser;
class ExpressionNode;

class HPSParserScope : public HFCShareableObject<HPSParserScope>
    {
public:

    HPSParserScope(HPSParser* pi_pParser);
    ~HPSParserScope();

    void                SetSession(const HFCPtr<HPSSession>& pi_pSession);
    void                SetOwner(HPSParserScope* pi_pScope);
    HPSParserScope*     GetOwner() const;
    bool               IsTopScope() const;

    void                AddParameter(HPANode* pi_pParameterNode);
    void                AddVariable(const Utf8String& pi_rString, ExpressionNode* pi_pNode);
    void                AddStatement(const Utf8String& pi_rString, StatementDefinitionNode* pi_pNode);

    void                Reset();
    void                SetParameterValue(int16_t pi_Pos, ExpressionNode* pi_pNode);
    StatementDefinitionNode*
    FindStatement(const Utf8String& pi_rString);
    VariableTokenNode*  FindVariable(const Utf8String& pi_rString);
    size_t              GetParameterCount() const;

protected:

    StatementDefinitionNode* FindStatementInternal(const Utf8String& pi_rString);
    VariableTokenNode*       FindVariableInternal(const Utf8String& pi_rString);

private:

    typedef vector<HFCPtr<VariableTokenNode> > ParameterList;

    typedef map<Utf8String, HFCPtr<VariableTokenNode> > VariableList;

    typedef map<Utf8String, HFCPtr<StatementDefinitionNode> > StatementList;


    HPSParser*          m_pParser;
    HFCPtr<HPSSession>  m_pSession;
    HPSParserScope*     m_pOwner;
    ParameterList       m_ParameterList;
    VariableList        m_VariableList;
    StatementList       m_StatementList;

    };

END_IMAGEPP_NAMESPACE

#include "HPSParserScope.hpp"

