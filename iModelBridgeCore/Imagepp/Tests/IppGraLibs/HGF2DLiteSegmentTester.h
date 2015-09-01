//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DLiteSegmentTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DLiteSegmentTester
//-----------------------------------------------------------------------------

#pragma once

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())
 
// Preparation of required environement
class HGF2DLiteSegmentTester : public testing::Test 
    {   

protected :

    HGF2DLiteSegmentTester();
    ~HGF2DLiteSegmentTester(){};

    // VERTICAL SEGMENT
    HGF2DLiteSegment    VerticalSegment1;
    HGF2DLiteSegment    VerticalSegment2;
    HGF2DLiteSegment    VerticalSegment3;
    HGF2DLiteSegment    VerticalSegment4;
    HGF2DLiteSegment    VerticalSegment5;
    HGF2DLiteSegment    CloseVerticalSegment1;

    HGF2DPosition   VerticalPoint0d0;
    HGF2DPosition   VerticalPoint0d1;
    HGF2DPosition   VerticalPoint0d5;
    HGF2DPosition   VerticalPoint1d0;

    HGF2DPosition   Vertical3Point0d0;
    HGF2DPosition   Vertical3Point0d1;
    HGF2DPosition   Vertical3Point0d5;
    HGF2DPosition   Vertical3Point1d0;

    HGF2DPosition   VerticalMidPoint1;
    HGF2DPosition   VerticalMidPoint3;

    HGF2DPosition   VerticalClosePoint1A;
    HGF2DPosition   VerticalClosePoint1B;
    HGF2DPosition   VerticalClosePoint1C;
    HGF2DPosition   VerticalClosePoint1D;
    HGF2DPosition   VerticalCloseMidPoint1;

    HGF2DPosition   VerticalClosePoint3A;
    HGF2DPosition   VerticalClosePoint3B;
    HGF2DPosition   VerticalClosePoint3C;
    HGF2DPosition   VerticalClosePoint3D;
    HGF2DPosition   VerticalCloseMidPoint3;

    // HORIZONTAL SEGMENT
    HGF2DLiteSegment    HorizontalSegment1;
    HGF2DLiteSegment    HorizontalSegment2;
    HGF2DLiteSegment    HorizontalSegment3;
    HGF2DLiteSegment    HorizontalSegment5;

    HGF2DPosition   HorizontalPoint0d0;
    HGF2DPosition   HorizontalPoint0d1;
    HGF2DPosition   HorizontalPoint0d5;
    HGF2DPosition   HorizontalPoint1d0;

    HGF2DPosition   Horizontal3Point0d0;
    HGF2DPosition   Horizontal3Point0d1;
    HGF2DPosition   Horizontal3Point0d5;
    HGF2DPosition   Horizontal3Point1d0;

    HGF2DPosition   HorizontalMidPoint1;
    HGF2DPosition   HorizontalMidPoint3;

    HGF2DPosition   HorizontalClosePoint1A;
    HGF2DPosition   HorizontalClosePoint1B;
    HGF2DPosition   HorizontalClosePoint1C;
    HGF2DPosition   HorizontalClosePoint1D;
    HGF2DPosition   HorizontalCloseMidPoint1;

    HGF2DPosition   HorizontalClosePoint3A;
    HGF2DPosition   HorizontalClosePoint3B;
    HGF2DPosition   HorizontalClosePoint3C;
    HGF2DPosition   HorizontalClosePoint3D;
    HGF2DPosition   HorizontalCloseMidPoint3;

    // MISC SEGMENTS
    HGF2DLiteSegment    MiscSegment1;
    HGF2DLiteSegment    MiscSegment2;
    HGF2DLiteSegment    MiscSegment6;
    HGF2DLiteSegment    MiscSegment7;
    HGF2DLiteSegment    DisjointSegment1;
    HGF2DLiteSegment    ContiguousExtentSegment1;
    HGF2DLiteSegment    FlirtingExtentSegment1;
    HGF2DLiteSegment    FlirtingExtentLinkedSegment1;
    HGF2DLiteSegment    ParallelSegment1;
    HGF2DLiteSegment    LinkedParallelSegment1;
    HGF2DLiteSegment    NearParallelSegment1;
    HGF2DLiteSegment    CloseNearParallelSegment1;
    HGF2DLiteSegment    ConnectedSegment1;
    HGF2DLiteSegment    ConnectingSegment1;
    HGF2DLiteSegment    ConnectedSegment1A;
    HGF2DLiteSegment    ConnectingSegment1A;
    HGF2DLiteSegment    LinkedSegment1;
    HGF2DLiteSegment    LinkedSegment1A;

    HGF2DPosition   Misc3Point0d0;
    HGF2DPosition   MiscMidPoint1;
    HGF2DPosition   MiscClosePoint3A;
    HGF2DPosition   MiscClosePoint3B;
    HGF2DPosition   MiscClosePoint3C;
    HGF2DPosition   MiscClosePoint3D;
    HGF2DPosition   MiscCloseMidPoint3;

    HGF2DPosition   VeryFarPoint;
    HGF2DPosition   VeryFarAlignedPoint;
    HGF2DPosition   VeryFarAlignedNegativePoint;
    HGF2DPosition   VeryFarNegativePoint;
    HGF2DPosition   MidPoint;
    HGF2DPosition   MiscMidPoint6;

    // LARGE SEGMENTS
    HGF2DLiteSegment    LargeSegment1;
    HGF2DLiteSegment    LargeSegment2;
    HGF2DLiteSegment    ParallelLargeSegment1;

    HGF2DPosition   LargePoint0d0;
    HGF2DPosition   LargePoint0d1;
    HGF2DPosition   LargePoint0d5;
    HGF2DPosition   LargePoint1d0;

    HGF2DPosition   LargeMidPoint1;

    HGF2DPosition   LargeClosePoint1A;
    HGF2DPosition   LargeClosePoint1B;
    HGF2DPosition   LargeClosePoint1C;
    HGF2DPosition   LargeClosePoint1D;
    HGF2DPosition   LargeCloseMidPoint1;

    // POSITIVE SEGMENTS
    HGF2DLiteSegment    PositiveSegment1;
    HGF2DLiteSegment    PositiveSegment2;
    HGF2DLiteSegment    ParallelPositiveSegment1;

    HGF2DPosition   PositivePoint0d0;
    HGF2DPosition   PositivePoint0d1;
    HGF2DPosition   PositivePoint0d5;
    HGF2DPosition   PositivePoint1d0;

    HGF2DPosition   PositiveMidPoint1;

    HGF2DPosition   PositiveClosePoint1A;
    HGF2DPosition   PositiveClosePoint1B;
    HGF2DPosition   PositiveClosePoint1C;
    HGF2DPosition   PositiveClosePoint1D;
    HGF2DPosition   PositiveCloseMidPoint1;

    // NEGATIVE SEGMENTS
    HGF2DLiteSegment   NegativeSegment1;
    HGF2DLiteSegment   NegativeSegment2;
    HGF2DLiteSegment   ParallelNegativeSegment1;

    HGF2DPosition   NegativePoint0d0;
    HGF2DPosition   NegativePoint0d1;
    HGF2DPosition   NegativePoint0d5;
    HGF2DPosition   NegativePoint1d0;

    HGF2DPosition   NegativeMidPoint1;

    HGF2DPosition   NegativeClosePoint1A;
    HGF2DPosition   NegativeClosePoint1B;
    HGF2DPosition   NegativeClosePoint1C;
    HGF2DPosition   NegativeClosePoint1D;
    HGF2DPosition   NegativeCloseMidPoint1;

    };

