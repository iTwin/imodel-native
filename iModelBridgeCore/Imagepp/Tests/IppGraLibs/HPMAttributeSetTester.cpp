//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HPMAttributeSetTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HPMAttributeSetTester.h"

HPMAttributeSetTester::HPMAttributeSetTester() 
    {
    //Default Value
    for(i = 0 ; i < 4 ; ++i)
        for(j = 0; j < 4 ; ++j)
            Matrix[i][j] = i+j;

    VectorByte.push_back('8');
    VectorByte.push_back('9');

    VectorDouble.push_back(8.0);
    VectorDouble.push_back(9.0);

    VectorUShort.push_back(8);
    VectorUShort.push_back(9);

    VectorChar.push_back('8');
    VectorChar.push_back('9');

    for(i = 0 ; i < 4 ; ++i)
        for(j = 0; j < 4 ; ++j)
            MatrixTest[i][j] = 12;

    VectorByteTest.push_back('1');
    VectorByteTest.push_back('2');

    VectorDoubleTest.push_back(1.0);
    VectorDoubleTest.push_back(2.0);

    VectorUShortTest.push_back(1);
    VectorUShortTest.push_back(2);

    VectorCharTest.push_back('1');
    VectorCharTest.push_back('2');

    StringValue = "Test";
    DoubleValue =   8.0;
    UShortValue =   8;
    UInt32Value =   8;
    ByteValue =     '8';
    uint32_tValue =  8;
    UInt64Value =   8;
    WStringValue =  L"Test";

    //HPMAttribute
    pHRFAttributeImageDescription =      new HRFAttributeImageDescription(WStringValue); 
    pHRFAttributeSampleValue =           new HRFAttributeMinSampleValue(VectorDouble);
    pHRFAttributeResolutionUnit =        new HRFAttributeResolutionUnit(UShortValue); 
    pHRFAttributeForeground =            new HRFAttributeForeground(UInt32Value);
    pHRFAttributeYResolution =           new HRFAttributeYResolution(DoubleValue);
    pHRFAttributeProjectedCSTypeLong =   new HRFAttributeProjectedCSTypeLong(uint32_tValue);
    pHRFAttributeGPSVersionID =          new HRFAttributeGPSVersionID(VectorByte);
    pHRFAttributeGPSAltitudeRef =        new HRFAttributeGPSAltitudeRef(ByteValue);
    pHRFAttributeISOSpeedRatings =       new HRFAttributeISOSpeedRatings(VectorUShort);
    pHRFAttributeExifVersion =           new HRFAttributeExifVersion(VectorChar);
    pHRFAttributeOriginalFileFormat =    new HRFAttributeOriginalFileFormat(WStringValue);
    pHRFAttributeOriginalFileSize =      new HRFAttributeOriginalFileSize(UInt64Value);
    pHRFAttributeDontSupportPersistentColor =  new HRFAttributeDontSupportPersistentColor(true);
    pHRFAttribute3DTransformationMatrix =      new HRFAttribute3DTransformationMatrix(Matrix);

    //AttributeSet
    AttributeSet.Set(pHRFAttributeImageDescription);
    AttributeSet.Set(pHRFAttributeSampleValue);
    AttributeSet.Set(pHRFAttributeResolutionUnit);
    AttributeSet.Set(pHRFAttribute3DTransformationMatrix);
    AttributeSet.Set(pHRFAttributeForeground);
    AttributeSet.Set(pHRFAttributeYResolution);
    AttributeSet.Set(pHRFAttributeProjectedCSTypeLong);
    AttributeSet.Set(pHRFAttributeGPSVersionID);
    AttributeSet.Set(pHRFAttributeGPSAltitudeRef);
    AttributeSet.Set(pHRFAttributeISOSpeedRatings);
    AttributeSet.Set(pHRFAttributeExifVersion);
    AttributeSet.Set(pHRFAttributeOriginalFileFormat);
    AttributeSet.Set(pHRFAttributeOriginalFileSize);
    AttributeSet.Set(pHRFAttributeDontSupportPersistentColor); 

    }

