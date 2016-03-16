//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFImgRGBLineEditor.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

#include "HFCBinStream.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFImgRGBFile;

/** ---------------------------------------------------------------------------
    This class handles ImgRGB raster file I/O operations. Read and write
    operations are performed line by line.

    @see HRFRasterFile
    @see HRFImgRGBLineEditor
    ---------------------------------------------------------------------------
 */
class HRFImgRGBLineEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFImgRGBFile;

    virtual       ~HRFImgRGBLineEditor  ();

    //:> Edition by block
    virtual HSTATUS ReadBlock(uint64_t     pi_PosBlockX,
                              uint64_t     pi_PosBlockY,
                              Byte*        po_pData) override;

    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }

    virtual HSTATUS WriteBlock(uint64_t     pi_PosBlockX,
                               uint64_t     pi_PosBlockY,
                               const Byte*  pi_pData) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket);
        }


protected:
    //:> See the parent for Pointer to the raster file, to the resolution descriptor
    //:> and to the capabilities

    //:> Constructor
    HRFImgRGBLineEditor (HFCPtr<HRFRasterFile> pi_rpRasterFile,
                         uint32_t              pi_Page,
                         uint16_t       pi_Resolution,
                         HFCAccessMode         pi_AccessMode);
private:
    //:> Methods Disabled
    HRFImgRGBLineEditor(const HRFImgRGBLineEditor& pi_rObj);
    HRFImgRGBLineEditor& operator=(const HRFImgRGBLineEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

