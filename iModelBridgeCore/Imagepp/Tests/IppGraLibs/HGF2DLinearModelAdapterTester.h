//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DLinearModelAdapterTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DLinearModelAdapterTester
//-----------------------------------------------------------------------------

#pragma once
 
// Preparation of required environement
class HGF2DLinearModelAdapterTester : public testing::Test
    {   

protected :

    HGF2DLinearModelAdapterTester();
    ~HGF2DLinearModelAdapterTester() {};

    // NonLinearIdentity
    HGF2DNonLinearTestIdentity NonLinearTestIdentity;
    HGF2DLiteExtent MyArea;

    // HGF2DLinearModelAdapter
    HGF2DLinearModelAdapter FirstAltaPhotoModel;
    HGF2DLinearModelAdapter FirstAltaPhotoModel2;
    HGF2DLinearModelAdapter AltaPhotoModel1;
    HGF2DLinearModelAdapter AltaPhotoModel2;

    //General
    HGF2DDisplacement   Translation;
    double              Rotation;
    double              ScalingX;
    double              ScalingY;
    double              Anortho;
    HGF2DAffine         Affine1;  
    HGF2DIdentity       MyIdentity; 

    // COORDINATE SYSTEMS
    HFCPtr<HGF2DCoordSys>   pWorld;
    HFCPtr<HGF2DCoordSys>   pSys1;

    };