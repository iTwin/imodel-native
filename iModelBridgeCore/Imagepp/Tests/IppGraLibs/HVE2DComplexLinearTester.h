//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HVE2DComplexLinearTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DComplexLinearTester
//-----------------------------------------------------------------------------

#pragma once
 
// Preparation of required environement
class HVE2DComplexLinearTester : public EnvironnementTest 
    {   
protected:

    HVE2DComplexLinearTester();

    ~HVE2DComplexLinearTester() {};

    // Used for intersection tests
    HVE2DSegment    Case1Segment1;
    HVE2DComplexLinear      ComplexLinearCase1;

    HVE2DSegment    Case2Segment1;
    HVE2DSegment    Case2Segment2;
    HVE2DSegment    Case2Segment3;
    HVE2DComplexLinear      ComplexLinearCase2;

    HVE2DSegment    Case3Segment1;
    HVE2DComplexLinear      ComplexLinearCase3;

    HVE2DSegment    Case4Segment1;
    HVE2DComplexLinear      ComplexLinearCase4;

    HVE2DSegment    Case5Segment1;
    HVE2DSegment    Case5Segment2;
    HVE2DComplexLinear      ComplexLinearCase5;

    HVE2DSegment    Case5ASegment1;
    HVE2DSegment    Case5ASegment2;
    HVE2DComplexLinear      ComplexLinearCase5A;

    HVE2DSegment    Case6Segment1;
    HVE2DSegment    Case6Segment2;
    HVE2DComplexLinear      ComplexLinearCase6;

    HVE2DSegment    Case7Segment1;
    HVE2DComplexLinear      ComplexLinearCase7;

    // VERTICAL ComplexLinear
    HVE2DComplexLinear    VerticalComplexLinear1;
    HVE2DComplexLinear    VerticalComplexLinear2;
    HVE2DComplexLinear    VerticalComplexLinear3;
    HVE2DComplexLinear    VerticalComplexLinear4;
    HVE2DComplexLinear    VerticalComplexLinear5;
    HVE2DComplexLinear    CloseVerticalComplexLinear1;
    
    // HORIZONTAL ComplexLinear
    HVE2DComplexLinear    HorizontalComplexLinear1;
    HVE2DComplexLinear    HorizontalComplexLinear2;
    HVE2DComplexLinear    HorizontalComplexLinear3;
    HVE2DComplexLinear    HorizontalComplexLinear5;

    // LARGE ComplexLinearS
    HVE2DComplexLinear    LargeComplexLinear1;
    HVE2DComplexLinear    LargeComplexLinear2;
    HVE2DComplexLinear    ParallelLargeComplexLinear1;
   
    // POSITIVE ComplexLinearS
    HVE2DComplexLinear    PositiveComplexLinear1;
    HVE2DComplexLinear    PositiveComplexLinear2;
    HVE2DComplexLinear    ParallelPositiveComplexLinear1;
   
    // NEGATIVE ComplexLinearS
    HVE2DComplexLinear    NegativeComplexLinear1;
    HVE2DComplexLinear    NegativeComplexLinear2;
    HVE2DComplexLinear    ParallelNegativeComplexLinear1;
   
    // NULL ComplexLinearS
    HVE2DComplexLinear    NullComplexLinear1;
    HVE2DComplexLinear    NullComplexLinear2;

        // MISC ComplexLinearS
    HVE2DComplexLinear    MiscComplexLinear1;
    HVE2DComplexLinear    MiscComplexLinear2;
    HVE2DComplexLinear    MiscComplexLinear3;
    HVE2DComplexLinear    MiscComplexLinear4;
    HVE2DComplexLinear    MiscComplexLinear6;
    HVE2DComplexLinear    MiscComplexLinear7;
    HVE2DComplexLinear    DisjointComplexLinear1;
    HVE2DComplexLinear    ContiguousExtentComplexLinear1;
    HVE2DComplexLinear    FlirtingExtentComplexLinear1;
    HVE2DComplexLinear    FlirtingExtentLinkedComplexLinear1;
    HVE2DComplexLinear    ParallelComplexLinear1;
    HVE2DComplexLinear    LinkedParallelComplexLinear1;
    HVE2DComplexLinear    NearParallelComplexLinear1;
    HVE2DComplexLinear    CloseNearParallelComplexLinear1;
    HVE2DComplexLinear    ConnectedComplexLinear1;
    HVE2DComplexLinear    ConnectingComplexLinear1;
    HVE2DComplexLinear    ConnectedComplexLinear1A;
    HVE2DComplexLinear    ConnectingComplexLinear1A;
    HVE2DComplexLinear    LinkedComplexLinear1;
    HVE2DComplexLinear    LinkedComplexLinear1A;
    HVE2DComplexLinear    MiscComplexLinear3A;

    // ComplexLinears
    HVE2DComplexLinear EmptyComplexLinear;
    HVE2DComplexLinear ComplexLinear1;
   
    HGF2DLocation ComplexLinear1Point0d0;
    HGF2DLocation ComplexLinear1Point0d1;
    HGF2DLocation ComplexLinear1Point0d5;
    HGF2DLocation ComplexLinear1Point1d0;
    HGF2DLocation ComplexLinearMidPoint1;

    HGF2DLocation ComplexLinearClosePoint1A;
    HGF2DLocation ComplexLinearClosePoint1B;
    HGF2DLocation ComplexLinearClosePoint1C;
    HGF2DLocation ComplexLinearClosePoint1D;
    HGF2DLocation ComplexLinearCloseMidPoint1;

    HVE2DComplexLinear ComplexLinear2;  // AutoClosed
   
    HGF2DLocation ComplexLinear2Point0d0;
    HGF2DLocation ComplexLinear2Point0d1;
    HGF2DLocation ComplexLinear2Point0d5;
    HGF2DLocation ComplexLinear2Point1d0;

    HGF2DLocation ComplexLinearMidPoint2;

    HGF2DLocation ComplexLinearClosePoint2A;
    HGF2DLocation ComplexLinearClosePoint2B;
    HGF2DLocation ComplexLinearClosePoint2C;
    HGF2DLocation ComplexLinearClosePoint2D;
    HGF2DLocation ComplexLinearCloseMidPoint2;

    // epsilon size container
    HVE2DComplexLinear ComplexLinear3;  // Open
    
    HGF2DLocation ComplexLinear3Point0d0;
    HGF2DLocation ComplexLinear3Point0d1;
    HGF2DLocation ComplexLinear3Point0d5;
    HGF2DLocation ComplexLinear3Point1d0;

    HGF2DLocation ComplexLinearMidPoint3;

    HGF2DLocation ComplexLinearClosePoint3A;
    HGF2DLocation ComplexLinearClosePoint3B;
    HGF2DLocation ComplexLinearClosePoint3C;
    HGF2DLocation ComplexLinearClosePoint3D;
    HGF2DLocation ComplexLinearCloseMidPoint3;

    // Used for intersection tests
    HVE2DComplexLinear ComplexComplexLinearCase1; 
    HVE2DComplexLinear ComplexComplexLinearCase2;
    HVE2DComplexLinear ComplexComplexLinearCase3; 
    HVE2DComplexLinear ComplexComplexLinearCase4; 
    HVE2DComplexLinear ComplexComplexLinearCase5;
    HVE2DComplexLinear ComplexComplexLinearCase5A; 
    HVE2DComplexLinear ComplexComplexLinearCase6; 
    HVE2DComplexLinear ComplexComplexLinearCase7; 

    //ContiguousTest
    HVE2DSegment ContiguousLinear2Segment1;
    HVE2DComplexLinear ContiguousLinear2;  // AutoClosed
    HGF2DLocation Linear2ContiguousPoint;

    HVE2DSegment ContiguousLinear3Segment1;
    HVE2DComplexLinear ContiguousLinear3; // Open
    HGF2DLocation Linear3ContiguousPoint;

    HVE2DComplexLinear ContiguousComplexLinear2;  // AutoClosed
    HGF2DLocation ComplexLinear2ContiguousPoint;

    HVE2DComplexLinear ContiguousComplexLinear3; 
    HGF2DLocation ComplexLinear3ContiguousPoint;
           
    };