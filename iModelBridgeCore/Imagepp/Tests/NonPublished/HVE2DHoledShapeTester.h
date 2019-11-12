//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DHoledShapeTester
//-----------------------------------------------------------------------------

#pragma once

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())
 
// Preparation of required environement
class HVE2DHoledShapeTester : public EnvironnementTest
    {   

protected:

    HVE2DHoledShapeTester();

    //Linears
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

    HVE2DSegment    Segment1F;
    HVE2DSegment    Segment2F;
    HVE2DSegment    Segment3F;
    HVE2DSegment    Segment4F;
    HVE2DComplexLinear  MyLinear6;

    HVE2DSegment    Segment1G;
    HVE2DSegment    Segment2G;
    HVE2DSegment    Segment3G;
    HVE2DSegment    Segment4G;
    HVE2DComplexLinear  MyLinear7;

    //Points
    HGF2DLocation Poly1Point0d0;
    HGF2DLocation Poly1Point0d1;
    HGF2DLocation Poly1Point0d5;
    HGF2DLocation Poly1Point1d0 ;

    };
