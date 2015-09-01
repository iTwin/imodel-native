//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSNode.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPSNode
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HPSNode.h>
#include "HPSSession.h"
#include "HPSParserScope.h"
#include "HPSInternalNodes.h"
#include <Imagepp/all/h/HPSException.h>
#include <Imagepp/all/h/HRARaster.h>

//---------------------------------------------------------------------------
HPSNode::HPSNode(HPAGrammarObject* pi_pObj,
                 const HPANodeList& pi_rList,
                 const HFCPtr<HPASession>& pi_pSession)
    : HPANode(pi_pObj, pi_rList, pi_pSession)
    {
    }

//---------------------------------------------------------------------------
HPSNode::~HPSNode()
    {
    static_cast<HPSSession*>(GetSession().GetPtr())->Clear();

    // should we delete here the objects in list?
    }

//---------------------------------------------------------------------------
HFCPtr<HRARaster> HPSNode::GetPage(uint32_t pi_PageID)
    {
    if (static_cast<HPSSession*>(GetSession().GetPtr())->CountPages() <= pi_PageID)
        {
        throw HPSOutOfRangeException(HFCPtr<HPANode>(new HPANode(GetGrammarObject(), GetSubNodes(), GetSession())),
                                     1,
                                     (double)static_cast<HPSSession*>(GetSession().GetPtr())->CountPages());
        }
    
    HPSRasterObjectValue* pObj = ((PageStatementNode*)static_cast<HPSSession*>(GetSession().GetPtr())->GetPageNode(pi_PageID))->ComputeObject();
    if (pObj == NULL || pObj->m_pObject == NULL)
        throw HPSTypeMismatchException(HFCPtr<HPANode>(static_cast<HPSSession*>(GetSession().GetPtr())->GetPageNode(pi_PageID)),
                                       HPSTypeMismatchException::IMAGE);
    return pObj->m_pObject;
    }

//---------------------------------------------------------------------------
HFCPtr<HGF2DWorldCluster> HPSNode::GetWorldCluster() const
    {
    return (HFCPtr<HGF2DWorldCluster>)(static_cast<HPSSession*>(GetSession().GetPtr())->GetWorldCluster());
    }

//---------------------------------------------------------------------------
HGF2DWorldIdentificator HPSNode::GetWorldID() const
    {
    return static_cast<HPSSession*>(GetSession().GetPtr())->GetCurrentWorldID();
    }

//---------------------------------------------------------------------------
short HPSNode::CountPages() const
    {
    if (static_cast<HPSSession*>(GetSession().GetPtr())->CountPages() == 0)
        {
        throw HPSNoImageInFileException(new HPANode(GetGrammarObject(),
                                       GetSubNodes(),
                                       GetSession()));
        }
    return (short)static_cast<HPSSession*>(GetSession().GetPtr())->CountPages();
    }

//---------------------------------------------------------------------------
const PageStatementNode* HPSNode::GetPageStatementNode(HPMObjectID pi_PageID) const
    {
    return (const PageStatementNode*)static_cast<HPSSession*>(GetSession().GetPtr())->GetPageNode((size_t)pi_PageID);
    }
