//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HPMAttributeTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HPMAttributeTester.h"

HPMAttributeTester::HPMAttributeTester() 
    {

    //Default Value
    for(i = 0 ; i < 4 ; ++i)
        for(j = 0; j < 4 ; ++j)
            Matrix[i][j] = i+j;

    VectorByte.push_back(8);
    VectorByte.push_back(9);

    VectorDouble.push_back(8.0);
    VectorDouble.push_back(9.0);

    VectorUShort.push_back(8);
    VectorUShort.push_back(9);

    VectorChar.push_back('8');
    VectorChar.push_back('9');

    for(i = 0 ; i < 4 ; ++i)
        for(j = 0; j < 4 ; ++j)
            MatrixTest[i][j] = 12;

    VectorByteTest.push_back(1);
    VectorByteTest.push_back(2);

    VectorDoubleTest.push_back(1.0);
    VectorDoubleTest.push_back(2.0);

    VectorUShortTest.push_back(1);
    VectorUShortTest.push_back(2);

    VectorCharTest.push_back('8');
    VectorCharTest.push_back('9');

    StringValue =   "Test";
    DoubleValue =   8.0;
    UShortValue =   8;
    UInt32Value =   8;
    ByteValue =     8;
    uint32_tValue =  8;
    UInt64Value =   8;
    WStringValue =  L"Test";

    //HPMGenericAttribute
    pHRFAttributeImageDescription =   new HRFAttributeImageDescription(WStringValue);
    pHRFAttributePageName =           new HRFAttributePageName(WStringValue); 
    pHRFAttributeDocumentName =       new HRFAttributeDocumentName(WStringValue);
    pHRFAttributeTitle =              new HRFAttributeTitle(WStringValue); 
    pHRFAttributeNotes =              new HRFAttributeNotes(WStringValue);
    pHRFAttributeKeyWord =            new HRFAttributeKeyWord(WStringValue);
    pHRFAttributeMake =               new HRFAttributeMake(WStringValue);
    pHRFAttributeModel =              new HRFAttributeModel(WStringValue);
    pHRFAttributeSoftware =           new HRFAttributeSoftware(WStringValue);
    pHRFAttributeVersion =            new HRFAttributeVersion(WStringValue);
    pHRFAttributeCopyright =          new HRFAttributeCopyright(WStringValue);
    pHRFAttributeArtist =             new HRFAttributeArtist(WStringValue);
    pHRFAttributeDirector =           new HRFAttributeDirector(WStringValue);
    pHRFAttributeCompany =            new HRFAttributeCompany(WStringValue);
    pHRFAttributeVendor =             new HRFAttributeVendor(WStringValue);
    pHRFAttributeHostComputer =       new HRFAttributeHostComputer(WStringValue);
    pHRFAttributeDateTime =           new HRFAttributeDateTime(WStringValue);
    pHRFAttributeInkNames =           new HRFAttributeInkNames(WStringValue);
    pHRFAttributeSecurityLevel =      new HRFAttributeSecurityLevel(WStringValue);
    pHRFAttributeLegalDisclaimer =    new HRFAttributeLegalDisclaimer(WStringValue);
    pHRFAttributeContentWarning =     new HRFAttributeContentWarning(WStringValue);
    pHRFAttributeMinSampleValue =     new HRFAttributeMinSampleValue(VectorDouble);
    pHRFAttributeMaxSampleValue =     new HRFAttributeMaxSampleValue(VectorDouble);
    pHRFAttributeResolutionUnit =     new HRFAttributeResolutionUnit(UShortValue);
    pHRFAttributeXResolution =        new HRFAttributeXResolution(DoubleValue);
    pHRFAttributeYResolution =        new HRFAttributeYResolution(DoubleValue);
    pHRFAttributeImageSlo =           new HRFAttributeImageSlo(UShortValue);
    pHRFAttributeForeground =         new HRFAttributeForeground(UInt32Value);
    pHRFAttributeBackground =         new HRFAttributeBackground(UInt32Value);
    pHRFAttributeGTModelType =        new HRFAttributeGTModelType(UShortValue);
    pHRFAttributePCSCitation =        new HRFAttributePCSCitation(WStringValue);
    pHRFAttributeProjectedCSType =    new HRFAttributeProjectedCSType(UShortValue);   
    pHRFAttributeGTCitation =         new HRFAttributeGTCitation(WStringValue);
    pHRFAttributeProjection =         new HRFAttributeProjection(UShortValue);
    pHRFAttributeProjCoordTrans =     new HRFAttributeProjCoordTrans(UShortValue);
    pHRFAttributeProjLinearUnits =    new HRFAttributeProjLinearUnits(UShortValue);
    pHRFAttributeGeographicType =     new HRFAttributeGeographicType(UShortValue);
    pHRFAttributeGeogCitation =       new HRFAttributeGeogCitation(WStringValue);
    pHRFAttributeGeogLinearUnits =    new HRFAttributeGeogLinearUnits(UShortValue);
    pHRFAttributeGeogEllipsoid =      new HRFAttributeGeogEllipsoid(UShortValue);
    pHRFAttributeGeogAzimuthUnits =   new HRFAttributeGeogAzimuthUnits(UShortValue);
    pHRFAttributeProjStdParallel1 =   new HRFAttributeProjStdParallel1(DoubleValue);
    pHRFAttributeProjStdParallel2 =   new HRFAttributeProjStdParallel2(DoubleValue);   
    pHRFAttributeProjNatOriginLat =   new HRFAttributeProjNatOriginLat(DoubleValue);
    pHRFAttributeProjFalseEasting =   new HRFAttributeProjFalseEasting(DoubleValue);
    pHRFAttributeProjCenterLong =     new HRFAttributeProjCenterLong(DoubleValue);
    pHRFAttributeProjCenterLat =      new HRFAttributeProjCenterLat(DoubleValue);
    pHRFAttributeProjAzimuthAngle =   new HRFAttributeProjAzimuthAngle(DoubleValue);
    pHRFAttributeVerticalCSType =     new HRFAttributeVerticalCSType(UShortValue);
    pHRFAttributeVerticalCitation =   new HRFAttributeVerticalCitation(WStringValue);
    pHRFAttributeVerticalDatum =      new HRFAttributeVerticalDatum(UShortValue);
    pHRFAttributeVerticalUnits =      new HRFAttributeVerticalUnits(UShortValue);
    pHRFAttributeGPSVersionID =       new HRFAttributeGPSVersionID(VectorByte);
    pHRFAttributeGPSLatitudeRef =     new HRFAttributeGPSLatitudeRef(WStringValue);
    pHRFAttributeGPSLatitude =        new HRFAttributeGPSLatitude(VectorDouble);
    pHRFAttributeGPSLongitudeRef =    new HRFAttributeGPSLongitudeRef(WStringValue);
    pHRFAttributeGPSLongitude =       new HRFAttributeGPSLongitude(VectorDouble);
    pHRFAttributeGPSAltitudeRef =     new HRFAttributeGPSAltitudeRef(ByteValue); 
    pHRFAttributeGPSAltitude =        new HRFAttributeGPSAltitude(DoubleValue);
    pHRFAttributeGPSTimeStamp =       new HRFAttributeGPSTimeStamp(VectorDouble);
    pHRFAttributeGPSSatellites =      new HRFAttributeGPSSatellites(WStringValue);
    pHRFAttributeGPSStatus =          new HRFAttributeGPSStatus(WStringValue);
    pHRFAttributeGPSMeasureMode =     new HRFAttributeGPSMeasureMode(WStringValue);
    pHRFAttributeGPSDOP =             new HRFAttributeGPSDOP(DoubleValue);
    pHRFAttributeGPSSpeedRef =        new HRFAttributeGPSSpeedRef(WStringValue);
    pHRFAttributeGPSSpeed =           new HRFAttributeGPSSpeed(DoubleValue);
    pHRFAttributeGPSTrackRef =        new HRFAttributeGPSTrackRef(WStringValue);
    pHRFAttributeGPSTrack =           new HRFAttributeGPSTrack(DoubleValue);
    pHRFAttributeGPSImgDirection =    new HRFAttributeGPSImgDirection(DoubleValue);
    pHRFAttributeGPSMapDatum =        new HRFAttributeGPSMapDatum(WStringValue);
    pHRFAttributeGPSDestLatitude =    new HRFAttributeGPSDestLatitude(VectorDouble);
    pHRFAttributeGPSDestLongitude =   new HRFAttributeGPSDestLongitude(VectorDouble);
    pHRFAttributeGPSDestDistance =    new HRFAttributeGPSDestDistance(DoubleValue);
    pHRFAttributeGPSDestBearing =     new HRFAttributeGPSDestBearing(DoubleValue);
    pHRFAttributeGPSDateStamp =       new HRFAttributeGPSDateStamp(WStringValue);
    pHRFAttributeGPSDifferential =    new HRFAttributeGPSDifferential(UShortValue);
    pHRFAttributeExposureTime =       new HRFAttributeExposureTime(DoubleValue);
    pHRFAttributeFNumber =            new HRFAttributeFNumber(DoubleValue);
    pHRFAttributeExposureProgram =    new HRFAttributeExposureProgram(UShortValue);
    pHRFAttributeISOSpeedRatings =    new HRFAttributeISOSpeedRatings(VectorUShort);
    pHRFAttributeExifVersion =        new HRFAttributeExifVersion(VectorChar);
    pHRFAttributeDateTimeOriginal =   new HRFAttributeDateTimeOriginal(WStringValue); 
    pHRFAttributeApertureValue =      new HRFAttributeApertureValue(DoubleValue);
    pHRFAttributeBrightnessValue =    new HRFAttributeBrightnessValue(DoubleValue); 
    pHRFAttributeMaxApertureValue =   new HRFAttributeMaxApertureValue(DoubleValue);
    pHRFAttributeSubjectDistance =    new HRFAttributeSubjectDistance(DoubleValue);
    pHRFAttributeMeteringMode =       new HRFAttributeMeteringMode(UShortValue);
    pHRFAttributeLightSource =        new HRFAttributeLightSource(UShortValue);
    pHRFAttributeFlash =              new HRFAttributeFlash(UShortValue);
    pHRFAttributeFocalLength =        new HRFAttributeFocalLength(DoubleValue);
    pHRFAttributeSubjectArea =        new HRFAttributeSubjectArea(VectorUShort);
    pHRFAttributeMakerNote =          new HRFAttributeMakerNote(VectorByte);
    pHRFAttributeUserComment =        new HRFAttributeUserComment(VectorByte);
    pHRFAttributeSubSecTime =         new HRFAttributeSubSecTime(WStringValue);
    pHRFAttributeFlashpixVersion =    new HRFAttributeFlashpixVersion(VectorByte);
    pHRFAttributeColorSpace =         new HRFAttributeColorSpace(UShortValue);
    pHRFAttributePixelXDimension =    new HRFAttributePixelXDimension(uint32_tValue); 
    pHRFAttributePixelYDimension =    new HRFAttributePixelYDimension(uint32_tValue);
    pHRFAttributeRelatedSoundFile =   new HRFAttributeRelatedSoundFile(WStringValue);
    pHRFAttributeFlashEnergy =        new HRFAttributeFlashEnergy(DoubleValue);
    pHRFAttributeSubjectLocation =    new HRFAttributeSubjectLocation(VectorUShort);
    pHRFAttributeExposureIndex =      new HRFAttributeExposureIndex(DoubleValue);
    pHRFAttributeSensingMethod =      new HRFAttributeSensingMethod(UShortValue);
    pHRFAttributeFileSource =         new HRFAttributeFileSource(ByteValue);
    pHRFAttributeSceneType =          new HRFAttributeSceneType(ByteValue);
    pHRFAttributeCFAPattern =         new HRFAttributeCFAPattern(VectorByte);
    pHRFAttributeCustomRendered =     new HRFAttributeCustomRendered(UShortValue);
    pHRFAttributeExposureMode =       new HRFAttributeExposureMode(UShortValue);
    pHRFAttributeWhiteBalance =       new HRFAttributeWhiteBalance(UShortValue);
    pHRFAttributeDigitalZoomRatio =   new HRFAttributeDigitalZoomRatio(DoubleValue);
    pHRFAttributeSceneCaptureType =   new HRFAttributeSceneCaptureType(UShortValue);
    pHRFAttributeGainControl =        new HRFAttributeGainControl(UShortValue);
    pHRFAttributeContrast =           new HRFAttributeContrast(UShortValue);
    pHRFAttributeSaturation =         new HRFAttributeSaturation(UShortValue);
    pHRFAttributeSharpness =          new HRFAttributeSharpness(UShortValue);
    pHRFAttributeImageUniqueID =      new HRFAttributeImageUniqueID(WStringValue);
    pHRFAttributeImageGamma =         new HRFAttributeImageGamma(DoubleValue);
    pHRFAttributeTimeModification =   new HRFAttributeTimeModification(WStringValue);
    pHRFAttributeHGRFile =            new HRFAttributeHGRFile(StringValue);
    pHRFAttributeHGRVersion =         new HRFAttributeHGRVersion(StringValue);
    pHRFAttributeHGROwner =           new HRFAttributeHGROwner(StringValue);
    pHRFAttributeHGRDescription =     new HRFAttributeHGRDescription(StringValue);
    pHRFAttributeOriginalFileSize =   new HRFAttributeOriginalFileSize(UInt64Value);
    pHPSAttributeImageDescription =   new HPSAttributeImageDescription(WStringValue);

    pHRFAttributeOriginalFileFormat =          new HRFAttributeOriginalFileFormat(WStringValue);
    pHRFAttributeOriginalFileCompression =     new HRFAttributeOriginalFileCompression(WStringValue);     
    pHRFAttributeSubjectDistanceRange =        new HRFAttributeSubjectDistanceRange(UShortValue);
    pHRFAttributeDeviceSettingDescription =    new HRFAttributeDeviceSettingDescription(VectorByte);
    pHRFAttributeFocalLengthIn35mmFilm =       new HRFAttributeFocalLengthIn35mmFilm(UShortValue);
    pHRFAttributeSpatialFrequencyResponse =    new HRFAttributeSpatialFrequencyResponse(VectorByte);
    pHRFAttributeFocalPlaneXResolution =       new HRFAttributeFocalPlaneXResolution(DoubleValue);
    pHRFAttributeFocalPlaneYResolution =       new HRFAttributeFocalPlaneYResolution(DoubleValue);
    pHRFAttributeFocalPlaneResolutionUnit =    new HRFAttributeFocalPlaneResolutionUnit(UShortValue);
    pHRFAttributeSubSecTimeDigitized =         new HRFAttributeSubSecTimeDigitized(WStringValue);
    pHRFAttributeSubSecTimeOriginal =          new HRFAttributeSubSecTimeOriginal(WStringValue);
    pHRFAttributeExposureBiasValue =           new HRFAttributeExposureBiasValue(DoubleValue); 
    pHRFAttributeDateTimeDigitized =           new HRFAttributeDateTimeDigitized(WStringValue);
    pHRFAttributeComponentsConfiguration =     new HRFAttributeComponentsConfiguration(VectorByte);
    pHRFAttributeCompressedBitsPerPixel =      new HRFAttributeCompressedBitsPerPixel(DoubleValue);
    pHRFAttributeShutterSpeedValue =           new HRFAttributeShutterSpeedValue(DoubleValue);
    pHRFAttributeGeogGeodeticDatum =           new HRFAttributeGeogGeodeticDatum(UShortValue);
    pHRFAttributeGeogPrimeMeridian =           new HRFAttributeGeogPrimeMeridian(UShortValue);
    pHRFAttribute3DTranformationMatrix =       new HRFAttribute3DTransformationMatrix(Matrix);
    pHRFAttributeOnDemandRastersInfo =         new HRFAttributeOnDemandRastersInfo(StringValue);
    pHRFAttributeProjectedCSTypeLong =         new HRFAttributeProjectedCSTypeLong(uint32_tValue);
    pHRFAttributeDontSupportPersistentColor =  new HRFAttributeDontSupportPersistentColor(true);
    pHRFAttributeProjLinearUnitSize =          new HRFAttributeProjLinearUnitSize(DoubleValue);
    pHRFAttributeGeogLinearUnitSize =          new HRFAttributeGeogLinearUnitSize(DoubleValue);
    pHRFAttributeGeogAngularUnits =            new HRFAttributeGeogAngularUnits(UShortValue);
    pHRFAttributeGeogAngularUnitSize =         new HRFAttributeGeogAngularUnitSize(DoubleValue);
    pHRFAttributeGeogSemiMajorAxis =           new HRFAttributeGeogSemiMajorAxis(DoubleValue);
    pHRFAttributeGeogSemiMinorAxis =           new HRFAttributeGeogSemiMinorAxis(DoubleValue);
    pHRFAttributeGeogInvFlattening =           new HRFAttributeGeogInvFlattening(DoubleValue);  
    pHRFAttributeGeogPrimeMeridianLong =       new HRFAttributeGeogPrimeMeridianLong(DoubleValue);
    pHRFAttributeProjNatOriginLong =           new HRFAttributeProjNatOriginLong(DoubleValue);
    pHRFAttributeProjFalseNorthing =           new HRFAttributeProjFalseNorthing(DoubleValue);
    pHRFAttributeProjFalseOriginLong =         new HRFAttributeProjFalseOriginLong(DoubleValue);
    pHRFAttributeProjFalseOriginLat =          new HRFAttributeProjFalseOriginLat(DoubleValue);
    pHRFAttributeProjFalseOriginEasting =      new HRFAttributeProjFalseOriginEasting(DoubleValue);
    pHRFAttributeProjFalseOriginNorthing =     new HRFAttributeProjFalseOriginNorthing(DoubleValue);   
    pHRFAttributeProjCenterEasting =           new HRFAttributeProjCenterEasting(DoubleValue);
    pHRFAttributeProjCenterNorthing =          new HRFAttributeProjCenterNorthing(DoubleValue);
    pHRFAttributeProjScaleAtNatOrigin =        new HRFAttributeProjScaleAtNatOrigin(DoubleValue);
    pHRFAttributeProjScaleAtCenter =           new HRFAttributeProjScaleAtCenter(DoubleValue); 
    pHRFAttributeProjStraightVertPoleLong =    new HRFAttributeProjStraightVertPoleLong(DoubleValue);
    pHRFAttributeProjRectifiedGridAngle =      new HRFAttributeProjRectifiedGridAngle(DoubleValue);
    pHRFAttributeGPSImgDirectionRef =          new HRFAttributeGPSImgDirectionRef(WStringValue);
    pHRFAttributeGPSDestLatitudeRef =          new HRFAttributeGPSDestLatitudeRef(WStringValue);
    pHRFAttributeGPSDestLongitudeRef =         new HRFAttributeGPSDestLongitudeRef(WStringValue);
    pHRFAttributeGPSDestBearingRef =           new HRFAttributeGPSDestBearingRef(WStringValue);
    pHRFAttributeGPSDestDistanceRef =          new HRFAttributeGPSDestDistanceRef(WStringValue);    
    pHRFAttributeGPSProcessingMethod =         new HRFAttributeGPSProcessingMethod(VectorByte);
    pHRFAttributeGPSAreaInformation =          new HRFAttributeGPSAreaInformation(VectorByte);
    pHRFAttributeSpectralSensitivity =         new HRFAttributeSpectralSensitivity(WStringValue);
    pHRFAttributeOptoelectricConversionFactor =  new HRFAttributeOptoElectricConversionFactor(VectorByte);
    pHRFAttributeOriginalFileModificationDate =  new HRFAttributeOriginalFileModificationDate(UInt64Value);

    //HPMAttribute_T
    pImageDescription =         new HRFAttributeImageDescription(WStringValue);
    pSampleValue =              new HRFAttributeMinSampleValue(VectorDouble);
    pResolutionUnit =           new HRFAttributeResolutionUnit(UShortValue);
    pTransfoMatrix =            new HRFAttribute3DTransformationMatrix(Matrix);
    pForeground =               new HRFAttributeForeground(UInt32Value);
    pYResolution =              new HRFAttributeYResolution(DoubleValue);
    pProjectedLong =            new HRFAttributeProjectedCSTypeLong(uint32_tValue);
    pGPSVersionID =             new HRFAttributeGPSVersionID(VectorByte);
    pGPSAltitudeRef =           new HRFAttributeGPSAltitudeRef(ByteValue);
    pISOSpeedRatings =          new HRFAttributeISOSpeedRatings(VectorUShort);
    pExifVersion =              new HRFAttributeExifVersion(VectorChar);
    pOFileFormat =              new HRFAttributeOriginalFileFormat(WStringValue);
    pOFileSize =                new HRFAttributeOriginalFileSize(UInt64Value);
    pColor =                    new HRFAttributeDontSupportPersistentColor(true);

    }

