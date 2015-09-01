//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFIntergraphMonochromeFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFIntergraphMonochromeFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------

#pragma once

#include "HRFIntergraphFile.h"

#include "HRFRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFIntergraphMonochromeFile : public HRFIntergraphFile
    {
public:
    friend class HRFIntergraphResolutionEditor;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_IntergraphMonochrome, HRFIntergraphFile)

    // allow to Open an image file
    HRFIntergraphMonochromeFile (const HFCPtr<HFCURL>&  pi_rpURL,
                                 HFCAccessMode   pi_AccessMode = HFC_READ_ONLY,
                                 uint64_t       pi_Offset = 0);


    virtual         ~HRFIntergraphMonochromeFile ();

protected:

    // Constructor use only to create a child
    HRFIntergraphMonochromeFile (const HFCPtr<HFCURL>&  pi_rpURL,
                                 HFCAccessMode          pi_AccessMode,
                                 uint64_t              pi_Offset,
                                 bool                  pi_DontOpenFile);


    virtual void    GetTransfoModel();
    virtual bool   SetGlobalTransfoModel(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel);

private:
    // Methods Disabled
    HRFIntergraphMonochromeFile(const HRFIntergraphMonochromeFile& pi_rObj);
    HRFIntergraphMonochromeFile& operator=(const HRFIntergraphMonochromeFile& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


