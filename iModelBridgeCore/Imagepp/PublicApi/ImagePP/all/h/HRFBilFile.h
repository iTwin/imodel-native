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

#define RPIX_IDENTIFIER "RPIX"

#define RPIX_HEADERLENGTH 30

#define RPIX_MAJOR_VERSION 1
#define RPIX_MINOR_VERSION 0

#define RPIX_COMPRESSION_UNDEFINED    0
#define RPIX_COMPRESSION_NONE        1
#define RPIX_COMPRESSION_DEFAULT    RPIX_COPRESSION_NONE

#define RPIX_PIXEL_ORDER_UNDEFINED    0
#define RPIX_PIXEL_ORDER_NORMAL        1
#define RPIX_PIXEL_ORDER_REVERSE    2
#define RPIX_PIXEL_ORDER_DEFAULT    RPIX_PIXEL_ORDER_NORMAL

#define RPIX_SCANLINE_ORDER_UNDEFINED    0
#define RPIX_SCANLINE_ORDER_NORMAL        1
#define RPIX_SCANLINE_ORDER_INVERSE        2
#define RPIX_SCANLINE_ORDER_DEFAULT        RPIX_SCANLINE_ORDER_NORMAL

#define RPIX_INTERLEAVING_BIL        2

#define RPIX_CHANNEL_UNDEFINED        0



BEGIN_IMAGEPP_NAMESPACE

typedef struct RawPixelFileHeader
    {
    Byte identifier[4];    /* Always "RPIX" */
    uint32_t hdrlength;    /* Length of this header in bytes    */

    Byte majorversion;        /* Major revision # of RPIX format    */
    Byte minorversion;        /* Minor revision # of RPIX format    */

    uint32_t width;            /* Image width in pixels                    */
    uint32_t height;            /* Image height in pixels                    */
    Byte comptype;            /* Compression (none, FAXG3, FAXG4, ... )    */
    Byte pixelorder;        /* Pixel order                                */
    Byte scnlorder;        /* Scanline order                            */
    Byte interleave;        /* Interleaving (BIP/BIL/Planar)            */

    Byte numbands;            /* Number of bands in image (1-255) */
    Byte rchannel;            /* Default red channel assignment    */
    Byte gchannel;            /* Default green channel assignment */
    Byte bchannel;            /* Default blue channel assignment    */

    Byte reserved[8];        /* For later use */
    } RawPixelFileHeader;

typedef struct BilFileInfo
    {
    uint32_t NbBitsPerBandPerPixel;
    uint32_t NbBytesPerBand;
    uint32_t NbBytesPerRow;
    uint32_t NbBandGapBytes;
    bool   IsMsByteFirst;

    bool   HaveRedStats;
    int32_t RedMinValue;
    int32_t RedMaxValue;
    double RedMeanValue;
    double RedStdDeviation;

    bool   HaveGreenStats;
    int32_t GreenMinValue;
    int32_t GreenMaxValue;
    double GreenMeanValue;
    double GreenStdDeviation;

    bool   HaveBlueStats;
    int32_t BlueMinValue;
    int32_t BlueMaxValue;
    double BlueMeanValue;
    double BlueStdDeviation;
    } BilFileInfo;

typedef struct BilGeoRefInfo
    {
    double  m_A00;
    double  m_A01;
    double  m_A10;
    double  m_A11;
    double  m_Tx;
    double  m_Ty;
    } BilGeoRefInfo;

class HRFBilCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFBilCapabilities();

    };

class HRFBilFile : public HRFRasterFile
    {
public:
    HDECLARE_CLASS_ID(HRFFileId_Bil, HRFRasterFile)

    friend class HRFBilLineEditor;
    friend struct HRFBilCreator;
    // allow to Open or to create an empty file
    HRFBilFile       (const HFCPtr<HFCURL>& pi_rpURL,
                      HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                      uint64_t             pi_Offset = 0);

    virtual                               ~HRFBilFile();

    // File capabilities
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities () const override;

    // File information
    const HGF2DWorldIdentificator GetWorldIdentificator () const override;

    // File manipulation
    bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage) override;

    HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 uint16_t           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode) override;


    int32_t GetRedChannel       () const;
    int32_t GetGreenChannel     () const;
    int32_t GetBlueChannel      () const;
    int32_t GetBandNumber       () const;
    int32_t GetHeaderLength     () const;
    int32_t GetTotalRowBytes    () const;
    int32_t GetBandRowBytes     () const;
    int32_t GetBandGapBytes     () const;
    bool   IsMsByteFirst       () const;

    void                  Save() override;

    uint64_t                GetFileCurrentSize() const override;
