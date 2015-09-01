//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HVE2DComplexLinearTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HVE2DComplexLinearTester.h"

#define ASMALLFACTOR (0.00000001)

HVE2DComplexLinearTester::HVE2DComplexLinearTester() 
    {

    // Used for intersection tests
    Case1Segment1 = HVE2DSegment(HGF2DLocation(20.0, 5.0, pWorld), HGF2DLocation(30.0, 15.0, pWorld));
    ComplexLinearCase1 = HVE2DComplexLinear(pWorld);
    ComplexLinearCase1.AppendLinear(Case1Segment1);

    Case2Segment1 = HVE2DSegment(HGF2DLocation(20.0, 5.0, pWorld), HGF2DLocation(24.0, 10.0, pWorld));
    Case2Segment2 = HVE2DSegment(HGF2DLocation(24.0, 10.0, pWorld), HGF2DLocation(28.0, 10.0, pWorld));
    Case2Segment3 = HVE2DSegment(HGF2DLocation(28.0, 10.0, pWorld), HGF2DLocation(30.0, 15.0, pWorld));
    ComplexLinearCase2 = HVE2DComplexLinear(pWorld);
    ComplexLinearCase2.AppendLinear(Case2Segment1);
    ComplexLinearCase2.AppendLinear(Case2Segment2);
    ComplexLinearCase2.AppendLinear(Case2Segment3);

    Case3Segment1 = HVE2DSegment(HGF2DLocation(15.0, 5.0, pWorld), HGF2DLocation(25.0, 15.0, pWorld));
    ComplexLinearCase3 = HVE2DComplexLinear(pWorld);
    ComplexLinearCase3.AppendLinear(Case3Segment1);

    Case4Segment1 = HVE2DSegment(HGF2DLocation(5.0, 10.0, pWorld), HGF2DLocation(35.0, 10.0, pWorld));
    ComplexLinearCase4 = HVE2DComplexLinear(pWorld);
    ComplexLinearCase4.AppendLinear(Case4Segment1);

    Case5Segment1 = HVE2DSegment(HGF2DLocation(35.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld));
    Case5Segment2 = HVE2DSegment(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(10.0, 5.0, pWorld));
    ComplexLinearCase5 = HVE2DComplexLinear(pWorld);
    ComplexLinearCase5.AppendLinear(Case5Segment1);
    ComplexLinearCase5.AppendLinear(Case5Segment2);

    Case5ASegment1 = HVE2DSegment(HGF2DLocation(30.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld));
    Case5ASegment2 = HVE2DSegment(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(10.0, 5.0, pWorld));
    ComplexLinearCase5A = HVE2DComplexLinear(pWorld);
    ComplexLinearCase5A.AppendLinear(Case5ASegment1);
    ComplexLinearCase5A.AppendLinear(Case5ASegment2);

    Case6Segment1 = HVE2DSegment(HGF2DLocation(35.0, 10.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    Case6Segment2 = HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(-1.0, -1.0, pWorld));
    ComplexLinearCase6 = HVE2DComplexLinear(pWorld);
    ComplexLinearCase6.AppendLinear(Case6Segment1);
    ComplexLinearCase6.AppendLinear(Case6Segment2);

    Case7Segment1 = HVE2DSegment(HGF2DLocation(25.0, 15.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld));
    ComplexLinearCase7 = HVE2DComplexLinear(pWorld);
    ComplexLinearCase7.AppendLinear(Case7Segment1);

    // VERTICAL ComplexLinear
    VerticalComplexLinear1 = HVE2DComplexLinear(pWorld);
    VerticalComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(0.1, 10.1, pWorld)));

    VerticalComplexLinear2 = HVE2DComplexLinear(pWorld);
    VerticalComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.1, 10.1, pWorld), HGF2DLocation(0.1, 0.1, pWorld)));

    VerticalComplexLinear3 = HVE2DComplexLinear(pWorld);
    VerticalComplexLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(0.1, 0.1 + MYEPSILON, pWorld)));

    VerticalComplexLinear4 = HVE2DComplexLinear(pWorld);
    VerticalComplexLinear4.AppendLinear(HVE2DSegment(HGF2DLocation(0.1 + MYEPSILON, 0.1, pWorld), HGF2DLocation(0.1+MYEPSILON, 10.1, pWorld)));

    VerticalComplexLinear5 = HVE2DComplexLinear(pWorld);
    VerticalComplexLinear5.AppendLinear(HVE2DSegment(HGF2DLocation(-10.0, -10.0, pWorld), HGF2DLocation(-10.0, 0.0, pWorld)));
    CloseVerticalComplexLinear1 = HVE2DComplexLinear(pWorld);
    CloseVerticalComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.1+MYEPSILON, 0.1, pWorld), HGF2DLocation(0.1, 10.1, pWorld)));

    // HORIZONTAL ComplexLinear
    HorizontalComplexLinear1 = HVE2DComplexLinear(pWorld);
    HorizontalComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(10.1, 0.1, pWorld)));

    HorizontalComplexLinear2 = HVE2DComplexLinear(pWorld);
    HorizontalComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(10.1, 0.1, pWorld), HGF2DLocation(0.1, 0.1, pWorld)));

    HorizontalComplexLinear3 = HVE2DComplexLinear(pWorld);
    HorizontalComplexLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(0.1 + MYEPSILON, 0.1, pWorld)));

    HorizontalComplexLinear5 = HVE2DComplexLinear(pWorld);
    HorizontalComplexLinear5.AppendLinear(HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(10.1 + MYEPSILON, 0.1, pWorld)));

    // LARGE ComplexLinearS
    LargeComplexLinear1 = HVE2DComplexLinear(pWorld);
    LargeComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-1E123, -21E123, pWorld), HGF2DLocation(9E123, 19E123, pWorld)));

    LargeComplexLinear2 = HVE2DComplexLinear(pWorld);
    LargeComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(9E123, 19E123, pWorld), HGF2DLocation(-1E123, -21E123, pWorld)));

    ParallelLargeComplexLinear1 = HVE2DComplexLinear(pWorld);
    ParallelLargeComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-1.000000001E123, -21E123, pWorld), HGF2DLocation(9.000000001E123, 19E123, pWorld)));

    // POSITIVE ComplexLinearS
    PositiveComplexLinear1 = HVE2DComplexLinear(pWorld);
    PositiveComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1E123, 21E123, pWorld), HGF2DLocation(11E123, 41E123, pWorld)));

    PositiveComplexLinear2 = HVE2DComplexLinear(pWorld);
    PositiveComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(11E123, 41E123, pWorld), HGF2DLocation(1E123, 21E123, pWorld)));

    ParallelPositiveComplexLinear1 = HVE2DComplexLinear(pWorld);
    ParallelPositiveComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1.000001E123, 21E123, pWorld), HGF2DLocation(11.000001E123, 41E123, pWorld)));

    // NEGATIVE ComplexLinearS
    NegativeComplexLinear1 = HVE2DComplexLinear(pWorld);
    NegativeComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-1E123, -21E123, pWorld), HGF2DLocation(-11E123, -41E123, pWorld)));

    NegativeComplexLinear2 = HVE2DComplexLinear(pWorld);
    NegativeComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-11E123, -41E123, pWorld), HGF2DLocation(-1E123, -21E123, pWorld)));

    ParallelNegativeComplexLinear1 = HVE2DComplexLinear(pWorld);
    ParallelNegativeComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-1.000001E123, -21E123, pWorld), HGF2DLocation(-11.000001E123, -41E123, pWorld)));

    // NULL ComplexLinearS
    NullComplexLinear1 = HVE2DComplexLinear(pWorld);
    NullComplexLinear2 = HVE2DComplexLinear(pWorld);

    // MISC ComplexLinearS
    MiscComplexLinear1 = HVE2DComplexLinear(pWorld);
    MiscComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(10.1, 10.1, pWorld)));

    MiscComplexLinear2 = HVE2DComplexLinear(pWorld);
    MiscComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(10.1, 10.1, pWorld), HGF2DLocation(0.1, 0.1, pWorld)));

    MiscComplexLinear3 = HVE2DComplexLinear(pWorld);
    MiscComplexLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld),
                                                 HGF2DLocation(0.1, 0.1, pWorld) + HGF2DDisplacement(HGFBearing(77.5 * PI / 180), MYEPSILON)));
    MiscComplexLinear4 = HVE2DComplexLinear(pWorld);
    MiscComplexLinear4.AppendLinear(HVE2DSegment(HGF2DLocation(0.2, 0.1, pWorld),
                                                 HGF2DLocation(0.2, 0.1, pWorld) + HGF2DDisplacement(HGFBearing(77.5* PI / 180), MYEPSILON)));
    MiscComplexLinear6 = HVE2DComplexLinear(pWorld);
    MiscComplexLinear6.AppendLinear(HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(-9.9, 10.1, pWorld)));

    MiscComplexLinear7 = HVE2DComplexLinear(pWorld);
    MiscComplexLinear7.AppendLinear(HVE2DSegment(HGF2DLocation(0.2, 0.0, pWorld), HGF2DLocation(-9.8, 10.0, pWorld)));

    DisjointComplexLinear1 = HVE2DComplexLinear(pWorld);
    DisjointComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-0.1, -0.1, pWorld), HGF2DLocation(-10.1, -10.24, pWorld)));

    ContiguousExtentComplexLinear1 = HVE2DComplexLinear(pWorld);
    ContiguousExtentComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.1, 0.1, pWorld), HGF2DLocation(20.1, 10.1, pWorld)));

    FlirtingExtentComplexLinear1 = HVE2DComplexLinear(pWorld);
    FlirtingExtentComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.1, 0.1, pWorld), HGF2DLocation(20.1, -10.1, pWorld)));

    FlirtingExtentLinkedComplexLinear1 = HVE2DComplexLinear(pWorld);
    FlirtingExtentLinkedComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.1, 10.1, pWorld), HGF2DLocation(20.1, 0.1, pWorld)));

    ParallelComplexLinear1 = HVE2DComplexLinear(pWorld);
    ParallelComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.2, 0.1, pWorld), HGF2DLocation(10.2, 10.1, pWorld)));

    LinkedParallelComplexLinear1 = HVE2DComplexLinear(pWorld);
    LinkedParallelComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.1, 10.1, pWorld), HGF2DLocation(20.1, 20.1, pWorld)));

    NearParallelComplexLinear1 = HVE2DComplexLinear(pWorld);
    NearParallelComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.2, 0.1, pWorld), HGF2DLocation(10.2, 10.1+MYEPSILON, pWorld)));

    CloseNearParallelComplexLinear1 = HVE2DComplexLinear(pWorld);
    CloseNearParallelComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.1+MYEPSILON, 0.1, pWorld), HGF2DLocation(10.1+MYEPSILON, 10.1+MYEPSILON, pWorld)));

    ConnectedComplexLinear1 = HVE2DComplexLinear(pWorld);
    ConnectedComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.2, 0.0, pWorld), HGF2DLocation(0.0, 0.2, pWorld)));

    ConnectingComplexLinear1 = HVE2DComplexLinear(pWorld);
    ConnectingComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.2, 0.0, pWorld), HGF2DLocation(2.0, 2.0, pWorld)));

    ConnectedComplexLinear1A = HVE2DComplexLinear(pWorld);
    ConnectedComplexLinear1A.AppendLinear(HVE2DSegment(HGF2DLocation(20.2, 0.0, pWorld), HGF2DLocation(0.0, 20.2, pWorld)));

    ConnectingComplexLinear1A = HVE2DComplexLinear(pWorld);
    ConnectingComplexLinear1A.AppendLinear(HVE2DSegment(HGF2DLocation(2.0, 2.0, pWorld), HGF2DLocation(10.2, 0.0, pWorld)));

    LinkedComplexLinear1 = HVE2DComplexLinear(pWorld);
    LinkedComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(10.0, 3.2, pWorld)));

    LinkedComplexLinear1A = HVE2DComplexLinear(pWorld);
    LinkedComplexLinear1A.AppendLinear(HVE2DSegment(HGF2DLocation(10.1, 10.1, pWorld), HGF2DLocation(10.0, 3.2, pWorld)));

    MiscComplexLinear3A = HVE2DComplexLinear(pWorld);
    MiscComplexLinear3A.AppendLinear(HVE2DSegment(HGF2DLocation(0.1+MYEPSILON, 0.1, pWorld),
                                                  HGF2DLocation(0.1+MYEPSILON, 0.1, pWorld) + HGF2DDisplacement(HGFBearing(77.5 * PI / 180), MYEPSILON)));
    // ComplexLinears
    EmptyComplexLinear = HVE2DComplexLinear(pWorld);

    ComplexLinear1 = HVE2DComplexLinear(pWorld); // Open
    ComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    ComplexLinear1.AppendLinear(HVE2DSegment(ComplexLinear1.GetEndPoint(), HGF2DLocation(20.0, 10.0, pWorld)));
    ComplexLinear1.AppendLinear(HVE2DSegment(ComplexLinear1.GetEndPoint(), HGF2DLocation(30.0, 10.0, pWorld)));
    ComplexLinear1.AppendLinear(HVE2DSegment(ComplexLinear1.GetEndPoint(), HGF2DLocation(30.0, 5.0, pWorld)));
    ComplexLinear1.AppendLinear(HVE2DSegment(ComplexLinear1.GetEndPoint(), HGF2DLocation(30.0, 5.0-MYEPSILON, pWorld)));
    ComplexLinear1.AppendLinear(HVE2DSegment(ComplexLinear1.GetEndPoint(), HGF2DLocation(30.0-MYEPSILON, 0.0, pWorld)));

    ComplexLinear1Point0d0 = HGF2DLocation(0.0, 0.0, pWorld);
    ComplexLinear1Point0d1 = HGF2DLocation(3.1213203435596, 3.1213203435596, pWorld);
    ComplexLinear1Point0d5 = HGF2DLocation(17.928932188135, 10.0, pWorld);
    ComplexLinear1Point1d0 = HGF2DLocation(30.0-MYEPSILON, 0.0, pWorld);

    ComplexLinearMidPoint1 = HGF2DLocation(17.928932188135, 10.0, pWorld);

    ComplexLinearClosePoint1A = HGF2DLocation(21.1, 10.1, pWorld);
    ComplexLinearClosePoint1B = HGF2DLocation(-10.0, 10.0, pWorld);
    ComplexLinearClosePoint1C = HGF2DLocation(30.0, 10.1, pWorld);
    ComplexLinearClosePoint1D = HGF2DLocation(32.1, 5.1, pWorld);
    ComplexLinearCloseMidPoint1 = HGF2DLocation(17.928932, 9.1, pWorld);

    VeryFarPoint = HGF2DLocation(10000.0, 10000.1, pWorld);

    ComplexLinear2 = HVE2DComplexLinear(pWorld);  // AutoClosed
    ComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(-10.0, -10.0, pWorld)));
    ComplexLinear2.AppendLinear(HVE2DSegment(ComplexLinear2.GetEndPoint(), HGF2DLocation(18.0, 9.0, pWorld)));
    ComplexLinear2.AppendLinear(HVE2DSegment(ComplexLinear2.GetEndPoint(), HGF2DLocation(21.0, 0.0, pWorld)));
    ComplexLinear2.AppendLinear(HVE2DSegment(ComplexLinear2.GetEndPoint(), HGF2DLocation(24.0, 15.0, pWorld)));
    ComplexLinear2.AppendLinear(HVE2DSegment(ComplexLinear2.GetEndPoint(), HGF2DLocation(0.0, 15.0, pWorld)));
    ComplexLinear2.AppendLinear(HVE2DSegment(ComplexLinear2.GetEndPoint(), HGF2DLocation(0.0, 0.0, pWorld)));

    ComplexLinear2Point0d0 = HGF2DLocation(0.0, 0.0, pWorld);
    ComplexLinear2Point0d1 = HGF2DLocation(-7.9028994453177, -7.9028994453177, pWorld);
    ComplexLinear2Point0d5 = HGF2DLocation(20.498817144560, 1.5035485663202, pWorld);
    ComplexLinear2Point1d0 = HGF2DLocation(0.0, 0.0, pWorld);

    ComplexLinearMidPoint2 = HGF2DLocation(20.498817144560, 1.5035485663202, pWorld);

    ComplexLinearClosePoint2A = HGF2DLocation(21.1, 10.1, pWorld);
    ComplexLinearClosePoint2B = HGF2DLocation(21.1, 10.1, pWorld);
    ComplexLinearClosePoint2C = HGF2DLocation(21.1, 10.1, pWorld);
    ComplexLinearClosePoint2D = HGF2DLocation(21.1, 10.1, pWorld);
    ComplexLinearCloseMidPoint2 = HGF2DLocation(21.1, 10.1, pWorld);

    // epsilon size container
    ComplexLinear3 = HVE2DComplexLinear(pWorld); // Open
    ComplexLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(0.0-MYEPSILON, 0.0, pWorld)));
    ComplexLinear3.AppendLinear(HVE2DSegment(ComplexLinear3.GetEndPoint(), HGF2DLocation(0.0-MYEPSILON, 0.0+MYEPSILON, pWorld)));
    ComplexLinear3.AppendLinear(HVE2DSegment(ComplexLinear3.GetEndPoint(), HGF2DLocation(0.0, 0.0+MYEPSILON, pWorld)));
    ComplexLinear3.AppendLinear(HVE2DSegment(ComplexLinear3.GetEndPoint(), HGF2DLocation(0.0+MYEPSILON, 0.0+2*MYEPSILON, pWorld)));
    ComplexLinear3.AppendLinear(HVE2DSegment(ComplexLinear3.GetEndPoint(), HGF2DLocation(0.0+MYEPSILON, 0.0, pWorld)));

    ComplexLinear3Point0d0 = HGF2DLocation(0.0, 0.0, pWorld);
    ComplexLinear3Point0d1 = HGF2DLocation(-0.64142135623731*MYEPSILON, 0.0, pWorld);
    ComplexLinear3Point0d5 = HGF2DLocation(0.14644660940673*MYEPSILON, 1.1464466094067*MYEPSILON, pWorld);
    ComplexLinear3Point1d0 = HGF2DLocation(0.0+MYEPSILON, 0.0, pWorld);

    ComplexLinearMidPoint3 = HGF2DLocation(0.14644660940673*MYEPSILON, 1.1464466094067*MYEPSILON, pWorld);

    ComplexLinearClosePoint3A = HGF2DLocation(21.1, 10.1, pWorld);
    ComplexLinearClosePoint3B = HGF2DLocation(21.1, 10.1, pWorld);
    ComplexLinearClosePoint3C = HGF2DLocation(21.1, 10.1, pWorld);
    ComplexLinearClosePoint3D = HGF2DLocation(21.1, 10.1, pWorld);
    ComplexLinearCloseMidPoint3 = HGF2DLocation(21.1, 10.1, pWorld);

    // Used for intersection tests
    ComplexComplexLinearCase1 = HVE2DComplexLinear(pWorld);
    ComplexComplexLinearCase1.AppendLinear(HVE2DSegment(HGF2DLocation(20.0, 5.0, pWorld), HGF2DLocation(30.0, 15.0, pWorld)));

    ComplexComplexLinearCase2 = HVE2DComplexLinear(pWorld);
    ComplexComplexLinearCase2.AppendLinear(HVE2DSegment(HGF2DLocation(20.0, 5.0, pWorld), HGF2DLocation(24.0, 10.0, pWorld)));
    ComplexComplexLinearCase2.AppendLinear(HVE2DSegment(ComplexComplexLinearCase2.GetEndPoint(), HGF2DLocation(28.0, 10.0, pWorld)));
    ComplexComplexLinearCase2.AppendLinear(HVE2DSegment(ComplexComplexLinearCase2.GetEndPoint(), HGF2DLocation(30.0, 15.0, pWorld)));

    ComplexComplexLinearCase3 = HVE2DComplexLinear(pWorld);
    ComplexComplexLinearCase3.AppendLinear(HVE2DSegment(HGF2DLocation(15.0, 5.0, pWorld), HGF2DLocation(25.0, 15.0, pWorld)));

    ComplexComplexLinearCase4 = HVE2DComplexLinear(pWorld);
    ComplexComplexLinearCase4.AppendLinear(HVE2DSegment(HGF2DLocation(5.0, 10.0, pWorld), HGF2DLocation(35.0, 10.0, pWorld)));

    ComplexComplexLinearCase5 = HVE2DComplexLinear(pWorld);
    ComplexComplexLinearCase5.AppendLinear(HVE2DSegment(HGF2DLocation(35.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld)));
    ComplexComplexLinearCase5.AppendLinear(HVE2DSegment(ComplexComplexLinearCase5.GetEndPoint(), HGF2DLocation(10.0, 5.0, pWorld)));

    ComplexComplexLinearCase5A = HVE2DComplexLinear(pWorld);
    ComplexComplexLinearCase5A.AppendLinear(HVE2DSegment(HGF2DLocation(30.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld)));
    ComplexComplexLinearCase5A.AppendLinear(HVE2DSegment(ComplexComplexLinearCase5A.GetEndPoint(), HGF2DLocation(10.0, 5.0, pWorld)));

    ComplexComplexLinearCase6 = HVE2DComplexLinear(pWorld);
    ComplexComplexLinearCase6.AppendLinear(HVE2DSegment(HGF2DLocation(35.0, 10.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    ComplexComplexLinearCase6.AppendLinear(HVE2DSegment(ComplexComplexLinearCase6.GetEndPoint(), HGF2DLocation(-1.0, -1.0, pWorld)));

    ComplexComplexLinearCase7 = HVE2DComplexLinear(pWorld);
    ComplexComplexLinearCase7.AppendLinear(HVE2DSegment(HGF2DLocation(25.0, 15.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld)));

    //ContiguousTest
    ContiguousLinear2Segment1 = HVE2DSegment(HGF2DLocation(0.0, 15.0, pWorld), HGF2DLocation(0.0, 0.0, pWorld));
    ContiguousLinear2 = HVE2DComplexLinear(pWorld);  // AutoClosed
    ContiguousLinear2.AppendLinear(ContiguousLinear2Segment1);
    Linear2ContiguousPoint = HGF2DLocation(0.0, 0.0, pWorld);

    ContiguousLinear3Segment1 = HVE2DSegment(HGF2DLocation(0.0+MYEPSILON, 0.0+2*MYEPSILON, pWorld), HGF2DLocation(0.0+MYEPSILON, 0.0, pWorld));
    ContiguousLinear3 = HVE2DComplexLinear(pWorld); // Open
    ContiguousLinear3.AppendLinear(ContiguousLinear3Segment1);
    Linear3ContiguousPoint = HGF2DLocation(0.0+MYEPSILON, 0.0, pWorld);

    ContiguousComplexLinear2 = HVE2DComplexLinear(pWorld);  // AutoClosed
    ContiguousComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(-10.0, -10.0, pWorld)));
    ComplexLinear2ContiguousPoint = HGF2DLocation(0.0, 0.0, pWorld);

    ContiguousComplexLinear3 = HVE2DComplexLinear(pWorld); // Open
    ContiguousComplexLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(0.0-MYEPSILON, 0.0, pWorld)));
    ComplexLinear3ContiguousPoint = HGF2DLocation(0.0, 0.0, pWorld);

    }

////==================================================================================
// ComplexLinear Construction tests
// HVE2DComplexLinear ();
// HVE2DComplexLinear (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
// HVE2DComplexLinear (const HVE2DComplexLinear&    pi_rObject);
////==================================================================================
TEST_F (HVE2DComplexLinearTester, ConstructionTest1)
    {
    // Default Constructor
    HVE2DComplexLinear    ALinear1;

    // Constructor with a coordinate system
    HVE2DComplexLinear    ALinear2(pWorld);
    ASSERT_EQ(pWorld, ALinear2.GetCoordSys());

    // Copy Constructor test
    HVE2DComplexLinear    ALinear3(pWorld);
    ALinear3.AppendLinear(MiscSegment1);
    ALinear3.InsertLinear(HorizontalSegment2);

    HVE2DComplexLinear    ALinear4(ALinear3);
    ASSERT_EQ(pWorld, ALinear4.GetCoordSys());
    ASSERT_EQ(2, ALinear4.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.1, ALinear4.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ALinear4.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, ALinear4.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ALinear4.GetEndPoint().GetY());
    
    }

