//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPADynamicParser.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPADynamicParser
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HPAParser.h>

BEGIN_IMAGEPP_NAMESPACE

class HPADynamicParser : public HPAParser
    {
public:

    HPADynamicParser();
    virtual             ~HPADynamicParser();

    void                ParseGrammar(const HFCPtr<HPASession>& pi_pSession);
    virtual HPANode*    Parse(const HFCPtr<HPASession>& pi_pSession);

protected:

private:

    // Tokens

    HPAToken            EQ_tk;
    HPAToken            OR_tk;
    HPAToken            TOKEN_tk;
    HPAToken            BEGIN_tk;
    HPAToken            END_tk;
    HPAToken            SC_tk;
    HPAToken            Number_tk;
    HPAToken            String_tk;
    HPAToken            Identifier_tk;

    // Rules

    HPARule             GrammarDesc;
    HPARule             DeclarationList;
    HPARule             Declaration;
    HPARule             TokenDeclaration;
    HPARule             RuleDeclaration;
    HPARule             ProductionList;
    HPARule             Production;
    HPARule             GrammarObject;

    // ...

    friend class GrammarDescNode;
    friend class ProductionNode;
    friend class ProductionListNode;
    friend class RuleDeclarationNode;
    friend class TokenDeclarationNode;
    friend class GrammarObjectNode;

    typedef map<WString, HPAGrammarObject*> RuleList;
    typedef map<WString, HPAGrammarObject*> TokenList;

    RuleList            m_Rules;
    TokenList           m_Tokens;
    HPAGrammarObject*   m_pNewStartRule;
    HPADefaultTokenizer* m_pNewTokenizer;
    HPAToken*           m_pStringToken;
    HPAToken*           m_pNumberToken;
    HPAToken*           m_pIdentifierToken;

    };

END_IMAGEPP_NAMESPACE