//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFiTiffFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFiTiffFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
#pragma once

#include "HFCMacros.h"
#include "HRFTiffFile.h"
#include "HRFRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFiTiffCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFiTiffCapabilities();
    };


class HRFiTiffFile : public HRFTiffFile
    {
public:
    friend class HRFiTiffTileEditor;
    friend class HRFiTiffLuraWavePaddedTileEditor;
    friend class HRFiTiffLuraWaveNonPaddedTileEditor;
    friend class HRFiTiffStripEditor;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_iTiff, HRFTiffFile)

    // allow to Open an image file
    HRFiTiffFile          (const HFCPtr<HFCURL>&          pi_rpURL,
                           HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
                           uint64_t                       pi_Offset = 0);

    virtual                               ~HRFiTiffFile           ();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&     GetCapabilities       () const;

    // File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    // File manipulation
    virtual bool                          AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    virtual void                         Save();

    // AltaPhoto project
    bool                                 Read_AltaPhotoBlob    (vector<Byte>* po_pData) const;
    bool                                 Write_AltaPhotoBlob   (const vector<Byte>& pi_pData);

    virtual    uint64_t                  GetFileCurrentSize() const;

    virtual void       SetDefaultRatioToMeter(double pi_RatioToMeter,
                                              uint32_t pi_Page = 0,
                                              bool   pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation = false,
                                              bool   pi_InterpretUnitINTGR = false) override;


protected:

    typedef struct tagHMRHeader
        {
        bool                   m_HMRDirDirty;
        // File version
        //  HMR_VERSION_TILE_SLO4(default) or HMR_VERSION_TILE
        uint32_t                m_Version;
        uint32_t                m_MinorVersion;

        // String describes the System Coordinate.
        char                   m_SystemCoord[HMR_LgStringSystemCoord+1];

        // Histogramm
        char                   m_HistoDateTime[HMR_LgStringDateTime+1];
        HArrayAutoPtr<uint32_t>   m_pHistogram;
        uint32_t                m_HistogramLength;

        // Padding line at the top if the Version is HMR_VERSION_TILEPADDING
        // Always = 0 because this version is not supported
        uint32_t                m_PaddingLines;

        // Shape information
        bool                   m_HMRClipShapeInFile;
        int32_t                m_HMRClipShapeLength;
        HArrayAutoPtr<double>  m_pHMRClipShape;

        // TileFlag info
        // All tileFlags for all resolution...
        // see at the definition of TIFFTAG_iTiff_TILEFLAG in
        // HTiffTag.h for the specification.
        uint32_t                m_iTiffTileFlagsLength;
        HArrayAutoPtr<Byte>   m_piTiffTileFlags;

        // Decimation method used for sub-resolution.
        bool                   m_HasDecimationMethod;
        HArrayAutoPtr<Byte>    m_pDecimationMethod;
        uint32_t                m_DecimationMethodCount;

        // Transparency palette
        HArrayAutoPtr<Byte>   m_pTransPalette;
        size_t                  m_TransPaletteCount;

        // RGB or BGR
        unsigned short         m_HMRPixelTypeSpec;         // see HRFTiffFile.h

        string                  m_SourceFileCreationTime;

        // 3D Matrix Information
        HAutoPtr<HFCMatrix<4, 4>>   m_p3DTranfoMatrix;

        // WellKnownText
        string                      m_WellKnownText;

        // Info on all OnDemandRaster objects of an OnDemandMosaic object
        string                      m_OnDemandRastersInfo;

        ListOfChannelIndex          m_ChannelsWithNoDataValue;
        ListOfChannelNoDataValue    m_ChannelsNoDataValue;


        } HMRHeader;

    HArrayAutoPtr<HAutoPtr<HMRHeader> >  m_ppHMRHeaders;

    // Methods

    // Constructor use only to create a child
    //
    HRFiTiffFile(const HFCPtr<HFCURL>&  pi_rpURL,
                 HFCAccessMode          pi_AccessMode,
                 uint64_t              pi_Offset,
                 bool                  pi_DontOpenFile);

    virtual bool    Open                (bool pi_CreateBigTifFormat=false);
    virtual bool    Open                (const HFCPtr<HFCURL>&  pi_rpURL)   {
        return T_Super::Open(pi_rpURL);
        }


    virtual void     CreateDescriptors();
    virtual void     ReloadDescriptors();
    virtual void     SaveDescriptors(uint32_t pi_Page = -1) override;
    virtual void     InitPrivateTagDefault(HMRHeader* po_rpHMRHeader);
    virtual bool     ReadPrivateDirectory(uint32_t pi_Page, HMRHeader* po_rpHMRHeader);
    virtual void     WritePrivateDirectory(uint32_t pi_Page);

    Byte                    GetDownSamplingMethodCode(HRFDownSamplingMethod pi_DownSamplingMethod) const;
    HRFDownSamplingMethod   GetDownSamplingMethod(Byte pi_Sampling) const;

    void                    SetClipShape    (HMRHeader*             pio_pHMRHeader,
                                             const HRFClipShape&    pi_rShape);
    HRFClipShape*           GetClipShape    (const HMRHeader*       pi_pHMRHeader);
    void                    SetHistogram    (HMRHeader*             pi_pHMRHeader,
                                             const HRPHistogram&    pi_rHistogram);

    void                    SaveiTiffFile();

    bool                    Write3DMatrixToTiffTag(HFCMatrix<4, 4>& pi_r3DMatrix);
    HFCMatrix<4, 4>*        Create3DMatrixFromiTiffTag() const;

    static bool             IsSamePixelTypeForAllResolutions       (HFCPtr<HRFPageDescriptor>       pi_pPage);
    static void             GenerateNoDataValuesTagData            (ListOfChannelIndex*             po_pChannelsWithNoDataValue,
                                                                    ListOfChannelNoDataValue*       po_pChannelsNoDataValue,
                                                                    const HRPChannelOrg&            pi_rChannelOrg);

