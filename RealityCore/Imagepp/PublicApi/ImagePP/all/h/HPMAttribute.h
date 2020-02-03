//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// HPMAttribute
//-----------------------------------------------------------------------------
#pragma once

#include "HPMPersistentObject.h"
#include "HFCMatrix.h"

BEGIN_IMAGEPP_NAMESPACE
// To add a new HPMAttribute, you must add an unique HPMAttributesID in the enum below and create a new HPM_DEFINE_ATTRIBUTE entry with an unique name.
// The name is not translated and it is used by internal apps to display attribute info.
enum HPMAttributesID 
     {
     HRF_ATTRIBUTEID_ImageDescription = 1,
     HRF_ATTRIBUTEID_PageName, 
     HRF_ATTRIBUTEID_DocumentName,
     HRF_ATTRIBUTEID_Title,
     HRF_ATTRIBUTEID_Notes,
     HRF_ATTRIBUTEID_KeyWord,
     HRF_ATTRIBUTEID_Make,
     HRF_ATTRIBUTEID_Model,
     HRF_ATTRIBUTEID_Software,
     HRF_ATTRIBUTEID_Version,
     HRF_ATTRIBUTEID_Copyright,
     HRF_ATTRIBUTEID_Artist,
     HRF_ATTRIBUTEID_Director,
     HRF_ATTRIBUTEID_Company,
     HRF_ATTRIBUTEID_Vendor,
     HRF_ATTRIBUTEID_HostComputer,
     HRF_ATTRIBUTEID_DateTime,
     HRF_ATTRIBUTEID_InkNames,
     HRF_ATTRIBUTEID_SecurityLevel,
     HRF_ATTRIBUTEID_LegalDisclaimer,
     HRF_ATTRIBUTEID_ContentWarning,
     HRF_ATTRIBUTEID_MinSampleValue,
     HRF_ATTRIBUTEID_MaxSampleValue,
     HRF_ATTRIBUTEID_ResolutionUnit,
     HRF_ATTRIBUTEID_XResolution,
     HRF_ATTRIBUTEID_YResolution,
     HRF_ATTRIBUTEID_3DTransformationMatrix,
     HRF_ATTRIBUTEID_OnDemandRastersInfo,
     HRF_ATTRIBUTEID_ImageSlo,
     HRF_ATTRIBUTEID_Foreground,
     HRF_ATTRIBUTEID_Background,
     HRF_ATTRIBUTEID_DontSupportPersistentColor,
     HRF_ATTRIBUTEID_GTModelType,
     HRF_ATTRIBUTEID_GTRasterType,
     HRF_ATTRIBUTEID_PCSCitation,
     HRF_ATTRIBUTEID_ProjectedCSType,
     HRF_ATTRIBUTEID_ProjectedCSTypeLong,
     HRF_ATTRIBUTEID_GTCitation,
     HRF_ATTRIBUTEID_Projection,
     HRF_ATTRIBUTEID_ProjCoordTrans,
     HRF_ATTRIBUTEID_ProjLinearUnits,
     HRF_ATTRIBUTEID_ProjLinearUnitSize,
     HRF_ATTRIBUTEID_GeographicType,
     HRF_ATTRIBUTEID_GeogCitation,
     HRF_ATTRIBUTEID_GeogGeodeticDatum,
     HRF_ATTRIBUTEID_GeogPrimeMeridian,
     HRF_ATTRIBUTEID_GeogLinearUnits,
     HRF_ATTRIBUTEID_GeogLinearUnitSize,
     HRF_ATTRIBUTEID_GeogAngularUnits,
     HRF_ATTRIBUTEID_GeogAngularUnitSize,
     HRF_ATTRIBUTEID_GeogEllipsoid,
     HRF_ATTRIBUTEID_GeogSemiMajorAxis,
     HRF_ATTRIBUTEID_GeogSemiMinorAxis,
     HRF_ATTRIBUTEID_GeogInvFlattening,
     HRF_ATTRIBUTEID_GeogAzimuthUnits,
     HRF_ATTRIBUTEID_GeogPrimeMeridianLong,
     HRF_ATTRIBUTEID_ProjStdParallel1,
     HRF_ATTRIBUTEID_ProjStdParallel2,
     HRF_ATTRIBUTEID_ProjNatOriginLong,
     HRF_ATTRIBUTEID_ProjNatOriginLat,
     HRF_ATTRIBUTEID_ProjFalseEasting,
     HRF_ATTRIBUTEID_ProjFalseNorthing,
     HRF_ATTRIBUTEID_ProjFalseOriginLong,
     HRF_ATTRIBUTEID_ProjFalseOriginLat,
     HRF_ATTRIBUTEID_ProjFalseOriginEasting,
     HRF_ATTRIBUTEID_ProjFalseOriginNorthing,
     HRF_ATTRIBUTEID_ProjCenterLong,
     HRF_ATTRIBUTEID_ProjCenterLat,
     HRF_ATTRIBUTEID_ProjCenterEasting,
     HRF_ATTRIBUTEID_ProjCenterNorthing,
     HRF_ATTRIBUTEID_ProjScaleAtNatOrigin,
     HRF_ATTRIBUTEID_ProjScaleAtCenter,
     HRF_ATTRIBUTEID_ProjAzimuthAngle,
     HRF_ATTRIBUTEID_ProjStraightVertPoleLong,
     HRF_ATTRIBUTEID_ProjRectifiedGridAngle,
     HRF_ATTRIBUTEID_VerticalCSType,
     HRF_ATTRIBUTEID_VerticalCitation,
     HRF_ATTRIBUTEID_VerticalDatum,
     HRF_ATTRIBUTEID_VerticalUnits,
     HRF_ATTRIBUTEID_VerticalUnitRatioToMeter,
     HRF_ATTRIBUTEID_GPSVersionID,
     HRF_ATTRIBUTEID_GPSLatitudeRef,
     HRF_ATTRIBUTEID_GPSLatitude,
     HRF_ATTRIBUTEID_GPSLongitudeRef,
     HRF_ATTRIBUTEID_GPSLongitude,
     HRF_ATTRIBUTEID_GPSAltitudeRef,
     HRF_ATTRIBUTEID_GPSAltitude,
     HRF_ATTRIBUTEID_GPSTimeStamp,
     HRF_ATTRIBUTEID_GPSSatellites,
     HRF_ATTRIBUTEID_GPSStatus,
     HRF_ATTRIBUTEID_GPSMeasureMode,
     HRF_ATTRIBUTEID_GPSDOP,
     HRF_ATTRIBUTEID_GPSSpeedRef,
     HRF_ATTRIBUTEID_GPSSpeed,
     HRF_ATTRIBUTEID_GPSTrackRef,
     HRF_ATTRIBUTEID_GPSTrack,
     HRF_ATTRIBUTEID_GPSImgDirectionRef,
     HRF_ATTRIBUTEID_GPSImgDirection,
     HRF_ATTRIBUTEID_GPSMapDatum,
     HRF_ATTRIBUTEID_GPSDestLatitudeRef,
     HRF_ATTRIBUTEID_GPSDestLatitude,
     HRF_ATTRIBUTEID_GPSDestLongitudeRef,
     HRF_ATTRIBUTEID_GPSDestLongitude,
     HRF_ATTRIBUTEID_GPSDestBearingRef,
     HRF_ATTRIBUTEID_GPSDestBearing,
     HRF_ATTRIBUTEID_GPSDestDistanceRef,
     HRF_ATTRIBUTEID_GPSDestDistance,
     HRF_ATTRIBUTEID_GPSProcessingMethod,
     HRF_ATTRIBUTEID_GPSAreaInformation,
     HRF_ATTRIBUTEID_GPSDateStamp,
     HRF_ATTRIBUTEID_GPSDifferential,
     HRF_ATTRIBUTEID_ExposureTime,
     HRF_ATTRIBUTEID_FNumber,
     HRF_ATTRIBUTEID_ExposureProgram,
     HRF_ATTRIBUTEID_SpectralSensitivity,
     HRF_ATTRIBUTEID_ISOSpeedRatings,
     HRF_ATTRIBUTEID_OptoElectricConversionFactor,
     HRF_ATTRIBUTEID_ExifVersion,
     HRF_ATTRIBUTEID_DateTimeOriginal,
     HRF_ATTRIBUTEID_DateTimeDigitized,
     HRF_ATTRIBUTEID_ComponentsConfiguration,
     HRF_ATTRIBUTEID_CompressedBitsPerPixel,
     HRF_ATTRIBUTEID_ShutterSpeedValue,
     HRF_ATTRIBUTEID_ApertureValue,
     HRF_ATTRIBUTEID_BrightnessValue,
     HRF_ATTRIBUTEID_ExposureBiasValue,
     HRF_ATTRIBUTEID_MaxApertureValue,
     HRF_ATTRIBUTEID_SubjectDistance,
     HRF_ATTRIBUTEID_MeteringMode,
     HRF_ATTRIBUTEID_LightSource,
     HRF_ATTRIBUTEID_Flash,
     HRF_ATTRIBUTEID_FocalLength,
     HRF_ATTRIBUTEID_SubjectArea,
     HRF_ATTRIBUTEID_MakerNote,
     HRF_ATTRIBUTEID_UserComment,
     HRF_ATTRIBUTEID_SubSecTime,
     HRF_ATTRIBUTEID_SubSecTimeOriginal,
     HRF_ATTRIBUTEID_SubSecTimeDigitized,
     HRF_ATTRIBUTEID_FlashpixVersion,
     HRF_ATTRIBUTEID_ColorSpace,
     HRF_ATTRIBUTEID_PixelXDimension,
     HRF_ATTRIBUTEID_PixelYDimension,
     HRF_ATTRIBUTEID_RelatedSoundFile,
     HRF_ATTRIBUTEID_FlashEnergy,
     HRF_ATTRIBUTEID_SpatialFrequencyResponse,
     HRF_ATTRIBUTEID_FocalPlaneXResolution,
     HRF_ATTRIBUTEID_FocalPlaneYResolution,
     HRF_ATTRIBUTEID_FocalPlaneResolutionUnit,
     HRF_ATTRIBUTEID_SubjectLocation,
     HRF_ATTRIBUTEID_ExposureIndex,
     HRF_ATTRIBUTEID_SensingMethod,
     HRF_ATTRIBUTEID_FileSource,
     HRF_ATTRIBUTEID_SceneType,
     HRF_ATTRIBUTEID_CFAPattern,
     HRF_ATTRIBUTEID_CustomRendered,
     HRF_ATTRIBUTEID_ExposureMode,
     HRF_ATTRIBUTEID_WhiteBalance,
     HRF_ATTRIBUTEID_DigitalZoomRatio,
     HRF_ATTRIBUTEID_FocalLengthIn35mmFilm,
     HRF_ATTRIBUTEID_SceneCaptureType,
     HRF_ATTRIBUTEID_GainControl,
     HRF_ATTRIBUTEID_Contrast,
     HRF_ATTRIBUTEID_Saturation,
     HRF_ATTRIBUTEID_Sharpness,
     HRF_ATTRIBUTEID_DeviceSettingDescription,
     HRF_ATTRIBUTEID_SubjectDistanceRange,
     HRF_ATTRIBUTEID_ImageUniqueID,
     HRF_ATTRIBUTEID_GIF_ApplicationCode,
     HRF_ATTRIBUTEID_GIF_Software,
     HRF_ATTRIBUTEID_ImageGamma,
     HRF_ATTRIBUTEID_TimeModification,
     HRF_ATTRIBUTEID_HGR_File,
     HRF_ATTRIBUTEID_HGR_Version,
     HRF_ATTRIBUTEID_HGR_Owner,
     HRF_ATTRIBUTEID_HGR_Description,
     HRF_ATTRIBUTEID_OriginalFileFormat,
     HRF_ATTRIBUTEID_OriginalFileCompression,
     HRF_ATTRIBUTEID_OriginalFileSize,
     HRF_ATTRIBUTEID_OriginalFileModificationDate,

     //HPSParser
     HPS_ATTRIBUTEID_ImageDescription = 20000,
     }; 
 
