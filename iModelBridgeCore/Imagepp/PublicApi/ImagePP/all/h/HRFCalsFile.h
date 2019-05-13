//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"

#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
class  HRPPixelType;
class  HFCBinStream;
class  HCDCodecCCITTFax4;
class  HFCURL;

#define HRF_CALS_TYPE1_RECORD_SIZE           128
#define HRF_CALS_TYPE1_BLOCK_SIZE           2048

#define HRF_CALS_TYPICAL_READ_BUFFER_SIZE   2048
#define HRF_CALS_CACHED_READ_STROKE            1


class HRFCalsCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFCalsCapabilities();

    };

class HRFCalsFile : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_Cals, HRFRasterFile)

    friend class HRFCalsLineEditor;

    // allow to Open an image file
    HRFCalsFile (const HFCPtr<HFCURL>& pi_rpURL,
                 HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                 uint64_t             pi_Offset = 0);

    virtual        ~HRFCalsFile();

    // File capabilities
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities () const override;

    // File information
    const HGF2DWorldIdentificator GetWorldIdentificator () const override;
    const   HRFScanlineOrientation        GetScanlineOrientation () const;

    HFCPtr<HGF2DTransfoModel>             GetTransfoModel() const;


    // File manipulation
    bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage) override;

    HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 uint16_t           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode) override;


    void                          Save() override;

    uint64_t                     GetFileCurrentSize() const override;

    void                          SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                                 uint32_t pi_Page = 0,
                                                                 bool   pi_CheckSpecificUnitSpec = false,
                                                                 bool   pi_InterpretUnitINTGR = false) override;

protected:
    // Constructor use only to create a child
    HRFCalsFile (const HFCPtr<HFCURL>&  pi_rpURL,
                 HFCAccessMode          pi_AccessMode,
                 uint64_t              pi_Offset,
                 bool                  pi_DontOpenFile);
    virtual bool   Open ();
    virtual void    CreateDescriptors ();

    const   HFCBinStream*
    GetCalsFilePtr () const;

    const  HFCPtr<HCDCodecCCITTFax4>&
    GetCalsCodecPtr () const;

