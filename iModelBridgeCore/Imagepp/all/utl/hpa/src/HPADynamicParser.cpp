//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpa/src/HPADynamicParser.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPADynamicParser
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPADynamicParser.h>
#include <Imagepp/all/h/HPASession.h>

/////////////////////////////////////////////////////////////////////////////
// Specialized node types.  When a rule is resolved a grammar object
// is created or (for topmost rule) the parser is told to use the new
// parser.

BEGIN_IMAGEPP_NAMESPACE
//---------------------------------------------------------------------------
class TokenDeclarationNode : public HPANode
    {
public:
    TokenDeclarationNode(HPAGrammarObject* pi_pObj,
                         const HPANodeList& pi_rList,
                         const HFCPtr<HPASession>& pi_pParser);
    virtual ~TokenDeclarationNode();
    };

TokenDeclarationNode::TokenDeclarationNode(HPAGrammarObject* pi_pObj,
                                           const HPANodeList& pi_rList,
                                           const HFCPtr<HPASession>& pi_pSession)
    : HPANode(pi_pObj, pi_rList, pi_pSession)
    {
    HPADynamicParser* pParser = (HPADynamicParser*)(pi_pSession->GetParser());
    HASSERT(pi_rList.size() == 4);
    HASSERT(pi_rList[1]->GetGrammarObject() == &pParser->Identifier_tk);
    HASSERT(pi_rList[2]->GetGrammarObject() == &pParser->String_tk);
    HPATokenNode* pNameNode = (const HFCPtr<HPATokenNode>&)(pi_rList[1]);
    HPATokenNode* pStringNode = (const HFCPtr<HPATokenNode>&)(pi_rList[2]);
    HPAToken* m_pNewToken = new HPAToken;
    pParser->m_pNewTokenizer->AddSymbol(pStringNode->GetText(), *m_pNewToken);
    pParser->m_Tokens.insert(HPADynamicParser::TokenList::value_type(pNameNode->GetText(), m_pNewToken));
    }

TokenDeclarationNode::~TokenDeclarationNode()
    {
    }

//---------------------------------------------------------------------------
class GrammarObjectNode : public HPANode
    {
public:
    GrammarObjectNode(HPAGrammarObject* pi_pObj,
                      const HPANodeList& pi_rList,
                      const HFCPtr<HPASession>& pi_pSession);
    virtual ~GrammarObjectNode();
    HPAGrammarObject* m_pDesignatedObject;
    };

GrammarObjectNode::GrammarObjectNode(HPAGrammarObject* pi_pObj,
                                     const HPANodeList& pi_rList,
                                     const HFCPtr<HPASession>& pi_pSession)
    : HPANode(pi_pObj, pi_rList, pi_pSession)
    {
    HPADynamicParser* pParser = (HPADynamicParser*)(pi_pSession->GetParser());
    HASSERT(pi_rList.size() == 1);
    m_pDesignatedObject = 0;
    HPATokenNode* pNode = (const HFCPtr<HPATokenNode>&)(pi_rList.front());
    if (pNode->GetGrammarObject() == &pParser->String_tk)
        {
        HPADynamicParser::TokenList::iterator itr = pParser->m_Tokens.find(pNode->GetText());
        if (itr != pParser->m_Tokens.end())
            m_pDesignatedObject = (*itr).second;
        else
            {
            m_pDesignatedObject = new HPAToken;
            pParser->m_pNewTokenizer->AddSymbol(pNode->GetText(), *(HPAToken*)m_pDesignatedObject);
            pParser->m_Tokens.insert(HPADynamicParser::TokenList::value_type(pNode->GetText(), m_pDesignatedObject));
            }
        }
    else if (pNode->GetGrammarObject() == &pParser->Identifier_tk)
        {
        HPADynamicParser::RuleList::iterator itr = pParser->m_Rules.find(pNode->GetText());
        if (itr != pParser->m_Rules.end())
            m_pDesignatedObject = (*itr).second;
        else
            {
            if (pNode->GetText() == L"STRING")
                {
                m_pDesignatedObject = pParser->m_pStringToken;
                }
            else if (pNode->GetText() == L"NUMBER")
                {
                m_pDesignatedObject = pParser->m_pNumberToken;
                }
            else if (pNode->GetText() == L"IDENTIFIER")
                {
                m_pDesignatedObject = pParser->m_pIdentifierToken;
                }
            else {
                HPADynamicParser::TokenList::iterator itr2 = pParser->m_Tokens.find(pNode->GetText());
                if (itr2 != pParser->m_Tokens.end())
                    {
                    m_pDesignatedObject = (*itr2).second;
                    }
                else
                    {
                    m_pDesignatedObject = new HPARule;
                    m_pDesignatedObject->SetName(pNode->GetText());
                    pParser->m_Rules.insert(HPADynamicParser::TokenList::value_type(pNode->GetText(), m_pDesignatedObject));
                    }
                }
            }
        }
    else
        HASSERT(false);
    }

