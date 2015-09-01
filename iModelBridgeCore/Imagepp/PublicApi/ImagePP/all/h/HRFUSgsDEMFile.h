//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFUSgsDEMFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFUSgsDEMFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
#pragma once

#include "HFCAccessMode.h"
#include "HFCBinStream.h"
#include "HFCMacros.h"
#include "HFCURL.h"
#include "HRFGdalSupportedFile.h"
#include "HRFMacros.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFUSgsDEMEditor.h"
#include "HTIFFTag.h"
#include "HCPGeoTiffKeys.h"

//--------------------------------------------------
// class HRFUSgsDEMCapabilities
//--------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
class HRFUSgsDEMCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFUSgsDEMCapabilities();
    };


struct HRFUSgsDEMCreator : public HRFRasterFileCreator
    {

    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const;

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

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFUSgsDEMCreator)

private:

    // Disabled methods
    HRFUSgsDEMCreator();
    };

class HRFUSgsDEMFile : public HRFGdalSupportedFile
    {
public:
    friend HRFUSgsDEMCreator;
    friend HRFUSgsDEMEditor;


    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_USgsDEM, HRFGdalSupportedFile)

    // allow to Open an image file
    HRFUSgsDEMFile          (const HFCPtr<HFCURL>&          pi_rpURL,
                             HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
                             uint64_t                      pi_Offset = 0);

    virtual                             ~HRFUSgsDEMFile           ();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities       () const;

    // File manipulation
    virtual bool                        AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*        CreateResolutionEditor(uint32_t                  pi_Page,
                                                               unsigned short           pi_Resolution,
                                                               HFCAccessMode             pi_AccessMode);

    virtual RasterFileGeocodingPtr      ExtractGeocodingInformation();

    virtual void                        Save();

protected:

    // Protected Methods
    virtual void                        CreateDescriptors() override;

    virtual void                        HandleNoDisplayBands () override;

    virtual HRFScanlineOrientation      GetScanLineOrientation() const override;
    virtual HRPChannelType::ChannelRole GetBandRole(int32_t pi_RasterBand) const override;

private:

    // Methods Disabled
    HRFUSgsDEMFile(const HRFUSgsDEMFile& pi_rObj);
    HRFUSgsDEMFile& operator=(const HRFUSgsDEMFile& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
