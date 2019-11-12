//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFCMacros.h"
#include "HFCAccessMode.h"

#include "HRFRasterFile.h"

BEGIN_IMAGEPP_NAMESPACE
typedef struct RSTSubFileInfo {
    BeFileName fileName;
    int32_t layerNumber;
    int32_t color;
    int32_t visibility;
    int32_t views;
    int32_t lock;
    bool operator<(const RSTSubFileInfo& object) const
        {
        return layerNumber < object.layerNumber;
        };
    } RSTSubFileInfo;


/** ---------------------------------------------------------------------------
    This class handle RST raster file operations
    ---------------------------------------------------------------------------
 */
class HRFIrasbRSTFile : public HRFRasterFile
    {
public:
    // Class ID for this class
    HDECLARE_CLASS_ID(HRFFileId_IrasbRST, HRFRasterFile)

    // Allow to open an image file
    IMAGEPP_EXPORT                   HRFIrasbRSTFile        (const HFCPtr<HFCURL>& pi_rpURL,
                                                     HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                                     uint64_t             pi_Offset = 0);

    virtual                 ~HRFIrasbRSTFile       ();

    // Capabilities of raster file
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() const override;

    // File information
    const HGF2DWorldIdentificator
    GetWorldIdentificator () const override;

    IMAGEPP_EXPORT const list<RSTSubFileInfo>&
    GetFileInfoList () const;

    HRFResolutionEditor*
    CreateResolutionEditor (uint32_t      pi_Page,
                            uint16_t pi_Resolution,
                            HFCAccessMode pi_AccessMode) override;

    void            Save() override;

    uint64_t       GetFileCurrentSize() const override;

    void            SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                   uint32_t pi_Page = 0,
                                                   bool   pi_CheckSpecificUnitSpec = false,
                                                   bool   pi_InterpretUnitINTGR = false) override;

private:
    // Methods disabled
    HRFIrasbRSTFile(const HRFIrasbRSTFile& pi_rObj);
    HRFIrasbRSTFile& operator=(const HRFIrasbRSTFile& pi_rObj);

    list<RSTSubFileInfo> m_listOfFileInfo;
    };


/** ---------------------------------------------------------------------------
    This struct handle RST raster file creations
    ---------------------------------------------------------------------------
 */
struct HRFIrasbRSTCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    bool                 IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                               uint64_t                pi_Offset = 0) const override;

    // Identification information
    Utf8String               GetLabel() const override;
    Utf8String               GetSchemes() const override;
    Utf8String               GetExtensions() const override;

    virtual Utf8String GetShortName() const override { return "RST"; }

    // File format is multi-file
    bool                 GetRelatedURLs(const HFCPtr<HFCURL>& pi_rpURL,
                                                 ListOfRelatedURLs&    pio_rRelatedURLs) const override;
    // Capabilities of Raster file
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() override;

    // Allow to open an image file (read-only)
    HFCPtr<HRFRasterFile> Create(const HFCPtr<HFCURL>& pi_rpURL,
                                         HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                         uint64_t             pi_Offset = 0) const override;

    void                          OpenFile(const HFCPtr<HFCURL>& pi_rpURL,
                                           list<RSTSubFileInfo>& po_rListOfRSTSubFileInfo,
                                           uint64_t             pi_Offset = 0) const;

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFIrasbRSTCreator)

    // Disabled methods
    HRFIrasbRSTCreator();
    };
END_IMAGEPP_NAMESPACE


