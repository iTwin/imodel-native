//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFTgaLineEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
//:Ignore
class HRFTgaFile;
//:End Ignore

/**----------------------------------------------------------------------------
 This class provides the interface to read and write raster data in the correct
 format for Tga file.
-----------------------------------------------------------------------------*/

class HRFTgaLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFTgaFile;

    virtual ~HRFTgaLineEditor  ();

    //:> Edition by Block
    virtual HSTATUS ReadBlock(uint64_t pi_PosBlockX,
                              uint64_t pi_PosBlockY,
                              Byte*   po_pData,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t pi_PosBlockX,
                              uint64_t pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }


    virtual HSTATUS WriteBlock(uint64_t pi_PosBlockX,
                               uint64_t pi_PosBlockY,
                               const Byte*            pi_pData,
                               HFCLockMonitor const*  pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }


protected:
    //:> See the parent for Pointer to the raster file, to the resolution descriptor
    //:> and to the capabilities

    //:> Constructor
    HRFTgaLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                     uint32_t              pi_Page,
                     unsigned short       pi_Resolution,
                     HFCAccessMode         pi_AccessMode);
private:
    //:> Attributes
    HArrayAutoPtr<Byte>       m_pLineBuffer;
    uint32_t                    m_BytesPerLine;
    uint32_t                    m_RasterDataOffset;
    Byte                       m_AlphaChannelBits;

    //:> Methods Disabled
    HRFTgaLineEditor(const HRFTgaLineEditor& pi_rObj);
    HRFTgaLineEditor& operator=(const HRFTgaLineEditor& pi_rObj);

    };
END_IMAGEPP_NAMESPACE