//==================================================================================
// Set(const HFCPtr<HPMGenericAttribute>& pi_rpAttribute);
// size() const;
//================================================================================== 
TEST_F (HPMAttributeSetTester, SetTest)
    {
   
    ASSERT_EQ(14, AttributeSet.size());

    }

//==================================================================================
// HPMAttributeSet(const HPMAttributeSet& pi_rObj);
// HPMAttributeSet& operator=(const HPMAttributeSet& pi_rObj);
//==================================================================================
TEST_F (HPMAttributeSetTester, CopyTest)
    {

    HPMAttributeSet AttributeSetCopy(AttributeSet);
    ASSERT_EQ(14, AttributeSetCopy.size());
    ASSERT_TRUE(AttributeSetCopy.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeDontSupportPersistentColor::ATTRIBUTE_ID)));
    ASSERT_FALSE(AttributeSetCopy.HasAttribute(static_cast<HPMAttributesID>(HPSAttributeImageDescription::ATTRIBUTE_ID)));
   
    HPMAttributeSet AttributeSetEq = AttributeSet;
    ASSERT_EQ(14, AttributeSetEq.size());
    ASSERT_TRUE(AttributeSetEq.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeDontSupportPersistentColor::ATTRIBUTE_ID)));
    ASSERT_FALSE(AttributeSetEq.HasAttribute(static_cast<HPMAttributesID>(HPSAttributeImageDescription::ATTRIBUTE_ID))); 
   
    }

//==================================================================================
// HasAttribute(const HPMAttributesID& pi_rID) const;
// HasAttribute() const;
//================================================================================== 
TEST_F (HPMAttributeSetTester, HasAttributeTest)
    { 
    ASSERT_TRUE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeImageDescription::ATTRIBUTE_ID)));
    ASSERT_TRUE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeMinSampleValue::ATTRIBUTE_ID)));
    ASSERT_TRUE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeResolutionUnit::ATTRIBUTE_ID)));
    ASSERT_TRUE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttribute3DTransformationMatrix::ATTRIBUTE_ID)));
    ASSERT_TRUE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeForeground::ATTRIBUTE_ID)));
    ASSERT_TRUE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeYResolution::ATTRIBUTE_ID)));
    ASSERT_TRUE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeProjectedCSTypeLong::ATTRIBUTE_ID)));
    ASSERT_TRUE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeGPSVersionID::ATTRIBUTE_ID)));
    ASSERT_TRUE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeGPSAltitudeRef::ATTRIBUTE_ID)));
    ASSERT_TRUE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeISOSpeedRatings::ATTRIBUTE_ID)));
    ASSERT_TRUE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeExifVersion::ATTRIBUTE_ID)));
    ASSERT_TRUE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeOriginalFileFormat::ATTRIBUTE_ID)));
    ASSERT_TRUE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeOriginalFileSize::ATTRIBUTE_ID)));
    ASSERT_TRUE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeDontSupportPersistentColor::ATTRIBUTE_ID)));

    ASSERT_FALSE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeGIFApplicationCode::ATTRIBUTE_ID)));
    ASSERT_FALSE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeImageGamma::ATTRIBUTE_ID)));
    ASSERT_FALSE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeHGRVersion::ATTRIBUTE_ID)));
    ASSERT_FALSE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeHGRFile::ATTRIBUTE_ID)));
    ASSERT_FALSE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeHGROwner::ATTRIBUTE_ID)));
    ASSERT_FALSE(AttributeSet.HasAttribute(static_cast<HPMAttributesID>(HRFAttributeHGRDescription::ATTRIBUTE_ID)));

    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeImageDescription>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeMinSampleValue>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeResolutionUnit>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttribute3DTransformationMatrix>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeForeground>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeYResolution>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeProjectedCSTypeLong>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeGPSVersionID>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeGPSAltitudeRef>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeISOSpeedRatings>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeExifVersion>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeOriginalFileFormat>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeOriginalFileSize>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeDontSupportPersistentColor>());

    ASSERT_FALSE(AttributeSet.HasAttribute<HRFAttributeGIFApplicationCode>());
    ASSERT_FALSE(AttributeSet.HasAttribute<HRFAttributeImageGamma>());
    ASSERT_FALSE(AttributeSet.HasAttribute<HRFAttributeHGRVersion>());
    ASSERT_FALSE(AttributeSet.HasAttribute<HRFAttributeHGRFile>());
    ASSERT_FALSE(AttributeSet.HasAttribute<HRFAttributeHGROwner>());
    ASSERT_FALSE(AttributeSet.HasAttribute<HRFAttributeHGRDescription>());

    }

