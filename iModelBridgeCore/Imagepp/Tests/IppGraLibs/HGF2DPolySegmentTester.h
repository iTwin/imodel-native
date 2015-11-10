//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DPolySegmentTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DPolySegmentTester
//-----------------------------------------------------------------------------

#pragma once
 
// Preparation of required environement
class HGF2DPolySegmentTester : public EnvironnementTest 
    {   

protected : 
   
    HGF2DPolySegmentTester();

    // VERTICAL PolySegment
    HGF2DPolySegment VerticalPolySegment1;
    HGF2DPolySegment VerticalPolySegment2;
    HGF2DPolySegment VerticalPolySegment3;
    HGF2DPolySegment VerticalPolySegment4;
    HGF2DPolySegment VerticalPolySegment5;
    HGF2DPolySegment CloseVerticalPolySegment1;

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

    // HORIZONTAL PolySegment
    HGF2DPolySegment    HorizontalPolySegment1;
    HGF2DPolySegment    HorizontalPolySegment2;
    HGF2DPolySegment    HorizontalPolySegment3;
    HGF2DPolySegment    HorizontalPolySegment5;

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

    // MISC PolySegmentS
    HGF2DPolySegment MiscPolySegment1;
    HGF2DPolySegment MiscPolySegment2;
    HGF2DPolySegment MiscPolySegment3;
    HGF2DPolySegment MiscPolySegment4;
    HGF2DPolySegment MiscPolySegment6;
    HGF2DPolySegment MiscPolySegment7;
    HGF2DPolySegment DisjointPolySegment1;
    HGF2DPolySegment ContiguousExtentPolySegment1;
    HGF2DPolySegment FlirtingExtentPolySegment1;
    HGF2DPolySegment FlirtingExtentLinkedPolySegment1;
    HGF2DPolySegment ParallelPolySegment1;
    HGF2DPolySegment LinkedParallelPolySegment1;
    HGF2DPolySegment NearParallelPolySegment1;
    HGF2DPolySegment CloseNearParallelPolySegment1;
    HGF2DPolySegment ConnectedPolySegment1;
    HGF2DPolySegment ConnectingPolySegment1;
    HGF2DPolySegment ConnectedPolySegment1A;
    HGF2DPolySegment ConnectingPolySegment1A;
    HGF2DPolySegment LinkedPolySegment1;
    HGF2DPolySegment LinkedPolySegment1A;
    HGF2DPolySegment MiscPolySegment3A;

    HGF2DPosition   Misc3Point0d0;
    HGF2DPosition   Misc3Point0d1;
    HGF2DPosition   Misc3Point0d5;
    HGF2DPosition   Misc3Point1d0;
    HGF2DPosition   MiscMidPoint1;
    HGF2DPosition   MiscMidPoint3;

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

    // LARGE PolySegmentS
    HGF2DPolySegment    LargePolySegment1;
    HGF2DPolySegment    LargePolySegment2;
    HGF2DPolySegment    ParallelLargePolySegment1;

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

    // POSITIVE PolySegmentS
    HGF2DPolySegment    PositivePolySegment1;
    HGF2DPolySegment    PositivePolySegment2;
    HGF2DPolySegment    ParallelPositivePolySegment1;

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

    // NEGATIVE PolySegmentS
    HGF2DPolySegment    NegativePolySegment1;
    HGF2DPolySegment    NegativePolySegment2;
    HGF2DPolySegment    ParallelNegativePolySegment1;

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

    // NULL PolySegmentS
    HGF2DPolySegment    NullPolySegment1;
    HGF2DPolySegment    NullPolySegment2;

    // PolySegments
    HGF2DPolySegment      EmptyPolySegment;

    HGF2DPolySegment      PolySegment1; // Open

    HGF2DPosition   PolySegment1Point0d0;
    HGF2DPosition   PolySegment1Point0d1;
    HGF2DPosition   PolySegment1Point0d5;
    HGF2DPosition   PolySegment1Point1d0;

    HGF2DPosition   PolySegmentMidPoint1;

    HGF2DPosition   PolySegmentClosePoint1A;
    HGF2DPosition   PolySegmentClosePoint1B;
    HGF2DPosition   PolySegmentClosePoint1C;
    HGF2DPosition   PolySegmentClosePoint1D;
    HGF2DPosition   PolySegmentCloseMidPoint1;

    HGF2DPolySegment      PolySegment2;  // AutoClosed
    
    HGF2DPosition   PolySegment2Point0d0;
    HGF2DPosition   PolySegment2Point0d1;
    HGF2DPosition   PolySegment2Point0d5;
    HGF2DPosition   PolySegment2Point1d0;

    HGF2DPosition   PolySegmentMidPoint2;

    HGF2DPosition   PolySegmentClosePoint2A;
    HGF2DPosition   PolySegmentClosePoint2B;
    HGF2DPosition   PolySegmentClosePoint2C;
    HGF2DPosition   PolySegmentClosePoint2D;
    HGF2DPosition   PolySegmentCloseMidPoint2;

    // epsilon size container
    HGF2DPolySegment      PolySegment3; // Open

    HGF2DPosition   PolySegment3Point0d0;
    HGF2DPosition   PolySegment3Point0d1;
    HGF2DPosition   PolySegment3Point0d5;
    HGF2DPosition   PolySegment3Point1d0;

    HGF2DPosition   PolySegmentMidPoint3;

    HGF2DPosition   PolySegmentClosePoint3A;
    HGF2DPosition   PolySegmentClosePoint3B;
    HGF2DPosition   PolySegmentClosePoint3C;
    HGF2DPosition   PolySegmentClosePoint3D;
    HGF2DPosition   PolySegmentCloseMidPoint3;

     // Used for intersection tests
    HGF2DPolySegment      ComplexPolySegmentCase1;

    HGF2DPolySegment      ComplexPolySegmentCase2;
    HGF2DPolySegment      ComplexPolySegmentCase3;
    HGF2DPolySegment      ComplexPolySegmentCase4;
    HGF2DPolySegment      ComplexPolySegmentCase5;
    HGF2DPolySegment      ComplexPolySegmentCase5A;
    HGF2DPolySegment      ComplexPolySegmentCase6;
    HGF2DPolySegment      ComplexPolySegmentCase7;

    HGF2DSegment    AutoCrossingPolySegment1Segment1;
    HGF2DSegment    AutoCrossingPolySegment1Segment2;
    HGF2DSegment    AutoCrossingPolySegment1Segment3;
    HGF2DSegment    AutoCrossingPolySegment1Segment4;
    HGF2DSegment    AutoCrossingPolySegment1Segment5;
    HGF2DPolySegment      AutoCrossingPolySegment1;

    HGF2DSegment    AutoCrossingPolySegment2Segment1;
    HGF2DSegment    AutoCrossingPolySegment2Segment2;
    HGF2DSegment    AutoCrossingPolySegment2Segment3;
    HGF2DSegment    AutoCrossingPolySegment2Segment4;
    HGF2DPolySegment      AutoCrossingPolySegment2;

    HGF2DPolySegment      AutoCrossingPolySegment3;
    HGF2DPolySegment      AutoCrossingPolySegment4;
    HGF2DPolySegment      AutoCrossingPolySegment5;
    HGF2DPolySegment      AutoCrossingPolySegment6;
    HGF2DPolySegment      AutoCrossingPolySegment7;
    HGF2DPolySegment      AutoCrossingPolySegment8;
    HGF2DPolySegment      AutoCrossingPolySegment9;
    HGF2DPolySegment      AutoCrossingPolySegment10;
    HGF2DPolySegment      AutoCrossingPolySegment11;
    HGF2DPolySegment      AutoCrossingPolySegment12;
    HGF2DPolySegment      AutoCrossingPolySegment13;

    HGF2DSegment    AutoConnectingPolySegment1Segment1;
    HGF2DSegment    AutoConnectingPolySegment1Segment2;
    HGF2DSegment    AutoConnectingPolySegment1Segment3;
    HGF2DSegment    AutoConnectingPolySegment1Segment4;
    HGF2DPolySegment      AutoConnectingPolySegment1;

    HGF2DSegment    DisjointPolySegment1Segment11;
    HGF2DSegment    DisjointPolySegment1Segment21;
    HGF2DPolySegment      DisjointPolySegment11;

    HGF2DSegment    ContiguousExtentPolySegment1Segment11;
    HGF2DSegment    ContiguousExtentPolySegment1Segment21;
    HGF2DPolySegment      ContiguousExtentPolySegment11;

    HGF2DSegment    FlirtingExtentPolySegment1Segment11;
    HGF2DSegment    FlirtingExtentPolySegment1Segment21;
    HGF2DPolySegment      FlirtingExtentPolySegment11;

    HGF2DSegment    FlirtingExtentLinkedPolySegment1Segment11;
    HGF2DSegment    FlirtingExtentLinkedPolySegment1Segment21;
    HGF2DPolySegment      FlirtingExtentLinkedPolySegment11;

    HGF2DSegment    ConnectedPolySegment1Segment11;
    HGF2DSegment    ConnectedPolySegment1Segment21;
    HGF2DPolySegment      ConnectedPolySegment11;

    HGF2DSegment    ConnectingPolySegment1Segment11;
    HGF2DSegment    ConnectingPolySegment1Segment21;
    HGF2DPolySegment      ConnectingPolySegment11;

    HGF2DSegment    ConnectedPolySegment1ASegment11;
    HGF2DSegment    ConnectedPolySegment1ASegment21;
    HGF2DPolySegment      ConnectedPolySegment1A1;

    HGF2DSegment    ConnectingPolySegment1ASegment11;
    HGF2DSegment    ConnectingPolySegment1ASegment21;
    HGF2DPolySegment      ConnectingPolySegment1A1;

    HGF2DSegment    LinkedPolySegment1Segment11;
    HGF2DSegment    LinkedPolySegment1Segment21;
    HGF2DPolySegment      LinkedPolySegment11;

    HGF2DSegment    LinkedPolySegment1ASegment11;
    HGF2DSegment    LinkedPolySegment1ASegment21;
    HGF2DPolySegment      LinkedPolySegment1A1;
    
    };