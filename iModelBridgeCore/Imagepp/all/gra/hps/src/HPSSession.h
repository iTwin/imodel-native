//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSSession.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Class : HPSSession
//---------------------------------------------------------------------------
// Stores the current state of one parsing process.
//---------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCURL.h>

#include <Imagepp/all/h/HPANode.h>
#include <Imagepp/all/h/HPASession.h>
#include <Imagepp/all/h/HPSWorldCluster.h>

BEGIN_IMAGEPP_NAMESPACE

class HPMPool;

class HPSParser;
class HPSParserScope;

class HPSSession : public HPASession
    {
public:
    HPSSession(HFCBinStream*                  pi_pStream,
               HPMPool*                       pi_pPool,
               const HFCPtr<HFCURL>&          pi_rpURL,
               HPSParserScope*                pi_pTopScope,
               const HFCPtr<HPSWorldCluster>  pi_rpHPSWorldCluster = HFCPtr<HPSWorldCluster>(new HPSWorldCluster()),
               const HGF2DWorldIdentificator* pi_pCurrentWorldId = 0);
    virtual ~HPSSession();

    // World management

    bool                   ModifyCluster(HGF2DWorldIdentificator pi_World,
                                          const HGF2DTransfoModel& pi_rRelation,
                                          HGF2DWorldIdentificator pi_BaseWorld);
    HFCPtr<HPSWorldCluster> GetWorldCluster() const;
    void                    ChangeCurrentWorld(HGF2DWorldIdentificator pi_NewID);
    HGF2DWorldIdentificator GetCurrentWorldID() const;
    HFCPtr<HGF2DCoordSys>   GetCurrentWorld() const;
    HFCPtr<HGF2DCoordSys>   GetWorld(HGF2DWorldIdentificator pi_WorldID) const;

    // Page management

    void                    AddPageNode(HPANode* pi_pNode);
    HPANode*                GetPageNode(size_t pi_PageID) const;
    size_t                  CountPages() const;
    void                    Clear();

    // Scope management

    HPSParserScope*         GetTopScope() const;
    HPSParserScope*         GetCurrentScope() const;
    void                    ChangeScope(HPSParserScope* pi_pScope);

    // Other

    HFCPtr<HFCURL>          GetURL() const;
    HPMPool*                GetPool() const;

protected:

private:

    // Disabled methods

    HPSSession(const HPSSession&);
    HPSSession& operator=(const HPSSession&);

    // Scopes

    HPSParserScope*     m_pTopScope;
    HPSParserScope*     m_pCurrentScope;

    // Other
    HPMPool*                  m_pPool;
    HFCPtr<HPSWorldCluster>   m_pCluster;
    HFCPtr<HGF2DCoordSys>     m_pCurrentCoordSys;
    HGF2DWorldIdentificator   m_CurrentWorldID;
    HFCPtr<HFCURL>            m_pURL;
    HPANodeList               m_PageNodes;
    };

END_IMAGEPP_NAMESPACE
#include "HPSSession.hpp"
