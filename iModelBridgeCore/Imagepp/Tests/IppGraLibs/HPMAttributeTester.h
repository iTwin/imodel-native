//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HPMAttributeTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPMAttributeTester
//-----------------------------------------------------------------------------

#pragma once

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())
 
// Preparation of required environement
class HPMAttributeTester : public testing::Test
    {   

protected :
  
    HPMAttributeTester();
    ~HPMAttributeTester() {};
    
    //Default Value
    int32_t                 i;
    int32_t                 j;

    HFCMatrix<4,4, double>  Matrix;
    vector<Byte>            VectorByte;
    vector<double>          VectorDouble;
    vector<unsigned short>          VectorUShort;
    vector<char>            VectorChar;

    HFCMatrix<4,4, double>  MatrixTest;
    vector<Byte>            VectorByteTest;
    vector<double>          VectorDoubleTest;
    vector<unsigned short>          VectorUShortTest;
    vector<char>            VectorCharTest;

    string                  StringValue;
    double                  DoubleValue;
    unsigned short          UShortValue;
    uint32_t                UInt32Value;
    Byte                    ByteValue;
    uint32_t                 uint32_tValue;
    uint64_t                UInt64Value;
    WString                 WStringValue;

    //HPMGenericAttribute
    HFCPtr<HPMGenericAttribute>  pHRFAttributeImageDescription;
    HFCPtr<HPMGenericAttribute>  pHRFAttributePageName;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeDocumentName;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeTitle;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeNotes;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeKeyWord;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeMake;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeModel;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeSoftware;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeVersion;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeCopyright;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeArtist;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeDirector;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeCompany;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeVendor;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeHostComputer;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeDateTime;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeInkNames;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeSecurityLevel;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeLegalDisclaimer;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeContentWarning;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeMinSampleValue;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeMaxSampleValue;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeResolutionUnit;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeXResolution;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeYResolution;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeImageSlo;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeForeground;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeBackground;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGTModelType;
    HFCPtr<HPMGenericAttribute>  pHRFAttributePCSCitation;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjectedCSType;  
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGTCitation;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjection;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjCoordTrans;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjLinearUnits;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGeographicType;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGeogCitation;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGeogLinearUnits;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGeogEllipsoid;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGeogAzimuthUnits;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjStdParallel1;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjStdParallel2;  
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjNatOriginLat;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjFalseEasting;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjCenterLong;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjCenterLat;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjAzimuthAngle;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeVerticalCSType;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeVerticalCitation;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeVerticalDatum;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeVerticalUnits;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSVersionID;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSLatitudeRef;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSLatitude;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSLongitudeRef;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSLongitude;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSAltitudeRef;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSAltitude;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSTimeStamp;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSSatellites;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSStatus;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSMeasureMode;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSDOP;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSSpeedRef;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSSpeed;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSTrackRef;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSTrack;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSImgDirection;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSMapDatum;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSDestLatitude;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSDestLongitude;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSDestDistance;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSDestBearing;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSDateStamp;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSDifferential;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeExposureTime;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeFNumber;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeExposureProgram;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeISOSpeedRatings;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeExifVersion;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeDateTimeOriginal;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeApertureValue;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeBrightnessValue;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeMaxApertureValue;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeSubjectDistance;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeMeteringMode;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeLightSource;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeFlash;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeFocalLength;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeSubjectArea;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeMakerNote;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeUserComment;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeSubSecTime;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeFlashpixVersion;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeColorSpace;
    HFCPtr<HPMGenericAttribute>  pHRFAttributePixelXDimension;
    HFCPtr<HPMGenericAttribute>  pHRFAttributePixelYDimension;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeRelatedSoundFile;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeFlashEnergy;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeSubjectLocation;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeExposureIndex;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeSensingMethod;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeFileSource;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeSceneType;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeCFAPattern;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeCustomRendered;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeExposureMode;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeWhiteBalance;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeDigitalZoomRatio;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeSceneCaptureType;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGainControl;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeContrast;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeSaturation;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeSharpness;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeImageUniqueID; 
    HFCPtr<HPMGenericAttribute>  pHRFAttributeImageGamma;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeTimeModification;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeHGRFile;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeHGRVersion;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeHGROwner;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeHGRDescription;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeOriginalFileSize;
    HFCPtr<HPMGenericAttribute>  pHPSAttributeImageDescription;

    HFCPtr<HPMGenericAttribute>  pHRFAttributeOriginalFileFormat;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeOriginalFileCompression;   
    HFCPtr<HPMGenericAttribute>  pHRFAttributeSubjectDistanceRange;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGIFApplicationCode;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeDeviceSettingDescription;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeFocalLengthIn35mmFilm;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeSpatialFrequencyResponse;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeFocalPlaneXResolution;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeFocalPlaneYResolution;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeFocalPlaneResolutionUnit;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeSubSecTimeDigitized;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeSubSecTimeOriginal;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeExposureBiasValue; 
    HFCPtr<HPMGenericAttribute>  pHRFAttributeDateTimeDigitized;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeComponentsConfiguration;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeCompressedBitsPerPixel;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeShutterSpeedValue;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGeogGeodeticDatum;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGeogPrimeMeridian;
    HFCPtr<HPMGenericAttribute>  pHRFAttribute3DTranformationMatrix;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeOnDemandRastersInfo;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjectedCSTypeLong;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeDontSupportPersistentColor;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjLinearUnitSize;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGeogLinearUnitSize;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGeogAngularUnits;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGeogAngularUnitSize;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGeogSemiMajorAxis;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGeogSemiMinorAxis;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGeogInvFlattening;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGeogPrimeMeridianLong;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjNatOriginLong;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjFalseNorthing;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjFalseOriginLong;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjFalseOriginLat;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjFalseOriginEasting;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjFalseOriginNorthing;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjCenterEasting;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjCenterNorthing;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjScaleAtNatOrigin;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjScaleAtCenter; 
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjStraightVertPoleLong;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeProjRectifiedGridAngle;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSImgDirectionRef;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSDestLatitudeRef;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSDestLongitudeRef;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSDestBearingRef;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSDestDistanceRef;  
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSProcessingMethod;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeGPSAreaInformation;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeSpectralSensitivity;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeOptoelectricConversionFactor;
    HFCPtr<HPMGenericAttribute>  pHRFAttributeOriginalFileModificationDate;

    //HPMAttribute_T
    HRFAttributeImageDescription*               pImageDescription;
    HRFAttributeMinSampleValue*                    pSampleValue;
    HRFAttributeResolutionUnit*                 pResolutionUnit;
    HRFAttribute3DTransformationMatrix*         pTransfoMatrix;
    HRFAttributeForeground*                     pForeground;
    HRFAttributeYResolution*                    pYResolution;
    HRFAttributeProjectedCSTypeLong*            pProjectedLong;
    HRFAttributeGPSVersionID*                   pGPSVersionID;
    HRFAttributeGPSAltitudeRef*                 pGPSAltitudeRef;
    HRFAttributeISOSpeedRatings*                pISOSpeedRatings;
    HRFAttributeExifVersion*                    pExifVersion;
    HRFAttributeOriginalFileFormat*             pOFileFormat;
    HRFAttributeOriginalFileSize*               pOFileSize;
    HRFAttributeDontSupportPersistentColor*     pColor;

    };