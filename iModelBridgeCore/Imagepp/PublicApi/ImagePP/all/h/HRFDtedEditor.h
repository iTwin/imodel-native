//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFDtedEditor.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#ifdef IPP_HAVE_GDAL_SUPPORT

#include "HRFResolutionEditor.h"
#include "HRFGdalSupportedFileEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFDtedFile;


class HRFDtedEditor : public HRFGdalSupportedFileEditor
    {
public:
    DEFINE_T_SUPER(HRFGdalSupportedFileEditor)

    friend class HRFDtedFile;

    virtual ~HRFDtedEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              Byte*                    po_pData) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }


protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFDtedEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                  uint32_t              pi_Page,
                  uint16_t       pi_Resolution,
                  HFCAccessMode         pi_AccessMode);

private:
    // Methods Disabled
    HRFDtedEditor(const HRFDtedEditor& pi_rObj);
    HRFDtedEditor& operator=(const HRFDtedEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
#endif
