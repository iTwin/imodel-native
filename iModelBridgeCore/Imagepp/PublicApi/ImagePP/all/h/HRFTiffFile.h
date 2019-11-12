//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFTiffFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"
#include "HTIFFFile.h"

BEGIN_IMAGEPP_NAMESPACE
//class HTIFFFile;
class HRPPixelType;

#define PIXELTYPESPEC_RGB  0
#define PIXELTYPESPEC_BGR  1

// List of Const. added here so they can be shared between HMR and iTiff
static const int32_t HMR_LgStringDescriptor     = 128;
static const int32_t HMR_LgStringSystemCoord    = 128;
static const int32_t HMR_LgStringDateTime       = 20;
//static const int32_t HMR_DimTile                = 256;
//static const int32_t HMR_SizeHeader             = 2800; // Approx. dimension

//-----------------------------------------------------------------------------
// Data types used to store/pass along all channels No Data Value
//-----------------------------------------------------------------------------
typedef vector<uint32_t>     ListOfChannelIndex;
typedef vector<double>     ListOfChannelNoDataValue;

//-----------------------------------------------------------------------------
// HRFTiffBlockCapabilities
// Note: Define in the .h because GeoTiff used the same.
//-----------------------------------------------------------------------------
class HRFTiffBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFTiffBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability

        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_WRITE_CREATE, // AccessMode
                                  INT32_MAX,              // MaxSizeInBytes
                                  32,                    // MinWidth
                                  1024,                  // MaxWidth
                                  16,                    // WidthIncrement
                                  32,                    // MinHeight
                                  1024,                  // MaxHeight
                                  16,                    // HeightIncrement
                                  false));               // Not Square
        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_WRITE,        // AccessMode
                                  INT32_MAX,              // MaxSizeInBytes
                                  1,                     // MinWidth
                                  INT32_MAX,              // MaxWidth
                                  1,                     // WidthIncrement
                                  1,                     // MinHeight
                                  INT32_MAX,              // MaxHeight
                                  1,                     // HeightIncrement
                                  false));               // Not Square

        // Strip Capability
        Add(new HRFStripCapability(HFC_READ_WRITE_CREATE,  // AccessMode
                                   INT32_MAX,               // MaxSizeInBytes
                                   32,                     // MinHeight
                                   INT32_MAX,               // MaxHeight
                                   16));                   // HeightIncrement

        // Strip Capability
        Add(new HRFStripCapability(HFC_READ_WRITE,         // AccessMode
                                   INT32_MAX,               // MaxSizeInBytes
                                   1,                      // MinHeight
                                   INT32_MAX,               // MaxHeight
                                   1));                    // HeightIncrement
        }
    };

//-----------------------------------------------------------------------------
// HRFTiff1BitBlockCapabilities
// Tile creation is removed from the 1bit capabilities.
//-----------------------------------------------------------------------------
class HRFTiff1BitBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFTiff1BitBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability

        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_WRITE,        // AccessMode
                                  INT32_MAX,              // MaxSizeInBytes
                                  1,                     // MinWidth
                                  INT32_MAX,              // MaxWidth
                                  1,                     // WidthIncrement
                                  1,                     // MinHeight
                                  INT32_MAX,              // MaxHeight
                                  1,                     // HeightIncrement
                                  false));               // Not Square

        // Strip Capability
        Add(new HRFStripCapability(HFC_READ_WRITE_CREATE,  // AccessMode
                                   INT32_MAX,               // MaxSizeInBytes
                                   32,                     // MinHeight
                                   INT32_MAX,               // MaxHeight
                                   16));                   // HeightIncrement

        // Strip Capability
        Add(new HRFStripCapability(HFC_READ_WRITE,         // AccessMode
                                   INT32_MAX,               // MaxSizeInBytes
                                   1,                      // MinHeight
                                   INT32_MAX,               // MaxHeight
                                   1));                    // HeightIncrement
        }
    };

class HRFTiffCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFTiffCapabilities();
    };


class HRFTiffFile : public HRFRasterFile
    {
public:
    friend class HRFTiffTileEditor;
    friend class HRFTiffStripEditor;
    friend class HRFiTiffTileEditor;
    friend class HRFiTiffStripEditor;
    friend class HRFiTiffLuraWavePaddedTileEditor;
    friend class HRFiTiffLuraWaveNonPaddedTileEditor;
    friend class HRFGeoTiffFile;

    // Generic function for any pixel found in tiff format
    static HFCPtr<HRPPixelType> CreatePixelTypeFromFile(HTIFFFile*                          pi_pTIFFFile,
                                                        uint32_t                            pi_Page = 0,
                                                        Byte*                             pi_pAlphaPalette = 0,
                                                        uint16_t                     pi_HMRPixelTypeSpec = PIXELTYPESPEC_RGB,
                                                        const ListOfChannelIndex*           pi_pChannelsWithNoDataValue = 0,
                                                        const ListOfChannelNoDataValue*     pi_pChannelsNoDataValue = 0);

    // Generic function for any codec found in tiff format
    static HFCPtr<HCDCodec>     CreateCodecFromFile(HTIFFFile*      pi_pTIFFFile,
                                                    uint32_t        pi_Page = 0);


    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_Tiff, HRFRasterFile)

    // allow to Open an image file
     HRFTiffFile     (const HFCPtr<HFCURL>&          pi_rpURL,
					  HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
					  uint64_t                       pi_Offset = 0);

    virtual                               ~HRFTiffFile          ();

    // File capabilities
    const HFCPtr<HRFRasterFileCapabilities>&    GetCapabilities       () const override;

    // File information
    const HGF2DWorldIdentificator GetWorldIdentificator () const override;

    // File manipulation
    bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage) override;

    HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 uint16_t           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode) override;

    void                          Save() override;

    // GeoKeys are use presently in Geotiff and Itiff files.
    void                                  ResetGeokeys(uint32_t pi_Page = 0);

    // Private utility method.
    IMAGEPP_EXPORT void                           Set1BitPhotometric(bool pi_MinIsWhite);

    IMAGEPP_EXPORT HRFScanlineOrientation         GetScanLineOrientation() const;

    IMAGEPP_EXPORT uint64_t                      GetFileCurrentSize() const override;

    //Compact the iTIFF file, removing freeblocks
    IMAGEPP_EXPORT bool                          CompactITIFF();

