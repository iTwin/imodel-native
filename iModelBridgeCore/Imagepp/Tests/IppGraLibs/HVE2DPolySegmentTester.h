//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HVE2DPolySegmentTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DPolySegmentTester
//-----------------------------------------------------------------------------

#pragma once
 
// Preparation of required environement
class HVE2DPolySegmentTester : public EnvironnementTest 
    {   

protected : 
   
    HVE2DPolySegmentTester();

    // VERTICAL PolySegment
    HVE2DPolySegment VerticalPolySegment1;
    HVE2DPolySegment VerticalPolySegment2;
    HVE2DPolySegment VerticalPolySegment3;
    HVE2DPolySegment VerticalPolySegment4;
    HVE2DPolySegment VerticalPolySegment5;
    HVE2DPolySegment CloseVerticalPolySegment1;

    HGF2DLocation   VerticalPoint0d0;
    HGF2DLocation   VerticalPoint0d1;
    HGF2DLocation   VerticalPoint0d5;
    HGF2DLocation   VerticalPoint1d0;

    HGF2DLocation   Vertical3Point0d0;
    HGF2DLocation   Vertical3Point0d1;
    HGF2DLocation   Vertical3Point0d5;
    HGF2DLocation   Vertical3Point1d0;

    HGF2DLocation   VerticalMidPoint1;
    HGF2DLocation   VerticalMidPoint3;

    HGF2DLocation   VerticalClosePoint1A;
    HGF2DLocation   VerticalClosePoint1B;
    HGF2DLocation   VerticalClosePoint1C;
    HGF2DLocation   VerticalClosePoint1D;
    HGF2DLocation   VerticalCloseMidPoint1;

    HGF2DLocation   VerticalClosePoint3A;
    HGF2DLocation   VerticalClosePoint3B;
    HGF2DLocation   VerticalClosePoint3C;
    HGF2DLocation   VerticalClosePoint3D;
    HGF2DLocation   VerticalCloseMidPoint3;

    // HORIZONTAL PolySegment
    HVE2DPolySegment    HorizontalPolySegment1;
    HVE2DPolySegment    HorizontalPolySegment2;
    HVE2DPolySegment    HorizontalPolySegment3;
    HVE2DPolySegment    HorizontalPolySegment5;

    HGF2DLocation   HorizontalPoint0d0;
    HGF2DLocation   HorizontalPoint0d1;
    HGF2DLocation   HorizontalPoint0d5;
    HGF2DLocation   HorizontalPoint1d0;

    HGF2DLocation   Horizontal3Point0d0;
    HGF2DLocation   Horizontal3Point0d1;
    HGF2DLocation   Horizontal3Point0d5;
    HGF2DLocation   Horizontal3Point1d0;

    HGF2DLocation   HorizontalMidPoint1;
    HGF2DLocation   HorizontalMidPoint3;

    HGF2DLocation   HorizontalClosePoint1A;
    HGF2DLocation   HorizontalClosePoint1B;
    HGF2DLocation   HorizontalClosePoint1C;
    HGF2DLocation   HorizontalClosePoint1D;
    HGF2DLocation   HorizontalCloseMidPoint1;

    HGF2DLocation   HorizontalClosePoint3A;
    HGF2DLocation   HorizontalClosePoint3B;
    HGF2DLocation   HorizontalClosePoint3C;
    HGF2DLocation   HorizontalClosePoint3D;
    HGF2DLocation   HorizontalCloseMidPoint3;

    // MISC PolySegmentS
    HVE2DPolySegment MiscPolySegment1;
    HVE2DPolySegment MiscPolySegment2;
    HVE2DPolySegment MiscPolySegment3;
    HVE2DPolySegment MiscPolySegment4;
    HVE2DPolySegment MiscPolySegment6;
    HVE2DPolySegment MiscPolySegment7;
    HVE2DPolySegment DisjointPolySegment1;
    HVE2DPolySegment ContiguousExtentPolySegment1;
    HVE2DPolySegment FlirtingExtentPolySegment1;
    HVE2DPolySegment FlirtingExtentLinkedPolySegment1;
    HVE2DPolySegment ParallelPolySegment1;
    HVE2DPolySegment LinkedParallelPolySegment1;
    HVE2DPolySegment NearParallelPolySegment1;
    HVE2DPolySegment CloseNearParallelPolySegment1;
    HVE2DPolySegment ConnectedPolySegment1;
    HVE2DPolySegment ConnectingPolySegment1;
    HVE2DPolySegment ConnectedPolySegment1A;
    HVE2DPolySegment ConnectingPolySegment1A;
    HVE2DPolySegment LinkedPolySegment1;
    HVE2DPolySegment LinkedPolySegment1A;
    HVE2DPolySegment MiscPolySegment3A;

    HGF2DLocation   Misc3Point0d0;
    HGF2DLocation   Misc3Point0d1;
    HGF2DLocation   Misc3Point0d5;
    HGF2DLocation   Misc3Point1d0;
    HGF2DLocation   MiscMidPoint1;
    HGF2DLocation   MiscMidPoint3;

    HGF2DLocation   MiscClosePoint3A;
    HGF2DLocation   MiscClosePoint3B;
    HGF2DLocation   MiscClosePoint3C;
    HGF2DLocation   MiscClosePoint3D;
    HGF2DLocation   MiscCloseMidPoint3;

    HGF2DLocation   VeryFarPoint;
    HGF2DLocation   VeryFarAlignedPoint;
    HGF2DLocation   VeryFarAlignedNegativePoint;
    HGF2DLocation   VeryFarNegativePoint;
    HGF2DLocation   MidPoint;

    HGF2DLocation   MiscMidPoint6;

    // LARGE PolySegmentS
    HVE2DPolySegment    LargePolySegment1;
    HVE2DPolySegment    LargePolySegment2;
    HVE2DPolySegment    ParallelLargePolySegment1;

    HGF2DLocation   LargePoint0d0;
    HGF2DLocation   LargePoint0d1;
    HGF2DLocation   LargePoint0d5;
    HGF2DLocation   LargePoint1d0;

    HGF2DLocation   LargeMidPoint1;

    HGF2DLocation   LargeClosePoint1A;
    HGF2DLocation   LargeClosePoint1B;
    HGF2DLocation   LargeClosePoint1C;
    HGF2DLocation   LargeClosePoint1D;
    HGF2DLocation   LargeCloseMidPoint1;

    // POSITIVE PolySegmentS
    HVE2DPolySegment    PositivePolySegment1;
    HVE2DPolySegment    PositivePolySegment2;
    HVE2DPolySegment    ParallelPositivePolySegment1;

    HGF2DLocation   PositivePoint0d0;
    HGF2DLocation   PositivePoint0d1;
    HGF2DLocation   PositivePoint0d5;
    HGF2DLocation   PositivePoint1d0;

    HGF2DLocation   PositiveMidPoint1;

    HGF2DLocation   PositiveClosePoint1A;
    HGF2DLocation   PositiveClosePoint1B;
    HGF2DLocation   PositiveClosePoint1C;
    HGF2DLocation   PositiveClosePoint1D;
    HGF2DLocation   PositiveCloseMidPoint1;

    // NEGATIVE PolySegmentS
    HVE2DPolySegment    NegativePolySegment1;
    HVE2DPolySegment    NegativePolySegment2;
    HVE2DPolySegment    ParallelNegativePolySegment1;

    HGF2DLocation   NegativePoint0d0;
    HGF2DLocation   NegativePoint0d1;
    HGF2DLocation   NegativePoint0d5;
    HGF2DLocation   NegativePoint1d0;

    HGF2DLocation   NegativeMidPoint1;

    HGF2DLocation   NegativeClosePoint1A;
    HGF2DLocation   NegativeClosePoint1B;
    HGF2DLocation   NegativeClosePoint1C;
    HGF2DLocation   NegativeClosePoint1D;
    HGF2DLocation   NegativeCloseMidPoint1;

    // NULL PolySegmentS
    HVE2DPolySegment    NullPolySegment1;
    HVE2DPolySegment    NullPolySegment2;

    // PolySegments
    HVE2DPolySegment      EmptyPolySegment;

    HVE2DPolySegment      PolySegment1; // Open

    HGF2DLocation   PolySegment1Point0d0;
    HGF2DLocation   PolySegment1Point0d1;
    HGF2DLocation   PolySegment1Point0d5;
    HGF2DLocation   PolySegment1Point1d0;

    HGF2DLocation   PolySegmentMidPoint1;

    HGF2DLocation   PolySegmentClosePoint1A;
    HGF2DLocation   PolySegmentClosePoint1B;
    HGF2DLocation   PolySegmentClosePoint1C;
    HGF2DLocation   PolySegmentClosePoint1D;
    HGF2DLocation   PolySegmentCloseMidPoint1;

    HVE2DPolySegment      PolySegment2;  // AutoClosed
    
    HGF2DLocation   PolySegment2Point0d0;
    HGF2DLocation   PolySegment2Point0d1;
    HGF2DLocation   PolySegment2Point0d5;
    HGF2DLocation   PolySegment2Point1d0;

    HGF2DLocation   PolySegmentMidPoint2;

    HGF2DLocation   PolySegmentClosePoint2A;
    HGF2DLocation   PolySegmentClosePoint2B;
    HGF2DLocation   PolySegmentClosePoint2C;
    HGF2DLocation   PolySegmentClosePoint2D;
    HGF2DLocation   PolySegmentCloseMidPoint2;

    // epsilon size container
    HVE2DPolySegment      PolySegment3; // Open

    HGF2DLocation   PolySegment3Point0d0;
    HGF2DLocation   PolySegment3Point0d1;
    HGF2DLocation   PolySegment3Point0d5;
    HGF2DLocation   PolySegment3Point1d0;

    HGF2DLocation   PolySegmentMidPoint3;

    HGF2DLocation   PolySegmentClosePoint3A;
    HGF2DLocation   PolySegmentClosePoint3B;
    HGF2DLocation   PolySegmentClosePoint3C;
    HGF2DLocation   PolySegmentClosePoint3D;
    HGF2DLocation   PolySegmentCloseMidPoint3;

     // Used for intersection tests
    HVE2DPolySegment      ComplexPolySegmentCase1;

    HVE2DPolySegment      ComplexPolySegmentCase2;
    HVE2DPolySegment      ComplexPolySegmentCase3;
    HVE2DPolySegment      ComplexPolySegmentCase4;
    HVE2DPolySegment      ComplexPolySegmentCase5;
    HVE2DPolySegment      ComplexPolySegmentCase5A;
    HVE2DPolySegment      ComplexPolySegmentCase6;
    HVE2DPolySegment      ComplexPolySegmentCase7;

    HVE2DSegment    AutoCrossingPolySegment1Segment1;
    HVE2DSegment    AutoCrossingPolySegment1Segment2;
    HVE2DSegment    AutoCrossingPolySegment1Segment3;
    HVE2DSegment    AutoCrossingPolySegment1Segment4;
    HVE2DSegment    AutoCrossingPolySegment1Segment5;
    HVE2DPolySegment      AutoCrossingPolySegment1;

    HVE2DSegment    AutoCrossingPolySegment2Segment1;
    HVE2DSegment    AutoCrossingPolySegment2Segment2;
    HVE2DSegment    AutoCrossingPolySegment2Segment3;
    HVE2DSegment    AutoCrossingPolySegment2Segment4;
    HVE2DPolySegment      AutoCrossingPolySegment2;

    HVE2DPolySegment      AutoCrossingPolySegment3;
    HVE2DPolySegment      AutoCrossingPolySegment4;
    HVE2DPolySegment      AutoCrossingPolySegment5;
    HVE2DPolySegment      AutoCrossingPolySegment6;
    HVE2DPolySegment      AutoCrossingPolySegment7;
    HVE2DPolySegment      AutoCrossingPolySegment8;
    HVE2DPolySegment      AutoCrossingPolySegment9;
    HVE2DPolySegment      AutoCrossingPolySegment10;
    HVE2DPolySegment      AutoCrossingPolySegment11;
    HVE2DPolySegment      AutoCrossingPolySegment12;
    HVE2DPolySegment      AutoCrossingPolySegment13;

    HVE2DSegment    AutoConnectingPolySegment1Segment1;
    HVE2DSegment    AutoConnectingPolySegment1Segment2;
    HVE2DSegment    AutoConnectingPolySegment1Segment3;
    HVE2DSegment    AutoConnectingPolySegment1Segment4;
    HVE2DPolySegment      AutoConnectingPolySegment1;

    HVE2DSegment    DisjointPolySegment1Segment11;
    HVE2DSegment    DisjointPolySegment1Segment21;
    HVE2DPolySegment      DisjointPolySegment11;

    HVE2DSegment    ContiguousExtentPolySegment1Segment11;
    HVE2DSegment    ContiguousExtentPolySegment1Segment21;
    HVE2DPolySegment      ContiguousExtentPolySegment11;

    HVE2DSegment    FlirtingExtentPolySegment1Segment11;
    HVE2DSegment    FlirtingExtentPolySegment1Segment21;
    HVE2DPolySegment      FlirtingExtentPolySegment11;

    HVE2DSegment    FlirtingExtentLinkedPolySegment1Segment11;
    HVE2DSegment    FlirtingExtentLinkedPolySegment1Segment21;
    HVE2DPolySegment      FlirtingExtentLinkedPolySegment11;

    HVE2DSegment    ConnectedPolySegment1Segment11;
    HVE2DSegment    ConnectedPolySegment1Segment21;
    HVE2DPolySegment      ConnectedPolySegment11;

    HVE2DSegment    ConnectingPolySegment1Segment11;
    HVE2DSegment    ConnectingPolySegment1Segment21;
    HVE2DPolySegment      ConnectingPolySegment11;

    HVE2DSegment    ConnectedPolySegment1ASegment11;
    HVE2DSegment    ConnectedPolySegment1ASegment21;
    HVE2DPolySegment      ConnectedPolySegment1A1;

    HVE2DSegment    ConnectingPolySegment1ASegment11;
    HVE2DSegment    ConnectingPolySegment1ASegment21;
    HVE2DPolySegment      ConnectingPolySegment1A1;

    HVE2DSegment    LinkedPolySegment1Segment11;
    HVE2DSegment    LinkedPolySegment1Segment21;
    HVE2DPolySegment      LinkedPolySegment11;

    HVE2DSegment    LinkedPolySegment1ASegment11;
    HVE2DSegment    LinkedPolySegment1ASegment21;
    HVE2DPolySegment      LinkedPolySegment1A1;
    
    };