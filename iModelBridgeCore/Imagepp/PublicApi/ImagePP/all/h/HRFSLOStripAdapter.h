//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFSLOStripAdapter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFSLOStripAdapter
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HGF2DTranslation.h"
#include "HFCMacros.h"
#include "HFCAccessMode.h"
#include "HFCURL.h"
#include "HRFMacros.h"
#include "HRFRasterFileExtender.h"

#include "HRFCombinedRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFRasterFileSLOAdapterCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFRasterFileSLOAdapterCapabilities(const HFCPtr<HRFRasterFileCapabilities>& pi_rpCapabilities);
    };


class HRFSLOStripAdapter : public HRFRasterFileExtender
    {
public:
    friend class HRFSLOStripEditor;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFSLOStripAdapterId, HRFRasterFileExtender)

    // allow to Open an image file
    HRFSLOStripAdapter(HFCPtr<HRFRasterFile>&  pi_rpAdaptedFile);

    virtual         ~HRFSLOStripAdapter();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities       () const;

    // File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    // File manipulation
    virtual bool   AddPage(HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*
    CreateResolutionEditor(uint32_t pi_Page,
                           unsigned short pi_Resolution,
                           HFCAccessMode pi_AccessMode);

    virtual void    Save();

    // This methods allow to know if the file need the adapter
    IMAGEPP_EXPORT static bool NeedSLOAdapterFor(HFCPtr<HRFRasterFile> const& pi_rpForRasterFile);

    // This methods allow to Create the best adapter for the specified raster file.
    // If it is not possible to adapt the raster file we return the original raster file.
    IMAGEPP_EXPORT static HFCPtr<HRFRasterFile>
    CreateBestAdapterFor(HFCPtr<HRFRasterFile> pi_rpForRasterFile);

    // This methods allow to if it is possible to adapt this raster file.
    static bool    CanAdapt(HFCPtr<HRFRasterFile> pi_rpFromRasterFile);



protected:

    // Adapted
    HFCPtr<HRFRasterFileCapabilities>
    m_pRasterFileCapabilities;

    // Methods
    virtual bool   Open                ();
    virtual void    CreateDescriptors   ();


private:

    void            Close();
    void            SynchronizeFiles();

    // Create the file
    bool           Create();

    HFCPtr<HGF2DTransfoModel>
    CreateSLOTransfoModel(HRFScanlineOrientation  pi_ScanlineOrientation,
                          uint32_t pi_RasterPhysicalWidth,
                          uint32_t pi_RasterPhysicalHeight);

    // Methods Disabled
    HRFSLOStripAdapter(const HRFSLOStripAdapter& pi_rObj);
    HRFSLOStripAdapter&  operator=(const HRFSLOStripAdapter& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

