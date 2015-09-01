//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFAdaptLineToStrip.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFAdaptLineToStrip
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HRFBlockAdapter.h"

BEGIN_IMAGEPP_NAMESPACE
class  HRFRasterFile;

//-----------------------------------------------------------------------------
// This specific implementation of this object add
// the supported thing to the list.
//-----------------------------------------------------------------------------
class HRFAdaptLineToStripCapabilities : public HRFBlockAdapterCapabilities
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFAdaptLineToStripCapabilities)

public:
    HRFAdaptLineToStripCapabilities();
    };

//-----------------------------------------------------------------------------
// This is a utility class to create a specific Implementation object.
// There will be an object that derives from this one for each Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------
class HRFAdaptLineToStripCreator : public HRFBlockAdapterCreator
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFAdaptLineToStripCreator)

public:
    // Obtain the capabilities of stretcher
    virtual HRFBlockAdapterCapabilities* GetCapabilities() const;

    // Creation of implementator
    virtual HRFBlockAdapter*             Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                uint32_t              pi_Page,
                                                unsigned short       pi_Resolution,
                                                HFCAccessMode         pi_AccessMode) const;
    };


//-----------------------------------------------------------------------------
// This specific implementation adapt the storage type to another
//-----------------------------------------------------------------------------
class HRFAdaptLineToStrip : public HRFBlockAdapter
    {
public:
    DEFINE_T_SUPER(HRFBlockAdapter)

    HRFAdaptLineToStrip
    (HRFBlockAdapterCapabilities* pi_pCapabilities,
     HFCPtr<HRFRasterFile>        pi_rpRasterFile,
     uint32_t                     pi_Page,
     unsigned short              pi_Resolution,
     HFCAccessMode                pi_AccessMode);

    virtual                ~HRFAdaptLineToStrip();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t              pi_PosBlockX,
                              uint64_t              pi_PosBlockY,
                              Byte*                 po_pData,
                              HFCLockMonitor const* pi_pSisterFileLock = 0);

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }

    virtual HSTATUS ReadBlockRLE(uint64_t              pi_PosBlockX,
                                 uint64_t              pi_PosBlockY,
                                 HFCPtr<HCDPacketRLE>& po_rpPacketRLE,
                                 HFCLockMonitor const* pi_pSisterFileLock = 0);

    virtual HSTATUS WriteBlock(uint64_t              pi_PosBlockX,
                               uint64_t              pi_PosBlockY,
                               const Byte*           pi_pData,
                               HFCLockMonitor const* pi_pSisterFileLock = 0);

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }

    virtual HSTATUS WriteBlockRLE(uint64_t              pi_PosBlockX,
                                  uint64_t              pi_PosBlockY,
                                  HFCPtr<HCDPacketRLE>& pi_rpPacketRLE,
                                  HFCLockMonitor const* pi_pSisterFileLock = 0);

protected:

    // Resolution dimension
    uint32_t                                m_Height;

    // Tile dimension
    uint32_t                                m_BlockHeight;

    // Calc the number of bytes per Image Width
    uint32_t                                m_ExactBytesPerWidth;

private:
    // Methods Disabled
    HRFAdaptLineToStrip(const HRFAdaptLineToStrip& pi_rObj);
    HRFAdaptLineToStrip& operator=(const HRFAdaptLineToStrip& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
