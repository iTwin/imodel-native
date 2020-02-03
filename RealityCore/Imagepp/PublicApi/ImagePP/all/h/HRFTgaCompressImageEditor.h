//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HRFTgaCompressImageEditor
//---------------------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"
#include "HRFResolutionEditor.h"
#include "HCDCodecTgaRLE.h"

BEGIN_IMAGEPP_NAMESPACE
//:Ignore
class HRFTgaFile;
//:End Ignore

/**----------------------------------------------------------------------------
 This class provides the interface to read and write raster data in a
 compressed tga file. It uses the HCDCodecTGARLE codec to decompress the raster
 data from RLE to raw.

  @see HCDCodecTGARLE
-----------------------------------------------------------------------------*/

class HRFTgaCompressImageEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFTgaFile;

    virtual ~HRFTgaCompressImageEditor  ();

    //:> Edition by block
    virtual HSTATUS ReadBlock(uint64_t pi_PosBlockX,
                              uint64_t pi_PosBlockY,
                              Byte*   po_pData) override;

    HSTATUS ReadBlock(uint64_t pi_PosBlockX,
                              uint64_t pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }


protected:
    //:> See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    //:> Constructor
    HRFTgaCompressImageEditor(HFCPtr<HRFTgaFile>    pi_rpRasterFile,
                              uint32_t              pi_Page,
                              uint16_t       pi_Resolution,
                              HFCAccessMode         pi_AccessMode);

private:

    HSTATUS             ReadCompressedImage(HFCPtr<HCDPacket>& po_rpPacket);

    //:> Attributes
    HFCPtr<HCDCodecTGARLE>  m_pCodec;

    //:> Methods Disabled
    HRFTgaCompressImageEditor(const HRFTgaCompressImageEditor& pi_rObj);
    HRFTgaCompressImageEditor& operator=(const HRFTgaCompressImageEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

