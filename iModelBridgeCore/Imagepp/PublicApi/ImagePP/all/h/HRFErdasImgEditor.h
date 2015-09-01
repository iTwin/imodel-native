//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFErdasImgEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFGdalSupportedFileEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFErdasImgFile;

class HRFErdasImgEditor : public HRFGdalSupportedFileEditor
    {
public:
    DEFINE_T_SUPER(HRFGdalSupportedFileEditor)

    friend class HRFErdasImgFile;

    virtual ~HRFErdasImgEditor  ();

protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFErdasImgEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                      uint32_t              pi_Page,
                      unsigned short       pi_Resolution,
                      HFCAccessMode         pi_AccessMode);

private:
    // Methods Disabled
    HRFErdasImgEditor(const HRFErdasImgEditor& pi_rObj);
    HRFErdasImgEditor& operator=(const HRFErdasImgEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