protected:

    // Constructor use only to create a child
    //
    HRFTiffFile         (const HFCPtr<HFCURL>&  pi_rpURL,
                         HFCAccessMode          pi_AccessMode,
                         uint64_t              pi_Offset,
                         bool                  pi_DontOpenFile);
    virtual bool                       Open                (bool pi_CreateBigTifFormat=false);
    virtual bool                       Open                (const HFCPtr<HFCURL>&  pi_rpURL);
    virtual void                        CreateDescriptors   ();
    virtual void                        ReloadDescriptors();
    virtual void                        SaveDescriptors(uint32_t pi_Page = -1);


    // Use by the File and the Editor
    HTIFFFile*                   GetFilePtr            () const;
    void                                SetDirectory          (uint32_t pi_DirIndex);
    void                                SetImageInSubImage    (uint32_t pi_IndexImage);
    uint32_t                            CalcNumberOfPage      () const;
    uint16_t                     CalcNumberOfSubResolution(uint32_t pi_IndexImage) const;
    uint32_t                            GetIndexOfPage        (uint32_t pi_Page) const;

    // Thumbnail interface
    HFCPtr<HRFThumbnail>                ReadThumbnailFromFile (uint32_t pi_Page);
    void                                AddThumbnailToFile    (uint32_t pi_Page);

    void                                AddResolutionToFile   (uint32_t pi_Page,
                                                               uint16_t pi_Resolution);

    void                                WritePixelTypeAndCodecToFile(
        uint32_t                    pi_Page,
        const HFCPtr<HRPPixelType>& pi_rpPixelType,
        const HFCPtr<HCDCodec>&     pi_rpCodec,
        uint32_t                    pi_BlockWidth,
        uint32_t                    pi_BlockHeight);

    void                                GetSystemDateTime     (char* datetime);

    void                                WritePaletteToFile    (uint32_t                 pi_Page,
                                                               const HRPPixelPalette&   pi_rPalette);

    HFCPtr<HGF2DTransfoModel>           CreateTransfoModelFromTiffMatrix() const;

    virtual bool        WriteTransfoModel(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel);
    bool                       WriteTransfoModelToTiffMatrix(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel);


    // Get common TIFF family tags
    void                                GetBaselineTags       (HPMAttributeSet*     po_pTagList,
                                                               const HRPPixelType&  pi_rPixelType) const;

    void                                SaveTiffFile     (bool pi_CloseFile);

public:
    static bool s_BypassFileSharing;

private:
    friend struct HRFTiffCreator;

    typedef enum
        {
        FULLIMAGE,
        PAGE,
        REDUCEDIMAGE,
        MASK,
        EMPTYPAGE,
        HMR,
        HMR2
        } DirectoryType;

    typedef struct tagDirectoryEntries
        {
        DirectoryType   Type;
        } DirectoryEntries;

    typedef struct
        {
        DirectoryEntries    m_MainImage;
        uint32_t             m_NumSubImage;
        DirectoryEntries*   m_pSubImages;
        } ImageType;

    // File pointer
    HAutoPtr<HTIFFFile>     m_pTiff;

    // Number and description of each directory.
    uint32_t                        m_NumberDir;
    HArrayAutoPtr<DirectoryEntries> m_pDirectories;

    // Methods

    bool          Create           (bool pi_CreateBigTifFormat=false);

    void           ScanDirectories  ();
    HFCAccessMode  GetSpecialAccessMode    (const HFCAccessMode& pi_rCurrentAccessMode) const;
    HRFBlockAccess GetSpecialBlockAccessType (const HRFBlockAccess& pi_rCurrentBlockAccessType) const;

    void           WriteSampleLimitValueToDir(vector<double> const&  pi_rSampleValue,
                                              bool                   pi_IsMinLimit,
                                              HTIFFFile*             po_pTIFFFile);




    // Methods Disabled
    HRFTiffFile(const HRFTiffFile& pi_rObj);
    HRFTiffFile& operator=(const HRFTiffFile& pi_rObj);
    };

// TIFF Creator.
struct HRFTiffCreator : public HRFRasterFileCreator
    {

    // Opens the file and verifies if it is the right type
    bool                     IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                                   uint64_t             pi_Offset = 0) const override;

    // Identification information
    Utf8String                   GetLabel() const override;
    Utf8String                   GetSchemes() const override;
    Utf8String                   GetExtensions() const override;

    virtual Utf8String GetShortName() const override { return "TIF"; }

    // capabilities of Raster file.
    const HFCPtr<HRFRasterFileCapabilities>&    GetCapabilities() override;

    // allow to Open an image file READ/WRITE and CREATE
    HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const override;

private:
    friend struct HRFGeoTiffCreator;

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFTiffCreator)

    // Disabled methodes
    HRFTiffCreator();

    };
END_IMAGEPP_NAMESPACE