//==================================================================================
// GetData() const;
// SetData(const T& pi_rData);
//==================================================================================
TEST_F (HPMAttributeTester, DataTest)
    {
  
    //string
    ASSERT_STREQ(WStringValue.c_str(), pImageDescription->GetData().c_str());

    pImageDescription->SetData(L"Nouveau");
    ASSERT_STREQ(L"Nouveau", pImageDescription->GetData().c_str());

    //vector<double>
    ASSERT_DOUBLE_EQ(VectorDouble.front(), pSampleValue->GetData().front());
    ASSERT_DOUBLE_EQ(VectorDouble.back(), pSampleValue->GetData().back());
    
    pSampleValue->SetData(VectorDoubleTest);

    ASSERT_DOUBLE_EQ(1.0, pSampleValue->GetData().front());
    ASSERT_DOUBLE_EQ(2.0, pSampleValue->GetData().back());

    //UShort
    ASSERT_DOUBLE_EQ(UShortValue, pResolutionUnit->GetData());

    pResolutionUnit->SetData(1);
    ASSERT_DOUBLE_EQ(1, pResolutionUnit->GetData());

    //HFCMatrix<4,4, double>
    ASSERT_TRUE(Matrix.IsEqualTo(pTransfoMatrix->GetData()));
  
    pTransfoMatrix->SetData(MatrixTest);
    ASSERT_TRUE(MatrixTest.IsEqualTo(pTransfoMatrix->GetData()));

    //UInt32
    ASSERT_DOUBLE_EQ(UInt32Value, pForeground->GetData());

    pForeground->SetData(1);
    ASSERT_DOUBLE_EQ(1, pForeground->GetData());

    //bool
    ASSERT_TRUE(pColor->GetData());

    pColor->SetData(false);
    ASSERT_FALSE(pColor->GetData());

    //double
    ASSERT_DOUBLE_EQ(DoubleValue, pYResolution->GetData());

    pYResolution->SetData(1.0);
    ASSERT_DOUBLE_EQ(1.0, pYResolution->GetData());

    //uint32_t
    ASSERT_DOUBLE_EQ(uint32_tValue, pProjectedLong->GetData());

    pProjectedLong->SetData(1);
    ASSERT_DOUBLE_EQ(1, pProjectedLong->GetData());

    //vector<Byte>
    ASSERT_EQ(VectorByte.front(), pGPSVersionID->GetData().front());
    ASSERT_EQ(VectorByte.back(), pGPSVersionID->GetData().back());
    
    pGPSVersionID->SetData(VectorByteTest);

    ASSERT_EQ(1, pGPSVersionID->GetData().front());
    ASSERT_EQ(2, pGPSVersionID->GetData().back());

    //Byte
    ASSERT_EQ(ByteValue, pGPSAltitudeRef->GetData());

    pGPSAltitudeRef->SetData(1);
    ASSERT_EQ(1, pGPSAltitudeRef->GetData());
    
    //vector<UShort>
    ASSERT_DOUBLE_EQ(VectorUShort.front(), pISOSpeedRatings->GetData().front());
    ASSERT_DOUBLE_EQ(VectorUShort.back(), pISOSpeedRatings->GetData().back());
    
    pISOSpeedRatings->SetData(VectorUShortTest);

    ASSERT_DOUBLE_EQ(1, pISOSpeedRatings->GetData().front());
    ASSERT_DOUBLE_EQ(2, pISOSpeedRatings->GetData().back());

    //<vector<char>
    ASSERT_EQ(VectorChar.front(), pExifVersion->GetData().front());
    ASSERT_EQ(VectorChar.back(), pExifVersion->GetData().back());
    
    pExifVersion->SetData(VectorCharTest);

    ASSERT_EQ('8', pExifVersion->GetData().front());
    ASSERT_EQ('9', pExifVersion->GetData().back());

    //WString
    ASSERT_STREQ(WStringValue.c_str(), pOFileFormat->GetData().c_str());

    pOFileFormat->SetData(L"Nouveau");
    ASSERT_STREQ(L"Nouveau", pOFileFormat->GetData().c_str());

    //UInt64
    ASSERT_EQ(UInt64Value, pOFileSize->GetData());

    pOFileSize->SetData(1);
    ASSERT_EQ(1, pOFileSize->GetData());

    }

