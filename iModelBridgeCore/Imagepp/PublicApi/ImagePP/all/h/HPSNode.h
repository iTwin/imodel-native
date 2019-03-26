//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPSNode.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Class : HPSNode
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HPANode.h>
#include <ImagePP/all/h/HGF2DWorldCluster.h>
#include <ImagePP/all/h/HGF2DWorld.h>

BEGIN_IMAGEPP_NAMESPACE
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
    IMAGEPP_EXPORT int16_t CountPages() const;
    IMAGEPP_EXPORT const PageStatementNode*  GetPageStatementNode(HPMObjectID pi_PageID) const;

protected:

private:

    };

END_IMAGEPP_NAMESPACE
