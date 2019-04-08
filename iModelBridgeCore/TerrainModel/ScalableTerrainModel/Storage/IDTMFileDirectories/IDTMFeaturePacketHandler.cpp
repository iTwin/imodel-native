//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/IDTMFeaturePacketHandler.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>



#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeaturePacketHandler.h>

#include <STMInternal/Storage/HTGFFPacketManager.h>

namespace IDTMFile {

FeaturePacketHandler::PacketEditor::PacketID FeaturePacketHandler::PacketEditor::GetID () const
    {
    return GetPacketID();
    }

size_t FeaturePacketHandler::PacketEditor::CountPoints () const
    {
    return GetBase().GetDir().CountPoints(GetPacketID());
    }

size_t FeaturePacketHandler::PacketEditor::CountFeatures () const
    {
    return GetBase().GetDir().CountFeatures(GetPacketID());
    }

bool FeaturePacketHandler::PacketEditor::GetFeatures   (Packet& po_rHeaders,
                                                        Packet& po_rPoints) const
    {
    return GetBase().GetFeatures(GetPacketID(), po_rHeaders, po_rPoints);
    }

bool FeaturePacketHandler::IsCompatibleWith (const FeatureDir& pi_rDir)
    {
    return FeatureHeaderPacketHandler::IsCompatibleWith(pi_rDir) && PointPacketHandler::IsCompatibleWith(pi_rDir);
    }

FeaturePacketHandler::CPtr FeaturePacketHandler::CreateFrom (const FeatureDir* pi_rpDir)
    {
    if (0 == pi_rpDir)
        return CPtr();

    FeatureHeaderPacketHandler::CPtr pHeaderHandler = FeatureHeaderPacketHandler::CreateFrom(pi_rpDir);
    if (0 == pHeaderHandler)
        return CPtr();

    PointPacketHandler::CPtr pPointHandler = PointPacketHandler::CreateFrom(pi_rpDir);
    if (0 == pPointHandler)
        return CPtr();

    return new FeaturePacketHandler(pi_rpDir, pHeaderHandler, pPointHandler);
    }

FeaturePacketHandler::PacketCIter FeaturePacketHandler::PacketBegin () const
    {
    return PacketIterMgr<CPacket>(m_pHeaderHandler->GetDir()).begin(*this);
    }

FeaturePacketHandler::PacketCIter FeaturePacketHandler::PacketEnd () const
    {
    return PacketIterMgr<CPacket>(m_pHeaderHandler->GetDir()).end(*this);
    }

bool FeaturePacketHandler::GetFeatures (PacketID    pi_ID,
                                        Packet&     po_rHeaders,
                                        Packet&     po_rPoints) const
    {
    return m_pHeaderHandler->GetHeaders(pi_ID, po_rHeaders) && m_pPointHandler->GetPoints(pi_ID, po_rPoints);
    }

FeaturePacketHandler::FeaturePacketHandler (const FeatureDir*                           pi_pDir,
                                            const FeatureHeaderPacketHandler::CPtr&     pi_rpHeaderHandler,
                                            const PointPacketHandler::CPtr&             pi_rpPointHandler)
    :   m_pDir(const_cast<FeatureDir*>(pi_pDir)),
        m_pHeaderHandler(pi_rpHeaderHandler),
        m_pPointHandler(pi_rpPointHandler)
    {
    HPRECONDITION(0 != m_pDir);
    HPRECONDITION(0 != m_pHeaderHandler);
    HPRECONDITION(0 != m_pPointHandler);
    }


} //End namespace IDTMFile