//==================================================================================
// GetID() const;
//==================================================================================
TEST_F (HPMAttributeTester, GetIDTest)
    {

    //string
    ASSERT_EQ(HRFAttributeImageDescription::ATTRIBUTE_ID, pHRFAttributeImageDescription->GetID());
    ASSERT_EQ(HRFAttributeImageDescription::ATTRIBUTE_ID, (new HRFAttributeImageDescription)->GetID());

    //vector<double>
    ASSERT_EQ(HRFAttributeMinSampleValue::ATTRIBUTE_ID, pHRFAttributeMinSampleValue->GetID());
    ASSERT_EQ(HRFAttributeMinSampleValue::ATTRIBUTE_ID, (new HRFAttributeMinSampleValue)->GetID());

    //UShort    
    ASSERT_EQ(HRFAttributeResolutionUnit::ATTRIBUTE_ID, pHRFAttributeResolutionUnit->GetID());
    ASSERT_EQ(HRFAttributeResolutionUnit::ATTRIBUTE_ID, (new HRFAttributeResolutionUnit)->GetID());

    //HFCMatrix<4,4, double>
    ASSERT_EQ(HRFAttribute3DTransformationMatrix::ATTRIBUTE_ID, pHRFAttribute3DTranformationMatrix->GetID());
    ASSERT_EQ(HRFAttribute3DTransformationMatrix::ATTRIBUTE_ID, (new HRFAttribute3DTransformationMatrix)->GetID());

    //UInt32
    ASSERT_EQ(HRFAttributeForeground::ATTRIBUTE_ID, pHRFAttributeForeground->GetID());
    ASSERT_EQ(HRFAttributeForeground::ATTRIBUTE_ID, (new HRFAttributeForeground)->GetID());

    //bool
    ASSERT_EQ(HRFAttributeDontSupportPersistentColor::ATTRIBUTE_ID, pHRFAttributeDontSupportPersistentColor->GetID());
    ASSERT_EQ(HRFAttributeDontSupportPersistentColor::ATTRIBUTE_ID, (new HRFAttributeDontSupportPersistentColor)->GetID());

    //double
    ASSERT_EQ(HRFAttributeYResolution::ATTRIBUTE_ID, pHRFAttributeYResolution->GetID());
    ASSERT_EQ(HRFAttributeYResolution::ATTRIBUTE_ID, (new HRFAttributeYResolution)->GetID());

    //uint32_t
    ASSERT_EQ(HRFAttributeProjectedCSTypeLong::ATTRIBUTE_ID, pHRFAttributeProjectedCSTypeLong->GetID());
    ASSERT_EQ(HRFAttributeProjectedCSTypeLong::ATTRIBUTE_ID, (new HRFAttributeProjectedCSTypeLong)->GetID());

    //vector<Byte>
    ASSERT_EQ(HRFAttributeGPSVersionID::ATTRIBUTE_ID, pHRFAttributeGPSVersionID->GetID());
    ASSERT_EQ(HRFAttributeGPSVersionID::ATTRIBUTE_ID, (new HRFAttributeGPSVersionID)->GetID());

    //Byte
    ASSERT_EQ(HRFAttributeGPSAltitudeRef::ATTRIBUTE_ID, pHRFAttributeGPSAltitudeRef->GetID());
    ASSERT_EQ(HRFAttributeGPSAltitudeRef::ATTRIBUTE_ID, (new HRFAttributeGPSAltitudeRef)->GetID());

    //vector<UShort>
    ASSERT_EQ(HRFAttributeISOSpeedRatings::ATTRIBUTE_ID, pHRFAttributeISOSpeedRatings->GetID());
    ASSERT_EQ(HRFAttributeISOSpeedRatings::ATTRIBUTE_ID, (new HRFAttributeISOSpeedRatings)->GetID());

    //<vector<char>
    ASSERT_EQ(HRFAttributeExifVersion::ATTRIBUTE_ID, pHRFAttributeExifVersion->GetID());
    ASSERT_EQ(HRFAttributeExifVersion::ATTRIBUTE_ID, (new HRFAttributeExifVersion)->GetID());

    //WString
    ASSERT_EQ(HRFAttributeOriginalFileFormat::ATTRIBUTE_ID, pHRFAttributeOriginalFileFormat->GetID());
    ASSERT_EQ(HRFAttributeOriginalFileFormat::ATTRIBUTE_ID, (new HRFAttributeOriginalFileFormat)->GetID());

    //UInt64
    ASSERT_EQ(HRFAttributeOriginalFileSize::ATTRIBUTE_ID, pHRFAttributeOriginalFileSize->GetID());
    ASSERT_EQ(HRFAttributeOriginalFileSize::ATTRIBUTE_ID, (new HRFAttributeOriginalFileSize)->GetID());

    }