GrammarObjectNode::~GrammarObjectNode()
    {
    }

//---------------------------------------------------------------------------
class ProductionNode : public HPANode
    {
public:
    ProductionNode(HPAGrammarObject* pi_pObj,
                   const HPANodeList& pi_rList,
                   const HFCPtr<HPASession>& pi_pSession);
    virtual ~ProductionNode();
    HPAProduction m_Production;
    };

ProductionNode::ProductionNode(HPAGrammarObject* pi_pObj,
                               const HPANodeList& pi_rList,
                               const HFCPtr<HPASession>& pi_pSession)
    : HPANode(pi_pObj, pi_rList, pi_pSession)
    {
    HPADynamicParser* pParser = (HPADynamicParser*)(pi_pSession->GetParser());
    HASSERT(pi_rList.size() > 0);
    if (pi_rList.front()->GetGrammarObject() == &(pParser->Production))
        {
        HASSERT(pi_rList.size() == 2);
        HASSERT(pi_rList[1]->GetGrammarObject() == &(pParser->GrammarObject));
        m_Production = ((const HFCPtr<ProductionNode>&)(pi_rList.front()))->m_Production;
        m_Production.AddGrammarObject(((const HFCPtr<GrammarObjectNode>&)pi_rList[1])->m_pDesignatedObject);
        }
    else
        {
        HASSERT(pi_rList.size() == 1);
        HASSERT(pi_rList.front()->GetGrammarObject() == &(pParser->GrammarObject));
        m_Production.AddGrammarObject(((const HFCPtr<GrammarObjectNode>&)(pi_rList.front()))->m_pDesignatedObject);
        }
    }

ProductionNode::~ProductionNode()
    {
    }

//---------------------------------------------------------------------------
class ProductionListNode : public HPANode
    {
public:
    ProductionListNode(HPAGrammarObject* pi_pObj,
                       const HPANodeList& pi_rList,
                       const HFCPtr<HPASession>& pi_pSession);
    virtual ~ProductionListNode();
    HPAProductionList m_ProductionList;
    };

ProductionListNode::ProductionListNode(HPAGrammarObject* pi_pObj,
                                       const HPANodeList& pi_rList,
                                       const HFCPtr<HPASession>& pi_pSession)
    : HPANode(pi_pObj, pi_rList, pi_pSession)
    {
    HASSERT((pi_rList.size() == 1) || (pi_rList.size() == 3));
    HPADynamicParser* pParser = (HPADynamicParser*)(pi_pSession->GetParser());
    if (pi_rList.size() == 3)
        {
        HASSERT(pi_rList.front()->GetGrammarObject() == &(pParser->Production));
        m_ProductionList = ((const HFCPtr<ProductionListNode>&)(pi_rList[2]))->m_ProductionList;
        }
    HASSERT(pi_rList.front()->GetGrammarObject() == &(pParser->Production));
    m_ProductionList.push_back(((const HFCPtr<ProductionNode>&)(pi_rList.front()))->m_Production);
    }

ProductionListNode::~ProductionListNode()
    {
    }

//---------------------------------------------------------------------------
class RuleDeclarationNode : public HPANode
    {
public:
    RuleDeclarationNode(HPAGrammarObject* pi_pObj,
                        const HPANodeList& pi_rList,
                        const HFCPtr<HPASession>& pi_pSession);
    virtual ~RuleDeclarationNode();
    };

