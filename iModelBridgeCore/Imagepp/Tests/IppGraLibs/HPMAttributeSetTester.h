//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HPMAttributeSetTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPMAttributeSetTester
//-----------------------------------------------------------------------------

#pragma once

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())
 
// Preparation of required environement
class HPMAttributeSetTester : public testing::Test
    {   

protected :
  
    HPMAttributeSetTester();
    ~HPMAttributeSetTester() {};

    int32_t                 i;
    int32_t                 j;

    //Default Value
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

    //HPMAttribute
    HFCPtr<HPMGenericAttribute>   pHRFAttributeImageDescription; 
    HFCPtr<HPMGenericAttribute>   pHRFAttributeSampleValue;
    HFCPtr<HPMGenericAttribute>   pHRFAttributeResolutionUnit;
    HFCPtr<HPMGenericAttribute>   pHRFAttribute3DTransformationMatrix;
    HFCPtr<HPMGenericAttribute>   pHRFAttributeForeground;
    HFCPtr<HPMGenericAttribute>   pHRFAttributeYResolution;
    HFCPtr<HPMGenericAttribute>   pHRFAttributeProjectedCSTypeLong;
    HFCPtr<HPMGenericAttribute>   pHRFAttributeGPSVersionID;
    HFCPtr<HPMGenericAttribute>   pHRFAttributeGPSAltitudeRef;
    HFCPtr<HPMGenericAttribute>   pHRFAttributeISOSpeedRatings;
    HFCPtr<HPMGenericAttribute>   pHRFAttributeExifVersion;
    HFCPtr<HPMGenericAttribute>   pHRFAttributeOriginalFileFormat;
    HFCPtr<HPMGenericAttribute>   pHRFAttributeOriginalFileSize;
    HFCPtr<HPMGenericAttribute>   pHRFAttributeDontSupportPersistentColor;

    //HPMAttributeSet
    HPMAttributeSet AttributeSet;

    };