//==================================================================================
// SameAttributeAs(const HPMGenericAttribute& pi_rObj); 
//==================================================================================
TEST_F (HPMAttributeTester, SameAttributeAsTest)
    {

    // Test for same attribute type not same type same data.

    //string
    ASSERT_TRUE(pHRFAttributeImageDescription->SameAttributeAs(HRFAttributeImageDescription(L"__SameAttributeAsTest")));

    //vector<double>
    ASSERT_TRUE(pHRFAttributeMinSampleValue->SameAttributeAs(HRFAttributeMinSampleValue()));

    //UShort
    ASSERT_TRUE(pHRFAttributeResolutionUnit->SameAttributeAs(HRFAttributeResolutionUnit()));

    //HFCMatrix<4,4, double>
    ASSERT_TRUE(pHRFAttribute3DTranformationMatrix->SameAttributeAs(HRFAttribute3DTransformationMatrix()));

    //UInt32
    ASSERT_TRUE(pHRFAttributeForeground->SameAttributeAs(HRFAttributeForeground()));

    //bool
    ASSERT_TRUE(pHRFAttributeDontSupportPersistentColor->SameAttributeAs(HRFAttributeDontSupportPersistentColor()));

    //double
    ASSERT_TRUE(pHRFAttributeYResolution->SameAttributeAs(HRFAttributeYResolution()));

    //uint32_t
    ASSERT_TRUE(pHRFAttributeProjectedCSTypeLong->SameAttributeAs(HRFAttributeProjectedCSTypeLong()));

    //vector<Byte>
    ASSERT_TRUE(pHRFAttributeGPSVersionID->SameAttributeAs(HRFAttributeGPSVersionID()));

    //Byte
    ASSERT_TRUE(pHRFAttributeGPSAltitudeRef->SameAttributeAs(HRFAttributeGPSAltitudeRef()));

    //vector<UShort>
    ASSERT_TRUE(pHRFAttributeISOSpeedRatings->SameAttributeAs(HRFAttributeISOSpeedRatings()));

    //<vector<char>
    ASSERT_TRUE(pHRFAttributeExifVersion->SameAttributeAs(HRFAttributeExifVersion()));

    //WString
    ASSERT_TRUE(pHRFAttributeOriginalFileFormat->SameAttributeAs(HRFAttributeOriginalFileFormat()));

    //UInt64
    ASSERT_TRUE(pHRFAttributeOriginalFileSize->SameAttributeAs(HRFAttributeOriginalFileSize()));

    }

