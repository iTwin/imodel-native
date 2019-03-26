//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFErMapperSupportedFileEditor.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#if defined(IPP_HAVE_ERMAPPER_SUPPORT)  

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFErMapperSupportedFile;

class HRFErMapperSupportedFileEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFErMapperSupportedFile;

    virtual ~HRFErMapperSupportedFileEditor  ();

    // Edition by block
    virtual HSTATUS ReadBlock(uint64_t               pi_PosBlockX,
                              uint64_t               pi_PosBlockY,
                              Byte*                  po_pData) override;

    virtual HSTATUS ReadBlock(uint64_t               pi_PosBlockX,
                              uint64_t               pi_PosBlockY,
                              HFCPtr<HCDPacket>&     po_rpPacket)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }

    virtual HSTATUS WriteBlock(uint64_t               pi_PosBlockX,
                               uint64_t               pi_PosBlockY,
                               const Byte*            pi_pData) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket);
        }

protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFErMapperSupportedFileEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   uint16_t       pi_Resolution,
                                   HFCAccessMode         pi_AccessMode);

    HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                        uint64_t                pi_PosBlockY,
                        uint64_t                pi_BlockWidth,
                        uint64_t                pi_BlockHeight,
                        Byte*                   po_pData);

private:

    uint16_t                 m_ResNb;
    HAutoPtr<HGFTileIDDescriptor>   m_pTileIDDesc;
    HArrayAutoPtr<Byte>            m_LineBuffer;
    HArrayAutoPtr<Byte*>           m_ppLineChannelsBuffers;

    uint32_t                         m_ChannelsQty;

    // Methods Disabled
    HRFErMapperSupportedFileEditor(const HRFErMapperSupportedFileEditor& pi_rObj);
    HRFErMapperSupportedFileEditor& operator=(const HRFErMapperSupportedFileEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

#endif
