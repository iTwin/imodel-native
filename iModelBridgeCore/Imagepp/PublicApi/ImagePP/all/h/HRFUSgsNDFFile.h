//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFUSgsNDFFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HFCMacros.h"
#include "HFCAccessMode.h"
#include "HFCURL.h"
#include "HRFMacros.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFRasterFile.h"
#include "HRFUSgsNDFLineEditor.h"
#include "HFCBinStream.h"
#include "HCPGeoTiffKeys.h"

BEGIN_IMAGEPP_NAMESPACE
typedef struct NDFWaveLengths
    {
    float m_Min;
    float m_Max;
    } NDFWaveLengths;

typedef struct NDFRadiometric
    {
    float m_Gain;
    float m_Bias;
    } NDFRadiometric;

typedef struct NDFBand
    {
    string         m_Name;
    string         m_FileName;
    NDFWaveLengths m_WaveLengths;
    NDFRadiometric m_Radiometric;
    } NDFBand;

typedef struct NDFPixelSpacing
    {
    double m_Horizontal;
    double m_Vertical;
    string  m_Units;
    } NDFPixelSpacing;

typedef struct NDFPixel
    {
    string          m_PixelFormat;
    string          m_PixelOrder;
    int32_t          m_BitsPerPixel;
    NDFPixelSpacing m_PixelSpacingInfo;
    } NDFPixel;

typedef struct NDFCoordinateSystem
    {
    string  m_Longitude;
    string  m_Latitude;
    double m_Easting;
    double m_Northing;
    } NDFCoordinateSystem;

typedef struct NDFCorner
    {
    NDFCoordinateSystem m_UpperLeft;
    NDFCoordinateSystem m_UpperRight;
    NDFCoordinateSystem m_LowerLeft;
    NDFCoordinateSystem m_LowerRight;
    } NDFCorner;

typedef struct NDFRefPosition
    {
    NDFCoordinateSystem m_CoordSystem;
    int32_t              m_Pixel;
    int32_t              m_Line;
    } NDFRefPosition;

typedef struct NDFRefOffset
    {
    float m_X;
    float m_Y;
    } NDFRefOffset;

typedef struct NDFReference
    {
    string         m_Point;
    NDFRefPosition m_Position;
    NDFRefOffset   m_Offset;
    } NDFReference;

typedef struct NDFUSGSParameters
    {
    double m_p1;
    double m_p2;
    double m_p3;
    double m_p4;
    double m_p5;
    double m_p6;
    double m_p7;
    double m_p8;
    double m_p9;
    double m_p10;
    double m_p11;
    double m_p12;
    double m_p13;
    double m_p14;
    double m_p15;
    } NDFUSGSParameters;

typedef struct NDFUSGS
    {
    int32_t            m_ProjNumber;
    string            m_MapZone;
    NDFUSGSParameters m_ProjParameters;
    } NDFUSGS;

typedef struct NDFSatellite
    {
    string m_SatNumber;
    string m_SatInstrument;
    } NDFSatellite;

typedef struct NDFOffset
    {
    double m_X;
    double m_Y;
    double m_Z;
    } NDFOffset;

typedef struct NDFEarthEllipsoid
    {
    float    m_SemiMajorAxis;
    float    m_SemiMinorAxis;
    NDFOffset m_OriginOffsetInfo;
    NDFOffset m_RotationOffsetInfo;
    } NDFEarthEllipsoid;

typedef struct NDFProcessing
    {
    int32_t m_Level;
    string m_DateTime;
    string m_Software;
    } NDFProcessing;

typedef struct NDFSun
    {
    float m_Elevation;
    float m_Azimuth;
    } NDFSun;

typedef struct NDFFileInfo
    {
    NDFPixel               m_PixelInfo;
    uint32_t               m_Width;
    uint32_t               m_Height;
    HRFScanlineOrientation m_DataOrientation;
    int32_t                 m_NumberOfDataFile;
    string                 m_Interleaving;
    string                 m_TapeSpanningFlag;
    int32_t                 m_StartLineNumber;
    int32_t                 m_startDataFile;
    int32_t                 m_LinesPerVolume;
    int32_t                 m_BlockingFactor;
    int32_t                 m_RecordSize;
    } NDFFileInfo;