//==================================================================================
// FindAttributeCP() const; 
// FindAttributeP();
//================================================================================== 
TEST_F (HPMAttributeSetTester, FindAttributeTest)
    { 
    //String
    HRFAttributeImageDescription const* pImage = AttributeSet.FindAttributeCP<HRFAttributeImageDescription>();
    ASSERT_TRUE(WStringValue == pImage->GetData());
    ASSERT_EQ(HRFAttributeImageDescription::ATTRIBUTE_ID, pImage->GetID());

    HRFAttributeImageDescription* pImage2 = AttributeSet.FindAttributeP<HRFAttributeImageDescription>();
    ASSERT_TRUE(WStringValue == pImage2->GetData());
    ASSERT_EQ(HRFAttributeImageDescription::ATTRIBUTE_ID, pImage2->GetID());

    //Vector<double>
    HRFAttributeMinSampleValue const* pSample = AttributeSet.FindAttributeCP<HRFAttributeMinSampleValue>();
    ASSERT_DOUBLE_EQ(VectorDouble.front(), pSample->GetData().front());
    ASSERT_DOUBLE_EQ(VectorDouble.back(), pSample->GetData().back());
    ASSERT_EQ(HRFAttributeMinSampleValue::ATTRIBUTE_ID, pSample->GetID());

    HRFAttributeMinSampleValue* pSample2 = AttributeSet.FindAttributeP<HRFAttributeMinSampleValue>();
    ASSERT_DOUBLE_EQ(VectorDouble.front(), pSample2->GetData().front());
    ASSERT_DOUBLE_EQ(VectorDouble.back(), pSample2->GetData().back());
    ASSERT_EQ(HRFAttributeMinSampleValue::ATTRIBUTE_ID, pSample2->GetID());

    //UShort
    HRFAttributeResolutionUnit const* pResUnit = AttributeSet.FindAttributeCP<HRFAttributeResolutionUnit>();
    ASSERT_DOUBLE_EQ(UShortValue, pResUnit->GetData());
    ASSERT_EQ(HRFAttributeResolutionUnit::ATTRIBUTE_ID, pResUnit->GetID());

    HRFAttributeResolutionUnit* pResUnit2 = AttributeSet.FindAttributeP<HRFAttributeResolutionUnit>();
    ASSERT_DOUBLE_EQ(UShortValue, pResUnit2->GetData());
    ASSERT_EQ(HRFAttributeResolutionUnit::ATTRIBUTE_ID, pResUnit2->GetID());

    //HFCMatrix<4,4, double>
    HRFAttribute3DTransformationMatrix const* p3dTransfo = AttributeSet.FindAttributeCP<HRFAttribute3DTransformationMatrix>();
    ASSERT_TRUE(Matrix.IsEqualTo(p3dTransfo->GetData()));
    ASSERT_EQ(HRFAttribute3DTransformationMatrix::ATTRIBUTE_ID, p3dTransfo->GetID());

    HRFAttribute3DTransformationMatrix* p3dTransfo2 = AttributeSet.FindAttributeP<HRFAttribute3DTransformationMatrix>();
    ASSERT_TRUE(Matrix.IsEqualTo(p3dTransfo2->GetData()));
    ASSERT_EQ(HRFAttribute3DTransformationMatrix::ATTRIBUTE_ID, p3dTransfo2->GetID());

    //UInt32
    HRFAttributeForeground const* pForeground = AttributeSet.FindAttributeCP<HRFAttributeForeground>(); 
    ASSERT_DOUBLE_EQ(UInt32Value, pForeground->GetData());
    ASSERT_EQ(HRFAttributeForeground::ATTRIBUTE_ID, pForeground->GetID());

    HRFAttributeForeground* pForeground2 = AttributeSet.FindAttributeP<HRFAttributeForeground>(); 
    ASSERT_DOUBLE_EQ(UInt32Value, pForeground2->GetData());
    ASSERT_EQ(HRFAttributeForeground::ATTRIBUTE_ID, pForeground2->GetID());

    //bool
    HRFAttributeDontSupportPersistentColor const* pColor = AttributeSet.FindAttributeCP<HRFAttributeDontSupportPersistentColor>();
    ASSERT_TRUE(pColor->GetData());
    ASSERT_EQ(HRFAttributeDontSupportPersistentColor::ATTRIBUTE_ID, pColor->GetID());

    HRFAttributeDontSupportPersistentColor* pColor2 = AttributeSet.FindAttributeP<HRFAttributeDontSupportPersistentColor>();
    ASSERT_TRUE(pColor2->GetData());
    ASSERT_EQ(HRFAttributeDontSupportPersistentColor::ATTRIBUTE_ID, pColor2->GetID());

    //double
    HRFAttributeYResolution const* pYRes = AttributeSet.FindAttributeCP<HRFAttributeYResolution>();
    ASSERT_DOUBLE_EQ(DoubleValue, pYRes->GetData());
    ASSERT_EQ(HRFAttributeYResolution::ATTRIBUTE_ID, pYRes->GetID());

    HRFAttributeYResolution* pYRes2 = AttributeSet.FindAttributeP<HRFAttributeYResolution>();
    ASSERT_DOUBLE_EQ(DoubleValue, pYRes2->GetData());
    ASSERT_EQ(HRFAttributeYResolution::ATTRIBUTE_ID, pYRes2->GetID());

    //uint32_t
    HRFAttributeProjectedCSTypeLong const* pCSLong = AttributeSet.FindAttributeCP<HRFAttributeProjectedCSTypeLong>();
    ASSERT_DOUBLE_EQ(uint32_tValue, pCSLong->GetData());
    ASSERT_EQ(HRFAttributeProjectedCSTypeLong::ATTRIBUTE_ID, pCSLong->GetID());

    HRFAttributeProjectedCSTypeLong* pCSLong2 = AttributeSet.FindAttributeP<HRFAttributeProjectedCSTypeLong>();
    ASSERT_DOUBLE_EQ(uint32_tValue, pCSLong2->GetData());
    ASSERT_EQ(HRFAttributeProjectedCSTypeLong::ATTRIBUTE_ID, pCSLong2->GetID());

    //vector<Byte>
    HRFAttributeGPSVersionID const* pVersionID = AttributeSet.FindAttributeCP<HRFAttributeGPSVersionID>();
    ASSERT_TRUE(VectorByte.front() == pVersionID->GetData().front());
    ASSERT_TRUE(VectorByte.back() == pVersionID->GetData().back());
    ASSERT_EQ(HRFAttributeGPSVersionID::ATTRIBUTE_ID, pVersionID->GetID());

    HRFAttributeGPSVersionID* pVersionID2 = AttributeSet.FindAttributeP<HRFAttributeGPSVersionID>();
    ASSERT_TRUE(VectorByte.front() == pVersionID2->GetData().front());
    ASSERT_TRUE(VectorByte.back() == pVersionID2->GetData().back());
    ASSERT_EQ(HRFAttributeGPSVersionID::ATTRIBUTE_ID, pVersionID2->GetID());

    //Byte 
    HRFAttributeGPSAltitudeRef const* pAltRef = AttributeSet.FindAttributeCP<HRFAttributeGPSAltitudeRef>();
    ASSERT_TRUE(ByteValue == pAltRef->GetData());
    ASSERT_EQ(HRFAttributeGPSAltitudeRef::ATTRIBUTE_ID, pAltRef->GetID());

    HRFAttributeGPSAltitudeRef* pAltRef2 = AttributeSet.FindAttributeP<HRFAttributeGPSAltitudeRef>();
    ASSERT_TRUE(ByteValue == pAltRef2->GetData());
    ASSERT_EQ(HRFAttributeGPSAltitudeRef::ATTRIBUTE_ID, pAltRef2->GetID());

    //vector<UShort>
    HRFAttributeISOSpeedRatings const* pIsoSpeed = AttributeSet.FindAttributeCP<HRFAttributeISOSpeedRatings>();
    ASSERT_DOUBLE_EQ(VectorUShort.front(), pIsoSpeed->GetData().front());
    ASSERT_DOUBLE_EQ(VectorUShort.back(), pIsoSpeed->GetData().back());  
    ASSERT_EQ(HRFAttributeISOSpeedRatings::ATTRIBUTE_ID, pIsoSpeed->GetID());

    HRFAttributeISOSpeedRatings* pIsoSpeed2 = AttributeSet.FindAttributeP<HRFAttributeISOSpeedRatings>();
    ASSERT_DOUBLE_EQ(VectorUShort.front(), pIsoSpeed2->GetData().front());
    ASSERT_DOUBLE_EQ(VectorUShort.back(), pIsoSpeed2->GetData().back());  
    ASSERT_EQ(HRFAttributeISOSpeedRatings::ATTRIBUTE_ID, pIsoSpeed2->GetID());

    //<vector<char>
    HRFAttributeExifVersion const* pExif = AttributeSet.FindAttributeCP<HRFAttributeExifVersion>();
    ASSERT_TRUE(VectorChar.front() == pExif->GetData().front());
    ASSERT_TRUE(VectorChar.back() == pExif->GetData().back());  
    ASSERT_EQ(HRFAttributeExifVersion::ATTRIBUTE_ID, pExif->GetID());

    HRFAttributeExifVersion* pExif2 = AttributeSet.FindAttributeP<HRFAttributeExifVersion>();
    ASSERT_TRUE(VectorChar.front() == pExif2->GetData().front());
    ASSERT_TRUE(VectorChar.back() == pExif2->GetData().back());  
    ASSERT_EQ(HRFAttributeExifVersion::ATTRIBUTE_ID, pExif2->GetID());

    //WString
    HRFAttributeOriginalFileFormat const* pFFormat = AttributeSet.FindAttributeCP<HRFAttributeOriginalFileFormat>();
    ASSERT_TRUE(WStringValue == pFFormat->GetData());
    ASSERT_EQ(HRFAttributeOriginalFileFormat::ATTRIBUTE_ID, pFFormat->GetID());

    HRFAttributeOriginalFileFormat* pFFormat2 = AttributeSet.FindAttributeP<HRFAttributeOriginalFileFormat>();
    ASSERT_TRUE(WStringValue == pFFormat2->GetData());
    ASSERT_EQ(HRFAttributeOriginalFileFormat::ATTRIBUTE_ID, pFFormat2->GetID());

    //UInt64
    HRFAttributeOriginalFileSize const* pFileSize = AttributeSet.FindAttributeCP<HRFAttributeOriginalFileSize>();
    ASSERT_EQ(UInt64Value,  pFileSize->GetData());  
    ASSERT_EQ(HRFAttributeOriginalFileSize::ATTRIBUTE_ID, pFileSize->GetID());

    HRFAttributeOriginalFileSize* pFileSize2 = AttributeSet.FindAttributeP<HRFAttributeOriginalFileSize>();
    ASSERT_EQ(UInt64Value,  pFileSize2->GetData());  
    ASSERT_EQ(HRFAttributeOriginalFileSize::ATTRIBUTE_ID, pFileSize2->GetID());

    }

