//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMFeatureHeaderPacketHandler.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : PointTileHandler
//-----------------------------------------------------------------------------

#pragma once

#include <STMInternal/Storage/HTGFFDirectoryHandler.h>
#include <STMInternal/Storage/HTGFFPacketIter.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeatureDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeatureHeaderDir.h>

namespace IDTMFile {

class FeatureHeaderPacketHandler : public HTGFF::DirectoryHandler
    {
private:
    class PacketEditor : public HTGFF::PacketEditorBase<const FeatureHeaderPacketHandler>
        {
        PacketID                GetID                              () const;
        //size_t                  GetSize                            () const;
        bool                    GetHeaders                         (Packet&                     po_rHeaders) const;
        };
public:
    typedef BentleyApi::ImagePP::HFCPtr<FeatureHeaderPacketHandler>      
                                CPtr;

    typedef const PacketEditor  CPacket;
    typedef HTGFF::PacketIter<CPacket> 
                                PacketCIter;

    const FeatureHeaderDir&     GetDir                             () const {return *m_pDir;}   
    FeatureHeaderDir&           GetDir                             () {return *m_pDir;}

    PacketCIter                 PacketBegin                        () const;
    PacketCIter                 PacketEnd                          () const;

    bool                        GetHeaders                         (PacketID                    pi_ID,
                                                                    Packet&                     po_rHeaders) const;

private:
    friend class                FeaturePacketHandler;

    static bool                 IsCompatibleWith                   (const FeatureDir&           pi_rDir);
    static CPtr                 CreateFrom                         (const FeatureDir*           pi_rpDir);

    explicit                    FeatureHeaderPacketHandler         (FeatureHeaderDir*           pi_rpDir);

    virtual bool                _Save                              () override { return true; }
    virtual bool                _Load                              () override { return true; }

    FeatureHeaderDir*           m_pDir;
    };


} //End namespace IDTMFile