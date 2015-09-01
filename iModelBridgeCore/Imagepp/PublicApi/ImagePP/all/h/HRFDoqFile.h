//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFDoqFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFDoqFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
#pragma once

#include "HFCMacros.h"
#include "HFCAccessMode.h"
#include "HFCURL.h"
#include "HFCBinStream.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFDoqEditor.h"
#include "HRFMacros.h"
#include "HTIFFTag.h"

BEGIN_IMAGEPP_NAMESPACE
//--------------------------------------------------
// Keywords
//--------------------------------------------------
enum KeywordName {  BITS_PER_PIXEL,
                    SAMPLES_AND_LINES,
                    DATA_FILE_SIZE,
                    BYTE_COUNT,
                    BAND_ORGANIZATION,
                    BAND_CONTENT,
                    BEGIN,
                    END,
                    RASTER_ORDER,
                    H_RESOLUTION,
                    H_RESOLUTION_UNITS,
                    XY_ORIGIN,
                    HORIZONTAL_DATUM,
                    COORDINATE_ZONE,
                    HORIZONTAL_COORDINATE_SYSTEM
                 };


#define DOQ_HEADER_LINE_LENGTH 80





//--------------------------------------------------
// class HRFDoqCapabilities
//--------------------------------------------------
class HRFDoqCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFDoqCapabilities();
    };

// Doq Creator.
struct HRFDoqCreator : public HRFRasterFileCreator
    {
    friend class HRFDoqFile;
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



    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFDoqCreator)
private:



    // Disabled methods
    HRFDoqCreator();
    };



class HRFDoqFile : public HRFRasterFile
    {
public:
    friend struct HRFDoqCreator;
    friend class HRFDoqEditor;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_Doq, HRFRasterFile)

    // allow to Open an image file
    HRFDoqFile          (const HFCPtr<HFCURL>&          pi_rpURL,
                         HFCAccessMode                 pi_AccessMode = HFC_READ_ONLY,
                         uint64_t                     pi_Offset = 0);

    virtual                               ~HRFDoqFile           ();

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

    virtual void                          SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                                 uint32_t pi_Page = 0,
                                                                 bool   pi_CheckSpecificUnitSpec = false,
                                                                 bool   pi_InterpretUnitINTGR = false);

    // Constructor use only to create a child
    //
    HRFDoqFile(const HFCPtr<HFCURL>&  pi_rpURL,
               HFCAccessMode          pi_AccessMode,
               uint64_t              pi_Offset,
               bool                  pi_DontOpenFile);

    virtual bool           Open();
    virtual void            GetHeader();
    virtual void            ReadHeader();
    virtual void            ReadBufferLine() const;
    virtual void            CreateDescriptors();
    virtual void            CreatePixelType();
    virtual void            MapHeader();
    bool                   GetGeoRefInfo();

    //access
    uint32_t                GetHeaderSize()const;
    uint32_t                GetBitsPerPixel()const;
    HFCBinStream*           GetFilePtr() const;
    uint32_t                GetNbBands()const;
    uint32_t                GetImageWidth()const;
    uint32_t                GetImageHeight()const;
    size_t                  GetTotalRowBytes() const;
    bool                   GetKeywordLine(KeywordName pi_Keyword)const;
    bool                   GetField( KeywordName pi_Keyword, uint32_t pi_NbArgs, unsigned short* po_pReturnVal)const;
    string                  GetField( KeywordName pi_Keyword, uint32_t pi_NbArgs)const;
    bool                   GetField( KeywordName pi_Keyword, uint32_t pi_NbArgs, double* po_pReturnVal)const;
    bool                   GetField( KeywordName pi_Keyword, uint32_t pi_NbArgs, uint32_t* po_pReturnVal)const;
    string                  GetFieldString(int pi_Offset, uint32_t pi_ArgNb)const;

    bool                   IsValidGeoRefInfo() const;
    HFCPtr<HGF2DTransfoModel> BuildTransfoModel() const;
    double                 CalculateFactorModelToMeter() const;
    HRFScanlineOrientation  GetScanLineOrientation()const;




private:
    typedef struct DoqFileHeaderLine
        {
        const char*    m_Keyword;
        uint32_t         m_NbArgs;
        bool           m_MultipleKeyword;      //this keyword can be found more than once in the header
        uint32_t         m_NextLineToRead;
        } DoqFileHeaderLine;

    typedef struct DoqGeoRefInfo
        {
        double  m_A00;
        double  m_A01;
        double  m_A10;
        double  m_A11;
        double  m_Tx;
        double  m_Ty;
        } DoqGeoRefInfo;

    typedef map<KeywordName, DoqFileHeaderLine> KeywordMap;

    // Attributs
    uint32_t                           m_ImageWidth;
    uint32_t                           m_ImageHeight;
    unsigned short                      m_BitsPerPixel;
    double                              m_RatioToMeter;
    HRFScanlineOrientation              m_SLO;
    uint32_t                           m_DataSize;
    uint32_t                           m_HeaderSize;
    vector<string>                      m_BandContent;

    HAutoPtr<HFCBinStream>              m_pDoqFile;
    HFCPtr<HRPPixelType>                m_pPixelType;
    mutable HAutoPtr<char>              m_pLine;
    HAutoPtr<char>                      m_pHeaderBuffer;
    mutable uint64_t                    m_CurrentBufferPos;


    DoqGeoRefInfo                       m_DoqGeoRefInfo;

    KeywordMap                          m_KeywordMap;

    // Methods
    HRFScanlineOrientation              TranslateScanlineOrientation(const string& pi_rString);


    // Methods Disabled

    HRFDoqFile(const HRFDoqFile& pi_rObj);
    HRFDoqFile& operator=(const HRFDoqFile& pi_rObj);
    };

END_IMAGEPP_NAMESPACE