//================================================================================== 
// Clear();
//================================================================================== 
TEST_F (HPMAttributeSetTester, ClearTest)
    { 

    ASSERT_EQ(14, AttributeSet.size());

    AttributeSet.Clear();

    ASSERT_EQ(0, AttributeSet.size());
     
    }

//================================================================================== 
// Remove(const HFCPtr<HPMGenericAttribute>& pi_rpAttribute); 
// Remove(const HPMGenericAttribute& pi_rAttribute);
// Remove();
//================================================================================== 
TEST_F (HPMAttributeSetTester, RemoveTest)
    { 

    ASSERT_EQ(14, AttributeSet.size());

    AttributeSet.Remove(*pHRFAttributeImageDescription);
    ASSERT_EQ(13, AttributeSet.size());
    ASSERT_FALSE(AttributeSet.HasAttribute<HRFAttributeImageDescription>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeDontSupportPersistentColor>());

    AttributeSet.Remove(*pHRFAttributeSampleValue);
    ASSERT_EQ(12, AttributeSet.size());
    ASSERT_FALSE(AttributeSet.HasAttribute<HRFAttributeMinSampleValue>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeDontSupportPersistentColor>());

    AttributeSet.Remove(*pHRFAttributeResolutionUnit);
    ASSERT_EQ(11, AttributeSet.size());
    ASSERT_FALSE(AttributeSet.HasAttribute<HRFAttributeResolutionUnit>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeDontSupportPersistentColor>());

    AttributeSet.Remove(*pHRFAttribute3DTransformationMatrix);
    ASSERT_EQ(10, AttributeSet.size());
    ASSERT_FALSE(AttributeSet.HasAttribute<HRFAttribute3DTransformationMatrix>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeDontSupportPersistentColor>());

    AttributeSet.Remove(*pHRFAttributeForeground);
    ASSERT_EQ(9, AttributeSet.size());
    ASSERT_FALSE(AttributeSet.HasAttribute<HRFAttributeForeground>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeDontSupportPersistentColor>());

    AttributeSet.Remove<HRFAttributeExifVersion>(); 
    ASSERT_EQ(8, AttributeSet.size());
    ASSERT_FALSE(AttributeSet.HasAttribute<HRFAttributeExifVersion>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeDontSupportPersistentColor>());

    AttributeSet.Remove<HRFAttributeOriginalFileFormat>(); 
    ASSERT_EQ(7, AttributeSet.size());
    ASSERT_FALSE(AttributeSet.HasAttribute<HRFAttributeOriginalFileFormat>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeDontSupportPersistentColor>());

    AttributeSet.Remove<HRFAttributeOriginalFileSize>(); 
    ASSERT_EQ(6, AttributeSet.size());
    ASSERT_FALSE(AttributeSet.HasAttribute<HRFAttributeOriginalFileSize>());
    ASSERT_TRUE(AttributeSet.HasAttribute<HRFAttributeDontSupportPersistentColor>());

    AttributeSet.Remove<HRFAttributeDontSupportPersistentColor>(); 
    ASSERT_EQ(5, AttributeSet.size());
    ASSERT_FALSE(AttributeSet.HasAttribute<HRFAttributeDontSupportPersistentColor>());

    }

