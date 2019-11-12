//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

#include "HFCBinStream.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFImgMappedFile;

/** ---------------------------------------------------------------------------
    This class handles ImgMapped raster file I/O operations. Read and write
    operations are performed line by line.

    @see HRFRasterFile
    @see HRFImgMappedFile
    ---------------------------------------------------------------------------
 */
class HRFImgMappedLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFImgMappedFile;

    virtual       ~HRFImgMappedLineEditor  ();

    //:> Edition by block
    virtual HSTATUS ReadBlock(uint64_t    pi_PosBlockX,
                              uint64_t    pi_PosBlockY,
                              Byte*       po_pData) override;

    HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }


    virtual HSTATUS WriteBlock(uint64_t       pi_PosBlockX,
                               uint64_t       pi_PosBlockY,
                               const Byte*    pi_pData) override;

    HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket);
        }



protected:
    //:> See the parent for Pointer to the raster file, to the resolution descriptor
    //:> and to the capabilities

    //:> Constructor
    HRFImgMappedLineEditor (HFCPtr<HRFRasterFile> pi_rpRasterFile,
                            uint32_t              pi_Page,
                            uint16_t       pi_Resolution,
                            HFCAccessMode         pi_AccessMode);
private:
    //:> Methods Disabled
    HRFImgMappedLineEditor(const HRFImgMappedLineEditor& pi_rObj);
    HRFImgMappedLineEditor& operator=(const HRFImgMappedLineEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
