//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMAttribute.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    
    virtual WString GetDataAsString() const = 0;
  
    // A unique name use for internal purpose. Name is not translated.
    virtual WString GetName () const = 0; 

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

    virtual WString GetDataAsString() const override; 

    // A unique name use for internal purpose. Name is not translated.
    virtual WString GetName () const override;

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
    template<> inline WString ClassName::GetName() const {return Name;}

typedef HFCMatrix<4,4,double> _HFCMatrix4x4Double;       // Use a typedef that will remove ',' to satisfy our macro.

// ------------------------------------------------------------------------------------------------------------------------------------------------------
//                   Class Name                                DataType              AttributeID                                   Name
HPM_DEFINE_ATTRIBUTE(HRFAttributeImageDescription,             WString,              HRF_ATTRIBUTEID_ImageDescription,             L"Image Description");
HPM_DEFINE_ATTRIBUTE(HRFAttributePageName,                     WString,              HRF_ATTRIBUTEID_PageName,                     L"Page Name");
HPM_DEFINE_ATTRIBUTE(HRFAttributeDocumentName,                 WString,              HRF_ATTRIBUTEID_DocumentName,                 L"Document Name");
HPM_DEFINE_ATTRIBUTE(HRFAttributeTitle,                        WString,              HRF_ATTRIBUTEID_Title,                        L"Title");
HPM_DEFINE_ATTRIBUTE(HRFAttributeNotes,                        WString,              HRF_ATTRIBUTEID_Notes,                        L"Notes");
HPM_DEFINE_ATTRIBUTE(HRFAttributeKeyWord,                      WString,              HRF_ATTRIBUTEID_KeyWord,                      L"Key Word");
HPM_DEFINE_ATTRIBUTE(HRFAttributeMake,                         WString,              HRF_ATTRIBUTEID_Make,                         L"Make");
HPM_DEFINE_ATTRIBUTE(HRFAttributeModel,                        WString,              HRF_ATTRIBUTEID_Model,                        L"Model");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSoftware,                     WString,              HRF_ATTRIBUTEID_Software,                     L"Software");
HPM_DEFINE_ATTRIBUTE(HRFAttributeVersion,                      WString,              HRF_ATTRIBUTEID_Version,                      L"Version");
HPM_DEFINE_ATTRIBUTE(HRFAttributeCopyright,                    WString,              HRF_ATTRIBUTEID_Copyright,                    L"Copyright");
HPM_DEFINE_ATTRIBUTE(HRFAttributeArtist,                       WString,              HRF_ATTRIBUTEID_Artist,                       L"Artist");
HPM_DEFINE_ATTRIBUTE(HRFAttributeDirector,                     WString,              HRF_ATTRIBUTEID_Director,                     L"Director");
HPM_DEFINE_ATTRIBUTE(HRFAttributeCompany,                      WString,              HRF_ATTRIBUTEID_Company,                      L"Company");
HPM_DEFINE_ATTRIBUTE(HRFAttributeVendor,                       WString,              HRF_ATTRIBUTEID_Vendor,                       L"Vendor");
HPM_DEFINE_ATTRIBUTE(HRFAttributeHostComputer,                 WString,              HRF_ATTRIBUTEID_HostComputer,                 L"Host Computer");
HPM_DEFINE_ATTRIBUTE(HRFAttributeDateTime,                     WString,              HRF_ATTRIBUTEID_DateTime,                     L"Date Time");
HPM_DEFINE_ATTRIBUTE(HRFAttributeInkNames,                     WString,              HRF_ATTRIBUTEID_InkNames,                     L"Ink Names");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSecurityLevel,                WString,              HRF_ATTRIBUTEID_SecurityLevel,                L"Security Level");
HPM_DEFINE_ATTRIBUTE(HRFAttributeLegalDisclaimer,              WString,              HRF_ATTRIBUTEID_LegalDisclaimer,              L"Legal Disclaimer");
HPM_DEFINE_ATTRIBUTE(HRFAttributeContentWarning,               WString,              HRF_ATTRIBUTEID_ContentWarning,               L"Content Warning");
HPM_DEFINE_ATTRIBUTE(HRFAttributeMinSampleValue,               vector<double>,       HRF_ATTRIBUTEID_MinSampleValue,               L"Sample Minimum Value");
HPM_DEFINE_ATTRIBUTE(HRFAttributeMaxSampleValue,               vector<double>,       HRF_ATTRIBUTEID_MaxSampleValue,               L"Sample Maximum Value");
HPM_DEFINE_ATTRIBUTE(HRFAttributeResolutionUnit,               unsigned short,               HRF_ATTRIBUTEID_ResolutionUnit,               L"Resolution Unit");
HPM_DEFINE_ATTRIBUTE(HRFAttributeXResolution,                  double,               HRF_ATTRIBUTEID_XResolution,                  L"X Resolution");
HPM_DEFINE_ATTRIBUTE(HRFAttributeYResolution,                  double,               HRF_ATTRIBUTEID_YResolution,                  L"Y Resolution");
HPM_DEFINE_ATTRIBUTE(HRFAttribute3DTransformationMatrix,       _HFCMatrix4x4Double,  HRF_ATTRIBUTEID_3DTransformationMatrix,       L"3D Tranformation Matrix");
HPM_DEFINE_ATTRIBUTE(HRFAttributeOnDemandRastersInfo,          string,               HRF_ATTRIBUTEID_OnDemandRastersInfo,          L"OnDemand Rasters Info");
HPM_DEFINE_ATTRIBUTE(HRFAttributeImageSlo,                     unsigned short,               HRF_ATTRIBUTEID_ImageSlo,                     L"Slo");
HPM_DEFINE_ATTRIBUTE(HRFAttributeForeground,                   uint32_t,               HRF_ATTRIBUTEID_Foreground,                   L"Foreground");
HPM_DEFINE_ATTRIBUTE(HRFAttributeBackground,                   uint32_t,               HRF_ATTRIBUTEID_Background,                   L"Background");
HPM_DEFINE_ATTRIBUTE(HRFAttributeDontSupportPersistentColor,   bool,                 HRF_ATTRIBUTEID_DontSupportPersistentColor,   L"Persistent Color");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGTModelType,                  unsigned short,               HRF_ATTRIBUTEID_GTModelType,                  L"GTModelType");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGTRasterType,                 unsigned short,               HRF_ATTRIBUTEID_GTRasterType,                 L"GTRasterType");
HPM_DEFINE_ATTRIBUTE(HRFAttributePCSCitation,                  WString,              HRF_ATTRIBUTEID_PCSCitation,                  L"PCSCitation");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjectedCSType,              unsigned short,               HRF_ATTRIBUTEID_ProjectedCSType,              L"ProjectedCSType");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjectedCSTypeLong,          uint32_t,              HRF_ATTRIBUTEID_ProjectedCSTypeLong,          L"ProjectedCSTypeLong");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGTCitation,                   WString,              HRF_ATTRIBUTEID_GTCitation,                   L"GTCitation");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjection,                   unsigned short,               HRF_ATTRIBUTEID_Projection,                   L"Projection");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjCoordTrans,               unsigned short,               HRF_ATTRIBUTEID_ProjCoordTrans,               L"ProjCoordTrans");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjLinearUnits,              unsigned short,               HRF_ATTRIBUTEID_ProjLinearUnits,              L"ProjLinearUnits");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjLinearUnitSize,           double,               HRF_ATTRIBUTEID_ProjLinearUnitSize,           L"ProjLinearUnitSize");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeographicType,               unsigned short,               HRF_ATTRIBUTEID_GeographicType,               L"GeographicType");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogCitation,                 WString,              HRF_ATTRIBUTEID_GeogCitation,                 L"GeogCitation");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogGeodeticDatum,            unsigned short,               HRF_ATTRIBUTEID_GeogGeodeticDatum,            L"GeogGeodeticDatum");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogPrimeMeridian,            unsigned short,               HRF_ATTRIBUTEID_GeogPrimeMeridian,            L"GeogPrimeMeridian");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogLinearUnits,              unsigned short,               HRF_ATTRIBUTEID_GeogLinearUnits,              L"GeogLinearUnits");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogLinearUnitSize,           double,               HRF_ATTRIBUTEID_GeogLinearUnitSize,           L"GeogLinearUnitSize");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogAngularUnits,             unsigned short,               HRF_ATTRIBUTEID_GeogAngularUnits,             L"GeogAngularUnits");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogAngularUnitSize,          double,               HRF_ATTRIBUTEID_GeogAngularUnitSize,          L"GeogAngularUnitSize");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogEllipsoid,                unsigned short,               HRF_ATTRIBUTEID_GeogEllipsoid,                L"GeogEllipsoid");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogSemiMajorAxis,            double,               HRF_ATTRIBUTEID_GeogSemiMajorAxis,            L"GeogSemiMajorAxis");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogSemiMinorAxis,            double,               HRF_ATTRIBUTEID_GeogSemiMinorAxis,            L"GeogSemiMinorAxis");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogInvFlattening,            double,               HRF_ATTRIBUTEID_GeogInvFlattening,            L"GeogInvFlattening");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogAzimuthUnits,             unsigned short,               HRF_ATTRIBUTEID_GeogAzimuthUnits,             L"GeogAzimuthUnits");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGeogPrimeMeridianLong,        double,               HRF_ATTRIBUTEID_GeogPrimeMeridianLong,        L"GeogPrimeMeridianLong");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjStdParallel1,             double,               HRF_ATTRIBUTEID_ProjStdParallel1,             L"ProjStdParallel1");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjStdParallel2,             double,               HRF_ATTRIBUTEID_ProjStdParallel2,             L"ProjStdParallel2");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjNatOriginLong,            double,               HRF_ATTRIBUTEID_ProjNatOriginLong,            L"ProjNatOriginLong");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjNatOriginLat,             double,               HRF_ATTRIBUTEID_ProjNatOriginLat,             L"ProjNatOriginLat");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjFalseEasting,             double,               HRF_ATTRIBUTEID_ProjFalseEasting,             L"ProjFalseEasting");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjFalseNorthing,            double,               HRF_ATTRIBUTEID_ProjFalseNorthing,            L"ProjFalseNorthing");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjFalseOriginLong,          double,               HRF_ATTRIBUTEID_ProjFalseOriginLong,          L"ProjFalseOriginLong");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjFalseOriginLat,           double,               HRF_ATTRIBUTEID_ProjFalseOriginLat,           L"ProjFalseOriginLat");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjFalseOriginEasting,       double,               HRF_ATTRIBUTEID_ProjFalseOriginEasting,       L"ProjFalseOriginEasting");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjFalseOriginNorthing,      double,               HRF_ATTRIBUTEID_ProjFalseOriginNorthing,      L"ProjFalseOriginNorthing");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjCenterLong,               double,               HRF_ATTRIBUTEID_ProjCenterLong,               L"ProjCenterLong");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjCenterLat,                double,               HRF_ATTRIBUTEID_ProjCenterLat,                L"ProjCenterLat");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjCenterEasting,            double,               HRF_ATTRIBUTEID_ProjCenterEasting,            L"ProjCenterEasting");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjCenterNorthing,           double,               HRF_ATTRIBUTEID_ProjCenterNorthing,           L"ProjCenterNorthing");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjScaleAtNatOrigin,         double,               HRF_ATTRIBUTEID_ProjScaleAtNatOrigin,         L"ProjScaleAtNatOrigin");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjScaleAtCenter,            double,               HRF_ATTRIBUTEID_ProjScaleAtCenter,            L"ProjScaleAtCenter");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjAzimuthAngle,             double,               HRF_ATTRIBUTEID_ProjAzimuthAngle,             L"ProjAzimuthAngle");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjStraightVertPoleLong,     double,               HRF_ATTRIBUTEID_ProjStraightVertPoleLong,     L"ProjStraightVertPoleLong");
HPM_DEFINE_ATTRIBUTE(HRFAttributeProjRectifiedGridAngle,       double,               HRF_ATTRIBUTEID_ProjRectifiedGridAngle,       L"ProjRectifiedGridAngle");
HPM_DEFINE_ATTRIBUTE(HRFAttributeVerticalCSType,               unsigned short,               HRF_ATTRIBUTEID_VerticalCSType,               L"VerticalCSType");
HPM_DEFINE_ATTRIBUTE(HRFAttributeVerticalCitation,             WString,              HRF_ATTRIBUTEID_VerticalCitation,             L"VerticalCitation");
HPM_DEFINE_ATTRIBUTE(HRFAttributeVerticalDatum,                unsigned short,               HRF_ATTRIBUTEID_VerticalDatum,                L"VerticalDatum");
HPM_DEFINE_ATTRIBUTE(HRFAttributeVerticalUnits,                unsigned short,               HRF_ATTRIBUTEID_VerticalUnits,                L"VerticalUnits");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSVersionID,                 vector<Byte>,         HRF_ATTRIBUTEID_GPSVersionID,                 L"GPSVersionID");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSLatitudeRef,               WString,              HRF_ATTRIBUTEID_GPSLatitudeRef,               L"GPSLatitudeRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSLatitude,                  vector<double>,       HRF_ATTRIBUTEID_GPSLatitude,                  L"GPSLatitude");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSLongitudeRef,              WString,              HRF_ATTRIBUTEID_GPSLongitudeRef,              L"GPSLongitudeRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSLongitude,                 vector<double>,       HRF_ATTRIBUTEID_GPSLongitude,                 L"GPSLongitude");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSAltitudeRef,               Byte,                 HRF_ATTRIBUTEID_GPSAltitudeRef,               L"GPSAltitudeRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSAltitude,                  double,               HRF_ATTRIBUTEID_GPSAltitude,                  L"GPSAltitude");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSTimeStamp,                 vector<double>,       HRF_ATTRIBUTEID_GPSTimeStamp,                 L"GPSTimeStamp");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSSatellites,                WString,              HRF_ATTRIBUTEID_GPSSatellites,                L"GPSSatellites");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSStatus,                    WString,              HRF_ATTRIBUTEID_GPSStatus,                    L"GPSStatus");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSMeasureMode,               WString,              HRF_ATTRIBUTEID_GPSMeasureMode,               L"GPSMeasureMode");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDOP,                       double,               HRF_ATTRIBUTEID_GPSDOP,                       L"GPSDOP");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSSpeedRef,                  WString,              HRF_ATTRIBUTEID_GPSSpeedRef,                  L"GPSSpeedRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSSpeed,                     double,               HRF_ATTRIBUTEID_GPSSpeed,                     L"GPSSpeed");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSTrackRef,                  WString,              HRF_ATTRIBUTEID_GPSTrackRef,                  L"GPSTrackRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSTrack,                     double,               HRF_ATTRIBUTEID_GPSTrack,                     L"GPSTrack");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSImgDirectionRef,           WString,              HRF_ATTRIBUTEID_GPSImgDirectionRef,           L"GPSImgDirectionRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSImgDirection,              double,               HRF_ATTRIBUTEID_GPSImgDirection,              L"GPSImgDirection");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSMapDatum,                  WString,              HRF_ATTRIBUTEID_GPSMapDatum,                  L"GPSMapDatum");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDestLatitudeRef,           WString,              HRF_ATTRIBUTEID_GPSDestLatitudeRef,           L"GPSDestLatitudeRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDestLatitude,              vector<double>,       HRF_ATTRIBUTEID_GPSDestLatitude,              L"GPSDestLatitude");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDestLongitudeRef,          WString,              HRF_ATTRIBUTEID_GPSDestLongitudeRef,          L"GPSDestLongitudeRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDestLongitude,             vector<double>,       HRF_ATTRIBUTEID_GPSDestLongitude,             L"GPSDestLongitude");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDestBearingRef,            WString,              HRF_ATTRIBUTEID_GPSDestBearingRef,            L"GPSDestBearingRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDestBearing,               double,               HRF_ATTRIBUTEID_GPSDestBearing,               L"GPSDestBearing");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDestDistanceRef,           WString,              HRF_ATTRIBUTEID_GPSDestDistanceRef,           L"GPSDestDistanceRef");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDestDistance,              double,               HRF_ATTRIBUTEID_GPSDestDistance,              L"GPSDestDistance");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSProcessingMethod,          vector<Byte>,         HRF_ATTRIBUTEID_GPSProcessingMethod,          L"GPSProcessingMethod");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSAreaInformation,           vector<Byte>,         HRF_ATTRIBUTEID_GPSAreaInformation,           L"GPSAreaInformation");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDateStamp,                 WString,              HRF_ATTRIBUTEID_GPSDateStamp,                 L"GPSDateStamp");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGPSDifferential,              unsigned short,               HRF_ATTRIBUTEID_GPSDifferential,              L"GPSDifferential");
HPM_DEFINE_ATTRIBUTE(HRFAttributeExposureTime,                 double,               HRF_ATTRIBUTEID_ExposureTime,                 L"ExposureTime");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFNumber,                      double,               HRF_ATTRIBUTEID_FNumber,                      L"FNumber");
HPM_DEFINE_ATTRIBUTE(HRFAttributeExposureProgram,              unsigned short,               HRF_ATTRIBUTEID_ExposureProgram,              L"ExposureProgram");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSpectralSensitivity,          WString,              HRF_ATTRIBUTEID_SpectralSensitivity,          L"SpectralSensitivity");
HPM_DEFINE_ATTRIBUTE(HRFAttributeISOSpeedRatings,              vector<unsigned short>,       HRF_ATTRIBUTEID_ISOSpeedRatings,              L"ISOSpeedRatings");
HPM_DEFINE_ATTRIBUTE(HRFAttributeOptoElectricConversionFactor, vector<Byte>,         HRF_ATTRIBUTEID_OptoElectricConversionFactor, L"OptoelectricConversionFactor");
HPM_DEFINE_ATTRIBUTE(HRFAttributeExifVersion,                  vector<char>,         HRF_ATTRIBUTEID_ExifVersion,                  L"ExifVersion");
HPM_DEFINE_ATTRIBUTE(HRFAttributeDateTimeOriginal,             WString,              HRF_ATTRIBUTEID_DateTimeOriginal,             L"DateTimeOriginal");
HPM_DEFINE_ATTRIBUTE(HRFAttributeDateTimeDigitized,            WString,              HRF_ATTRIBUTEID_DateTimeDigitized,            L"DateTimeDigitized");
HPM_DEFINE_ATTRIBUTE(HRFAttributeComponentsConfiguration,      vector<Byte>,         HRF_ATTRIBUTEID_ComponentsConfiguration,      L"ComponentsConfiguration");
HPM_DEFINE_ATTRIBUTE(HRFAttributeCompressedBitsPerPixel,       double,               HRF_ATTRIBUTEID_CompressedBitsPerPixel,       L"CompressedBitsPerPixel");
HPM_DEFINE_ATTRIBUTE(HRFAttributeShutterSpeedValue,            double,               HRF_ATTRIBUTEID_ShutterSpeedValue,            L"ShutterSpeedValue");
HPM_DEFINE_ATTRIBUTE(HRFAttributeApertureValue,                double,               HRF_ATTRIBUTEID_ApertureValue,                L"ApertureValue");
HPM_DEFINE_ATTRIBUTE(HRFAttributeBrightnessValue,              double,               HRF_ATTRIBUTEID_BrightnessValue,              L"BrightnessValue");
HPM_DEFINE_ATTRIBUTE(HRFAttributeExposureBiasValue,            double,               HRF_ATTRIBUTEID_ExposureBiasValue,            L"ExposureBiasValue");
HPM_DEFINE_ATTRIBUTE(HRFAttributeMaxApertureValue,             double,               HRF_ATTRIBUTEID_MaxApertureValue,             L"MaxApertureValue");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSubjectDistance,              double,               HRF_ATTRIBUTEID_SubjectDistance,              L"SubjectDistance");
HPM_DEFINE_ATTRIBUTE(HRFAttributeMeteringMode,                 unsigned short,               HRF_ATTRIBUTEID_MeteringMode,                 L"MeteringMode");
HPM_DEFINE_ATTRIBUTE(HRFAttributeLightSource,                  unsigned short,               HRF_ATTRIBUTEID_LightSource,                  L"LightSource");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFlash,                        unsigned short,               HRF_ATTRIBUTEID_Flash,                        L"Flash");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFocalLength,                  double,               HRF_ATTRIBUTEID_FocalLength,                  L"FocalLength");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSubjectArea,                  vector<unsigned short>,       HRF_ATTRIBUTEID_SubjectArea,                  L"SubjectArea");
HPM_DEFINE_ATTRIBUTE(HRFAttributeMakerNote,                    vector<Byte>,         HRF_ATTRIBUTEID_MakerNote,                    L"MakerNote");
HPM_DEFINE_ATTRIBUTE(HRFAttributeUserComment,                  vector<Byte>,         HRF_ATTRIBUTEID_UserComment,                  L"UserComment");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSubSecTime,                   WString,              HRF_ATTRIBUTEID_SubSecTime,                   L"SubSecTime");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSubSecTimeOriginal,           WString,              HRF_ATTRIBUTEID_SubSecTimeOriginal,           L"SubSecTimeOriginal");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSubSecTimeDigitized,          WString,              HRF_ATTRIBUTEID_SubSecTimeDigitized,          L"SubSecTimeDigitized");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFlashpixVersion,              vector<Byte>,         HRF_ATTRIBUTEID_FlashpixVersion,              L"FlashpixVersion");
HPM_DEFINE_ATTRIBUTE(HRFAttributeColorSpace,                   unsigned short,               HRF_ATTRIBUTEID_ColorSpace,                   L"ColorSpace");
HPM_DEFINE_ATTRIBUTE(HRFAttributePixelXDimension,              uint32_t,              HRF_ATTRIBUTEID_PixelXDimension,              L"PixelXDimension");
HPM_DEFINE_ATTRIBUTE(HRFAttributePixelYDimension,              uint32_t,              HRF_ATTRIBUTEID_PixelYDimension,              L"PixelYDimension");
HPM_DEFINE_ATTRIBUTE(HRFAttributeRelatedSoundFile,             WString,              HRF_ATTRIBUTEID_RelatedSoundFile,             L"RelatedSoundFile");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFlashEnergy,                  double,               HRF_ATTRIBUTEID_FlashEnergy,                  L"FlashEnergy");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSpatialFrequencyResponse,     vector<Byte>,         HRF_ATTRIBUTEID_SpatialFrequencyResponse,     L"SpatialFrequencyResponse");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFocalPlaneXResolution,        double,               HRF_ATTRIBUTEID_FocalPlaneXResolution,        L"FocalPlaneXResolution");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFocalPlaneYResolution,        double,               HRF_ATTRIBUTEID_FocalPlaneYResolution,        L"FocalPlaneYResolution");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFocalPlaneResolutionUnit,     unsigned short,               HRF_ATTRIBUTEID_FocalPlaneResolutionUnit,     L"FocalPlaneResolutionUnit");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSubjectLocation,              vector<unsigned short>,       HRF_ATTRIBUTEID_SubjectLocation,              L"SubjectLocation");
HPM_DEFINE_ATTRIBUTE(HRFAttributeExposureIndex,                double,               HRF_ATTRIBUTEID_ExposureIndex,                L"ExposureIndex");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSensingMethod,                unsigned short,               HRF_ATTRIBUTEID_SensingMethod,                L"SensingMethod");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFileSource,                   Byte,                 HRF_ATTRIBUTEID_FileSource,                   L"FileSource");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSceneType,                    Byte,                 HRF_ATTRIBUTEID_SceneType,                    L"SceneType");
HPM_DEFINE_ATTRIBUTE(HRFAttributeCFAPattern,                   vector<Byte>,         HRF_ATTRIBUTEID_CFAPattern,                   L"CFAPattern");
HPM_DEFINE_ATTRIBUTE(HRFAttributeCustomRendered,               unsigned short,               HRF_ATTRIBUTEID_CustomRendered,               L"CustomRendered");
HPM_DEFINE_ATTRIBUTE(HRFAttributeExposureMode,                 unsigned short,               HRF_ATTRIBUTEID_ExposureMode,                 L"ExposureMode");
HPM_DEFINE_ATTRIBUTE(HRFAttributeWhiteBalance,                 unsigned short,               HRF_ATTRIBUTEID_WhiteBalance,                 L"WhiteBalance");
HPM_DEFINE_ATTRIBUTE(HRFAttributeDigitalZoomRatio,             double,               HRF_ATTRIBUTEID_DigitalZoomRatio,             L"DigitalZoomRatio");
HPM_DEFINE_ATTRIBUTE(HRFAttributeFocalLengthIn35mmFilm,        unsigned short,               HRF_ATTRIBUTEID_FocalLengthIn35mmFilm,        L"FocalLengthIn35mmFilm");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSceneCaptureType,             unsigned short,               HRF_ATTRIBUTEID_SceneCaptureType,             L"SceneCaptureType");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGainControl,                  unsigned short,               HRF_ATTRIBUTEID_GainControl,                  L"GainControl");
HPM_DEFINE_ATTRIBUTE(HRFAttributeContrast,                     unsigned short,               HRF_ATTRIBUTEID_Contrast,                     L"Contrast");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSaturation,                   unsigned short,               HRF_ATTRIBUTEID_Saturation,                   L"Saturation");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSharpness,                    unsigned short,               HRF_ATTRIBUTEID_Sharpness,                    L"Sharpness");
HPM_DEFINE_ATTRIBUTE(HRFAttributeDeviceSettingDescription,     vector<Byte>,         HRF_ATTRIBUTEID_DeviceSettingDescription,     L"DeviceSettingDescription");
HPM_DEFINE_ATTRIBUTE(HRFAttributeSubjectDistanceRange,         unsigned short,               HRF_ATTRIBUTEID_SubjectDistanceRange,         L"SubjectDistanceRange");
HPM_DEFINE_ATTRIBUTE(HRFAttributeImageUniqueID,                WString,              HRF_ATTRIBUTEID_ImageUniqueID,                L"ImageUniqueID");
HPM_DEFINE_ATTRIBUTE(HRFAttributeGIFApplicationCode,           string,               HRF_ATTRIBUTEID_GIF_ApplicationCode,          L"Application Code");
// HRFPngFile.h
HPM_DEFINE_ATTRIBUTE(HRFAttributeImageGamma,                   double,               HRF_ATTRIBUTEID_ImageGamma,                   L"Image Gamma");
HPM_DEFINE_ATTRIBUTE(HRFAttributeTimeModification,             WString,              HRF_ATTRIBUTEID_TimeModification,             L"Time Modification");
// HRFHGRPageFile
HPM_DEFINE_ATTRIBUTE(HRFAttributeHGRFile,                      string,               HRF_ATTRIBUTEID_HGR_File,                     L"File");
HPM_DEFINE_ATTRIBUTE(HRFAttributeHGRVersion,                   string,               HRF_ATTRIBUTEID_HGR_Version,                  L"Version");
HPM_DEFINE_ATTRIBUTE(HRFAttributeHGROwner,                     string,               HRF_ATTRIBUTEID_HGR_Owner,                    L"Owner");
HPM_DEFINE_ATTRIBUTE(HRFAttributeHGRDescription,               string,               HRF_ATTRIBUTEID_HGR_Description,              L"Description");
// HRSObjectStore
HPM_DEFINE_ATTRIBUTE(HRFAttributeOriginalFileFormat,           WString,              HRF_ATTRIBUTEID_OriginalFileFormat,           L"Original File Format");
HPM_DEFINE_ATTRIBUTE(HRFAttributeOriginalFileCompression,      WString,              HRF_ATTRIBUTEID_OriginalFileCompression,      L"Original File Compression");
HPM_DEFINE_ATTRIBUTE(HRFAttributeOriginalFileSize,             uint64_t,               HRF_ATTRIBUTEID_OriginalFileSize,             L"Original File Size");
HPM_DEFINE_ATTRIBUTE(HRFAttributeOriginalFileModificationDate, uint64_t,               HRF_ATTRIBUTEID_OriginalFileModificationDate, L"Original File Modification Date");
// HPSParser
HPM_DEFINE_ATTRIBUTE(HPSAttributeImageDescription,             WString,              HPS_ATTRIBUTEID_ImageDescription,             L"HPS_IMAGE_DESCRIPTION");

END_IMAGEPP_NAMESPACE

#include "HPMAttribute.hpp"
