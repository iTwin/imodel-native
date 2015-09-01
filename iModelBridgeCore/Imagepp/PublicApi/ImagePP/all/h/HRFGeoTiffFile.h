//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFGeoTiffFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFGeoTiffFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HRFTiffFile.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"
#include "HCPGeoTiffKeys.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFGeoTiffCapabilities : public HRFTiffCapabilities
    {
public:
    HRFGeoTiffCapabilities();

    };


class HRFGeoTiffFile : public HRFTiffFile
    {
public:
    friend class HRFTiffTileEditor;
    friend class HRFTiffStripEditor;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_GeoTiff, HRFTiffFile)

    // Allow to Open an image file
    HRFGeoTiffFile        (const HFCPtr<HFCURL>&  pi_rpURL,
                           HFCAccessMode          pi_AccessMode = HFC_READ_ONLY,
                           uint64_t              pi_Offset = 0);

    virtual                               ~HRFGeoTiffFile       ();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities       () const;

    // File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    // File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    virtual void                          Save();

    virtual uint64_t                     GetFileCurrentSize() const;

    // Specific GeoTiff Method
    virtual void                          SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                                 uint32_t pi_Page = 0,
                                                                 bool   pi_CheckSpecificUnitSpec = false,
                                                                 bool   pi_InterpretUnitINTGR = false);

    IMAGEPP_EXPORT void                           GetDefaultInterpretationGeoRef(double* po_RatioToMeter=0,
                                                                         bool*   po_InterpretUnit=0,
                                                                         bool*   po_InterpretUnitINTGR=0);

protected:

    // Methods

    // Constructor use only to create a child
    //
    HRFGeoTiffFile        (const HFCPtr<HFCURL>&  pi_rpURL,
                           HFCAccessMode          pi_AccessMode,
                           uint64_t              pi_Offset,
                           bool                  pi_DontOpenFile);

    virtual bool                         Open                  (bool pi_CreateBigTifFormat=false);
    virtual bool                         Open                  (const HFCPtr<HFCURL>&  pi_rpURL);
    virtual void                          CreateDescriptors     ();
    virtual bool                         WriteTransfoModel(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel) {
        return T_Super::WriteTransfoModel(pi_rpTransfoModel);
        }

    virtual bool                         WriteTransfoModel(HCPGeoTiffKeys const*                pi_rpGeoTiffKeys,
                                                           const HFCPtr<HGF2DTransfoModel>&     pi_rpTransfoModel,
                                                           uint32_t                            pi_Page);
    virtual void                          StoreUsingModelTransformationTag ();

private:
    friend struct HRFGeoTiffCreator;

    // Members
    bool                       m_StoreUsingMatrix;
    bool                       m_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation;
    bool                       m_DefaultCoordSysIsIntergraphIfUnitNotResolved;
    // Ratio apply to the transfo model to put bak the units in meter.
    double                     m_RatioToMeter;

    // Methods
    HFCPtr<HGF2DTransfoModel>   CreateTransfoModelFromGeoTiff (HCPGeoTiffKeys const*                pi_rpGeoTiffKeys,
                                                               uint32_t                            pi_PageNb);
    void                        WriteTransfoModelFromGeoTiff  (HCPGeoTiffKeys const*                pi_rpGeoTiffKeys,
                                                               const HFCPtr<HGF2DTransfoModel>&     pi_pModel,
                                                               uint32_t                            pi_Page);
    void                        GetGeoTiffKeys                (HFCPtr<HCPGeoTiffKeys>&              po_rpGeoTiffKeys);
    void                        SaveGeoTiffFile();



    // Methods Disabled
    HRFGeoTiffFile(const HRFGeoTiffFile& pi_rObj);
    HRFGeoTiffFile& operator=(const HRFGeoTiffFile& pi_rObj);
    };

// GeoTiff Creator.
struct HRFGeoTiffCreator : public HRFTiffCreator
    {
    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const;

    // Identification information
    virtual WString                   GetLabel() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;

protected :
    // Disabled methods
    HRFGeoTiffCreator();

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFGeoTiffCreator)


    };
END_IMAGEPP_NAMESPACE

