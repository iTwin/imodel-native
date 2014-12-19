//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSSession.hpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Inline methods for class HPSSession
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
inline bool HPSSession::ModifyCluster(HGF2DWorldIdentificator pi_World,
                                       const HGF2DTransfoModel& pi_rRelation,
                                       HGF2DWorldIdentificator pi_BaseWorld)
    {
    return m_pCluster->SetRelation(pi_World, pi_rRelation, pi_BaseWorld);
    }

//---------------------------------------------------------------------------
inline HFCPtr<HPSWorldCluster> HPSSession::GetWorldCluster() const
    {
    return m_pCluster;
    }

//---------------------------------------------------------------------------
inline void HPSSession::ChangeCurrentWorld(HGF2DWorldIdentificator pi_NewID)
    {
    m_CurrentWorldID = pi_NewID;
    m_pCurrentCoordSys = m_pCluster->GetWorldReference(pi_NewID);
    }

//---------------------------------------------------------------------------
inline HGF2DWorldIdentificator HPSSession::GetCurrentWorldID() const
    {
    return m_CurrentWorldID;
    }

//---------------------------------------------------------------------------
inline HFCPtr<HGF2DCoordSys> HPSSession::GetCurrentWorld() const
    {
    return m_pCurrentCoordSys;
    }

//---------------------------------------------------------------------------
inline HFCPtr<HGF2DCoordSys> HPSSession::GetWorld(HGF2DWorldIdentificator pi_WorldID) const
    {
    return (HFCPtr<HGF2DCoordSys>)m_pCluster->GetWorldReference(pi_WorldID);
    }

//---------------------------------------------------------------------------
inline void HPSSession::AddPageNode(HPANode* pi_pNode)
    {
    m_PageNodes.push_back(pi_pNode);
    }

//---------------------------------------------------------------------------
inline HPANode* HPSSession::GetPageNode(size_t pi_PageID) const
    {
    return m_PageNodes[pi_PageID];
    }

//---------------------------------------------------------------------------
inline size_t HPSSession::CountPages() const
    {
    return m_PageNodes.size();
    }

//---------------------------------------------------------------------------
inline HPSParserScope* HPSSession::GetTopScope() const
    {
    return m_pTopScope;
    }

//---------------------------------------------------------------------------
inline HPSParserScope* HPSSession::GetCurrentScope() const
    {
    return m_pCurrentScope;
    }

//---------------------------------------------------------------------------
inline void HPSSession::ChangeScope(HPSParserScope* pi_pScope)
    {
    m_pCurrentScope = pi_pScope;
    }

//---------------------------------------------------------------------------
inline HFCPtr<HFCURL> HPSSession::GetURL() const
    {
    return m_pURL;
    }

//---------------------------------------------------------------------------
inline HPMPool* HPSSession::GetPool() const
    {
    return m_pPool;
    }
