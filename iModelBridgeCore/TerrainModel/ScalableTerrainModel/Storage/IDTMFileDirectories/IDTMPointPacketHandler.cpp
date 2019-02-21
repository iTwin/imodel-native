//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/IDTMPointPacketHandler.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMFileDirectories/IDTMPointPacketHandler.h>

#include <STMInternal/Storage/HTGFFPacketManager.h>

namespace IDTMFile {

PointPacketHandler::PacketEditor::PacketID PointPacketHandler::PacketEditor::GetID () const
    {
    return GetPacketID();
    }

size_t PointPacketHandler::PacketEditor::CountPoints () const
    {
    return GetBase().GetDir().CountPoints(GetPacketID());
    }

bool PointPacketHandler::PacketEditor::GetPoints (Packet& po_rPoints) const
    {
    return GetBase().GetPoints(GetPacketID(), po_rPoints);
    }


bool PointPacketHandler::IsCompatibleWith (const PointDir& pi_rDir)
    {
    return true;
    }

PointPacketHandler::CPtr PointPacketHandler::CreateFrom (const PointDir* pi_rpDir)
    {
    if (0 == pi_rpDir || !IsCompatibleWith(*pi_rpDir))
        return CPtr();

    return new PointPacketHandler(pi_rpDir);
    }

bool PointPacketHandler::IsCompatibleWith (const FeatureDir& pi_rDir)
    {
    return true;
    }

PointPacketHandler::CPtr PointPacketHandler::CreateFrom (const FeatureDir* pi_rpDir)
    {
    if (0 == pi_rpDir || !IsCompatibleWith(*pi_rpDir))
        return CPtr();

    return CreateFrom(pi_rpDir->GetPointDirP());
    }


PointPacketHandler::PointPacketHandler (const PointDir* pi_rpDir)
    :   m_pDir(const_cast<PointDir*>(pi_rpDir))
    {

    }


PointPacketHandler::PacketCIter PointPacketHandler::PacketBegin () const
    {
    return PacketIterMgr<CPacket>(GetDir()).begin(*this);
    }
PointPacketHandler::PacketCIter PointPacketHandler::PacketEnd () const
    {
    return PacketIterMgr<CPacket>(GetDir()).end(*this);
    }


bool PointPacketHandler::GetPoints (PacketID    pi_ID,
                                    Packet&     po_rPoints) const
    {
    return GetDir().GetPoints(pi_ID, po_rPoints);
    }

} //End namespace IDTMFile