class HPMGenericAttribute : public HFCShareableObject<HPMGenericAttribute>
    {
    HDECLARE_BASECLASS_ID(HPMGenericAttributeId_Base)

public:
    IMAGEPP_EXPORT explicit HPMGenericAttribute();
    virtual ~HPMGenericAttribute() {}

    //! Same kind of attribute
    bool SameAttributeAs (HPMGenericAttribute const& pi_attribute);

    virtual HPMAttributesID GetID () const = 0;

    virtual HPMGenericAttribute* Clone () const = 0;
    
    virtual Utf8String GetDataAsString() const = 0;
  
    // A unique name use for internal purpose. Name is not translated.
    virtual Utf8String GetName () const = 0; 

protected:
    IMAGEPP_EXPORT HPMGenericAttribute(const HPMGenericAttribute& pi_rObj);
    
private:
    //Disabled   
    HPMGenericAttribute& operator=(const HPMGenericAttribute& pi_rObj);
    };

template<class T, HPMAttributesID ID> class HPMAttribute_T : public HPMGenericAttribute
    {
public : 
    enum ATTRIBUTE_ID { ATTRIBUTE_ID = ID}; 

    typedef T DataType;

    HPMAttribute_T(); 
    HPMAttribute_T(const T& pi_rData);
    virtual ~HPMAttribute_T() {}
    
    // inherit from HPMGenericAttribute

    virtual HPMAttributesID GetID () const override;

    virtual HPMGenericAttribute* Clone () const override;

    virtual Utf8String GetDataAsString() const override; 

    // A unique name use for internal purpose. Name is not translated.
    virtual Utf8String GetName () const override;

    const T& GetData () const;
    void SetData (const T& pi_rData);
    
private:
    // Disabled
    HPMAttribute_T(const HPMAttribute_T& pi_rObj);
    HPMAttribute_T& operator=(const HPMAttribute_T& pi_rObj);
    
    T   m_Data;       
    };

