//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFNitfEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"
#include "HRFGdalSupportedFileEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFNitfFile;

class HRFNitfEditor : public HRFGdalSupportedFileEditor
    {
public:
    DEFINE_T_SUPER(HRFGdalSupportedFileEditor)

    friend class HRFNitfFile;

    virtual ~HRFNitfEditor  ();

protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFNitfEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                  uint32_t              pi_Page,
                  unsigned short       pi_Resolution,
                  HFCAccessMode         pi_AccessMode);

    void Masking8BitsBlock(Byte* po_pData);

private:

    uint32_t m_Mask;

    // Methods Disabled
    HRFNitfEditor(const HRFNitfEditor& pi_rObj);
    HRFNitfEditor& operator=(const HRFNitfEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
