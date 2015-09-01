//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFDtedFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFDtedFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
#pragma once

#include "HRFGdalSupportedFile.h"
#include "HFCMacros.h"
#include "HFCAccessMode.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFMacros.h"

BEGIN_IMAGEPP_NAMESPACE
class HFCURL;

//--------------------------------------------------
// class HRFDtedCapabilities
//--------------------------------------------------
class HRFDtedCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFDtedCapabilities();
    };


struct HRFDtedCreator : public HRFRasterFileCreator
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

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFDtedCreator)

private:

    // Disabled methods
    HRFDtedCreator();
    };

class HRFDtedEditor;

class HRFDtedFile : public HRFGdalSupportedFile
    {
public:
    friend struct HRFDtedCreator;
    friend class HRFDtedEditor;


    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_Dted, HRFGdalSupportedFile)

    // allow to Open an image file
    HRFDtedFile          (const HFCPtr<HFCURL>&          pi_rpURL,
                          HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
                          uint64_t                      pi_Offset = 0);

    virtual                               ~HRFDtedFile           ();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities       () const;

    // File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    virtual void                          Save();

    virtual void                          SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                                 uint32_t pi_Page = 0,
                                                                 bool   pi_CheckSpecificUnitSpec = false,
                                                                 bool   pi_InterpretUnitINTGR = false);

protected:

    // Protected Methods
    virtual void                        CreateDescriptors() override;

    virtual void                        HandleNoDisplayBands () override;

    virtual void                        DetectOptimalBlockAccess() override;

    virtual HRFScanlineOrientation      GetScanLineOrientation() const override;
    virtual HRPChannelType::ChannelRole GetBandRole(int32_t pi_RasterBand) const override;

private:

    // Methods Disabled
    HRFDtedFile(const HRFDtedFile& pi_rObj);
    HRFDtedFile& operator=(const HRFDtedFile& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


