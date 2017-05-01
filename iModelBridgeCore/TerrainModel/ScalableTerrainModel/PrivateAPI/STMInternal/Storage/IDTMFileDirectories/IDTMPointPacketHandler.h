//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMPointPacketHandler.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : PointPacketHandler
//-----------------------------------------------------------------------------

#pragma once

#include <STMInternal/Storage/HTGFFDirectoryHandler.h>

#include <STMInternal/Storage/IDTMFileDirectories/IDTMPointDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeatureDir.h>
#include <STMInternal/Storage/HTGFFPacketIter.h>

namespace IDTMFile {

class PointPacketHandler : public HTGFF::DirectoryHandler
    {
private:
    class PacketEditor : public HTGFF::PacketEditorBase<const PointPacketHandler>
        {
    public:
         PacketID         GetID                              () const;
         size_t           CountPoints                        () const;
         bool             GetPoints                          (Packet& po_rPoints) const;
        };
public:
    typedef BentleyApi::ImagePP::HFCPtr<PointPacketHandler> CPtr;

    typedef const PacketEditor  CPacket;
    typedef HTGFF::PacketIter<CPacket> 
                                PacketCIter;

    const PointDir&             GetDir                             () const {return *m_pDir;}   
    PointDir&                   GetDir                             () {return *m_pDir;}

     static bool          IsCompatibleWith                   (const PointDir&             pi_rDir); 
     static bool          IsCompatibleWith                   (const FeatureDir&           pi_rDir); 

     static CPtr          CreateFrom                         (const PointDir*             pi_rpDir);
     static CPtr          CreateFrom                         (const FeatureDir*           pi_rpDir);


     PacketCIter          PacketBegin                        () const;
     PacketCIter          PacketEnd                          () const;

     bool                 GetPoints                          (PacketID                    pi_ID,
                                                                    Packet&                     po_rPoints) const;

private:
    explicit                    PointPacketHandler                 (const PointDir*             pi_rpDir);


    virtual bool                _Save                              () override { return true; }
    virtual bool                _Load                              () override { return true; }

    PointDir*                   m_pDir;
    };


} //End namespace IDTMFile