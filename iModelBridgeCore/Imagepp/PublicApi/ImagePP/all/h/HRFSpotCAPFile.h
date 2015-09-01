//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFSpotCAPFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities () const;

    // File information
    virtual const HGF2DWorldIdentificator   GetWorldIdentificator () const;

    // File manipulation
    virtual bool                           AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*            CreateResolutionEditor(uint32_t                  pi_Page,
                                                                   unsigned short           pi_Resolution,
                                                                   HFCAccessMode             pi_AccessMode);

    virtual void                            Save();


    //--------------------------------------
    // Sharing control methods for other header files
    //--------------------------------------

    // Imag
    HFCBinStreamLockManager*      GetImagLockManager                ();
    void                          ImagSharingControlCreate          (HFCURL* pi_pImagUrl);
    bool                         ImagSharingControlNeedSynchronization();
    void                          ImagSharingControlSynchronize     ();
    void                          ImagSharingControlIncrementCount  ();
    bool                         ImagSharingControlIsLocked        ();

    HRFSharingControl*            GetImagSharingControl             ();

    // Lead
    HFCBinStreamLockManager*      GetLeadLockManager                ();
    void                          LeadSharingControlCreate          (HFCURL* pi_pLeadUrl);
    bool                         LeadSharingControlNeedSynchronization();
    void                          LeadSharingControlSynchronize     ();
    void                          LeadSharingControlIncrementCount  ();
    bool                         LeadSharingControlIsLocked        ();

    HRFSharingControl*            GetLeadSharingControl             ();

    // Vold
    HFCBinStreamLockManager*      GetVoldLockManager                ();
    void                          VoldSharingControlCreate          (HFCURL* pi_pVoldUrl);
    bool                         VoldSharingControlNeedSynchronization();
    void                          VoldSharingControlSynchronize     ();
    void                          VoldSharingControlIncrementCount  ();
    bool                         VoldSharingControlIsLocked        ();

    HRFSharingControl*            GetVoldSharingControl             ();


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

    vector<WString>                     m_SceneNumbers;
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



    HAutoPtr<HRFSharingControl>  m_pImagSharingControl;
    HAutoPtr<HRFSharingControl>  m_pLeadSharingControl;
    HAutoPtr<HRFSharingControl>  m_pVoldSharingControl;


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
    virtual bool                           IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                         uint64_t                  pi_Offset) const;

    // Identification information
    virtual WString                         GetLabel() const;
    virtual WString                         GetSchemes() const;
    virtual WString                         GetExtensions() const;

    // File format is multi-file
    virtual bool                           GetRelatedURLs(const HFCPtr<HFCURL>& pi_rpURL,
                                                           ListOfRelatedURLs&    pio_rRelatedURLs) const;


    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>           Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                   HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                                   uint64_t             pi_Offset = 0) const;

    void                                    ImagSharingControlCreate    (const HFCPtr<HFCURL>& pi_pURL);
    HRFSharingControl*                      GetImagSharingControl       () const;
    HFCBinStreamLockManager*                GetImagLockManager          () const;


protected:

    HAutoPtr<HRFSharingControl>  m_pImagSharingControl;

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFSpotCAPCreator)

    // Disabled methodes
    HRFSpotCAPCreator();
    };
END_IMAGEPP_NAMESPACE

