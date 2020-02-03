//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFAdaptStripToImage
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
class HRFAdaptStripToImageCapabilities : public HRFBlockAdapterCapabilities
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFAdaptStripToImageCapabilities)

public:
    HRFAdaptStripToImageCapabilities();
    };

//-----------------------------------------------------------------------------
// This is a utility class to create a specific Implementation object.
// There will be an object that derives from this one for each Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------
class HRFAdaptStripToImageCreator : public HRFBlockAdapterCreator
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFAdaptStripToImageCreator)

public:
    virtual ~HRFAdaptStripToImageCreator();
        
    // Obtain the capabilities of stretcher
    virtual HRFBlockAdapterCapabilities* GetCapabilities() const;

    // Creation of implementator
    virtual HRFBlockAdapter*             Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                uint32_t              pi_Page,
                                                uint16_t       pi_Resolution,
                                                HFCAccessMode         pi_AccessMode) const;
    };

//-----------------------------------------------------------------------------
// This specific implementation adapt the storage type to another
//-----------------------------------------------------------------------------
class HRFAdaptStripToImage : public HRFBlockAdapter
    {
public:
    DEFINE_T_SUPER(HRFBlockAdapter)

    // friend class HRFRasterFile;
    HRFAdaptStripToImage(HRFBlockAdapterCapabilities*  pi_pCapabilities,
                         HFCPtr<HRFRasterFile>         pi_rpRasterFile,
                         uint32_t                     pi_Page,
                         uint16_t                pi_Resolution,
                         HFCAccessMode                 pi_AccessMode);

    virtual                ~HRFAdaptStripToImage();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket) override;

    virtual HSTATUS ReadBlockRLE(uint64_t                 pi_PosBlockX,
                                 uint64_t                 pi_PosBlockY,
                                 HFCPtr<HCDPacketRLE>&    po_rpPacketRLE) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const Byte*              pi_pData) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override;

    virtual HSTATUS WriteBlockRLE(uint64_t              pi_PosBlockX,
                                  uint64_t              pi_PosBlockY,
                                  HFCPtr<HCDPacketRLE>& pi_rpPacketRLE) override;

protected:
    uint32_t        m_ExactBytesPerImageWidth;
    uint32_t        m_ImageHeight;
    uint32_t        m_StripHeight;
    uint32_t        m_ExactBytesPerStrip;
    uint32_t        m_NumberOfStripByImageHeight;
    HArrayAutoPtr<Byte> m_pStripsData;

private:
    // Methods Disabled
    HRFAdaptStripToImage(const HRFAdaptStripToImage& pi_rObj);
    HRFAdaptStripToImage& operator=(const HRFAdaptStripToImage& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