//================================================================================== 
// GetAttribute(const HPMAttributesID& pi_rID) const;
//================================================================================== 
TEST_F (HPMAttributeSetTester, GetAttributeTest)
    { 

    //String 
    HFCPtr<HPMGenericAttribute> pImage = AttributeSet.GetAttribute(static_cast<HPMAttributesID>(HRFAttributeImageDescription::ATTRIBUTE_ID));
    ASSERT_TRUE(WStringValue == static_cast<HRFAttributeImageDescription*>(pImage.GetPtr())->GetData());
    ASSERT_EQ(HRFAttributeImageDescription::ATTRIBUTE_ID, pImage->GetID());

    //Vector<double>
    HFCPtr<HPMGenericAttribute> pSample = AttributeSet.GetAttribute(static_cast<HPMAttributesID>(HRFAttributeMinSampleValue::ATTRIBUTE_ID)); 
    ASSERT_DOUBLE_EQ(VectorDouble.front(), static_cast<HRFAttributeMinSampleValue*>(pSample.GetPtr())->GetData().front());
    ASSERT_DOUBLE_EQ(VectorDouble.back(), static_cast<HRFAttributeMinSampleValue*>(pSample.GetPtr())->GetData().back());
    ASSERT_EQ(HRFAttributeMinSampleValue::ATTRIBUTE_ID, pSample->GetID());

    //UShort
    HFCPtr<HPMGenericAttribute> pResUnit = AttributeSet.GetAttribute(static_cast<HPMAttributesID>(HRFAttributeResolutionUnit::ATTRIBUTE_ID));
    ASSERT_DOUBLE_EQ(UShortValue, static_cast<HRFAttributeResolutionUnit*>(pResUnit.GetPtr())->GetData());
    ASSERT_EQ(HRFAttributeResolutionUnit::ATTRIBUTE_ID, pResUnit->GetID());

    //HFCMatrix<4,4, double>
    HFCPtr<HPMGenericAttribute> p3dTransfo = AttributeSet.GetAttribute(static_cast<HPMAttributesID>(HRFAttribute3DTransformationMatrix::ATTRIBUTE_ID));
    ASSERT_TRUE(Matrix.IsEqualTo(static_cast<HRFAttribute3DTransformationMatrix*>(p3dTransfo.GetPtr())->GetData()));
    ASSERT_EQ(HRFAttribute3DTransformationMatrix::ATTRIBUTE_ID, p3dTransfo->GetID());

    //UInt32
    HFCPtr<HPMGenericAttribute> pForeground = AttributeSet.GetAttribute(static_cast<HPMAttributesID>(HRFAttributeForeground::ATTRIBUTE_ID)); 
    ASSERT_DOUBLE_EQ(UInt32Value, static_cast<HRFAttributeForeground*>(pForeground.GetPtr())->GetData());
    ASSERT_EQ(HRFAttributeForeground::ATTRIBUTE_ID, pForeground->GetID());

    //bool
    HFCPtr<HPMGenericAttribute> pColor = AttributeSet.GetAttribute(static_cast<HPMAttributesID>(HRFAttributeDontSupportPersistentColor::ATTRIBUTE_ID));
    ASSERT_TRUE(static_cast<HRFAttributeDontSupportPersistentColor*>(pColor.GetPtr())->GetData());
    ASSERT_EQ(HRFAttributeDontSupportPersistentColor::ATTRIBUTE_ID, pColor->GetID());

    //double
    HFCPtr<HPMGenericAttribute> pYRes = AttributeSet.GetAttribute(static_cast<HPMAttributesID>(HRFAttributeYResolution::ATTRIBUTE_ID));
    ASSERT_DOUBLE_EQ(DoubleValue, static_cast<HRFAttributeYResolution*>(pYRes.GetPtr())->GetData());
    ASSERT_EQ(HRFAttributeYResolution::ATTRIBUTE_ID, pYRes->GetID());

    //uint32_t
    HFCPtr<HPMGenericAttribute> pCSLong = AttributeSet.GetAttribute(static_cast<HPMAttributesID>(HRFAttributeProjectedCSTypeLong::ATTRIBUTE_ID));
    ASSERT_DOUBLE_EQ(uint32_tValue, static_cast<HRFAttributeProjectedCSTypeLong*>(pCSLong.GetPtr())->GetData());
    ASSERT_EQ(HRFAttributeProjectedCSTypeLong::ATTRIBUTE_ID, pCSLong->GetID());

    //vector<Byte>
    HFCPtr<HPMGenericAttribute> pVersionID = AttributeSet.GetAttribute(static_cast<HPMAttributesID>(HRFAttributeGPSVersionID::ATTRIBUTE_ID));
    ASSERT_TRUE(VectorByte.front() == static_cast<HRFAttributeGPSVersionID*>(pVersionID.GetPtr())->GetData().front());
    ASSERT_TRUE(VectorByte.back() == static_cast<HRFAttributeGPSVersionID*>(pVersionID.GetPtr())->GetData().back());
    ASSERT_EQ(HRFAttributeGPSVersionID::ATTRIBUTE_ID, pVersionID->GetID());

    //Byte 
    HFCPtr<HPMGenericAttribute> pAltRef = AttributeSet.GetAttribute(static_cast<HPMAttributesID>(HRFAttributeGPSAltitudeRef::ATTRIBUTE_ID));
    ASSERT_TRUE(ByteValue == static_cast<HRFAttributeGPSAltitudeRef*>(pAltRef.GetPtr())->GetData());
    ASSERT_EQ(HRFAttributeGPSAltitudeRef::ATTRIBUTE_ID, pAltRef->GetID());

    //vector<UShort>
    HFCPtr<HPMGenericAttribute> pIsoSpeed = AttributeSet.GetAttribute(static_cast<HPMAttributesID>(HRFAttributeISOSpeedRatings::ATTRIBUTE_ID));
    ASSERT_DOUBLE_EQ(VectorUShort.front(), static_cast<HRFAttributeISOSpeedRatings*>(pIsoSpeed.GetPtr())->GetData().front());
    ASSERT_DOUBLE_EQ(VectorUShort.back(), static_cast<HRFAttributeISOSpeedRatings*>(pIsoSpeed.GetPtr())->GetData().back());  
    ASSERT_EQ(HRFAttributeISOSpeedRatings::ATTRIBUTE_ID, pIsoSpeed->GetID());

    //<vector<char>
    HFCPtr<HPMGenericAttribute> pExif = AttributeSet.GetAttribute(static_cast<HPMAttributesID>(HRFAttributeExifVersion::ATTRIBUTE_ID));
    ASSERT_TRUE(VectorChar.front() == static_cast<HRFAttributeExifVersion*>(pExif.GetPtr())->GetData().front());
    ASSERT_TRUE(VectorChar.back() == static_cast<HRFAttributeExifVersion*>(pExif.GetPtr())->GetData().back());  
    ASSERT_EQ(HRFAttributeExifVersion::ATTRIBUTE_ID, pExif->GetID());

    //WString
    HFCPtr<HPMGenericAttribute> pFileFormat2 = AttributeSet.GetAttribute(static_cast<HPMAttributesID>(HRFAttributeOriginalFileFormat::ATTRIBUTE_ID));
    ASSERT_TRUE(WStringValue == static_cast<HRFAttributeOriginalFileFormat*>(pFileFormat2.GetPtr())->GetData());
    ASSERT_EQ(HRFAttributeOriginalFileFormat::ATTRIBUTE_ID, pFileFormat2->GetID());

    //UInt64
    HFCPtr<HPMGenericAttribute> pFileSize = AttributeSet.GetAttribute(static_cast<HPMAttributesID>(HRFAttributeOriginalFileSize::ATTRIBUTE_ID));
    ASSERT_EQ(UInt64Value,  static_cast<HRFAttributeOriginalFileSize*>(pFileSize.GetPtr())->GetData());  
    ASSERT_EQ(HRFAttributeOriginalFileSize::ATTRIBUTE_ID, pFileSize->GetID());

    }



