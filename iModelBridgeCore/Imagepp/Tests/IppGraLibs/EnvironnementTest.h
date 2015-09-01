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

    };