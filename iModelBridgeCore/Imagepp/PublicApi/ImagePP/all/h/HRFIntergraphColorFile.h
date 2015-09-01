//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFIntergraphColorFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------

#pragma once

#include "HRFIntergraphFile.h"

#include "HRFRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFIntergraphColorFile : public HRFIntergraphFile
    {
public:
    friend class HRFIntergraphResolutionEditor;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_IntergraphColor, HRFIntergraphFile)

    // allow to Open an image file
    HRFIntergraphColorFile (const HFCPtr<HFCURL>&  pi_rpURL,
                            HFCAccessMode   pi_AccessMode = HFC_READ_ONLY,
                            uint64_t       pi_Offset = 0);


    virtual         ~HRFIntergraphColorFile ();

    // File manipulation
    virtual bool    AddPage (HFCPtr<HRFPageDescriptor> pi_pPage);

protected:

    // Constructor use only to create a child
    HRFIntergraphColorFile (const HFCPtr<HFCURL>& pi_rpURL,
                            HFCAccessMode         pi_AccessMode,
                            uint64_t             pi_Offset,
                            bool                 pi_DontOpenFile);

    virtual void    GetTransfoModel      ();
    virtual bool   SetGlobalTransfoModel(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel);

private:
    // Methods Disabled
    HRFIntergraphColorFile(const HRFIntergraphColorFile& pi_rObj);
    HRFIntergraphColorFile& operator=(const HRFIntergraphColorFile& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