RuleDeclarationNode::RuleDeclarationNode(HPAGrammarObject* pi_pObj,
                                         const HPANodeList& pi_rList,
                                         const HFCPtr<HPASession>& pi_pSession)
    : HPANode(pi_pObj, pi_rList, pi_pSession)
    {
    HPADynamicParser* pParser = (HPADynamicParser*)(pi_pSession->GetParser());
    HASSERT(pi_rList.size() == 4);
    HASSERT(pi_rList.front()->GetGrammarObject() == &pParser->Identifier_tk);
    HASSERT(pi_rList[2]->GetGrammarObject() == &(pParser->ProductionList));
    HPATokenNode* pNode = (const HFCPtr<HPATokenNode>&)(pi_rList.front());
    HPADynamicParser::RuleList::iterator itr = pParser->m_Rules.find(pNode->GetText());
    if (itr != pParser->m_Rules.end())
        {
        *(HPARule*)((*itr).second) = ((const HFCPtr<ProductionListNode>&)(pi_rList[2]))->m_ProductionList;
        }
    else
        {
        HPARule* pNewRule = new HPARule(((const HFCPtr<ProductionListNode>&)(pi_rList[2]))->m_ProductionList);
        pNewRule->SetName(pNode->GetText());
        pParser->m_Rules.insert(HPADynamicParser::TokenList::value_type(pNode->GetText(), pNewRule));
        }
    }

RuleDeclarationNode::~RuleDeclarationNode()
    {
    }

//---------------------------------------------------------------------------
class GrammarDescNode : public HPANode
    {
public:
    GrammarDescNode(HPAGrammarObject* pi_pObj,
                    const HPANodeList& pi_rList,
                    const HFCPtr<HPASession>& pi_pSession);
    virtual ~GrammarDescNode();
    };

GrammarDescNode::GrammarDescNode(HPAGrammarObject* pi_pObj,
                                 const HPANodeList& pi_rList,
                                 const HFCPtr<HPASession>& pi_pSession)
    : HPANode(pi_pObj, pi_rList, pi_pSession)
    {
    HPADynamicParser* pParser = (HPADynamicParser*)(pi_pSession->GetParser());
    HASSERT(pi_rList.size() == 4);
    HASSERT(pi_rList[3]->GetGrammarObject() == &pParser->Identifier_tk);
    HPATokenNode* pNode = (const HFCPtr<HPATokenNode>&)(pi_rList[3]);
    HPADynamicParser::RuleList::iterator itr = pParser->m_Rules.find(pNode->GetText());
    if (itr != pParser->m_Rules.end())
        {
        pParser->m_pNewStartRule = (*itr).second;
        }
    // else handle an error.
    }

GrammarDescNode::~GrammarDescNode()
    {
    // Nothing to do
    }


/////////////////////////////////////////////////////////////////////////////
// Creators for above node types

static struct GrammarDescNodeCreator : public HPANodeCreator
    {
    virtual HPANode* Create(HPAGrammarObject* pi_pObj,
                            const HPANodeList& pi_rList,
                            const HFCPtr<HPASession>& pi_pSession)
        {
        return new GrammarDescNode(pi_pObj, pi_rList, pi_pSession);
        }
    } s_GrammarDescNodeCreator;

static struct GrammarObjectNodeCreator : public HPANodeCreator
    {
    virtual HPANode* Create(HPAGrammarObject* pi_pObj,
                            const HPANodeList& pi_rList,
                            const HFCPtr<HPASession>& pi_pSession)
        {
        return new GrammarObjectNode(pi_pObj, pi_rList, pi_pSession);
        }
    } s_GrammarObjectNodeCreator;

static struct ProductionNodeCreator : public HPANodeCreator
    {
    virtual HPANode* Create(HPAGrammarObject* pi_pObj,
                            const HPANodeList& pi_rList,
                            const HFCPtr<HPASession>& pi_pSession)
        {
        return new ProductionNode(pi_pObj, pi_rList, pi_pSession);
        }
    } s_ProductionNodeCreator;

static struct ProductionListNodeCreator : public HPANodeCreator
    {
    virtual HPANode* Create(HPAGrammarObject* pi_pObj,
                            const HPANodeList& pi_rList,
                            const HFCPtr<HPASession>& pi_pSession)
        {
        return new ProductionListNode(pi_pObj, pi_rList, pi_pSession);
        }
    } s_ProductionListNodeCreator;

static struct RuleDeclarationNodeCreator : public HPANodeCreator
    {
    virtual HPANode* Create(HPAGrammarObject* pi_pObj,
                            const HPANodeList& pi_rList,
                            const HFCPtr<HPASession>& pi_pSession)
        {
        return new RuleDeclarationNode(pi_pObj, pi_rList, pi_pSession);
        }
    } s_RuleDeclarationNodeCreator;

