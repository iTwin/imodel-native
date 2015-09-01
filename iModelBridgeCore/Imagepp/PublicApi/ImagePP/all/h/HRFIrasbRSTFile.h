//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFIrasbRSTFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFCMacros.h"
#include "HFCAccessMode.h"

#include "HRFRasterFile.h"

BEGIN_IMAGEPP_NAMESPACE
typedef struct RSTSubFileInfo {
    WString fileName;
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
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() const;

    // File information
    virtual const HGF2DWorldIdentificator
    GetWorldIdentificator () const;

    IMAGEPP_EXPORT const list<RSTSubFileInfo>&
    GetFileInfoList () const;

    virtual HRFResolutionEditor*
    CreateResolutionEditor (uint32_t      pi_Page,
                            unsigned short pi_Resolution,
                            HFCAccessMode pi_AccessMode);

    virtual void            Save();

    virtual uint64_t       GetFileCurrentSize() const;

    virtual void            SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                   uint32_t pi_Page = 0,
                                                   bool   pi_CheckSpecificUnitSpec = false,
                                                   bool   pi_InterpretUnitINTGR = false);

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
    virtual bool                 IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                               uint64_t                pi_Offset = 0) const;

    // Identification information
    virtual WString               GetLabel() const;
    virtual WString               GetSchemes() const;
    virtual WString               GetExtensions() const;

    // File format is multi-file
    virtual bool                 GetRelatedURLs(const HFCPtr<HFCURL>& pi_rpURL,
                                                 ListOfRelatedURLs&    pio_rRelatedURLs) const;
    // Capabilities of Raster file
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // Allow to open an image file (read-only)
    virtual HFCPtr<HRFRasterFile> Create(const HFCPtr<HFCURL>& pi_rpURL,
                                         HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                         uint64_t             pi_Offset = 0) const;

    void                          OpenFile(const HFCPtr<HFCURL>& pi_rpURL,
                                           list<RSTSubFileInfo>& po_rListOfRSTSubFileInfo,
                                           uint64_t             pi_Offset = 0) const;

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFIrasbRSTCreator)

    // Disabled methods
    HRFIrasbRSTCreator();
    };
END_IMAGEPP_NAMESPACE