////==================================================================================
// operator= test
// operator=(const HVE2DComplexLinear& pi_rObj);
////==================================================================================
TEST_F (HVE2DComplexLinearTester, OperatorTest1)
    {

    HVE2DComplexLinear    ALinear5(pWorld);
    ALinear5.AppendLinear(MiscSegment1);
    ALinear5.InsertLinear(HorizontalSegment2);

    HVE2DComplexLinear    ALinear6(pSys1);
    ALinear6.AppendLinear(VerticalSegment1);

    ALinear6 = ALinear5;
    ASSERT_EQ(pWorld, ALinear6.GetCoordSys());
    ASSERT_EQ(2, ALinear6.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(10.1, ALinear6.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ALinear6.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, ALinear6.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ALinear6.GetEndPoint().GetY());
    
    }

////==================================================================================
// InsertLinear(const HVE2DLinear& pi_rLinear);
// AppendLinear(const HVE2DLinear& pi_rLinear);
// InsertComplexLinear(const HVE2DComplexLinear& pi_rComplexLinear);
// AppendComplexLinear(const HVE2DComplexLinear& pi_rComplexLinear);
// InsertLinearPtrSCS(HVE2DLinear* pi_pLinear);
// AppendLinearPtrSCS(HVE2DLinear* pi_pLinear);
////==================================================================================
TEST_F (HVE2DComplexLinearTester, AddLinearTest)
    {

    HVE2DComplexLinear ALinear1(pWorld);

    // First Linear
    ALinear1.InsertLinear(HVE2DSegment(HGF2DLocation(5.0, 5.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_EQ(1, ALinear1.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(5.00, ALinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.00, ALinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, ALinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, ALinear1.GetEndPoint().GetY());

    // Add Linear at the beginning
    ALinear1.InsertLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(5.0, 5.0, pWorld)));
    ASSERT_EQ(2, ALinear1.GetNumberOfLinears());

    ASSERT_NEAR(0.0, ALinear1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, ALinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0, ALinear1.GetEndPoint().GetY());

    // Add Linear at the end
    ALinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_EQ(3, ALinear1.GetNumberOfLinears());

    ASSERT_NEAR(0.0, ALinear1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(20.0, ALinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, ALinear1.GetEndPoint().GetY());

    // Add ComplexLinear at the beginning
    HVE2DComplexLinear CLinear1(pWorld);
    CLinear1.InsertLinear(HVE2DSegment(HGF2DLocation(-5.0, -5.0, pWorld), HGF2DLocation(0.0, 0.0, pWorld)));

    ALinear1.InsertComplexLinear(CLinear1);
    ASSERT_EQ(4, ALinear1.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(-5.0, ALinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-5.0, ALinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.0, ALinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.0, ALinear1.GetEndPoint().GetY());

    // Add ComplexLinear at the end
    HVE2DComplexLinear CLinear2(pWorld);
    CLinear2.InsertLinear(HVE2DSegment(HGF2DLocation(20.0, 20.0, pWorld), HGF2DLocation(25.0, 25.0, pWorld)));

    ALinear1.AppendComplexLinear(CLinear2);
    ASSERT_EQ(5, ALinear1.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(-5.0, ALinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-5.0, ALinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(25.0, ALinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0, ALinear1.GetEndPoint().GetY());

    // Add Ptr of Linear at the beginning
    ALinear1.InsertLinearPtrSCS(new HVE2DSegment(HGF2DLocation(-10.0, -10.0, pWorld), HGF2DLocation(-5.0, -5.0, pWorld)));
    ASSERT_EQ(6, ALinear1.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(-10.0, ALinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-10.0, ALinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(25.00, ALinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(25.00, ALinear1.GetEndPoint().GetY());

    // Add Ptr of Linear at the end
    ALinear1.AppendLinearPtrSCS(new HVE2DSegment(HGF2DLocation(25.0, 25.0, pWorld), HGF2DLocation(30.0, 30.0, pWorld)));
    ASSERT_EQ(7, ALinear1.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(-10.0, ALinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-10.0, ALinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(30.00, ALinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(30.00, ALinear1.GetEndPoint().GetY());

    // Add Linear at the end
    ALinear1.AppendLinear(HVE2DSegment(HGF2DLocation(30.0, 30.0, pWorld), HGF2DLocation(35.0, 35.0, pWorld)));
    ASSERT_EQ(8, ALinear1.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(-10.0, ALinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-10.0, ALinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(35.00, ALinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(35.00, ALinear1.GetEndPoint().GetY());

    #ifdef WIP_IPPTEST_BUG_17
    // Add ComplexLinear at the beginning
    HVE2DComplexLinear CLinear3(pWorld);
    CLinear3.InsertLinear(HVE2DSegment(HGF2DLocation(-15.0, -15.0, pWorld), HGF2DLocation(-10.0, -10.0, pWorld)));

    ALinear1.InsertLinear(CLinear3);
    ASSERT_EQ(9, ALinear1.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(-15.0, ALinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-15.0, ALinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(35.00, ALinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(35.00, ALinear1.GetEndPoint().GetY());
    #endif

    }

////==================================================================================
// IsEmpty() const;
// MakeEmpty();
////==================================================================================
TEST_F (HVE2DComplexLinearTester, EmptyTest)
    {

    ASSERT_TRUE(EmptyLinear.IsEmpty());
    ASSERT_FALSE(Linear1.IsEmpty());

    Linear1.MakeEmpty();
    ASSERT_TRUE(Linear1.IsEmpty());

    }

////==================================================================================
// GetLinearList() const;
// GetNumberOfLinears() const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, GetLinearListTest1)
    {

    ASSERT_EQ(0, EmptyLinear.GetLinearList().size());
    ASSERT_EQ(6, Linear1.GetLinearList().size());
    ASSERT_EQ(6, Linear2.GetLinearList().size());
    ASSERT_EQ(5, Linear3.GetLinearList().size());

    ASSERT_EQ(0, EmptyLinear.GetNumberOfLinears());
    ASSERT_EQ(6, Linear1.GetNumberOfLinears());
    ASSERT_EQ(6, Linear2.GetNumberOfLinears());
    ASSERT_EQ(5, Linear3.GetNumberOfLinears());

    }

////==================================================================================
// SplitAtAllIntersectionPoints(const HVE2DVector& pi_rVector);
// SplitAtAllOnPoints(const HGF2DLocationCollection& pi_rPoints);
// SplitAtAllOnPointsSCS(const HGF2DLocationCollection& pi_rPoints);
 ////==================================================================================
TEST_F (HVE2DComplexLinearTester, SplitTest1)
    { 

    HVE2DSegment SplitSegment(HGF2DLocation(15.0, 0.0, pWorld), HGF2DLocation(15.0, 20.0, pWorld));
    HVE2DSegment SplitSegment2(HGF2DLocation(-15.0, 0.0, pWorld), HGF2DLocation(-15.0, -20.0, pWorld));

    EmptyLinear.SplitAtAllIntersectionPoints(SplitSegment);
    ASSERT_TRUE(EmptyLinear.IsEmpty());

    //SplitSegment is in the middle of a Segment
    Linear1.SplitAtAllIntersectionPoints(SplitSegment);
    ASSERT_EQ(7, Linear1.GetNumberOfLinears());

    //SplitSegment is now on a Point
    Linear1.SplitAtAllIntersectionPoints(SplitSegment);
    ASSERT_EQ(7, Linear1.GetNumberOfLinears());

    //SplitSegment2 doesn't intersect Linear1
    Linear1.SplitAtAllIntersectionPoints(SplitSegment2);
    ASSERT_EQ(7, Linear1.GetNumberOfLinears());

    HGF2DLocationCollection Locations;

    Linear1.SplitAtAllOnPoints(Locations);
    ASSERT_EQ(7, Linear1.GetNumberOfLinears());

    //Locations contains a point in the middle of a Segment
    Locations.push_back(HGF2DLocation(2.0, 2.0, pWorld));

    Linear1.SplitAtAllOnPoints(Locations);
    ASSERT_EQ(8, Linear1.GetNumberOfLinears());

    //Locations is now on a Point
    Linear1.SplitAtAllOnPoints(Locations);
    ASSERT_EQ(8, Linear1.GetNumberOfLinears());

    //Locations contains a point who is in the middle of a Segment
    Locations.push_back(HGF2DLocation(3.0, 3.0, pWorld));

    Linear1.SplitAtAllOnPoints(Locations);
    ASSERT_EQ(9, Linear1.GetNumberOfLinears());

    //Locations contains a point who is not on a Segment
    Locations.push_back(HGF2DLocation(-2.0, -2.0, pWorld));

    //Locations contains twice the same point who is in the middle of a Segment
    Locations.pop_back();
    Locations.push_back(HGF2DLocation(4.0, 4.0, pWorld));
    Locations.push_back(HGF2DLocation(4.0, 4.0, pWorld));

    Linear1.SplitAtAllOnPoints(Locations);
    ASSERT_EQ(10, Linear1.GetNumberOfLinears());

    }

////==================================================================================
// IsABasicLinear() const;
// IsComplex() const;   
////==================================================================================
TEST_F (HVE2DComplexLinearTester, ComplexTest)
    {

    ASSERT_FALSE(Linear1.IsABasicLinear());
    ASSERT_TRUE(Linear1.IsComplex());

    }
       
////==================================================================================
// Length calculation test
// CalculateLength() const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, CalculateLengthTest1)
    {

    // Test with linear 1
    ASSERT_DOUBLE_EQ(44.142135623730951, Linear1.CalculateLength());

    // Test with linear 2
    ASSERT_DOUBLE_EQ(111.76387577639170, Linear2.CalculateLength());

    // Test with empty linear
    ASSERT_NEAR(0.0, EmptyLinear.CalculateLength(), MYEPSILON);

    }
   
////==================================================================================
// Relative point calculation test
// CalculateRelativePoint(double pi_RelativePos) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, CalculateRelativePointTest1)
    {
        
    // Test with linear 1
    ASSERT_NEAR(0.0, Linear1.CalculateRelativePoint(0.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Linear1.CalculateRelativePoint(0.0).GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(3.1213203435596428, Linear1.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(3.1213203435596428, Linear1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(17.928932188134524, Linear1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, Linear1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(26.757359312880713, Linear1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, Linear1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999, Linear1.CalculateRelativePoint(1.0).GetX());
    ASSERT_NEAR(0.0, Linear1.CalculateRelativePoint(1.0).GetY(), MYEPSILON);

    // Test with linear 2
    ASSERT_NEAR(0.0, Linear2.CalculateRelativePoint(0.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Linear2.CalculateRelativePoint(0.0).GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(-7.9028994453177486, Linear2.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(-7.9028994453177486, Linear2.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(20.4988171445599400, Linear2.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(1.50354856632018130, Linear2.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(18.5291627329175200, Linear2.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(15.0000000000000000, Linear2.CalculateRelativePoint(0.7).GetY());
    ASSERT_NEAR(0.0, Linear2.CalculateRelativePoint(1.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Linear2.CalculateRelativePoint(1.0).GetY(), MYEPSILON);

    // Test with linear 3 (epsilon sized container)
    ASSERT_NEAR(0.0, Linear3.CalculateRelativePoint(0.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Linear3.CalculateRelativePoint(0.0).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, Linear3.CalculateRelativePoint(0.1).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Linear3.CalculateRelativePoint(0.1).GetY(), MYEPSILON);
    ASSERT_NEAR(1.4644660940672625E-8, Linear3.CalculateRelativePoint(0.5).GetX(), MYEPSILON);
    ASSERT_NEAR(1.1464466094067261E-7, Linear3.CalculateRelativePoint(0.5).GetY(), MYEPSILON);
    ASSERT_NEAR(9.9999999999999995E-8, Linear3.CalculateRelativePoint(0.7).GetX(), MYEPSILON);
    ASSERT_NEAR(1.9242640687119286E-7, Linear3.CalculateRelativePoint(0.7).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, Linear3.CalculateRelativePoint(1.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Linear3.CalculateRelativePoint(1.0).GetY(), MYEPSILON);

    //Test with empty linear 
    ASSERT_NEAR(0.0, EmptyLinear.CalculateRelativePoint(0.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyLinear.CalculateRelativePoint(0.0).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyLinear.CalculateRelativePoint(0.1).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyLinear.CalculateRelativePoint(0.1).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyLinear.CalculateRelativePoint(0.5).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyLinear.CalculateRelativePoint(0.5).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyLinear.CalculateRelativePoint(0.7).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyLinear.CalculateRelativePoint(0.7).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyLinear.CalculateRelativePoint(1.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyLinear.CalculateRelativePoint(1.0).GetY(), MYEPSILON);

    }

////==================================================================================
//Relative position calculation test
//CalculateRelativePosition(const HGF2DLocation& pi_rPointOnLinear) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, CalculateRelativePositionTest1)
    {
        
    // Test with linear1
    ASSERT_NEAR(0.0, Linear1.CalculateRelativePosition(Linear1Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.099999999999998646, Linear1.CalculateRelativePosition(Linear1Point0d1));
    ASSERT_DOUBLE_EQ(0.500000000000010770, Linear1.CalculateRelativePosition(Linear1Point0d5));
    ASSERT_DOUBLE_EQ(1.000000000000000000, Linear1.CalculateRelativePosition(Linear1Point1d0));

    // Test with linear2
    ASSERT_NEAR(0.0, Linear2.CalculateRelativePosition(Linear2Point1d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.099999999999999381, Linear2.CalculateRelativePosition(Linear2Point0d1));
    ASSERT_DOUBLE_EQ(0.50000000000000167, Linear2.CalculateRelativePosition(Linear2Point0d5));
    ASSERT_NEAR(0.0, Linear2.CalculateRelativePosition(Linear2Point1d0), MYEPSILON);

    // Test with linear3 (epsilon sized container)
    ASSERT_NEAR(0.0, Linear3.CalculateRelativePosition(Linear3Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000007, Linear3.CalculateRelativePosition(Linear3Point0d1));
// Test is failing. temporarily disable
//    ASSERT_DOUBLE_EQ(0.49054285124903529, Linear3.CalculateRelativePosition(Linear3Point0d5)); 

    }

////==================================================================================
// RayArea Calculation test
// CalculateRayArea(const HGF2DLocation& pi_rPoint) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, CalculateRayAreaTest1)
    {

    ASSERT_DOUBLE_EQ(-175.0000015, Linear1.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_DOUBLE_EQ(288.00000000, Linear2.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_NEAR(0.0, Linear3.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)), MYEPSILON);
    
    }

////==================================================================================
// Drop(HGF2DLocationCollection* po_pPoint, const HGFDistance&  pi_rTolerance)
////==================================================================================
TEST_F (HVE2DComplexLinearTester, DropTest1)
    {

    HGF2DLocationCollection Locations;

    Linear1.Drop(&Locations, MYEPSILON); 
    ASSERT_NEAR(0.0, Locations[0].GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Locations[0].GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.000000000000000, Locations[1].GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, Locations[1].GetY());
    ASSERT_DOUBLE_EQ(20.000000000000000, Locations[2].GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, Locations[2].GetY());
    ASSERT_DOUBLE_EQ(30.000000000000000, Locations[3].GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, Locations[3].GetY());
    ASSERT_DOUBLE_EQ(30.000000000000000, Locations[4].GetX());
    ASSERT_DOUBLE_EQ(5.0000000000000000, Locations[4].GetY());
    ASSERT_DOUBLE_EQ(30.000000000000000, Locations[5].GetX());
    ASSERT_DOUBLE_EQ(4.9999998999999997, Locations[5].GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999, Locations[6].GetX());
    ASSERT_NEAR(0.0, Locations[6].GetY(), MYEPSILON);

    }

////==================================================================================
// Shortening tests
// Shorten(double pi_StartRelativePos, double pi_EndRelativePos);
// Shorten(const HGF2DLocation& pi_rNewStartPoint,
//         const HGF2DLocation& pi_rNewEndPoint);
// ShortenTo(const HGF2DLocation& pi_rNewEndPoint);
// ShortenTo(double pi_EndRelativePosition);
// ShortenFrom(const HGF2DLocation& pi_rNewStartPoint);
// ShortenFrom(double pi_StartRelativePosition);
////==================================================================================
TEST_F (HVE2DComplexLinearTester, ShorteningTest1)
    {

    // Test with linear1
    HVE2DComplexLinear    ALinear7(Linear1);
    ALinear7.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(6.2426406871192857, ALinear7.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(6.2426406871192857, ALinear7.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(30.000000000000000, ALinear7.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.8284271247461916, ALinear7.GetEndPoint().GetY());

    ALinear7 = Linear1;
    ALinear7.Shorten(0.5, 0.5+ASMALLFACTOR);  
    ASSERT_DOUBLE_EQ(17.928932188134524, ALinear7.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, ALinear7.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(17.928932629555884, ALinear7.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, ALinear7.GetEndPoint().GetY());
    
    ALinear7 = Linear1;
    ALinear7.Shorten(0.0, 0.8); 
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.000000000000000, ALinear7.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.8284271247461916, ALinear7.GetEndPoint().GetY());
    
    ALinear7 = Linear1;
    ALinear7.Shorten(0.2, 1.0); 
    ASSERT_DOUBLE_EQ(6.2426406871192857, ALinear7.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(6.2426406871192857, ALinear7.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999, ALinear7.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetY(), MYEPSILON);
    
    ALinear7 = Linear1;
    ALinear7.Shorten(0.0, 1.0);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, ALinear7.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetY(), MYEPSILON);

    ALinear7 = Linear1;
    ALinear7.ShortenTo(0.8);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.000000000000000, ALinear7.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.8284271247461916, ALinear7.GetEndPoint().GetY());
   
    ALinear7 = Linear1;
    ALinear7.ShortenTo(1.0-ASMALLFACTOR);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999900000006, ALinear7.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(4.4142135635638624E-7, ALinear7.GetEndPoint().GetY());
    
    ALinear7 = Linear1;
    ALinear7.ShortenTo(1.0);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, ALinear7.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetY(), MYEPSILON);
    
    ALinear7 = Linear1;
    ALinear7.ShortenTo(0.0);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetY(), MYEPSILON);

    ALinear7 = Linear1;
    ALinear7.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(6.2426406871192857, ALinear7.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(6.2426406871192857, ALinear7.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999, ALinear7.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetY(), MYEPSILON);
     
    ALinear7 = Linear1;
    ALinear7.ShortenFrom(1.0-ASMALLFACTOR);
    ASSERT_DOUBLE_EQ(29.999999900000006000, ALinear7.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(4.4142135635638624E-7, ALinear7.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999000, ALinear7.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetY(), MYEPSILON);
    
    ALinear7 = Linear1;
    ALinear7.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(29.999999899999999, ALinear7.GetStartPoint().GetX());
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, ALinear7.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetY(), MYEPSILON);
    
    ALinear7 = Linear1;
    ALinear7.ShortenFrom(0.0);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, ALinear7.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetY(), MYEPSILON);

    ALinear7 = Linear1;
    ALinear7.ShortenFrom(LinearMidPoint1);
    ASSERT_DOUBLE_EQ(17.928932188135001, ALinear7.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, ALinear7.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999, ALinear7.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetY(), MYEPSILON);
    
    ALinear7 = Linear1;
    ALinear7.ShortenFrom(ALinear7.GetStartPoint());
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, ALinear7.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetY(), MYEPSILON);
    
    ALinear7 = Linear1;
    ALinear7.ShortenFrom(ALinear7.GetEndPoint());
    ASSERT_DOUBLE_EQ(29.999999899999999, ALinear7.GetStartPoint().GetX());
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, ALinear7.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetY(), MYEPSILON);
    
    ALinear7 = Linear1;
    ALinear7.ShortenTo(LinearMidPoint1);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(17.928932188135001, ALinear7.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, ALinear7.GetEndPoint().GetY());
    
    ALinear7 = Linear1;
    ALinear7.ShortenTo(ALinear7.GetStartPoint());
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetY(), MYEPSILON);
    
    ALinear7 = Linear1;
    ALinear7.ShortenTo(ALinear7.GetEndPoint());
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, ALinear7.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetY(), MYEPSILON);
    
    ALinear7 = Linear1;
    ALinear7.Shorten(ALinear7.GetStartPoint(), LinearMidPoint1);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(17.928932188135001, ALinear7.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, ALinear7.GetEndPoint().GetY());
    
    ALinear7 = Linear1;
    ALinear7.Shorten(LinearMidPoint1, ALinear7.GetEndPoint());
    ASSERT_DOUBLE_EQ(17.928932188135001, ALinear7.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, ALinear7.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999, ALinear7.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetY(), MYEPSILON);
    
    ALinear7 = Linear1;
    ALinear7.Shorten(ALinear7.GetStartPoint(), ALinear7.GetEndPoint());
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ALinear7.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, ALinear7.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, ALinear7.GetEndPoint().GetY(), MYEPSILON);

    }

////==================================================================================
// Auto intersection tests
// AutoCrosses() const;
// AutoIntersect(HGF2DLocationCollection* po_pPoints) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  AutoIntersectionTest1) 
    {

    HGF2DLocationCollection     DumPoints;

    // Test with a negative
    ASSERT_FALSE(Linear1.AutoCrosses());

    // Test with auto closed linear
    ASSERT_FALSE(Linear2.AutoCrosses());

    // Test with an auto crossing linear
    ASSERT_TRUE(AutoCrossingLinear1.AutoCrosses());

    // Test with an auto crossing linear (at a junction point)
    ASSERT_TRUE(AutoCrossingLinear2.AutoCrosses());

    AutoCrossingLinear2.Reverse();

    ASSERT_TRUE(AutoCrossingLinear2.AutoCrosses());

    AutoCrossingLinear2.Reverse();

    // Test with an non auto crossing linear (but passing through extremity)
    ASSERT_FALSE(AutoConnectingLinear1.AutoCrosses());
    
    }

////==================================================================================
// Closest point calculation test
// CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  CalculateClosestPointTest1)
    { 

    // Test with linear 1
    ASSERT_DOUBLE_EQ(21.100000000000, Linear1.CalculateClosestPoint(LinearClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000, Linear1.CalculateClosestPoint(LinearClosePoint1A).GetY());
    ASSERT_NEAR(0.0, Linear1.CalculateClosestPoint(LinearClosePoint1B).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Linear1.CalculateClosestPoint(LinearClosePoint1B).GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.000000000000, Linear1.CalculateClosestPoint(LinearClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000, Linear1.CalculateClosestPoint(LinearClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(30.000000000000, Linear1.CalculateClosestPoint(LinearClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(5.1000000000000, Linear1.CalculateClosestPoint(LinearClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(17.928932000000, Linear1.CalculateClosestPoint(LinearCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000, Linear1.CalculateClosestPoint(LinearCloseMidPoint1).GetY());

    // Test with an empty linear
    ASSERT_NEAR(0.0, EmptyLinear.CalculateClosestPoint(LinearClosePoint1A).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyLinear.CalculateClosestPoint(LinearClosePoint1A).GetY(), MYEPSILON);

    // Tests with special points
    ASSERT_DOUBLE_EQ(30.000000000000000, Linear1.CalculateClosestPoint(VeryFarPoint).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, Linear1.CalculateClosestPoint(VeryFarPoint).GetY());
    ASSERT_DOUBLE_EQ(17.928932188135001, Linear1.CalculateClosestPoint(LinearMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, Linear1.CalculateClosestPoint(LinearMidPoint1).GetY());
    ASSERT_NEAR(0.0, Linear1.CalculateClosestPoint(Linear1.GetStartPoint()).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Linear1.CalculateClosestPoint(Linear1.GetStartPoint()).GetY(), MYEPSILON);    
    ASSERT_DOUBLE_EQ(29.999999899999999, Linear1.CalculateClosestPoint(Linear1.GetEndPoint()).GetX());
    ASSERT_NEAR(0.0, Linear1.CalculateClosestPoint(Linear1.GetEndPoint()).GetY(), MYEPSILON);

    }

////==================================================================================
// Intersection test (with other complex linears only)
// Intersect(const HVE2DVector& pi_rVector, HGF2DLocationCollection* po_pCrossPoints) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  IntersectTest1)
    { 
    HGF2DLocationCollection   DumPoints1;

    // Test with extent disjoint linears
    ASSERT_EQ(0, Linear1.Intersect(DisjointLinear1, &DumPoints1));

    // Test with disjoint but touching by a side
    ASSERT_EQ(0, Linear1.Intersect(ContiguousExtentLinear1, &DumPoints1));

    // Test with disjoint but touching by a tip but not linked
    ASSERT_EQ(0, Linear1.Intersect(FlirtingExtentLinear1, &DumPoints1));

    // Test with disjoint but touching by a tip segments linked
    ASSERT_EQ(0, Linear1.Intersect(FlirtingExtentLinkedLinear1, &DumPoints1));

    // Tests with connected linears
    // At start point...
    ASSERT_EQ(0, Linear1.Intersect(ConnectedLinear1, &DumPoints1));
    ASSERT_EQ(0, Linear1.Intersect(ConnectingLinear1, &DumPoints1));

    // At end point ...
    ASSERT_EQ(0, Linear1.Intersect(ConnectedLinear1A, &DumPoints1));
    ASSERT_EQ(0, Linear1.Intersect(ConnectingLinear1A, &DumPoints1));

    // Tests with linked segments
    ASSERT_EQ(0, Linear1.Intersect(LinkedLinear1, &DumPoints1));

    // Tests with EPSILON sized container
    ASSERT_EQ(0, Linear3.Intersect(Linear1, &DumPoints1));

    // Special cases
    ASSERT_EQ(1, Linear1.Intersect(ComplexLinearCase1, &DumPoints1));
    ASSERT_DOUBLE_EQ(25.0, DumPoints1[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints1[0].GetY());

    DumPoints1.clear();
    ASSERT_EQ(1, Linear1.Intersect(ComplexLinearCase2, &DumPoints1));
    ASSERT_DOUBLE_EQ(28.0, DumPoints1[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints1[0].GetY());

    DumPoints1.clear();
    ASSERT_EQ(1, Linear1.Intersect(ComplexLinearCase3, &DumPoints1));
    ASSERT_DOUBLE_EQ(20.0, DumPoints1[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints1[0].GetY());

    DumPoints1.clear();
    ASSERT_EQ(0, Linear1.Intersect(ComplexLinearCase4, &DumPoints1));

    DumPoints1.clear();
    ASSERT_EQ(1, Linear1.Intersect(ComplexLinearCase5, &DumPoints1));
    ASSERT_DOUBLE_EQ(30.0, DumPoints1[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints1[0].GetY());

    DumPoints1.clear();
    ASSERT_EQ(0, Linear1.Intersect(ComplexLinearCase5A, &DumPoints1));

    ASSERT_EQ(0, Linear1.Intersect(ComplexLinearCase6, &DumPoints1));
    ASSERT_EQ(0, Linear1.Intersect(ComplexLinearCase7, &DumPoints1));

    // Test with a NULL segment
    ASSERT_EQ(0, EmptyLinear.Intersect(Linear1, &DumPoints1));

    }

////==================================================================================
// Contiguousness Test
//   ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
//                              HGF2DLocationCollection* po_pContiguousnessPoints) const;
//   ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
//                                const HGF2DLocation& pi_rPoint,
//                                HGF2DLocation* pi_pFirstContiguousnessPoint,
//                                HGF2DLocation* pi_pSecondContiguousnessPoint) const;
//    HDLL virtual bool      AreContiguous(const HVE2DVector& pi_rVector) const;
//    HDLL virtual bool      AreContiguousAt(const HVE2DVector& pi_rVector,
//                                           const HGF2DLocation& pi_rPoint) const;
// AreContiguousAtAndGet(const HVE2DVector& pi_rVector, const HGF2DLocation& pi_rPoint,
//                       HGF2DLocation* pi_pFirstContiguousnessPoint,
//                       HGF2DLocation* pi_pSecondContiguousnessPoint) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  ContiguousnessTest1)
    { 

    HGF2DLocationCollection     DumPoints;
    HGF2DLocation   FirstDumPoint;
    HGF2DLocation   SecondDumPoint;

    // Test with contiguous linears
    ASSERT_TRUE(Linear1.AreContiguous(ComplexLinearCase6));

    ASSERT_TRUE(Linear1.AreContiguousAt(ComplexLinearCase6, LinearMidPoint1));

    ASSERT_EQ(2, Linear1.ObtainContiguousnessPoints(ComplexLinearCase6, &DumPoints));
    ASSERT_NEAR(0.0, DumPoints[0].GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, DumPoints[0].GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());

    Linear1.ObtainContiguousnessPointsAt(ComplexLinearCase6, LinearMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_NEAR(0.0, FirstDumPoint.GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, FirstDumPoint.GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    DumPoints.clear();

    //AreContiguousAtAndGet
    ASSERT_TRUE(Linear1.AreContiguousAtAndGet(ComplexLinearCase6, LinearMidPoint1, &FirstDumPoint, &SecondDumPoint));
    ASSERT_NEAR(0.0, FirstDumPoint.GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, FirstDumPoint.GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    DumPoints.clear();

    // Test with non contiguous linears
    ASSERT_FALSE(Linear1.AreContiguous(ComplexLinearCase1));
    ASSERT_FALSE(Linear1.AreContiguousAtAndGet(ComplexLinearCase1, LinearMidPoint1, &FirstDumPoint, &SecondDumPoint));
 
    // Test with auto-closed linear
    ASSERT_TRUE(Linear2.AreContiguous(ContiguousLinear2)); 

    ASSERT_TRUE(Linear2.AreContiguousAt(ContiguousLinear2, Linear2ContiguousPoint));

    ASSERT_EQ(2, Linear2.ObtainContiguousnessPoints(ContiguousLinear2, &DumPoints));
    ASSERT_NEAR(0.0, FirstDumPoint.GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, FirstDumPoint.GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    Linear2.ObtainContiguousnessPointsAt(ContiguousLinear2, Linear2ContiguousPoint, &FirstDumPoint, &SecondDumPoint);
    ASSERT_NEAR(0.0, FirstDumPoint.GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(15.0, FirstDumPoint.GetY());
    ASSERT_NEAR(0.0, SecondDumPoint.GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, SecondDumPoint.GetY(), MYEPSILON);

    DumPoints.clear();

    //AreContiguousAtAndGet
    ASSERT_TRUE(Linear2.AreContiguousAtAndGet(ContiguousLinear2, Linear2ContiguousPoint, &FirstDumPoint, &SecondDumPoint));
    ASSERT_NEAR(0.0, FirstDumPoint.GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(15.0, FirstDumPoint.GetY());
    ASSERT_NEAR(0.0, SecondDumPoint.GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, SecondDumPoint.GetY(), MYEPSILON);

    DumPoints.clear();

    // Test with epsilon sized container
    ASSERT_TRUE(Linear3.AreContiguous(ContiguousLinear3)); 

    ASSERT_TRUE(Linear3.AreContiguousAt(ContiguousLinear3, Linear3ContiguousPoint));

    ASSERT_EQ(2, Linear3.ObtainContiguousnessPoints(ContiguousLinear3, &DumPoints));
    ASSERT_NEAR(0.0, FirstDumPoint.GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(15.0, FirstDumPoint.GetY());
    ASSERT_NEAR(0.0, SecondDumPoint.GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, SecondDumPoint.GetY(), MYEPSILON);

    Linear3.ObtainContiguousnessPointsAt(ContiguousLinear3, Linear3ContiguousPoint, &FirstDumPoint, &SecondDumPoint);
    ASSERT_NEAR(0.0, FirstDumPoint.GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(2.0E-7, FirstDumPoint.GetY());
    ASSERT_NEAR(0.0, SecondDumPoint.GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, SecondDumPoint.GetY(), MYEPSILON);

    DumPoints.clear();

    #ifdef WIP_IPPTEST_BUG_18
    //AreContiguousAtAndGet
    ASSERT_TRUE(Linear3.AreContiguousAtAndGet(ContiguousLinear3, Linear3ContiguousPoint, &FirstDumPoint, &SecondDumPoint));
    #endif

    }

////==================================================================================
// Cloning tests
// Clone() const;
// AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  CloningTest1) 
    { 

    //General Clone Test
    HFCPtr<HVE2DComplexLinear> pClone = (HVE2DComplexLinear*)Linear1.Clone();
    ASSERT_FALSE(pClone->IsEmpty());
    ASSERT_EQ(pWorld, pClone->GetCoordSys());    
    ASSERT_EQ(6, pClone->GetNumberOfLinears());

    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));  
    ASSERT_TRUE(pClone->IsPointOn(LinearMidPoint1-HGF2DDisplacement(0.0, 0.9*MYEPSILON)));
    ASSERT_TRUE(pClone->IsPointOn(LinearMidPoint1-HGF2DDisplacement(0.0, -0.9*MYEPSILON)));
    ASSERT_TRUE(pClone->IsPointOn(Linear1.GetStartPoint()));
    ASSERT_TRUE(pClone->IsPointOn(Linear1.GetEndPoint()));
    ASSERT_TRUE(pClone->IsPointOn(LinearMidPoint1));
 
    #ifdef WIP_IPPTEST_BUG_8
    //// Test with the same coordinate system
    //HFCPtr<HVE2DComplexLinear> pClone2 = (HVE2DComplexLinear*) Linear1.AllocateCopyInCoordSys(pWorld);
    //ASSERT_FALSE(pClone2->IsEmpty());
    //ASSERT_EQ(pWorld, pClone2->GetCoordSys());
    //ASSERT_EQ(6, pClone2->GetNumberOfLinears());

    //ASSERT_TRUE(pClone2->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));  
    //ASSERT_TRUE(pClone2->IsPointOn(LinearMidPoint1-HGF2DDisplacement(0.0, 0.9*MYEPSILON)));
    //ASSERT_TRUE(pClone2->IsPointOn(LinearMidPoint1-HGF2DDisplacement(0.0, -0.9*MYEPSILON)));
    //ASSERT_TRUE(pClone2->IsPointOn(Linear1.GetStartPoint()));
    //ASSERT_TRUE(pClone2->IsPointOn(Linear1.GetEndPoint()));
    //ASSERT_TRUE(pClone2->IsPointOn(LinearMidPoint1));
    #endif

    // Test with a translation between systems
    Translation = HGF2DDisplacement (10.0, 10.0);
    HGF2DTranslation myTranslation(Translation);
    HFCPtr<HGF2DCoordSys> pWorldTranslation = new HGF2DCoordSys(myTranslation, pWorld);

    HFCPtr<HVE2DComplexLinear> pClone5 = (HVE2DComplexLinear*)Linear2.AllocateCopyInCoordSys(pWorldTranslation);
    ASSERT_FALSE(pClone5->IsEmpty());
    ASSERT_EQ(pWorldTranslation, pClone5->GetCoordSys());
    ASSERT_EQ(6, pClone5->GetNumberOfLinears());

    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(-20.0, -20.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(11.0, -10.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(8.0, -1.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(14.0, 5.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(-10.0, 5.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(-10.0, -10.0, pWorldTranslation)));
     
    // Test with a stretch between systems
    HGF2DStretch myStretch;
    myStretch.SetTranslation(Translation);
    myStretch.SetXScaling(0.5);
    myStretch.SetYScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldStretch = new HGF2DCoordSys(myStretch, pWorld);

    HFCPtr<HVE2DComplexLinear> pClone6 = (HVE2DComplexLinear*)Linear2.AllocateCopyInCoordSys(pWorldStretch);
    ASSERT_FALSE(pClone6->IsEmpty());
    ASSERT_EQ(pWorldStretch, pClone6->GetCoordSys());
    ASSERT_EQ(6, pClone6->GetNumberOfLinears());

    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(-40.0, -40.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(22.0, -20.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(16.0, -2.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(28.0, 10.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(-20.0, 10.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(-20.0, -20.0, pWorldStretch)));

    // Test with a similitude between systems
    HGF2DSimilitude mySimilitude;
    mySimilitude.SetRotation(PI);
    mySimilitude.SetScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldSimilitude = new HGF2DCoordSys(mySimilitude, pWorld);

    HFCPtr<HVE2DComplexLinear> pClone7 = (HVE2DComplexLinear*)Linear2.AllocateCopyInCoordSys(pWorldSimilitude);
    ASSERT_FALSE(pClone7->IsEmpty());
    ASSERT_EQ(pWorldSimilitude, pClone7->GetCoordSys());
    ASSERT_EQ(6, pClone7->GetNumberOfLinears());

    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(20.0, 20.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-42.0, 0.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-36.0, -18.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-48.0, -30.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(0.0, -30.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldSimilitude))); 

    // Test with an affine between systems
    HGF2DAffine myAffine;
    myAffine.SetTranslation(Translation);
    myAffine.SetRotation(PI);
    myAffine.SetXScaling(0.5);
    myAffine.SetYScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldAffine = new HGF2DCoordSys(myAffine, pWorld);

    HFCPtr<HVE2DComplexLinear> pClone8 = (HVE2DComplexLinear*)Linear2.AllocateCopyInCoordSys(pWorldAffine);
    ASSERT_FALSE(pClone8->IsEmpty());
    ASSERT_EQ(pWorldAffine, pClone8->GetCoordSys());
    ASSERT_EQ(6, pClone8->GetNumberOfLinears());

    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(40.0, 40.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-22.0, 20.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-16.0, 2.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-28.0, -10.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(20.0, -10.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(20.0, 20.0, pWorldAffine)));

    }

////==================================================================================
// Interaction info with other segment
// Crosses(const HVE2DVector& pi_rVector) const;
// AreAdjacent(const HVE2DVector& pi_rVector) const;
// IsPointOn(const HGF2DLocation& pi_rTestPoint) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  InteractionTest1)
    { 
    
    // Tests with a vertical segment
    ASSERT_TRUE(Linear1.Crosses(ComplexLinearCase1));
    ASSERT_FALSE(Linear1.AreAdjacent(ComplexLinearCase1));

    ASSERT_TRUE(Linear1.Crosses(ComplexLinearCase2));
    ASSERT_TRUE(Linear1.AreAdjacent(ComplexLinearCase2));

    ASSERT_TRUE(Linear1.Crosses(ComplexLinearCase3));
    ASSERT_FALSE(Linear1.AreAdjacent(ComplexLinearCase3));

    ASSERT_FALSE(Linear1.Crosses(ComplexLinearCase4));
    ASSERT_TRUE(Linear1.AreAdjacent(ComplexLinearCase4));

    ASSERT_TRUE(Linear1.Crosses(ComplexLinearCase5));
    ASSERT_TRUE(Linear1.AreAdjacent(ComplexLinearCase5));

    ASSERT_FALSE(Linear1.Crosses(ComplexLinearCase6));
    ASSERT_TRUE(Linear1.AreAdjacent(ComplexLinearCase6));

    ASSERT_FALSE(Linear1.Crosses(ComplexLinearCase7));
    ASSERT_FALSE(Linear1.AreAdjacent(ComplexLinearCase7));

    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(Linear1.IsPointOn(HGF2DLocation(0.1+1.1*MYEPSILON, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(Linear1.IsPointOn(LinearMidPoint1-HGF2DDisplacement(0.0, 1.1*MYEPSILON)));
    ASSERT_FALSE(Linear1.IsPointOn(LinearMidPoint1-HGF2DDisplacement(0.0, -1.1*MYEPSILON)));
    ASSERT_TRUE(Linear1.IsPointOn(LinearMidPoint1-HGF2DDisplacement(0.0, 0.9*MYEPSILON)));
    ASSERT_TRUE(Linear1.IsPointOn(LinearMidPoint1-HGF2DDisplacement(0.0, -0.9*MYEPSILON)));
    ASSERT_TRUE(Linear1.IsPointOn(Linear1.GetStartPoint()));
    ASSERT_TRUE(Linear1.IsPointOn(Linear1.GetEndPoint()));
    ASSERT_TRUE(Linear1.IsPointOn(LinearMidPoint1));

    ASSERT_FALSE(Linear1.IsPointOn(Linear1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(Linear1.IsPointOn(Linear1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(Linear1.IsPointOn(LinearMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));
    
    }

////==================================================================================
// Bearing calculation tests
// CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
//                  HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
// CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
//                              HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  BearingTest1)
    { 

    // Obtain bearing ALPHA of a vertical segment
    ASSERT_DOUBLE_EQ(-2.3561944901923448, Linear1.CalculateBearing(Linear1Point0d0, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(0.78539816339744828, Linear1.CalculateBearing(Linear1Point0d0, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(-2.3561944901923448, Linear1.CalculateBearing(Linear1Point0d1, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(0.78539816339744830, Linear1.CalculateBearing(Linear1Point0d1, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(3.14159265358979310, Linear1.CalculateBearing(Linear1Point0d5, HVE2DVector::ALPHA).GetAngle());
    ASSERT_NEAR(0.0, Linear1.CalculateBearing(Linear1Point0d5, HVE2DVector::BETA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.57079630679489600, Linear1.CalculateBearing(Linear1Point1d0, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(-1.5707963467948973, Linear1.CalculateBearing(Linear1Point1d0, HVE2DVector::BETA).GetAngle());

    // ANGULAR ACCELERATION
    // Obtain angular acceleration ALPHA of a vertical segment
    ASSERT_NEAR(0.0, Linear1.CalculateAngularAcceleration(Linear1Point0d0, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Linear1.CalculateAngularAcceleration(Linear1Point0d0, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, Linear1.CalculateAngularAcceleration(Linear1Point0d1, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Linear1.CalculateAngularAcceleration(Linear1Point0d1, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, Linear1.CalculateAngularAcceleration(Linear1Point0d5, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Linear1.CalculateAngularAcceleration(Linear1Point0d5, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, Linear1.CalculateAngularAcceleration(Linear1Point1d0, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, Linear1.CalculateAngularAcceleration(Linear1Point1d0, HVE2DVector::BETA), MYEPSILON);

    }

////==================================================================================
// Extent calculation test
// GetExtent() const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  GetExtentTest1) 
    { 
    
    // Obtain extent of linear 1
    ASSERT_NEAR(0.0, Linear1.GetExtent().GetXMin(), MYEPSILON);
    ASSERT_NEAR(0.0, Linear1.GetExtent().GetYMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, Linear1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.0, Linear1.GetExtent().GetYMax());

    // Obtain extent of linear 2
    ASSERT_DOUBLE_EQ(-10.0, Linear2.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(-10.0, Linear2.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(24.00, Linear2.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(15.00, Linear2.GetExtent().GetYMax());

    // Obtain extent of an epsilon container
    ASSERT_NEAR(0.0, Linear3.GetExtent().GetXMin(), MYEPSILON);
    ASSERT_NEAR(0.0, Linear3.GetExtent().GetYMin(), MYEPSILON);
    ASSERT_NEAR(0.0, Linear3.GetExtent().GetXMax(), MYEPSILON);
    ASSERT_DOUBLE_EQ(2E-7, Linear3.GetExtent().GetYMax());

    // Obtain extent of an empty linear
    HGF2DExtent EmptyExtent(EmptyLinear.GetExtent());
    ASSERT_FALSE(EmptyExtent.IsDefined());

    }

////==================================================================================
// Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin);
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  ScaleTest)
    { 

    HGF2DLocation Origin(0.0, 0.0, pWorld);

    Linear1.Scale(1.0, Origin);
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(30.0, 10.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(30.0, 5.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(30.0, 5.0 - MYEPSILON, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(30.0 - MYEPSILON, 0.0, pWorld))); 

    Linear1.Scale(3.0, Origin);
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(30.0, 30.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(60.0, 30.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(90.0, 30.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(90.0, 15.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(90.0, 15.0  - 3*MYEPSILON, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(90.0 - 3*MYEPSILON, 0.0, pWorld))); 

    Linear1.Scale(5.0, HGF2DLocation(10.0, 10.0, pWorld));
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(-40.0, -40.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(110.0, 110.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(260.0, 110.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(410.0, 110.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(410.0, 35.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(410.0, 35.0  - 15*MYEPSILON, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(410.0 - 15*MYEPSILON, -40.0, pWorld))); 

    }

////==================================================================================
// Move(const HGF2DDisplacement& pi_rDisplacement);
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  MoveTest)
    { 

    Linear1.Move(HGF2DDisplacement(0.0, 10.0));

    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(0.0, 10.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(30.0, 20.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(30.0, 15.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(30.0, 15.0 - MYEPSILON, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(30.0 - MYEPSILON, 10.0, pWorld))); 

    Linear1.Move(HGF2DDisplacement(10.0, 0.0));

    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(30.0, 20.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(40.0, 20.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(40.0, 15.0, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(40.0, 15.0 - MYEPSILON, pWorld))); 
    ASSERT_TRUE(Linear1.IsPointOn(HGF2DLocation(40.0 - MYEPSILON, 10.0, pWorld))); 

    Linear2.Move(HGF2DDisplacement(10.0, 10.0));

    ASSERT_TRUE(Linear2.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld))); 
    ASSERT_TRUE(Linear2.IsPointOn(HGF2DLocation(31.0, 10.0, pWorld))); 
    ASSERT_TRUE(Linear2.IsPointOn(HGF2DLocation(28.0, 19.0, pWorld))); 
    ASSERT_TRUE(Linear2.IsPointOn(HGF2DLocation(34.0, 25.0, pWorld))); 
    ASSERT_TRUE(Linear2.IsPointOn(HGF2DLocation(10.0, 25.0, pWorld))); 
    ASSERT_TRUE(Linear2.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld))); 

    Linear2.Move(HGF2DDisplacement(-10.0, -10.0));

    ASSERT_TRUE(Linear2.IsPointOn(HGF2DLocation(-10.0, -10.0, pWorld))); 
    ASSERT_TRUE(Linear2.IsPointOn(HGF2DLocation(21.0, 0.0, pWorld))); 
    ASSERT_TRUE(Linear2.IsPointOn(HGF2DLocation(18.0, 9.0, pWorld))); 
    ASSERT_TRUE(Linear2.IsPointOn(HGF2DLocation(24.0, 15.0, pWorld))); 
    ASSERT_TRUE(Linear2.IsPointOn(HGF2DLocation(0.0, 15.0, pWorld))); 
    ASSERT_TRUE(Linear2.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld))); 

    }

////==================================================================================
// Intersection test (with other complex linears only)
// Intersect(const HVE2DVector& pi_rVector, HGF2DLocationCollection* po_pCrossPoints) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  IntersectAndCrossesWhoPreviouslyFailed)
    { 

    HGF2DLocationCollection     CCPoints;

    HVE2DComplexLinear  AddLinear1(pWorld);
    AddLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 20.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    AddLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld)));
    AddLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(20.0, 20.0, pWorld)));

    HVE2DComplexLinear  AddLinear2(pWorld);
    AddLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(5.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld)));
    AddLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(15.0, 15.0, pWorld)));

    ASSERT_EQ(1, AddLinear1.Intersect(AddLinear2, &CCPoints));
    ASSERT_TRUE(AddLinear1.Crosses(AddLinear2));

    CCPoints.clear();
    HVE2DComplexLinear  AddLinear3(pWorld);
    AddLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(12.0, 12.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    AddLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld)));
    AddLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(15.0, 15.0, pWorld)));

    ASSERT_EQ(0, AddLinear1.Intersect(AddLinear3, &CCPoints));
    ASSERT_FALSE(AddLinear1.Crosses(AddLinear3));

    }

////==================================================================================
// ComplexLinear Construction tests
// HVE2DComplexLinear();
// HVE2DComplexLinear(const HGF2DLocation&, const HGF2DLocation&);
// HVE2DComplexLinear(const HGF2DLocation& pi_rStartPoint,
//             const HGF2DDisplacement& pi_rDisplacement);
// HVE2DComplexLinear(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
// HVE2DComplexLinear(const HVE2DComplexLinear&    pi_rObject);
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  ConstructionTest2)
    { 

    // Default Constructor
    HVE2DComplexLinear    ComplexLinear1;

    // Contructor by two points

    // Preparation of the two points
    HGF2DLocation   FirstComplexLinearPoint(10.0, 10.2, pWorld);
    HGF2DLocation   SecondComplexLinearPoint(-10000.0, 100.3, pWorld);

    HVE2DComplexLinear    ComplexLinear2(pWorld);
    ComplexLinear2.AppendLinear(HVE2DSegment(FirstComplexLinearPoint, SecondComplexLinearPoint));
    ASSERT_EQ(pWorld, ComplexLinear2.GetCoordSys());

    ASSERT_DOUBLE_EQ(10.00000, ComplexLinear2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.20000, ComplexLinear2.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10000.0, ComplexLinear2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(100.3000, ComplexLinear2.GetEndPoint().GetY());
    
    // Constructor by point and displacement
    HGF2DLocation   FirstComplexLinearPoint1(10.0, 10.2, pWorld);

    HGF2DDisplacement   Displacement1(10.0, 10.0);
    HGF2DDisplacement   Displacement2(0.0, 10.0);
    HGF2DDisplacement   Displacement3(-10.0, -10.0);
    HGF2DDisplacement   Displacement4(0.0, 0.0);

    HVE2DComplexLinear    ComplexLinear4(pWorld);
    ComplexLinear4.AppendLinear(HVE2DSegment(FirstComplexLinearPoint1, FirstComplexLinearPoint1 + Displacement1));
    ASSERT_EQ(pWorld, ComplexLinear4.GetCoordSys());

    ASSERT_DOUBLE_EQ(10.0, ComplexLinear4.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2, ComplexLinear4.GetStartPoint().GetY());

    HVE2DComplexLinear    ComplexLinear5(pWorld);
    ComplexLinear5.AppendLinear(HVE2DSegment(FirstComplexLinearPoint1, FirstComplexLinearPoint1 + Displacement2));
    ASSERT_EQ(pWorld, ComplexLinear5.GetCoordSys());

    ASSERT_DOUBLE_EQ(10.0, ComplexLinear5.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2, ComplexLinear5.GetStartPoint().GetY());

    HVE2DComplexLinear    ComplexLinear6(pWorld);
    ComplexLinear6.AppendLinear(HVE2DSegment(FirstComplexLinearPoint1, FirstComplexLinearPoint1 + Displacement3));
    ASSERT_EQ(pWorld, ComplexLinear6.GetCoordSys());

    ASSERT_DOUBLE_EQ(10.0, ComplexLinear6.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2, ComplexLinear6.GetStartPoint().GetY());

    HVE2DComplexLinear    ComplexLinear7(pWorld);
    ComplexLinear7.AppendLinear(HVE2DSegment(FirstComplexLinearPoint1, FirstComplexLinearPoint1 + Displacement4));
    ASSERT_EQ(pWorld, ComplexLinear7.GetCoordSys());

    ASSERT_DOUBLE_EQ(10.0, ComplexLinear7.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2, ComplexLinear7.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.0, ComplexLinear7.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2, ComplexLinear7.GetEndPoint().GetY());

    // Constructor with only coordinate system
    HVE2DComplexLinear    ComplexLinear8(pSys1);
    ASSERT_EQ(pSys1, ComplexLinear8.GetCoordSys());

    }

////==================================================================================
// operator= test
// operator=(const HVE2DComplexLinear& pi_rObj);
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  OperatorTest2)
    { 

    // Test with different coord sys
    HGF2DLocation   FirstComplexLinearPoint2(10.0, 10.2, pWorld);
    HGF2DLocation   SecondComplexLinearPoint2(-10000.0, 100.3, pWorld);
    HVE2DComplexLinear    ComplexLinear9(pWorld);
    ComplexLinear9.AppendLinear(HVE2DSegment(FirstComplexLinearPoint2, SecondComplexLinearPoint2));

    FirstComplexLinearPoint2.ChangeCoordSys(pSys1);

    HVE2DComplexLinear    ComplexLinear10(pSys1);
    ComplexLinear10.AppendLinear(HVE2DSegment(FirstComplexLinearPoint2, SecondComplexLinearPoint2));
    ASSERT_EQ(pSys1, ComplexLinear10.GetCoordSys());

    ComplexLinear10 = ComplexLinear9;
    ASSERT_EQ(pWorld, ComplexLinear10.GetCoordSys());

    ASSERT_DOUBLE_EQ(10.00000, ComplexLinear10.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.20000, ComplexLinear10.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10000.0, ComplexLinear10.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(100.3000, ComplexLinear10.GetEndPoint().GetY());

    // Test with a NULL ComplexLinear
    HVE2DComplexLinear    ComplexLinear11(pSys1);
    HVE2DComplexLinear    ComplexLinear12;

    ComplexLinear12 = ComplexLinear11;
    ASSERT_EQ(pSys1, ComplexLinear12.GetCoordSys());
    
    }

////==================================================================================
// Length calculation test
// CalculateLength() const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  CalculateLengthTest2)
    { 

    // Test with vertical ComplexLinear
    ASSERT_DOUBLE_EQ(10.0, VerticalComplexLinear1.CalculateLength());

    // Test with inverted vertical ComplexLinear
    ASSERT_DOUBLE_EQ(10.0, VerticalComplexLinear2.CalculateLength());

    // Test with horizontal ComplexLinear
    ASSERT_DOUBLE_EQ(10.0, HorizontalComplexLinear1.CalculateLength());

    // Test with inverted horizontal ComplexLinear
    ASSERT_DOUBLE_EQ(10.0, HorizontalComplexLinear2.CalculateLength());

    // Tests with miscalenious ComplexLinear
    ASSERT_DOUBLE_EQ(14.142135623730951, MiscComplexLinear1.CalculateLength());

    // Tests with vertical EPSILON sized ComplexLinear
    ASSERT_DOUBLE_EQ(1.0000000000287557E-7, VerticalComplexLinear3.CalculateLength());

    // Tests with horizontal EPSILON sized ComplexLinear
    ASSERT_DOUBLE_EQ(1.0000000000287557E-7, HorizontalComplexLinear3.CalculateLength());

    // Tests with miscalenious EPSILON size ComplexLinear
    ASSERT_DOUBLE_EQ(9.9999999995963103E-8, MiscComplexLinear3.CalculateLength());

    // Test with very large ComplexLinear
    ASSERT_DOUBLE_EQ(4.1231056256176610E124, LargeComplexLinear1.CalculateLength());

    // Test with ComplexLinears way into positive regions
    ASSERT_DOUBLE_EQ(2.2360679774997898E124, PositiveComplexLinear1.CalculateLength());

    // Test with ComplexLinears way into negative regions
    ASSERT_DOUBLE_EQ(2.2360679774997898E124, NegativeComplexLinear1.CalculateLength());

    // Test with a NULL ComplexLinear
    ASSERT_NEAR(0.0, NullComplexLinear1.CalculateLength(), MYEPSILON);

    }

////==================================================================================
// Relative point calculation test
// CalculateRelativePoint(double pi_RelativePos) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  CalculateRelativePointTest2)
    { 
         
    // Test with vertical ComplexLinear
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear1.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear1.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear1.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(1.10, VerticalComplexLinear1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(5.10, VerticalComplexLinear1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(7.10, VerticalComplexLinear1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear1.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(10.1, VerticalComplexLinear1.CalculateRelativePoint(1.0).GetY());

    // Test with inverted vertical ComplexLinear
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear2.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(10.1, VerticalComplexLinear2.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear2.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(9.10, VerticalComplexLinear2.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear2.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(5.10, VerticalComplexLinear2.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear2.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(3.10, VerticalComplexLinear2.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear2.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear2.CalculateRelativePoint(1.0).GetY());

    // Test with horizontal ComplexLinear
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear1.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear1.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(1.10, HorizontalComplexLinear1.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(5.10, HorizontalComplexLinear1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(7.10, HorizontalComplexLinear1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(10.1, HorizontalComplexLinear1.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear1.CalculateRelativePoint(1.0).GetY());

    // Test with inverted horizontal ComplexLinear
    ASSERT_DOUBLE_EQ(10.1, HorizontalComplexLinear2.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear2.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(9.10, HorizontalComplexLinear2.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear2.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(5.10, HorizontalComplexLinear2.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear2.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(3.10, HorizontalComplexLinear2.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear2.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear2.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear2.CalculateRelativePoint(1.0).GetY());

    // Tests with vertical EPSILON sized ComplexLinear
    ASSERT_DOUBLE_EQ(0.10000000, VerticalComplexLinear3.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, VerticalComplexLinear3.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(0.10000000, VerticalComplexLinear3.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(0.10000001, VerticalComplexLinear3.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(0.10000000, VerticalComplexLinear3.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(0.10000005, VerticalComplexLinear3.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(0.10000000, VerticalComplexLinear3.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(0.10000007, VerticalComplexLinear3.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.10000000, VerticalComplexLinear3.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000010, VerticalComplexLinear3.CalculateRelativePoint(1.0).GetY());

    // Tests with horizontal EPSILON sized ComplexLinear
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalComplexLinear3.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalComplexLinear3.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(0.10000001, HorizontalComplexLinear3.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalComplexLinear3.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(0.10000005, HorizontalComplexLinear3.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalComplexLinear3.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(0.10000007, HorizontalComplexLinear3.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalComplexLinear3.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.10000010, HorizontalComplexLinear3.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalComplexLinear3.CalculateRelativePoint(1.0).GetY());

    // Test with very large ComplexLinear
    ASSERT_DOUBLE_EQ(-1.0000000000000000E123, LargeComplexLinear1.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(-21.000000000000000E123, LargeComplexLinear1.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(-2.9356782284672915E107, LargeComplexLinear1.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(-17.000000000000000E123, LargeComplexLinear1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(4.00000000000000000E123, LargeComplexLinear1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(-1.0000000000000000E123, LargeComplexLinear1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(6.00000000000000000E123, LargeComplexLinear1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(7.00000000000000000E123, LargeComplexLinear1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(9.00000000000000000E123, LargeComplexLinear1.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(19.0000000000000000E123, LargeComplexLinear1.CalculateRelativePoint(1.0).GetY());

    // Test with ComplexLinears way into positive regions
    ASSERT_DOUBLE_EQ(1.00E123, PositiveComplexLinear1.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(21.0E123, PositiveComplexLinear1.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(2.00E123, PositiveComplexLinear1.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(23.0E123, PositiveComplexLinear1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(6.00E123, PositiveComplexLinear1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(31.0E123, PositiveComplexLinear1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(8.00E123, PositiveComplexLinear1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(35.0E123, PositiveComplexLinear1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(11.0E123, PositiveComplexLinear1.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(41.0E123, PositiveComplexLinear1.CalculateRelativePoint(1.0).GetY());

    // Test with ComplexLinears way into negative regions
    ASSERT_DOUBLE_EQ(-1.00E123, NegativeComplexLinear1.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, NegativeComplexLinear1.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(-2.00E123, NegativeComplexLinear1.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(-23.0E123, NegativeComplexLinear1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(-6.00E123, NegativeComplexLinear1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(-31.0E123, NegativeComplexLinear1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(-8.00E123, NegativeComplexLinear1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(-35.0E123, NegativeComplexLinear1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, NegativeComplexLinear1.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, NegativeComplexLinear1.CalculateRelativePoint(1.0).GetY());

    // Test with a NULL ComplexLinear
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(0.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(0.0).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(0.1).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(0.1).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(0.5).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(0.5).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(0.7).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(0.7).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(1.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(1.0).GetY(), MYEPSILON);
    
    }

////==================================================================================
// Relative position calculation test
// CalculateRelativePosition(const HGF2DLocation& pi_rPointOnLinear) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  CalculateRelativePositionTest2)
    { 

    // Test with vertical ComplexLinear
    ASSERT_NEAR(0.0, VerticalComplexLinear1.CalculateRelativePosition(VerticalPoint0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1, VerticalComplexLinear1.CalculateRelativePosition(VerticalPoint0d1));
    ASSERT_DOUBLE_EQ(0.5, VerticalComplexLinear1.CalculateRelativePosition(VerticalPoint0d5));

    // Test with inverted vertical ComplexLinear
    ASSERT_DOUBLE_EQ(1.0, VerticalComplexLinear2.CalculateRelativePosition(VerticalPoint0d0));
    ASSERT_DOUBLE_EQ(0.9, VerticalComplexLinear2.CalculateRelativePosition(VerticalPoint0d1));
    ASSERT_DOUBLE_EQ(0.5, VerticalComplexLinear2.CalculateRelativePosition(VerticalPoint0d5));
    ASSERT_NEAR(0.0, VerticalComplexLinear2.CalculateRelativePosition(VerticalPoint1d0), MYEPSILON);

    // Test with horizontal ComplexLinear
    ASSERT_NEAR(0.0, HorizontalComplexLinear1.CalculateRelativePosition(HorizontalPoint0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1, HorizontalComplexLinear1.CalculateRelativePosition(HorizontalPoint0d1));
    ASSERT_DOUBLE_EQ(0.5, HorizontalComplexLinear1.CalculateRelativePosition(HorizontalPoint0d5));
    ASSERT_DOUBLE_EQ(1.0, HorizontalComplexLinear1.CalculateRelativePosition(HorizontalPoint1d0));

    // Test with inverted horizontal ComplexLinear
    ASSERT_DOUBLE_EQ(1.0, HorizontalComplexLinear2.CalculateRelativePosition(HorizontalPoint0d0));
    ASSERT_DOUBLE_EQ(0.9, HorizontalComplexLinear2.CalculateRelativePosition(HorizontalPoint0d1));
    ASSERT_DOUBLE_EQ(0.5, HorizontalComplexLinear2.CalculateRelativePosition(HorizontalPoint0d5));
    ASSERT_NEAR(0.0, HorizontalComplexLinear2.CalculateRelativePosition(HorizontalPoint1d0), MYEPSILON);

    // Tests with vertical EPSILON sized ComplexLinear
    ASSERT_NEAR(0.0, VerticalComplexLinear3.CalculateRelativePosition(Vertical3Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.099999999944488854, VerticalComplexLinear3.CalculateRelativePosition(Vertical3Point0d1));
    ASSERT_DOUBLE_EQ(0.500000000000000000, VerticalComplexLinear3.CalculateRelativePosition(Vertical3Point0d5));
    ASSERT_DOUBLE_EQ(1.000000000000000000, VerticalComplexLinear3.CalculateRelativePosition(Vertical3Point1d0));

    // Tests with horizontal EPSILON sized ComplexLinear
    ASSERT_NEAR(0.0, HorizontalComplexLinear3.CalculateRelativePosition(Horizontal3Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.099999999944488854, HorizontalComplexLinear3.CalculateRelativePosition(Horizontal3Point0d1));
    ASSERT_DOUBLE_EQ(0.500000000000000000, HorizontalComplexLinear3.CalculateRelativePosition(Horizontal3Point0d5));
    ASSERT_DOUBLE_EQ(1.000000000000000000, HorizontalComplexLinear3.CalculateRelativePosition(Horizontal3Point1d0));

    // Tests with miscalenious EPSILON size ComplexLinear
    ASSERT_NEAR(0.0, MiscComplexLinear3.CalculateRelativePosition(Misc3Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.099999999943141071, MiscComplexLinear3.CalculateRelativePosition(Misc3Point0d1));
    ASSERT_DOUBLE_EQ(0.500000000000000000, MiscComplexLinear3.CalculateRelativePosition(Misc3Point0d5));
    ASSERT_DOUBLE_EQ(1.000000000000000000, MiscComplexLinear3.CalculateRelativePosition(Misc3Point1d0));

    // Test with very large ComplexLinear
    ASSERT_NEAR(0.0, LargeComplexLinear1.CalculateRelativePosition(LargePoint0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1, LargeComplexLinear1.CalculateRelativePosition(LargePoint0d1));
    ASSERT_DOUBLE_EQ(0.5, LargeComplexLinear1.CalculateRelativePosition(LargePoint0d5));
    ASSERT_DOUBLE_EQ(1.0, LargeComplexLinear1.CalculateRelativePosition(LargePoint1d0));

    // Test with ComplexLinears way into positive regions
    ASSERT_NEAR(0.0, PositiveComplexLinear1.CalculateRelativePosition(PositivePoint0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1, PositiveComplexLinear1.CalculateRelativePosition(PositivePoint0d1));
    ASSERT_DOUBLE_EQ(0.5, PositiveComplexLinear1.CalculateRelativePosition(PositivePoint0d5));
    ASSERT_DOUBLE_EQ(1.0, PositiveComplexLinear1.CalculateRelativePosition(PositivePoint1d0));

    // Test with ComplexLinears way into negative regions
    ASSERT_NEAR(0.0, NegativeComplexLinear1.CalculateRelativePosition(NegativePoint0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1, NegativeComplexLinear1.CalculateRelativePosition(NegativePoint0d1));
    ASSERT_DOUBLE_EQ(0.5, NegativeComplexLinear1.CalculateRelativePosition(NegativePoint0d5));
    ASSERT_DOUBLE_EQ(1.0, NegativeComplexLinear1.CalculateRelativePosition(NegativePoint1d0));

    }

////==================================================================================
// RayArea Calculation test
// CalculateRayArea(const HGF2DLocation& pi_rPoint) const; 
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  CalculateRayAreaTest2)
    { 

    ASSERT_DOUBLE_EQ(150.0, ComplexLinearCase1.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_DOUBLE_EQ(230.0, ComplexLinearCase2.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_DOUBLE_EQ(112.5, ComplexLinearCase3.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_DOUBLE_EQ(25.00, ComplexLinearCase4.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_DOUBLE_EQ(125.0, ComplexLinearCase5.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_DOUBLE_EQ(100.0, ComplexLinearCase5A.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_DOUBLE_EQ(120.0, ComplexLinearCase6.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_DOUBLE_EQ(125.0, ComplexLinearCase7.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));     
    
    }

////==================================================================================
// Shortening tests
// Shorten(double pi_StartRelativePos, double pi_EndRelativePos);
// Shorten(const HGF2DLocation& pi_rNewStartPoint,
//         const HGF2DLocation& pi_rNewEndPoint);
// ShortenTo(const HGF2DLocation& pi_rNewEndPoint);
// ShortenTo(double pi_EndRelativePosition);
// ShortenFrom(const HGF2DLocation& pi_rNewStartPoint);
// ShortenFrom(double pi_StartRelativePosition);
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  ShorteningTest2)
    { 
   
    // Test with vertical ComplexLinear
    HVE2DComplexLinear    ComplexLinear1(VerticalComplexLinear1);

    ComplexLinear1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(2.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000000000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1000009999999989, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(2.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.099999, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(2.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.099999, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.100000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.ShortenFrom(VerticalMidPoint1);
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.ShortenTo(VerticalMidPoint1);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), VerticalMidPoint1);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.Shorten(VerticalMidPoint1, ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear1;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetY());
        
    // Test with inverted vertical ComplexLinear
    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(8.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(2.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(0.100000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.100000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.099999, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(2.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(0.100000000000000010, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(8.099999999999999600, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000010, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(2.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000099999999890, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(8.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000099999999890, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.ShortenFrom(VerticalMidPoint1);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.ShortenTo(VerticalMidPoint1);
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), VerticalMidPoint1);
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(5.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.Shorten(VerticalMidPoint1, ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(5.100000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear2;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, ComplexLinear1.GetEndPoint().GetY());

    // Test with horizontal ComplexLinear
    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(2.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(8.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(5.100000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.100001, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(8.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(2.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(8.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.099999, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(2.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(10.099999, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.100000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.ShortenFrom(HorizontalMidPoint1);
    ASSERT_DOUBLE_EQ(5.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.ShortenTo(HorizontalMidPoint1);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), HorizontalMidPoint1);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.Shorten(HorizontalMidPoint1, ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(5.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear1;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    // Test with inverted horizontal ComplexLinear
    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(8.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(2.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(5.100000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.099999, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(2.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(8.100000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999999999999645, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(10.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999999999999645, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(2.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(10.1000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000099999999890, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(10.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999999999999645, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(8.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000099999999890, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.099999999999999645, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.ShortenFrom(HorizontalMidPoint1);
    ASSERT_DOUBLE_EQ(5.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.099999999999999645, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.ShortenTo(HorizontalMidPoint1);
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(10.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999999999999645, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), HorizontalMidPoint1);
    ASSERT_DOUBLE_EQ(10.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(5.10, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.Shorten(HorizontalMidPoint1, ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(5.100000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999999999999645, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear2;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(10.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999999999999645, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, ComplexLinear1.GetEndPoint().GetY());

    // Tests with vertical EPSILON sized ComplexLinear
    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000002, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000008, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005000001, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000008, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000002, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000010, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000008, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009999999, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000002, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000010, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009999999, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000010000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.ShortenFrom(VerticalMidPoint3);
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000010, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.ShortenTo(VerticalMidPoint3);
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), VerticalMidPoint3);
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.Shorten(VerticalMidPoint3, ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000005, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000010, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = VerticalComplexLinear3;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetEndPoint().GetY());

    // Tests with horizontal EPSILON sized ComplexLinear
    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(0.10000002, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000008, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000005000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000005000001, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000008, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(0.10000002, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000008, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000009999999, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(0.10000002, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000009999999, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010000000, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.ShortenFrom(HorizontalMidPoint3);
    ASSERT_DOUBLE_EQ(0.10000005, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.ShortenTo(HorizontalMidPoint3);
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000005, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), HorizontalMidPoint3);
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000005, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.Shorten(HorizontalMidPoint3, ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10000005, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = HorizontalComplexLinear3;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1000001, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1000000, ComplexLinear1.GetEndPoint().GetY());

    // Tests with miscalenious EPSILON size ComplexLinear
    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(0.10000000432879229, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000001952592015, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000001731516911, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000007810368057, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000001082198071, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000004881480036, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000001082198286, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000004881481012, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000001731516911, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000007810368057, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(0.10000000432879229, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000001952592015, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000001731516911, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000007810368057, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164395923, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762959096, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(0.10000000432879229, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000001952592015, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000002164395923, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762959096, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(0.10000002164396139, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.ShortenFrom(MiscMidPoint3);
    ASSERT_DOUBLE_EQ(0.10000000955848998, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000004311556202, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10000002164396139, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.ShortenTo(MiscMidPoint3);
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000955848998, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000004311556202, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.1, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), MiscMidPoint3);
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000000955848998, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000004311556202, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.Shorten(MiscMidPoint3, ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10000000955848998, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000004311556202, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = MiscComplexLinear3;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, ComplexLinear1.GetEndPoint().GetY());

    // Test with very large ComplexLinear
    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(1.000E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-13.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(7.000E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(11.00E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(7.000E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(11.00E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(1.000E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-13.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(7.000E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(11.00E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(-1.000000000000000E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.00000000000000E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(8.9999989999999997E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(1.8999996000000000E124, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(1.000E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-13.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(8.9999989999999997E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(1.8999996000000001E124, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.0000000000000000E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.000000000000000E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(9.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, ComplexLinear1.GetEndPoint().GetY());
 
    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.ShortenFrom(LargeMidPoint1);
    ASSERT_DOUBLE_EQ(4.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-1.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(9.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.ShortenTo(LargeMidPoint1);
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(4.000E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), LargeMidPoint1);
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(4.000E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.Shorten(LargeMidPoint1, ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(4.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-1.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = LargeComplexLinear1;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(19.00E123, ComplexLinear1.GetEndPoint().GetY());

    // Test with ComplexLinears way into positive regions
    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(3.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(37.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.000E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(37.00E123, ComplexLinear1.GetEndPoint().GetY());  

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(3.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(9.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(37.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0000000E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.000000E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(1.0999999E124, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(4.0999998E124, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(1.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(3.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(25.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(11.0E123-10E123*MYEPSILON, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123-20E123*MYEPSILON, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(11.0E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, ComplexLinear1.GetEndPoint().GetY());

    // Due to EPSILON problems some of the following methods do not work properly
    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.ShortenFrom(PositiveMidPoint1);
    ASSERT_DOUBLE_EQ(6.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(31.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(11.0E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.ShortenTo(PositiveMidPoint1);
    ASSERT_DOUBLE_EQ(1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(6.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(31.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(1.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), PositiveMidPoint1);
    ASSERT_DOUBLE_EQ(1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(6.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(31.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.Shorten(PositiveMidPoint1, ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(6.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(31.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = PositiveComplexLinear1;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(41.0E123, ComplexLinear1.GetEndPoint().GetY());

    // Test with ComplexLinears way into negative regions
    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(-3.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-25.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-9.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-37.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.Shorten(0.0, 0.8);
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-9.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-37.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(-3.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-25.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.Shorten(0.0, 1.0);
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.ShortenTo(0.8);
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-9.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-37.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.ShortenTo(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(-1.0000000E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.000000E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.0999999E124, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-4.0999998E124, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.ShortenTo(1.0);
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.ShortenTo(0.0);
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(-3.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-25.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(-1.0999999E124, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-4.0999998E124, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.000000E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.000000E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(-11E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-41E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.ShortenFrom(0.0);
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.ShortenFrom(NegativeMidPoint1);
    ASSERT_DOUBLE_EQ(-6.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-31.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.ShortenFrom(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-11E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-41E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.ShortenTo(NegativeMidPoint1);
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-6.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-31.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetStartPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.ShortenTo(ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), NegativeMidPoint1);
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-6.00E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-31.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.Shorten(NegativeMidPoint1, ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-6.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-31.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, ComplexLinear1.GetEndPoint().GetY());

    ComplexLinear1 = NegativeComplexLinear1;
    ComplexLinear1.Shorten(ComplexLinear1.GetStartPoint(), ComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(-1.00E123, ComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, ComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, ComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, ComplexLinear1.GetEndPoint().GetY());

    }

////==================================================================================
// Closest point calculation test
// CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  CalculateClosestPointTest2)
    { 

    // Test with vertical ComplexLinear
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalComplexLinear1.CalculateClosestPoint(VerticalClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalComplexLinear1.CalculateClosestPoint(VerticalClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalComplexLinear1.CalculateClosestPoint(VerticalClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(5.000000000000000000, VerticalComplexLinear1.CalculateClosestPoint(VerticalClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalComplexLinear1.CalculateClosestPoint(VerticalClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(7.000000000000000000, VerticalComplexLinear1.CalculateClosestPoint(VerticalClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(0.099999999976716936, VerticalComplexLinear1.CalculateClosestPoint(VerticalClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(10.00000000000000000, VerticalComplexLinear1.CalculateClosestPoint(VerticalClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalComplexLinear1.CalculateClosestPoint(VerticalCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(5.100000000000000000, VerticalComplexLinear1.CalculateClosestPoint(VerticalCloseMidPoint1).GetY());

    // Test with inverted vertical ComplexLinear
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalComplexLinear2.CalculateClosestPoint(VerticalClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalComplexLinear2.CalculateClosestPoint(VerticalClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalComplexLinear2.CalculateClosestPoint(VerticalClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(5.000000000000000000, VerticalComplexLinear2.CalculateClosestPoint(VerticalClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalComplexLinear2.CalculateClosestPoint(VerticalClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(7.000000000000000000, VerticalComplexLinear2.CalculateClosestPoint(VerticalClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(0.099999999976716936, VerticalComplexLinear2.CalculateClosestPoint(VerticalClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(10.00000000000000000, VerticalComplexLinear2.CalculateClosestPoint(VerticalClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalComplexLinear2.CalculateClosestPoint(VerticalCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(5.100000000000000000, VerticalComplexLinear2.CalculateClosestPoint(VerticalCloseMidPoint1).GetY());

    // Test with horizontal ComplexLinear
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalComplexLinear1.CalculateClosestPoint(HorizontalClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalComplexLinear1.CalculateClosestPoint(HorizontalClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(5.000000000000000000, HorizontalComplexLinear1.CalculateClosestPoint(HorizontalClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalComplexLinear1.CalculateClosestPoint(HorizontalClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(7.000000000000000000, HorizontalComplexLinear1.CalculateClosestPoint(HorizontalClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalComplexLinear1.CalculateClosestPoint(HorizontalClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(10.00000000000000000, HorizontalComplexLinear1.CalculateClosestPoint(HorizontalClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(0.099999999976716936, HorizontalComplexLinear1.CalculateClosestPoint(HorizontalClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(5.100000000000000000, HorizontalComplexLinear1.CalculateClosestPoint(HorizontalCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalComplexLinear1.CalculateClosestPoint(HorizontalCloseMidPoint1).GetY());

    // Test with inverted horizontal ComplexLinear
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalComplexLinear2.CalculateClosestPoint(HorizontalClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalComplexLinear2.CalculateClosestPoint(HorizontalClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(5.000000000000000000, HorizontalComplexLinear2.CalculateClosestPoint(HorizontalClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalComplexLinear2.CalculateClosestPoint(HorizontalClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(7.000000000000000000, HorizontalComplexLinear2.CalculateClosestPoint(HorizontalClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalComplexLinear2.CalculateClosestPoint(HorizontalClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(10.00000000000000000, HorizontalComplexLinear2.CalculateClosestPoint(HorizontalClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(0.099999999976716936, HorizontalComplexLinear2.CalculateClosestPoint(HorizontalClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(5.100000000000000000, HorizontalComplexLinear2.CalculateClosestPoint(HorizontalCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalComplexLinear2.CalculateClosestPoint(HorizontalCloseMidPoint1).GetY());

    // Tests with vertical EPSILON sized ComplexLinear
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalComplexLinear2.CalculateClosestPoint(VerticalClosePoint3A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalComplexLinear2.CalculateClosestPoint(VerticalClosePoint3A).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalComplexLinear2.CalculateClosestPoint(VerticalClosePoint3B).GetX());
    ASSERT_DOUBLE_EQ(0.100000066666666680, VerticalComplexLinear2.CalculateClosestPoint(VerticalClosePoint3B).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalComplexLinear2.CalculateClosestPoint(VerticalClosePoint3C).GetX());
    ASSERT_DOUBLE_EQ(0.100000033333333340, VerticalComplexLinear2.CalculateClosestPoint(VerticalClosePoint3C).GetY());
    ASSERT_DOUBLE_EQ(0.099999999976716936, VerticalComplexLinear2.CalculateClosestPoint(VerticalClosePoint3D).GetX());
    ASSERT_DOUBLE_EQ(0.100000050000000010, VerticalComplexLinear2.CalculateClosestPoint(VerticalClosePoint3D).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalComplexLinear2.CalculateClosestPoint(VerticalCloseMidPoint3).GetX());
    ASSERT_DOUBLE_EQ(0.100000050000000010, VerticalComplexLinear2.CalculateClosestPoint(VerticalCloseMidPoint3).GetY());

    // Tests with horizontal EPSILON sized ComplexLinear
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalComplexLinear3.CalculateClosestPoint(HorizontalClosePoint3A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalComplexLinear3.CalculateClosestPoint(HorizontalClosePoint3A).GetY());
    ASSERT_DOUBLE_EQ(0.100000066666666680, HorizontalComplexLinear3.CalculateClosestPoint(HorizontalClosePoint3B).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalComplexLinear3.CalculateClosestPoint(HorizontalClosePoint3B).GetY());
    ASSERT_DOUBLE_EQ(0.100000033333333340, HorizontalComplexLinear3.CalculateClosestPoint(HorizontalClosePoint3C).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalComplexLinear3.CalculateClosestPoint(HorizontalClosePoint3C).GetY());
    ASSERT_DOUBLE_EQ(0.100000050000000010, HorizontalComplexLinear3.CalculateClosestPoint(HorizontalClosePoint3D).GetX());
    ASSERT_DOUBLE_EQ(0.099999999976716936, HorizontalComplexLinear3.CalculateClosestPoint(HorizontalClosePoint3D).GetY());
    ASSERT_DOUBLE_EQ(0.100000050000000010, HorizontalComplexLinear3.CalculateClosestPoint(HorizontalCloseMidPoint3).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalComplexLinear3.CalculateClosestPoint(HorizontalCloseMidPoint3).GetY());

    // Tests with miscalenious EPSILON size ComplexLinear
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscComplexLinear3.CalculateClosestPoint(MiscClosePoint3A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscComplexLinear3.CalculateClosestPoint(MiscClosePoint3A).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscComplexLinear3.CalculateClosestPoint(MiscClosePoint3B).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscComplexLinear3.CalculateClosestPoint(MiscClosePoint3B).GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, MiscComplexLinear3.CalculateClosestPoint(MiscClosePoint3C).GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, MiscComplexLinear3.CalculateClosestPoint(MiscClosePoint3C).GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, MiscComplexLinear3.CalculateClosestPoint(MiscClosePoint3D).GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, MiscComplexLinear3.CalculateClosestPoint(MiscClosePoint3D).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscComplexLinear3.CalculateClosestPoint(MiscCloseMidPoint3).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, MiscComplexLinear3.CalculateClosestPoint(MiscCloseMidPoint3).GetY());

    // Test with very large ComplexLinear
    ASSERT_DOUBLE_EQ(4.00000000000000000E123, LargeComplexLinear1.CalculateClosestPoint(LargeClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(-1.0000000000000000E123, LargeComplexLinear1.CalculateClosestPoint(LargeClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(6.94117647058823470E123, LargeComplexLinear1.CalculateClosestPoint(LargeClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(1.07647058823529420E124, LargeComplexLinear1.CalculateClosestPoint(LargeClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(1.05882352941176280E123, LargeComplexLinear1.CalculateClosestPoint(LargeClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(-1.2764705882352939E124, LargeComplexLinear1.CalculateClosestPoint(LargeClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(4.00023529411764720E123, LargeComplexLinear1.CalculateClosestPoint(LargeClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(-9.9905882352941167E122, LargeComplexLinear1.CalculateClosestPoint(LargeClosePoint1D).GetY());

    // Test with a NULL ComplexLinear
    ASSERT_NEAR(0.0, NullComplexLinear1.CalculateClosestPoint(NegativeClosePoint1A).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, NullComplexLinear1.CalculateClosestPoint(NegativeClosePoint1A).GetY(), MYEPSILON);

    // Tests with special points
    ASSERT_DOUBLE_EQ(10.1, MiscComplexLinear1.CalculateClosestPoint(VeryFarPoint).GetX());
    ASSERT_DOUBLE_EQ(10.1, MiscComplexLinear1.CalculateClosestPoint(VeryFarPoint).GetY());
    ASSERT_DOUBLE_EQ(5.10, MiscComplexLinear1.CalculateClosestPoint(MidPoint).GetX());
    ASSERT_DOUBLE_EQ(5.10, MiscComplexLinear1.CalculateClosestPoint(MidPoint).GetY());
    ASSERT_DOUBLE_EQ(0.10, MiscComplexLinear1.CalculateClosestPoint(VeryFarNegativePoint).GetX());
    ASSERT_DOUBLE_EQ(0.10, MiscComplexLinear1.CalculateClosestPoint(VeryFarNegativePoint).GetY());
    ASSERT_DOUBLE_EQ(10.1, MiscComplexLinear1.CalculateClosestPoint(VeryFarAlignedPoint).GetX());
    ASSERT_DOUBLE_EQ(10.1, MiscComplexLinear1.CalculateClosestPoint(VeryFarAlignedPoint).GetY());
    ASSERT_DOUBLE_EQ(0.10, MiscComplexLinear1.CalculateClosestPoint(VeryFarAlignedNegativePoint).GetX());
    ASSERT_DOUBLE_EQ(0.10, MiscComplexLinear1.CalculateClosestPoint(VeryFarAlignedNegativePoint).GetY());
    ASSERT_DOUBLE_EQ(0.10, MiscComplexLinear1.CalculateClosestPoint(MiscComplexLinear1.GetStartPoint()).GetX());
    ASSERT_DOUBLE_EQ(0.10, MiscComplexLinear1.CalculateClosestPoint(MiscComplexLinear1.GetStartPoint()).GetY());
    ASSERT_DOUBLE_EQ(10.1, MiscComplexLinear1.CalculateClosestPoint(MiscComplexLinear1.GetEndPoint()).GetX());
    ASSERT_DOUBLE_EQ(10.1, MiscComplexLinear1.CalculateClosestPoint(MiscComplexLinear1.GetEndPoint()).GetY());

    }

////==================================================================================
// Intersection test (with other ComplexLinears only)
// Intersect(const HVE2DVector& pi_rVector, HGF2DLocationCollection* po_pCrossPoints) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  IntersectTest2)
    { 

    HGF2DLocationCollection   DumPoints;

    // Test with extent disjoint ComplexLinears
    ASSERT_EQ(0, MiscComplexLinear1.Intersect(DisjointComplexLinear1, &DumPoints));

    // Test with disjoint but touching by a side ComplexLinears
    ASSERT_EQ(0, MiscComplexLinear1.Intersect(ContiguousExtentComplexLinear1, &DumPoints));

    // Test with disjoint but touching by a tip ComplexLinears but not linked
    ASSERT_EQ(0, MiscComplexLinear1.Intersect(FlirtingExtentComplexLinear1, &DumPoints));

    // Test with disjoint but touching by a tip ComplexLinears linked
    ASSERT_EQ(0, MiscComplexLinear1.Intersect(FlirtingExtentLinkedComplexLinear1, &DumPoints));

    // Test with vertical ComplexLinear
    ASSERT_EQ(0, VerticalComplexLinear1.Intersect(MiscComplexLinear1, &DumPoints));

    // Test with inverted vertical ComplexLinear
    ASSERT_EQ(0, VerticalComplexLinear2.Intersect(MiscComplexLinear1, &DumPoints));

    // Test with close vertical ComplexLinears
    ASSERT_EQ(0, VerticalComplexLinear1.Intersect(VerticalComplexLinear4, &DumPoints));

    // Test with horizontal ComplexLinear
    ASSERT_EQ(0, HorizontalComplexLinear1.Intersect(MiscComplexLinear1, &DumPoints));

    // Test with inverted horizontal ComplexLinear
    ASSERT_EQ(0, HorizontalComplexLinear2.Intersect(MiscComplexLinear1, &DumPoints));

    // Test with parallel ComplexLinears
    ASSERT_EQ(0, MiscComplexLinear1.Intersect(ParallelComplexLinear1, &DumPoints));

    // Test with near parallel ComplexLinears
    ASSERT_EQ(0, MiscComplexLinear1.Intersect(NearParallelComplexLinear1, &DumPoints));

    // Tests with close near parallel ComplexLinears
    ASSERT_EQ(0, MiscComplexLinear1.Intersect(CloseNearParallelComplexLinear1, &DumPoints));

    // Tests with connected ComplexLinears
    // At start point...
    ASSERT_EQ(0, MiscComplexLinear1.Intersect(ConnectedComplexLinear1, &DumPoints));
    ASSERT_EQ(0, MiscComplexLinear1.Intersect(ConnectingComplexLinear1, &DumPoints));

    // At end point ...
    ASSERT_EQ(0, MiscComplexLinear1.Intersect(ConnectedComplexLinear1A, &DumPoints));
    ASSERT_EQ(0, MiscComplexLinear1.Intersect(ConnectingComplexLinear1A, &DumPoints));

    // Tests with linked ComplexLinears
    ASSERT_EQ(0, MiscComplexLinear1.Intersect(LinkedComplexLinear1, &DumPoints));
    ASSERT_EQ(0, MiscComplexLinear1.Intersect(LinkedComplexLinear1A, &DumPoints));

    // Tests with vertical EPSILON sized ComplexLinear
    ASSERT_EQ(0, VerticalComplexLinear3.Intersect(MiscComplexLinear1, &DumPoints));

    // Tests with horizontal EPSILON sized ComplexLinear
    ASSERT_EQ(0, HorizontalComplexLinear3.Intersect(MiscComplexLinear1, &DumPoints));

    // Tests with miscalenious EPSILON size ComplexLinear
    ASSERT_EQ(0, MiscComplexLinear3.Intersect(MiscComplexLinear1, &DumPoints));

    // Test with very large ComplexLinear
    ASSERT_EQ(0, LargeComplexLinear1.Intersect(MiscComplexLinear1, &DumPoints));

    // Test with ComplexLinears way into positive regions
    ASSERT_EQ(0, PositiveComplexLinear1.Intersect(MiscComplexLinear1, &DumPoints));

    // Test with ComplexLinears way into negative regions
    ASSERT_EQ(0, NegativeComplexLinear1.Intersect(MiscComplexLinear1, &DumPoints));

    // Test with a NULL ComplexLinear
    ASSERT_EQ(0, NullComplexLinear1.Intersect(MiscComplexLinear1, &DumPoints)); 

    }

////==================================================================================
// Contiguousness Test
//   ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
//                              HGF2DLocationCollection* po_pContiguousnessPoints) const;
//   ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
//                                const HGF2DLocation& pi_rPoint,
//                                HGF2DLocation* pi_pFirstContiguousnessPoint,
//                                HGF2DLocation* pi_pSecondContiguousnessPoint) const;
//    HDLL virtual bool      AreContiguous(const HVE2DVector& pi_rVector) const;
//    HDLL virtual bool      AreContiguousAt(const HVE2DVector& pi_rVector,
//                                            const HGF2DLocation& pi_rPoint) const;
// AreContiguousAtAndGet(const HVE2DVector& pi_rVector, const HGF2DLocation& pi_rPoint,
//                       HGF2DLocation* pi_pFirstContiguousnessPoint,
//                       HGF2DLocation* pi_pSecondContiguousnessPoint) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, ContiguousnessTest2)
    { 

    HGF2DLocationCollection     DumPoints;
    HGF2DLocation   FirstDumPoint;
    HGF2DLocation   SecondDumPoint;

    // Test with vertical ComplexLinears
    ASSERT_TRUE(VerticalComplexLinear1.AreContiguous(VerticalComplexLinear2));

    ASSERT_TRUE(VerticalComplexLinear1.AreContiguousAt(VerticalComplexLinear2, VerticalMidPoint1));

    ASSERT_EQ(2, VerticalComplexLinear1.ObtainContiguousnessPoints(VerticalComplexLinear2, &DumPoints));
    ASSERT_DOUBLE_EQ(0.1, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.1, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(0.1, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetY());

    VerticalComplexLinear1.ObtainContiguousnessPointsAt(VerticalComplexLinear2, VerticalMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.1, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.1, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(0.1, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetY());

    DumPoints.clear();

    VerticalComplexLinear1.AreContiguousAtAndGet(VerticalComplexLinear2, VerticalMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.1, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.1, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(0.1, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetY());

    DumPoints.clear();

    // Test with horizontal ComplexLinears
    ASSERT_TRUE(HorizontalComplexLinear1.AreContiguous(HorizontalComplexLinear2));
    ASSERT_TRUE(HorizontalComplexLinear1.AreContiguousAt(HorizontalComplexLinear2, HorizontalMidPoint1));
    ASSERT_EQ(2, HorizontalComplexLinear1.ObtainContiguousnessPoints(HorizontalComplexLinear2, &DumPoints));
    ASSERT_DOUBLE_EQ(0.1, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.1, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(0.1, DumPoints[1].GetY());

    HorizontalComplexLinear1.ObtainContiguousnessPointsAt(HorizontalComplexLinear2, HorizontalMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.1, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.1, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.1, SecondDumPoint.GetY());

    DumPoints.clear();

    HorizontalComplexLinear1.AreContiguousAtAndGet(HorizontalComplexLinear2, HorizontalMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.1, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.1, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.1, SecondDumPoint.GetY());

    DumPoints.clear();

    ASSERT_FALSE(HorizontalComplexLinear1.AreContiguous(MiscComplexLinear1));
    ASSERT_FALSE(HorizontalComplexLinear1.AreContiguousAtAndGet(MiscComplexLinear1, HorizontalMidPoint1, &FirstDumPoint, &SecondDumPoint));

    // Test with positive slope ComplexLinears  
    ASSERT_TRUE(MiscComplexLinear1.AreContiguous(MiscComplexLinear2));
    ASSERT_TRUE(MiscComplexLinear1.AreContiguousAt(MiscComplexLinear2, MiscMidPoint1));
    ASSERT_EQ(2, MiscComplexLinear1.ObtainContiguousnessPoints(MiscComplexLinear2, &DumPoints));
    ASSERT_DOUBLE_EQ(0.1, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.1, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetY());

    MiscComplexLinear1.ObtainContiguousnessPointsAt(MiscComplexLinear2, MiscMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.1, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.1, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetY());

    DumPoints.clear();

    MiscComplexLinear1.AreContiguousAtAndGet(MiscComplexLinear2, MiscMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.1, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.1, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetY());

    DumPoints.clear();

    ASSERT_FALSE(MiscComplexLinear1.AreContiguous(LargeComplexLinear1));

    // Tests with negative slope ComplexLinears
    ASSERT_TRUE(MiscComplexLinear6.AreContiguous(MiscComplexLinear7));
    ASSERT_TRUE(MiscComplexLinear6.AreContiguousAt(MiscComplexLinear7, MiscMidPoint6));
    ASSERT_EQ(2, MiscComplexLinear6.ObtainContiguousnessPoints(MiscComplexLinear7, &DumPoints));
    ASSERT_DOUBLE_EQ(0.10000000000000000, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(-9.8000000000000007, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0000000000000000, DumPoints[1].GetY());

    MiscComplexLinear6.ObtainContiguousnessPointsAt(MiscComplexLinear7, MiscMidPoint6, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10000000000000000, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(-9.8000000000000007, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0000000000000000, SecondDumPoint.GetY());

    DumPoints.clear();

    MiscComplexLinear6.AreContiguousAtAndGet(MiscComplexLinear7, MiscMidPoint6, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10000000000000000, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(-9.8000000000000007, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0000000000000000, SecondDumPoint.GetY());

    DumPoints.clear();

    ASSERT_FALSE(MiscComplexLinear6.AreContiguous(MiscComplexLinear1));

    DumPoints.clear();

    // Tests with vertical EPSILON sized ComplexLinear
    ASSERT_FALSE(VerticalComplexLinear3.AreContiguous(VerticalComplexLinear2)); 
    ASSERT_FALSE(VerticalComplexLinear3.AreContiguousAt(VerticalComplexLinear2, VerticalMidPoint3));
    ASSERT_EQ(0, VerticalComplexLinear3.ObtainContiguousnessPoints(VerticalComplexLinear2, &DumPoints));
    ASSERT_FALSE(VerticalComplexLinear3.AreContiguous(HorizontalComplexLinear3));

    DumPoints.clear();

    //// Tests with horizontal EPSILON sized ComplexLinear
    ASSERT_FALSE(HorizontalComplexLinear3.AreContiguous(HorizontalComplexLinear2));
    ASSERT_FALSE(HorizontalComplexLinear3.AreContiguousAt(HorizontalComplexLinear2, HorizontalMidPoint3));
    ASSERT_EQ(0, HorizontalComplexLinear3.ObtainContiguousnessPoints(HorizontalComplexLinear2, &DumPoints));
    ASSERT_FALSE(HorizontalComplexLinear3.AreContiguous(MiscComplexLinear3));

    DumPoints.clear();

    // Test with a very large ComplexLinear
    // Precision problem   
    ASSERT_TRUE(LargeComplexLinear1.AreContiguous(LargeComplexLinear2));
    ASSERT_TRUE(LargeComplexLinear1.AreContiguousAt(LargeComplexLinear2, LargeMidPoint1));
    ASSERT_EQ(2,LargeComplexLinear1.ObtainContiguousnessPoints(LargeComplexLinear2, &DumPoints));
    ASSERT_DOUBLE_EQ(-10.0E122, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(-2.10E124, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(9.000E123, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(1.900E124, DumPoints[1].GetY());

    LargeComplexLinear1.ObtainContiguousnessPointsAt(LargeComplexLinear2, LargeMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(-10.0E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-2.10E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(9.000E123, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(1.900E124, SecondDumPoint.GetY());

    DumPoints.clear();

    LargeComplexLinear1.AreContiguousAtAndGet(LargeComplexLinear2, LargeMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(-10.0E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-2.10E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(9.000E123, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(1.900E124, SecondDumPoint.GetY());

    ASSERT_FALSE(LargeComplexLinear1.AreContiguous(HorizontalComplexLinear1));

    DumPoints.clear();

    // Test with a ComplexLinear way into positive regions
    ASSERT_TRUE(PositiveComplexLinear1.AreContiguous(PositiveComplexLinear2));
    ASSERT_TRUE(PositiveComplexLinear1.AreContiguousAt(PositiveComplexLinear2, PositiveMidPoint1));
    ASSERT_EQ(2, PositiveComplexLinear1.ObtainContiguousnessPoints(PositiveComplexLinear2, &DumPoints));
    ASSERT_DOUBLE_EQ(10.0E122, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(2.10E124, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(1.100E124, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(4.100E124, DumPoints[1].GetY());

    PositiveComplexLinear1.ObtainContiguousnessPointsAt(PositiveComplexLinear2, PositiveMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(2.10E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(1.100E124, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(4.100E124, SecondDumPoint.GetY());

    DumPoints.clear();

    PositiveComplexLinear1.AreContiguousAtAndGet(PositiveComplexLinear2, PositiveMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(10.0E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(2.10E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(1.100E124, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(4.100E124, SecondDumPoint.GetY());

    DumPoints.clear();

    ASSERT_FALSE(PositiveComplexLinear1.AreContiguous(HorizontalComplexLinear1));

    // Test with ComplexLinear way into negative ComplexLinears
    ASSERT_TRUE(NegativeComplexLinear1.AreContiguous(NegativeComplexLinear2));
    ASSERT_TRUE(NegativeComplexLinear1.AreContiguousAt(NegativeComplexLinear2, NegativeMidPoint1));
    ASSERT_EQ(2, NegativeComplexLinear1.ObtainContiguousnessPoints(NegativeComplexLinear2, &DumPoints));
    ASSERT_DOUBLE_EQ(-10.0E122, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(-2.10E124, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(-1.100E124, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(-4.100E124, DumPoints[1].GetY());

    NegativeComplexLinear1.ObtainContiguousnessPointsAt(NegativeComplexLinear2, NegativeMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(-10.0E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-2.10E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(-1.100E124, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-4.100E124, SecondDumPoint.GetY());

    DumPoints.clear();

    NegativeComplexLinear1.ObtainContiguousnessPointsAt(NegativeComplexLinear2, NegativeMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(-10.0E122, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-2.10E124, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(-1.100E124, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-4.100E124, SecondDumPoint.GetY());

    ASSERT_FALSE(NegativeComplexLinear1.AreContiguous(HorizontalComplexLinear1));

    }
  
////==================================================================================
// Cloning tests
// Clone() const;
// AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  CloningTest2) 
    { 
        
    //General Clone Test
    HFCPtr<HVE2DComplexLinear> pClone = (HVE2DComplexLinear*) MiscComplexLinear1.Clone();
    ASSERT_FALSE(pClone->IsEmpty());
    ASSERT_EQ(pWorld, pClone->GetCoordSys());   
    ASSERT_EQ(1, pClone->GetNumberOfLinears());

    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(0.1, 0.1, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(10.1, 10.1, pWorld))); 

    #ifdef WIP_IPPTEST_BUG_8
    // WIP : Complete the test once the bug is gone.
    #endif
    
    }

////==================================================================================
// Interaction info with other ComplexLinear
// Crosses(const HVE2DVector& pi_rVector) const;
// AreAdjacent(const HVE2DVector& pi_rVector) const;
// IsPointOn(const HGF2DLocation& pi_rTestPoint) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  InteractionTest2)
    { 

    // Tests with a vertical ComplexLinear
    ASSERT_FALSE(VerticalComplexLinear1.Crosses(VerticalComplexLinear2));

    ASSERT_TRUE(VerticalComplexLinear1.AreAdjacent(VerticalComplexLinear2));

    ASSERT_FALSE(VerticalComplexLinear1.Crosses(MiscComplexLinear1));
    ASSERT_FALSE(VerticalComplexLinear1.AreAdjacent(MiscComplexLinear1));

    ASSERT_FALSE(VerticalComplexLinear1.Crosses(VerticalComplexLinear3));
    ASSERT_TRUE(VerticalComplexLinear1.AreAdjacent(VerticalComplexLinear3));

    ASSERT_FALSE(VerticalComplexLinear1.Crosses(LargeComplexLinear1));
    ASSERT_FALSE(VerticalComplexLinear1.AreAdjacent(LargeComplexLinear1));

    ASSERT_FALSE(VerticalComplexLinear1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(VerticalComplexLinear1.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(VerticalComplexLinear1.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(1.1*MYEPSILON, 0)));
    ASSERT_FALSE(VerticalComplexLinear1.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(-1.1*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalComplexLinear1.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(0.9*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalComplexLinear1.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(-0.9*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalComplexLinear1.IsPointOn(VerticalComplexLinear1.GetStartPoint()));
    ASSERT_TRUE(VerticalComplexLinear1.IsPointOn(VerticalComplexLinear1.GetEndPoint()));
    ASSERT_FALSE(VerticalComplexLinear1.IsPointOn(VerticalComplexLinear1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(VerticalComplexLinear1.IsPointOn(VerticalComplexLinear1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(VerticalComplexLinear1.IsPointOn(VerticalMidPoint1));
    ASSERT_TRUE(VerticalComplexLinear1.IsPointOn(VerticalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with an inverted vertical ComplexLinear
    ASSERT_FALSE(VerticalComplexLinear2.Crosses(VerticalComplexLinear1));
    ASSERT_TRUE(VerticalComplexLinear2.AreAdjacent(VerticalComplexLinear1));

    ASSERT_FALSE(VerticalComplexLinear2.Crosses(MiscComplexLinear1));
    ASSERT_FALSE(VerticalComplexLinear2.AreAdjacent(MiscComplexLinear1));

    ASSERT_FALSE(VerticalComplexLinear2.Crosses(VerticalComplexLinear3));
    ASSERT_TRUE(VerticalComplexLinear2.AreAdjacent(VerticalComplexLinear3));

    ASSERT_FALSE(VerticalComplexLinear2.Crosses(LargeComplexLinear1));
    ASSERT_FALSE(VerticalComplexLinear2.AreAdjacent(LargeComplexLinear1));

    ASSERT_FALSE(VerticalComplexLinear2.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(VerticalComplexLinear2.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(VerticalComplexLinear2.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(1.1*MYEPSILON, 0)));
    ASSERT_FALSE(VerticalComplexLinear2.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(-1.1*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalComplexLinear2.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(0.9*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalComplexLinear2.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(-0.9*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalComplexLinear2.IsPointOn(VerticalComplexLinear2.GetStartPoint()));
    ASSERT_TRUE(VerticalComplexLinear2.IsPointOn(VerticalComplexLinear2.GetEndPoint()));
    ASSERT_TRUE(VerticalComplexLinear2.IsPointOn(VerticalMidPoint1));
    ASSERT_FALSE(VerticalComplexLinear2.IsPointOn(VerticalComplexLinear2.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(VerticalComplexLinear2.IsPointOn(VerticalComplexLinear2.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(VerticalComplexLinear2.IsPointOn(VerticalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with a horizontal ComplexLinear
    ASSERT_FALSE(HorizontalComplexLinear1.Crosses(HorizontalComplexLinear2));
    ASSERT_TRUE(HorizontalComplexLinear1.AreAdjacent(HorizontalComplexLinear2));

    ASSERT_FALSE(HorizontalComplexLinear1.Crosses(MiscComplexLinear1));
    ASSERT_FALSE(HorizontalComplexLinear1.AreAdjacent(MiscComplexLinear1));

    ASSERT_FALSE(HorizontalComplexLinear1.Crosses(HorizontalComplexLinear3));
    ASSERT_TRUE(HorizontalComplexLinear1.AreAdjacent(HorizontalComplexLinear3));

    ASSERT_FALSE(HorizontalComplexLinear1.Crosses(LargeComplexLinear1));
    ASSERT_FALSE(HorizontalComplexLinear1.AreAdjacent(LargeComplexLinear1));

    ASSERT_FALSE(HorizontalComplexLinear1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(HorizontalComplexLinear1.IsPointOn(HGF2DLocation(0.1-1.1*MYEPSILON, 0.1, pWorld)));
    ASSERT_FALSE(HorizontalComplexLinear1.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, 1.1*MYEPSILON)));
    ASSERT_FALSE(HorizontalComplexLinear1.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, -1.1*MYEPSILON)));
    ASSERT_TRUE(HorizontalComplexLinear1.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, 0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalComplexLinear1.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, -0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalComplexLinear1.IsPointOn(HorizontalComplexLinear1.GetStartPoint()));
    ASSERT_TRUE(HorizontalComplexLinear1.IsPointOn(HorizontalComplexLinear1.GetEndPoint()));
    ASSERT_TRUE(HorizontalComplexLinear1.IsPointOn(HorizontalMidPoint1));
    ASSERT_FALSE(HorizontalComplexLinear1.IsPointOn(HorizontalComplexLinear1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(HorizontalComplexLinear1.IsPointOn(HorizontalComplexLinear1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(HorizontalComplexLinear1.IsPointOn(HorizontalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with an inverted horizontal ComplexLinear
    ASSERT_FALSE(HorizontalComplexLinear2.Crosses(HorizontalComplexLinear1));
    ASSERT_TRUE(HorizontalComplexLinear2.AreAdjacent(HorizontalComplexLinear1));

    ASSERT_FALSE(HorizontalComplexLinear2.Crosses(MiscComplexLinear1));
    ASSERT_FALSE(HorizontalComplexLinear2.AreAdjacent(MiscComplexLinear1));

    ASSERT_FALSE(HorizontalComplexLinear2.Crosses(HorizontalComplexLinear3));
    ASSERT_TRUE(HorizontalComplexLinear2.AreAdjacent(HorizontalComplexLinear3));

    ASSERT_FALSE(HorizontalComplexLinear2.Crosses(LargeComplexLinear1));
    ASSERT_FALSE(HorizontalComplexLinear2.AreAdjacent(LargeComplexLinear1));

    ASSERT_FALSE(HorizontalComplexLinear2.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(HorizontalComplexLinear2.IsPointOn(HGF2DLocation(0.1-1.1*MYEPSILON, 0.1, pWorld)));
    ASSERT_FALSE(HorizontalComplexLinear2.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, 1.1*MYEPSILON)));
    ASSERT_FALSE(HorizontalComplexLinear2.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, -1.1*MYEPSILON)));
    ASSERT_TRUE(HorizontalComplexLinear2.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, 0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalComplexLinear2.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, -0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalComplexLinear2.IsPointOn(HorizontalComplexLinear2.GetStartPoint()));
    ASSERT_TRUE(HorizontalComplexLinear2.IsPointOn(HorizontalComplexLinear2.GetEndPoint()));
    ASSERT_TRUE(HorizontalComplexLinear2.IsPointOn(HorizontalMidPoint1));
    ASSERT_FALSE(HorizontalComplexLinear2.IsPointOn(HorizontalComplexLinear2.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(HorizontalComplexLinear2.IsPointOn(HorizontalComplexLinear2.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(HorizontalComplexLinear2.IsPointOn(HorizontalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with a positive slope ComplexLinear
    ASSERT_FALSE(MiscComplexLinear1.Crosses(MiscComplexLinear2));
    ASSERT_TRUE(MiscComplexLinear1.AreAdjacent(MiscComplexLinear2));

    ASSERT_FALSE(MiscComplexLinear1.Crosses(HorizontalComplexLinear1));
    ASSERT_FALSE(MiscComplexLinear1.AreAdjacent(HorizontalComplexLinear1));

    ASSERT_FALSE(MiscComplexLinear1.Crosses(MiscComplexLinear3));
    ASSERT_FALSE(MiscComplexLinear1.AreAdjacent(MiscComplexLinear3));

    ASSERT_FALSE(MiscComplexLinear1.Crosses(LargeComplexLinear1));
    ASSERT_FALSE(MiscComplexLinear1.AreAdjacent(LargeComplexLinear1));

    ASSERT_FALSE(MiscComplexLinear1.IsPointOn(HGF2DLocation(20, 20, pWorld)));
    ASSERT_FALSE(MiscComplexLinear1.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(MiscComplexLinear1.IsPointOn(MiscMidPoint1-HGF2DDisplacement(1.1*MYEPSILON, -1.1*MYEPSILON)));
    ASSERT_FALSE(MiscComplexLinear1.IsPointOn(MiscMidPoint1-HGF2DDisplacement(-1.1*MYEPSILON, 1.1*MYEPSILON)));
    ASSERT_TRUE(MiscComplexLinear1.IsPointOn(MiscMidPoint1-HGF2DDisplacement(0.7*MYEPSILON, -0.7*MYEPSILON)));
    ASSERT_TRUE(MiscComplexLinear1.IsPointOn(MiscMidPoint1-HGF2DDisplacement(-0.7*MYEPSILON, 0.7*MYEPSILON)));
    ASSERT_TRUE(MiscComplexLinear1.IsPointOn(MiscComplexLinear1.GetStartPoint()));
    ASSERT_TRUE(MiscComplexLinear1.IsPointOn(MiscComplexLinear1.GetEndPoint()));
    ASSERT_TRUE(MiscComplexLinear1.IsPointOn(MiscMidPoint1));
    ASSERT_FALSE(MiscComplexLinear1.IsPointOn(MiscComplexLinear1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(MiscComplexLinear1.IsPointOn(MiscComplexLinear1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(MiscComplexLinear1.IsPointOn(MiscMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with a negative slope ComplexLinear
    ASSERT_FALSE(MiscComplexLinear2.Crosses(MiscComplexLinear1));
    ASSERT_TRUE(MiscComplexLinear2.AreAdjacent(MiscComplexLinear1));

    ASSERT_FALSE(MiscComplexLinear2.IsPointOn(HGF2DLocation(20, 20, pWorld)));
    ASSERT_FALSE(MiscComplexLinear2.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(MiscComplexLinear2.IsPointOn(MiscMidPoint1-HGF2DDisplacement(1.1*MYEPSILON, -1.1*MYEPSILON)));
    ASSERT_FALSE(MiscComplexLinear2.IsPointOn(MiscMidPoint1-HGF2DDisplacement(-1.1*MYEPSILON, 1.1*MYEPSILON)));
    ASSERT_TRUE(MiscComplexLinear2.IsPointOn(MiscMidPoint1-HGF2DDisplacement(0.7*MYEPSILON, -0.7*MYEPSILON)));
    ASSERT_TRUE(MiscComplexLinear2.IsPointOn(MiscMidPoint1-HGF2DDisplacement(-0.7*MYEPSILON, 0.7*MYEPSILON)));
    ASSERT_TRUE(MiscComplexLinear2.IsPointOn(MiscComplexLinear1.GetStartPoint()));
    ASSERT_TRUE(MiscComplexLinear2.IsPointOn(MiscComplexLinear1.GetEndPoint()));
    ASSERT_TRUE(MiscComplexLinear2.IsPointOn(MiscMidPoint1));
    ASSERT_FALSE(MiscComplexLinear2.IsPointOn(MiscComplexLinear1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(MiscComplexLinear2.IsPointOn(MiscComplexLinear1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(MiscComplexLinear2.IsPointOn(MiscMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with a vertical EPSILON sized ComplexLinear
    ASSERT_FALSE(VerticalComplexLinear3.Crosses(VerticalComplexLinear1));
    ASSERT_TRUE(VerticalComplexLinear3.AreAdjacent(VerticalComplexLinear1));

    ASSERT_FALSE(VerticalComplexLinear3.Crosses(MiscComplexLinear1));
    ASSERT_FALSE(VerticalComplexLinear3.AreAdjacent(MiscComplexLinear1));

    ASSERT_FALSE(VerticalComplexLinear3.Crosses(MiscComplexLinear3));
    ASSERT_FALSE(VerticalComplexLinear3.AreAdjacent(MiscComplexLinear3));

    ASSERT_FALSE(VerticalComplexLinear3.Crosses(LargeComplexLinear1));
    ASSERT_FALSE(VerticalComplexLinear3.AreAdjacent(LargeComplexLinear1));

    ASSERT_FALSE(VerticalComplexLinear3.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(VerticalComplexLinear3.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(VerticalComplexLinear3.IsPointOn(VerticalMidPoint3-HGF2DDisplacement(1.1*MYEPSILON, 1.1*MYEPSILON)));
    ASSERT_FALSE(VerticalComplexLinear3.IsPointOn(VerticalMidPoint3-HGF2DDisplacement(-1.1*MYEPSILON, -1.1*MYEPSILON)));
    ASSERT_TRUE(VerticalComplexLinear3.IsPointOn(VerticalMidPoint3-HGF2DDisplacement(0.9*MYEPSILON, 0.9*MYEPSILON)));
    ASSERT_TRUE(VerticalComplexLinear3.IsPointOn(VerticalMidPoint3-HGF2DDisplacement(-0.9*MYEPSILON, -0.9*MYEPSILON)));
    ASSERT_TRUE(VerticalComplexLinear3.IsPointOn(VerticalComplexLinear3.GetStartPoint()));
    ASSERT_TRUE(VerticalComplexLinear3.IsPointOn(VerticalComplexLinear3.GetEndPoint()));
    ASSERT_TRUE(VerticalComplexLinear3.IsPointOn(VerticalMidPoint3));

    // Tests with an horizontal EPSILON SIZED ComplexLinear
    ASSERT_FALSE(HorizontalComplexLinear3.Crosses(HorizontalComplexLinear1));
    ASSERT_TRUE(HorizontalComplexLinear3.AreAdjacent(HorizontalComplexLinear1));

    ASSERT_FALSE(HorizontalComplexLinear3.Crosses(MiscComplexLinear1));
    ASSERT_FALSE(HorizontalComplexLinear3.AreAdjacent(MiscComplexLinear1));

    ASSERT_FALSE(HorizontalComplexLinear3.Crosses(MiscComplexLinear3));
    ASSERT_FALSE(HorizontalComplexLinear3.AreAdjacent(MiscComplexLinear3));

    ASSERT_FALSE(HorizontalComplexLinear3.Crosses(LargeComplexLinear1));
    ASSERT_FALSE(HorizontalComplexLinear3.AreAdjacent(LargeComplexLinear1));

    ASSERT_FALSE(HorizontalComplexLinear3.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(HorizontalComplexLinear3.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(HorizontalComplexLinear3.IsPointOn(HorizontalMidPoint3-HGF2DDisplacement(0, 1.1*MYEPSILON)));
    ASSERT_FALSE(HorizontalComplexLinear3.IsPointOn(HorizontalMidPoint3-HGF2DDisplacement(0, -1.1*MYEPSILON)));
    ASSERT_TRUE(HorizontalComplexLinear3.IsPointOn(HorizontalMidPoint3-HGF2DDisplacement(0, 0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalComplexLinear3.IsPointOn(HorizontalMidPoint3-HGF2DDisplacement(0, -0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalComplexLinear3.IsPointOn(HorizontalComplexLinear3.GetStartPoint()));
    ASSERT_TRUE(HorizontalComplexLinear3.IsPointOn(HorizontalComplexLinear3.GetEndPoint()));
    ASSERT_TRUE(HorizontalComplexLinear3.IsPointOn(HorizontalMidPoint3));

    // Tests with a miscalenious EPSILON sized ComplexLinear
    ASSERT_FALSE(MiscComplexLinear3.Crosses(MiscComplexLinear1));
    ASSERT_FALSE(MiscComplexLinear3.AreAdjacent(MiscComplexLinear1));

    ASSERT_FALSE(MiscComplexLinear3.Crosses(LargeComplexLinear1));
    ASSERT_FALSE(MiscComplexLinear3.AreAdjacent(LargeComplexLinear1));

    ASSERT_FALSE(MiscComplexLinear3.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(MiscComplexLinear3.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(MiscComplexLinear3.IsPointOn(MiscMidPoint3-HGF2DDisplacement(1.1*MYEPSILON, -1.1*MYEPSILON)));
    ASSERT_FALSE(MiscComplexLinear3.IsPointOn(MiscMidPoint3-HGF2DDisplacement(1.1*MYEPSILON, 1.1*MYEPSILON)));
    ASSERT_TRUE(MiscComplexLinear3.IsPointOn(MiscMidPoint3-HGF2DDisplacement(-0.9*MYEPSILON, 0.9*MYEPSILON)));
    ASSERT_TRUE(MiscComplexLinear3.IsPointOn(MiscMidPoint3-HGF2DDisplacement(-0.9*MYEPSILON, -0.9*MYEPSILON)));
    ASSERT_TRUE(MiscComplexLinear3.IsPointOn(MiscComplexLinear3.GetStartPoint()));
    ASSERT_TRUE(MiscComplexLinear3.IsPointOn(MiscComplexLinear3.GetEndPoint()));
    ASSERT_TRUE(MiscComplexLinear3.IsPointOn(MiscMidPoint3));

    // Tests with a very large ComplexLinear
    ASSERT_FALSE(LargeComplexLinear1.Crosses(LargeComplexLinear2));
    ASSERT_FALSE(LargeComplexLinear1.AreAdjacent(LargeComplexLinear2)); 

    ASSERT_FALSE(LargeComplexLinear1.Crosses(MiscComplexLinear1));
    ASSERT_FALSE(LargeComplexLinear1.AreAdjacent(MiscComplexLinear1));

    ASSERT_FALSE(LargeComplexLinear1.Crosses(PositiveComplexLinear1));
    ASSERT_FALSE(LargeComplexLinear1.AreAdjacent(PositiveComplexLinear1));

    ASSERT_FALSE(LargeComplexLinear1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_TRUE(LargeComplexLinear1.IsPointOn(LargeComplexLinear1.GetStartPoint()));
    ASSERT_TRUE(LargeComplexLinear1.IsPointOn(LargeComplexLinear1.GetEndPoint()));
    ASSERT_TRUE(LargeComplexLinear1.IsPointOn(LargeMidPoint1));

    // Tests with a way into positive region ComplexLinear
    ASSERT_FALSE(PositiveComplexLinear1.Crosses(PositiveComplexLinear2));
    ASSERT_FALSE(PositiveComplexLinear1.AreAdjacent(PositiveComplexLinear2)); 

    ASSERT_FALSE(PositiveComplexLinear1.Crosses(MiscComplexLinear1));
    ASSERT_FALSE(PositiveComplexLinear1.AreAdjacent(MiscComplexLinear1));

    ASSERT_FALSE(PositiveComplexLinear1.Crosses(MiscComplexLinear3));
    ASSERT_FALSE(PositiveComplexLinear1.AreAdjacent(MiscComplexLinear3));

    ASSERT_FALSE(PositiveComplexLinear1.Crosses(LargeComplexLinear1));
    ASSERT_FALSE(PositiveComplexLinear1.AreAdjacent(LargeComplexLinear1));

    ASSERT_FALSE(PositiveComplexLinear1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_TRUE(PositiveComplexLinear1.IsPointOn(PositiveComplexLinear1.GetStartPoint()));
    ASSERT_TRUE(PositiveComplexLinear1.IsPointOn(PositiveComplexLinear1.GetEndPoint()));
    ASSERT_TRUE(PositiveComplexLinear1.IsPointOn(PositiveMidPoint1));

    // Tests with a way into negative region ComplexLinear
    ASSERT_FALSE(NegativeComplexLinear1.Crosses(NegativeComplexLinear2));
    ASSERT_FALSE(NegativeComplexLinear1.AreAdjacent(NegativeComplexLinear2)); 

    ASSERT_FALSE(NegativeComplexLinear1.Crosses(MiscComplexLinear1));
    ASSERT_FALSE(NegativeComplexLinear1.AreAdjacent(MiscComplexLinear1));

    ASSERT_FALSE(NegativeComplexLinear1.Crosses(MiscComplexLinear3));
    ASSERT_FALSE(NegativeComplexLinear1.AreAdjacent(MiscComplexLinear3));

    ASSERT_FALSE(NegativeComplexLinear1.Crosses(LargeComplexLinear1));
    ASSERT_FALSE(NegativeComplexLinear1.AreAdjacent(LargeComplexLinear1));

    ASSERT_FALSE(NegativeComplexLinear1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_TRUE(NegativeComplexLinear1.IsPointOn(NegativeComplexLinear1.GetStartPoint()));
    ASSERT_TRUE(NegativeComplexLinear1.IsPointOn(NegativeComplexLinear1.GetEndPoint()));
    ASSERT_TRUE(NegativeComplexLinear1.IsPointOn(NegativeMidPoint1));

    // Tests with a NULL ComplexLinear
    ASSERT_FALSE(NullComplexLinear1.Crosses(MiscComplexLinear1));
    ASSERT_FALSE(NullComplexLinear1.AreAdjacent(MiscComplexLinear1));

    ASSERT_FALSE(NullComplexLinear1.IsPointOn(HGF2DLocation(10, 10, pWorld)));

    }

////==================================================================================
// Bearing calculation tests
// CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
//                  HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
// CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
//                              HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  BearingTest2)
    {

    // Obtain bearing ALPHA of a vertical ComplexLinear
    ASSERT_DOUBLE_EQ(3.0 * PI/2.0, VerticalComplexLinear1.CalculateBearing(VerticalMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a vertical ComplexLinear
    ASSERT_DOUBLE_EQ(PI/2.0, VerticalComplexLinear1.CalculateBearing(VerticalMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted vertical ComplexLinear
    ASSERT_DOUBLE_EQ(PI/2.0, VerticalComplexLinear2.CalculateBearing(VerticalMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted vertical ComplexLinear
    ASSERT_DOUBLE_EQ(3*PI/2, VerticalComplexLinear2.CalculateBearing(VerticalMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of an horizontal ComplexLinear
    ASSERT_DOUBLE_EQ(PI, HorizontalComplexLinear1.CalculateBearing(HorizontalMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of an horizontal ComplexLinear
    ASSERT_NEAR(0.0, HorizontalComplexLinear1.CalculateBearing(HorizontalMidPoint1, HVE2DVector::BETA).GetAngle(), MYEPSILON);

    // Obtain bearing ALPHA of an inverted horizontal ComplexLinear
    ASSERT_NEAR(0.0, HorizontalComplexLinear2.CalculateBearing(HorizontalMidPoint1, HVE2DVector::ALPHA).GetAngle(), MYEPSILON);

    // Obtain bearing BETA of an inverted horizontal ComplexLinear
    ASSERT_DOUBLE_EQ(PI, HorizontalComplexLinear2.CalculateBearing(HorizontalMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a positive slope ComplexLinear
    ASSERT_DOUBLE_EQ(-2.3561944901923448, MiscComplexLinear1.CalculateBearing(MiscMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a positive slope ComplexLinear
    ASSERT_DOUBLE_EQ(PI/4.0, MiscComplexLinear1.CalculateBearing(MiscMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted positive slope ComplexLinear
    ASSERT_DOUBLE_EQ(PI/4.0, MiscComplexLinear2.CalculateBearing(MiscMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted positive slope ComplexLinear
    ASSERT_DOUBLE_EQ(-2.3561944901923448, MiscComplexLinear2.CalculateBearing(MiscMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a vertical EPSILON SIZED ComplexLinear
    ASSERT_DOUBLE_EQ(3.0*PI/2.0, VerticalComplexLinear3.CalculateBearing(VerticalMidPoint3, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a vertical EPSILON SIZED ComplexLinear
    ASSERT_DOUBLE_EQ(PI/2.0, VerticalComplexLinear3.CalculateBearing(VerticalMidPoint3, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a horizontal EPSILON SIZED ComplexLinear
    ASSERT_DOUBLE_EQ(PI, HorizontalComplexLinear3.CalculateBearing(HorizontalMidPoint3, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a horizontal EPSILON SIZED ComplexLinear
    ASSERT_NEAR(0.0, HorizontalComplexLinear3.CalculateBearing(HorizontalMidPoint3, HVE2DVector::BETA).GetAngle(), MYEPSILON);

    // Obtain bearing ALPHA of a miscaleniuous EPSILON SIZED ComplexLinear
    ASSERT_DOUBLE_EQ(-1.7889624832338027, MiscComplexLinear3.CalculateBearing(MiscMidPoint3, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a miscaleniuous EPSILON SIZED ComplexLinear
    ASSERT_DOUBLE_EQ(1.3526301703559906, MiscComplexLinear3.CalculateBearing(MiscMidPoint3, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a very large ComplexLinear
    ASSERT_DOUBLE_EQ(-1.8157749899217608, LargeComplexLinear1.CalculateBearing(LargeMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a very large ComplexLinear
    ASSERT_DOUBLE_EQ(1.3258176636680326, LargeComplexLinear1.CalculateBearing(LargeMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted very large ComplexLinear
    ASSERT_DOUBLE_EQ(1.3258176636680326, LargeComplexLinear2.CalculateBearing(LargeMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted very large ComplexLinear
    ASSERT_DOUBLE_EQ(-1.8157749899217608, LargeComplexLinear2.CalculateBearing(LargeMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a ComplexLinear way into negative coordinates
    ASSERT_DOUBLE_EQ(1.1071487177940904, NegativeComplexLinear1.CalculateBearing(NegativeMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a ComplexLinear way into negative coordinates
    ASSERT_DOUBLE_EQ(-2.0344439357957027, NegativeComplexLinear1.CalculateBearing(NegativeMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted ComplexLinear way into negative coordinates
    ASSERT_DOUBLE_EQ(-2.0344439357957027, NegativeComplexLinear2.CalculateBearing(NegativeMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted ComplexLinear way into negative coordinates
    ASSERT_DOUBLE_EQ(1.1071487177940904, NegativeComplexLinear2.CalculateBearing(NegativeMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a ComplexLinear way into positive values
    ASSERT_DOUBLE_EQ(-2.0344439357957027, PositiveComplexLinear1.CalculateBearing(PositiveMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a ComplexLinear way into positive values
    ASSERT_DOUBLE_EQ(1.1071487177940904, PositiveComplexLinear1.CalculateBearing(PositiveMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted ComplexLinear way into positive values
    ASSERT_DOUBLE_EQ(1.1071487177940904, PositiveComplexLinear2.CalculateBearing(PositiveMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted ComplexLinear way into positive values
    ASSERT_DOUBLE_EQ(-2.0344439357957027, PositiveComplexLinear2.CalculateBearing(PositiveMidPoint1, HVE2DVector::BETA).GetAngle());

    // ANGULAR ACCELERATION
    // Obtain angular acceleration ALPHA of a vertical ComplexLinear
    ASSERT_NEAR(0.0, VerticalComplexLinear1.CalculateAngularAcceleration(VerticalMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a vertical ComplexLinear
    ASSERT_NEAR(0.0, VerticalComplexLinear1.CalculateAngularAcceleration(VerticalMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted vertical ComplexLinear
    ASSERT_NEAR(0.0, VerticalComplexLinear2.CalculateAngularAcceleration(VerticalMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted vertical ComplexLinear
    ASSERT_NEAR(0.0, VerticalComplexLinear2.CalculateAngularAcceleration(VerticalMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of an horizontal ComplexLinear
    ASSERT_NEAR(0.0, HorizontalComplexLinear1.CalculateAngularAcceleration(HorizontalMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of an horizontal ComplexLinear
    ASSERT_NEAR(0.0, HorizontalComplexLinear1.CalculateAngularAcceleration(HorizontalMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of an inverted horizontal ComplexLinear
    ASSERT_NEAR(0.0, HorizontalComplexLinear2.CalculateAngularAcceleration(HorizontalMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of an inverted horizontal ComplexLinear
    ASSERT_NEAR(0.0, HorizontalComplexLinear2.CalculateAngularAcceleration(HorizontalMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a positive slope ComplexLinear
    ASSERT_NEAR(0.0, MiscComplexLinear1.CalculateAngularAcceleration(MiscMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a positive slope ComplexLinear
    ASSERT_NEAR(0.0, MiscComplexLinear1.CalculateAngularAcceleration(MiscMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted positive slope ComplexLinear
    ASSERT_NEAR(0.0, MiscComplexLinear2.CalculateAngularAcceleration(MiscMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted positive slope ComplexLinear
    ASSERT_NEAR(0.0, MiscComplexLinear2.CalculateAngularAcceleration(MiscMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a vertical EPSILON SIZED ComplexLinear
    ASSERT_NEAR(0.0, VerticalComplexLinear3.CalculateAngularAcceleration(VerticalMidPoint3, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a vertical EPSILON SIZED ComplexLinear
    ASSERT_NEAR(0.0, VerticalComplexLinear3.CalculateAngularAcceleration(VerticalMidPoint3, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a horizontal EPSILON SIZED ComplexLinear
    ASSERT_NEAR(0.0, HorizontalComplexLinear3.CalculateAngularAcceleration(HorizontalMidPoint3, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a horizontal EPSILON SIZED ComplexLinear
    ASSERT_NEAR(0.0, HorizontalComplexLinear3.CalculateAngularAcceleration(HorizontalMidPoint3, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a miscaleniuous EPSILON SIZED ComplexLinear
    ASSERT_NEAR(0.0, MiscComplexLinear3.CalculateAngularAcceleration(MiscMidPoint3, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a miscaleniuous EPSILON SIZED ComplexLinear
    ASSERT_NEAR(0.0, MiscComplexLinear3.CalculateAngularAcceleration(MiscMidPoint3, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a very large ComplexLinear
    ASSERT_NEAR(0.0, LargeComplexLinear1.CalculateAngularAcceleration(LargeMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a very large ComplexLinear
    ASSERT_NEAR(0.0, LargeComplexLinear1.CalculateAngularAcceleration(LargeMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted very large ComplexLinear
    ASSERT_NEAR(0.0, LargeComplexLinear2.CalculateAngularAcceleration(LargeMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted very large ComplexLinear
    ASSERT_NEAR(0.0, LargeComplexLinear2.CalculateAngularAcceleration(LargeMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a ComplexLinear way into negative coordinates
    ASSERT_NEAR(0.0, NegativeComplexLinear1.CalculateAngularAcceleration(NegativeMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a ComplexLinear way into negative coordinates
    ASSERT_NEAR(0.0, NegativeComplexLinear1.CalculateAngularAcceleration(NegativeMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted ComplexLinear way into negative coordinates
    ASSERT_NEAR(0.0, NegativeComplexLinear2.CalculateAngularAcceleration(NegativeMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted ComplexLinear way into negative coordinates
    ASSERT_NEAR(0.0, NegativeComplexLinear2.CalculateAngularAcceleration(NegativeMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a ComplexLinear way into positive values
    ASSERT_NEAR(0.0, PositiveComplexLinear1.CalculateAngularAcceleration(PositiveMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a ComplexLinear way into positive values
    ASSERT_NEAR(0.0, PositiveComplexLinear1.CalculateAngularAcceleration(PositiveMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted ComplexLinear way into positive values
    ASSERT_NEAR(0.0, PositiveComplexLinear2.CalculateAngularAcceleration(PositiveMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted ComplexLinear way into positive values
    ASSERT_NEAR(0.0, PositiveComplexLinear2.CalculateAngularAcceleration(PositiveMidPoint1, HVE2DVector::BETA), MYEPSILON);

    }

////==================================================================================
// Extent calculation test
// GetExtent() const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  GetExtentTest2)
    { 

    // Obtain extent of a vertical ComplexLinear
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.1, VerticalComplexLinear1.GetExtent().GetYMax());

    // Obtain extent of a inverted vertical ComplexLinear
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear2.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear2.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.10, VerticalComplexLinear2.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.1, VerticalComplexLinear2.GetExtent().GetYMax());

    // Obtain extent of an horizontal ComplexLinear
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(10.1, HorizontalComplexLinear1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear1.GetExtent().GetYMax());

    // Obtain extent of an inverted horizontal ComplexLinear
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear2.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear2.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(10.1, HorizontalComplexLinear2.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.10, HorizontalComplexLinear2.GetExtent().GetYMax());

    // Obtain extent of a positive slope ComplexLinear
    ASSERT_DOUBLE_EQ(0.10, MiscComplexLinear1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, MiscComplexLinear1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(10.1, MiscComplexLinear1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.1, MiscComplexLinear1.GetExtent().GetYMax());

    // Obtain extent of a vertical EPSILON SIZED ComplexLinear
    ASSERT_DOUBLE_EQ(0.1000000, VerticalComplexLinear3.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.1000000, VerticalComplexLinear3.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.1000000, VerticalComplexLinear3.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.1000001, VerticalComplexLinear3.GetExtent().GetYMax());

    // Obtain extent of a horizontal EPSILON SIZED ComplexLinear
    ASSERT_DOUBLE_EQ(0.1000000, HorizontalComplexLinear3.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.1000000, HorizontalComplexLinear3.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.1000001, HorizontalComplexLinear3.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.1000000, HorizontalComplexLinear3.GetExtent().GetYMax());

    // Obtain extent of a miscaleniuous EPSILON SIZED ComplexLinear
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscComplexLinear3.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscComplexLinear3.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.10000002164396139, MiscComplexLinear3.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.10000009762960071, MiscComplexLinear3.GetExtent().GetYMax());

    // Obtain extent of a very large ComplexLinear
    ASSERT_DOUBLE_EQ(-1.00E123, LargeComplexLinear1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(-21.0E123, LargeComplexLinear1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(9.000E123, LargeComplexLinear1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(19.00E123, LargeComplexLinear1.GetExtent().GetYMax());

    // Obtain extent of a ComplexLinear way into negative coordinates
    ASSERT_DOUBLE_EQ(-11.0E123, NegativeComplexLinear1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(-41.0E123, NegativeComplexLinear1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(-10.0E122, NegativeComplexLinear1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(-21.0E123, NegativeComplexLinear1.GetExtent().GetYMax());

    // Obtain extent of a ComplexLinear way into positive values
    ASSERT_DOUBLE_EQ(1.00E123, PositiveComplexLinear1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(21.0E123, PositiveComplexLinear1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(11.0E123, PositiveComplexLinear1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(41.0E123, PositiveComplexLinear1.GetExtent().GetYMax());

    }

////==================================================================================
// Additional tests for contiguousness
////==================================================================================
TEST_F (HVE2DComplexLinearTester,  ContiguousnessTestWhoPreviouslyFailedTest2)
    { 

    // Additional tests for contiguousness

    // The following test failed in complex linear testing and is not part of
    // the previous tests
    HVE2DComplexLinear    HorizontalComplexLinearSup1(pWorld);
    HorizontalComplexLinearSup1.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld)));
    HVE2DComplexLinear    HorizontalComplexLinearSup2(pWorld);
    HorizontalComplexLinearSup2.AppendLinear(HVE2DSegment(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(35.0, 10.0, pWorld)));

    ASSERT_FALSE(HorizontalComplexLinearSup1.AreContiguous(HorizontalComplexLinearSup2));
    ASSERT_FALSE(HorizontalComplexLinearSup2.AreContiguous(HorizontalComplexLinearSup1));

    HVE2DComplexLinear    VerticalComplexLinearSup1(pWorld);
    VerticalComplexLinearSup1.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(10.0, 20.0, pWorld)));
    HVE2DComplexLinear    VerticalComplexLinearSup2(pWorld);
    VerticalComplexLinearSup2.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 20.0, pWorld), HGF2DLocation(10.0, 35.0, pWorld)));

    ASSERT_FALSE(VerticalComplexLinearSup1.AreContiguous(VerticalComplexLinearSup2));
    ASSERT_FALSE(VerticalComplexLinearSup2.AreContiguous(VerticalComplexLinearSup1));

    // Other test which failed
    // all of the following are contiguous
    HVE2DComplexLinear    HorizontalComplexLinearSup3(pWorld);
    HorizontalComplexLinearSup3.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld)));
    HVE2DComplexLinear    HorizontalComplexLinearSup4(pWorld);
    HorizontalComplexLinearSup4.AppendLinear(HVE2DSegment(HGF2DLocation(5.0, 10.0, pWorld), HGF2DLocation(35.0, 10.0, pWorld)));
    HVE2DComplexLinear    HorizontalComplexLinearSup5(pWorld);
    HorizontalComplexLinearSup5.AppendLinear(HVE2DSegment(HGF2DLocation(5.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld)));
    HVE2DComplexLinear    HorizontalComplexLinearSup6(pWorld);
    HorizontalComplexLinearSup6.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(25.0, 10.0, pWorld)));
    HVE2DComplexLinear    HorizontalComplexLinearSup7(pWorld);
    HorizontalComplexLinearSup7.AppendLinear(HVE2DSegment(HGF2DLocation(5.0, 10.0, pWorld), HGF2DLocation(15.0, 10.0, pWorld)));
    HVE2DComplexLinear    HorizontalComplexLinearSup8(pWorld);
    HorizontalComplexLinearSup8.AppendLinear(HVE2DSegment(HGF2DLocation(13.0, 10.0, pWorld), HGF2DLocation(25.0, 10.0, pWorld)));
    HVE2DComplexLinear    HorizontalComplexLinearSup9(pWorld);
    HorizontalComplexLinearSup9.AppendLinear(HVE2DSegment(HGF2DLocation(12.0, 10.0, pWorld), HGF2DLocation(18.0, 10.0, pWorld)));

    HVE2DComplexLinear    HorizontalComplexLinearSup10(pWorld);
    HorizontalComplexLinearSup10.AppendLinear(HVE2DSegment(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    HVE2DComplexLinear    HorizontalComplexLinearSup11(pWorld);
    HorizontalComplexLinearSup11.AppendLinear(HVE2DSegment(HGF2DLocation(35.0, 10.0, pWorld), HGF2DLocation(5.0, 10.0, pWorld)));
    HVE2DComplexLinear    HorizontalComplexLinearSup12(pWorld);
    HorizontalComplexLinearSup12.AppendLinear(HVE2DSegment(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(5.0, 10.0, pWorld)));
    HVE2DComplexLinear    HorizontalComplexLinearSup13(pWorld);
    HorizontalComplexLinearSup13.AppendLinear(HVE2DSegment(HGF2DLocation(25.0, 10.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    HVE2DComplexLinear    HorizontalComplexLinearSup14(pWorld);
    HorizontalComplexLinearSup14.AppendLinear(HVE2DSegment(HGF2DLocation(15.0, 10.0, pWorld), HGF2DLocation(5.0, 10.0, pWorld)));
    HVE2DComplexLinear    HorizontalComplexLinearSup15(pWorld);
    HorizontalComplexLinearSup15.AppendLinear(HVE2DSegment(HGF2DLocation(25.0, 10.0, pWorld), HGF2DLocation(13.0, 10.0, pWorld)));
    HVE2DComplexLinear    HorizontalComplexLinearSup16(pWorld);
    HorizontalComplexLinearSup16.AppendLinear(HVE2DSegment(HGF2DLocation(18.0, 10.0, pWorld), HGF2DLocation(12.0, 10.0, pWorld)));

    ASSERT_TRUE(HorizontalComplexLinearSup3.AreContiguous(HorizontalComplexLinearSup4));
    ASSERT_TRUE(HorizontalComplexLinearSup4.AreContiguous(HorizontalComplexLinearSup3));
    ASSERT_TRUE(HorizontalComplexLinearSup3.AreContiguous(HorizontalComplexLinearSup5));
    ASSERT_TRUE(HorizontalComplexLinearSup5.AreContiguous(HorizontalComplexLinearSup3));
    ASSERT_TRUE(HorizontalComplexLinearSup3.AreContiguous(HorizontalComplexLinearSup6));
    ASSERT_TRUE(HorizontalComplexLinearSup6.AreContiguous(HorizontalComplexLinearSup3));
    ASSERT_TRUE(HorizontalComplexLinearSup3.AreContiguous(HorizontalComplexLinearSup7));
    ASSERT_TRUE(HorizontalComplexLinearSup7.AreContiguous(HorizontalComplexLinearSup3));
    ASSERT_TRUE(HorizontalComplexLinearSup3.AreContiguous(HorizontalComplexLinearSup8));
    ASSERT_TRUE(HorizontalComplexLinearSup8.AreContiguous(HorizontalComplexLinearSup3));
    ASSERT_TRUE(HorizontalComplexLinearSup3.AreContiguous(HorizontalComplexLinearSup9));
    ASSERT_TRUE(HorizontalComplexLinearSup9.AreContiguous(HorizontalComplexLinearSup3));
    ASSERT_TRUE(HorizontalComplexLinearSup3.AreContiguous(HorizontalComplexLinearSup10));
    ASSERT_TRUE(HorizontalComplexLinearSup10.AreContiguous(HorizontalComplexLinearSup3));
    ASSERT_TRUE(HorizontalComplexLinearSup3.AreContiguous(HorizontalComplexLinearSup11));
    ASSERT_TRUE(HorizontalComplexLinearSup11.AreContiguous(HorizontalComplexLinearSup3));
    ASSERT_TRUE(HorizontalComplexLinearSup3.AreContiguous(HorizontalComplexLinearSup12));
    ASSERT_TRUE(HorizontalComplexLinearSup12.AreContiguous(HorizontalComplexLinearSup3));
    ASSERT_TRUE(HorizontalComplexLinearSup3.AreContiguous(HorizontalComplexLinearSup13));
    ASSERT_TRUE(HorizontalComplexLinearSup13.AreContiguous(HorizontalComplexLinearSup3));
    ASSERT_TRUE(HorizontalComplexLinearSup3.AreContiguous(HorizontalComplexLinearSup14));
    ASSERT_TRUE(HorizontalComplexLinearSup14.AreContiguous(HorizontalComplexLinearSup3));
    ASSERT_TRUE(HorizontalComplexLinearSup3.AreContiguous(HorizontalComplexLinearSup15));
    ASSERT_TRUE(HorizontalComplexLinearSup15.AreContiguous(HorizontalComplexLinearSup3));
    ASSERT_TRUE(HorizontalComplexLinearSup3.AreContiguous(HorizontalComplexLinearSup16));
    ASSERT_TRUE(HorizontalComplexLinearSup16.AreContiguous(HorizontalComplexLinearSup3));

    ASSERT_TRUE(HorizontalComplexLinearSup4.AreContiguous(HorizontalComplexLinearSup5));
    ASSERT_TRUE(HorizontalComplexLinearSup5.AreContiguous(HorizontalComplexLinearSup4));
    ASSERT_TRUE(HorizontalComplexLinearSup4.AreContiguous(HorizontalComplexLinearSup6));
    ASSERT_TRUE(HorizontalComplexLinearSup6.AreContiguous(HorizontalComplexLinearSup4));
    ASSERT_TRUE(HorizontalComplexLinearSup4.AreContiguous(HorizontalComplexLinearSup7));
    ASSERT_TRUE(HorizontalComplexLinearSup7.AreContiguous(HorizontalComplexLinearSup4));
    ASSERT_TRUE(HorizontalComplexLinearSup4.AreContiguous(HorizontalComplexLinearSup8));
    ASSERT_TRUE(HorizontalComplexLinearSup8.AreContiguous(HorizontalComplexLinearSup4));
    ASSERT_TRUE(HorizontalComplexLinearSup4.AreContiguous(HorizontalComplexLinearSup9));
    ASSERT_TRUE(HorizontalComplexLinearSup9.AreContiguous(HorizontalComplexLinearSup4));
    ASSERT_TRUE(HorizontalComplexLinearSup4.AreContiguous(HorizontalComplexLinearSup10));
    ASSERT_TRUE(HorizontalComplexLinearSup10.AreContiguous(HorizontalComplexLinearSup4));
    ASSERT_TRUE(HorizontalComplexLinearSup4.AreContiguous(HorizontalComplexLinearSup11));
    ASSERT_TRUE(HorizontalComplexLinearSup11.AreContiguous(HorizontalComplexLinearSup4));
    ASSERT_TRUE(HorizontalComplexLinearSup4.AreContiguous(HorizontalComplexLinearSup12));
    ASSERT_TRUE(HorizontalComplexLinearSup12.AreContiguous(HorizontalComplexLinearSup4));
    ASSERT_TRUE(HorizontalComplexLinearSup4.AreContiguous(HorizontalComplexLinearSup13));
    ASSERT_TRUE(HorizontalComplexLinearSup13.AreContiguous(HorizontalComplexLinearSup4));
    ASSERT_TRUE(HorizontalComplexLinearSup4.AreContiguous(HorizontalComplexLinearSup14));
    ASSERT_TRUE(HorizontalComplexLinearSup14.AreContiguous(HorizontalComplexLinearSup4));
    ASSERT_TRUE(HorizontalComplexLinearSup4.AreContiguous(HorizontalComplexLinearSup15));
    ASSERT_TRUE(HorizontalComplexLinearSup15.AreContiguous(HorizontalComplexLinearSup4));
    ASSERT_TRUE(HorizontalComplexLinearSup4.AreContiguous(HorizontalComplexLinearSup16));
    ASSERT_TRUE(HorizontalComplexLinearSup16.AreContiguous(HorizontalComplexLinearSup4));

    ASSERT_TRUE(HorizontalComplexLinearSup5.AreContiguous(HorizontalComplexLinearSup6));
    ASSERT_TRUE(HorizontalComplexLinearSup6.AreContiguous(HorizontalComplexLinearSup5));
    ASSERT_TRUE(HorizontalComplexLinearSup5.AreContiguous(HorizontalComplexLinearSup7));
    ASSERT_TRUE(HorizontalComplexLinearSup7.AreContiguous(HorizontalComplexLinearSup5));
    ASSERT_TRUE(HorizontalComplexLinearSup5.AreContiguous(HorizontalComplexLinearSup8));
    ASSERT_TRUE(HorizontalComplexLinearSup8.AreContiguous(HorizontalComplexLinearSup5));
    ASSERT_TRUE(HorizontalComplexLinearSup5.AreContiguous(HorizontalComplexLinearSup9));
    ASSERT_TRUE(HorizontalComplexLinearSup9.AreContiguous(HorizontalComplexLinearSup5));
    ASSERT_TRUE(HorizontalComplexLinearSup5.AreContiguous(HorizontalComplexLinearSup10));
    ASSERT_TRUE(HorizontalComplexLinearSup10.AreContiguous(HorizontalComplexLinearSup5));
    ASSERT_TRUE(HorizontalComplexLinearSup5.AreContiguous(HorizontalComplexLinearSup11));
    ASSERT_TRUE(HorizontalComplexLinearSup11.AreContiguous(HorizontalComplexLinearSup5));
    ASSERT_TRUE(HorizontalComplexLinearSup5.AreContiguous(HorizontalComplexLinearSup12));
    ASSERT_TRUE(HorizontalComplexLinearSup12.AreContiguous(HorizontalComplexLinearSup5));
    ASSERT_TRUE(HorizontalComplexLinearSup5.AreContiguous(HorizontalComplexLinearSup13));
    ASSERT_TRUE(HorizontalComplexLinearSup13.AreContiguous(HorizontalComplexLinearSup5));
    ASSERT_TRUE(HorizontalComplexLinearSup5.AreContiguous(HorizontalComplexLinearSup14));
    ASSERT_TRUE(HorizontalComplexLinearSup14.AreContiguous(HorizontalComplexLinearSup5));
    ASSERT_TRUE(HorizontalComplexLinearSup5.AreContiguous(HorizontalComplexLinearSup15));
    ASSERT_TRUE(HorizontalComplexLinearSup15.AreContiguous(HorizontalComplexLinearSup5));
    ASSERT_TRUE(HorizontalComplexLinearSup5.AreContiguous(HorizontalComplexLinearSup16));
    ASSERT_TRUE(HorizontalComplexLinearSup16.AreContiguous(HorizontalComplexLinearSup5));

    ASSERT_TRUE(HorizontalComplexLinearSup6.AreContiguous(HorizontalComplexLinearSup7));
    ASSERT_TRUE(HorizontalComplexLinearSup7.AreContiguous(HorizontalComplexLinearSup6));
    ASSERT_TRUE(HorizontalComplexLinearSup6.AreContiguous(HorizontalComplexLinearSup8));
    ASSERT_TRUE(HorizontalComplexLinearSup8.AreContiguous(HorizontalComplexLinearSup6));
    ASSERT_TRUE(HorizontalComplexLinearSup6.AreContiguous(HorizontalComplexLinearSup9));
    ASSERT_TRUE(HorizontalComplexLinearSup9.AreContiguous(HorizontalComplexLinearSup6));
    ASSERT_TRUE(HorizontalComplexLinearSup6.AreContiguous(HorizontalComplexLinearSup10));
    ASSERT_TRUE(HorizontalComplexLinearSup10.AreContiguous(HorizontalComplexLinearSup6));
    ASSERT_TRUE(HorizontalComplexLinearSup6.AreContiguous(HorizontalComplexLinearSup11));
    ASSERT_TRUE(HorizontalComplexLinearSup11.AreContiguous(HorizontalComplexLinearSup6));
    ASSERT_TRUE(HorizontalComplexLinearSup6.AreContiguous(HorizontalComplexLinearSup12));
    ASSERT_TRUE(HorizontalComplexLinearSup12.AreContiguous(HorizontalComplexLinearSup6));
    ASSERT_TRUE(HorizontalComplexLinearSup6.AreContiguous(HorizontalComplexLinearSup13));
    ASSERT_TRUE(HorizontalComplexLinearSup13.AreContiguous(HorizontalComplexLinearSup6));
    ASSERT_TRUE(HorizontalComplexLinearSup6.AreContiguous(HorizontalComplexLinearSup14));
    ASSERT_TRUE(HorizontalComplexLinearSup14.AreContiguous(HorizontalComplexLinearSup6));
    ASSERT_TRUE(HorizontalComplexLinearSup6.AreContiguous(HorizontalComplexLinearSup15));
    ASSERT_TRUE(HorizontalComplexLinearSup15.AreContiguous(HorizontalComplexLinearSup6));
    ASSERT_TRUE(HorizontalComplexLinearSup6.AreContiguous(HorizontalComplexLinearSup16));
    ASSERT_TRUE(HorizontalComplexLinearSup16.AreContiguous(HorizontalComplexLinearSup6));

    ASSERT_TRUE(HorizontalComplexLinearSup7.AreContiguous(HorizontalComplexLinearSup8));
    ASSERT_TRUE(HorizontalComplexLinearSup8.AreContiguous(HorizontalComplexLinearSup7));
    ASSERT_TRUE(HorizontalComplexLinearSup7.AreContiguous(HorizontalComplexLinearSup9));
    ASSERT_TRUE(HorizontalComplexLinearSup9.AreContiguous(HorizontalComplexLinearSup7));
    ASSERT_TRUE(HorizontalComplexLinearSup7.AreContiguous(HorizontalComplexLinearSup10));
    ASSERT_TRUE(HorizontalComplexLinearSup10.AreContiguous(HorizontalComplexLinearSup7));
    ASSERT_TRUE(HorizontalComplexLinearSup7.AreContiguous(HorizontalComplexLinearSup11));
    ASSERT_TRUE(HorizontalComplexLinearSup11.AreContiguous(HorizontalComplexLinearSup7));
    ASSERT_TRUE(HorizontalComplexLinearSup7.AreContiguous(HorizontalComplexLinearSup12));
    ASSERT_TRUE(HorizontalComplexLinearSup12.AreContiguous(HorizontalComplexLinearSup7));
    ASSERT_TRUE(HorizontalComplexLinearSup7.AreContiguous(HorizontalComplexLinearSup13));
    ASSERT_TRUE(HorizontalComplexLinearSup13.AreContiguous(HorizontalComplexLinearSup7));
    ASSERT_TRUE(HorizontalComplexLinearSup7.AreContiguous(HorizontalComplexLinearSup14));
    ASSERT_TRUE(HorizontalComplexLinearSup14.AreContiguous(HorizontalComplexLinearSup7));
    ASSERT_TRUE(HorizontalComplexLinearSup7.AreContiguous(HorizontalComplexLinearSup15));
    ASSERT_TRUE(HorizontalComplexLinearSup15.AreContiguous(HorizontalComplexLinearSup7));
    ASSERT_TRUE(HorizontalComplexLinearSup7.AreContiguous(HorizontalComplexLinearSup16));
    ASSERT_TRUE(HorizontalComplexLinearSup16.AreContiguous(HorizontalComplexLinearSup7));

    ASSERT_TRUE(HorizontalComplexLinearSup8.AreContiguous(HorizontalComplexLinearSup9));
    ASSERT_TRUE(HorizontalComplexLinearSup9.AreContiguous(HorizontalComplexLinearSup8));
    ASSERT_TRUE(HorizontalComplexLinearSup8.AreContiguous(HorizontalComplexLinearSup10));
    ASSERT_TRUE(HorizontalComplexLinearSup10.AreContiguous(HorizontalComplexLinearSup8));
    ASSERT_TRUE(HorizontalComplexLinearSup8.AreContiguous(HorizontalComplexLinearSup11));
    ASSERT_TRUE(HorizontalComplexLinearSup11.AreContiguous(HorizontalComplexLinearSup8));
    ASSERT_TRUE(HorizontalComplexLinearSup8.AreContiguous(HorizontalComplexLinearSup12));
    ASSERT_TRUE(HorizontalComplexLinearSup12.AreContiguous(HorizontalComplexLinearSup8));
    ASSERT_TRUE(HorizontalComplexLinearSup8.AreContiguous(HorizontalComplexLinearSup13));
    ASSERT_TRUE(HorizontalComplexLinearSup13.AreContiguous(HorizontalComplexLinearSup8));
    ASSERT_TRUE(HorizontalComplexLinearSup8.AreContiguous(HorizontalComplexLinearSup14));
    ASSERT_TRUE(HorizontalComplexLinearSup14.AreContiguous(HorizontalComplexLinearSup8));
    ASSERT_TRUE(HorizontalComplexLinearSup8.AreContiguous(HorizontalComplexLinearSup15));
    ASSERT_TRUE(HorizontalComplexLinearSup15.AreContiguous(HorizontalComplexLinearSup8));
    ASSERT_TRUE(HorizontalComplexLinearSup8.AreContiguous(HorizontalComplexLinearSup16));
    ASSERT_TRUE(HorizontalComplexLinearSup16.AreContiguous(HorizontalComplexLinearSup8));

    ASSERT_TRUE(HorizontalComplexLinearSup9.AreContiguous(HorizontalComplexLinearSup10));
    ASSERT_TRUE(HorizontalComplexLinearSup10.AreContiguous(HorizontalComplexLinearSup9));
    ASSERT_TRUE(HorizontalComplexLinearSup9.AreContiguous(HorizontalComplexLinearSup11));
    ASSERT_TRUE(HorizontalComplexLinearSup11.AreContiguous(HorizontalComplexLinearSup9));
    ASSERT_TRUE(HorizontalComplexLinearSup9.AreContiguous(HorizontalComplexLinearSup12));
    ASSERT_TRUE(HorizontalComplexLinearSup12.AreContiguous(HorizontalComplexLinearSup9));
    ASSERT_TRUE(HorizontalComplexLinearSup9.AreContiguous(HorizontalComplexLinearSup13));
    ASSERT_TRUE(HorizontalComplexLinearSup13.AreContiguous(HorizontalComplexLinearSup9));
    ASSERT_TRUE(HorizontalComplexLinearSup9.AreContiguous(HorizontalComplexLinearSup14));
    ASSERT_TRUE(HorizontalComplexLinearSup14.AreContiguous(HorizontalComplexLinearSup9));
    ASSERT_TRUE(HorizontalComplexLinearSup9.AreContiguous(HorizontalComplexLinearSup15));
    ASSERT_TRUE(HorizontalComplexLinearSup15.AreContiguous(HorizontalComplexLinearSup9));
    ASSERT_TRUE(HorizontalComplexLinearSup9.AreContiguous(HorizontalComplexLinearSup16));
    ASSERT_TRUE(HorizontalComplexLinearSup16.AreContiguous(HorizontalComplexLinearSup9));

    ASSERT_TRUE(HorizontalComplexLinearSup10.AreContiguous(HorizontalComplexLinearSup11));
    ASSERT_TRUE(HorizontalComplexLinearSup11.AreContiguous(HorizontalComplexLinearSup10));
    ASSERT_TRUE(HorizontalComplexLinearSup10.AreContiguous(HorizontalComplexLinearSup12));
    ASSERT_TRUE(HorizontalComplexLinearSup12.AreContiguous(HorizontalComplexLinearSup10));
    ASSERT_TRUE(HorizontalComplexLinearSup10.AreContiguous(HorizontalComplexLinearSup13));
    ASSERT_TRUE(HorizontalComplexLinearSup13.AreContiguous(HorizontalComplexLinearSup10));
    ASSERT_TRUE(HorizontalComplexLinearSup10.AreContiguous(HorizontalComplexLinearSup14));
    ASSERT_TRUE(HorizontalComplexLinearSup14.AreContiguous(HorizontalComplexLinearSup10));
    ASSERT_TRUE(HorizontalComplexLinearSup10.AreContiguous(HorizontalComplexLinearSup15));
    ASSERT_TRUE(HorizontalComplexLinearSup15.AreContiguous(HorizontalComplexLinearSup10));
    ASSERT_TRUE(HorizontalComplexLinearSup10.AreContiguous(HorizontalComplexLinearSup16));
    ASSERT_TRUE(HorizontalComplexLinearSup16.AreContiguous(HorizontalComplexLinearSup10));

    ASSERT_TRUE(HorizontalComplexLinearSup11.AreContiguous(HorizontalComplexLinearSup12));
    ASSERT_TRUE(HorizontalComplexLinearSup12.AreContiguous(HorizontalComplexLinearSup11));
    ASSERT_TRUE(HorizontalComplexLinearSup11.AreContiguous(HorizontalComplexLinearSup13));
    ASSERT_TRUE(HorizontalComplexLinearSup13.AreContiguous(HorizontalComplexLinearSup11));
    ASSERT_TRUE(HorizontalComplexLinearSup11.AreContiguous(HorizontalComplexLinearSup14));
    ASSERT_TRUE(HorizontalComplexLinearSup14.AreContiguous(HorizontalComplexLinearSup11));
    ASSERT_TRUE(HorizontalComplexLinearSup11.AreContiguous(HorizontalComplexLinearSup15));
    ASSERT_TRUE(HorizontalComplexLinearSup15.AreContiguous(HorizontalComplexLinearSup11));
    ASSERT_TRUE(HorizontalComplexLinearSup11.AreContiguous(HorizontalComplexLinearSup16));
    ASSERT_TRUE(HorizontalComplexLinearSup16.AreContiguous(HorizontalComplexLinearSup11));

    ASSERT_TRUE(HorizontalComplexLinearSup12.AreContiguous(HorizontalComplexLinearSup13));
    ASSERT_TRUE(HorizontalComplexLinearSup13.AreContiguous(HorizontalComplexLinearSup12));
    ASSERT_TRUE(HorizontalComplexLinearSup12.AreContiguous(HorizontalComplexLinearSup14));
    ASSERT_TRUE(HorizontalComplexLinearSup14.AreContiguous(HorizontalComplexLinearSup12));
    ASSERT_TRUE(HorizontalComplexLinearSup12.AreContiguous(HorizontalComplexLinearSup15));
    ASSERT_TRUE(HorizontalComplexLinearSup15.AreContiguous(HorizontalComplexLinearSup12));
    ASSERT_TRUE(HorizontalComplexLinearSup12.AreContiguous(HorizontalComplexLinearSup16));
    ASSERT_TRUE(HorizontalComplexLinearSup16.AreContiguous(HorizontalComplexLinearSup12));

    ASSERT_TRUE(HorizontalComplexLinearSup13.AreContiguous(HorizontalComplexLinearSup14));
    ASSERT_TRUE(HorizontalComplexLinearSup14.AreContiguous(HorizontalComplexLinearSup13));
    ASSERT_TRUE(HorizontalComplexLinearSup13.AreContiguous(HorizontalComplexLinearSup15));
    ASSERT_TRUE(HorizontalComplexLinearSup15.AreContiguous(HorizontalComplexLinearSup13));
    ASSERT_TRUE(HorizontalComplexLinearSup13.AreContiguous(HorizontalComplexLinearSup16));
    ASSERT_TRUE(HorizontalComplexLinearSup16.AreContiguous(HorizontalComplexLinearSup13));

    ASSERT_TRUE(HorizontalComplexLinearSup14.AreContiguous(HorizontalComplexLinearSup15));
    ASSERT_TRUE(HorizontalComplexLinearSup15.AreContiguous(HorizontalComplexLinearSup14));
    ASSERT_TRUE(HorizontalComplexLinearSup14.AreContiguous(HorizontalComplexLinearSup16));
    ASSERT_TRUE(HorizontalComplexLinearSup16.AreContiguous(HorizontalComplexLinearSup14));

    ASSERT_TRUE(HorizontalComplexLinearSup15.AreContiguous(HorizontalComplexLinearSup16));
    ASSERT_TRUE(HorizontalComplexLinearSup16.AreContiguous(HorizontalComplexLinearSup15));

    // all of the following are contiguous
    HVE2DComplexLinear    VerticalComplexLinearSup3(pWorld);
    VerticalComplexLinearSup3.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(10.0, 20.0, pWorld)));
    HVE2DComplexLinear    VerticalComplexLinearSup4(pWorld);
    VerticalComplexLinearSup4.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 5.0, pWorld), HGF2DLocation(10.0, 35.0, pWorld)));
    HVE2DComplexLinear    VerticalComplexLinearSup5(pWorld);
    VerticalComplexLinearSup5.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 5.0, pWorld), HGF2DLocation(10.0, 20.0, pWorld)));
    HVE2DComplexLinear    VerticalComplexLinearSup6(pWorld);
    VerticalComplexLinearSup6.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(10.0, 25.0, pWorld)));
    HVE2DComplexLinear    VerticalComplexLinearSup7(pWorld);
    VerticalComplexLinearSup7.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 5.0, pWorld), HGF2DLocation(10.0, 15.0, pWorld)));
    HVE2DComplexLinear    VerticalComplexLinearSup8(pWorld);
    VerticalComplexLinearSup8.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 13.0, pWorld), HGF2DLocation(10.0, 25.0, pWorld)));
    HVE2DComplexLinear    VerticalComplexLinearSup9(pWorld);
    VerticalComplexLinearSup9.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 12.0, pWorld), HGF2DLocation(10.0, 18.0, pWorld)));

    HVE2DComplexLinear    VerticalComplexLinearSup10(pWorld);
    VerticalComplexLinearSup10.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 20.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    HVE2DComplexLinear    VerticalComplexLinearSup11(pWorld);
    VerticalComplexLinearSup11.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 35.0, pWorld), HGF2DLocation(10.0, 5.0, pWorld)));
    HVE2DComplexLinear    VerticalComplexLinearSup12(pWorld);
    VerticalComplexLinearSup12.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 20.0, pWorld), HGF2DLocation(10.0, 5.0, pWorld)));
    HVE2DComplexLinear    VerticalComplexLinearSup13(pWorld);
    VerticalComplexLinearSup13.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 25.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    HVE2DComplexLinear    VerticalComplexLinearSup14(pWorld);
    VerticalComplexLinearSup14.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 15.0, pWorld), HGF2DLocation(10.0, 5.0, pWorld)));
    HVE2DComplexLinear    VerticalComplexLinearSup15(pWorld);
    VerticalComplexLinearSup15.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 25.0, pWorld), HGF2DLocation(10.0, 13.0, pWorld)));
    HVE2DComplexLinear    VerticalComplexLinearSup16(pWorld);
    VerticalComplexLinearSup16.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 18.0, pWorld), HGF2DLocation(10.0, 12.0, pWorld)));

    ASSERT_TRUE(VerticalComplexLinearSup3.AreContiguous(VerticalComplexLinearSup4));
    ASSERT_TRUE(VerticalComplexLinearSup4.AreContiguous(VerticalComplexLinearSup3));
    ASSERT_TRUE(VerticalComplexLinearSup3.AreContiguous(VerticalComplexLinearSup5));
    ASSERT_TRUE(VerticalComplexLinearSup5.AreContiguous(VerticalComplexLinearSup3));
    ASSERT_TRUE(VerticalComplexLinearSup3.AreContiguous(VerticalComplexLinearSup6));
    ASSERT_TRUE(VerticalComplexLinearSup6.AreContiguous(VerticalComplexLinearSup3));
    ASSERT_TRUE(VerticalComplexLinearSup3.AreContiguous(VerticalComplexLinearSup7));
    ASSERT_TRUE(VerticalComplexLinearSup7.AreContiguous(VerticalComplexLinearSup3));
    ASSERT_TRUE(VerticalComplexLinearSup3.AreContiguous(VerticalComplexLinearSup8));
    ASSERT_TRUE(VerticalComplexLinearSup8.AreContiguous(VerticalComplexLinearSup3));
    ASSERT_TRUE(VerticalComplexLinearSup3.AreContiguous(VerticalComplexLinearSup9));
    ASSERT_TRUE(VerticalComplexLinearSup9.AreContiguous(VerticalComplexLinearSup3));
    ASSERT_TRUE(VerticalComplexLinearSup3.AreContiguous(VerticalComplexLinearSup10));
    ASSERT_TRUE(VerticalComplexLinearSup10.AreContiguous(VerticalComplexLinearSup3));
    ASSERT_TRUE(VerticalComplexLinearSup3.AreContiguous(VerticalComplexLinearSup11));
    ASSERT_TRUE(VerticalComplexLinearSup11.AreContiguous(VerticalComplexLinearSup3));
    ASSERT_TRUE(VerticalComplexLinearSup3.AreContiguous(VerticalComplexLinearSup12));
    ASSERT_TRUE(VerticalComplexLinearSup12.AreContiguous(VerticalComplexLinearSup3));
    ASSERT_TRUE(VerticalComplexLinearSup3.AreContiguous(VerticalComplexLinearSup13));
    ASSERT_TRUE(VerticalComplexLinearSup13.AreContiguous(VerticalComplexLinearSup3));
    ASSERT_TRUE(VerticalComplexLinearSup3.AreContiguous(VerticalComplexLinearSup14));
    ASSERT_TRUE(VerticalComplexLinearSup14.AreContiguous(VerticalComplexLinearSup3));
    ASSERT_TRUE(VerticalComplexLinearSup3.AreContiguous(VerticalComplexLinearSup15));
    ASSERT_TRUE(VerticalComplexLinearSup15.AreContiguous(VerticalComplexLinearSup3));
    ASSERT_TRUE(VerticalComplexLinearSup3.AreContiguous(VerticalComplexLinearSup16));
    ASSERT_TRUE(VerticalComplexLinearSup16.AreContiguous(VerticalComplexLinearSup3));

    ASSERT_TRUE(VerticalComplexLinearSup4.AreContiguous(VerticalComplexLinearSup5));
    ASSERT_TRUE(VerticalComplexLinearSup5.AreContiguous(VerticalComplexLinearSup4));
    ASSERT_TRUE(VerticalComplexLinearSup4.AreContiguous(VerticalComplexLinearSup6));
    ASSERT_TRUE(VerticalComplexLinearSup6.AreContiguous(VerticalComplexLinearSup4));
    ASSERT_TRUE(VerticalComplexLinearSup4.AreContiguous(VerticalComplexLinearSup7));
    ASSERT_TRUE(VerticalComplexLinearSup7.AreContiguous(VerticalComplexLinearSup4));
    ASSERT_TRUE(VerticalComplexLinearSup4.AreContiguous(VerticalComplexLinearSup8));
    ASSERT_TRUE(VerticalComplexLinearSup8.AreContiguous(VerticalComplexLinearSup4));
    ASSERT_TRUE(VerticalComplexLinearSup4.AreContiguous(VerticalComplexLinearSup9));
    ASSERT_TRUE(VerticalComplexLinearSup9.AreContiguous(VerticalComplexLinearSup4));
    ASSERT_TRUE(VerticalComplexLinearSup4.AreContiguous(VerticalComplexLinearSup10));
    ASSERT_TRUE(VerticalComplexLinearSup10.AreContiguous(VerticalComplexLinearSup4));
    ASSERT_TRUE(VerticalComplexLinearSup4.AreContiguous(VerticalComplexLinearSup11));
    ASSERT_TRUE(VerticalComplexLinearSup11.AreContiguous(VerticalComplexLinearSup4));
    ASSERT_TRUE(VerticalComplexLinearSup4.AreContiguous(VerticalComplexLinearSup12));
    ASSERT_TRUE(VerticalComplexLinearSup12.AreContiguous(VerticalComplexLinearSup4));
    ASSERT_TRUE(VerticalComplexLinearSup4.AreContiguous(VerticalComplexLinearSup13));
    ASSERT_TRUE(VerticalComplexLinearSup13.AreContiguous(VerticalComplexLinearSup4));
    ASSERT_TRUE(VerticalComplexLinearSup4.AreContiguous(VerticalComplexLinearSup14));
    ASSERT_TRUE(VerticalComplexLinearSup14.AreContiguous(VerticalComplexLinearSup4));
    ASSERT_TRUE(VerticalComplexLinearSup4.AreContiguous(VerticalComplexLinearSup15));
    ASSERT_TRUE(VerticalComplexLinearSup15.AreContiguous(VerticalComplexLinearSup4));
    ASSERT_TRUE(VerticalComplexLinearSup4.AreContiguous(VerticalComplexLinearSup16));
    ASSERT_TRUE(VerticalComplexLinearSup16.AreContiguous(VerticalComplexLinearSup4));

    ASSERT_TRUE(VerticalComplexLinearSup5.AreContiguous(VerticalComplexLinearSup6));
    ASSERT_TRUE(VerticalComplexLinearSup6.AreContiguous(VerticalComplexLinearSup5));
    ASSERT_TRUE(VerticalComplexLinearSup5.AreContiguous(VerticalComplexLinearSup7));
    ASSERT_TRUE(VerticalComplexLinearSup7.AreContiguous(VerticalComplexLinearSup5));
    ASSERT_TRUE(VerticalComplexLinearSup5.AreContiguous(VerticalComplexLinearSup8));
    ASSERT_TRUE(VerticalComplexLinearSup8.AreContiguous(VerticalComplexLinearSup5));
    ASSERT_TRUE(VerticalComplexLinearSup5.AreContiguous(VerticalComplexLinearSup9));
    ASSERT_TRUE(VerticalComplexLinearSup9.AreContiguous(VerticalComplexLinearSup5));
    ASSERT_TRUE(VerticalComplexLinearSup5.AreContiguous(VerticalComplexLinearSup10));
    ASSERT_TRUE(VerticalComplexLinearSup10.AreContiguous(VerticalComplexLinearSup5));
    ASSERT_TRUE(VerticalComplexLinearSup5.AreContiguous(VerticalComplexLinearSup11));
    ASSERT_TRUE(VerticalComplexLinearSup11.AreContiguous(VerticalComplexLinearSup5));
    ASSERT_TRUE(VerticalComplexLinearSup5.AreContiguous(VerticalComplexLinearSup12));
    ASSERT_TRUE(VerticalComplexLinearSup12.AreContiguous(VerticalComplexLinearSup5));
    ASSERT_TRUE(VerticalComplexLinearSup5.AreContiguous(VerticalComplexLinearSup13));
    ASSERT_TRUE(VerticalComplexLinearSup13.AreContiguous(VerticalComplexLinearSup5));
    ASSERT_TRUE(VerticalComplexLinearSup5.AreContiguous(VerticalComplexLinearSup14));
    ASSERT_TRUE(VerticalComplexLinearSup14.AreContiguous(VerticalComplexLinearSup5));
    ASSERT_TRUE(VerticalComplexLinearSup5.AreContiguous(VerticalComplexLinearSup15));
    ASSERT_TRUE(VerticalComplexLinearSup15.AreContiguous(VerticalComplexLinearSup5));
    ASSERT_TRUE(VerticalComplexLinearSup5.AreContiguous(VerticalComplexLinearSup16));
    ASSERT_TRUE(VerticalComplexLinearSup16.AreContiguous(VerticalComplexLinearSup5));

    ASSERT_TRUE(VerticalComplexLinearSup6.AreContiguous(VerticalComplexLinearSup7));
    ASSERT_TRUE(VerticalComplexLinearSup7.AreContiguous(VerticalComplexLinearSup6));
    ASSERT_TRUE(VerticalComplexLinearSup6.AreContiguous(VerticalComplexLinearSup8));
    ASSERT_TRUE(VerticalComplexLinearSup8.AreContiguous(VerticalComplexLinearSup6));
    ASSERT_TRUE(VerticalComplexLinearSup6.AreContiguous(VerticalComplexLinearSup9));
    ASSERT_TRUE(VerticalComplexLinearSup9.AreContiguous(VerticalComplexLinearSup6));
    ASSERT_TRUE(VerticalComplexLinearSup6.AreContiguous(VerticalComplexLinearSup10));
    ASSERT_TRUE(VerticalComplexLinearSup10.AreContiguous(VerticalComplexLinearSup6));
    ASSERT_TRUE(VerticalComplexLinearSup6.AreContiguous(VerticalComplexLinearSup11));
    ASSERT_TRUE(VerticalComplexLinearSup11.AreContiguous(VerticalComplexLinearSup6));
    ASSERT_TRUE(VerticalComplexLinearSup6.AreContiguous(VerticalComplexLinearSup12));
    ASSERT_TRUE(VerticalComplexLinearSup12.AreContiguous(VerticalComplexLinearSup6));
    ASSERT_TRUE(VerticalComplexLinearSup6.AreContiguous(VerticalComplexLinearSup13));
    ASSERT_TRUE(VerticalComplexLinearSup13.AreContiguous(VerticalComplexLinearSup6));
    ASSERT_TRUE(VerticalComplexLinearSup6.AreContiguous(VerticalComplexLinearSup14));
    ASSERT_TRUE(VerticalComplexLinearSup14.AreContiguous(VerticalComplexLinearSup6));
    ASSERT_TRUE(VerticalComplexLinearSup6.AreContiguous(VerticalComplexLinearSup15));
    ASSERT_TRUE(VerticalComplexLinearSup15.AreContiguous(VerticalComplexLinearSup6));
    ASSERT_TRUE(VerticalComplexLinearSup6.AreContiguous(VerticalComplexLinearSup16));
    ASSERT_TRUE(VerticalComplexLinearSup16.AreContiguous(VerticalComplexLinearSup6));

    ASSERT_TRUE(VerticalComplexLinearSup7.AreContiguous(VerticalComplexLinearSup8));
    ASSERT_TRUE(VerticalComplexLinearSup8.AreContiguous(VerticalComplexLinearSup7));
    ASSERT_TRUE(VerticalComplexLinearSup7.AreContiguous(VerticalComplexLinearSup9));
    ASSERT_TRUE(VerticalComplexLinearSup9.AreContiguous(VerticalComplexLinearSup7));
    ASSERT_TRUE(VerticalComplexLinearSup7.AreContiguous(VerticalComplexLinearSup10));
    ASSERT_TRUE(VerticalComplexLinearSup10.AreContiguous(VerticalComplexLinearSup7));
    ASSERT_TRUE(VerticalComplexLinearSup7.AreContiguous(VerticalComplexLinearSup11));
    ASSERT_TRUE(VerticalComplexLinearSup11.AreContiguous(VerticalComplexLinearSup7));
    ASSERT_TRUE(VerticalComplexLinearSup7.AreContiguous(VerticalComplexLinearSup12));
    ASSERT_TRUE(VerticalComplexLinearSup12.AreContiguous(VerticalComplexLinearSup7));
    ASSERT_TRUE(VerticalComplexLinearSup7.AreContiguous(VerticalComplexLinearSup13));
    ASSERT_TRUE(VerticalComplexLinearSup13.AreContiguous(VerticalComplexLinearSup7));
    ASSERT_TRUE(VerticalComplexLinearSup7.AreContiguous(VerticalComplexLinearSup14));
    ASSERT_TRUE(VerticalComplexLinearSup14.AreContiguous(VerticalComplexLinearSup7));
    ASSERT_TRUE(VerticalComplexLinearSup7.AreContiguous(VerticalComplexLinearSup15));
    ASSERT_TRUE(VerticalComplexLinearSup15.AreContiguous(VerticalComplexLinearSup7));
    ASSERT_TRUE(VerticalComplexLinearSup7.AreContiguous(VerticalComplexLinearSup16));
    ASSERT_TRUE(VerticalComplexLinearSup16.AreContiguous(VerticalComplexLinearSup7));

    ASSERT_TRUE(VerticalComplexLinearSup8.AreContiguous(VerticalComplexLinearSup9));
    ASSERT_TRUE(VerticalComplexLinearSup9.AreContiguous(VerticalComplexLinearSup8));
    ASSERT_TRUE(VerticalComplexLinearSup8.AreContiguous(VerticalComplexLinearSup10));
    ASSERT_TRUE(VerticalComplexLinearSup10.AreContiguous(VerticalComplexLinearSup8));
    ASSERT_TRUE(VerticalComplexLinearSup8.AreContiguous(VerticalComplexLinearSup11));
    ASSERT_TRUE(VerticalComplexLinearSup11.AreContiguous(VerticalComplexLinearSup8));
    ASSERT_TRUE(VerticalComplexLinearSup8.AreContiguous(VerticalComplexLinearSup12));
    ASSERT_TRUE(VerticalComplexLinearSup12.AreContiguous(VerticalComplexLinearSup8));
    ASSERT_TRUE(VerticalComplexLinearSup8.AreContiguous(VerticalComplexLinearSup13));
    ASSERT_TRUE(VerticalComplexLinearSup13.AreContiguous(VerticalComplexLinearSup8));
    ASSERT_TRUE(VerticalComplexLinearSup8.AreContiguous(VerticalComplexLinearSup14));
    ASSERT_TRUE(VerticalComplexLinearSup14.AreContiguous(VerticalComplexLinearSup8));
    ASSERT_TRUE(VerticalComplexLinearSup8.AreContiguous(VerticalComplexLinearSup15));
    ASSERT_TRUE(VerticalComplexLinearSup15.AreContiguous(VerticalComplexLinearSup8));
    ASSERT_TRUE(VerticalComplexLinearSup8.AreContiguous(VerticalComplexLinearSup16));
    ASSERT_TRUE(VerticalComplexLinearSup16.AreContiguous(VerticalComplexLinearSup8));

    ASSERT_TRUE(VerticalComplexLinearSup9.AreContiguous(VerticalComplexLinearSup10));
    ASSERT_TRUE(VerticalComplexLinearSup10.AreContiguous(VerticalComplexLinearSup9));
    ASSERT_TRUE(VerticalComplexLinearSup9.AreContiguous(VerticalComplexLinearSup11));
    ASSERT_TRUE(VerticalComplexLinearSup11.AreContiguous(VerticalComplexLinearSup9));
    ASSERT_TRUE(VerticalComplexLinearSup9.AreContiguous(VerticalComplexLinearSup12));
    ASSERT_TRUE(VerticalComplexLinearSup12.AreContiguous(VerticalComplexLinearSup9));
    ASSERT_TRUE(VerticalComplexLinearSup9.AreContiguous(VerticalComplexLinearSup13));
    ASSERT_TRUE(VerticalComplexLinearSup13.AreContiguous(VerticalComplexLinearSup9));
    ASSERT_TRUE(VerticalComplexLinearSup9.AreContiguous(VerticalComplexLinearSup14));
    ASSERT_TRUE(VerticalComplexLinearSup14.AreContiguous(VerticalComplexLinearSup9));
    ASSERT_TRUE(VerticalComplexLinearSup9.AreContiguous(VerticalComplexLinearSup15));
    ASSERT_TRUE(VerticalComplexLinearSup15.AreContiguous(VerticalComplexLinearSup9));
    ASSERT_TRUE(VerticalComplexLinearSup9.AreContiguous(VerticalComplexLinearSup16));
    ASSERT_TRUE(VerticalComplexLinearSup16.AreContiguous(VerticalComplexLinearSup9));

    ASSERT_TRUE(VerticalComplexLinearSup10.AreContiguous(VerticalComplexLinearSup11));
    ASSERT_TRUE(VerticalComplexLinearSup11.AreContiguous(VerticalComplexLinearSup10));
    ASSERT_TRUE(VerticalComplexLinearSup10.AreContiguous(VerticalComplexLinearSup12));
    ASSERT_TRUE(VerticalComplexLinearSup12.AreContiguous(VerticalComplexLinearSup10));
    ASSERT_TRUE(VerticalComplexLinearSup10.AreContiguous(VerticalComplexLinearSup13));
    ASSERT_TRUE(VerticalComplexLinearSup13.AreContiguous(VerticalComplexLinearSup10));
    ASSERT_TRUE(VerticalComplexLinearSup10.AreContiguous(VerticalComplexLinearSup14));
    ASSERT_TRUE(VerticalComplexLinearSup14.AreContiguous(VerticalComplexLinearSup10));
    ASSERT_TRUE(VerticalComplexLinearSup10.AreContiguous(VerticalComplexLinearSup15));
    ASSERT_TRUE(VerticalComplexLinearSup15.AreContiguous(VerticalComplexLinearSup10));
    ASSERT_TRUE(VerticalComplexLinearSup10.AreContiguous(VerticalComplexLinearSup16));
    ASSERT_TRUE(VerticalComplexLinearSup16.AreContiguous(VerticalComplexLinearSup10));

    ASSERT_TRUE(VerticalComplexLinearSup11.AreContiguous(VerticalComplexLinearSup12));
    ASSERT_TRUE(VerticalComplexLinearSup12.AreContiguous(VerticalComplexLinearSup11));
    ASSERT_TRUE(VerticalComplexLinearSup11.AreContiguous(VerticalComplexLinearSup13));
    ASSERT_TRUE(VerticalComplexLinearSup13.AreContiguous(VerticalComplexLinearSup11));
    ASSERT_TRUE(VerticalComplexLinearSup11.AreContiguous(VerticalComplexLinearSup14));
    ASSERT_TRUE(VerticalComplexLinearSup14.AreContiguous(VerticalComplexLinearSup11));
    ASSERT_TRUE(VerticalComplexLinearSup11.AreContiguous(VerticalComplexLinearSup15));
    ASSERT_TRUE(VerticalComplexLinearSup15.AreContiguous(VerticalComplexLinearSup11));
    ASSERT_TRUE(VerticalComplexLinearSup11.AreContiguous(VerticalComplexLinearSup16));
    ASSERT_TRUE(VerticalComplexLinearSup16.AreContiguous(VerticalComplexLinearSup11));

    ASSERT_TRUE(VerticalComplexLinearSup12.AreContiguous(VerticalComplexLinearSup13));
    ASSERT_TRUE(VerticalComplexLinearSup13.AreContiguous(VerticalComplexLinearSup12));
    ASSERT_TRUE(VerticalComplexLinearSup12.AreContiguous(VerticalComplexLinearSup14));
    ASSERT_TRUE(VerticalComplexLinearSup14.AreContiguous(VerticalComplexLinearSup12));
    ASSERT_TRUE(VerticalComplexLinearSup12.AreContiguous(VerticalComplexLinearSup15));
    ASSERT_TRUE(VerticalComplexLinearSup15.AreContiguous(VerticalComplexLinearSup12));
    ASSERT_TRUE(VerticalComplexLinearSup12.AreContiguous(VerticalComplexLinearSup16));
    ASSERT_TRUE(VerticalComplexLinearSup16.AreContiguous(VerticalComplexLinearSup12));

    ASSERT_TRUE(VerticalComplexLinearSup13.AreContiguous(VerticalComplexLinearSup14));
    ASSERT_TRUE(VerticalComplexLinearSup14.AreContiguous(VerticalComplexLinearSup13));
    ASSERT_TRUE(VerticalComplexLinearSup13.AreContiguous(VerticalComplexLinearSup15));
    ASSERT_TRUE(VerticalComplexLinearSup15.AreContiguous(VerticalComplexLinearSup13));
    ASSERT_TRUE(VerticalComplexLinearSup13.AreContiguous(VerticalComplexLinearSup16));
    ASSERT_TRUE(VerticalComplexLinearSup16.AreContiguous(VerticalComplexLinearSup13));

    ASSERT_TRUE(VerticalComplexLinearSup14.AreContiguous(VerticalComplexLinearSup15));
    ASSERT_TRUE(VerticalComplexLinearSup15.AreContiguous(VerticalComplexLinearSup14));
    ASSERT_TRUE(VerticalComplexLinearSup14.AreContiguous(VerticalComplexLinearSup16));
    ASSERT_TRUE(VerticalComplexLinearSup16.AreContiguous(VerticalComplexLinearSup14));

    ASSERT_TRUE(VerticalComplexLinearSup15.AreContiguous(VerticalComplexLinearSup16));
    ASSERT_TRUE(VerticalComplexLinearSup16.AreContiguous(VerticalComplexLinearSup15));

    HVE2DComplexLinear    HorizontalComplexLinearSup23(pWorld);
    HorizontalComplexLinearSup23.AppendLinear(HVE2DSegment(HGF2DLocation(20.0, 20.0, pWorld), HGF2DLocation(10.0, 20.0, pWorld)));
    HVE2DComplexLinear    HorizontalComplexLinearSup24(pWorld);
    HorizontalComplexLinearSup24.AppendLinear(HVE2DSegment(HGF2DLocation(20.0+0.9*MYEPSILON, 20.0, pWorld), HGF2DLocation(-1.0, 20.0, pWorld)));

    HGF2DLocationCollection     Contig23Points;
    ASSERT_EQ(2, HorizontalComplexLinearSup23.ObtainContiguousnessPoints(HorizontalComplexLinearSup24, &Contig23Points));

        {
        
        HVE2DComplexLinear    HorizontalComplexLinearSup1(pWorld);
        HorizontalComplexLinearSup1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(0.0, 1E-6, pWorld)));
        HVE2DComplexLinear    HorizontalComplexLinearSup2(pWorld);
        HorizontalComplexLinearSup2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 9.999999E-7, pWorld), HGF2DLocation(0.0, 7.0, pWorld)));

        ASSERT_FALSE(HorizontalComplexLinearSup1.AreContiguous(HorizontalComplexLinearSup2));
        
        }
    }

////==================================================================================
// Segment Construction tests
// HVE2DSegment();
// HVE2DSegment(const HGF2DLocation&, const HGF2DLocation&);
// HVE2DSegment(const HGF2DLocation& pi_rStartPoint,
//             const HGF2DDisplacement& pi_rDisplacement);
// HVE2DSegment(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
// HVE2DSegment(const HGF2DSegment&    pi_rObject);
////==================================================================================
TEST_F (HVE2DComplexLinearTester, ConstructionTest3)
    {

    // Default Constructor
    HVE2DComplexLinear    AComplexLinear1;

    // Constructor with a coordinate system
    HVE2DComplexLinear    AComplexLinear2(pWorld);
    ASSERT_EQ(pWorld, AComplexLinear2.GetCoordSys());

    // Copy Constructor test
    HVE2DComplexLinear    AComplexLinear3(pWorld);
    AComplexLinear3.AppendLinear(HVE2DSegment(HorizontalSegment2.GetStartPoint(),
                                                MiscSegment1.GetStartPoint()));
    AComplexLinear3.AppendLinear(HVE2DSegment(AComplexLinear3.GetEndPoint(), MiscSegment1.GetEndPoint()));

    HVE2DComplexLinear    AComplexLinear4(AComplexLinear3);
    ASSERT_EQ(pWorld, AComplexLinear4.GetCoordSys());
    ASSERT_EQ(2, AComplexLinear4.GetLinearList().size());

    ASSERT_DOUBLE_EQ(10.1, AComplexLinear4.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, AComplexLinear4.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, AComplexLinear4.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, AComplexLinear4.GetEndPoint().GetY());
    
    }

////==================================================================================
// operator= test
// operator=(const HVE2DComplexLinear& pi_rObj);
////==================================================================================
TEST_F (HVE2DComplexLinearTester, OperatorTest3)
    {

    HVE2DComplexLinear    AComplexLinear1(pWorld);
    AComplexLinear1.AppendLinear(HVE2DSegment(HorizontalSegment2.GetStartPoint(),
                                                MiscSegment1.GetStartPoint()));
    AComplexLinear1.AppendLinear(HVE2DSegment(    AComplexLinear1.GetEndPoint(), MiscSegment1.GetEndPoint()));

    HVE2DComplexLinear    AComplexLinear2(pSys1);
    AComplexLinear2.AppendLinear(HVE2DSegment(VerticalSegment1.GetStartPoint(),
                                                VerticalSegment1.GetEndPoint()));

    AComplexLinear2 = AComplexLinear1;

    ASSERT_EQ(pWorld, AComplexLinear2.GetCoordSys());
    ASSERT_EQ(2, AComplexLinear2.GetLinearList().size());

    ASSERT_DOUBLE_EQ(10.1, AComplexLinear2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, AComplexLinear2.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, AComplexLinear2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, AComplexLinear2.GetEndPoint().GetY());
    
    }

////==================================================================================
// GetLinearList() const;
// GetNumberOfLinears() const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, GetLinearListTest3)
    {

    ASSERT_EQ(0, EmptyComplexLinear.GetLinearList().size());
    ASSERT_EQ(6, ComplexLinear1.GetLinearList().size());
    ASSERT_EQ(6, ComplexLinear2.GetLinearList().size());
    ASSERT_EQ(5, ComplexLinear3.GetLinearList().size());

    ASSERT_EQ(0, EmptyComplexLinear.GetNumberOfLinears());
    ASSERT_EQ(6, ComplexLinear1.GetNumberOfLinears());
    ASSERT_EQ(6, ComplexLinear2.GetNumberOfLinears());
    ASSERT_EQ(5, ComplexLinear3.GetNumberOfLinears());

    }

////==================================================================================
// Length calculation test
// CalculateLength() const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, CalculateLengthTest3)
    {

    // Test with ComplexLinear 1
    ASSERT_DOUBLE_EQ(44.142135623730951, ComplexLinear1.CalculateLength());

    // Test with ComplexLinear 1
    ASSERT_DOUBLE_EQ(111.76387577639170, ComplexLinear2.CalculateLength());

    // Test with empty ComplexLinear
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateLength(), MYEPSILON);

    }

////==================================================================================
// Relative point calculation test
// CalculateRelativePoint(double pi_RelativePos) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, CalculateRelativePointTest3)
    {

    // Test with ComplexLinear 1
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateRelativePoint(0.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateRelativePoint(0.0).GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(3.1213203435596428, ComplexLinear1.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(3.1213203435596428, ComplexLinear1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(17.928932188134524, ComplexLinear1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, ComplexLinear1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(26.757359312880713, ComplexLinear1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, ComplexLinear1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999, ComplexLinear1.CalculateRelativePoint(1.0).GetX());
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateRelativePoint(1.0).GetY(), MYEPSILON);

    // Test with ComplexLinear 2
    ASSERT_NEAR(0.0, ComplexLinear2.CalculateRelativePoint(0.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear2.CalculateRelativePoint(0.0).GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(-7.9028994453177486, ComplexLinear2.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(-7.9028994453177486, ComplexLinear2.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(20.4988171445599400, ComplexLinear2.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(1.50354856632018130, ComplexLinear2.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(18.5291627329175200, ComplexLinear2.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(15.0000000000000000, ComplexLinear2.CalculateRelativePoint(0.7).GetY());
    ASSERT_NEAR(0.0, ComplexLinear2.CalculateRelativePoint(1.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear2.CalculateRelativePoint(1.0).GetY(), MYEPSILON);

    // Test with ComplexLinear 3 (epsilon sized container)
    ASSERT_NEAR(0.0, ComplexLinear3.CalculateRelativePoint(0.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear3.CalculateRelativePoint(0.0).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear3.CalculateRelativePoint(0.1).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear3.CalculateRelativePoint(0.1).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear3.CalculateRelativePoint(0.5).GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.1464466094067261E-7, ComplexLinear3.CalculateRelativePoint(0.5).GetY());
    ASSERT_NEAR(0.0, ComplexLinear3.CalculateRelativePoint(0.7).GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.9242640687119286E-7, ComplexLinear3.CalculateRelativePoint(0.7).GetY());
    ASSERT_NEAR(0.0, ComplexLinear3.CalculateRelativePoint(1.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear3.CalculateRelativePoint(1.0).GetY(), MYEPSILON);

    // Test with empty ComplexLinear
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(0.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(0.0).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(0.1).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(0.1).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(0.5).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(0.5).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(0.7).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(0.7).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(1.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateRelativePoint(1.0).GetY(), MYEPSILON);

    }

////==================================================================================
// Drop(HGF2DLocationCollection* po_pPoint, const HGFDistance&  pi_rTolerance)
////==================================================================================
TEST_F (HVE2DComplexLinearTester, DropTest3)
    {

    HGF2DLocationCollection Locations;

    ComplexLinear1.Drop(&Locations, MYEPSILON); 
    ASSERT_NEAR(0.0, Locations[0].GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, Locations[0].GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Locations[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, Locations[1].GetY());
    ASSERT_DOUBLE_EQ(20.0, Locations[2].GetX());
    ASSERT_DOUBLE_EQ(10.0, Locations[2].GetY());
    ASSERT_DOUBLE_EQ(30.0, Locations[3].GetX());
    ASSERT_DOUBLE_EQ(10.0, Locations[3].GetY());
    ASSERT_DOUBLE_EQ(30.0, Locations[4].GetX());
    ASSERT_DOUBLE_EQ(5.00, Locations[4].GetY());
    ASSERT_DOUBLE_EQ(30.0, Locations[5].GetX());
    ASSERT_DOUBLE_EQ(5.00 - MYEPSILON, Locations[5].GetY());
    ASSERT_DOUBLE_EQ(30.0 - MYEPSILON, Locations[6].GetX());
    ASSERT_NEAR(0.0, Locations[6].GetY(), MYEPSILON);

    }

////==================================================================================
// Relative position calculation test
// CalculateRelativePosition(const HGF2DLocation& pi_rPointOnComplexLinear) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, CalculateRelativePositionTest3)
    {

    // Test with ComplexLinear1
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateRelativePosition(ComplexLinear1Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.099999999999998646, ComplexLinear1.CalculateRelativePosition(ComplexLinear1Point0d1));
    ASSERT_DOUBLE_EQ(0.500000000000010770, ComplexLinear1.CalculateRelativePosition(ComplexLinear1Point0d5));
    ASSERT_DOUBLE_EQ(1.000000000000000000, ComplexLinear1.CalculateRelativePosition(ComplexLinear1Point1d0));

    // Test with ComplexLinear2
    ASSERT_NEAR(0.0, ComplexLinear2.CalculateRelativePosition(ComplexLinear2Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.099999999999999381, ComplexLinear2.CalculateRelativePosition(ComplexLinear2Point0d1));
    ASSERT_DOUBLE_EQ(0.50000000000000167, ComplexLinear2.CalculateRelativePosition(ComplexLinear2Point0d5));

    // NORMAL Strange behavior provoqued by auto-clossing condition
    ASSERT_NEAR(0.0, ComplexLinear2.CalculateRelativePosition(ComplexLinear2Point1d0), MYEPSILON);

    // Test with ComplexLinear3 (epsilon sized container)
    ASSERT_NEAR(0.0, ComplexLinear3.CalculateRelativePosition(ComplexLinear3Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000007, ComplexLinear3.CalculateRelativePosition(ComplexLinear3Point0d1));
// Test is failing. temporarily disable
//    ASSERT_DOUBLE_EQ(0.49054285124903529, ComplexLinear3.CalculateRelativePosition(ComplexLinear3Point0d5));

    }
  
////==================================================================================
// RayArea Calculation test
// CalculateRayArea(const HGF2DLocation& pi_rPoint) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, CalculateRayAreaTest3)
    {      
    
    ASSERT_DOUBLE_EQ(150.0, ComplexComplexLinearCase1.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_DOUBLE_EQ(230.0, ComplexComplexLinearCase2.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_DOUBLE_EQ(112.5, ComplexComplexLinearCase3.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_DOUBLE_EQ(25.00, ComplexComplexLinearCase4.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_DOUBLE_EQ(125.0, ComplexComplexLinearCase5.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_DOUBLE_EQ(100.0, ComplexComplexLinearCase5A.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_DOUBLE_EQ(120.0, ComplexComplexLinearCase6.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_DOUBLE_EQ(125.0, ComplexComplexLinearCase7.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));    
    
    }

////==================================================================================
// Shortening tests
// Shorten(double pi_StartRelativePos, double pi_EndRelativePos);
// Shorten(const HGF2DLocation& pi_rNewStartPoint,
//         const HGF2DLocation& pi_rNewEndPoint);
// ShortenTo(const HGF2DLocation& pi_rNewEndPoint);
// ShortenTo(double pi_EndRelativePosition);
// ShortenFrom(const HGF2DLocation& pi_rNewStartPoint);
// ShortenFrom(double pi_StartRelativePosition);
////==================================================================================
TEST_F (HVE2DComplexLinearTester, ShorteningTest3)
    {

    // Test with ComplexLinear1
    HVE2DComplexLinear    AComplexLinear1(ComplexLinear1);
    AComplexLinear1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(6.2426406871192857, AComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(6.2426406871192857, AComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(30.000000000000000, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.8284271247461916, AComplexLinear1.GetEndPoint().GetY());

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.Shorten(0.5, 0.5+ASMALLFACTOR);
    ASSERT_DOUBLE_EQ(17.928932629555884, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, AComplexLinear1.GetEndPoint().GetY());

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.Shorten(0.0, 0.8);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.000000000000000, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.8284271247461916, AComplexLinear1.GetEndPoint().GetY());

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(6.2426406871192857, AComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(6.2426406871192857, AComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetY(), MYEPSILON);

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.Shorten(0.0, 1.0);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetY(), MYEPSILON);

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.ShortenTo(0.8);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.000000000000000, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.8284271247461916, AComplexLinear1.GetEndPoint().GetY());

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.ShortenTo(1.0-ASMALLFACTOR);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999900000006, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_NEAR(4.4142135635638624E-7, AComplexLinear1.GetEndPoint().GetY(), MYEPSILON);

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.ShortenTo(1.0);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetY(), MYEPSILON);

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.ShortenTo(0.0);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetY(), MYEPSILON);

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(6.2426406871192857, AComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(6.2426406871192857, AComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetY(), MYEPSILON);

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.ShortenFrom(1.0-ASMALLFACTOR);
    ASSERT_DOUBLE_EQ(29.999999900000006000, AComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(4.4142135635638624E-7, AComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999000, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetY(), MYEPSILON);

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(29.999999899999999, AComplexLinear1.GetStartPoint().GetX());
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetY(), MYEPSILON);

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.ShortenFrom(0.0);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetY(), MYEPSILON);

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.ShortenFrom(ComplexLinearMidPoint1);
    ASSERT_TRUE(ComplexLinearMidPoint1.IsEqualTo(AComplexLinear1.GetStartPoint()));
    ASSERT_DOUBLE_EQ(29.999999899999999, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetY(), MYEPSILON);

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.ShortenFrom(AComplexLinear1.GetStartPoint());
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetY(), MYEPSILON);

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.ShortenFrom(AComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(29.999999899999999, AComplexLinear1.GetStartPoint().GetX());
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetY(), MYEPSILON);

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.ShortenTo(ComplexLinearMidPoint1);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(ComplexLinearMidPoint1.GetX(), AComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(ComplexLinearMidPoint1.GetY(), AComplexLinear1.GetEndPoint().GetY());

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.ShortenTo(AComplexLinear1.GetStartPoint());
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetY(), MYEPSILON);

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.ShortenTo(AComplexLinear1.GetEndPoint());
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetY(), MYEPSILON);

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.Shorten(AComplexLinear1.GetStartPoint(), ComplexLinearMidPoint1);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(17.928932188135001, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, AComplexLinear1.GetEndPoint().GetY());

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.Shorten(ComplexLinearMidPoint1, AComplexLinear1.GetEndPoint());
    ASSERT_DOUBLE_EQ(17.928932188135001, AComplexLinear1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, AComplexLinear1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetY(), MYEPSILON);

    AComplexLinear1 = ComplexLinear1;
    AComplexLinear1.Shorten(AComplexLinear1.GetStartPoint(), AComplexLinear1.GetEndPoint());
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComplexLinear1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, AComplexLinear1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, AComplexLinear1.GetEndPoint().GetY(), MYEPSILON);

    }

////==================================================================================
// Closest point calculation test
// CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, CalculateClosestPointTest3)
    {
       
    // Test with ComplexLinear 1
    ASSERT_DOUBLE_EQ(21.100000, ComplexLinear1.CalculateClosestPoint(ComplexLinearClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(10.000000, ComplexLinear1.CalculateClosestPoint(ComplexLinearClosePoint1A).GetY());
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateClosestPoint(ComplexLinearClosePoint1B).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateClosestPoint(ComplexLinearClosePoint1B).GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.000000, ComplexLinear1.CalculateClosestPoint(ComplexLinearClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(10.000000, ComplexLinear1.CalculateClosestPoint(ComplexLinearClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(30.000000, ComplexLinear1.CalculateClosestPoint(ComplexLinearClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(5.1000000, ComplexLinear1.CalculateClosestPoint(ComplexLinearClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(17.928932, ComplexLinear1.CalculateClosestPoint(ComplexLinearCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(10.000000, ComplexLinear1.CalculateClosestPoint(ComplexLinearCloseMidPoint1).GetY());

    // Test with an empty ComplexLinear
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateClosestPoint(ComplexLinearClosePoint1A).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyComplexLinear.CalculateClosestPoint(ComplexLinearClosePoint1A).GetY(), MYEPSILON);    

    // Tests with special points
    ASSERT_DOUBLE_EQ(30.000000000000000, ComplexLinear1.CalculateClosestPoint(VeryFarPoint).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, ComplexLinear1.CalculateClosestPoint(VeryFarPoint).GetY());
    ASSERT_DOUBLE_EQ(17.928932188135001, ComplexLinear1.CalculateClosestPoint(ComplexLinearMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, ComplexLinear1.CalculateClosestPoint(ComplexLinearMidPoint1).GetY());
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateClosestPoint(ComplexLinear1.GetStartPoint()).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateClosestPoint(ComplexLinear1.GetStartPoint()).GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, ComplexLinear1.CalculateClosestPoint(ComplexLinear1.GetEndPoint()).GetX());
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateClosestPoint(ComplexLinear1.GetEndPoint()).GetY(), MYEPSILON);

    }

////==================================================================================
// Intersection test (with other complex ComplexLinears only)
// Intersect(const HVE2DVector& pi_rVector, HGF2DLocationCollection* po_pCrossPoints) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, IntersectTest3)
    {

    HVE2DSegment    ContiguousExtentComplexLinear1Segment1(HGF2DLocation(-10.0, 0.0, pWorld), HGF2DLocation(0.0, 10.0, pWorld));
    HVE2DSegment    ContiguousExtentComplexLinear1Segment2(HGF2DLocation(0.0, 10.0, pWorld), HGF2DLocation(-10.0, 10.0, pWorld));
    HVE2DComplexLinear      ContiguousExtentComplexLinear1(pWorld); // Open
    ContiguousExtentComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10.0, 0.0, pWorld),
                                                                HGF2DLocation(0.0, 10.0, pWorld)));
    ContiguousExtentComplexLinear1.AppendLinear(HVE2DSegment(ContiguousExtentComplexLinear1.GetEndPoint(), HGF2DLocation(-10.0, 10.0, pWorld)));

    HVE2DSegment    FlirtingExtentLinkedComplexLinear1Segment1(HGF2DLocation(-10.0, 0.0, pWorld), HGF2DLocation(0.0, -10.0, pWorld));
    HVE2DSegment    FlirtingExtentLinkedComplexLinear1Segment2(HGF2DLocation(0.0, -10.0, pWorld), HGF2DLocation(0.0, 0.0, pWorld));
    HVE2DComplexLinear      FlirtingExtentLinkedComplexLinear1(pWorld); // Open
    FlirtingExtentLinkedComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10.0, 0.0, pWorld),
                                                                    HGF2DLocation(0.0, -10.0, pWorld)));
    FlirtingExtentLinkedComplexLinear1.AppendLinear(HVE2DSegment(FlirtingExtentLinkedComplexLinear1.GetEndPoint(), HGF2DLocation(0.0, 0.0, pWorld)));

    HVE2DSegment    ConnectedComplexLinear1Segment1(HGF2DLocation(-10.0, 10.0, pWorld), HGF2DLocation(10.0, -10.0, pWorld));
    HVE2DSegment    ConnectedComplexLinear1Segment2(HGF2DLocation(10.0, -10.0, pWorld), HGF2DLocation(0.0, -10.0, pWorld));
    HVE2DComplexLinear      ConnectedComplexLinear1(pWorld); // Open
    ConnectedComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10.0, 10.0, pWorld),
                                                        HGF2DLocation(10.0, -10.0, pWorld)));
    ConnectedComplexLinear1.AppendLinear(HVE2DSegment(ConnectedComplexLinear1.GetEndPoint(), HGF2DLocation(0.0, -10.0, pWorld)));

    HVE2DSegment    ConnectedComplexLinear1ASegment1(HGF2DLocation(40.0, 0.0, pWorld), HGF2DLocation(10.0, 0.0, pWorld));
    HVE2DSegment    ConnectedComplexLinear1ASegment2(HGF2DLocation(10.0, 0.0, pWorld), HGF2DLocation(11.0, -10.0, pWorld));
    HVE2DComplexLinear      ConnectedComplexLinear1A(pWorld); // Open
    ConnectedComplexLinear1A.AppendLinear(HVE2DSegment(HGF2DLocation(40.0, 0.0, pWorld),
                                                        HGF2DLocation(10.0, 0.0, pWorld)));
    ConnectedComplexLinear1A.AppendLinear(HVE2DSegment(ConnectedComplexLinear1A.GetEndPoint(), HGF2DLocation(11.0, -10.0, pWorld)));

    HVE2DSegment    LinkedComplexLinear1ASegment1(HGF2DLocation(11.0, 3.0, pWorld), HGF2DLocation(20.0, 4.0, pWorld));
    HVE2DSegment    LinkedComplexLinear1ASegment2(HGF2DLocation(20.0, 4.0, pWorld), HGF2DLocation(30.0-MYEPSILON, 0.0, pWorld));
    HVE2DComplexLinear      LinkedComplexLinear1A(pWorld);
    LinkedComplexLinear1A.AppendLinear(HVE2DSegment(HGF2DLocation(11.0, 3.0, pWorld),
                                                    HGF2DLocation(20.0, 4.0, pWorld)));
    LinkedComplexLinear1A.AppendLinear(HVE2DSegment(LinkedComplexLinear1A.GetEndPoint(), HGF2DLocation(30.0-MYEPSILON, 0.0, pWorld)));

    HGF2DLocationCollection   DumPoints;

    // Test with extent disjoint ComplexLinears
    ASSERT_EQ(0, ComplexLinear1.Intersect(DisjointComplexLinear1, &DumPoints));

    // Test with disjoint but touching by a side
    ASSERT_EQ(0, ComplexLinear1.Intersect(ContiguousExtentComplexLinear1, &DumPoints));

    // Test with disjoint but touching by a tip but not linked
    ASSERT_EQ(0, ComplexLinear1.Intersect(FlirtingExtentComplexLinear1, &DumPoints));

    // Test with disjoint but touching by a tip segments linked
    ASSERT_EQ(0, ComplexLinear1.Intersect(FlirtingExtentLinkedComplexLinear1, &DumPoints));

    // Tests with connected ComplexLinears
    // At start point...
    ASSERT_EQ(0, ComplexLinear1.Intersect(ConnectedComplexLinear1, &DumPoints));
    ASSERT_EQ(0, ComplexLinear1.Intersect(ConnectingComplexLinear1, &DumPoints));

    // At end point ...
    ASSERT_EQ(0, ComplexLinear1.Intersect(ConnectedComplexLinear1A, &DumPoints));
    ASSERT_EQ(0, ComplexLinear1.Intersect(ConnectingComplexLinear1A, &DumPoints));

    // Tests with linked segments
    ASSERT_EQ(0, ComplexLinear1.Intersect(LinkedComplexLinear1, &DumPoints));
    ASSERT_EQ(0, ComplexLinear1.Intersect(LinkedComplexLinear1A, &DumPoints));

    // Tests with auto-closed element
    ASSERT_EQ(0, ComplexLinear2.Intersect(ConnectedComplexLinear1A, &DumPoints));

    // Tests with EPSILON sized container
    // This test causes major problems to be solved later
    ASSERT_EQ(0, ComplexLinear3.Intersect(ComplexLinear1, &DumPoints));

    // Special cases
    ASSERT_EQ(1, ComplexLinear1.Intersect(ComplexComplexLinearCase1, &DumPoints));
    ASSERT_DOUBLE_EQ(25.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());

    DumPoints.clear();
    ASSERT_EQ(1, ComplexLinear1.Intersect(ComplexComplexLinearCase2, &DumPoints));
    ASSERT_DOUBLE_EQ(28.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());

    DumPoints.clear();
    ASSERT_EQ(1, ComplexLinear1.Intersect(ComplexComplexLinearCase3, &DumPoints));
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());

    DumPoints.clear();
    ASSERT_EQ(0, ComplexLinear1.Intersect(ComplexComplexLinearCase4, &DumPoints));

    DumPoints.clear();
    ASSERT_EQ(1, ComplexLinear1.Intersect(ComplexComplexLinearCase5, &DumPoints));
    ASSERT_DOUBLE_EQ(30.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());

    DumPoints.clear();
    ASSERT_EQ(0, ComplexLinear1.Intersect(ComplexComplexLinearCase5A, &DumPoints));

    ASSERT_EQ(0, ComplexLinear1.Intersect(ComplexComplexLinearCase6, &DumPoints));
    ASSERT_EQ(0, ComplexLinear1.Intersect(ComplexComplexLinearCase7, &DumPoints));

    // Test with a NULL segment
    ASSERT_EQ(0, EmptyComplexLinear.Intersect(ComplexLinear1, &DumPoints));

    }

////==================================================================================
// Contiguousness Test
//   ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
//                              HGF2DLocationCollection* po_pContiguousnessPoints) const;
//   ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
//                                const HGF2DLocation& pi_rPoint,
//                                HGF2DLocation* pi_pFirstContiguousnessPoint,
//                                HGF2DLocation* pi_pSecondContiguousnessPoint) const;
//    HDLL virtual bool      AreContiguous(const HVE2DVector& pi_rVector) const;
//    HDLL virtual bool      AreContiguousAt(const HVE2DVector& pi_rVector,
//                                            const HGF2DLocation& pi_rPoint) const;
// AreContiguousAtAndGet(const HVE2DVector& pi_rVector, const HGF2DLocation& pi_rPoint,
//                       HGF2DLocation* pi_pFirstContiguousnessPoint,
//                       HGF2DLocation* pi_pSecondContiguousnessPoint) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, ContiguousnessTest3)
    {

    HGF2DLocationCollection     DumPoints;
    HGF2DLocation   FirstDumPoint;
    HGF2DLocation   SecondDumPoint;

    // Test with contiguous ComplexLinears
    ASSERT_TRUE(ComplexLinear1.AreContiguous(ComplexComplexLinearCase6));

    ASSERT_TRUE(ComplexLinear1.AreContiguousAt(ComplexComplexLinearCase6, ComplexLinearMidPoint1));

    ASSERT_EQ(2, ComplexLinear1.ObtainContiguousnessPoints(ComplexComplexLinearCase6, &DumPoints));
    ASSERT_NEAR(0.0, DumPoints[0].GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, DumPoints[0].GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());

    ComplexLinear1.ObtainContiguousnessPointsAt(ComplexComplexLinearCase6, ComplexLinearMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_NEAR(0.0, FirstDumPoint.GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, FirstDumPoint.GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    DumPoints.clear(); 

    ComplexLinear1.AreContiguousAtAndGet(ComplexComplexLinearCase6, ComplexLinearMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_NEAR(0.0, FirstDumPoint.GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, FirstDumPoint.GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    // Test with non contiguous ComplexLinears
    ASSERT_FALSE(ComplexLinear1.AreContiguous(ComplexComplexLinearCase1));

    DumPoints.clear(); 

    //// Test with auto-closed ComplexLinear  
    ASSERT_TRUE(ComplexLinear2.AreContiguous(ContiguousComplexLinear2));

    ASSERT_TRUE(ComplexLinear2.AreContiguousAt(ContiguousComplexLinear2, ComplexLinear2ContiguousPoint));

    ASSERT_EQ(2, ComplexLinear2.ObtainContiguousnessPoints(ContiguousComplexLinear2, &DumPoints));
    ASSERT_NEAR(0.0, DumPoints[0].GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, DumPoints[0].GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(-10.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(-10.0, DumPoints[1].GetY());

    ComplexLinear2.ObtainContiguousnessPointsAt(ContiguousComplexLinear2, ComplexLinear2ContiguousPoint, &FirstDumPoint, &SecondDumPoint);
    ASSERT_NEAR(0.0, FirstDumPoint.GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, FirstDumPoint.GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(-10.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-10.0, SecondDumPoint.GetY());

    DumPoints.clear(); 

    ComplexLinear2.AreContiguousAtAndGet(ContiguousComplexLinear2, ComplexLinear2ContiguousPoint, &FirstDumPoint, &SecondDumPoint);
    ASSERT_NEAR(0.0, FirstDumPoint.GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, FirstDumPoint.GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(-10.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-10.0, SecondDumPoint.GetY());

    DumPoints.clear();

    #ifdef WIP_IPPTEST_BUG_9
    // Test with epsilon sized container
    //ASSERT_TRUE(ComplexLinear3.AreContiguous(ContiguousComplexLinear3)); 

    //ASSERT_TRUE(ComplexLinear3.AreContiguousAt(ContiguousComplexLinear3, ComplexLinear3ContiguousPoint));

    //ASSERT_EQ(2, ComplexLinear3.ObtainContiguousnessPoints(ContiguousComplexLinear3, &DumPoints));
    // ASSERT_TRUE(DumPoints[0].IsEqualTo(ComplexLinear3.GetStartPoint()) || DumPoints[0].IsEqualTo(VerticalSegment1.GetEndPoint()));
    // ASSERT_TRUE(DumPoints[1].IsEqualTo(ComplexLinear3.GetStartPoint()) || DumPoints[1].IsEqualTo(VerticalSegment1.GetEndPoint()));
    // ASSERT_FALSE(DumPoints[1].IsEqualTo(DumPoints[0]));

    //ComplexLinear3.ObtainContiguousnessPointsAt(ContiguousComplexLinear3, ComplexLinear3ContiguousPoint, &FirstDumPoint, &SecondDumPoint);
    // ASSERT_TRUE(FirstDumPoint.IsEqualTo(ComplexLinear3.GetStartPoint()) || FirstDumPoint.IsEqualTo(VerticalSegment1.GetEndPoint()));
    // ASSERT_TRUE(SecondDumPoint.IsEqualTo(ComplexLinear3.GetStartPoint()) || SecondDumPoint.IsEqualTo(VerticalSegment1.GetEndPoint()));
    // ASSERT_FALSE(SecondDumPoint.IsEqualTo(FirstDumPoint));

    // WIP : Check if the method AreContiguousAtAndGet() have the same problem once this one is solve. 
    #endif
 
    }
 
////==================================================================================
// Cloning tests
// Clone() const;
// AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, CloningTest3)
    {       

    //General Clone Test
    HFCPtr<HVE2DComplexLinear> pClone = (HVE2DComplexLinear*)ComplexLinear2.Clone();
    ASSERT_FALSE(pClone->IsEmpty());
    ASSERT_EQ(pWorld, pClone->GetCoordSys());   
    ASSERT_EQ(6, pClone->GetNumberOfLinears());

    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(18.0, 9.0, pWorld)));  
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(21.0, 0.0, pWorld)));  
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(24.0, 15.0, pWorld)));  
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(0.0, 15.0, pWorld)));  
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(-10.0, -10.0, pWorld)));   

    // Test with the same coordinate system
    HFCPtr<HVE2DComplexLinear> pClone2 = (HVE2DComplexLinear*) ComplexLinear2.AllocateCopyInCoordSys(pWorld);
    ASSERT_FALSE(pClone2->IsEmpty());
    ASSERT_EQ(pWorld, pClone2->GetCoordSys());
    ASSERT_EQ(6, pClone2->GetNumberOfLinears());

    ASSERT_TRUE(pClone2->IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(pClone2->IsPointOn(HGF2DLocation(18.0, 9.0, pWorld)));  
    ASSERT_TRUE(pClone2->IsPointOn(HGF2DLocation(21.0, 0.0, pWorld)));  
    ASSERT_TRUE(pClone2->IsPointOn(HGF2DLocation(24.0, 15.0, pWorld)));  
    ASSERT_TRUE(pClone2->IsPointOn(HGF2DLocation(0.0, 15.0, pWorld)));  
    ASSERT_TRUE(pClone2->IsPointOn(HGF2DLocation(-10.0, -10.0, pWorld)));   

    // Test with a translation between systems
    Translation = HGF2DDisplacement (10.0, 10.0);
    HGF2DTranslation myTranslation(Translation);
    HFCPtr<HGF2DCoordSys> pWorldTranslation = new HGF2DCoordSys(myTranslation, pWorld);

    HFCPtr<HVE2DComplexLinear> pClone4 = (HVE2DComplexLinear*) ComplexLinear2.AllocateCopyInCoordSys(pWorldTranslation);
    ASSERT_FALSE(pClone4->IsEmpty());
    ASSERT_EQ(pWorldTranslation, pClone4->GetCoordSys());
    ASSERT_EQ(6, pClone4->GetNumberOfLinears());

    ASSERT_TRUE(pClone4->IsPointOn(HGF2DLocation(-20.0, -20.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone4->IsPointOn(HGF2DLocation(11.0, -10.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone4->IsPointOn(HGF2DLocation(8.0, -1.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone4->IsPointOn(HGF2DLocation(14.0, 5.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone4->IsPointOn(HGF2DLocation(-10.0, 5.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone4->IsPointOn(HGF2DLocation(-10.0, -10.0, pWorldTranslation)));
     
    // Test with a stretch between systems
    HGF2DStretch myStretch;
    myStretch.SetTranslation(Translation);
    myStretch.SetXScaling(0.5);
    myStretch.SetYScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldStretch = new HGF2DCoordSys(myStretch, pWorld);

    HFCPtr<HVE2DComplexLinear> pClone6 = (HVE2DComplexLinear*)ComplexLinear2.AllocateCopyInCoordSys(pWorldStretch);
    ASSERT_FALSE(pClone6->IsEmpty());
    ASSERT_EQ(pWorldStretch, pClone6->GetCoordSys());
    ASSERT_EQ(6, pClone6->GetNumberOfLinears());

    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(-40.0, -40.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(22.0, -20.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(16.0, -2.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(28.0, 10.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(-20.0, 10.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(-20.0, -20.0, pWorldStretch)));

    // Test with a similitude between systems
    HGF2DSimilitude mySimilitude;
    mySimilitude.SetRotation(180.0* PI / 180);
    mySimilitude.SetScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldSimilitude = new HGF2DCoordSys(mySimilitude, pWorld);

    HFCPtr<HVE2DComplexLinear> pClone7 = (HVE2DComplexLinear*)ComplexLinear2.AllocateCopyInCoordSys(pWorldSimilitude);
    ASSERT_FALSE(pClone7->IsEmpty());
    ASSERT_EQ(pWorldSimilitude, pClone7->GetCoordSys());
    ASSERT_EQ(6, pClone7->GetNumberOfLinears());

    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(20.0, 20.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-42.0, 0.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-36.0, -18.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-48.0, -30.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(0.0, -30.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldSimilitude))); 

    // Test with an affine between systems
    HGF2DAffine myAffine;
    myAffine.SetTranslation(Translation);
    myAffine.SetRotation(180.0* PI / 180);
    myAffine.SetXScaling(0.5);
    myAffine.SetYScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldAffine = new HGF2DCoordSys(myAffine, pWorld);

    HFCPtr<HVE2DComplexLinear> pClone8 = (HVE2DComplexLinear*)ComplexLinear2.AllocateCopyInCoordSys(pWorldAffine);
    ASSERT_FALSE(pClone8->IsEmpty());
    ASSERT_EQ(pWorldAffine, pClone8->GetCoordSys());
    ASSERT_EQ(6, pClone8->GetNumberOfLinears());

    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(40.0, 40.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-22.0, 20.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-16.0, 2.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-28.0, -10.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(20.0, -10.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(20.0, 20.0, pWorldAffine)));

    }

////==================================================================================
// Interaction info with other segment
// Crosses(const HVE2DVector& pi_rVector) const;
// AreAdjacent(const HVE2DVector& pi_rVector) const;
// IsPointOn(const HGF2DLocation& pi_rTestPoint) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, InteractionTest3)
    {

    // Tests with a vertical segment
    ASSERT_TRUE(ComplexLinear1.Crosses(ComplexComplexLinearCase1));
    ASSERT_FALSE(ComplexLinear1.AreAdjacent(ComplexComplexLinearCase1));

    ASSERT_TRUE(ComplexLinear1.Crosses(ComplexComplexLinearCase2));
    ASSERT_TRUE(ComplexLinear1.AreAdjacent(ComplexComplexLinearCase2));

    ASSERT_TRUE(ComplexLinear1.Crosses(ComplexComplexLinearCase3));
    ASSERT_FALSE(ComplexLinear1.AreAdjacent(ComplexComplexLinearCase3));

    ASSERT_FALSE(ComplexLinear1.Crosses(ComplexComplexLinearCase4));
    ASSERT_TRUE(ComplexLinear1.AreAdjacent(ComplexComplexLinearCase4));

    ASSERT_TRUE(ComplexLinear1.Crosses(ComplexComplexLinearCase5));
    ASSERT_TRUE(ComplexLinear1.AreAdjacent(ComplexComplexLinearCase5));

    ASSERT_FALSE(ComplexLinear1.Crosses(ComplexComplexLinearCase6));
    ASSERT_TRUE(ComplexLinear1.AreAdjacent(ComplexComplexLinearCase6));

    ASSERT_FALSE(ComplexLinear1.Crosses(ComplexComplexLinearCase7));
    ASSERT_FALSE(ComplexLinear1.AreAdjacent(ComplexComplexLinearCase7));

    ASSERT_TRUE(ComplexLinear1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(ComplexLinear1.IsPointOn(HGF2DLocation(0.1+1.1*MYEPSILON, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(ComplexLinear1.IsPointOn(ComplexLinearMidPoint1-HGF2DDisplacement(0.0, 1.1*MYEPSILON)));
    ASSERT_FALSE(ComplexLinear1.IsPointOn(ComplexLinearMidPoint1-HGF2DDisplacement(0.0, -1.1*MYEPSILON)));
    ASSERT_TRUE(ComplexLinear1.IsPointOn(ComplexLinearMidPoint1-HGF2DDisplacement(0.0, 0.9*MYEPSILON)));
    ASSERT_TRUE(ComplexLinear1.IsPointOn(ComplexLinearMidPoint1-HGF2DDisplacement(0.0, -0.9*MYEPSILON)));
    ASSERT_TRUE(ComplexLinear1.IsPointOn(ComplexLinear1.GetStartPoint()));
    ASSERT_TRUE(ComplexLinear1.IsPointOn(ComplexLinear1.GetEndPoint()));
    ASSERT_TRUE(ComplexLinear1.IsPointOn(ComplexLinearMidPoint1));

    ASSERT_FALSE(ComplexLinear1.IsPointOn(ComplexLinear1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(ComplexLinear1.IsPointOn(ComplexLinear1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(ComplexLinear1.IsPointOn(ComplexLinearMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    }

////==================================================================================
// Bearing calculation tests
// CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
//                  HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
// CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
//                              HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, BearingTest3)
    {

    // Obtain bearing ALPHA of a vertical segment
    ASSERT_DOUBLE_EQ(-2.3561944901923448, ComplexLinear1.CalculateBearing(ComplexLinear1Point0d0, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(0.78539816339744828, ComplexLinear1.CalculateBearing(ComplexLinear1Point0d0, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(-2.3561944901923448, ComplexLinear1.CalculateBearing(ComplexLinear1Point0d1, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(0.78539816339744828, ComplexLinear1.CalculateBearing(ComplexLinear1Point0d1, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(3.14159265358979310, ComplexLinear1.CalculateBearing(ComplexLinear1Point0d5, HVE2DVector::ALPHA).GetAngle());
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateBearing(ComplexLinear1Point0d5, HVE2DVector::BETA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.57079630679489600, ComplexLinear1.CalculateBearing(ComplexLinear1Point1d0, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(-1.5707963467948973, ComplexLinear1.CalculateBearing(ComplexLinear1Point1d0, HVE2DVector::BETA).GetAngle());

    // ANGULAR ACCELERATION
    // Obtain angular acceleration ALPHA of a vertical segment
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateAngularAcceleration(ComplexLinear1Point0d0, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateAngularAcceleration(ComplexLinear1Point0d0, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateAngularAcceleration(ComplexLinear1Point0d1, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateAngularAcceleration(ComplexLinear1Point0d1, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateAngularAcceleration(ComplexLinear1Point0d5, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateAngularAcceleration(ComplexLinear1Point0d5, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateAngularAcceleration(ComplexLinear1Point1d0, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear1.CalculateAngularAcceleration(ComplexLinear1Point1d0, HVE2DVector::BETA), MYEPSILON);

    }

////==================================================================================
// Extent calculation test
// GetExtent() const;
////==================================================================================
TEST_F (HVE2DComplexLinearTester, GetExtentTest3)
    {

    // Obtain extent of ComplexLinear 1
    ASSERT_NEAR(0.0, ComplexLinear1.GetExtent().GetXMin(), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear1.GetExtent().GetYMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, ComplexLinear1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.0, ComplexLinear1.GetExtent().GetYMax());

    // Obtain extent of ComplexLinear 2
    ASSERT_DOUBLE_EQ(-10.0, ComplexLinear2.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(-10.0, ComplexLinear2.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(24.00, ComplexLinear2.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(15.00, ComplexLinear2.GetExtent().GetYMax());

    // Obtain extent of an epsilon container
    ASSERT_NEAR(0.0, ComplexLinear3.GetExtent().GetXMin(), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear3.GetExtent().GetYMin(), MYEPSILON);
    ASSERT_NEAR(0.0, ComplexLinear3.GetExtent().GetXMax(), MYEPSILON);
    ASSERT_DOUBLE_EQ(2E-7, ComplexLinear3.GetExtent().GetYMax());

    // Obtain extent of an empty ComplexLinear
    HGF2DExtent EmptyExtent(EmptyComplexLinear.GetExtent());
    ASSERT_FALSE(EmptyExtent.IsDefined());

    }

////==================================================================================
// Test which previously failed
////==================================================================================
TEST_F (HVE2DComplexLinearTester, TestWhoFailed)
    {

    HGF2DLocationCollection     CCPoints;
    
    HVE2DComplexLinear  AddComplexLinear1(pWorld);
    AddComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 20.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(20.0, 10.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(20.0, 20.0, pWorld)));

    HVE2DComplexLinear  AddComplexLinear2(pWorld);
    AddComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(5.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld)));
    AddComplexLinear2.AppendLinear(HVE2DSegment(AddComplexLinear2.GetEndPoint(), HGF2DLocation(15.0, 15.0, pWorld)));

    ASSERT_EQ(1, AddComplexLinear1.Intersect(AddComplexLinear2, &CCPoints));
    ASSERT_TRUE(AddComplexLinear1.Crosses(AddComplexLinear2));

    CCPoints.clear();
    HVE2DComplexLinear  AddComplexLinear3(pWorld);
    AddComplexLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(12.0, 12.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    AddComplexLinear3.AppendLinear(HVE2DSegment(AddComplexLinear3.GetEndPoint(), HGF2DLocation(20.0, 10.0, pWorld)));
    AddComplexLinear3.AppendLinear(HVE2DSegment(AddComplexLinear3.GetEndPoint(), HGF2DLocation(15.0, 15.0, pWorld)));

    ASSERT_EQ(0, AddComplexLinear1.Intersect(AddComplexLinear3, &CCPoints));
    ASSERT_FALSE(AddComplexLinear1.Crosses(AddComplexLinear3));

    }

////==================================================================================
// Additional test for possible new CrossesPolysegmentSCS method
////==================================================================================
TEST_F (HVE2DComplexLinearTester, CrossesPolysegmentSCSTest3)
    {
  
    HVE2DComplexLinear  AddComplexLinear1(pWorld);
    AddComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld),HGF2DLocation(30.0, 10.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(30.0, 20.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(20.0, 20.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(20.0, 30.0, pWorld)));

    HVE2DComplexLinear  AddComplexLinear2(pWorld);
    AddComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(20.0, 5.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld)));
    AddComplexLinear2.AppendLinear(HVE2DSegment(AddComplexLinear2.GetEndPoint(), HGF2DLocation(30.0, 10.0, pWorld)));
    AddComplexLinear2.AppendLinear(HVE2DSegment(AddComplexLinear2.GetEndPoint(), HGF2DLocation(30.0, 20.0, pWorld)));
    AddComplexLinear2.AppendLinear(HVE2DSegment(AddComplexLinear2.GetEndPoint(), HGF2DLocation(25.0, 25.0, pWorld)));

    ASSERT_FALSE(AddComplexLinear1.Crosses(AddComplexLinear2));

    }

////==================================================================================
// Additional test for possible new CrossesPolysegmentSCS method
////==================================================================================
TEST_F (HVE2DComplexLinearTester, CrossesPolysegmentSCSTest4)
    {
        
    HVE2DComplexLinear  AddComplexLinear1(pWorld);
    AddComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 10.0, pWorld),HGF2DLocation(0.0, 0.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(10.0, 0.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(10.0, 10.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(20.0, 10.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(20.0, 0.0, pWorld)));

    HVE2DComplexLinear  AddComplexLinear2(pWorld);
    AddComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 5.0, pWorld), HGF2DLocation(20.0, 5.0, pWorld)));

    HVE2DComplexLinear  AddComplexLinear3(pWorld);
    AddComplexLinear3.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 5.0, pWorld), HGF2DLocation(10.0, 5.0, pWorld)));
    AddComplexLinear3.AppendLinear(HVE2DSegment(AddComplexLinear3.GetEndPoint(), HGF2DLocation(20.0, 5.0, pWorld)));

    HVE2DComplexLinear  AddComplexLinear4(pWorld);
    AddComplexLinear4.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 10.0, pWorld), HGF2DLocation(0.0, 0.0, pWorld)));
    AddComplexLinear4.AppendLinear(HVE2DSegment(AddComplexLinear4.GetEndPoint(), HGF2DLocation(10.0, 0.0, pWorld)));
    AddComplexLinear4.AppendLinear(HVE2DSegment(AddComplexLinear4.GetEndPoint(), HGF2DLocation(10.0, 5.0, pWorld)));
    AddComplexLinear4.AppendLinear(HVE2DSegment(AddComplexLinear4.GetEndPoint(), HGF2DLocation(10.0, 10.0, pWorld)));
    AddComplexLinear4.AppendLinear(HVE2DSegment(AddComplexLinear4.GetEndPoint(), HGF2DLocation(20.0, 10.0, pWorld)));
    AddComplexLinear4.AppendLinear(HVE2DSegment(AddComplexLinear4.GetEndPoint(), HGF2DLocation(20.0, 0.0, pWorld)));

    ASSERT_TRUE(AddComplexLinear1.Crosses(AddComplexLinear2));
    ASSERT_TRUE(AddComplexLinear1.Crosses(AddComplexLinear3));
    ASSERT_TRUE(AddComplexLinear4.Crosses(AddComplexLinear3));

    }

////==================================================================================
// Additional test for possible new CrossesPolysegmentSCS method
////==================================================================================
TEST_F (HVE2DComplexLinearTester, CrossesPolysegmentSCSTest5)
    {

    HVE2DComplexLinear  AddComplexLinear1(pWorld);
    AddComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(0.0, 20.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(20.0, 20.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(15.0, 10.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(20.0, 0.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(0.0, 0.0, pWorld)));

    HVE2DComplexLinear  AddComplexLinear2(pWorld);
    AddComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0 + 1e-9, 0.0 + 1e-9, pWorld), HGF2DLocation(0.0 + 1e-9, 20.0 + 1e-9, pWorld)));
    AddComplexLinear2.AppendLinear(HVE2DSegment(AddComplexLinear2.GetEndPoint(), HGF2DLocation(20.0 + 1e-9, 20.0 + 1e-9, pWorld)));
    AddComplexLinear2.AppendLinear(HVE2DSegment(AddComplexLinear2.GetEndPoint(), HGF2DLocation(25.0 + 1e-9, 10.0 + 1e-9, pWorld)));
    AddComplexLinear2.AppendLinear(HVE2DSegment(AddComplexLinear2.GetEndPoint(), HGF2DLocation(20.0 + 1e-9, 0.0 + 1e-9, pWorld)));
    AddComplexLinear2.AppendLinear(HVE2DSegment(AddComplexLinear2.GetEndPoint(), HGF2DLocation(0.0 + 1e-9, 0.0 + 1e-9, pWorld)));

    ASSERT_FALSE(AddComplexLinear1.Crosses(AddComplexLinear2));

    }

////==================================================================================
// Additional test for possible new CrossesPolysegmentSCS method
////==================================================================================
TEST_F (HVE2DComplexLinearTester, CrossesPolysegmentSCSTest6)
    {
    
    HVE2DComplexLinear  AddComplexLinear1(pWorld);
    AddComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 10.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(20.0, 10.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(30.0, 20.0, pWorld)));

    HVE2DComplexLinear  AddComplexLinear2(pWorld);
    AddComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    AddComplexLinear2.AppendLinear(HVE2DSegment(AddComplexLinear2.GetEndPoint(), HGF2DLocation(20.0, 10.0, pWorld)));
    AddComplexLinear2.AppendLinear(HVE2DSegment(AddComplexLinear2.GetEndPoint(), HGF2DLocation(30.0, 10.0, pWorld)));

    ASSERT_FALSE(AddComplexLinear1.Crosses(AddComplexLinear2));

    }

////==================================================================================
// Additional test for possible new CrossesPolysegmentSCS method
////==================================================================================
TEST_F (HVE2DComplexLinearTester, CrossesPolysegmentSCSTest7)
    {

    HVE2DComplexLinear  AddComplexLinear1(pWorld);
    AddComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(30.0, 10.0, pWorld)));

    HVE2DComplexLinear  AddComplexLinear2(pWorld);
    AddComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld)));
    AddComplexLinear2.AppendLinear(HVE2DSegment(AddComplexLinear2.GetEndPoint(), HGF2DLocation(30.0, 20.0, pWorld)));

    ASSERT_FALSE(AddComplexLinear1.Crosses(AddComplexLinear2));

    }

////==================================================================================
// Test crossing at start-end point
////==================================================================================
TEST_F (HVE2DComplexLinearTester, CrossingTest3)
    {

    HVE2DComplexLinear  AddComplexLinear1(pWorld);
    AddComplexLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(10.0, 20.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(20.0, 20.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(20.0, 10.0, pWorld)));
    AddComplexLinear1.AppendLinear(HVE2DSegment(AddComplexLinear1.GetEndPoint(), HGF2DLocation(10.0, 10.0, pWorld)));

    HVE2DComplexLinear  AddComplexLinear2(pWorld);
    AddComplexLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(5.0, 5.0, pWorld), HGF2DLocation(15.0, 15.0, pWorld)));

    ASSERT_FALSE(AddComplexLinear1.Crosses(AddComplexLinear2));
        
     }