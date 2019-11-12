//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
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

//-----------------------------------------------------------------------------
// Static Members
//-----------------------------------------------------------------------------
//Blocks

class HRFERSCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFERSCapabilities();
    };

class HRFERSPageFile : public HRFPageFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_ERSPage, HRFPageFile)

    // allow to Open an image file
    IMAGEPP_EXPORT                     HRFERSPageFile(const HFCPtr<HFCURL>&   pi_rpURL,
                                              HFCAccessMode           pi_AccessMode = HFC_READ_ONLY);

    IMAGEPP_EXPORT virtual             ~HRFERSPageFile();

    // File capabilities
    const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities       () const override;

    const HGF2DWorldIdentificator GetWorldIdentificator () const override;

    void                          WriteToDisk() override;

protected:
    // capabilities
    HFCPtr<HRFRasterFileCapabilities>     m_pPageCapabilities;

private:

    class ERSCoordinateSpaceInfo
        {
    public :

        enum CoordType
            {
            UNDEFINED,  //This type is for initialization purpose and coding logic only
            RAW,        //meters
            EN,         //Easting-Northing
            LL          //latitude and longitude
            };

        ERSCoordinateSpaceInfo()
            {
            m_CoordinateType = UNDEFINED;
            }

        AString     m_Datum;
        AString     m_Projection;
        CoordType   m_CoordinateType;

        //Optional entries
        HAutoPtr<AString>   m_pUnits;
        HAutoPtr<double>   m_pRotation;     //Radians
        };

    class ERSCellInfo
        {
    public :

        ERSCellInfo()
            {
            m_XDimension = 1;
            m_YDimension = 1;
            }

        double m_XDimension;
        double m_YDimension;
        };

    class ERSRegistrationCoord
        {
    public :

        ERSRegistrationCoord()
            {
            m_CoordType = ERSCoordinateSpaceInfo::UNDEFINED;
            }

        ERSCoordinateSpaceInfo::CoordType m_CoordType;
        double                           m_X;
        double                           m_Y;
        };

    class ERSRasterInfo
        {
    public :

        HAutoPtr<double>               m_pRegistrationCellX;
        HAutoPtr<double>               m_pRegistrationCellY;
        HAutoPtr<ERSCellInfo>           m_pCellInfo;
        HAutoPtr<ERSRegistrationCoord>  m_pRegistrationCoord;
        };

    class ERSDatasetHeaderInfo
        {
    public :
        enum DatasetType
            {
            ERSTORAGE,
            TRANSLATED
            };

        enum DataType
            {
            RASTER,
            VECTOR
            };

        AString                 m_Version;
        DatasetType             m_DatasetType;
        DataType                m_DataType;
        ERSCoordinateSpaceInfo  m_CoordSpaceInfo;
        ERSRasterInfo           m_RasterInfo;
        };

    // Members.
    HAutoPtr<HFCBinStream>  m_pFile;

    ERSDatasetHeaderInfo    m_ERSInfo;

    // Private methods
    bool                   ReadEntryValue(const string& pi_rStringToParse, AStringR po_rValue) const;

    void                    ReadLine(string& po_rString);
    void                    CleanUpString(string& pio_rString) const;
    bool                   IsValidChar(const char pi_Char) const;

    bool                   IsValidERSFile() const;

    bool                   ConvertStringToDouble(AStringCR pi_rString, double* po_pDouble) const;

    void                    ReadFile();

    void                    ReadCoordinateSpaceBlock(uint32_t& po_rCoordSpaceRequiredEntries);
    void                    ReadRasterInfoBlock(uint32_t& po_rRasterInfoRequiredEntries);
    void                    ReadCellInfoBlock();
    void                    ReadRegistrationCoordBlock();

    void                    ValidateERSFile(uint32_t pi_DatasetHeadRequiredEntries,
                                            uint32_t pi_RasterInfoRequiredEntries,
                                            uint32_t pi_CoordSpaceRequiredEntries);



    void                    CreateDescriptor();

    HFCPtr<HGF2DTransfoModel>
    BuildTransfoModel(double                        pi_OriginX,
                      double                        pi_OriginY,
                      double                        pi_PixelSizeX,
                      double                        pi_PixelSizeY,
                      double                        pi_Rotation,
                      double                        pi_RegistrationX,
                      double                        pi_RegistrationY,
                      GeoCoordinates::BaseGCSP     pi_pBaseGCS) const;

    // Methods Disabled
    HRFERSPageFile(const HRFERSPageFile& pi_rObj);
    HRFERSPageFile&  operator=(const HRFERSPageFile& pi_rObj);
    };

//-----------------------------------------------------------------------------
// This is a utility class to create a specific object.
// It is used by the Page File factory.
//-----------------------------------------------------------------------------
class HRFERSPageFileCreator : public HRFPageFileCreator
    {
public:

    // Creation of this specific instance
    virtual bool               HasFor          (const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile, bool pi_ApplyonAllFiles=false) const;
    virtual HFCPtr<HRFPageFile> CreateFor       (const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile) const;
    virtual HFCPtr<HFCURL>      ComposeURLFor   (const HFCPtr<HFCURL>&   pi_rpURLFileName) const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFERSPageFileCreator)
    };
END_IMAGEPP_NAMESPACE

