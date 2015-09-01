//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFHMRFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"

#include "HRFTiffFile.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"

#include "HRFRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFHMRCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFHMRCapabilities();

    };


class HRFHMRFile : public HRFTiffFile
    {
public:
    friend class HRFHMRTileEditor;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_HMR, HRFTiffFile)

    // allow to Open an image file
    HRFHMRFile            (const HFCPtr<HFCURL>&          pi_rpURL,
                           HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
                           uint64_t                      pi_Offset = 0);

    virtual                               ~HRFHMRFile           ();

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

protected:

    // Methods

    // Constructor use only to create a child
    //
    HRFHMRFile                      (const HFCPtr<HFCURL>&  pi_rpURL,
                                     HFCAccessMode          pi_AccessMode,
                                     uint64_t              pi_Offset,
                                     bool                  pi_DontOpenFile);

    virtual bool                       Open                (bool pi_CreateBigTifFormat=false);
    virtual bool                       Open                (const HFCPtr<HFCURL>&  pi_rpURL)   {
        return T_Super::Open(pi_rpURL);
        }

    virtual void                          CreateDescriptors               ();
    virtual double                       GetResolutionRatio    (uint32_t pi_Page,
                                                                 unsigned short pi_Resolution) const;

    HFCPtr<HGF2DTransfoModel> CreateTransfoModelFromTiffMatrix() const;

private:
    friend struct HRFHMRCreator;
    bool                   m_HMRDirDirty;
    // File version
    //  HMR_VERSION_TILE
    //      Format Standard TIFF, (Y padding at the bottom)
    //      Real dimension image. (without padding)
    //  HMR_VERSION_TILEPADDING
    //      Special format (Y padding at the top)
    //      Real dimension image includes the padding.
    uint32_t                m_Version;

    // String describes the System Coordinate.
    char                   m_SystemCoord[HMR_LgStringSystemCoord+1];

    // Origin of the image.
    double                 m_OriginX;
    double                 m_OriginY;

    // PixelSize of the image..
    double                 m_PixelSizeX;
    double                 m_PixelSizeY;

    // Histogramm
    char                   m_HistoDateTime[HMR_LgStringDateTime+1];
    HArrayAutoPtr<uint32_t>   m_pHistogram;       // Fixe to HISTOGRAM_ENTRY

    // Shape information
    bool                   m_HMRClipShapeInFile;
    uint32_t                m_HMRClipShapeLength;
    HArrayAutoPtr<double>  m_pHMRClipShape;

    int32_t                m_HMRTransparentShapeLength;
    HArrayAutoPtr<double>  m_pHMRTransparentShape;
    bool                   m_IsBundleTagPresent;

    // User data
    int32_t                m_HMRUserDataLength;
    HArrayAutoPtr<Byte>   m_pHMRUserData;


    // These members are shared with the HMRResolutionEditor

    // Padding line at the top if the Version is HMR_VERSION_TILEPADDING
    // Image 0
    uint32_t                m_PaddingLines;
    HArrayAutoPtr<uint32_t>   m_aPaddingByRes;

    // Pixel size in byte...
    uint32_t                m_Pixel1_4Bits;
    uint32_t                m_BytesByPixel;

    // Buffer used to fill a tile, if the file has a padding lines.
    HArrayAutoPtr<Byte>    m_pTileBuffer;


    // Methods

    void                                  InitPrivateTagDefault           ();
    bool                                 ReadPrivateDirectory            ();
    void                                  WritePrivateDirectory           ();
    HFCPtr<HGF2DTransfoModel>             AddHMRInfoToTransfoModel        (HFCPtr<HGF2DTransfoModel>& pi_rpTransfo,
                                                                           uint32_t                   pi_ImageHeight);
    void                                  RemoveHMRInfoFromTransfoModel   (HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel);

    void                                  SetClipShape                    (const HRFClipShape& pi_rShape);
    HRFClipShape*                         GetClipShape                    ();
    void                                  SaveHmrFile();

    void                AllocMembers (const HFCPtr<HRPPixelType>& pi_rPixelType,
                                      uint32_t pi_BlockWidth,
                                      uint32_t pi_BlockHeight);

    // Methods Disabled
    HRFHMRFile(const HRFHMRFile& pi_rObj);
    HRFHMRFile& operator=(const HRFHMRFile& pi_rObj);
    };

// HMR Creator.
struct HRFHMRCreator : public HRFRasterFileCreator
    {
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
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFHMRCreator)

    // Disabled methodes
    HRFHMRCreator();
    };
END_IMAGEPP_NAMESPACE