//==================================================================================
// Clone() const; 
//==================================================================================
TEST_F (HPMAttributeTester, CloneTest)
    {

    //string
    HPMGenericAttribute* pHRFAttributeImageDescriptionClone = pHRFAttributeImageDescription->Clone();

    ASSERT_EQ(HRFAttributeImageDescription::ATTRIBUTE_ID, pHRFAttributeImageDescriptionClone->GetID());
    ASSERT_STREQ(WStringValue.c_str(), static_cast<HRFAttributeImageDescription*> (pHRFAttributeImageDescriptionClone)->GetData().c_str());

    //vector<double>
    HPMGenericAttribute* pHRFAttributeSampleValueClone = pHRFAttributeMinSampleValue->Clone();

    ASSERT_EQ(HRFAttributeMinSampleValue::ATTRIBUTE_ID, pHRFAttributeSampleValueClone->GetID());
    ASSERT_DOUBLE_EQ(VectorDouble.front(), static_cast<HRFAttributeMinSampleValue*> (pHRFAttributeSampleValueClone)->GetData().front());
    ASSERT_DOUBLE_EQ(VectorDouble.back(), static_cast<HRFAttributeMinSampleValue*> (pHRFAttributeSampleValueClone)->GetData().back());

    //UShort
    HPMGenericAttribute* pHRFAttributeResolutionUnitClone = pHRFAttributeResolutionUnit->Clone();

    ASSERT_EQ(HRFAttributeResolutionUnit::ATTRIBUTE_ID, pHRFAttributeResolutionUnitClone->GetID());
    ASSERT_DOUBLE_EQ(UShortValue, static_cast<HRFAttributeResolutionUnit*> (pHRFAttributeResolutionUnitClone)->GetData());

    //HFCMatrix<4,4, double>
    HPMGenericAttribute* pHRFAttribute3DTranformationMatrixClone = pHRFAttribute3DTranformationMatrix->Clone();

    ASSERT_EQ(HRFAttribute3DTransformationMatrix::ATTRIBUTE_ID, pHRFAttribute3DTranformationMatrixClone->GetID());
    ASSERT_TRUE(Matrix.IsEqualTo(static_cast<HRFAttribute3DTransformationMatrix*> (pHRFAttribute3DTranformationMatrixClone)->GetData()));

    //UInt32
    HPMGenericAttribute* pHRFAttributeForegroundClone = pHRFAttributeForeground->Clone();
    
    ASSERT_EQ(HRFAttributeForeground::ATTRIBUTE_ID, pHRFAttributeForegroundClone->GetID());
    ASSERT_EQ(UInt32Value, static_cast<HRFAttributeImageSlo*> (pHRFAttributeForegroundClone)->GetData());

    //bool
    HPMGenericAttribute* pHRFAttributeDontSupportPersistentColorClone = pHRFAttributeDontSupportPersistentColor->Clone();

    ASSERT_EQ(HRFAttributeDontSupportPersistentColor::ATTRIBUTE_ID, pHRFAttributeDontSupportPersistentColorClone->GetID());
    ASSERT_TRUE(static_cast<HRFAttributeDontSupportPersistentColor*> (pHRFAttributeDontSupportPersistentColorClone)->GetData());

    //double
    HPMGenericAttribute* pHRFAttributeYResolutionClone = pHRFAttributeYResolution->Clone();

    ASSERT_EQ(HRFAttributeYResolution::ATTRIBUTE_ID, pHRFAttributeYResolutionClone->GetID());
    ASSERT_DOUBLE_EQ(DoubleValue, static_cast<HRFAttributeYResolution*> (pHRFAttributeYResolutionClone)->GetData());

    //uint32_t
    HPMGenericAttribute* pHRFAttributeProjectedCSTypeLongClone = pHRFAttributeProjectedCSTypeLong->Clone();

    ASSERT_EQ(HRFAttributeProjectedCSTypeLong::ATTRIBUTE_ID, pHRFAttributeProjectedCSTypeLongClone->GetID());
    ASSERT_EQ(uint32_tValue, static_cast<HRFAttributeProjectedCSTypeLong*> (pHRFAttributeProjectedCSTypeLongClone)->GetData());
    
    //vector<Byte>
    HPMGenericAttribute* pHRFAttributeGPSVersionIDClone = pHRFAttributeGPSVersionID->Clone();

    ASSERT_EQ(HRFAttributeGPSVersionID::ATTRIBUTE_ID, pHRFAttributeGPSVersionIDClone->GetID());
    ASSERT_EQ(VectorByte.front(), static_cast<HRFAttributeGPSVersionID*> (pHRFAttributeGPSVersionIDClone)->GetData().front());
    ASSERT_EQ(VectorByte.back(), static_cast<HRFAttributeGPSVersionID*> (pHRFAttributeGPSVersionIDClone)->GetData().back());

    //Byte
    HPMGenericAttribute* pHRFAttributeGPSAltitudeRefClone = pHRFAttributeGPSAltitudeRef->Clone();

    ASSERT_EQ(HRFAttributeGPSAltitudeRef::ATTRIBUTE_ID, pHRFAttributeGPSAltitudeRefClone->GetID());
    ASSERT_EQ(ByteValue, static_cast<HRFAttributeGPSAltitudeRef*> (pHRFAttributeGPSAltitudeRefClone)->GetData());

    //vector<UShort>
    HPMGenericAttribute* pHRFAttributeISOSpeedRatingsClone = pHRFAttributeISOSpeedRatings->Clone();
    
    ASSERT_EQ(HRFAttributeISOSpeedRatings::ATTRIBUTE_ID, pHRFAttributeISOSpeedRatings->GetID());
    ASSERT_EQ(VectorUShort.front(), static_cast<HRFAttributeISOSpeedRatings*> (pHRFAttributeISOSpeedRatingsClone)->GetData().front());
    ASSERT_EQ(VectorUShort.back(), static_cast<HRFAttributeISOSpeedRatings*> (pHRFAttributeISOSpeedRatingsClone)->GetData().back());

    //<vector<char>
    HPMGenericAttribute* pHRFAttributeExifVersionClone = pHRFAttributeExifVersion->Clone();

    ASSERT_EQ(HRFAttributeExifVersion::ATTRIBUTE_ID, pHRFAttributeExifVersion->GetID());
    ASSERT_EQ(VectorChar.front(), static_cast<HRFAttributeExifVersion*> (pHRFAttributeExifVersionClone)->GetData().front());
    ASSERT_EQ(VectorChar.back(), static_cast<HRFAttributeExifVersion*> (pHRFAttributeExifVersionClone)->GetData().back());

    //WString
    HPMGenericAttribute* pHRFAttributeOriginalFileFormatClone = pHRFAttributeOriginalFileFormat->Clone();

    ASSERT_EQ(HRFAttributeOriginalFileFormat::ATTRIBUTE_ID, pHRFAttributeOriginalFileFormatClone->GetID());
    ASSERT_STREQ(WStringValue.c_str(), static_cast<HRFAttributeOriginalFileFormat*> (pHRFAttributeOriginalFileFormatClone)->GetData().c_str());

    //UInt64
    HPMGenericAttribute* pHRFAttributeOriginalFileSizeClone = pHRFAttributeOriginalFileSize->Clone();

    ASSERT_EQ(HRFAttributeOriginalFileSize::ATTRIBUTE_ID, pHRFAttributeOriginalFileSizeClone->GetID());
    ASSERT_EQ(UInt64Value, static_cast<HRFAttributeOriginalFileSize*> (pHRFAttributeOriginalFileSizeClone)->GetData());

    }