typedef struct NDFHeaderInfo
    {
    string            m_NDFRevision;
    string            m_DataSetType;
    string            m_ProductNumber;
    NDFFileInfo       m_FileInfo;
    NDFCorner         m_CornerInfo;
    NDFReference      m_RefInfo;
    double           m_Orientation;
    string            m_MapProjName;
    NDFUSGS           m_USGSInfo;
    string            m_HorizontalDatum;
    NDFEarthEllipsoid m_EarthEllipsoidInfo;
    string            m_WRS;
    string            m_AcquisitionDateTime;
    NDFSatellite      m_SatelliteInfo;
    NDFProcessing     m_ProcessingInfo;
    string            m_ProductSize;
    string            m_Resampling;
    NDFSun            m_SunInfo;
    int32_t            m_NumberOfBands;
    } NDFHeaderInfo;

//--------------------------------------------------
// class HRFUSgsNDFCapabilities
//--------------------------------------------------
class HRFUSgsNDFCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFUSgsNDFCapabilities();

    };

//--------------------------------------------------
// struct HRFUSgsNDFCreator
//--------------------------------------------------
struct HRFUSgsNDFCreator : public HRFRasterFileCreator
    {
public :

    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile  (const HFCPtr<HFCURL>& pi_rpURL,
                                                     uint64_t             pi_Offset) const;

    // Identification information
    virtual WString                   GetLabel      ()  const;
    virtual WString                   GetSchemes    ()  const;
    virtual WString                   GetExtensions ()  const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create        (const HFCPtr<HFCURL>& pi_rpURL,
                                                     HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                                     uint64_t             pi_Offset = 0) const;
    void                              SetImageBand  (int32_t pi_RedBand,
                                                     int32_t pi_GreenBand,
                                                     int32_t pi_BlueBand);
    int32_t                            GetRedBand    ();
    int32_t                            GetGreenBand  ();
    int32_t                            GetBlueBand   ();

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFUSgsNDFCreator)

private:
    int32_t                            m_RedBand;
    int32_t                            m_GreenBand;
    int32_t                            m_BlueBand;
    HRFUSgsNDFCreator();
    };

//--------------------------------------------------
// class HRFUSgsNDFFile
//--------------------------------------------------
class HRFUSgsNDFFile : public HRFRasterFile
    {
    friend HRFUSgsNDFCreator;
    friend HRFUSgsNDFLineEditor;

public:
    HDECLARE_CLASS_ID(HRFFileId_USgsNDF, HRFRasterFile)

    HRFUSgsNDFFile        (const HFCPtr<HFCURL>& pi_rpURL,
                           int32_t                pi_RedBand,
                           int32_t                pi_GreenBand,
                           int32_t                pi_BlueBand,
                           HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                           uint64_t             pi_Offset = 0);

    virtual                               ~HRFUSgsNDFFile           ();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const;

    // File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t      pi_Page,
                                                                 unsigned short pi_Resolution,
                                                                 HFCAccessMode pi_AccessMode);

    virtual void                          Save();

    virtual void                          SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                                 uint32_t pi_Page = 0,
                                                                 bool   pi_CheckSpecificUnitSpec = false,
                                                                 bool   pi_InterpretUnitINTGR = false);

protected:
    HRFUSgsNDFFile    (const HFCPtr<HFCURL>&  pi_rpURL,
                       HFCAccessMode          pi_AccessMode,
                       uint64_t              pi_Offset,
                       bool                  pi_DontOpenFile);

    virtual bool                         Open              ();
    virtual void                          CreateDescriptors ();

private:
    NDFHeaderInfo               m_HeaderInfo;
    NDFBand*                    m_Bands;
    HAutoPtr<HFCBinStream>      m_pRedFile;
    HAutoPtr<HFCBinStream>      m_pGreenFile;
    HAutoPtr<HFCBinStream>      m_pBlueFile;

    int32_t                     m_ImgRedBand;
    int32_t                     m_ImgGreenBand;
    int32_t                     m_ImgBlueBand;

    bool                        GetHeaderFile               (HFCBinStream& pi_rHeaderFile);
    bool                        GetBandNumber               (int32_t& pio_rBand, const WString& pi_rFileName);
    bool                        Create                      ();

    bool                        GetCoordSystem               (short& po_rCode);

    HFCPtr<HGF2DTransfoModel>   CreateTransfoModelFromNDF   ();

    void                        GetGeoKeyTag                (HFCPtr<HCPGeoTiffKeys>& po_rpGeoTiffKeys);

    // Methods Disabled
    HRFUSgsNDFFile                  (const HRFUSgsNDFFile& pi_rObj);
    HRFUSgsNDFFile&                 operator=                   (const HRFUSgsNDFFile& pi_rObj);


    };
END_IMAGEPP_NAMESPACE