static struct TokenDeclarationNodeCreator : public HPANodeCreator
    {
    virtual HPANode* Create(HPAGrammarObject* pi_pObj,
                            const HPANodeList& pi_rList,
                            const HFCPtr<HPASession>& pi_pSession)
        {
        return new TokenDeclarationNode(pi_pObj, pi_rList, pi_pSession);
        }
    } s_TokenDeclarationNodeCreator;

END_IMAGEPP_NAMESPACE
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPADynamicParser::HPADynamicParser()
    {
    // Tokenizer setup

    HPADefaultTokenizer* pTok = new HPADefaultTokenizer;
    pTok->AddSymbol(L"=",             EQ_tk);
    pTok->AddSymbol(L"|",             OR_tk);
    pTok->AddSymbol(L";",             SC_tk);
    pTok->AddSymbol(L"TOKEN",         TOKEN_tk);
    pTok->AddSymbol(L"GRAMMAR_BEGIN", BEGIN_tk);
    pTok->AddSymbol(L"GRAMMAR_END",   END_tk);
    pTok->SetNumberToken(Number_tk);
    pTok->SetStringToken(String_tk);
    pTok->SetIdentifierToken(Identifier_tk);
    SetTokenizer(pTok);

    // Grammar definition

    GrammarDesc       =    BEGIN_tk + DeclarationList + END_tk + Identifier_tk;

    DeclarationList   =    DeclarationList + Declaration
                           || Declaration;

    Declaration       =    RuleDeclaration
                           || TokenDeclaration;

    TokenDeclaration  =    TOKEN_tk + Identifier_tk + String_tk + SC_tk;

    RuleDeclaration   =    Identifier_tk + EQ_tk + ProductionList + SC_tk;

    ProductionList    =    Production
                           || Production + OR_tk + ProductionList;

    Production        =    Production + GrammarObject
                           || GrammarObject;

    GrammarObject     =    Identifier_tk
                           || String_tk;

    SetStartRule(&GrammarDesc);

    // Association of node creators with rules.

    GrammarDesc(&s_GrammarDescNodeCreator);
    TokenDeclaration(&s_TokenDeclarationNodeCreator);
    RuleDeclaration(&s_RuleDeclarationNodeCreator);
    ProductionList(&s_ProductionListNodeCreator);
    Production(&s_ProductionNodeCreator);
    GrammarObject(&s_GrammarObjectNodeCreator);

    m_pNewTokenizer = 0;
    m_pNewStartRule = 0;
    m_pNumberToken = 0;
    m_pIdentifierToken = 0;
    m_pStringToken = 0;
    }


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPADynamicParser::~HPADynamicParser()
    {
    delete m_pNewTokenizer;
    delete m_pNumberToken;
    delete m_pIdentifierToken;
    delete m_pStringToken;
    m_pNewTokenizer = 0;
    m_pNewStartRule = 0;
    m_pNumberToken = 0;
    m_pIdentifierToken = 0;
    m_pStringToken = 0;

        {
        RuleList::iterator itr = m_Rules.begin();
        while (itr != m_Rules.end())
            {
            delete (*itr).second;
            ++itr;
            }

// No need for this code anymore since the STL map bug
// is corrected (_Lockit)
//        m_Rules.clear();
        }

        {
        TokenList::iterator itr = m_Tokens.begin();
        while (itr != m_Tokens.end())
            {
            delete (*itr).second;
            ++itr;
            }
// No need for this code anymore since the STL map bug
// is corrected (_Lockit)
//        m_Tokens.clear();
        }
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void HPADynamicParser::ParseGrammar(const HFCPtr<HPASession>& pi_pSession)
    {
    HASSERT(!m_pNewTokenizer);

    m_pNewTokenizer = new HPADefaultTokenizer;
    m_pStringToken = new HPAToken;
    m_pNumberToken = new HPAToken;
    m_pIdentifierToken = new HPAToken;
    m_pNewTokenizer->SetStringToken(*m_pStringToken);
    m_pNewTokenizer->SetNumberToken(*m_pNumberToken);
    m_pNewTokenizer->SetIdentifierToken(*m_pIdentifierToken);
    delete HPAParser::Parse(pi_pSession);
    SetTokenizer(m_pNewTokenizer);
    SetStartRule(m_pNewStartRule);
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPANode* HPADynamicParser::Parse(const HFCPtr<HPASession>& pi_pSession)
    {
    HASSERT(m_pNewTokenizer != 0);
    return HPAParser::Parse(pi_pSession);
    }
