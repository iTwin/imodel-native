//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFErMapperSupportedFileEditor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

class HRFErMapperSupportedFile;

class HRFErMapperSupportedFileEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFErMapperSupportedFile;

    virtual ~HRFErMapperSupportedFileEditor  ();

    // Edition by block
    virtual HSTATUS                 ReadBlock     (uint32_t     pi_PosBlockX,
                                                   uint32_t     pi_PosBlockY,
                                                   Byte*       po_pData,
                                                   HFCLockMonitor const* pi_pSisterFileLock = 0);
    virtual HSTATUS          ReadBlock     (uint32_t                 pi_PosBlockX,
                                            uint32_t                 pi_PosBlockY,
                                            HFCPtr<HCDPacket>&       po_rpPacket,
                                            HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }

    virtual HSTATUS                 WriteBlock    (uint32_t     pi_PosBlockX,
                                                   uint32_t     pi_PosBlockY,
                                                   const Byte* pi_pData,
                                                   HFCLockMonitor const* pi_pSisterFileLock = 0);
    virtual HSTATUS          WriteBlock    (uint32_t                 pi_PosBlockX,
                                            uint32_t                 pi_PosBlockY,
                                            const HFCPtr<HCDPacket>& pi_rpPacket,
                                            HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }

protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFErMapperSupportedFileEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   unsigned short       pi_Resolution,
                                   HFCAccessMode         pi_AccessMode);

    HSTATUS ReadBlock(uint32_t              pi_PosBlockX,
                      uint32_t              pi_PosBlockY,
                      uint32_t              pi_BlockWidth,
                      uint32_t              pi_BlockHeight,
                      Byte*                po_pData,
                      HFCLockMonitor const* pi_pSisterFileLock);

private:

    unsigned short                 m_ResNb;
    HAutoPtr<HGFTileIDDescriptor>   m_pTileIDDesc;
    HArrayAutoPtr<Byte>            m_LineBuffer;
    HArrayAutoPtr<Byte*>           m_ppLineChannelsBuffers;

    uint32_t                         m_ChannelsQty;

    // Methods Disabled
    HRFErMapperSupportedFileEditor(const HRFErMapperSupportedFileEditor& pi_rObj);
    HRFErMapperSupportedFileEditor& operator=(const HRFErMapperSupportedFileEditor& pi_rObj);
    };

