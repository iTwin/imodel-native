//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/HGF2DHoledShapeTester.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DHoledShapeTester
//-----------------------------------------------------------------------------

#pragma once

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())
 
// Preparation of required environement
class HGF2DHoledShapeTester : public EnvironnementTest
    {   

protected:

    HGF2DHoledShapeTester();

    //Linears
    HGF2DSegment Segment1A; 
    HGF2DSegment Segment2A;
    HGF2DSegment Segment3A;
    HGF2DSegment Segment4A;
    HGF2DPolySegment MyLinear1; 

    HGF2DSegment Segment1B;
    HGF2DSegment Segment2B;
    HGF2DSegment Segment3B;
    HGF2DSegment Segment4B;
    HGF2DPolySegment MyLinear2;

    HGF2DSegment    Segment1D;
    HGF2DSegment    Segment2D;
    HGF2DSegment    Segment3D;
    HGF2DSegment    Segment4D;
    HGF2DPolySegment  MyLinear4;

    HGF2DSegment    Segment1E;
    HGF2DSegment    Segment2E;
    HGF2DSegment    Segment3E;
    HGF2DSegment    Segment4E;
    HGF2DPolySegment  MyLinear5;

    HGF2DSegment    Segment1F;
    HGF2DSegment    Segment2F;
    HGF2DSegment    Segment3F;
    HGF2DSegment    Segment4F;
    HGF2DPolySegment  MyLinear6;


    HGF2DSegment    Segment1G;
    HGF2DSegment    Segment2G;
    HGF2DSegment    Segment3G;
    HGF2DSegment    Segment4G;
    HGF2DPolySegment  MyLinear7;

    //Points
    HGF2DPosition Poly1Point0d0;
    HGF2DPosition Poly1Point0d1;
    HGF2DPosition Poly1Point0d5;
    HGF2DPosition Poly1Point1d0 ;

    };