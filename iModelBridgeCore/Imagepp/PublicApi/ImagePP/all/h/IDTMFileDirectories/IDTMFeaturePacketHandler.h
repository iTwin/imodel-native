//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/IDTMFileDirectories/IDTMFeaturePacketHandler.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : FeaturePacketHandler
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HTGFFDirectoryHandler.h>

#include <Imagepp/all/h/IDTMFileDirectories/IDTMFeatureDir.h>
#include <Imagepp/all/h/IDTMFileDirectories/IDTMFeatureHeaderPacketHandler.h>
#include <Imagepp/all/h/IDTMFileDirectories/IDTMPointPacketHandler.h>

namespace IDTMFile {

class FeaturePacketHandler : public HTGFF::DirectoryHandler
    {
private:
    class PacketEditor : public HTGFF::PacketEditorBase<const FeaturePacketHandler>
        {
    public:
        _HDLLg PacketID         GetID                              () const;

        _HDLLg size_t           CountPoints                        () const;
        _HDLLg size_t           CountFeatures                      () const;

        _HDLLg bool             GetFeatures                        (Packet&                     po_rHeaders,
                                                                    Packet&                     po_rPoints) const;
        };
public:
    typedef HFCPtr<FeaturePacketHandler>      
                                CPtr;

    typedef const PacketEditor  CPacket;
    typedef HTGFF::PacketIter<CPacket> 
                                PacketCIter;

    _HDLLg static bool          IsCompatibleWith                   (const FeatureDir&           pi_rDir); 
    _HDLLg static CPtr          CreateFrom                         (const FeatureDir*           pi_rpDir);

    const FeatureDir&           GetDir                             () const {
        return *m_pDir;
        }
    FeatureDir&                 GetDir                             () {
        return *m_pDir;
        }

    _HDLLg PacketCIter          PacketBegin                        () const;
    _HDLLg PacketCIter          PacketEnd                          () const;

    _HDLLg bool                 GetFeatures                        (PacketID                    pi_ID,
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