//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMFeaturePacketHandler.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : FeaturePacketHandler
//-----------------------------------------------------------------------------

#pragma once

#include <STMInternal/Storage/HTGFFDirectoryHandler.h>

#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeatureDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeatureHeaderPacketHandler.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMPointPacketHandler.h>

namespace IDTMFile {

class FeaturePacketHandler : public HTGFF::DirectoryHandler
    {
private:
    class PacketEditor : public HTGFF::PacketEditorBase<const FeaturePacketHandler>
        {
    public:
         PacketID         GetID                              () const;

         size_t           CountPoints                        () const;
         size_t           CountFeatures                      () const;

         bool             GetFeatures                        (Packet&                     po_rHeaders,
                                                                    Packet&                     po_rPoints) const;
        };
public:
    typedef BentleyApi::ImagePP::HFCPtr<FeaturePacketHandler>      
                                CPtr;

    typedef const PacketEditor  CPacket;
    typedef HTGFF::PacketIter<CPacket> 
                                PacketCIter;

     static bool          IsCompatibleWith                   (const FeatureDir&           pi_rDir); 
     static CPtr          CreateFrom                         (const FeatureDir*           pi_rpDir);

    const FeatureDir&           GetDir                             () const {
        return *m_pDir;
        }
    FeatureDir&                 GetDir                             () {
        return *m_pDir;
        }

     PacketCIter          PacketBegin                        () const;
     PacketCIter          PacketEnd                          () const;

     bool                 GetFeatures                        (PacketID                    pi_ID,
                                                                    Packet&                     po_rHeaders,
                                                                    Packet&                     po_rPoints) const;
private:
    explicit                    FeaturePacketHandler               (const FeatureDir*           pi_pDir,
                                                                    const FeatureHeaderPacketHandler::CPtr&
                                                                    pi_rpHeaderHandler,
                                                                    const PointPacketHandler::CPtr&
                                                                    pi_rpPointHandler);

    virtual bool                _Save                              () override { return true; }
    virtual bool                _Load                              () override { return true; }

    FeatureDir*                 m_pDir;
    FeatureHeaderPacketHandler::CPtr
    m_pHeaderHandler;
    PointPacketHandler::CPtr    m_pPointHandler;
    };

} //End namespace IDTMFile