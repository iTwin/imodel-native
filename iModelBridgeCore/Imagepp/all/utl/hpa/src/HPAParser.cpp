//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpa/src/HPAParser.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPAParser
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPAParser.h>
#include <Imagepp/all/h/HPATokenizer.h>
#include <Imagepp/all/h/HPAProduction.h>
#include <Imagepp/all/h/HPAToken.h>
#include <Imagepp/all/h/HPARule.h>
#include <Imagepp/all/h/HPAException.h>
#include <Imagepp/all/h/HPASession.h>

//---------------------------------------------------------------------------
// Constructor for this class.
//---------------------------------------------------------------------------
HPAParser::HPAParser(HPATokenizer* pi_pTokenizer,
                     HPAGrammarObject* pi_pStartRule)
    : m_pTokenizer(pi_pTokenizer), m_pStartRule(pi_pStartRule)
    {
    // Nothing else to do
    }

//---------------------------------------------------------------------------
// Destructor for this class.
//---------------------------------------------------------------------------
HPAParser::~HPAParser()
    {
    delete m_pTokenizer;
    }


//---------------------------------------------------------------------------
// This method returns true if the specified object can be used as a
// starting object for the specified rule, even indirectly (through
// another rule).  This method uses recursivity to search for the starting
// object of the starting rules.
//---------------------------------------------------------------------------
bool HPAParser::CheckFirstObject(HPARule* pi_pRule,
                                  HPAGrammarObject* pi_pObj)
    {
    if (pi_pRule == pi_pObj)
        return true;
    bool Result = false;
    HPAProductionList::iterator itr = pi_pRule->m_Productions.begin();
    HPAProductionList::iterator itrEnd = pi_pRule->m_Productions.end();
    while (!Result && (itr != itrEnd))
        {
        Result = ((*itr).m_Syntax.front() == pi_pObj);
        if (!Result && !(*itr).m_Syntax.front()->IsTerminal())
            if ((*itr).m_Syntax.front() != pi_pRule)
                Result = CheckFirstObject((HPARule*)((*itr).m_Syntax.front()), pi_pObj);
        ++itr;
        }
    return Result;
    }


//---------------------------------------------------------------------------
// This method returns true if the specified object can be used as the
// second object for the specified rule, even indirectly (through
// another rule).
//---------------------------------------------------------------------------
bool HPAParser::CheckSecondObject(HPARule* pi_pRule,
                                   HPAGrammarObject* pi_pObj)
    {
    bool Result = false;
    HPAProductionList::iterator itr = pi_pRule->m_Productions.begin();
    HPAProductionList::iterator itrEnd = pi_pRule->m_Productions.end();
    while (!Result && (itr != itrEnd))
        {
        if ((*itr).m_Syntax.front() == pi_pRule)
            {
            Result = ((*itr).m_Syntax[1] == pi_pObj);
            if (!Result && !(*itr).m_Syntax[1]->IsTerminal())
                Result = CheckFirstObject((HPARule*)((*itr).m_Syntax[1]), pi_pObj);
            }
        ++itr;
        }
    return Result;
    }

//---------------------------------------------------------------------------
// Compare two productions : one being created from nodes obtained during
// parsing, and one that resolves to a rule.  To be compatible, the
// production to evaluate must match, object by object, the corresponding
// section of the other one.  Indirect compatiblity means that the rightmost
// object of the production to evaluate may not only match directly, but also
// through other rules.
//---------------------------------------------------------------------------
bool HPAParser::CheckIndirectCompatibility(const HPAProduction& pi_rSource,
                                            const HPAProduction& pi_rToEval)
    {
    bool Result = (pi_rToEval.m_Syntax.size() <= pi_rSource.m_Syntax.size());
    HPAProduction::Syntax::const_iterator itrL = pi_rToEval.m_Syntax.begin();
    HPAProduction::Syntax::const_iterator itrR = pi_rSource.m_Syntax.begin();
    size_t i = 0;
    size_t z = pi_rToEval.m_Syntax.size()-1;
    while (Result && (i < z))
        {
        Result = (*itrL == *itrR);
        ++itrL;
        ++itrR;
        ++i;
        }
    if (Result)
        Result = !((*itrR)->IsTerminal());
    if (Result)
        Result = CheckFirstObject((HPARule*)(*itrR), (*itrL));
    return Result;
    }

//---------------------------------------------------------------------------

struct ParserState {
    HPAProduction m_Production;
    HPANodeList m_NodeList;
    HPAReferingProdList m_ReferingProds;
    };