//==================================================================================
// GetValueAsString() const 
//==================================================================================
TEST_F (HPMAttributeTester, GetValueAsStringTest)
    {

    //string
    ASSERT_STREQ(L"Test", pHRFAttributeImageDescription->GetDataAsString().c_str());

    //vector<double> 
    ASSERT_STREQ(L"8, 9", pHRFAttributeMinSampleValue->GetDataAsString().c_str());

    //UShort
    ASSERT_STREQ(L"8", pHRFAttributeResolutionUnit->GetDataAsString().c_str());

    //UInt32
    ASSERT_STREQ(L"8", pHRFAttributeForeground->GetDataAsString().c_str());

    //bool
    ASSERT_STREQ(L"1", pHRFAttributeDontSupportPersistentColor->GetDataAsString().c_str());

    //double
    ASSERT_STREQ(L"8", pHRFAttributeYResolution->GetDataAsString().c_str());

    //uint32_t
    ASSERT_STREQ(L"8", pHRFAttributeProjectedCSTypeLong->GetDataAsString().c_str());

    //vector<Byte>
    ASSERT_STREQ(L"8, 9", pHRFAttributeGPSVersionID->GetDataAsString().c_str()); 

    //Byte
    ASSERT_STREQ(L"8", pHRFAttributeGPSAltitudeRef->GetDataAsString().c_str()); 

    //vector<UShort>
    ASSERT_STREQ(L"8, 9", pHRFAttributeISOSpeedRatings->GetDataAsString().c_str());

    //<vector<char>
    ASSERT_STREQ(L"8, 9", pHRFAttributeExifVersion->GetDataAsString().c_str());

    //WString
    ASSERT_STREQ(L"Test", pHRFAttributeOriginalFileFormat->GetDataAsString().c_str());

    //UInt64
    ASSERT_STREQ(L"8", pHRFAttributeOriginalFileSize->GetDataAsString().c_str());

    }