protected:
    // Methods
    // Constructor use only to create a child
    HRFBilFile         (const HFCPtr<HFCURL>&  pi_rpURL,
                        HFCAccessMode          pi_AccessMode,
                        uint64_t              pi_Offset,
                        bool                  pi_DontOpenFile);
    virtual bool                       Open();
    bool ReadHdrHeader();
    void  ModifyHdrHeader();
    void  GetBandsFromFile();
    void  GetBandsStatsFromFile();
    bool GetBandStdDeviation  ( const char*     pi_pStats,
                                 int32_t  pi_BandNumber,
                                 int32_t*  po_pMinValue,
                                 int32_t*  po_pMaxValue,
                                 double* po_pMeanValue,
                                 double* po_pStdDeviation);

    int32_t GetLongValueFromHeader(const char* pi_Header, const char* pi_Keyword);
    double GetDoubleValueFromHeader(const char* pi_Header, const char* pi_Keyword);
    bool FileByteOrderMostSignificant(const char* pi_Header);
    HFCPtr<HGF2DTransfoModel> BuildTransfoModel() const;
    void  ReadLine(string* po_pString, HFCBinStream* pi_pFile, bool pi_CleanUpLine, bool pi_RemoveHdrComments);
    bool IsValidGeoRefInfo() const;
    void  GetGeoRefInfo(const char* pi_pHdrHeader);
    bool GetTransfoModel();


private:
    // Members
    HAutoPtr<HFCBinStream>  m_pBilFile;
    HAutoPtr<HFCBinStream>  m_pHdrFile;
    HAutoPtr<HFCBinStream>  m_pBndFile;
    HAutoPtr<HFCBinStream>  m_pStxFile;

    RawPixelFileHeader        m_bilFileHeader;
    BilFileInfo             m_bilFileInfo;
    BilGeoRefInfo           m_geoRefInfo;

    // Methods Disabled
    HRFBilFile(const HRFBilFile& pi_rObj);
    HRFBilFile& operator=(const HRFBilFile& pi_rObj);

    HFCPtr<HRPPixelType>    CreatePixelTypeFromFile() const;
    HRFScanlineOrientation  GetScanLineOrientationFromFile() const;

    void  SetHeaderDoubleValue(string* pi_HeaderLine, const char* pi_Keyword, double pi_Value);
    void  SetHeaderCellsizeValue(string* pi_HeaderLine);
    static bool IsValidChar(const char pi_Char);
    static void  CleanUpString(string* pio_pString);
    };

// Bil Creator.
struct HRFBilCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                  pi_Offset) const override;

    // Identification information
    Utf8String                   GetLabel() const override;
    Utf8String                   GetSchemes() const override;
    Utf8String                   GetExtensions() const override;

    virtual Utf8String GetShortName() const override { return "BIL"; }

    // File format is multi-file
    bool                     GetRelatedURLs(const HFCPtr<HFCURL>& pi_rpURL,
                                                     ListOfRelatedURLs&    pio_rRelatedURLs) const override;
    // capabilities of Raster file.
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() override;

    // allow to Open an image file READ/WRITE and CREATE
    HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t              pi_Offset = 0) const override;

protected:
    bool IsKindOfFileWithExternalHeader(const HFCPtr<HFCURL>& pi_rpURL, uint64_t pi_Offset) const;
    bool IsKindOfFileWithInternalHeader(const HFCPtr<HFCURL>& pi_rpURL, uint64_t pi_Offset) const;

    bool IsKindOfFileOpenFromExternalHeader(const HFCPtr<HFCURL>& pi_rpURL, uint64_t pi_Offset) const;
    uint32_t GetULongValueFromHeader(const char*           pi_Header,
                                   const char*           pi_Keyword,
                                   const HFCPtr<HFCURL>& pi_rpURL) const;
    bool IsValidChar(const char pi_Char)const;
    void  CleanUpString(string* pio_pString)const;

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFBilCreator)


    // Disabled methodes
    HRFBilCreator();
    };

END_IMAGEPP_NAMESPACE

