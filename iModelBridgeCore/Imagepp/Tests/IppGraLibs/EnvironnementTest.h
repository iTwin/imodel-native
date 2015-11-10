//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/EnvironnementTest.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : EnvironnementTest
//-----------------------------------------------------------------------------

#pragma once 

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())

// Preparation of required environement
class EnvironnementTest : public testing::Test 
    {

protected:

    EnvironnementTest();

    // TRANSFORMATION MODELS
    HGF2DDisplacement   Translation;
    double              Rotation;
    double              ScalingX;
    double              ScalingY;
    double              Anortho;
    HGF2DAffine Affine1;   
    HGF2DProjectiveGrid Projective;

    // COORDINATE SYSTEMS
    HFCPtr<HGF2DCoordSys>   pWorld;
    HFCPtr<HGF2DCoordSys>   pSys1;
    HFCPtr<HGF2DCoordSys>   pNoLinear;

    // VERTICAL SEGMENT
    HVE2DSegment    VerticalSegment1;
    HVE2DSegment    VerticalSegment2;
    HVE2DSegment    VerticalSegment3;
    HVE2DSegment    VerticalSegment4;
    HVE2DSegment    VerticalSegment5;
    HVE2DSegment    CloseVerticalSegment1;

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

    // HORIZONTAL SEGMENT
    HVE2DSegment    HorizontalSegment1;
    HVE2DSegment    HorizontalSegment2;
    HVE2DSegment    HorizontalSegment3;
    HVE2DSegment    HorizontalSegment5;

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

    // MISC SEGMENTS
    HVE2DSegment    MiscSegment1;
    HVE2DSegment    MiscSegment2;
    HVE2DSegment    MiscSegment3;
                                
    HVE2DSegment    MiscSegment4;
    HVE2DSegment    MiscSegment6;
    HVE2DSegment    MiscSegment7;
    HVE2DSegment    DisjointSegment1;
    HVE2DSegment    ContiguousExtentSegment1;
    HVE2DSegment    FlirtingExtentSegment1;
    HVE2DSegment    FlirtingExtentLinkedSegment1;
    HVE2DSegment    ParallelSegment1;
    HVE2DSegment    LinkedParallelSegment1;
    HVE2DSegment    NearParallelSegment1;
    HVE2DSegment    CloseNearParallelSegment1;
    HVE2DSegment    ConnectedSegment1;
    HVE2DSegment    ConnectingSegment1;
    HVE2DSegment    ConnectedSegment1A;
    HVE2DSegment    ConnectingSegment1A;
    HVE2DSegment    LinkedSegment1;
    HVE2DSegment    LinkedSegment1A;

    HVE2DSegment    MiscSegment3A;

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

    HGF2DLocation   VeryFarAlignedPoint;
    HGF2DLocation   VeryFarAlignedNegativePoint;
    HGF2DLocation   VeryFarNegativePoint;
    HGF2DLocation   MidPoint;

    HGF2DLocation   MiscMidPoint6;

    // LARGE SEGMENTS
    HVE2DSegment    LargeSegment1;
    HVE2DSegment    LargeSegment2;
    HVE2DSegment    ParallelLargeSegment1;

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

    // POSITIVE SEGMENTS
    HVE2DSegment    PositiveSegment1;
    HVE2DSegment    PositiveSegment2;
    HVE2DSegment    ParallelPositiveSegment1;

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

    // NEGATIVE SEGMENTS
    HVE2DSegment    NegativeSegment1;
    HVE2DSegment    NegativeSegment2;
    HVE2DSegment    ParallelNegativeSegment1;

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

    // NULL SEGMENTS
    HVE2DSegment    NullSegment1;
    HVE2DSegment    NullSegment2;

    // Special SEGMENTS
    HVE2DSegment    SegmentInvalid; 
    HVE2DSegment    SegmentPSys1;

    // Creation of an identity model
    HGF2DIdentity MyIdentity;

    // Complex Linears
    HVE2DComplexLinear      EmptyLinear;

    HVE2DSegment    Linear1Segment1;
    HVE2DSegment    Linear1Segment2;
    HVE2DSegment    Linear1Segment3;
    HVE2DSegment    Linear1Segment4;
    HVE2DSegment    Linear1Segment5;
    HVE2DSegment    Linear1Segment6;
    HVE2DComplexLinear      Linear1; // Open

    HGF2DLocation   Linear1Point0d0;
    HGF2DLocation   Linear1Point0d1;
    HGF2DLocation   Linear1Point0d5;
    HGF2DLocation   Linear1Point1d0;
    HGF2DLocation   LinearMidPoint1;

    HGF2DLocation   LinearClosePoint1A;
    HGF2DLocation   LinearClosePoint1B;
    HGF2DLocation   LinearClosePoint1C;
    HGF2DLocation   LinearClosePoint1D;
    HGF2DLocation   LinearCloseMidPoint1;

    HVE2DSegment    Linear2Segment1;
    HVE2DSegment    Linear2Segment2;
    HVE2DSegment    Linear2Segment3;
    HVE2DSegment    Linear2Segment4;
    HVE2DSegment    Linear2Segment5;
    HVE2DSegment    Linear2Segment6;
    HVE2DComplexLinear      Linear2;  // AutoClosed

    HGF2DLocation   Linear2Point0d0;
    HGF2DLocation   Linear2Point0d1;
    HGF2DLocation   Linear2Point0d5;
    HGF2DLocation   Linear2Point1d0;

    HGF2DLocation   LinearMidPoint2;

    HGF2DLocation   LinearClosePoint2A;
    HGF2DLocation   LinearClosePoint2B;
    HGF2DLocation   LinearClosePoint2C;
    HGF2DLocation   LinearClosePoint2D;
    HGF2DLocation   LinearCloseMidPoint2;

     // epsilon size container
    HVE2DSegment    Linear3Segment1;
    HVE2DSegment    Linear3Segment2;
    HVE2DSegment    Linear3Segment3;
    HVE2DSegment    Linear3Segment4;
    HVE2DSegment    Linear3Segment5;
    HVE2DComplexLinear      Linear3; // Open

    HGF2DLocation   Linear3Point0d0;
    HGF2DLocation   Linear3Point0d1;
    HGF2DLocation   Linear3Point0d5;
    HGF2DLocation   Linear3Point1d0;
    HGF2DLocation   LinearMidPoint3;

    HGF2DLocation   LinearClosePoint3A;
    HGF2DLocation   LinearClosePoint3B;
    HGF2DLocation   LinearClosePoint3C;
    HGF2DLocation   LinearClosePoint3D;
    HGF2DLocation   LinearCloseMidPoint3;

    // Used for intersection tests
    HVE2DSegment        Case1Segment1;
    HVE2DComplexLinear  ComplexLinearCase1;

    HVE2DSegment        Case2Segment1;
    HVE2DSegment        Case2Segment2;
    HVE2DSegment        Case2Segment3;
    HVE2DComplexLinear  ComplexLinearCase2;

    HVE2DSegment        Case3Segment1;
    HVE2DComplexLinear  ComplexLinearCase3;

    HVE2DSegment        Case4Segment1;
    HVE2DComplexLinear  ComplexLinearCase4;

    HVE2DSegment        Case5Segment1;
    HVE2DSegment        Case5Segment2;
    HVE2DComplexLinear  ComplexLinearCase5;

    HVE2DSegment        Case5ASegment1;
    HVE2DSegment        Case5ASegment2;
    HVE2DComplexLinear  ComplexLinearCase5A;

    HVE2DSegment        Case6Segment1;
    HVE2DSegment        Case6Segment2;
    HVE2DSegment        Case6Segment3;
    HVE2DComplexLinear  ComplexLinearCase6;

    HVE2DSegment        Case7Segment1;
    HVE2DComplexLinear  ComplexLinearCase7;

    HVE2DSegment        AutoCrossingLinear1Segment1;
    HVE2DSegment        AutoCrossingLinear1Segment2;
    HVE2DSegment        AutoCrossingLinear1Segment3;
    HVE2DSegment        AutoCrossingLinear1Segment4;
    HVE2DSegment        AutoCrossingLinear1Segment5;
    HVE2DComplexLinear  AutoCrossingLinear1;

    HVE2DSegment        AutoCrossingLinear2Segment1;
    HVE2DSegment        AutoCrossingLinear2Segment2;
    HVE2DSegment        AutoCrossingLinear2Segment3;
    HVE2DSegment        AutoCrossingLinear2Segment4;
    HVE2DComplexLinear  AutoCrossingLinear2;

    HVE2DSegment        AutoConnectingLinear1Segment1;
    HVE2DSegment        AutoConnectingLinear1Segment2;
    HVE2DSegment        AutoConnectingLinear1Segment3;
    HVE2DSegment        AutoConnectingLinear1Segment4;
    HVE2DComplexLinear  AutoConnectingLinear1;

    HVE2DSegment        DisjointLinear1Segment1;
    HVE2DSegment        DisjointLinear1Segment2;
    HVE2DComplexLinear  DisjointLinear1; // Open

    HVE2DSegment        ContiguousExtentLinear1Segment1;
    HVE2DSegment        ContiguousExtentLinear1Segment2;
    HVE2DComplexLinear  ContiguousExtentLinear1; // Open

    HVE2DSegment        FlirtingExtentLinear1Segment1; 
    HVE2DSegment        FlirtingExtentLinear1Segment2; 
    HVE2DComplexLinear  FlirtingExtentLinear1; // Open

    HVE2DSegment        FlirtingExtentLinkedLinear1Segment1;
    HVE2DSegment        FlirtingExtentLinkedLinear1Segment2;
    HVE2DComplexLinear  FlirtingExtentLinkedLinear1; // Open

    HVE2DSegment        ConnectedLinear1Segment1;
    HVE2DSegment        ConnectedLinear1Segment2;
    HVE2DComplexLinear  ConnectedLinear1; // Open
   
    HVE2DSegment        ConnectingLinear1Segment1;
    HVE2DSegment        ConnectingLinear1Segment2;
    HVE2DComplexLinear  ConnectingLinear1;

    HVE2DSegment        ConnectedLinear1ASegment1;
    HVE2DSegment        ConnectedLinear1ASegment2; 
    HVE2DComplexLinear  ConnectedLinear1A; // Open
   
    HVE2DSegment        ConnectingLinear1ASegment1;
    HVE2DSegment        ConnectingLinear1ASegment2;
    HVE2DComplexLinear  ConnectingLinear1A;
   
    HVE2DSegment        LinkedLinear1Segment1;
    HVE2DSegment        LinkedLinear1Segment2;
    HVE2DComplexLinear  LinkedLinear1;

    HVE2DSegment        LinkedLinear1ASegment1;
    HVE2DSegment        LinkedLinear1ASegment2;
    HVE2DComplexLinear  LinkedLinear1A;

    // Shapes
    HVE2DRectangle  Rect1;

    HVE2DRectangle  NorthContiguousRect;
    HVE2DRectangle  EastContiguousRect;
    HVE2DRectangle  WestContiguousRect;
    HVE2DRectangle  SouthContiguousRect;

    HVE2DRectangle  NETipRect;
    HVE2DRectangle  NWTipRect;
    HVE2DRectangle  SETipRect;
    HVE2DRectangle  SWTipRect;

    HVE2DRectangle  VerticalFitRect;
    HVE2DRectangle  HorizontalFitRect;

    HVE2DRectangle  DisjointRect;
    HVE2DRectangle  NegativeRect;

    HVE2DRectangle  MiscRect1;

    HVE2DRectangle  EnglobRect1;
    HVE2DRectangle  EnglobRect2;
    HVE2DRectangle  EnglobRect3;

    HVE2DRectangle  IncludedRect1;
    HVE2DRectangle  IncludedRect2;
    HVE2DRectangle  IncludedRect3;
    HVE2DRectangle  IncludedRect4;
    HVE2DRectangle  IncludedRect5;
    HVE2DRectangle  IncludedRect6;
    HVE2DRectangle  IncludedRect7;
    HVE2DRectangle  IncludedRect8;
    HVE2DRectangle  IncludedRect9;

    HVE2DRectangle  RectPSys1;

    HVE2DSegment    SWCuttingSegment;
    HVE2DSegment    NECuttingSegment;
    HVE2DSegment    NWCuttingSegment;
    HVE2DSegment    SECuttingSegment;

    HVE2DSegment    NorthContiguousSegment;
    HVE2DSegment    SouthContiguousSegment;
    HVE2DSegment    EastContiguousSegment;
    HVE2DSegment    WestContiguousSegment;

    HGF2DLocation   RectClosePoint1A;
    HGF2DLocation   RectClosePoint1B;
    HGF2DLocation   RectClosePoint1C;
    HGF2DLocation   RectClosePoint1D;
    HGF2DLocation   RectCloseMidPoint1;

    HGF2DLocation   VeryFarPoint;

    HGF2DLocation   Rect1Point0d0;
    HGF2DLocation   Rect1Point0d1;
    HGF2DLocation   Rect1Point0d5;
    HGF2DLocation   Rect1Point1d0;

    HGF2DLocation   RectMidPoint1;
    
    // Lite for HGF version
        // VERTICAL SEGMENT
    HGF2DSegment    VerticalSegment1A;
    HGF2DSegment    VerticalSegment2A;
    HGF2DSegment    VerticalSegment3A;
    HGF2DSegment    VerticalSegment4A;
    HGF2DSegment    VerticalSegment5A;
    HGF2DSegment    CloseVerticalSegment1A;

    HGF2DPosition   VerticalPoint0d0A;
    HGF2DPosition   VerticalPoint0d1A;
    HGF2DPosition   VerticalPoint0d5A;
    HGF2DPosition   VerticalPoint1d0A;

    HGF2DPosition   Vertical3Point0d0A;
    HGF2DPosition   Vertical3Point0d1A;
    HGF2DPosition   Vertical3Point0d5A;
    HGF2DPosition   Vertical3Point1d0A;

    HGF2DPosition   VerticalMidPoint1A;
    HGF2DPosition   VerticalMidPoint3A;

    HGF2DPosition   VerticalClosePoint1AA;
    HGF2DPosition   VerticalClosePoint1BA;
    HGF2DPosition   VerticalClosePoint1CA;
    HGF2DPosition   VerticalClosePoint1DA;
    HGF2DPosition   VerticalCloseMidPoint1A;

    HGF2DPosition   VerticalClosePoint3AA;
    HGF2DPosition   VerticalClosePoint3BA;
    HGF2DPosition   VerticalClosePoint3CA;
    HGF2DPosition   VerticalClosePoint3DA;
    HGF2DPosition   VerticalCloseMidPoint3A;

    // HORIZONTAL SEGMENT
    HGF2DSegment    HorizontalSegment1A;
    HGF2DSegment    HorizontalSegment2A;
    HGF2DSegment    HorizontalSegment3A;
    HGF2DSegment    HorizontalSegment5A;

    HGF2DPosition   HorizontalPoint0d0A;
    HGF2DPosition   HorizontalPoint0d1A;
    HGF2DPosition   HorizontalPoint0d5A;
    HGF2DPosition   HorizontalPoint1d0A;

    HGF2DPosition   Horizontal3Point0d0A;
    HGF2DPosition   Horizontal3Point0d1A;
    HGF2DPosition   Horizontal3Point0d5A;
    HGF2DPosition   Horizontal3Point1d0A;

    HGF2DPosition   HorizontalMidPoint1A;
    HGF2DPosition   HorizontalMidPoint3A;

    HGF2DPosition   HorizontalClosePoint1AA;
    HGF2DPosition   HorizontalClosePoint1BA;
    HGF2DPosition   HorizontalClosePoint1CA;
    HGF2DPosition   HorizontalClosePoint1DA;
    HGF2DPosition   HorizontalCloseMidPoint1A;

    HGF2DPosition   HorizontalClosePoint3AA;
    HGF2DPosition   HorizontalClosePoint3BA;
    HGF2DPosition   HorizontalClosePoint3CA;
    HGF2DPosition   HorizontalClosePoint3DA;
    HGF2DPosition   HorizontalCloseMidPoint3A;

    // MISC SEGMENTS
    HGF2DSegment    MiscSegment1A;
    HGF2DSegment    MiscSegment2A;
    HGF2DSegment    MiscSegment3B;
                                
    HGF2DSegment    MiscSegment4A;
    HGF2DSegment    MiscSegment6A;
    HGF2DSegment    MiscSegment7A;
    HGF2DSegment    DisjointSegment1A;
    HGF2DSegment    ContiguousExtentSegment1A;
    HGF2DSegment    FlirtingExtentSegment1A;
    HGF2DSegment    FlirtingExtentLinkedSegment1A;
    HGF2DSegment    ParallelSegment1A;
    HGF2DSegment    LinkedParallelSegment1A;
    HGF2DSegment    NearParallelSegment1A;
    HGF2DSegment    CloseNearParallelSegment1A;
    HGF2DSegment    ConnectedSegment1B;
    HGF2DSegment    ConnectingSegment1B;
    HGF2DSegment    ConnectedSegment1AA;
    HGF2DSegment    ConnectingSegment1AA;
    HGF2DSegment    LinkedSegment1B;
    HGF2DSegment    LinkedSegment1AA;

    HGF2DSegment    MiscSegment3AA;

    HGF2DPosition   Misc3Point0d0A;
    HGF2DPosition   Misc3Point0d1A;
    HGF2DPosition   Misc3Point0d5A;
    HGF2DPosition   Misc3Point1d0A;
    HGF2DPosition   MiscMidPoint1A;

    HGF2DPosition   MiscMidPoint3A;
    HGF2DPosition   MiscClosePoint3AA;
    HGF2DPosition   MiscClosePoint3BA;
    HGF2DPosition   MiscClosePoint3CA;
    HGF2DPosition   MiscClosePoint3DA;
    HGF2DPosition   MiscCloseMidPoint3A;

    HGF2DPosition   VeryFarAlignedPointA;
    HGF2DPosition   VeryFarAlignedNegativePointA;
    HGF2DPosition   VeryFarNegativePointA;
    HGF2DPosition   MidPointA;

    HGF2DPosition   MiscMidPoint6A;

    // LARGE SEGMENTS
    HGF2DSegment    LargeSegment1A;
    HGF2DSegment    LargeSegment2A;
    HGF2DSegment    ParallelLargeSegment1A;

    HGF2DPosition   LargePoint0d0A;
    HGF2DPosition   LargePoint0d1A;
    HGF2DPosition   LargePoint0d5A;
    HGF2DPosition   LargePoint1d0A;

    HGF2DPosition   LargeMidPoint1A;

    HGF2DPosition   LargeClosePoint1AA;
    HGF2DPosition   LargeClosePoint1BA;
    HGF2DPosition   LargeClosePoint1CA;
    HGF2DPosition   LargeClosePoint1DA;
    HGF2DPosition   LargeCloseMidPoint1A;

    // POSITIVE SEGMENTS
    HGF2DSegment    PositiveSegment1A;
    HGF2DSegment    PositiveSegment2A;
    HGF2DSegment    ParallelPositiveSegment1A;

    HGF2DPosition   PositivePoint0d0A;
    HGF2DPosition   PositivePoint0d1A;
    HGF2DPosition   PositivePoint0d5A;
    HGF2DPosition   PositivePoint1d0A;

    HGF2DPosition   PositiveMidPoint1A;

    HGF2DPosition   PositiveClosePoint1AA;
    HGF2DPosition   PositiveClosePoint1BA;
    HGF2DPosition   PositiveClosePoint1CA;
    HGF2DPosition   PositiveClosePoint1DA;
    HGF2DPosition   PositiveCloseMidPoint1A;

    // NEGATIVE SEGMENTS
    HGF2DSegment    NegativeSegment1A;
    HGF2DSegment    NegativeSegment2A;
    HGF2DSegment    ParallelNegativeSegment1A;

    HGF2DPosition   NegativePoint0d0A;
    HGF2DPosition   NegativePoint0d1A;
    HGF2DPosition   NegativePoint0d5A;
    HGF2DPosition   NegativePoint1d0A;

    HGF2DPosition   NegativeMidPoint1A;

    HGF2DPosition   NegativeClosePoint1AA;
    HGF2DPosition   NegativeClosePoint1BA;
    HGF2DPosition   NegativeClosePoint1CA;
    HGF2DPosition   NegativeClosePoint1DA;
    HGF2DPosition   NegativeCloseMidPoint1A;

    // NULL SEGMENTS
    HGF2DSegment    NullSegment1A;
    HGF2DSegment    NullSegment2A;

    // Special SEGMENTS
    HGF2DSegment    SegmentInvalidA; 
    HGF2DSegment    SegmentPSys1A;


    // Complex Linears
     HGF2DPolySegment      EmptyLinearA;

     HGF2DSegment    Linear1Segment1A;
     HGF2DSegment    Linear1Segment2A;
     HGF2DSegment    Linear1Segment3A;
     HGF2DSegment    Linear1Segment4A;
     HGF2DSegment    Linear1Segment5A;
     HGF2DSegment    Linear1Segment6A;
     HGF2DPolySegment      Linear1A; // Open

     HGF2DPosition   Linear1Point0d0A;
     HGF2DPosition   Linear1Point0d1A;
     HGF2DPosition   Linear1Point0d5A;
     HGF2DPosition   Linear1Point1d0A;
     HGF2DPosition   LinearMidPoint1A;

     HGF2DPosition   LinearClosePoint1AA;
     HGF2DPosition   LinearClosePoint1BA;
     HGF2DPosition   LinearClosePoint1CA;
     HGF2DPosition   LinearClosePoint1DA;
     HGF2DPosition   LinearCloseMidPoint1A;

     HGF2DSegment    Linear2Segment1A;
     HGF2DSegment    Linear2Segment2A;
     HGF2DSegment    Linear2Segment3A;
     HGF2DSegment    Linear2Segment4A;
     HGF2DSegment    Linear2Segment5A;
     HGF2DSegment    Linear2Segment6A;
     HGF2DPolySegment      Linear2A;  // AutoClosed

     HGF2DPosition   Linear2Point0d0A;
     HGF2DPosition   Linear2Point0d1A;
     HGF2DPosition   Linear2Point0d5A;
     HGF2DPosition   Linear2Point1d0A;

     HGF2DPosition   LinearMidPoint2A;

     HGF2DPosition   LinearClosePoint2AA;
     HGF2DPosition   LinearClosePoint2BA;
     HGF2DPosition   LinearClosePoint2CA;
     HGF2DPosition   LinearClosePoint2DA;
     HGF2DPosition   LinearCloseMidPoint2A;

      // epsilon size container
     HGF2DSegment    Linear3Segment1A;
     HGF2DSegment    Linear3Segment2A;
     HGF2DSegment    Linear3Segment3A;
     HGF2DSegment    Linear3Segment4A;
     HGF2DSegment    Linear3Segment5A;
     HGF2DPolySegment      Linear3A; // Open

     HGF2DPosition   Linear3Point0d0A;
     HGF2DPosition   Linear3Point0d1A;
     HGF2DPosition   Linear3Point0d5A;
     HGF2DPosition   Linear3Point1d0A;
     HGF2DPosition   LinearMidPoint3A;

     HGF2DPosition   LinearClosePoint3AA;
     HGF2DPosition   LinearClosePoint3BA;
     HGF2DPosition   LinearClosePoint3CA;
     HGF2DPosition   LinearClosePoint3DA;
     HGF2DPosition   LinearCloseMidPoint3A;

    // Used for intersection tests
    HGF2DPolySegment  ComplexLinearCase1A;

    HGF2DPolySegment  ComplexLinearCase2A;

    HGF2DPolySegment  ComplexLinearCase3A;

    HGF2DPolySegment  ComplexLinearCase4A;

    HGF2DPolySegment  ComplexLinearCase5B;

    HGF2DPolySegment  ComplexLinearCase5AA;

    HGF2DPolySegment  ComplexLinearCase6A;

    HGF2DPolySegment  ComplexLinearCase7A;

    HGF2DPolySegment  AutoCrossingLinear1A;

    HGF2DPolySegment  AutoCrossingLinear2A;

    HGF2DPolySegment  AutoConnectingLinear1A;

    HGF2DPolySegment  DisjointLinear1A; // Open

    HGF2DPolySegment  ContiguousExtentLinear1A; // Open

    HGF2DPolySegment  FlirtingExtentLinear1A; // Open

    HGF2DPolySegment  FlirtingExtentLinkedLinear1A; // Open

    HGF2DPolySegment  ConnectedLinear1B; // Open
   
    HGF2DPolySegment  ConnectingLinear1B;

    HGF2DPolySegment  ConnectedLinear1AA; // Open
   
    HGF2DPolySegment  ConnectingLinear1AA;
   
    HGF2DPolySegment  LinkedLinear1B;

    HGF2DPolySegment  LinkedLinear1AA;

    // Shapes
    HGF2DRectangle  Rect1A;

    HGF2DRectangle  NorthContiguousRectA;
    HGF2DRectangle  EastContiguousRectA;
    HGF2DRectangle  WestContiguousRectA;
    HGF2DRectangle  SouthContiguousRectA;

    HGF2DRectangle  NETipRectA;
    HGF2DRectangle  NWTipRectA;
    HGF2DRectangle  SETipRectA;
    HGF2DRectangle  SWTipRectA;

    HGF2DRectangle  VerticalFitRectA;
    HGF2DRectangle  HorizontalFitRectA;

    HGF2DRectangle  DisjointRectA;
    HGF2DRectangle  NegativeRectA;

    HGF2DRectangle  MiscRect1A;

    HGF2DRectangle  EnglobRect1A;
    HGF2DRectangle  EnglobRect2A;
    HGF2DRectangle  EnglobRect3A;

    HGF2DRectangle  IncludedRect1A;
    HGF2DRectangle  IncludedRect2A;
    HGF2DRectangle  IncludedRect3A;
    HGF2DRectangle  IncludedRect4A;
    HGF2DRectangle  IncludedRect5A;
    HGF2DRectangle  IncludedRect6A;
    HGF2DRectangle  IncludedRect7A;
    HGF2DRectangle  IncludedRect8A;
    HGF2DRectangle  IncludedRect9A;

    HGF2DRectangle  RectPSys1A;

    HGF2DSegment    SWCuttingSegmentA;
    HGF2DSegment    NECuttingSegmentA;
    HGF2DSegment    NWCuttingSegmentA;
    HGF2DSegment    SECuttingSegmentA;

    HGF2DSegment    NorthContiguousSegmentA;
    HGF2DSegment    SouthContiguousSegmentA;
    HGF2DSegment    EastContiguousSegmentA;
    HGF2DSegment    WestContiguousSegmentA;

    HGF2DPosition   RectClosePoint1AA;
    HGF2DPosition   RectClosePoint1BA;
    HGF2DPosition   RectClosePoint1CA;
    HGF2DPosition   RectClosePoint1DA;
    HGF2DPosition   RectCloseMidPoint1A;

    HGF2DPosition   VeryFarPointA;

    HGF2DPosition   Rect1Point0d0A;
    HGF2DPosition   Rect1Point0d1A;
    HGF2DPosition   Rect1Point0d5A;
    HGF2DPosition   Rect1Point1d0A;

    HGF2DPosition   RectMidPoint1A;

    };