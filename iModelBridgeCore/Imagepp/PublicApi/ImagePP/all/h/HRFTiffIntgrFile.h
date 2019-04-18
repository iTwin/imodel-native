//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFTiffIntgrFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HRFTiffFile.h"
#include <ImagePP/h/HArrayAutoPtr.h>
#include "HFCMacros.h"
#include "HRFException.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFTiffIntgrCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFTiffIntgrCapabilities();
    };


class HRFTiffIntgrFile : public HRFTiffFile
    {
public:
    friend class HRFTiffTileEditor;
    friend class HRFTiffStripEditor;
    friend class HRFiTiffTileEditor;
    friend class HRFiTiffStripEditor;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_TiffIntgr, HRFTiffFile)

    // allow to Open an image file
    HRFTiffIntgrFile      (const HFCPtr<HFCURL>&          pi_rpURL,
                           HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
                           uint64_t                      pi_Offset = 0);

    virtual                               ~HRFTiffIntgrFile          ();

    bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage) override;

    // File capabilities
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities       () const override;

    // File information
    const HGF2DWorldIdentificator GetWorldIdentificator () const override;

    void                          Save() override;

protected:

    // Constructor use only to create a child
    //
    HRFTiffIntgrFile    (const HFCPtr<HFCURL>&  pi_rpURL,
                         HFCAccessMode          pi_AccessMode,
                         uint64_t              pi_Offset,
                         bool                  pi_DontOpenFile);

    void                        CreateDescriptors   () override;
    HFCPtr<HGF2DTransfoModel>           CreateTransfoModelFromTiffMatrix(double pi_Width,
                                                                         double pi_Height) const;

    bool                       WriteTransfoModel(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel) override;
    virtual bool                       WriteTransfoModelToTiffMatrix(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel);

private:
    friend struct HRFTiffIntgrCreator;

    void SaveTiffIntgrFile();

    // Methods Disabled
    HRFTiffIntgrFile(const HRFTiffIntgrFile& pi_rObj);
    HRFTiffIntgrFile& operator=(const HRFTiffIntgrFile& pi_rObj);
    };

// TIFF Creator.
struct HRFTiffIntgrCreator : public HRFRasterFileCreator
    {

    // Opens the file and verifies if it is the right type
    bool                     IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                                   uint64_t             pi_Offset = 0) const override;
    // Identification information
    Utf8String                   GetLabel() const override;
    Utf8String                   GetSchemes() const override;
    Utf8String                   GetExtensions() const override;

    virtual Utf8String GetShortName() const override { return "IntgrTIF"; }

    // capabilities of Raster file.
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() override;

    // allow to Open an image file READ/WRITE and CREATE
    HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const override;


private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFTiffIntgrCreator)

    // Disabled methodes
    HRFTiffIntgrCreator();
    };
END_IMAGEPP_NAMESPACE