#define HPM_DEFINE_ATTRIBUTE(ClassName, DataType, AttributeID, Name) \
    typedef HPMAttribute_T<DataType, AttributeID> ClassName; \
    template<> inline Utf8String ClassName::GetName() const {return Name;}

typedef HFCMatrix<4,4,double> _HFCMatrix4x4Double;       // Use a typedef that will remove ',' to satisfy our macro.

// ------------------------------------------------------------------------------------------------------------------------------------------------------
//                   Class Name                                DataType              AttributeID                                   Name
HPM_DEFINE_ATTRIBUTE(HRFAttributeImageDescription,             Utf8String,              HRF_ATTRIBUTEID_ImageDescription,             "Image Description");
HPM_DEFINE_ATTRIBUTE(HRFAttributePageName,                     Utf8String,              HRF_ATTRIBUTEID_PageName,                     "Page Name");
HPM_DEFINE_ATTRIBUTE(HRFAttributeDocumentName,                 Utf8String,              HRF_ATTRIBUTEID_DocumentName,                 "Document Name");
HPM_DEFINE_ATTRIBUTE(HRFAttributeTitle,                        Utf8String,              HRF_ATTRIBUTEID_Title,                        "Title");
HPM_DEFINE_ATTRIBUTE(HRFAttributeNotes,                        Utf8String,              HRF_ATTRIBUTEID_Notes,                        "Notes");
HPM_DEFINE_ATTRIBUTE(HRFAttributeKeyWord,                      Utf8String,              HRF_ATTRIBUTEID_KeyWord,                      "Key Word");
HPM_DEFINE_ATTRIBUTE(HRFAttributeMake,                         Utf8String,              HRF_ATTRIBUTEID_Make,                         "Make");
HPM_DEFINE_ATTRIBUTE(HRFAttributeModel,                        Utf8String,              HRF_ATTRIBUTEID_Model,                        "Model");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSoftware,                     Utf8String,              HRF_ATTRIBUTEID_Software,                     "Software");
HPM_DEFINE_ATTRIBUTE(HRFAttributeVersion,                      Utf8String,              HRF_ATTRIBUTEID_Version,                      "Version");
HPM_DEFINE_ATTRIBUTE(HRFAttributeCopyright,                    Utf8String,              HRF_ATTRIBUTEID_Copyright,                    "Copyright");
HPM_DEFINE_ATTRIBUTE(HRFAttributeArtist,                       Utf8String,              HRF_ATTRIBUTEID_Artist,                       "Artist");
HPM_DEFINE_ATTRIBUTE(HRFAttributeDirector,                     Utf8String,              HRF_ATTRIBUTEID_Director,                     "Director");
HPM_DEFINE_ATTRIBUTE(HRFAttributeCompany,                      Utf8String,              HRF_ATTRIBUTEID_Company,                      "Company");
HPM_DEFINE_ATTRIBUTE(HRFAttributeVendor,                       Utf8String,              HRF_ATTRIBUTEID_Vendor,                       "Vendor");
HPM_DEFINE_ATTRIBUTE(HRFAttributeHostComputer,                 Utf8String,              HRF_ATTRIBUTEID_HostComputer,                 "Host Computer");
HPM_DEFINE_ATTRIBUTE(HRFAttributeDateTime,                     Utf8String,              HRF_ATTRIBUTEID_DateTime,                     "Date Time");         // format: "%4d:%02d:%02d %02d:%02d:%02d"
HPM_DEFINE_ATTRIBUTE(HRFAttributeInkNames,                     Utf8String,              HRF_ATTRIBUTEID_InkNames,                     "Ink Names");         //        Year Month  Day  H    M   S
HPM_DEFINE_ATTRIBUTE(HRFAttributeSecurityLevel,                Utf8String,              HRF_ATTRIBUTEID_SecurityLevel,                "Security Level");
HPM_DEFINE_ATTRIBUTE(HRFAttributeLegalDisclaimer,              Utf8String,              HRF_ATTRIBUTEID_LegalDisclaimer,              "Legal Disclaimer");
HPM_DEFINE_ATTRIBUTE(HRFAttributeContentWarning,               Utf8String,              HRF_ATTRIBUTEID_ContentWarning,               "Content Warning");
HPM_DEFINE_ATTRIBUTE(HRFAttributeMinSampleValue,               vector<double>,       HRF_ATTRIBUTEID_MinSampleValue,               "Sample Minimum Value");
HPM_DEFINE_ATTRIBUTE(HRFAttributeMaxSampleValue,               vector<double>,       HRF_ATTRIBUTEID_MaxSampleValue,               "Sample Maximum Value");
HPM_DEFINE_ATTRIBUTE(HRFAttributeResolutionUnit,               uint16_t,               HRF_ATTRIBUTEID_ResolutionUnit,               "Resolution Unit");
HPM_DEFINE_ATTRIBUTE(HRFAttributeXResolution,                  double,               HRF_ATTRIBUTEID_XResolution,                  "X Resolution");
HPM_DEFINE_ATTRIBUTE(HRFAttributeYResolution,                  double,               HRF_ATTRIBUTEID_YResolution,                  "Y Resolution");
HPM_DEFINE_ATTRIBUTE(HRFAttribute3DTransformationMatrix,       _HFCMatrix4x4Double,  HRF_ATTRIBUTEID_3DTransformationMatrix,       "3D Tranformation Matrix");
HPM_DEFINE_ATTRIBUTE(HRFAttributeOnDemandRastersInfo,          string,               HRF_ATTRIBUTEID_OnDemandRastersInfo,          "OnDemand Rasters Info");
HPM_DEFINE_ATTRIBUTE(HRFAttributeImageSlo,                     uint16_t,               HRF_ATTRIBUTEID_ImageSlo,                     "Slo");
HPM_DEFINE_ATTRIBUTE(HRFAttributeForeground,                   uint32_t,               HRF_ATTRIBUTEID_Foreground,                   "Foreground");
HPM_DEFINE_ATTRIBUTE(HRFAttributeBackground,                   uint32_t,               HRF_ATTRIBUTEID_Background,                   "Background");
HPM_DEFINE_ATTRIBUTE(HRFAttributeDontSupportPersistentColor,   bool,                 HRF_ATTRIBUTEID_DontSupportPersistentColor,   "Persistent Color");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGTModelType,                  uint16_t,               HRF_ATTRIBUTEID_GTModelType,                  "GTModelType");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGTRasterType,                 uint16_t,               HRF_ATTRIBUTEID_GTRasterType,                 "GTRasterType");
HPM_DEFINE_ATTRIBUTE(HRFAttributePCSCitation,                  Utf8String,              HRF_ATTRIBUTEID_PCSCitation,                  "PCSCitation");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjectedCSType,              uint16_t,               HRF_ATTRIBUTEID_ProjectedCSType,              "ProjectedCSType");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjectedCSTypeLong,          uint32_t,              HRF_ATTRIBUTEID_ProjectedCSTypeLong,          "ProjectedCSTypeLong");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGTCitation,                   Utf8String,              HRF_ATTRIBUTEID_GTCitation,                   "GTCitation");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjection,                   uint16_t,               HRF_ATTRIBUTEID_Projection,                   "Projection");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjCoordTrans,               uint16_t,               HRF_ATTRIBUTEID_ProjCoordTrans,               "ProjCoordTrans");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjLinearUnits,              uint16_t,               HRF_ATTRIBUTEID_ProjLinearUnits,              "ProjLinearUnits");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjLinearUnitSize,           double,               HRF_ATTRIBUTEID_ProjLinearUnitSize,           "ProjLinearUnitSize");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeographicType,               uint16_t,               HRF_ATTRIBUTEID_GeographicType,               "GeographicType");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogCitation,                 Utf8String,              HRF_ATTRIBUTEID_GeogCitation,                 "GeogCitation");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogGeodeticDatum,            uint16_t,               HRF_ATTRIBUTEID_GeogGeodeticDatum,            "GeogGeodeticDatum");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogPrimeMeridian,            uint16_t,               HRF_ATTRIBUTEID_GeogPrimeMeridian,            "GeogPrimeMeridian");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogLinearUnits,              uint16_t,               HRF_ATTRIBUTEID_GeogLinearUnits,              "GeogLinearUnits");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogLinearUnitSize,           double,               HRF_ATTRIBUTEID_GeogLinearUnitSize,           "GeogLinearUnitSize");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogAngularUnits,             uint16_t,               HRF_ATTRIBUTEID_GeogAngularUnits,             "GeogAngularUnits");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogAngularUnitSize,          double,               HRF_ATTRIBUTEID_GeogAngularUnitSize,          "GeogAngularUnitSize");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogEllipsoid,                uint16_t,               HRF_ATTRIBUTEID_GeogEllipsoid,                "GeogEllipsoid");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogSemiMajorAxis,            double,               HRF_ATTRIBUTEID_GeogSemiMajorAxis,            "GeogSemiMajorAxis");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogSemiMinorAxis,            double,               HRF_ATTRIBUTEID_GeogSemiMinorAxis,            "GeogSemiMinorAxis");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogInvFlattening,            double,               HRF_ATTRIBUTEID_GeogInvFlattening,            "GeogInvFlattening");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogAzimuthUnits,             uint16_t,               HRF_ATTRIBUTEID_GeogAzimuthUnits,             "GeogAzimuthUnits");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogPrimeMeridianLong,        double,               HRF_ATTRIBUTEID_GeogPrimeMeridianLong,        "GeogPrimeMeridianLong");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjStdParallel1,             double,               HRF_ATTRIBUTEID_ProjStdParallel1,             "ProjStdParallel1");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjStdParallel2,             double,               HRF_ATTRIBUTEID_ProjStdParallel2,             "ProjStdParallel2");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjNatOriginLong,            double,               HRF_ATTRIBUTEID_ProjNatOriginLong,            "ProjNatOriginLong");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjNatOriginLat,             double,               HRF_ATTRIBUTEID_ProjNatOriginLat,             "ProjNatOriginLat");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjFalseEasting,             double,               HRF_ATTRIBUTEID_ProjFalseEasting,             "ProjFalseEasting");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjFalseNorthing,            double,               HRF_ATTRIBUTEID_ProjFalseNorthing,            "ProjFalseNorthing");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjFalseOriginLong,          double,               HRF_ATTRIBUTEID_ProjFalseOriginLong,          "ProjFalseOriginLong");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjFalseOriginLat,           double,               HRF_ATTRIBUTEID_ProjFalseOriginLat,           "ProjFalseOriginLat");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjFalseOriginEasting,       double,               HRF_ATTRIBUTEID_ProjFalseOriginEasting,       "ProjFalseOriginEasting");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjFalseOriginNorthing,      double,               HRF_ATTRIBUTEID_ProjFalseOriginNorthing,      "ProjFalseOriginNorthing");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjCenterLong,               double,               HRF_ATTRIBUTEID_ProjCenterLong,               "ProjCenterLong");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjCenterLat,                double,               HRF_ATTRIBUTEID_ProjCenterLat,                "ProjCenterLat");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjCenterEasting,            double,               HRF_ATTRIBUTEID_ProjCenterEasting,            "ProjCenterEasting");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjCenterNorthing,           double,               HRF_ATTRIBUTEID_ProjCenterNorthing,           "ProjCenterNorthing");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjScaleAtNatOrigin,         double,               HRF_ATTRIBUTEID_ProjScaleAtNatOrigin,         "ProjScaleAtNatOrigin");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjScaleAtCenter,            double,               HRF_ATTRIBUTEID_ProjScaleAtCenter,            "ProjScaleAtCenter");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjAzimuthAngle,             double,               HRF_ATTRIBUTEID_ProjAzimuthAngle,             "ProjAzimuthAngle");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjStraightVertPoleLong,     double,               HRF_ATTRIBUTEID_ProjStraightVertPoleLong,     "ProjStraightVertPoleLong");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjRectifiedGridAngle,       double,               HRF_ATTRIBUTEID_ProjRectifiedGridAngle,       "ProjRectifiedGridAngle");
HPM_DEFINE_ATTRIBUTE(HRFAttributeVerticalCSType,               uint16_t,       HRF_ATTRIBUTEID_VerticalCSType,               "VerticalCSType");
HPM_DEFINE_ATTRIBUTE(HRFAttributeVerticalCitation,             Utf8String,              HRF_ATTRIBUTEID_VerticalCitation,             "VerticalCitation");
HPM_DEFINE_ATTRIBUTE(HRFAttributeVerticalDatum,                uint16_t,       HRF_ATTRIBUTEID_VerticalDatum,                "VerticalDatum");
HPM_DEFINE_ATTRIBUTE(HRFAttributeVerticalUnits,                uint16_t,       HRF_ATTRIBUTEID_VerticalUnits,                "VerticalUnits");
HPM_DEFINE_ATTRIBUTE(HRFAttributeVerticalUnitRatioToMeter,     double,               HRF_ATTRIBUTEID_VerticalUnitRatioToMeter,     "VerticalUnitRatioToMeter");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSVersionID,                 vector<Byte>,         HRF_ATTRIBUTEID_GPSVersionID,                 "GPSVersionID");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSLatitudeRef,               Utf8String,              HRF_ATTRIBUTEID_GPSLatitudeRef,               "GPSLatitudeRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSLatitude,                  vector<double>,       HRF_ATTRIBUTEID_GPSLatitude,                  "GPSLatitude");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSLongitudeRef,              Utf8String,              HRF_ATTRIBUTEID_GPSLongitudeRef,              "GPSLongitudeRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSLongitude,                 vector<double>,       HRF_ATTRIBUTEID_GPSLongitude,                 "GPSLongitude");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSAltitudeRef,               Byte,                 HRF_ATTRIBUTEID_GPSAltitudeRef,               "GPSAltitudeRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSAltitude,                  double,               HRF_ATTRIBUTEID_GPSAltitude,                  "GPSAltitude");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSTimeStamp,                 vector<double>,       HRF_ATTRIBUTEID_GPSTimeStamp,                 "GPSTimeStamp");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSSatellites,                Utf8String,              HRF_ATTRIBUTEID_GPSSatellites,                "GPSSatellites");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSStatus,                    Utf8String,              HRF_ATTRIBUTEID_GPSStatus,                    "GPSStatus");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSMeasureMode,               Utf8String,              HRF_ATTRIBUTEID_GPSMeasureMode,               "GPSMeasureMode");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDOP,                       double,               HRF_ATTRIBUTEID_GPSDOP,                       "GPSDOP");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSSpeedRef,                  Utf8String,              HRF_ATTRIBUTEID_GPSSpeedRef,                  "GPSSpeedRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSSpeed,                     double,               HRF_ATTRIBUTEID_GPSSpeed,                     "GPSSpeed");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSTrackRef,                  Utf8String,              HRF_ATTRIBUTEID_GPSTrackRef,                  "GPSTrackRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSTrack,                     double,               HRF_ATTRIBUTEID_GPSTrack,                     "GPSTrack");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSImgDirectionRef,           Utf8String,              HRF_ATTRIBUTEID_GPSImgDirectionRef,           "GPSImgDirectionRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSImgDirection,              double,               HRF_ATTRIBUTEID_GPSImgDirection,              "GPSImgDirection");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSMapDatum,                  Utf8String,              HRF_ATTRIBUTEID_GPSMapDatum,                  "GPSMapDatum");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDestLatitudeRef,           Utf8String,              HRF_ATTRIBUTEID_GPSDestLatitudeRef,           "GPSDestLatitudeRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDestLatitude,              vector<double>,       HRF_ATTRIBUTEID_GPSDestLatitude,              "GPSDestLatitude");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDestLongitudeRef,          Utf8String,              HRF_ATTRIBUTEID_GPSDestLongitudeRef,          "GPSDestLongitudeRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDestLongitude,             vector<double>,       HRF_ATTRIBUTEID_GPSDestLongitude,             "GPSDestLongitude");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDestBearingRef,            Utf8String,              HRF_ATTRIBUTEID_GPSDestBearingRef,            "GPSDestBearingRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDestBearing,               double,               HRF_ATTRIBUTEID_GPSDestBearing,               "GPSDestBearing");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDestDistanceRef,           Utf8String,              HRF_ATTRIBUTEID_GPSDestDistanceRef,           "GPSDestDistanceRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDestDistance,              double,               HRF_ATTRIBUTEID_GPSDestDistance,              "GPSDestDistance");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSProcessingMethod,          vector<Byte>,         HRF_ATTRIBUTEID_GPSProcessingMethod,          "GPSProcessingMethod");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSAreaInformation,           vector<Byte>,         HRF_ATTRIBUTEID_GPSAreaInformation,           "GPSAreaInformation");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDateStamp,                 Utf8String,              HRF_ATTRIBUTEID_GPSDateStamp,                 "GPSDateStamp");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDifferential,              uint16_t,               HRF_ATTRIBUTEID_GPSDifferential,              "GPSDifferential");
HPM_DEFINE_ATTRIBUTE(HRFAttributeExposureTime,                 double,               HRF_ATTRIBUTEID_ExposureTime,                 "ExposureTime");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFNumber,                      double,               HRF_ATTRIBUTEID_FNumber,                      "FNumber");
HPM_DEFINE_ATTRIBUTE(HRFAttributeExposureProgram,              uint16_t,               HRF_ATTRIBUTEID_ExposureProgram,              "ExposureProgram");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSpectralSensitivity,          Utf8String,              HRF_ATTRIBUTEID_SpectralSensitivity,          "SpectralSensitivity");
HPM_DEFINE_ATTRIBUTE(HRFAttributeISOSpeedRatings,              vector<uint16_t>,       HRF_ATTRIBUTEID_ISOSpeedRatings,              "ISOSpeedRatings");
HPM_DEFINE_ATTRIBUTE(HRFAttributeOptoElectricConversionFactor, vector<Byte>,         HRF_ATTRIBUTEID_OptoElectricConversionFactor, "OptoelectricConversionFactor");
HPM_DEFINE_ATTRIBUTE(HRFAttributeExifVersion,                  vector<char>,         HRF_ATTRIBUTEID_ExifVersion,                  "ExifVersion");
HPM_DEFINE_ATTRIBUTE(HRFAttributeDateTimeOriginal,             Utf8String,              HRF_ATTRIBUTEID_DateTimeOriginal,             "DateTimeOriginal");
HPM_DEFINE_ATTRIBUTE(HRFAttributeDateTimeDigitized,            Utf8String,              HRF_ATTRIBUTEID_DateTimeDigitized,            "DateTimeDigitized");
HPM_DEFINE_ATTRIBUTE(HRFAttributeComponentsConfiguration,      vector<Byte>,         HRF_ATTRIBUTEID_ComponentsConfiguration,      "ComponentsConfiguration");
HPM_DEFINE_ATTRIBUTE(HRFAttributeCompressedBitsPerPixel,       double,               HRF_ATTRIBUTEID_CompressedBitsPerPixel,       "CompressedBitsPerPixel");
HPM_DEFINE_ATTRIBUTE(HRFAttributeShutterSpeedValue,            double,               HRF_ATTRIBUTEID_ShutterSpeedValue,            "ShutterSpeedValue");
HPM_DEFINE_ATTRIBUTE(HRFAttributeApertureValue,                double,               HRF_ATTRIBUTEID_ApertureValue,                "ApertureValue");
HPM_DEFINE_ATTRIBUTE(HRFAttributeBrightnessValue,              double,               HRF_ATTRIBUTEID_BrightnessValue,              "BrightnessValue");
HPM_DEFINE_ATTRIBUTE(HRFAttributeExposureBiasValue,            double,               HRF_ATTRIBUTEID_ExposureBiasValue,            "ExposureBiasValue");
HPM_DEFINE_ATTRIBUTE(HRFAttributeMaxApertureValue,             double,               HRF_ATTRIBUTEID_MaxApertureValue,             "MaxApertureValue");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSubjectDistance,              double,               HRF_ATTRIBUTEID_SubjectDistance,              "SubjectDistance");
HPM_DEFINE_ATTRIBUTE(HRFAttributeMeteringMode,                 uint16_t,               HRF_ATTRIBUTEID_MeteringMode,                 "MeteringMode");
HPM_DEFINE_ATTRIBUTE(HRFAttributeLightSource,                  uint16_t,               HRF_ATTRIBUTEID_LightSource,                  "LightSource");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFlash,                        uint16_t,               HRF_ATTRIBUTEID_Flash,                        "Flash");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFocalLength,                  double,               HRF_ATTRIBUTEID_FocalLength,                  "FocalLength");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSubjectArea,                  vector<uint16_t>,       HRF_ATTRIBUTEID_SubjectArea,                  "SubjectArea");
HPM_DEFINE_ATTRIBUTE(HRFAttributeMakerNote,                    vector<Byte>,         HRF_ATTRIBUTEID_MakerNote,                    "MakerNote");
HPM_DEFINE_ATTRIBUTE(HRFAttributeUserComment,                  vector<Byte>,         HRF_ATTRIBUTEID_UserComment,                  "UserComment");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSubSecTime,                   Utf8String,              HRF_ATTRIBUTEID_SubSecTime,                   "SubSecTime");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSubSecTimeOriginal,           Utf8String,              HRF_ATTRIBUTEID_SubSecTimeOriginal,           "SubSecTimeOriginal");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSubSecTimeDigitized,          Utf8String,              HRF_ATTRIBUTEID_SubSecTimeDigitized,          "SubSecTimeDigitized");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFlashpixVersion,              vector<Byte>,         HRF_ATTRIBUTEID_FlashpixVersion,              "FlashpixVersion");
HPM_DEFINE_ATTRIBUTE(HRFAttributeColorSpace,                   uint16_t,               HRF_ATTRIBUTEID_ColorSpace,                   "ColorSpace");
HPM_DEFINE_ATTRIBUTE(HRFAttributePixelXDimension,              uint32_t,              HRF_ATTRIBUTEID_PixelXDimension,              "PixelXDimension");
HPM_DEFINE_ATTRIBUTE(HRFAttributePixelYDimension,              uint32_t,              HRF_ATTRIBUTEID_PixelYDimension,              "PixelYDimension");
HPM_DEFINE_ATTRIBUTE(HRFAttributeRelatedSoundFile,             Utf8String,              HRF_ATTRIBUTEID_RelatedSoundFile,             "RelatedSoundFile");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFlashEnergy,                  double,               HRF_ATTRIBUTEID_FlashEnergy,                  "FlashEnergy");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSpatialFrequencyResponse,     vector<Byte>,         HRF_ATTRIBUTEID_SpatialFrequencyResponse,     "SpatialFrequencyResponse");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFocalPlaneXResolution,        double,               HRF_ATTRIBUTEID_FocalPlaneXResolution,        "FocalPlaneXResolution");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFocalPlaneYResolution,        double,               HRF_ATTRIBUTEID_FocalPlaneYResolution,        "FocalPlaneYResolution");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFocalPlaneResolutionUnit,     uint16_t,               HRF_ATTRIBUTEID_FocalPlaneResolutionUnit,     "FocalPlaneResolutionUnit");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSubjectLocation,              vector<uint16_t>,       HRF_ATTRIBUTEID_SubjectLocation,              "SubjectLocation");
HPM_DEFINE_ATTRIBUTE(HRFAttributeExposureIndex,                double,               HRF_ATTRIBUTEID_ExposureIndex,                "ExposureIndex");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSensingMethod,                uint16_t,               HRF_ATTRIBUTEID_SensingMethod,                "SensingMethod");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFileSource,                   Byte,                 HRF_ATTRIBUTEID_FileSource,                   "FileSource");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSceneType,                    Byte,                 HRF_ATTRIBUTEID_SceneType,                    "SceneType");
HPM_DEFINE_ATTRIBUTE(HRFAttributeCFAPattern,                   vector<Byte>,         HRF_ATTRIBUTEID_CFAPattern,                   "CFAPattern");
HPM_DEFINE_ATTRIBUTE(HRFAttributeCustomRendered,               uint16_t,               HRF_ATTRIBUTEID_CustomRendered,               "CustomRendered");
HPM_DEFINE_ATTRIBUTE(HRFAttributeExposureMode,                 uint16_t,               HRF_ATTRIBUTEID_ExposureMode,                 "ExposureMode");
HPM_DEFINE_ATTRIBUTE(HRFAttributeWhiteBalance,                 uint16_t,               HRF_ATTRIBUTEID_WhiteBalance,                 "WhiteBalance");
HPM_DEFINE_ATTRIBUTE(HRFAttributeDigitalZoomRatio,             double,               HRF_ATTRIBUTEID_DigitalZoomRatio,             "DigitalZoomRatio");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFocalLengthIn35mmFilm,        uint16_t,               HRF_ATTRIBUTEID_FocalLengthIn35mmFilm,        "FocalLengthIn35mmFilm");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSceneCaptureType,             uint16_t,               HRF_ATTRIBUTEID_SceneCaptureType,             "SceneCaptureType");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGainControl,                  uint16_t,               HRF_ATTRIBUTEID_GainControl,                  "GainControl");
HPM_DEFINE_ATTRIBUTE(HRFAttributeContrast,                     uint16_t,               HRF_ATTRIBUTEID_Contrast,                     "Contrast");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSaturation,                   uint16_t,               HRF_ATTRIBUTEID_Saturation,                   "Saturation");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSharpness,                    uint16_t,               HRF_ATTRIBUTEID_Sharpness,                    "Sharpness");
HPM_DEFINE_ATTRIBUTE(HRFAttributeDeviceSettingDescription,     vector<Byte>,         HRF_ATTRIBUTEID_DeviceSettingDescription,     "DeviceSettingDescription");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSubjectDistanceRange,         uint16_t,               HRF_ATTRIBUTEID_SubjectDistanceRange,         "SubjectDistanceRange");
HPM_DEFINE_ATTRIBUTE(HRFAttributeImageUniqueID,                Utf8String,              HRF_ATTRIBUTEID_ImageUniqueID,                "ImageUniqueID");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGIFApplicationCode,           string,               HRF_ATTRIBUTEID_GIF_ApplicationCode,          "Application Code");
// HRFPngFile.h
HPM_DEFINE_ATTRIBUTE(HRFAttributeImageGamma,                   double,               HRF_ATTRIBUTEID_ImageGamma,                   "Image Gamma");
HPM_DEFINE_ATTRIBUTE(HRFAttributeTimeModification,             Utf8String,              HRF_ATTRIBUTEID_TimeModification,             "Time Modification");
// HRFHGRPageFile
HPM_DEFINE_ATTRIBUTE(HRFAttributeHGRFile,                      string,               HRF_ATTRIBUTEID_HGR_File,                     "File");
HPM_DEFINE_ATTRIBUTE(HRFAttributeHGRVersion,                   string,               HRF_ATTRIBUTEID_HGR_Version,                  "Version");
HPM_DEFINE_ATTRIBUTE(HRFAttributeHGROwner,                     string,               HRF_ATTRIBUTEID_HGR_Owner,                    "Owner");
HPM_DEFINE_ATTRIBUTE(HRFAttributeHGRDescription,               string,               HRF_ATTRIBUTEID_HGR_Description,              "Description");
// HRSObjectStore
HPM_DEFINE_ATTRIBUTE(HRFAttributeOriginalFileFormat,           Utf8String,              HRF_ATTRIBUTEID_OriginalFileFormat,           "Original File Format");
HPM_DEFINE_ATTRIBUTE(HRFAttributeOriginalFileCompression,      Utf8String,              HRF_ATTRIBUTEID_OriginalFileCompression,      "Original File Compression");
HPM_DEFINE_ATTRIBUTE(HRFAttributeOriginalFileSize,             uint64_t,               HRF_ATTRIBUTEID_OriginalFileSize,             "Original File Size");
HPM_DEFINE_ATTRIBUTE(HRFAttributeOriginalFileModificationDate, uint64_t,               HRF_ATTRIBUTEID_OriginalFileModificationDate, "Original File Modification Date");
// HPSParser
HPM_DEFINE_ATTRIBUTE(HPSAttributeImageDescription,             Utf8String,              HPS_ATTRIBUTEID_ImageDescription,             "HPS_IMAGE_DESCRIPTION");

END_IMAGEPP_NAMESPACE

#include "HPMAttribute.hpp"
