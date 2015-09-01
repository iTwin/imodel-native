//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HVE2DComplexShapeTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DComplexShapeTester
//-----------------------------------------------------------------------------

#pragma once

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())
 
// Preparation of required environement
class HVE2DComplexShapeTester : public EnvironnementTest
    {   
protected:

    HVE2DComplexShapeTester();
    ~HVE2DComplexShapeTester() {};

    HVE2DComplexShape ComplexShape1;
    
    HVE2DSegment Segment1A; 
    HVE2DSegment Segment2A;
    HVE2DSegment Segment3A;
    HVE2DSegment Segment4A;
    HVE2DComplexLinear MyLinear1; 

    HVE2DSegment Segment1B;
    HVE2DSegment Segment2B;
    HVE2DSegment Segment3B;
    HVE2DSegment Segment4B;
    HVE2DComplexLinear MyLinear2;

    HVE2DSegment    Segment1C;
    HVE2DSegment    Segment2C;
    HVE2DSegment    Segment3C;
    HVE2DSegment    Segment4C;
    HVE2DComplexLinear  MyLinear3;

    HVE2DSegment    Segment1D;
    HVE2DSegment    Segment2D;
    HVE2DSegment    Segment3D;
    HVE2DSegment    Segment4D;

    HVE2DComplexLinear  MyLinear4;

    HVE2DSegment    Segment1E;
    HVE2DSegment    Segment2E;
    HVE2DSegment    Segment3E;
    HVE2DSegment    Segment4E;

    HVE2DComplexLinear  MyLinear5;

    HGF2DLocation Poly1Point0d0;
    HGF2DLocation Poly1Point0d1;
    HGF2DLocation Poly1Point0d5;
    HGF2DLocation Poly1Point1d0;

    };