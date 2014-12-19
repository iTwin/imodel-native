//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/IDTMFileDirectories/IDTMPointPacketHandler.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : PointPacketHandler
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HTGFFDirectoryHandler.h>

#include <Imagepp/all/h/IDTMFileDirectories/IDTMPointDir.h>
#include <Imagepp/all/h/IDTMFileDirectories/IDTMFeatureDir.h>
#include <Imagepp/all/h/HTGFFPacketIter.h>

namespace IDTMFile {

class PointPacketHandler : public HTGFF::DirectoryHandler
    {
private:
    class PacketEditor : public HTGFF::PacketEditorBase<const PointPacketHandler>
        {
    public:
        _HDLLg PacketID         GetID                              () const;
        _HDLLg size_t           CountPoints                        () const;
        _HDLLg bool             GetPoints                          (Packet& po_rPoints) const;
        };
public:
    typedef HFCPtr<PointPacketHandler>      
                                CPtr;

    typedef const PacketEditor  CPacket;
    typedef HTGFF::PacketIter<CPacket> 
                                PacketCIter;

    const PointDir&             GetDir                             () const {return *m_pDir;}   
    PointDir&                   GetDir                             () {return *m_pDir;}

    _HDLLg static bool          IsCompatibleWith                   (const PointDir&             pi_rDir); 
    _HDLLg static bool          IsCompatibleWith                   (const FeatureDir&           pi_rDir); 

    _HDLLg static CPtr          CreateFrom                         (const PointDir*             pi_rpDir);
    _HDLLg static CPtr          CreateFrom                         (const FeatureDir*           pi_rpDir);


    _HDLLg PacketCIter          PacketBegin                        () const;
    _HDLLg PacketCIter          PacketEnd                          () const;

    _HDLLg bool                 GetPoints                          (PacketID                    pi_ID,
                                                                    Packet&                     po_rPoints) const;

private:
    explicit                    PointPacketHandler                 (const PointDir*             pi_rpDir);


    virtual bool                _Save                              () override { return true; }
    virtual bool                _Load                              () override { return true; }

    PointDir*                   m_pDir;
    };


} //End namespace IDTMFile