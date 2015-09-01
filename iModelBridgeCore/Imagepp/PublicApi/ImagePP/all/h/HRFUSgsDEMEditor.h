//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFUSgsDEMEditor.h $
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

class HRFUSgsDEMEditor : public HRFGdalSupportedFileEditor
    {
public:
    DEFINE_T_SUPER(HRFGdalSupportedFileEditor)

    friend class HRFUSgsDEMFile;

    virtual ~HRFUSgsDEMEditor  ();

protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFUSgsDEMEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                     uint32_t              pi_Page,
                     unsigned short       pi_Resolution,
                     HFCAccessMode         pi_AccessMode);

private:
    // Methods Disabled
    HRFUSgsDEMEditor(const HRFUSgsDEMEditor& pi_rObj);
    HRFUSgsDEMEditor& operator=(const HRFUSgsDEMEditor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