private:


    struct CalsHeaderBlock
        {
        };

    enum CALS_TYPE
        {
        UNKNOWN = 0,
        MIL_28002C,
        MIL_PRF_28002B,
        MIL_R_28002A,
        };

    // Members
    struct CalsHeaderBlock_MIL_28002C : public CalsHeaderBlock
        {
        char SourceDocID [HRF_CALS_TYPE1_RECORD_SIZE];       // Source document Identifier
        char DestDocID   [HRF_CALS_TYPE1_RECORD_SIZE];       // Destination Document ID
        char TextFileID  [HRF_CALS_TYPE1_RECORD_SIZE];       // Text File Identifier
        char FigureID    [HRF_CALS_TYPE1_RECORD_SIZE];       // Table Identifier
        char SourceGraph [HRF_CALS_TYPE1_RECORD_SIZE];       // Source System Filename
        char DocClass    [HRF_CALS_TYPE1_RECORD_SIZE];       // Data file security label
        char RasterType  [HRF_CALS_TYPE1_RECORD_SIZE];       // Raster Data Type
        char Orientation [HRF_CALS_TYPE1_RECORD_SIZE];       // Raster Image Orientation
        char PelCount    [HRF_CALS_TYPE1_RECORD_SIZE];       // Raster Image Pel Count
        char Density     [HRF_CALS_TYPE1_RECORD_SIZE];       // Raster Image Density
        char Notes       [HRF_CALS_TYPE1_RECORD_SIZE];       // Notes
        char Padding     [640];                              // Pad header out to 2048 bytes.
        };

    // Members
    struct CalsHeaderBlock_MIL_PRF_28002B : public CalsHeaderBlock
        {
        char SpecVersion [HRF_CALS_TYPE1_RECORD_SIZE];       // Source document Identifier
        char SourceDocID [HRF_CALS_TYPE1_RECORD_SIZE];       // Source document Identifier
        char DestDocID   [HRF_CALS_TYPE1_RECORD_SIZE];       // Destination Document ID
        char DType       [HRF_CALS_TYPE1_RECORD_SIZE];       // Source System Filename
        char Orientation [HRF_CALS_TYPE1_RECORD_SIZE];       // Raster Image Orientation
        char PelCount    [HRF_CALS_TYPE1_RECORD_SIZE];       // Raster Image Pel Count
        char Density     [HRF_CALS_TYPE1_RECORD_SIZE];       // Raster Image Density
        char Doccls      [HRF_CALS_TYPE1_RECORD_SIZE];       //
        char Notes       [HRF_CALS_TYPE1_RECORD_SIZE];       // Notes
        char Padding     [896];                              // Pad header out to 2048 bytes.
        };

    // Members
    struct CalsHeaderBlock_MIL_R_28002A : public CalsHeaderBlock
        {
        char SpecVersion [HRF_CALS_TYPE1_RECORD_SIZE];       // Source document Identifier
        char SourceDocID [HRF_CALS_TYPE1_RECORD_SIZE];       // Source document Identifier
        char DestDocID   [HRF_CALS_TYPE1_RECORD_SIZE];       // Destination Document ID
        char ModuleID    [HRF_CALS_TYPE1_RECORD_SIZE];       // Text File Identifier
        char DType       [HRF_CALS_TYPE1_RECORD_SIZE];       // Source System Filename
        char Orientation [HRF_CALS_TYPE1_RECORD_SIZE];       // Raster Image Orientation
        char PelCount    [HRF_CALS_TYPE1_RECORD_SIZE];       // Raster Image Pel Count
        char Density     [HRF_CALS_TYPE1_RECORD_SIZE];       // Raster Image Density
        char Didid       [HRF_CALS_TYPE1_RECORD_SIZE];       // Notes
        char Doccls      [HRF_CALS_TYPE1_RECORD_SIZE];       // Notes
        char Notes       [HRF_CALS_TYPE1_RECORD_SIZE];       // Notes
        char Padding     [640];                              // Pad header out to 2048 bytes.
        };

    bool                   m_HasHeaderFilled;
    uint16_t         m_BitPerPixel;
    uint32_t                m_Width;
    uint32_t                m_Height;

    CALS_TYPE               m_CalsType;

    HRFScanlineOrientation  m_ScanlineOrientation;
    HAutoPtr<HFCBinStream>  m_pCalsFile;
    HFCPtr<HRPPixelType>    m_pPixelType;
    HFCPtr<HCDCodecCCITTFax4> m_pCodec;

    CalsHeaderBlock*        m_pCalsHeader;

    // Method
    void            SaveCalFile(bool pi_CloseFile);
    void            InitOpenedFile();

    bool           ConstructSlo();
    bool           ComputeRasterDimension();
    bool           Create();
    bool           CreateFileHeader(HFCPtr<HRFPageDescriptor> pi_pPage);
    bool           CreateHeaderBlock(HRFResolutionDescriptor* pi_pResolutionDescriptor);

    HFCPtr<HRPPixelType>
    GetPixelType() const;

    HRFCalsFile::CALS_TYPE
    GetCalsType();

    HFCPtr<HGF2DTransfoModel> CreateScanLineOrientationModel(HRFScanlineOrientation  scanlineOrientation,
                                                             uint32_t                physicalWidth,
                                                             uint32_t                physicalHeight) const;


    // Methods Disabled
    HRFCalsFile (const HRFCalsFile& pi_rObj);
    HRFCalsFile& operator=(const HRFCalsFile& pi_rObj);
    };

// Cals Creator.
struct HRFCalsCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const override;

    // Identification information
    Utf8String                   GetLabel()      const override;
    Utf8String                   GetSchemes()    const override;
    Utf8String                   GetExtensions() const override;

    virtual Utf8String GetShortName() const override { return "CAL"; }

    // capabilities of Raster file.
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() override;

    // allow to Open an image file READ/WRITE and CREATE
    HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const override;
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFCalsCreator)

    // Disabled methodes
    HRFCalsCreator();
    };
END_IMAGEPP_NAMESPACE