HPANode* HPAParser::Parse(const HFCPtr<HPASession>& pi_pSession)
    {
    HPRECONDITION(m_pStartRule != 0);

    typedef deque<ParserState> ParserStackType;
    typedef deque<HPANode*> NodeStackType;
    ParserStackType ParserStack;
    NodeStackType LookAhead;

    HPANode* pNode           = 0;
    HPANode* pLastResolution = 0;
    pi_pSession->SetParser(this);
    m_pTokenizer->BeginSession(pi_pSession->GetSrcStream(), pi_pSession);

    try
        {
            {
            LookAhead.clear();
            ParserStack.clear();
            ParserState State;
                {
                HPAProductionList::iterator itr = ((HPARule*)m_pStartRule)->m_Productions.begin();
                while(itr != ((HPARule*)m_pStartRule)->m_Productions.end())
                    {
                    State.m_ReferingProds.push_back(&(*itr));
                    ++itr;
                    }
                }
            HASSERT(State.m_ReferingProds.size() > 0);
            ParserStack.push_front(State);  // final state for top rule

            pNode = m_pTokenizer->GetToken();
            State.m_Production.AddGrammarObject(pNode->GetGrammarObject());
            State.m_NodeList.push_back(pNode);
            State.m_ReferingProds = pNode->GetGrammarObject()->GetReferingProdList();
            HASSERT(State.m_ReferingProds.size() > 0);
            ParserStack.push_front(State);  // beginning with first token....
            }

        // Here we have two states on stack, one for final node, and the one on top
        // which contains the node of the first token on the stream, and the list of
        // productions that begin with that token.  Which one will resolve to a rule?

        while (pNode)
            {
            // Getting next node to add on the current state.  If the look-ahead
            // contains one, take it there, otherwise read next token.

            if (LookAhead.size())
                {
                pNode = LookAhead.front();
                LookAhead.pop_front();
                }
            else
                pNode = m_pTokenizer->GetToken();

            // If the look-ahead is empty, and no token is available, maybe the
            // parsing is almost done, needing to resolve a final rule.  We check the
            // current state to find the production that should be resolved.

            if (!pNode)
                {
                if (ParserStack.size() != 0)
                    {
                    HPAReferingProdList::iterator itr = ParserStack.front().m_ReferingProds.begin();
                    while (itr != ParserStack.front().m_ReferingProds.end())
                        {
                        if (ParserStack.front().m_Production == *(*itr))
                            {
                            HPARule* pRule = (*itr)->GetRule();
                            ParserState State = ParserStack.front();
                            ParserStack.pop_front();
                            pLastResolution = pRule->Resolve(State.m_NodeList, pi_pSession);
                            pNode = pLastResolution;
                            break;
                            }
                        ++itr;
                        }
                    }
                }

            // If the rule resolved just above, or resolved in the preceding loop,
            // is the start rule for the grammar, the parsing is done.

            if (pLastResolution)
                if (pLastResolution->GetGrammarObject() == m_pStartRule)
                    break;

            // No rule resolved, no token?  Stop the parser here, something is wrong.

            if (!pNode)
                break;

            // Now we analyze the production being accumulated on the current state,
            // temporarily adding the new node to it, to check if at least one of the
            // possible productions to be resolved accept it directly.

            HPAProduction ProdToEval = ParserStack.front().m_Production;
            ProdToEval.AddGrammarObject(pNode->GetGrammarObject());
            bool HasFoundReferingProd = false;
            HPAReferingProdList::iterator itr = ParserStack.front().m_ReferingProds.begin();
            while (!HasFoundReferingProd && (itr != ParserStack.front().m_ReferingProds.end()))
                {
                HasFoundReferingProd = (ProdToEval <= *(*itr));
                ++itr;
                }
            if (HasFoundReferingProd)
                {
#if 0  //!!!! Est-ce que ca optimise vraiment?

                // Here we discard, from the list of productions to be resolved,
                // all those that did not accept the new grammar object.

                itr = ParserStack.front().m_ReferingProds.begin();
                while (itr != ParserStack.front().m_ReferingProds.end())
                    {
                    if (ProdToEval > *(*itr))
                        {
                        HPAReferingProdList::iterator itrNext = itr;
                        ++itrNext;
                        ParserStack.front().m_ReferingProds.erase(itr);
                        itr = itrNext;
                        }
                    else
                        ++itr;
                    }
#endif  // ?

                ParserStack.front().m_Production.AddGrammarObject(pNode->GetGrammarObject());
                ParserStack.front().m_NodeList.push_back(pNode);
                }

            // If no available productions in the current state accept the new node, we'll
            // have to push a new state on the state, to find the rule beginning by that
            // grammar object to be resolved as another new node that will be used instead.

            else
                {
                // Does one of the possible productions to be resolved accept the new
                // grammar object "indirectly", as a starting token/rule for the rule
                // that correspond to its position in the production?  This check is
                // done recursively many layers deep, because a token can be the starting
                // object of the starting rule of the starting rule of that rule...

                HasFoundReferingProd = false;
                HPAReferingProdList::iterator itr = ParserStack.front().m_ReferingProds.begin();
                while (!HasFoundReferingProd && (itr != ParserStack.front().m_ReferingProds.end()))
                    {
                    HasFoundReferingProd = CheckIndirectCompatibility(*(*itr), ProdToEval);
                    if (!HasFoundReferingProd)
                        ++itr;
                    }

                // Yes : we push a new state on the stack, whose production will be beginning
                // by that node, and that will resolve later to the needed rule.  Notice that
                // all productions that starts with that node, even indirecly through other
                // rules, are found and registered as possible productions to resolve for
                // that state.

                if (HasFoundReferingProd)
                    {
                    ParserState State;
                    State.m_Production.AddGrammarObject(pNode->GetGrammarObject());
                    State.m_NodeList.push_back(pNode);
                    HPAReferingProdList::const_iterator itr2 = pNode->GetGrammarObject()->GetReferingProdList().begin();
                    while (itr2 != pNode->GetGrammarObject()->GetReferingProdList().end())
                        {
                        if (CheckFirstObject((HPARule*)((*itr)->m_Syntax[ProdToEval.m_Syntax.size()-1]), (*itr2)->GetRule()))
                            State.m_ReferingProds.push_back(*itr2);
                        ++itr2;
                        }
                    HASSERT(State.m_ReferingProds.size() > 0);
                    ParserStack.push_front(State);
                    }

                // No : since no productions accept in any way the new token, maybe we
                // previously got enough nodes to match a complete production. The current
                // node is pushed in the look-ahead queue and we care about resolving
                // the matching production to its rule, popping the state out of the stack
                // and pushing the resulting node in the look-ahead queue for use by
                // next iteration (that will handle previous state with new node).

                else
                    {
                    bool Resolved = false;

                    itr = ParserStack.front().m_ReferingProds.begin();

                    // We look for the winner production and resolve it.

                    while (itr != ParserStack.front().m_ReferingProds.end())
                        {
                        if (ParserStack.front().m_Production == *(*itr))
                            {
                            HPARule* pRule = (*itr)->GetRule();
                            ParserState TopState = ParserStack.front();
                            ParserStack.pop_front();
                            pLastResolution = pRule->Resolve(TopState.m_NodeList, pi_pSession);
                            LookAhead.push_front(pNode);

                            // Special handling for left-recursive rules, whose parsing
                            // is influenced by the look-ahead.  We check the next object,
                            // still known through "pNode", to see if we will resolve
                            // again the same rule in next iterations.

                            if (pRule->IsLeftRecursive())
                                {
                                if (CheckSecondObject(pRule, pNode->GetGrammarObject()))
                                    {
                                    ParserState NewState;
                                    NewState.m_Production.AddGrammarObject(pLastResolution->GetGrammarObject());
                                    NewState.m_NodeList.push_back(pLastResolution);
                                    NewState.m_ReferingProds = pLastResolution->GetGrammarObject()->GetReferingProdList();
                                    HASSERT(NewState.m_ReferingProds.size() > 0);
                                    ParserStack.push_front(NewState);
                                    }
                                else
                                    LookAhead.push_front(pLastResolution);
                                }
                            else
                                LookAhead.push_front(pLastResolution);

                            // Resolved? Get out of the loop!

                            Resolved = true;
                            break;
                            }
                        ++itr;
                        }

                    // If the previous loop didn't find a production, we got an error
                    // in the source stream.  We need to handle it right here.

                    if (!Resolved)
                        {
                        HFCPtr<HPANode> pExceptionNode(pNode);
                        pNode = 0;
                        throw HPACannotFindProductionException(pExceptionNode);    // shoot all info about the node
                        }
                    }
                }

            // Did we resolve the start rule of the grammar?  If so, we break
            // the main loop to stop parsing.

            if (pLastResolution)
                if (pLastResolution->GetGrammarObject() == m_pStartRule)
                    pNode = 0;
            }
        }
    catch  (...)
        {
        //If a node has been created but is not yet in the ParserStack,
        //delete it.
        if ((pNode != 0) && (pNode->GetRefCount() == 0))
            {
            delete pNode;
            }

        m_pTokenizer->EndSession();
        throw;
        }

    m_pTokenizer->EndSession();

    // If the parsing loop completed without resolving the start rule of the grammar,
    // we got an error in the source stream, usually incomplete source.  We need to
    // handle it right here.

    if (pLastResolution->GetGrammarObject() != m_pStartRule)
    {
        HFCPtr<HPANode> pExceptionNode(pLastResolution);
        throw HPACannotResolveStartRuleException(pExceptionNode);  // Shoot all info about the last resolution node.
    }

    // If the parsing loop completed by resolving to start rule but there are still
    // nodes in the lookahead, maybe the starting rule has been resolved too early because
    // of an error in the source stream.  We handle this error right here.

    if (LookAhead.size() != 0)
        {
        NodeStackType::iterator NodeIter    = LookAhead.begin();
        NodeStackType::iterator NodeIterEnd = LookAhead.end();

        //The first lookahead node should be delete when the exception thrown
        //below is destructed.
        NodeIter++;

        while (NodeIter != NodeIterEnd)
            {
            delete *NodeIter;
            NodeIter++;
            }

        HFCPtr<HPANode> pExceptionNode(LookAhead.front());
        throw HPANodeLeftToParseException(pExceptionNode);  // Shoot all info about the unexpected node.
        }

    return pLastResolution;
    }
