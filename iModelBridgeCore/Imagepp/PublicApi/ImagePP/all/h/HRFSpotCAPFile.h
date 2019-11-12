//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HFCAccessMode.h"
#include "HFCURL.h"
#include "HRFMacros.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFRasterFile.h"

BEGIN_IMAGEPP_NAMESPACE
typedef struct SpotCAPFilFileHeader
    {
    uint32_t  SceneDirectoryName;
    uint32_t  SatelliteNumber;
    //TODO : decide if we need more info
    uint32_t  Year;
    uint32_t  Month;
    uint32_t  Day;
    uint32_t  ShiftAlongTrack;

    } SpotCAPFilFileHeader;

typedef struct SpotCAPImgFileHeader
    {
    int32_t  HeaderSize;
    uint32_t NbImageRecords;
    uint32_t NbBitsPerPixel;
    uint32_t NbBands;
    uint32_t ImageWidth;
    uint32_t ImageHeight;
    uint32_t NbLeftBorderPixels;
    uint32_t NbRightBorderPixels;
    uint32_t NbBytesPrefixDataPerRecord;
    uint32_t NbBytesSuffixDataPerRecord;
    int32_t RedChannel;
    int32_t GreenChannel;
    int32_t BlueChannel;
    } SpotCAPImgFileHeader;

typedef struct SpotCAPLeadFileHeader
    {
    uint32_t HeaderSize;
    string  ImageFormatDescription;
    string  SoftwareUsed;
    uint32_t NbHeaderRecords;
    uint32_t HeaderRecLength;
    uint32_t NbAncillaryRecords;
    uint32_t AncillaryRecordLength;
    uint32_t NbAnnotationRecords;
    uint32_t AnnotationRecordLength;

    uint32_t OffsetToHistoValuesPerRecord;

    } SpotCAPLeadFileHeader;

typedef struct SpotCAPVoldFileHeader
    {
    uint32_t RecordSize;
    string  DateOfCreation;
    string  Copyright;

    } SpotCAPVoldFileHeader;

typedef struct GeoRefInfo
    {
    double  m_A00;
    double  m_A01;
    double  m_A10;
    double  m_A11;
    double  m_Tx;
    double  m_Ty;
    } GeoRefInfo;




class HRFSpotCAPCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFSpotCAPCapabilities();

    };

class HRFSpotCAPFile : public HRFRasterFile
    {
public:
    HDECLARE_CLASS_ID(HRFFileId_SpotCAP, HRFRasterFile)

    friend class HRFSpotCAPLineEditor;

    // allow to Open or to create an empty file
    HRFSpotCAPFile      (const HFCPtr<HFCURL>& pi_rpURL,
                         HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                         uint64_t             pi_Offset = 0);

    virtual                                 ~HRFSpotCAPFile();

    // File capabilities
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities () const override;

    // File information
    const HGF2DWorldIdentificator   GetWorldIdentificator () const override;

    // File manipulation
    bool                           AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage) override;

    HRFResolutionEditor*            CreateResolutionEditor(uint32_t                  pi_Page,
                                                                   uint16_t           pi_Resolution,
                                                                   HFCAccessMode             pi_AccessMode) override;

    void                            Save() override;

protected:
    // Methods
    // Constructor use only to create a child
    HRFSpotCAPFile         (const HFCPtr<HFCURL>&  pi_rpURL,
                            HFCAccessMode          pi_AccessMode,
                            uint64_t              pi_Offset,
                            bool                  pi_DontOpenFile);

    virtual bool                        Open();
    virtual void                        CreateDescriptors();
    uint32_t                            GetNbBands() const;
    uint32_t                            GetImageWidth() const;
    uint32_t                            GetImageHeight() const;
    int32_t                             GetHeaderSize() const;
    uint32_t                            GetNbRightBorderPixels() const;
    uint32_t                            GetNbBytesPrefixDataPerRecord() const;
    uint32_t                            GetNbBytesSuffixDataPerRecord() const;
    uint32_t                            GetTotalBytesPerRow() const;




private:

    vector<Utf8String>                     m_SceneNumbers;
    HAutoPtr<HFCBinStream>              m_pFilFile;
    HAutoPtr<HFCBinStream>              m_pImagFile;
    HAutoPtr<HFCBinStream>              m_pLeadFile;
    HAutoPtr<HFCBinStream>              m_pVoldFile;
    SpotCAPFilFileHeader                m_FilHeader;
    SpotCAPImgFileHeader                m_ImgHeader;
    SpotCAPLeadFileHeader               m_LeadHeader;
    SpotCAPVoldFileHeader               m_VoldHeader;
    bool                               m_IsFilHeader;
    GeoRefInfo                          m_GeoRefInfo;
    HArrayAutoPtr<double>              m_pTiePoints;

    // Methods Disabled
    HRFSpotCAPFile(const HRFSpotCAPFile& pi_rObj);
    HRFSpotCAPFile& operator=(const HRFSpotCAPFile& pi_rObj);

    void                                Initialize();
    HFCPtr<HRPPixelType>                CreatePixelTypeFromFile() const;
    const HFCPtr<HRPHistogram>          GetHistogramFromFile()const;
    HRFScanlineOrientation              GetScanLineOrientation() const;
    HFCPtr<HGF2DTransfoModel>           CreateTransfoModel()const;
    bool                               ReadFilHeader();
    bool                               ReadImagHeader();
    bool                               ReadLeadHeader();
    bool                               ReadVoldHeader();
    bool                               IsFilHeader();
    int32_t                            GetRedChannel() const;
    int32_t                            GetGreenChannel() const;
    int32_t                            GetBlueChannel() const;
    bool                               Create();
    void                                SetDataTiePoints();

    //Utility
    bool                               IsValidChar(const char pi_Char) const;
    void                                CleanUpString(string* pio_pString) const;
    double                             ConvertStringToRadian(string* pio_pString) const;





    };

// Spot CAP Creator.
struct HRFSpotCAPCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    bool                           IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                         uint64_t                  pi_Offset) const override;

    // Identification information
    Utf8String                         GetLabel() const override;
    Utf8String                         GetSchemes() const override;
    Utf8String                         GetExtensions() const override;

    virtual Utf8String GetShortName() const override { return "SpotCAP"; }

    // File format is multi-file
    bool                           GetRelatedURLs(const HFCPtr<HFCURL>& pi_rpURL,
                                                           ListOfRelatedURLs&    pio_rRelatedURLs) const override;


    // capabilities of Raster file.
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() override;

    // allow to Open an image file READ/WRITE and CREATE
    HFCPtr<HRFRasterFile>           Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                   HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                                   uint64_t             pi_Offset = 0) const override;

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFSpotCAPCreator)

    // Disabled methodes
    HRFSpotCAPCreator();
    };
END_IMAGEPP_NAMESPACE

