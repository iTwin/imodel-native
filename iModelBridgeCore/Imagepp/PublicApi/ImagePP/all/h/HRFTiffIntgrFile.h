//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFTiffIntgrFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
#include "HRFCacheFileSharing.h"

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

    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities       () const;

    // File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    virtual void                          Save();

protected:

    // Constructor use only to create a child
    //
    HRFTiffIntgrFile    (const HFCPtr<HFCURL>&  pi_rpURL,
                         HFCAccessMode          pi_AccessMode,
                         uint64_t              pi_Offset,
                         bool                  pi_DontOpenFile);

    virtual void                        CreateDescriptors   ();
    HFCPtr<HGF2DTransfoModel>           CreateTransfoModelFromTiffMatrix(double pi_Width,
                                                                         double pi_Height) const;

    virtual bool                       WriteTransfoModel(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel);
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
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                                   uint64_t             pi_Offset = 0) const;
    // Identification information
    virtual WString                   GetLabel() const;
    virtual WString                   GetSchemes() const;
    virtual WString                   GetExtensions() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;


private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFTiffIntgrCreator)

    // Disabled methodes
    HRFTiffIntgrCreator();
    };
END_IMAGEPP_NAMESPACE