private:
    friend struct HRFiTiffCreator;


    void                    SaveDescriptor(uint32_t pi_Page);


    // Methods Disabled
    HRFiTiffFile(const HRFiTiffFile& pi_rObj);
    HRFiTiffFile& operator=(const HRFiTiffFile& pi_rObj);
    };

class HRFiTiff64File : public HRFiTiffFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_iTiff64, HRFTiffFile)


    // allow to Open an image file
    IMAGEPP_EXPORT HRFiTiff64File(const HFCPtr<HFCURL>& pi_rpURL,
                                 HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                 uint64_t              pi_Offset = 0);

    IMAGEPP_EXPORT virtual ~HRFiTiff64File();

    // File capabilities
    IMAGEPP_EXPORT virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities() const;

protected :
    IMAGEPP_EXPORT virtual bool Open(bool pi_CreateBigTifFormat=true);
    virtual bool               Open(const HFCPtr<HFCURL>&  pi_rpURL)   {
        return T_Super::Open(pi_rpURL);
        }

    };

// Base iTiff Creator.
struct HRFiTiffCreatorBase : public HRFRasterFileCreator
    {
    HRFiTiffCreatorBase(HCLASS_ID pi_ClassID);

    virtual                    ~HRFiTiffCreatorBase();

    virtual WString         GetSchemes() const;

    bool                    IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                          uint64_t             pi_Offset,
                                          bool                 pi_IsItiff64) const;

    virtual bool            IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                          uint64_t                pi_Offset = 0) const {return IsKindOfFile(pi_rpURL,pi_Offset,false);}

    virtual bool            SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const;


protected:

    bool                    ValidatePageDirectory(HTIFFFile*  pi_pTiffFilePtr,
                                                   uint32_t    pi_Page) const;

    bool                    ValidateHMRDirectory(HTIFFFile*   pi_pTiffFilePtr,
                                                  uint32_t     pi_Page,
                                                  Byte**     pTransPalette) const;
    };

// HMR Creator.
struct HRFiTiffCreator : public HRFiTiffCreatorBase
    {
    // Opens the file and verifies if it is the right type
    virtual bool                    IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const;
    // Identification information
    virtual WString                 GetLabel() const;
    virtual WString                 GetExtensions() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFiTiffCreator)


    // Disabled methodes
    HRFiTiffCreator();
    };

// HMR Creator.
struct HRFiTiff64Creator : public HRFiTiffCreatorBase
    {
    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const;

    // Identification information
    virtual WString                   GetLabel() const;
    virtual WString                   GetExtensions() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFiTiff64Creator)

    // Disabled methodes
    HRFiTiff64Creator();
    };
END_IMAGEPP_NAMESPACE

