//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPSNode.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Class : HPSNode
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HPANode.h>
#include <Imagepp/all/h/HGF2DWorldCluster.h>
#include <Imagepp/all/h/HGF2DWorld.h>

class PageStatementNode;
class HRARaster;

class HPSNode : public HPANode    // The node returned by Parse()
    {
public:

    HPSNode(HPAGrammarObject* pi_pObj,
            const HPANodeList& pi_rList,
            const HFCPtr<HPASession>& pi_pSession);
    virtual                   ~HPSNode();

    HFCPtr<HGF2DWorldCluster> GetWorldCluster() const;
    HGF2DWorldIdentificator   GetWorldID() const;
    HFCPtr<HRARaster>         GetPage(uint32_t pi_PageID = 0);
    _HDLLg short CountPages() const;
    _HDLLg const PageStatementNode*  GetPageStatementNode(HPMObjectID pi_PageID) const;

protected:

private:

    };

