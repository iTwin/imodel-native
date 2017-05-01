//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/IDTMFeatureHeaderPacketHandler.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeatureHeaderPacketHandler.h>

#include <STMInternal/Storage/HTGFFPacketManager.h>

namespace IDTMFile {

FeatureHeaderPacketHandler::PacketEditor::PacketID FeatureHeaderPacketHandler::PacketEditor::GetID () const
    {
    return GetPacketID();
    }

//size_t                  GetSize                            () const;
bool FeatureHeaderPacketHandler::PacketEditor::GetHeaders (Packet& po_rHeaders) const
    {
    return GetBase().GetHeaders(GetPacketID(), po_rHeaders);
    }


bool FeatureHeaderPacketHandler::IsCompatibleWith (const FeatureDir& pi_rDir)
    {
    return !pi_rDir.IsPointOnly();
    }

FeatureHeaderPacketHandler::CPtr FeatureHeaderPacketHandler::CreateFrom (const FeatureDir* pi_rpDir)
    {
    if (0 == pi_rpDir || !IsCompatibleWith(*pi_rpDir))
        return CPtr();

    HASSERT(0 != pi_rpDir->GetHeaderDirP());
    return new FeatureHeaderPacketHandler(pi_rpDir->GetHeaderDirP());
    }

FeatureHeaderPacketHandler::FeatureHeaderPacketHandler (FeatureHeaderDir* pi_rpDir)
    :   m_pDir(pi_rpDir)
    {

    }

FeatureHeaderPacketHandler::PacketCIter FeatureHeaderPacketHandler::PacketBegin () const
    {
    return PacketIterMgr<CPacket>(GetDir()).begin(*this);
    }

FeatureHeaderPacketHandler::PacketCIter FeatureHeaderPacketHandler::PacketEnd () const
    {
    return PacketIterMgr<CPacket>(GetDir()).end(*this);
    }


bool FeatureHeaderPacketHandler::GetHeaders    (PacketID    pi_ID,
                                                Packet&     po_rHeaders) const
    {
    return GetDir().GetHeaders(pi_ID, po_rHeaders);
    }

} //End namespace IDTMFile