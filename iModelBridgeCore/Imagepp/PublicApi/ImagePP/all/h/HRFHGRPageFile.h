//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFHGRPageFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFHGRPageFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HFCAccessMode.h"

#include "HRFPageFile.h"
#include "HRFRasterFileCapabilities.h"
#include "HGF2DWorld.h"


BEGIN_IMAGEPP_NAMESPACE
class HFCRUL;
class HFCBinStream;

class HRFHGRCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFHGRCapabilities();
    };

class HRFHGRPageFile : public HRFPageFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_HGRPage, HRFPageFile)

    typedef enum
        {
        VERSION_2_0,
        VERSION_2_1,
        VERSION_2_2,
        UNSUPPORTED_VERSION
        } FileVersion;

    // allow to Open an image file
    IMAGEPP_EXPORT                     HRFHGRPageFile(const HFCPtr<HFCURL>&   pi_rpURL,
                                              HFCAccessMode           pi_AccessMode = HFC_READ_ONLY);

    IMAGEPP_EXPORT                     HRFHGRPageFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                              uint32_t                 pi_Width,
                                              uint32_t                 pi_Height,
                                              HFCAccessMode            pi_AccessMode = HFC_READ_WRITE_CREATE,
                                              FileVersion              pi_FileVersion = VERSION_2_2);
    IMAGEPP_EXPORT virtual             ~HRFHGRPageFile();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities       () const;

    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    IMAGEPP_EXPORT virtual void                   WriteToDisk();

    IMAGEPP_EXPORT virtual void                   SetDefaultRatioToMeter(double pi_RatioToMeter);

protected:
    // capabilities
    HFCPtr<HRFRasterFileCapabilities>     m_pPageCapabilities;

private:


    // Members.
    HAutoPtr<HFCBinStream>  m_pFile;


    // ID section
    string          m_File;
    FileVersion     m_Version;

    // GeoRefSetting section
    double         m_OriginX;
    double         m_OriginY;
    double         m_PixelSizeX;
    double         m_PixelSizeY;
    uint32_t        m_ImageWidth;
    uint32_t        m_ImageHeight;
    double         m_Rotation;
    double         m_Affinity;

    // ImageInfo section
    string          m_Owner;
    string          m_Description;
    uint32_t        m_ScanningResX;
    uint32_t        m_ScanningResY;

    // Private methods

    FileVersion             GetFileVersion(const string& pi_rVersion) const;
    string                  GetFileVersion(FileVersion   pi_Version) const;

    bool                   IsValidHGRFile() const;

    bool                   ConvertStringToDouble(const string&        pi_rString,
                                                  double*              po_pDouble) const;

    bool                   ConvertStringToUnsignedLong(const string&   pi_rString,
                                                        uint32_t*         po_pLong) const;
    void                    ReadFile();
    void                    WriteFile();

    void                    CreateDescriptor();

    HFCPtr<HGF2DTransfoModel>
    BuildTransfoModel(double pi_OriginX,
                      double pi_OriginY,
                      double pi_PixelSizeX,
                      double pi_PixelSizeY,
                      double pi_Affinity,
                      double pi_Rotation) const;


    // Methods Disabled
    HRFHGRPageFile(const HRFHGRPageFile& pi_rObj);
    HRFHGRPageFile&  operator=(const HRFHGRPageFile& pi_rObj);
    };

//-----------------------------------------------------------------------------
// This is a utility class to create a specific object.
// It is used by the Page File factory.
//-----------------------------------------------------------------------------
class HRFHGRPageFileCreator : public HRFPageFileCreator
    {
public:

    // Creation of this specific instance
    virtual bool               HasFor          (const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile, bool pi_ApplyonAllFiles=false) const;
    virtual HFCPtr<HRFPageFile> CreateFor       (const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile) const;
    virtual HFCPtr<HFCURL>      ComposeURLFor   (const HFCPtr<HFCURL>&   pi_rpURLFileName) const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFHGRPageFileCreator)
    };
END_IMAGEPP_NAMESPACE

