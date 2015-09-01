//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpa/src/HPANode.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPANode
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPANode.h>
#include <Imagepp/all/h/HPAGrammarObject.h>

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPANode::HPANode(HPAGrammarObject* pi_pObj,
                 const HPASourcePos& pi_rStartPos,
                 const HPASourcePos& pi_rEndPos,
                 const HFCPtr<HPASession>& pi_pSession)
    : m_pGrammarObject(pi_pObj),
      m_StartPos(pi_rStartPos),
      m_EndPos(pi_rEndPos),
      m_pSession(pi_pSession),
      m_pOwner(0)
    {

    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPANode::HPANode(HPAGrammarObject*         pi_pObj,
                 const HPANodeList&        pi_rList,
                 const HFCPtr<HPASession>& pi_pSession,
                 bool                     pi_GetSubNodeOwnership)
    : m_pGrammarObject(pi_pObj),
      m_pSession(pi_pSession),
      m_SubNodes(pi_rList),
      m_pOwner(0)
    {
    HPRECONDITION(pi_rList.size() != 0);
    m_StartPos = pi_rList.front()->m_StartPos;
    m_EndPos   = pi_rList.back()->m_EndPos;

    if (pi_GetSubNodeOwnership == true)
        {
        HPANodeList::iterator itr = m_SubNodes.begin();
        while (itr != m_SubNodes.end())
            {
            (*itr)->SetOwner(this);
            ++itr;
            }
        }
    }

//---------------------------------------------------------------------------
// Deleting the tree is made by deleting its root.
//---------------------------------------------------------------------------
HPANode::~HPANode()
    {
    }

//---------------------------------------------------------------------------
// Free any value that this node is the owner of.
//---------------------------------------------------------------------------
void HPANode::FreeValue()
    {
    HFCPtr<HPANode>             pNode;
    const HPANodeList&          rSubNodeList(GetSubNodes());
    HPANodeList::const_iterator NodeIter(rSubNodeList.begin());
    HPANodeList::const_iterator NodeIterEnd(rSubNodeList.end());

    while (NodeIter != NodeIterEnd)
        {
        HASSERT((*NodeIter) != 0);

        (*NodeIter)->FreeValue();
        NodeIter++;
        }
    }

//---------------------------------------------------------------------------
// Delete sub-nodes.
//---------------------------------------------------------------------------
void HPANode::DeleteSubNodes()
    {
    m_SubNodes.clear();
    }

#ifdef __HMR_DEBUG_MEMBER

void HPANode::PrintState(int pi_Level)
    {
    for (int i = 0; i < pi_Level; i++) wcout << L"  ";
    wcout << m_pGrammarObject->GetName() << endl;
    HPANodeList::iterator itr = m_SubNodes.begin();
    while (itr != m_SubNodes.end())
        {
        (*itr)->PrintState(pi_Level+1);
        ++itr;
        }
    }
#endif