//==================================================================================
// GetName() 
//==================================================================================
TEST_F (HPMAttributeTester, GetNameTest)
    {

    //string
    ASSERT_STREQ(L"Image Description", pHRFAttributeImageDescription->GetName().c_str());

    //vector<double> 
    ASSERT_STREQ(L"Sample Minimum Value", pHRFAttributeMinSampleValue->GetName().c_str());

    //UShort
    ASSERT_STREQ(L"Resolution Unit", pHRFAttributeResolutionUnit->GetName().c_str());

    //HFCMatrix<4,4, double>
    ASSERT_STREQ(L"3D Tranformation Matrix", pHRFAttribute3DTranformationMatrix->GetName().c_str());

    //UInt32
    ASSERT_STREQ(L"Foreground", pHRFAttributeForeground->GetName().c_str());

    //bool
    ASSERT_STREQ(L"Persistent Color", pHRFAttributeDontSupportPersistentColor->GetName().c_str());

    //double
    ASSERT_STREQ(L"Y Resolution", pHRFAttributeYResolution->GetName().c_str());

    //uint32_t
    ASSERT_STREQ(L"ProjectedCSTypeLong", pHRFAttributeProjectedCSTypeLong->GetName().c_str());

    //vector<Byte>
    ASSERT_STREQ(L"GPSVersionID", pHRFAttributeGPSVersionID->GetName().c_str());

    //Byte
    ASSERT_STREQ(L"GPSAltitudeRef", pHRFAttributeGPSAltitudeRef->GetName().c_str());

    //vector<UShort>
    ASSERT_STREQ(L"ISOSpeedRatings", pHRFAttributeISOSpeedRatings->GetName().c_str());

    //<vector<char>
    ASSERT_STREQ(L"ExifVersion", pHRFAttributeExifVersion->GetName().c_str());

    //WString
    ASSERT_STREQ(L"Original File Format", pHRFAttributeOriginalFileFormat->GetName().c_str());

    //UInt64
    ASSERT_STREQ(L"Original File Size", pHRFAttributeOriginalFileSize->GetName().c_str());

    }
