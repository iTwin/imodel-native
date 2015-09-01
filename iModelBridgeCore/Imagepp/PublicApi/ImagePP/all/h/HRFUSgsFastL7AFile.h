//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFUSgsFastL7AFile.h $
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
#include "HRFUSgsFastL7ALineEditor.h"
#include "HFCBinStream.h"
#include "HCPGeoTiffKeys.h"

BEGIN_IMAGEPP_NAMESPACE
typedef struct FL7ASatellite
    {
    char  m_Satellite[10+1];
    char  m_Sensor[10+1];
    char  m_SensorMode[6+1];
    float m_LookAngle;
    char  m_Location[17+1];
    char  m_AcquisitionDate[8+1];
    } FL7ASatellite;

typedef struct FL7ARadiometric
    {
    double m_Gain;
    double m_Bias;
    } FL7ARadiometric;

typedef struct FL7ABand
    {
    char           m_FileName[29+1];
    FL7ARadiometric m_Radiometric;
    } FL7ABand;

typedef struct FL7ACoordinateSystem
    {
    char  m_Longitude[13+1];
    char  m_Latitude[12+1];
    float m_Easting;
    float m_Northing;
    } FL7ACoordinateSystem;

typedef struct FL7ACorner
    {
    FL7ACoordinateSystem m_UpperLeft;
    FL7ACoordinateSystem m_UpperRight;
    FL7ACoordinateSystem m_LowerLeft;
    FL7ACoordinateSystem m_LowerRight;
    } FL7ACorner;

typedef struct FL7AUSGSParameters
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
    } FL7AUSGSParameters;

typedef struct FL7ASun
    {
    float m_Elevation;
    float m_Azimuth;
    } FL7ASun;

typedef struct FL7ARefPosition
    {
    FL7ACoordinateSystem m_CoordSystem;
    int32_t               m_Pixel;
    int32_t               m_Line;
    } FL7ARefPosition;

typedef struct FL7AGeometric
    {
    char              m_ProjectionName[4+1];
    char              m_Ellipsoid[18+1];
    char              m_Datum[6+1];
    FL7AUSGSParameters m_ParamInfo;
    char              m_MapZone[6+1];
    FL7ACorner         m_CornerInfo;
    FL7ARefPosition    m_RefPosInfo;
    int32_t             m_Offset;
    double            m_OrientationAngle;
    FL7ASun            m_SunInfo;

    } FL7AGeometric;

typedef struct FL7AAdministrative
    {
    char         m_ReqId[20+1];
    char         m_Loc[17+1];
    char         m_AcquisitionDate[8+1];
    FL7ASatellite m_Satellite[4+1];
    char         m_ProductType[18+1];
    char         m_ProductSize[10+1];
    char         m_TypeOfProcessing[11+1];
    char         m_Resampling[2+1];
    int32_t        m_VolumeNumber;
    int32_t        m_NumberOfVolume;
    int32_t        m_PixelPerLine;
    int32_t        m_LinePerPanBand;
    int32_t        m_LineInOutput;
    int32_t        m_StartLineNumber;
    int32_t        m_BlockingFactor;
    int32_t        m_RecSize;
    float        m_PixelSize;
    int32_t        m_OutputBitsPerPixel;
    int32_t        m_AcquiredBitsPerPixel;
    char         m_BandsPresent[32+1];
    } FL7AAdministrative;

typedef struct FL7AHeaderInfo
    {
    FL7AAdministrative m_AdminInfo;
    int32_t             m_NumberOfBands;
    FL7ABand           m_Bands[6+1];
    char              m_Revision[3+1];
    FL7AGeometric      m_GeoInfo;
    } FL7AHeaderInfo;

//--------------------------------------------------
// class HRFUSgsFastL7ACapabilities
//--------------------------------------------------
class HRFUSgsFastL7ACapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFUSgsFastL7ACapabilities();

    };

//--------------------------------------------------
// struct HRFUSgsFastL7ACreator
//--------------------------------------------------
struct HRFUSgsFastL7ACreator : public HRFRasterFileCreator
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

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFUSgsFastL7ACreator)

private:
    HRFUSgsFastL7ACreator();
    };

//--------------------------------------------------
// class HRFUSgsFastL7AFile
//--------------------------------------------------
class HRFUSgsFastL7AFile : public HRFRasterFile
    {
    friend HRFUSgsFastL7ACreator;
    friend HRFUSgsFastL7ALineEditor;

public:
    HDECLARE_CLASS_ID(HRFFileId_USgsFastL7A, HRFRasterFile)

    HRFUSgsFastL7AFile     (const HFCPtr<HFCURL>& pi_rpURL,
                            HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                            uint64_t              pi_Offset = 0);

    virtual                               ~HRFUSgsFastL7AFile           ();

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
    HRFUSgsFastL7AFile    (const HFCPtr<HFCURL>&  pi_rpURL,
                           HFCAccessMode          pi_AccessMode,
                           uint64_t              pi_Offset,
                           bool                  pi_DontOpenFile);

    virtual bool                         Open                  ();
    virtual void                          CreateDescriptors     ();

private:
    FL7AHeaderInfo              m_HeaderInfo;
    HAutoPtr<HFCBinStream>      m_pRedFile;
    HAutoPtr<HFCBinStream>      m_pGreenFile;
    HAutoPtr<HFCBinStream>      m_pBlueFile;

    int32_t                     m_ImgRedBand;
    int32_t                     m_ImgGreenBand;
    int32_t                     m_ImgBlueBand;

    bool                        GetHeaderFile                   (HFCBinStream& pi_rHeaderFile);
    bool                        GetBandNumber                   (int32_t& pio_rBand,
                                                                 const WString& pi_rFileName);
    bool                        Create                          ();

    bool                        GetCoordSystem                  (short& po_rCode);

    HFCPtr<HGF2DTransfoModel>   CreateTransfoModelFromFastL7A   ();

    void                        GetGeoKeyTag                    (HFCPtr<HCPGeoTiffKeys>& po_rpGeoTiffKeys);
    void                        RTrim                           (string& pio_rString);

    // Methods Disabled
    HRFUSgsFastL7AFile                  (const HRFUSgsFastL7AFile& pi_rObj);
    HRFUSgsFastL7AFile&                 operator=                   (const HRFUSgsFastL7AFile& pi_rObj);


    };
END_IMAGEPP_NAMESPACE

