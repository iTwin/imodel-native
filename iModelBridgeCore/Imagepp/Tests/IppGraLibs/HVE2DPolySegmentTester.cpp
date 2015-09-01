//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HVE2DPolySegmentTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HVE2DPolySegmentTester.h"

// THE PRESENT TEST CANNOT MAKE TESTS WITH NON-LINEAR TRANSFORMATION MODELS

HVE2DPolySegmentTester::HVE2DPolySegmentTester() 
    {
    
    // VERTICAL PolySegment
    VerticalPolySegment1 = HVE2DPolySegment(pWorld);
    VerticalPolySegment1.AppendPoint(HGF2DLocation(0.1, 0.1, pWorld));
    VerticalPolySegment1.AppendPoint(HGF2DLocation(0.1, 10.1, pWorld));

    VerticalPolySegment2 = HVE2DPolySegment(pWorld);
    VerticalPolySegment2.AppendPoint(HGF2DLocation(0.1, 10.1, pWorld));
    VerticalPolySegment2.AppendPoint(HGF2DLocation(0.1, 0.1, pWorld));

    VerticalPolySegment3 = HVE2DPolySegment(pWorld);
    VerticalPolySegment3.AppendPoint(HGF2DLocation(0.1, 0.1, pWorld));
    VerticalPolySegment3.AppendPoint(HGF2DLocation(0.1, 0.1 + MYEPSILON, pWorld));

    VerticalPolySegment4 = HVE2DPolySegment(pWorld);
    VerticalPolySegment4.AppendPoint(HGF2DLocation(0.1 + MYEPSILON, 0.1, pWorld));
    VerticalPolySegment4.AppendPoint(HGF2DLocation(0.1+MYEPSILON, 10.1, pWorld));

    VerticalPolySegment5 = HVE2DPolySegment(HGF2DLocation(-10.0, -10.0, pWorld), HGF2DLocation(-10.0, 0.0, pWorld));
    CloseVerticalPolySegment1 = HVE2DPolySegment(HGF2DLocation(0.1+MYEPSILON, 0.1, pWorld), HGF2DLocation(0.1, 10.1, pWorld));

    VerticalPoint0d0 = HGF2DLocation(0.1, 0.1, pWorld);
    VerticalPoint0d1 = HGF2DLocation(0.1, 1.1, pWorld);
    VerticalPoint0d5 = HGF2DLocation(0.1, 5.1, pWorld);
    VerticalPoint1d0 = HGF2DLocation(0.1, 10.1, pWorld);

    Vertical3Point0d0 = HGF2DLocation(0.1, 0.1, pWorld);
    Vertical3Point0d1 = HGF2DLocation(0.1, 0.1 + (0.1 * MYEPSILON), pWorld);
    Vertical3Point0d5 = HGF2DLocation(0.1, 0.1 + (0.5 * MYEPSILON), pWorld);
    Vertical3Point1d0 = HGF2DLocation(0.1, 0.1 + MYEPSILON, pWorld);

    VerticalMidPoint1 = HGF2DLocation(0.1, 5.1, pWorld);
    VerticalMidPoint3 = HGF2DLocation(0.1, 0.1 + (MYEPSILON/2), pWorld);

    VerticalClosePoint1A = HGF2DLocation(0.0, 0.0, pWorld);
    VerticalClosePoint1B = HGF2DLocation(0.0, 5.0, pWorld);
    VerticalClosePoint1C = HGF2DLocation(0.2, 7.0, pWorld);
    VerticalClosePoint1D = HGF2DLocation(1000000.0, 10.0, pWorld);
    VerticalCloseMidPoint1 = HGF2DLocation(0.0, 5.1, pWorld);

    VerticalClosePoint3A = HGF2DLocation(0.0, 0.0, pWorld);
    VerticalClosePoint3B = HGF2DLocation(0.0, 0.1 + (2*MYEPSILON/3), pWorld);
    VerticalClosePoint3C = HGF2DLocation(0.2, 0.1 + (MYEPSILON/3), pWorld);
    VerticalClosePoint3D = HGF2DLocation(1000000.0, 0.1 + (MYEPSILON/2), pWorld);
    VerticalCloseMidPoint3 = HGF2DLocation(0.0, 0.1 + (MYEPSILON/2), pWorld);

    // HORIZONTAL PolySegment
    HorizontalPolySegment1 = HVE2DPolySegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(10.1, 0.1, pWorld));
    HorizontalPolySegment2 = HVE2DPolySegment(HGF2DLocation(10.1, 0.1, pWorld), HGF2DLocation(0.1, 0.1, pWorld));
    HorizontalPolySegment3 = HVE2DPolySegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(0.1 + MYEPSILON, 0.1, pWorld));
    HorizontalPolySegment5 = HVE2DPolySegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(10.1 + MYEPSILON, 0.1, pWorld));

    HorizontalPoint0d0 = HGF2DLocation(0.1, 0.1, pWorld);
    HorizontalPoint0d1 = HGF2DLocation(1.1, 0.1, pWorld);
    HorizontalPoint0d5 = HGF2DLocation(5.1, 0.1, pWorld);
    HorizontalPoint1d0 = HGF2DLocation(10.1, 0.1, pWorld);

    Horizontal3Point0d0 = HGF2DLocation(0.1, 0.1, pWorld);
    Horizontal3Point0d1 = HGF2DLocation(0.1 + (0.1 * MYEPSILON), 0.1, pWorld);
    Horizontal3Point0d5 = HGF2DLocation(0.1 + (0.5 * MYEPSILON), 0.1, pWorld);
    Horizontal3Point1d0 = HGF2DLocation(0.1 + MYEPSILON, 0.1, pWorld);

    HorizontalMidPoint1 = HGF2DLocation(5.1, 0.1, pWorld);
    HorizontalMidPoint3 = HGF2DLocation(0.1 + (MYEPSILON/2), 0.1, pWorld);

    HorizontalClosePoint1A = HGF2DLocation(0.0, 0.0, pWorld);
    HorizontalClosePoint1B = HGF2DLocation(5.0, 0.0, pWorld);
    HorizontalClosePoint1C = HGF2DLocation(7.0, 0.2, pWorld);
    HorizontalClosePoint1D = HGF2DLocation(10.0, 1000000.0, pWorld);
    HorizontalCloseMidPoint1 = HGF2DLocation(5.1, 0.0, pWorld);

    HorizontalClosePoint3A = HGF2DLocation(0.0, 0.0, pWorld);
    HorizontalClosePoint3B = HGF2DLocation(0.1 + (2*MYEPSILON/3), 0.0, pWorld);
    HorizontalClosePoint3C = HGF2DLocation(0.1 + (MYEPSILON/3), 0.2, pWorld);
    HorizontalClosePoint3D = HGF2DLocation(0.1 + (MYEPSILON/2), 1000000.0, pWorld);
    HorizontalCloseMidPoint3 = HGF2DLocation(0.1 + (MYEPSILON/2), 0.0, pWorld);

    // MISC PolySegmentS
    MiscPolySegment1 = HVE2DPolySegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(10.1, 10.1, pWorld));
    MiscPolySegment2 = HVE2DPolySegment(HGF2DLocation(10.1, 10.1, pWorld), HGF2DLocation(0.1, 0.1, pWorld));
    MiscPolySegment3 = HVE2DPolySegment(HGF2DLocation(0.1, 0.1, pWorld), 
                                        HGF2DLocation(0.1, 0.1, pWorld) + HGF2DDisplacement(HGFBearing(77.5* PI / 180), MYEPSILON));
    MiscPolySegment3A = HVE2DPolySegment(HGF2DLocation(0.1+MYEPSILON, 0.1, pWorld), 
                                         HGF2DLocation(0.1+MYEPSILON, 0.1, pWorld) + HGF2DDisplacement(HGFBearing(77.5* PI / 180), MYEPSILON));
    MiscPolySegment4 = HVE2DPolySegment(HGF2DLocation(0.2, 0.1, pWorld),
                                        HGF2DLocation(0.2, 0.1, pWorld) + HGF2DDisplacement(HGFBearing(77.5* PI / 180), MYEPSILON));
    MiscPolySegment6 = HVE2DPolySegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(-9.9, 10.1, pWorld));
    MiscPolySegment7 = HVE2DPolySegment(HGF2DLocation(0.2, 0.0, pWorld), HGF2DLocation(-9.8, 10.0, pWorld));
    DisjointPolySegment1 = HVE2DPolySegment(HGF2DLocation(-0.1, -0.1, pWorld), HGF2DLocation(-10.1, -10.24, pWorld));
    ContiguousExtentPolySegment1 = HVE2DPolySegment(HGF2DLocation(10.1, 0.1, pWorld), HGF2DLocation(20.1, 10.1, pWorld));
    FlirtingExtentPolySegment1 = HVE2DPolySegment(HGF2DLocation(10.1, 0.1, pWorld), HGF2DLocation(20.1, -10.1, pWorld));
    FlirtingExtentLinkedPolySegment1 = HVE2DPolySegment(HGF2DLocation(10.1, 10.1, pWorld), HGF2DLocation(20.1, 0.1, pWorld));
    ParallelPolySegment1 = HVE2DPolySegment(HGF2DLocation(0.2, 0.1, pWorld), HGF2DLocation(10.2, 10.1, pWorld));
    LinkedParallelPolySegment1 = HVE2DPolySegment(HGF2DLocation(10.1, 10.1, pWorld), HGF2DLocation(20.1, 20.1, pWorld));
    NearParallelPolySegment1 = HVE2DPolySegment(HGF2DLocation(0.2, 0.1, pWorld), HGF2DLocation(10.2, 10.1+MYEPSILON, pWorld));
    CloseNearParallelPolySegment1 = HVE2DPolySegment(HGF2DLocation(0.1+MYEPSILON, 0.1, pWorld), HGF2DLocation(10.1+MYEPSILON, 10.1+MYEPSILON, pWorld));
    ConnectedPolySegment1 = HVE2DPolySegment(HGF2DLocation(0.2, 0.0, pWorld), HGF2DLocation(0.0, 0.2, pWorld));
    ConnectingPolySegment1 = HVE2DPolySegment(HGF2DLocation(10.2, 0.0, pWorld), HGF2DLocation(2.0, 2.0, pWorld));
    ConnectedPolySegment1A = HVE2DPolySegment(HGF2DLocation(20.2, 0.0, pWorld), HGF2DLocation(0.0, 20.2, pWorld));
    ConnectingPolySegment1A = HVE2DPolySegment(HGF2DLocation(2.0, 2.0, pWorld), HGF2DLocation(10.2, 0.0, pWorld));
    LinkedPolySegment1 = HVE2DPolySegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(10.0, 3.2, pWorld));
    LinkedPolySegment1A = HVE2DPolySegment(HGF2DLocation(10.1, 10.1, pWorld), HGF2DLocation(10.0, 3.2, pWorld));

    Misc3Point0d0 = HGF2DLocation(0.1, 0.1, pWorld);
    Misc3Point0d1 = HGF2DLocation(0.1, 0.1, pWorld) + HGF2DDisplacement(HGFBearing(77.5* PI / 180), MYEPSILON * 0.1);
    Misc3Point0d5 = HGF2DLocation(0.1, 0.1, pWorld) + HGF2DDisplacement(HGFBearing(77.5* PI / 180), MYEPSILON * 0.5);
    Misc3Point1d0 = HGF2DLocation(0.1, 0.1, pWorld) + HGF2DDisplacement(HGFBearing(77.5* PI / 180), MYEPSILON);
    MiscMidPoint1 = HGF2DLocation(5.1, 5.1, pWorld);
    MiscMidPoint3 = HGF2DLocation(0.1, 0.1, pWorld)+ HGF2DDisplacement(HGFBearing(77.5* PI / 180), MYEPSILON / 2);

    MiscClosePoint3A = HGF2DLocation(0.0, 0.0, pWorld);
    MiscClosePoint3B = HGF2DLocation(0.0, 0.1 + (2*MYEPSILON/3), pWorld);
    MiscClosePoint3C = HGF2DLocation(0.2, 0.1 + (MYEPSILON/3), pWorld);
    MiscClosePoint3D = HGF2DLocation(1000000.0, 0.1 + (MYEPSILON/2), pWorld);
    MiscCloseMidPoint3 = HGF2DLocation(0.0, 0.1 + (MYEPSILON/2), pWorld);

    VeryFarPoint = HGF2DLocation(21E123, 1E123, pWorld);
    VeryFarAlignedPoint = HGF2DLocation(21E123, 21E123, pWorld);
    VeryFarAlignedNegativePoint = HGF2DLocation(-21E123, -21E123, pWorld);
    VeryFarNegativePoint = HGF2DLocation(-1E123, -21E123, pWorld);
    MidPoint = HGF2DLocation(5.1, 5.1, pWorld);

    MiscMidPoint6 = HGF2DLocation(-4.9, 5.1, pWorld);

    // LARGE PolySegmentS
    LargePolySegment1 = HVE2DPolySegment(HGF2DLocation(-1E123, -21E123, pWorld), HGF2DLocation(9E123, 19E123, pWorld));
    LargePolySegment2 = HVE2DPolySegment(HGF2DLocation(9E123, 19E123, pWorld), HGF2DLocation(-1E123, -21E123, pWorld));
    ParallelLargePolySegment1 = HVE2DPolySegment(HGF2DLocation(-1.000000001E123, -21E123, pWorld), HGF2DLocation(9.000000001E123, 19E123, pWorld));

    LargePoint0d0 = HGF2DLocation(-1E123, -21E123, pWorld);
    LargePoint0d1 = HGF2DLocation(0.0, -17E123, pWorld);
    LargePoint0d5 = HGF2DLocation(4E123, -1E123, pWorld);
    LargePoint1d0 = HGF2DLocation(9E123, 19E123, pWorld);

    LargeMidPoint1 = HGF2DLocation(4E123, -1E123, pWorld);

    LargeClosePoint1A = HGF2DLocation(0.0, 0.0, pWorld);
    LargeClosePoint1B = HGF2DLocation(1E124, 1E124, pWorld);
    LargeClosePoint1C = HGF2DLocation(-1E124, -1E124, pWorld);
    LargeClosePoint1D = HGF2DLocation(10.0, 1E120, pWorld);
    LargeCloseMidPoint1 = HGF2DLocation(4.0001E123, -1.0002E123, pWorld);

    // POSITIVE PolySegmentS
    PositivePolySegment1 = HVE2DPolySegment(HGF2DLocation(1E123, 21E123, pWorld), HGF2DLocation(11E123, 41E123, pWorld));
    PositivePolySegment2 = HVE2DPolySegment(HGF2DLocation(11E123, 41E123, pWorld), HGF2DLocation(1E123, 21E123, pWorld));
    ParallelPositivePolySegment1 = HVE2DPolySegment(HGF2DLocation(1.000001E123, 21E123, pWorld), HGF2DLocation(11.000001E123, 41E123, pWorld));

    PositivePoint0d0 = HGF2DLocation(1E123, 21E123, pWorld);
    PositivePoint0d1 = HGF2DLocation(2E123, 23E123, pWorld);
    PositivePoint0d5 = HGF2DLocation(6E123, 31E123, pWorld);
    PositivePoint1d0 = HGF2DLocation(11E123, 41E123, pWorld);

    PositiveMidPoint1 = HGF2DLocation(6E123, 31E123, pWorld);

    PositiveClosePoint1A = HGF2DLocation(0.0, 0.0, pWorld);
    PositiveClosePoint1B = HGF2DLocation(1E124, 1E124, pWorld);
    PositiveClosePoint1C = HGF2DLocation(-1E124, -1E124, pWorld);
    PositiveClosePoint1D = HGF2DLocation(10.0, 1E120, pWorld);
    PositiveCloseMidPoint1 = HGF2DLocation(6.0001E123, 31.0002E123, pWorld);

    // NEGATIVE PolySegmentS
    NegativePolySegment1 = HVE2DPolySegment(HGF2DLocation(-1E123, -21E123, pWorld), HGF2DLocation(-11E123, -41E123, pWorld));
    NegativePolySegment2 = HVE2DPolySegment(HGF2DLocation(-11E123, -41E123, pWorld), HGF2DLocation(-1E123, -21E123, pWorld));
    ParallelNegativePolySegment1 = HVE2DPolySegment(HGF2DLocation(-1.000001E123, -21E123, pWorld), HGF2DLocation(-11.000001E123, -41E123, pWorld));

    NegativePoint0d0 = HGF2DLocation(-1E123, -21E123, pWorld);
    NegativePoint0d1 = HGF2DLocation(-2E123, -23E123, pWorld);
    NegativePoint0d5 = HGF2DLocation(-6E123, -31E123, pWorld);
    NegativePoint1d0 = HGF2DLocation(-11E123, -41E123, pWorld);

    NegativeMidPoint1 = HGF2DLocation(-6E123, -31E123, pWorld);

    NegativeClosePoint1A = HGF2DLocation(0.0, 0.0, pWorld);
    NegativeClosePoint1B = HGF2DLocation(1E124, 1E124, pWorld);
    NegativeClosePoint1C = HGF2DLocation(-1E124, -1E124, pWorld);
    NegativeClosePoint1D = HGF2DLocation(10.0, 1E120, pWorld);
    NegativeCloseMidPoint1 = HGF2DLocation(-6.0001E123, -31.0002E123, pWorld);

    // NULL PolySegmentS
    NullPolySegment1 = HVE2DPolySegment(pWorld);
    NullPolySegment2 = HVE2DPolySegment(pWorld);

    // PolySegments
    EmptyPolySegment = HVE2DPolySegment(pWorld);

    PolySegment1 = HVE2DPolySegment(pWorld); // Open
    PolySegment1.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    PolySegment1.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    PolySegment1.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));
    PolySegment1.AppendPoint(HGF2DLocation(30.0, 10.0, pWorld));
    PolySegment1.AppendPoint(HGF2DLocation(30.0, 5.0, pWorld));
    PolySegment1.AppendPoint(HGF2DLocation(30.0, 5.0-MYEPSILON, pWorld));
    PolySegment1.AppendPoint(HGF2DLocation(30.0-MYEPSILON, 0.0, pWorld));

    PolySegment1Point0d0 = HGF2DLocation(0.0, 0.0, pWorld);
    PolySegment1Point0d1 = HGF2DLocation(3.1213203435596, 3.1213203435596, pWorld);
    PolySegment1Point0d5 = HGF2DLocation(17.928932188135, 10.0, pWorld);
    PolySegment1Point1d0 = HGF2DLocation(30.0-MYEPSILON, 0.0, pWorld);

    PolySegmentMidPoint1 = HGF2DLocation(17.928932188135, 10.0, pWorld);

    PolySegmentClosePoint1A = HGF2DLocation(21.1, 10.1, pWorld);
    PolySegmentClosePoint1B = HGF2DLocation(-10.0, 10.0, pWorld);
    PolySegmentClosePoint1C = HGF2DLocation(30.0, 10.1, pWorld);
    PolySegmentClosePoint1D = HGF2DLocation(32.1, 5.1, pWorld);
    PolySegmentCloseMidPoint1 = HGF2DLocation(17.928932, 9.1, pWorld);

    VeryFarPoint = HGF2DLocation(10000.0, 10000.1, pWorld);

    PolySegment2 = HVE2DPolySegment(pWorld);  // AutoClosed
    PolySegment2.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    PolySegment2.AppendPoint(HGF2DLocation(-10.0, -10.0, pWorld));
    PolySegment2.AppendPoint(HGF2DLocation(18.0, 9.0, pWorld));
    PolySegment2.AppendPoint(HGF2DLocation(21.0, 0.0, pWorld));
    PolySegment2.AppendPoint(HGF2DLocation(24.0, 15.0, pWorld));
    PolySegment2.AppendPoint(HGF2DLocation(0.0, 15.0, pWorld));
    PolySegment2.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));

    PolySegment2Point0d0 = HGF2DLocation(0.0, 0.0, pWorld);
    PolySegment2Point0d1 = HGF2DLocation(-7.9028994453177, -7.9028994453177, pWorld);
    PolySegment2Point0d5 = HGF2DLocation(20.498817144560, 1.5035485663202, pWorld);
    PolySegment2Point1d0 = HGF2DLocation(0.0, 0.0, pWorld);

    PolySegmentMidPoint2 = HGF2DLocation(20.498817144560, 1.5035485663202, pWorld);

    PolySegmentClosePoint2A = HGF2DLocation(21.1, 10.1, pWorld);
    PolySegmentClosePoint2B = HGF2DLocation(21.1, 10.1, pWorld);
    PolySegmentClosePoint2C = HGF2DLocation(21.1, 10.1, pWorld);
    PolySegmentClosePoint2D = HGF2DLocation(21.1, 10.1, pWorld);
    PolySegmentCloseMidPoint2 = HGF2DLocation(21.1, 10.1, pWorld);

    // epsilon size container
    PolySegment3 = HVE2DPolySegment(pWorld); // Open
    PolySegment3.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    PolySegment3.AppendPoint(HGF2DLocation(0.0-MYEPSILON, 0.0, pWorld));
    PolySegment3.AppendPoint(HGF2DLocation(0.0-MYEPSILON, 0.0+MYEPSILON, pWorld));
    PolySegment3.AppendPoint(HGF2DLocation(0.0, 0.0+MYEPSILON, pWorld));
    PolySegment3.AppendPoint(HGF2DLocation(0.0+MYEPSILON, 0.0+2*MYEPSILON, pWorld));
    PolySegment3.AppendPoint(HGF2DLocation(0.0+MYEPSILON, 0.0, pWorld));

    PolySegment3Point0d0 = HGF2DLocation(0.0, 0.0, pWorld);
    PolySegment3Point0d1 = HGF2DLocation(-0.64142135623731*MYEPSILON, 0.0, pWorld);
    PolySegment3Point0d5 = HGF2DLocation(0.14644660940673*MYEPSILON, 1.1464466094067*MYEPSILON, pWorld);
    PolySegment3Point1d0 = HGF2DLocation(0.0+MYEPSILON, 0.0, pWorld);

    PolySegmentMidPoint3 = HGF2DLocation(0.14644660940673*MYEPSILON, 1.1464466094067*MYEPSILON, pWorld);

    PolySegmentClosePoint3A = HGF2DLocation(21.1, 10.1, pWorld);
    PolySegmentClosePoint3B = HGF2DLocation(21.1, 10.1, pWorld);
    PolySegmentClosePoint3C = HGF2DLocation(21.1, 10.1, pWorld);
    PolySegmentClosePoint3D = HGF2DLocation(21.1, 10.1, pWorld);
    PolySegmentCloseMidPoint3 = HGF2DLocation(21.1, 10.1, pWorld);

     // Used for intersection tests
    ComplexPolySegmentCase1 = HVE2DPolySegment(pWorld);
    ComplexPolySegmentCase1.AppendPoint(HGF2DLocation(20.0, 5.0, pWorld));
    ComplexPolySegmentCase1.AppendPoint(HGF2DLocation(30.0, 15.0, pWorld));

    ComplexPolySegmentCase2 = HVE2DPolySegment(pWorld);
    ComplexPolySegmentCase2.AppendPoint(HGF2DLocation(20.0, 5.0, pWorld));
    ComplexPolySegmentCase2.AppendPoint(HGF2DLocation(24.0, 10.0, pWorld));
    ComplexPolySegmentCase2.AppendPoint(HGF2DLocation(28.0, 10.0, pWorld));
    ComplexPolySegmentCase2.AppendPoint(HGF2DLocation(30.0, 15.0, pWorld));

    ComplexPolySegmentCase3 = HVE2DPolySegment(pWorld);
    ComplexPolySegmentCase3.AppendPoint(HGF2DLocation(15.0, 5.0, pWorld));
    ComplexPolySegmentCase3.AppendPoint(HGF2DLocation(25.0, 15.0, pWorld));

    ComplexPolySegmentCase4 = HVE2DPolySegment(pWorld);
    ComplexPolySegmentCase4.AppendPoint(HGF2DLocation(5.0, 10.0, pWorld));
    ComplexPolySegmentCase4.AppendPoint(HGF2DLocation(35.0, 10.0, pWorld));

    ComplexPolySegmentCase5 = HVE2DPolySegment(pWorld);
    ComplexPolySegmentCase5.AppendPoint(HGF2DLocation(35.0, 10.0, pWorld));
    ComplexPolySegmentCase5.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));
    ComplexPolySegmentCase5.AppendPoint(HGF2DLocation(10.0, 5.0, pWorld));

    ComplexPolySegmentCase5A = HVE2DPolySegment(pWorld);
    ComplexPolySegmentCase5A.AppendPoint(HGF2DLocation(30.0, 10.0, pWorld));
    ComplexPolySegmentCase5A.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));
    ComplexPolySegmentCase5A.AppendPoint(HGF2DLocation(10.0, 5.0, pWorld));

    ComplexPolySegmentCase6 = HVE2DPolySegment(pWorld);
    ComplexPolySegmentCase6.AppendPoint(HGF2DLocation(35.0, 10.0, pWorld));
    ComplexPolySegmentCase6.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    ComplexPolySegmentCase6.AppendPoint(HGF2DLocation(-1.0, -1.0, pWorld));

    ComplexPolySegmentCase7 = HVE2DPolySegment(pWorld);
    ComplexPolySegmentCase7.AppendPoint(HGF2DLocation(25.0, 15.0, pWorld));
    ComplexPolySegmentCase7.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));

    AutoCrossingPolySegment1Segment1 = HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment1Segment2 = HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingPolySegment1Segment3 = HVE2DSegment(HGF2DLocation(15.0, 5.0, pWorld), HGF2DLocation(0.0, 5.0, pWorld));
    AutoCrossingPolySegment1Segment4 = HVE2DSegment(HGF2DLocation(0.0, 5.0, pWorld), HGF2DLocation(5.0, 0.0, pWorld));
    AutoCrossingPolySegment1Segment5 = HVE2DSegment(HGF2DLocation(5.0, 0.0, pWorld), HGF2DLocation(20.0, 20.0, pWorld));
    AutoCrossingPolySegment1 = HVE2DPolySegment(pWorld);
    AutoCrossingPolySegment1.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AutoCrossingPolySegment1.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment1.AppendPoint(HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingPolySegment1.AppendPoint(HGF2DLocation(0.0, 5.0, pWorld));
    AutoCrossingPolySegment1.AppendPoint(HGF2DLocation(5.0, 0.0, pWorld));
    AutoCrossingPolySegment1.AppendPoint(HGF2DLocation(20.0, 20.0, pWorld));

    AutoCrossingPolySegment2Segment1 = HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment2Segment2 = HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingPolySegment2Segment3 = HVE2DSegment(HGF2DLocation(15.0, 5.0, pWorld), HGF2DLocation(10.0, 5.0, pWorld));
    AutoCrossingPolySegment2Segment4 = HVE2DSegment(HGF2DLocation(10.0, 5.0, pWorld), HGF2DLocation(10.0, 15.0, pWorld));
    AutoCrossingPolySegment2 = HVE2DPolySegment(pWorld);
    AutoCrossingPolySegment2.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AutoCrossingPolySegment2.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment2.AppendPoint(HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingPolySegment2.AppendPoint(HGF2DLocation(10.0, 5.0, pWorld));
    AutoCrossingPolySegment2.AppendPoint(HGF2DLocation(10.0, 15.0, pWorld));

    AutoCrossingPolySegment3 = HVE2DPolySegment(pWorld);
    AutoCrossingPolySegment3.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AutoCrossingPolySegment3.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment3.AppendPoint(HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingPolySegment3.AppendPoint(HGF2DLocation(0.0, 5.0, pWorld));
    AutoCrossingPolySegment3.AppendPoint(HGF2DLocation(5.0, 0.0, pWorld));
    AutoCrossingPolySegment3.AppendPoint(HGF2DLocation(20.0, 20.0, pWorld));
    AutoCrossingPolySegment3.AppendPoint(HGF2DLocation(-20.0, 20.0, pWorld));
    AutoCrossingPolySegment3.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));

    AutoCrossingPolySegment4 = HVE2DPolySegment(pWorld);
    AutoCrossingPolySegment4.AppendPoint(HGF2DLocation(7.5, 7.5, pWorld));
    AutoCrossingPolySegment4.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment4.AppendPoint(HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingPolySegment4.AppendPoint(HGF2DLocation(0.0, 5.0, pWorld));
    AutoCrossingPolySegment4.AppendPoint(HGF2DLocation(5.0, 0.0, pWorld));
    AutoCrossingPolySegment4.AppendPoint(HGF2DLocation(20.0, 20.0, pWorld));
    AutoCrossingPolySegment4.AppendPoint(HGF2DLocation(-20.0, 20.0, pWorld));
    AutoCrossingPolySegment4.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AutoCrossingPolySegment4.AppendPoint(HGF2DLocation(7.5, 7.5, pWorld));

    AutoCrossingPolySegment5 = HVE2DPolySegment(pWorld);
    AutoCrossingPolySegment5.AppendPoint(HGF2DLocation(7.5, 7.5, pWorld));
    AutoCrossingPolySegment5.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AutoCrossingPolySegment5.AppendPoint(HGF2DLocation(-20.0, 20.0, pWorld));
    AutoCrossingPolySegment5.AppendPoint(HGF2DLocation(20.0, 20.0, pWorld));
    AutoCrossingPolySegment5.AppendPoint(HGF2DLocation(5.0, 0.0, pWorld));
    AutoCrossingPolySegment5.AppendPoint(HGF2DLocation(0.0, 5.0, pWorld));
    AutoCrossingPolySegment5.AppendPoint(HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingPolySegment5.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment5.AppendPoint(HGF2DLocation(7.5, 7.5, pWorld));

    AutoCrossingPolySegment6 = HVE2DPolySegment(pWorld);
    AutoCrossingPolySegment6.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AutoCrossingPolySegment6.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment6.AppendPoint(HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingPolySegment6.AppendPoint(HGF2DLocation(10.0, 5.0, pWorld));
    AutoCrossingPolySegment6.AppendPoint(HGF2DLocation(10, 10.0, pWorld));
    AutoCrossingPolySegment6.AppendPoint(HGF2DLocation(5.0, 15.0, pWorld));

    AutoCrossingPolySegment7 = HVE2DPolySegment(pWorld);
    AutoCrossingPolySegment7.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AutoCrossingPolySegment7.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment7.AppendPoint(HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingPolySegment7.AppendPoint(HGF2DLocation(15.0, 0.0, pWorld));
    AutoCrossingPolySegment7.AppendPoint(HGF2DLocation(14.0, -5.0, pWorld));
    AutoCrossingPolySegment7.AppendPoint(HGF2DLocation(13.0, -5.0, pWorld));
    AutoCrossingPolySegment7.AppendPoint(HGF2DLocation(12.0, 0.0, pWorld));
    AutoCrossingPolySegment7.AppendPoint(HGF2DLocation(11.0, 2.0, pWorld));
    AutoCrossingPolySegment7.AppendPoint(HGF2DLocation(10.0, 5.0, pWorld));
    AutoCrossingPolySegment7.AppendPoint(HGF2DLocation(10, 10.0, pWorld));
    AutoCrossingPolySegment7.AppendPoint(HGF2DLocation(5.0, 15.0, pWorld));

    AutoCrossingPolySegment8 = HVE2DPolySegment(pWorld);
    AutoCrossingPolySegment8.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AutoCrossingPolySegment8.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment8.AppendPoint(HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingPolySegment8.AppendPoint(HGF2DLocation(5.0, 5.0, pWorld));
    AutoCrossingPolySegment8.AppendPoint(HGF2DLocation(20.0, 20.0, pWorld));

    AutoCrossingPolySegment9 = HVE2DPolySegment(pWorld);
    AutoCrossingPolySegment9.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AutoCrossingPolySegment9.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment9.AppendPoint(HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingPolySegment9.AppendPoint(HGF2DLocation(5.0, 5.0, pWorld));
    AutoCrossingPolySegment9.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment9.AppendPoint(HGF2DLocation(20.0, 20.0, pWorld));

    AutoCrossingPolySegment10 = HVE2DPolySegment(pWorld);
    AutoCrossingPolySegment10.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AutoCrossingPolySegment10.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment10.AppendPoint(HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingPolySegment10.AppendPoint(HGF2DLocation(5.0, 5.0, pWorld));
    AutoCrossingPolySegment10.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment10.AppendPoint(HGF2DLocation(10.0, 15.0, pWorld));

    AutoCrossingPolySegment11 = HVE2DPolySegment(pWorld);
    AutoCrossingPolySegment11.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AutoCrossingPolySegment11.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment11.AppendPoint(HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingPolySegment11.AppendPoint(HGF2DLocation(0.0, 5.0, pWorld));
    AutoCrossingPolySegment11.AppendPoint(HGF2DLocation(5.0, 0.0, pWorld));
    AutoCrossingPolySegment11.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment11.AppendPoint(HGF2DLocation(20.0, 20.0, pWorld));

    AutoCrossingPolySegment12 = HVE2DPolySegment(pWorld);
    AutoCrossingPolySegment12.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AutoCrossingPolySegment12.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment12.AppendPoint(HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingPolySegment12.AppendPoint(HGF2DLocation(0.0, 5.0, pWorld));
    AutoCrossingPolySegment12.AppendPoint(HGF2DLocation(5.0, 0.0, pWorld));
    AutoCrossingPolySegment12.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment12.AppendPoint(HGF2DLocation(20.0, 20.0, pWorld));
    AutoCrossingPolySegment12.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));
    AutoCrossingPolySegment12.AppendPoint(HGF2DLocation(0.0, 10.0, pWorld));

    AutoCrossingPolySegment13 = HVE2DPolySegment(pWorld);
    AutoCrossingPolySegment13.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AutoCrossingPolySegment13.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment13.AppendPoint(HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingPolySegment13.AppendPoint(HGF2DLocation(0.0, 5.0, pWorld));
    AutoCrossingPolySegment13.AppendPoint(HGF2DLocation(5.0, 0.0, pWorld));
    AutoCrossingPolySegment13.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment13.AppendPoint(HGF2DLocation(20.0, 20.0, pWorld));
    AutoCrossingPolySegment13.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));
    AutoCrossingPolySegment13.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingPolySegment13.AppendPoint(HGF2DLocation(0.0, 10.0, pWorld));

    AutoConnectingPolySegment1Segment1 = HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    AutoConnectingPolySegment1Segment2 = HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(15.0, 5.0, pWorld));
    AutoConnectingPolySegment1Segment3 = HVE2DSegment(HGF2DLocation(15.0, 5.0, pWorld), HGF2DLocation(10.0, 0.0, pWorld));
    AutoConnectingPolySegment1Segment4 = HVE2DSegment(HGF2DLocation(10.0, 0.0, pWorld), HGF2DLocation(-10.0, 0.0, pWorld));
    AutoConnectingPolySegment1 = HVE2DPolySegment(pWorld);
    AutoConnectingPolySegment1.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AutoConnectingPolySegment1.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AutoConnectingPolySegment1.AppendPoint(HGF2DLocation(15.0, 5.0, pWorld));
    AutoConnectingPolySegment1.AppendPoint(HGF2DLocation(10.0, 0.0, pWorld));
    AutoConnectingPolySegment1.AppendPoint(HGF2DLocation(-10.0, 0.0, pWorld));

    DisjointPolySegment1Segment11 = HVE2DSegment(HGF2DLocation(-10.0, -10.0, pWorld), HGF2DLocation(-20.0, -10.0, pWorld));
    DisjointPolySegment1Segment21 = HVE2DSegment(HGF2DLocation(-20.0, -10.0, pWorld), HGF2DLocation(-30.0, -10.0, pWorld));
    DisjointPolySegment11 = HVE2DPolySegment(pWorld); // Open
    DisjointPolySegment11.AppendPoint(HGF2DLocation(-10.0, -10.0, pWorld));
    DisjointPolySegment11.AppendPoint(HGF2DLocation(-20.0, -10.0, pWorld));
    DisjointPolySegment11.AppendPoint(HGF2DLocation(-30.0, -10.0, pWorld));

    ContiguousExtentPolySegment1Segment11 = HVE2DSegment(HGF2DLocation(-10.0, 0.0, pWorld), HGF2DLocation(0.0, 10.0, pWorld));
    ContiguousExtentPolySegment1Segment21 = HVE2DSegment(HGF2DLocation(0.0, 10.0, pWorld), HGF2DLocation(-10.0, 10.0, pWorld));
    ContiguousExtentPolySegment11 = HVE2DPolySegment(pWorld); // Open
    ContiguousExtentPolySegment11.AppendPoint(HGF2DLocation(-10.0, 0.0, pWorld));
    ContiguousExtentPolySegment11.AppendPoint(HGF2DLocation(0.0, 10.0, pWorld));
    ContiguousExtentPolySegment11.AppendPoint(HGF2DLocation(-10.0, 10.0, pWorld));

    FlirtingExtentPolySegment1Segment11 = HVE2DSegment(HGF2DLocation(-10.0, 0.0, pWorld), HGF2DLocation(0.0, -10.0, pWorld));
    FlirtingExtentPolySegment1Segment21 = HVE2DSegment(HGF2DLocation(0.0, -10.0, pWorld), HGF2DLocation(-10.0, -10.0, pWorld));
    FlirtingExtentPolySegment11 = HVE2DPolySegment(pWorld); // Open
    FlirtingExtentPolySegment11.AppendPoint(HGF2DLocation(-10.0, 0.0, pWorld));
    FlirtingExtentPolySegment11.AppendPoint(HGF2DLocation(0.0, -10.0, pWorld));
    FlirtingExtentPolySegment11.AppendPoint(HGF2DLocation(-10.0, -10.0, pWorld));

    FlirtingExtentLinkedPolySegment1Segment11 = HVE2DSegment(HGF2DLocation(-10.0, 0.0, pWorld), HGF2DLocation(0.0, -10.0, pWorld));
    FlirtingExtentLinkedPolySegment1Segment21 = HVE2DSegment(HGF2DLocation(0.0, -10.0, pWorld), HGF2DLocation(0.0, 0.0, pWorld));
    FlirtingExtentLinkedPolySegment11 = HVE2DPolySegment(pWorld); // Open
    FlirtingExtentLinkedPolySegment11.AppendPoint(HGF2DLocation(-10.0, 0.0, pWorld));
    FlirtingExtentLinkedPolySegment11.AppendPoint(HGF2DLocation(0.0, -10.0, pWorld));
    FlirtingExtentLinkedPolySegment11.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));

    ConnectedPolySegment1Segment11 = HVE2DSegment(HGF2DLocation(-10.0, 10.0, pWorld), HGF2DLocation(10.0, -10.0, pWorld));
    ConnectedPolySegment1Segment21 = HVE2DSegment(HGF2DLocation(10.0, -10.0, pWorld), HGF2DLocation(0.0, -10.0, pWorld));
    ConnectedPolySegment11 = HVE2DPolySegment(pWorld); // Open
    ConnectedPolySegment11.AppendPoint(HGF2DLocation(-10.0, 10.0, pWorld));
    ConnectedPolySegment11.AppendPoint(HGF2DLocation(10.0, -10.0, pWorld));
    ConnectedPolySegment11.AppendPoint(HGF2DLocation(0.0, -10.0, pWorld));

    ConnectingPolySegment1Segment11 = HVE2DSegment(HGF2DLocation(5.0, 5.0, pWorld), HGF2DLocation(-5.0, 10.0, pWorld));
    ConnectingPolySegment1Segment21 = HVE2DSegment(HGF2DLocation(-5.0, 10.0, pWorld), HGF2DLocation(-15.0, 0.0, pWorld));
    ConnectingPolySegment11 = HVE2DPolySegment(pWorld);
    ConnectingPolySegment11.AppendPoint(HGF2DLocation(5.0, 5.0, pWorld));
    ConnectingPolySegment11.AppendPoint(HGF2DLocation(-5.0, 10.0, pWorld));
    ConnectingPolySegment11.AppendPoint(HGF2DLocation(-15.0, 0.0, pWorld));

    ConnectedPolySegment1ASegment11 = HVE2DSegment(HGF2DLocation(40.0, 0.0, pWorld), HGF2DLocation(10.0, 0.0, pWorld));
    ConnectedPolySegment1ASegment21 = HVE2DSegment(HGF2DLocation(10.0, 0.0, pWorld), HGF2DLocation(11.0, -10.0, pWorld));
    ConnectedPolySegment1A1 = HVE2DPolySegment(pWorld); // Open
    ConnectedPolySegment1A1.AppendPoint(HGF2DLocation(40.0, 0.0, pWorld));
    ConnectedPolySegment1A1.AppendPoint(HGF2DLocation(10.0, 0.0, pWorld));
    ConnectedPolySegment1A1.AppendPoint(HGF2DLocation(11.0, -10.0, pWorld));

    ConnectingPolySegment1ASegment11 = HVE2DSegment(HGF2DLocation(21.0, 20.0, pWorld), HGF2DLocation(20.0, 15.0, pWorld));
    ConnectingPolySegment1ASegment21 = HVE2DSegment(HGF2DLocation(20.0, 15.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld));
    ConnectingPolySegment1A1 = HVE2DPolySegment(pWorld);
    ConnectingPolySegment1A1.AppendPoint(HGF2DLocation(21.0, 20.0, pWorld));
    ConnectingPolySegment1A1.AppendPoint(HGF2DLocation(20.0, 15.0, pWorld));
    ConnectingPolySegment1A1.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));

    LinkedPolySegment1Segment11 = HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(20.0, 4.0, pWorld));
    LinkedPolySegment1Segment21 = HVE2DSegment(HGF2DLocation(20.0, 4.0, pWorld), HGF2DLocation(21, 0.0, pWorld));
    LinkedPolySegment11 = HVE2DPolySegment(pWorld);
    LinkedPolySegment11.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    LinkedPolySegment11.AppendPoint(HGF2DLocation(20.0, 4.0, pWorld));
    LinkedPolySegment11.AppendPoint(HGF2DLocation(21, 0.0, pWorld));

    LinkedPolySegment1ASegment11 = HVE2DSegment(HGF2DLocation(11.0, 3.0, pWorld), HGF2DLocation(20.0, 4.0, pWorld));
    LinkedPolySegment1ASegment21 = HVE2DSegment(HGF2DLocation(20.0, 4.0, pWorld), HGF2DLocation(30.0-MYEPSILON, 0.0, pWorld));
    LinkedPolySegment1A1 = HVE2DPolySegment(pWorld);
    LinkedPolySegment1A1.AppendPoint(HGF2DLocation(11.0, 3.0, pWorld));
    LinkedPolySegment1A1.AppendPoint(HGF2DLocation(20.0, 4.0, pWorld));
    LinkedPolySegment1A1.AppendPoint(HGF2DLocation(30.0-MYEPSILON, 0.0, pWorld));
    
    }

//==================================================================================
// PolySegment Construction tests
// HVE2DPolySegment();
// HVE2DPolySegment(const HGF2DLocation&, const HGF2DLocation&);
// HVE2DPolySegment(const HGF2DLocation& pi_rStartPoint,
//             const HGF2DDisplacement& pi_rDisplacement);
// HVE2DPolySegment(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
// HVE2DPolySegment(const HVE2DPolySegment&    pi_rObject);
//==================================================================================
TEST_F (HVE2DPolySegmentTester, ConstructionTest)
    {

    // Default Constructor
    HVE2DPolySegment    PolySegment1;

    // Preparation of the two points
    HGF2DLocation   FirstPolySegmentPoint(10.0, 10.2, pWorld);
    HGF2DLocation   SecondPolySegmentPoint(-10000.0, 100.3, pWorld);

    HVE2DPolySegment    PolySegment2(FirstPolySegmentPoint, SecondPolySegmentPoint);
    ASSERT_EQ(pWorld, PolySegment2.GetCoordSys());
    ASSERT_NEAR(10.00000, PolySegment2.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.20000, PolySegment2.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(-10000.0, PolySegment2.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(100.3000, PolySegment2.GetEndPoint().GetY(), MYEPSILON);

    SecondPolySegmentPoint.ChangeCoordSys(pSys1);

    HVE2DPolySegment    PolySegment3(FirstPolySegmentPoint, SecondPolySegmentPoint);
    ASSERT_EQ(pWorld, PolySegment3.GetCoordSys());
    ASSERT_NEAR(10.000000000000000, PolySegment3.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.200000000000000, PolySegment3.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(-10000.00000000000, PolySegment3.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(100.29999999999971, PolySegment3.GetEndPoint().GetY(), MYEPSILON);

    HGF2DDisplacement   Displacement1(10.0, 10.0);
    HGF2DDisplacement   Displacement2(0.0, 10.0);
    HGF2DDisplacement   Displacement3(-10.0, -10.0);
    HGF2DDisplacement   Displacement4(0.0, 0.0);

    HVE2DPolySegment    PolySegment4(FirstPolySegmentPoint, FirstPolySegmentPoint + Displacement1);
    ASSERT_EQ(pWorld, PolySegment4.GetCoordSys());
    ASSERT_NEAR(10.0, PolySegment4.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.2, PolySegment4.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(20.0, PolySegment4.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(20.2, PolySegment4.GetEndPoint().GetY(), MYEPSILON);

    HVE2DPolySegment    PolySegment5(FirstPolySegmentPoint, FirstPolySegmentPoint + Displacement2);
    ASSERT_EQ(pWorld, PolySegment5.GetCoordSys());
    ASSERT_NEAR(10.0, PolySegment5.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.2, PolySegment5.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.0, PolySegment5.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(20.2, PolySegment5.GetEndPoint().GetY(), MYEPSILON);

    HVE2DPolySegment    PolySegment6(FirstPolySegmentPoint, FirstPolySegmentPoint + Displacement3);
    ASSERT_EQ(pWorld, PolySegment6.GetCoordSys());
    ASSERT_NEAR(10.0000000000000000, PolySegment6.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.2000000000000000, PolySegment6.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, PolySegment6.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.19999999999999929, PolySegment6.GetEndPoint().GetY(), MYEPSILON);

    HVE2DPolySegment    PolySegment7(FirstPolySegmentPoint, FirstPolySegmentPoint + Displacement4);
    ASSERT_EQ(pWorld, PolySegment7.GetCoordSys());
    ASSERT_NEAR(10.0, PolySegment7.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.2, PolySegment7.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.0, PolySegment7.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.2, PolySegment7.GetEndPoint().GetY(), MYEPSILON);

    // Constructor with only coordinate system
    HVE2DPolySegment    PolySegment8(pSys1);
    ASSERT_EQ(pSys1, PolySegment8.GetCoordSys());

    //Copy Constructor
    HVE2DPolySegment    PolySegment9(PolySegment7);
    ASSERT_EQ(pWorld, PolySegment9.GetCoordSys());
    ASSERT_NEAR(10.0, PolySegment9.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.2, PolySegment9.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.0, PolySegment9.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.2, PolySegment9.GetEndPoint().GetY(), MYEPSILON);

    }

//==================================================================================
// operator= test
// operator=(const HVE2DPolySegment& pi_rObj);
//==================================================================================
TEST_F (HVE2DPolySegmentTester, OperatorTest)
    {

    // Test with different coord sys
    HGF2DLocation   FirstPolySegmentPoint(10.0, 10.2, pWorld);
    HGF2DLocation   SecondPolySegmentPoint(-10000.0, 100.3, pWorld);
    HVE2DPolySegment    PolySegment1(FirstPolySegmentPoint, SecondPolySegmentPoint);

    FirstPolySegmentPoint.ChangeCoordSys(pSys1);

    HVE2DPolySegment    PolySegment2(FirstPolySegmentPoint, SecondPolySegmentPoint);
    ASSERT_EQ(pSys1, PolySegment2.GetCoordSys());

    PolySegment2 = PolySegment1;
    ASSERT_EQ(pWorld, PolySegment2.GetCoordSys());
    ASSERT_DOUBLE_EQ(10.0, PolySegment2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.2, PolySegment2.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10000.0, PolySegment2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(100.3, PolySegment2.GetEndPoint().GetY());

    // Test with a NULL PolySegment
    HVE2DPolySegment    PolySegment3(pSys1);
    HVE2DPolySegment    PolySegment4;

    PolySegment4 = PolySegment3;
    ASSERT_EQ(pSys1, PolySegment4.GetCoordSys());

    }

//==================================================================================
// Type extraction test
// GetType() const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, GetTypeTest)
    {

    // Basic test
    ASSERT_EQ(HVE2DPolySegment::CLASS_ID, MiscPolySegment1.GetBasicLinearType());
    ASSERT_EQ(HVE2DPolySegment::CLASS_ID, VerticalPolySegment1.GetBasicLinearType());

    }

//==================================================================================
// Length calculation test
// CalculateLength() const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CalculateLengthTest)
    {

    // Test with vertical PolySegment
    ASSERT_DOUBLE_EQ(10.0, VerticalPolySegment1.CalculateLength());

    // Test with inverted vertical PolySegment
    ASSERT_DOUBLE_EQ(10.0, VerticalPolySegment2.CalculateLength());

    // Test with horizontal PolySegment
    ASSERT_DOUBLE_EQ(10.0, HorizontalPolySegment1.CalculateLength());

    // Test with inverted horizontal PolySegment
    ASSERT_DOUBLE_EQ(10.0, HorizontalPolySegment2.CalculateLength());

    // Tests with miscalenious PolySegment
    ASSERT_DOUBLE_EQ(14.142135623730951, MiscPolySegment1.CalculateLength());

    // Test with very large PolySegment
    ASSERT_DOUBLE_EQ(4.12310562561766099E124, LargePolySegment1.CalculateLength());

    // Test with PolySegments way into positive regions
    ASSERT_DOUBLE_EQ(2.2360679774997898e+124, PositivePolySegment1.CalculateLength());

    // Test with PolySegments way into negative regions
    ASSERT_DOUBLE_EQ(2.23606797749978976e+124, NegativePolySegment1.CalculateLength());

    // Test with a NULL PolySegment
    ASSERT_NEAR(0.0, NullPolySegment1.CalculateLength(), MYEPSILON);
    
    }

//==================================================================================
// Relative point calculation test
// CalculateRelativePoint(double pi_RelativePos) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CalculateRelativePointTest)
    {

    // Test with vertical PolySegment
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment1.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment1.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment1.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(1.10, VerticalPolySegment1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(5.10, VerticalPolySegment1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(7.10, VerticalPolySegment1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment1.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(10.1, VerticalPolySegment1.CalculateRelativePoint(1.0).GetY());

    // Test with inverted vertical PolySegment
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment2.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(10.1, VerticalPolySegment2.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment2.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(9.10, VerticalPolySegment2.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment2.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(5.10, VerticalPolySegment2.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment2.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(3.10, VerticalPolySegment2.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment2.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment2.CalculateRelativePoint(1.0).GetY());

    // Test with horizontal PolySegment
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment1.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment1.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(1.10, HorizontalPolySegment1.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(5.10, HorizontalPolySegment1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(7.10, HorizontalPolySegment1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(10.1, HorizontalPolySegment1.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment1.CalculateRelativePoint(1.0).GetY());

    // Test with inverted horizontal PolySegment
    ASSERT_DOUBLE_EQ(10.1, HorizontalPolySegment2.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment2.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(9.10, HorizontalPolySegment2.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment2.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(5.10, HorizontalPolySegment2.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment2.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(3.10, HorizontalPolySegment2.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment2.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment2.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment2.CalculateRelativePoint(1.0).GetY());

    // Tests with vertical EPSILON sized PolySegment
    ASSERT_DOUBLE_EQ(0.10000000, VerticalPolySegment3.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, VerticalPolySegment3.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(0.10000000, VerticalPolySegment3.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(0.10000001, VerticalPolySegment3.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(0.10000000, VerticalPolySegment3.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(0.10000005, VerticalPolySegment3.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(0.10000000, VerticalPolySegment3.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(0.10000007, VerticalPolySegment3.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.10000000, VerticalPolySegment3.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000010, VerticalPolySegment3.CalculateRelativePoint(1.0).GetY());

    // Tests with horizontal EPSILON sized PolySegment
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalPolySegment3.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalPolySegment3.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(0.10000001, HorizontalPolySegment3.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalPolySegment3.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(0.10000005, HorizontalPolySegment3.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalPolySegment3.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(0.10000007, HorizontalPolySegment3.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalPolySegment3.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.10000010, HorizontalPolySegment3.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000000, HorizontalPolySegment3.CalculateRelativePoint(1.0).GetY());

    // Tests with miscalenious EPSILON size PolySegment
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscPolySegment3.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscPolySegment3.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(0.10000000216439614, MiscPolySegment3.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(0.10000000976296007, MiscPolySegment3.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(0.10000001082198071, MiscPolySegment3.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(0.10000004881480036, MiscPolySegment3.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(0.10000001515077298, MiscPolySegment3.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(0.10000006834072050, MiscPolySegment3.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, MiscPolySegment3.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, MiscPolySegment3.CalculateRelativePoint(1.0).GetY());

    // Test with very large PolySegment
    ASSERT_DOUBLE_EQ(-1.00E123, LargePolySegment1.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, LargePolySegment1.CalculateRelativePoint(0.0).GetY());
    ASSERT_NEAR(0.0, LargePolySegment1.CalculateRelativePoint(0.1).GetX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(-17.0E123, LargePolySegment1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(4.000E123, LargePolySegment1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(-1.00E123, LargePolySegment1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(6.000E123, LargePolySegment1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(7.000E123, LargePolySegment1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(9.000E123, LargePolySegment1.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(19.00E123, LargePolySegment1.CalculateRelativePoint(1.0).GetY());

    // Test with PolySegments way into positive regions
    ASSERT_DOUBLE_EQ(1.00E123, PositivePolySegment1.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(21.0E123, PositivePolySegment1.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(2.00E123, PositivePolySegment1.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(23.0E123, PositivePolySegment1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(6.00E123, PositivePolySegment1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(31.0E123, PositivePolySegment1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(8.00E123, PositivePolySegment1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(35.0E123, PositivePolySegment1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(11.0E123, PositivePolySegment1.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(41.0E123, PositivePolySegment1.CalculateRelativePoint(1.0).GetY());

    // Test with PolySegments way into negative regions
    ASSERT_DOUBLE_EQ(-1.00E123, NegativePolySegment1.CalculateRelativePoint(0.0).GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, NegativePolySegment1.CalculateRelativePoint(0.0).GetY());
    ASSERT_DOUBLE_EQ(-2.00E123, NegativePolySegment1.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(-23.0E123, NegativePolySegment1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(-6.00E123, NegativePolySegment1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(-31.0E123, NegativePolySegment1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(-8.00E123, NegativePolySegment1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(-35.0E123, NegativePolySegment1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, NegativePolySegment1.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, NegativePolySegment1.CalculateRelativePoint(1.0).GetY());

    // Test with a NULL PolySegment
    ASSERT_NEAR(0.0, NullPolySegment1.CalculateRelativePoint(0.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, NullPolySegment1.CalculateRelativePoint(0.0).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, NullPolySegment1.CalculateRelativePoint(0.1).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, NullPolySegment1.CalculateRelativePoint(0.1).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, NullPolySegment1.CalculateRelativePoint(0.5).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, NullPolySegment1.CalculateRelativePoint(0.5).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, NullPolySegment1.CalculateRelativePoint(0.7).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, NullPolySegment1.CalculateRelativePoint(0.7).GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, NullPolySegment1.CalculateRelativePoint(1.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, NullPolySegment1.CalculateRelativePoint(1.0).GetY(), MYEPSILON);

    }

//==================================================================================
// Drop( HGF2DLocationCollection* po_pPoints, const HGFDistance& pi_rTolerance ) const
//==================================================================================
TEST_F (HVE2DPolySegmentTester, DropTest)
    {

    HGF2DLocationCollection Locations;

    // Test with vertical PolySegment
    VerticalPolySegment1.Drop(&Locations, MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10, Locations[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, Locations[0].GetY());
    ASSERT_DOUBLE_EQ(0.10, Locations[1].GetX());
    ASSERT_DOUBLE_EQ(10.1, Locations[1].GetY());

    Locations.clear();

    // Test with inverted vertical PolySegment
    VerticalPolySegment2.Drop(&Locations, MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10, Locations[0].GetX());
    ASSERT_DOUBLE_EQ(10.1, Locations[0].GetY());
    ASSERT_DOUBLE_EQ(0.10, Locations[1].GetX());
    ASSERT_DOUBLE_EQ(0.10, Locations[1].GetY());

    Locations.clear();

    // Test with horizontal PolySegment
    HorizontalPolySegment1.Drop(&Locations, MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10, Locations[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, Locations[0].GetY());
    ASSERT_DOUBLE_EQ(10.1, Locations[1].GetX());
    ASSERT_DOUBLE_EQ(0.10, Locations[1].GetY());

    Locations.clear();

    // Test with inverted horizontal PolySegment
    HorizontalPolySegment2.Drop(&Locations, MYEPSILON);
    ASSERT_DOUBLE_EQ(10.1, Locations[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, Locations[0].GetY());
    ASSERT_DOUBLE_EQ(0.10, Locations[1].GetX());
    ASSERT_DOUBLE_EQ(0.10, Locations[1].GetY());

    Locations.clear();

    // Tests with vertical EPSILON sized PolySegment
    VerticalPolySegment3.Drop(&Locations, MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000, Locations[0].GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Locations[0].GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Locations[1].GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Locations[1].GetY());

    Locations.clear();

    // Tests with horizontal EPSILON sized PolySegment
    HorizontalPolySegment3.Drop(&Locations, MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000, Locations[0].GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Locations[0].GetY());
    ASSERT_DOUBLE_EQ(0.1000001, Locations[1].GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Locations[1].GetY());

    }

//==================================================================================
// Reverse();
//==================================================================================
TEST_F (HVE2DPolySegmentTester, ReverseTest) 
    {

    HGF2DLocationCollection Locations;

    // Test with vertical PolySegment
    VerticalPolySegment1.Reverse();
    VerticalPolySegment1.Drop(&Locations, MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10, Locations[1].GetX());
    ASSERT_DOUBLE_EQ(0.10, Locations[1].GetY());
    ASSERT_DOUBLE_EQ(0.10, Locations[0].GetX());
    ASSERT_DOUBLE_EQ(10.1, Locations[0].GetY());

    Locations.clear();

    // Test with inverted vertical PolySegment
    VerticalPolySegment2.Reverse();
    VerticalPolySegment2.Drop(&Locations, MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10, Locations[1].GetX());
    ASSERT_DOUBLE_EQ(10.1, Locations[1].GetY());
    ASSERT_DOUBLE_EQ(0.10, Locations[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, Locations[0].GetY());

    Locations.clear();

    // Test with horizontal PolySegment
    HorizontalPolySegment1.Reverse();
    HorizontalPolySegment1.Drop(&Locations, MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10, Locations[1].GetX());
    ASSERT_DOUBLE_EQ(0.10, Locations[1].GetY());
    ASSERT_DOUBLE_EQ(10.1, Locations[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, Locations[0].GetY());

    Locations.clear();

    // Test with inverted horizontal PolySegment
    HorizontalPolySegment2.Reverse();
    HorizontalPolySegment2.Drop(&Locations, MYEPSILON);
    ASSERT_DOUBLE_EQ(10.1, Locations[1].GetX());
    ASSERT_DOUBLE_EQ(0.10, Locations[1].GetY());
    ASSERT_DOUBLE_EQ(0.10, Locations[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, Locations[0].GetY());

    Locations.clear();

    // Tests with vertical EPSILON sized PolySegment
    VerticalPolySegment3.Reverse();
    VerticalPolySegment3.Drop(&Locations, MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000, Locations[1].GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Locations[1].GetY());
    ASSERT_DOUBLE_EQ(0.1000000, Locations[0].GetX());
    ASSERT_DOUBLE_EQ(0.1000001, Locations[0].GetY());

    Locations.clear();

    // Tests with horizontal EPSILON sized PolySegment
    HorizontalPolySegment3.Reverse();
    HorizontalPolySegment3.Drop(&Locations, MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1000000, Locations[1].GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Locations[1].GetY());
    ASSERT_DOUBLE_EQ(0.1000001, Locations[0].GetX());
    ASSERT_DOUBLE_EQ(0.1000000, Locations[0].GetY());

    }

//==================================================================================
// Rotate( const HGFAngle& pi_rAngle, const HGF2DLocation& pi_rOrigin )
//==================================================================================
TEST_F (HVE2DPolySegmentTester, RotateTest) 
    {

    // Test with vertical PolySegment
    VerticalPolySegment1.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld));
    
    ASSERT_DOUBLE_EQ(-0.10000000000000000, VerticalPolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, VerticalPolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-0.10000000000000124, VerticalPolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-10.1000000000000000, VerticalPolySegment1.GetEndPoint().GetY());

    VerticalPolySegment1.Rotate(PI, HGF2DLocation(1.0, 1.0, pWorld));
    
    ASSERT_DOUBLE_EQ(2.1000000000000000, VerticalPolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(2.1000000000000000, VerticalPolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(2.1000000000000023, VerticalPolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(12.100000000000000, VerticalPolySegment1.GetEndPoint().GetY());

    // Test with horizontal PolySegment
    HorizontalPolySegment1.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld));
    
    ASSERT_DOUBLE_EQ(-0.10000000000000000, HorizontalPolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.10000000000000000, HorizontalPolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-10.1000000000000000, HorizontalPolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.09999999999999877, HorizontalPolySegment1.GetEndPoint().GetY());
       
    HorizontalPolySegment1.Rotate(PI, HGF2DLocation(1.0, 1.0, pWorld));
    
    ASSERT_DOUBLE_EQ(2.1000000000000000, HorizontalPolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(2.1000000000000000, HorizontalPolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(12.100000000000000, HorizontalPolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(2.0999999999999974, HorizontalPolySegment1.GetEndPoint().GetY());

    // Test with a NULL PolySegment
    NullPolySegment1.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld));
    
    ASSERT_TRUE(NullPolySegment1.IsNull());

    }

//==================================================================================
// Move( const HGF2DDisplacement& pi_rDisplacement )
//==================================================================================
TEST_F (HVE2DPolySegmentTester, MoveTest) 
    {

    // Test with vertical PolySegment
    VerticalPolySegment1.Move(HGF2DDisplacement(10.0, 10.0));
    
    ASSERT_DOUBLE_EQ(10.1, VerticalPolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, VerticalPolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, VerticalPolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.1, VerticalPolySegment1.GetEndPoint().GetY());

    VerticalPolySegment1.Move(HGF2DDisplacement(-10.0, -10.0));
    
    ASSERT_DOUBLE_EQ(0.099999999999999645, VerticalPolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, VerticalPolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999999999999645, VerticalPolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.10000000000000000, VerticalPolySegment1.GetEndPoint().GetY());

    VerticalPolySegment1.Move(HGF2DDisplacement(0.0, 0.0));
    
    ASSERT_DOUBLE_EQ(0.099999999999999645, VerticalPolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, VerticalPolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999999999999645, VerticalPolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.10000000000000000, VerticalPolySegment1.GetEndPoint().GetY());

    // Test with horizontal PolySegment
    HorizontalPolySegment1.Move(HGF2DDisplacement(0.0, 10.0));
    
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment1 .GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, HorizontalPolySegment1 .GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, HorizontalPolySegment1 .GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, HorizontalPolySegment1 .GetEndPoint().GetY());

    HorizontalPolySegment1.Move(HGF2DDisplacement(10.0, 0.0));
    
    ASSERT_DOUBLE_EQ(10.1, HorizontalPolySegment1 .GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, HorizontalPolySegment1 .GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.1, HorizontalPolySegment1 .GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, HorizontalPolySegment1 .GetEndPoint().GetY());

    HorizontalPolySegment1.Move(HGF2DDisplacement(-10.0, 0.0));
    
    ASSERT_DOUBLE_EQ(0.099999999999999645, HorizontalPolySegment1 .GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.10000000000000000, HorizontalPolySegment1 .GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.10000000000000000, HorizontalPolySegment1 .GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.10000000000000000, HorizontalPolySegment1 .GetEndPoint().GetY());

    HorizontalPolySegment1.Move(HGF2DDisplacement(0.0, -10.0));
    
    ASSERT_DOUBLE_EQ(0.099999999999999645, HorizontalPolySegment1 .GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, HorizontalPolySegment1 .GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.10000000000000000, HorizontalPolySegment1 .GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999645, HorizontalPolySegment1 .GetEndPoint().GetY());

    // Test with a NULL PolySegment
    NullPolySegment1.Move(HGF2DDisplacement(10.0, 10.0));
    
    ASSERT_TRUE(NullPolySegment1.IsNull());
    
    }

//==================================================================================
// Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin);
//==================================================================================
TEST_F (HVE2DPolySegmentTester, ScaleTest) 
    {

    // Test with vertical PolySegment
    VerticalPolySegment1.Scale(2.0, HGF2DLocation(0.0, 0.0, pWorld));
    
    ASSERT_DOUBLE_EQ(0.20, VerticalPolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.20, VerticalPolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.20, VerticalPolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(20.2, VerticalPolySegment1.GetEndPoint().GetY());

    VerticalPolySegment1.Scale(2.0, HGF2DLocation(0.0, 0.0, pWorld));
    
    ASSERT_DOUBLE_EQ(0.40, VerticalPolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.40, VerticalPolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.40, VerticalPolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(40.4, VerticalPolySegment1.GetEndPoint().GetY());

    VerticalPolySegment1.Scale(2.0, HGF2DLocation(1.0, 1.0, pWorld));
    
    ASSERT_DOUBLE_EQ(-0.2, VerticalPolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.2, VerticalPolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-0.2, VerticalPolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(79.8, VerticalPolySegment1.GetEndPoint().GetY());

    VerticalPolySegment1.Scale(1.0, HGF2DLocation(1.0, 1.0, pWorld));
    
    ASSERT_DOUBLE_EQ(-0.2, VerticalPolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.2, VerticalPolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-0.2, VerticalPolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(79.8, VerticalPolySegment1.GetEndPoint().GetY());

    // Test with horizontal PolySegment
    HorizontalPolySegment1.Scale(2.0, HGF2DLocation(0.0, 0.0, pWorld));
    
    ASSERT_DOUBLE_EQ(0.20, HorizontalPolySegment1 .GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.20, HorizontalPolySegment1 .GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(20.2, HorizontalPolySegment1 .GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.20, HorizontalPolySegment1 .GetEndPoint().GetY());

    HorizontalPolySegment1.Scale(2.0, HGF2DLocation(0.0, 0.0, pWorld));
    
    ASSERT_DOUBLE_EQ(0.40, HorizontalPolySegment1 .GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.40, HorizontalPolySegment1 .GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(40.4, HorizontalPolySegment1 .GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.40, HorizontalPolySegment1 .GetEndPoint().GetY()); 

    HorizontalPolySegment1.Scale(2.0, HGF2DLocation(1.0, 1.0, pWorld));
    
    ASSERT_DOUBLE_EQ(-0.2, HorizontalPolySegment1 .GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.2, HorizontalPolySegment1 .GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(79.8, HorizontalPolySegment1 .GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.2, HorizontalPolySegment1 .GetEndPoint().GetY());

    HorizontalPolySegment1.Scale(1.0, HGF2DLocation(1.0, 1.0, pWorld));
    
    ASSERT_DOUBLE_EQ(-0.2, HorizontalPolySegment1 .GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.2, HorizontalPolySegment1 .GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(79.8, HorizontalPolySegment1 .GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.2, HorizontalPolySegment1 .GetEndPoint().GetY());

    // Test with a NULL PolySegment
    NullPolySegment1.Scale(2.0, HGF2DLocation(0.0, 0.0, pWorld));
    
    ASSERT_TRUE(NullPolySegment1.IsNull());

    }

//==================================================================================
//AllocateParallelCopy(const HGFDistance& pi_rOffset,HVE2DVector::ArbitraryDirection pi_DirectionToRight, const HGF2DLine* pi_pFirstPointAlignment,
//                     const HGF2DLine* pi_pLastPointAlignment) const
//==================================================================================
TEST_F (HVE2DPolySegmentTester, AllocateParallelCopyTest)
    {

    // Test with vertical PolySegment
    HFCPtr<HVE2DPolySegment> pAVerticalPolySegment1 = VerticalPolySegment1.AllocateParallelCopy(1.0, HVE2DVector::ALPHA);

    ASSERT_DOUBLE_EQ(1.100000000000000000, pAVerticalPolySegment1->GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999756, pAVerticalPolySegment1->GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(1.100000000000000000, pAVerticalPolySegment1->GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.10000000000000000, pAVerticalPolySegment1->GetEndPoint().GetY());

    HFCPtr<HVE2DPolySegment> pBVerticalPolySegment1 = VerticalPolySegment1.AllocateParallelCopy(1.0, HVE2DVector::BETA);

    ASSERT_DOUBLE_EQ(-0.9000000000000000, pBVerticalPolySegment1->GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000013, pBVerticalPolySegment1->GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-0.9000000000000000, pBVerticalPolySegment1->GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1000000000000000, pBVerticalPolySegment1->GetEndPoint().GetY());

    // Test with inverted vertical PolySegment
    HFCPtr<HVE2DPolySegment> pAVerticalPolySegment2 = VerticalPolySegment2.AllocateParallelCopy(1.0, HVE2DVector::ALPHA);

    ASSERT_DOUBLE_EQ(-0.9000000000000000, pAVerticalPolySegment2->GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1000000000000000, pAVerticalPolySegment2->GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-0.9000000000000000, pAVerticalPolySegment2->GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000013, pAVerticalPolySegment2->GetEndPoint().GetY());

    HFCPtr<HVE2DPolySegment> pBVerticalPolySegment2 = VerticalPolySegment2.AllocateParallelCopy(1.0, HVE2DVector::BETA);

    ASSERT_DOUBLE_EQ(1.100000000000000000, pBVerticalPolySegment2->GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.10000000000000000, pBVerticalPolySegment2->GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(1.100000000000000000, pBVerticalPolySegment2->GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999756, pBVerticalPolySegment2->GetEndPoint().GetY());

    // Test with horizontal PolySegment
    HFCPtr<HVE2DPolySegment> pAHorizontalPolySegment1 = HorizontalPolySegment1.AllocateParallelCopy(1.0, HVE2DVector::ALPHA);

    ASSERT_DOUBLE_EQ(0.099999999999999825, pAHorizontalPolySegment1->GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.90000000000000000, pAHorizontalPolySegment1->GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.10000000000000000, pAHorizontalPolySegment1->GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.90000000000000000, pAHorizontalPolySegment1->GetEndPoint().GetY());

    HFCPtr<HVE2DPolySegment> pBHorizontalPolySegment1 = HorizontalPolySegment1.AllocateParallelCopy(1.0, HVE2DVector::BETA);

    ASSERT_DOUBLE_EQ(0.10, pBHorizontalPolySegment1->GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(1.10, pBHorizontalPolySegment1->GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, pBHorizontalPolySegment1->GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(1.10, pBHorizontalPolySegment1->GetEndPoint().GetY());
    
    // Test with horizontal PolySegment
    HFCPtr<HVE2DPolySegment> pAHorizontalPolySegment2 = HorizontalPolySegment2.AllocateParallelCopy(1.0, HVE2DVector::ALPHA);

    ASSERT_DOUBLE_EQ(10.1, pAHorizontalPolySegment2->GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(1.10, pAHorizontalPolySegment2->GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, pAHorizontalPolySegment2->GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(1.10, pAHorizontalPolySegment2->GetEndPoint().GetY());

    HFCPtr<HVE2DPolySegment> pBHorizontalPolySegment2 = HorizontalPolySegment2.AllocateParallelCopy(1.0, HVE2DVector::BETA);

    ASSERT_DOUBLE_EQ(10.10000000000000000, pBHorizontalPolySegment2->GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.90000000000000000, pBHorizontalPolySegment2->GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.099999999999999825, pBHorizontalPolySegment2->GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.90000000000000000, pBHorizontalPolySegment2->GetEndPoint().GetY());

    // Tests with vertical EPSILON sized PolySegment
    HFCPtr<HVE2DPolySegment> pAVerticalPolySegment3 = VerticalPolySegment3.AllocateParallelCopy(1.0, HVE2DVector::ALPHA);

    ASSERT_DOUBLE_EQ(1.100000000000000000, pAVerticalPolySegment3->GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.099999999999999756, pAVerticalPolySegment3->GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(1.100000000000000000, pAVerticalPolySegment3->GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.100000099999999760, pAVerticalPolySegment3->GetEndPoint().GetY());

    HFCPtr<HVE2DPolySegment> pBVerticalPolySegment3 = VerticalPolySegment3.AllocateParallelCopy(1.0, HVE2DVector::BETA);

    ASSERT_DOUBLE_EQ(-0.9000000000000000, pBVerticalPolySegment3->GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000013, pBVerticalPolySegment3->GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(-0.9000000000000000, pBVerticalPolySegment3->GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10000010000000013, pBVerticalPolySegment3->GetEndPoint().GetY());

    // Test with horizontal PolySegment
    HFCPtr<HVE2DPolySegment> pAHorizontalPolySegment3 = HorizontalPolySegment3.AllocateParallelCopy(1.0, HVE2DVector::ALPHA);

    ASSERT_DOUBLE_EQ(0.099999999999999825, pAHorizontalPolySegment3->GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.90000000000000000, pAHorizontalPolySegment3->GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.100000099999999830, pAHorizontalPolySegment3->GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-0.90000000000000000, pAHorizontalPolySegment3->GetEndPoint().GetY());

    HFCPtr<HVE2DPolySegment> pBHorizontalPolySegment3 = HorizontalPolySegment3.AllocateParallelCopy(1.0, HVE2DVector::BETA);

    ASSERT_DOUBLE_EQ(0.10000000000000000, pBHorizontalPolySegment3->GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(1.10000000000000000, pBHorizontalPolySegment3->GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10000010000000006, pBHorizontalPolySegment3->GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(1.10000000000000000, pBHorizontalPolySegment3->GetEndPoint().GetY());

    }

//==================================================================================
// GetClosestSegment(const HGF2DLocation& pi_rLocation) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, GetClosestSegmentTest)
    {

    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment1.GetClosestSegment(HGF2DLocation(0.0, 0.0, pWorld)).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment1.GetClosestSegment(HGF2DLocation(0.0, 0.0, pWorld)).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment1.GetClosestSegment(HGF2DLocation(0.0, 0.0, pWorld)).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, VerticalPolySegment1.GetClosestSegment(HGF2DLocation(0.0, 0.0, pWorld)).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment1.GetClosestSegment(HGF2DLocation(0.0, 0.0, pWorld)).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment1.GetClosestSegment(HGF2DLocation(0.0, 0.0, pWorld)).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, HorizontalPolySegment1.GetClosestSegment(HGF2DLocation(0.0, 0.0, pWorld)).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment1.GetClosestSegment(HGF2DLocation(0.0, 0.0, pWorld)).GetEndPoint().GetY());

    }

//==================================================================================
// Relative position calculation test
// CalculateRelativePosition(const HGF2DLocation& pi_rPointOnLinear) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CalculateRelativePositionTest)
    {

    // Test with vertical PolySegment
    ASSERT_NEAR(0.0, VerticalPolySegment1.CalculateRelativePosition(VerticalPoint0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1, VerticalPolySegment1.CalculateRelativePosition(VerticalPoint0d1));
    ASSERT_DOUBLE_EQ(0.5, VerticalPolySegment1.CalculateRelativePosition(VerticalPoint0d5));
    ASSERT_DOUBLE_EQ(1.0, VerticalPolySegment1.CalculateRelativePosition(VerticalPoint1d0));

    // Test with inverted vertical PolySegment
    ASSERT_DOUBLE_EQ(1.0, VerticalPolySegment2.CalculateRelativePosition(VerticalPoint0d0));
    ASSERT_DOUBLE_EQ(0.9, VerticalPolySegment2.CalculateRelativePosition(VerticalPoint0d1));
    ASSERT_DOUBLE_EQ(0.5, VerticalPolySegment2.CalculateRelativePosition(VerticalPoint0d5));
    ASSERT_NEAR(0.0, VerticalPolySegment2.CalculateRelativePosition(VerticalPoint1d0), MYEPSILON);

    // Test with horizontal PolySegment
    ASSERT_NEAR(0.0, HorizontalPolySegment1.CalculateRelativePosition(HorizontalPoint0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1, HorizontalPolySegment1.CalculateRelativePosition(HorizontalPoint0d1));
    ASSERT_DOUBLE_EQ(0.5, HorizontalPolySegment1.CalculateRelativePosition(HorizontalPoint0d5));
    ASSERT_DOUBLE_EQ(1.0, HorizontalPolySegment1.CalculateRelativePosition(HorizontalPoint1d0));

    // Test with inverted horizontal PolySegment
    ASSERT_DOUBLE_EQ(1.0, HorizontalPolySegment2.CalculateRelativePosition(HorizontalPoint0d0));
    ASSERT_DOUBLE_EQ(0.9, HorizontalPolySegment2.CalculateRelativePosition(HorizontalPoint0d1));
    ASSERT_DOUBLE_EQ(0.5, HorizontalPolySegment2.CalculateRelativePosition(HorizontalPoint0d5));
    ASSERT_NEAR(0.0, HorizontalPolySegment2.CalculateRelativePosition(HorizontalPoint1d0), MYEPSILON);

    // Tests with vertical EPSILON sized PolySegment
    ASSERT_NEAR(0.0, VerticalPolySegment3.CalculateRelativePosition(Vertical3Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.099999999944488854, VerticalPolySegment3.CalculateRelativePosition(Vertical3Point0d1));
    ASSERT_DOUBLE_EQ(0.500000000000000000, VerticalPolySegment3.CalculateRelativePosition(Vertical3Point0d5));
    ASSERT_DOUBLE_EQ(1.000000000000000000, VerticalPolySegment3.CalculateRelativePosition(Vertical3Point1d0));

    // Tests with horizontal EPSILON sized PolySegment
    ASSERT_NEAR(0.0, HorizontalPolySegment3.CalculateRelativePosition(Horizontal3Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.099999999944488854, HorizontalPolySegment3.CalculateRelativePosition(Horizontal3Point0d1));
    ASSERT_DOUBLE_EQ(0.500000000000000000, HorizontalPolySegment3.CalculateRelativePosition(Horizontal3Point0d5));
    ASSERT_DOUBLE_EQ(1.000000000000000000, HorizontalPolySegment3.CalculateRelativePosition(Horizontal3Point1d0));

    // Tests with miscalenious EPSILON size PolySegment
    ASSERT_NEAR(0.0, MiscPolySegment3.CalculateRelativePosition(Misc3Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.099999999943141071, MiscPolySegment3.CalculateRelativePosition(Misc3Point0d1));
    ASSERT_DOUBLE_EQ(0.500000000000000000, MiscPolySegment3.CalculateRelativePosition(Misc3Point0d5));
    ASSERT_DOUBLE_EQ(1.000000000000000000, MiscPolySegment3.CalculateRelativePosition(Misc3Point1d0));

    // Test with very large PolySegment
    ASSERT_NEAR(0.0, LargePolySegment1.CalculateRelativePosition(LargePoint0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1, LargePolySegment1.CalculateRelativePosition(LargePoint0d1));
    ASSERT_DOUBLE_EQ(0.5, LargePolySegment1.CalculateRelativePosition(LargePoint0d5));
    ASSERT_DOUBLE_EQ(1.0, LargePolySegment1.CalculateRelativePosition(LargePoint1d0));

    // Test with PolySegments way into positive regions
    ASSERT_NEAR(0.0, PositivePolySegment1.CalculateRelativePosition(PositivePoint0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1, PositivePolySegment1.CalculateRelativePosition(PositivePoint0d1));
    ASSERT_DOUBLE_EQ(0.5, PositivePolySegment1.CalculateRelativePosition(PositivePoint0d5));
    ASSERT_DOUBLE_EQ(1.0, PositivePolySegment1.CalculateRelativePosition(PositivePoint1d0));

    // Test with PolySegments way into negative regions
    ASSERT_NEAR(0.0, NegativePolySegment1.CalculateRelativePosition(NegativePoint0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.1, NegativePolySegment1.CalculateRelativePosition(NegativePoint0d1));
    ASSERT_DOUBLE_EQ(0.5, NegativePolySegment1.CalculateRelativePosition(NegativePoint0d5));
    ASSERT_DOUBLE_EQ(1.0, NegativePolySegment1.CalculateRelativePosition(NegativePoint1d0));

    }

//==================================================================================
// RayArea Calculation test
// CalculateRayArea(const HGF2DLocation& pi_rPoint) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CalculateRayAreaTest)
    {
    
     // Test with vertical PolySegment
    ASSERT_DOUBLE_EQ(0.505, VerticalPolySegment1.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));
    
    // Test with inverted vertical PolySegment
    ASSERT_DOUBLE_EQ(0.005, VerticalPolySegment2.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

   // Test with horizontal PolySegment
    ASSERT_DOUBLE_EQ(0.005, HorizontalPolySegment1.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Test with inverted horizontal PolySegment
    ASSERT_DOUBLE_EQ(0.505, HorizontalPolySegment2.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Tests with vertical EPSILON sized PolySegment
    ASSERT_DOUBLE_EQ(0.0050000050000000009, VerticalPolySegment3.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Tests with horizontal EPSILON sized PolySegment
    ASSERT_DOUBLE_EQ(0.005, HorizontalPolySegment3.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Tests with miscalenious EPSILON size PolySegment
    ASSERT_DOUBLE_EQ(0.0050000048814800363, MiscPolySegment3.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Test with very large PolySegment
    ASSERT_DOUBLE_EQ(-9.4999999999999990e+246, LargePolySegment1.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Test with PolySegments way into positive regions
    ASSERT_DOUBLE_EQ(2.0500000000000001e+247, PositivePolySegment1.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Test with PolySegments way into negative regions
    ASSERT_DOUBLE_EQ(2.0500000000000001e+247, NegativePolySegment1.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    }

//==================================================================================
// Shortening tests
// Shorten(double pi_StartRelativePos, double pi_EndRelativePos);
// Shorten(const HGF2DLocation& pi_rNewStartPoint,
//         const HGF2DLocation& pi_rNewEndPoint);
// ShortenTo(const HGF2DLocation& pi_rNewEndPoint);
// ShortenTo(double pi_EndRelativePosition);
// ShortenFrom(const HGF2DLocation& pi_rNewStartPoint);
// ShortenFrom(double pi_StartRelativePosition);
//==================================================================================
TEST_F (HVE2DPolySegmentTester, ShorteningTest)
    {

    // Test with vertical PolySegment
    HVE2DPolySegment    PolySegment1(VerticalPolySegment1);

    PolySegment1.Shorten(0.2, 0.8);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(2.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(8.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_NEAR(0.1000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(5.1000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(5.1000009999999989, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.Shorten(0.0, 0.8);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(8.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.Shorten(0.2, 1.0);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(2.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.Shorten(0.0, 1.0);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.ShortenTo(0.8);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(8.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.099999, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.ShortenTo(1.0);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.ShortenTo(0.0);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.ShortenFrom(0.2);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(2.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.099999, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.100000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.ShortenFrom(1.0);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.ShortenFrom(0.0);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.ShortenFrom(VerticalMidPoint1);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(5.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.ShortenFrom(PolySegment1.GetStartPoint());
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.ShortenFrom(PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.ShortenTo(VerticalMidPoint1);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(5.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.ShortenTo(PolySegment1.GetStartPoint());
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.ShortenTo(PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), VerticalMidPoint1);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(5.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.Shorten(VerticalMidPoint1, PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(5.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment1;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    // Test with inverted vertical PolySegment
    PolySegment1 = VerticalPolySegment2;
    PolySegment1.Shorten(0.2, 0.8);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(8.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(2.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_NEAR(0.100000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(5.100000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.100000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(5.099999, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.Shorten(0.0, 0.8);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(2.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.Shorten(0.2, 1.0);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(8.100000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.Shorten(0.0, 1.0);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.ShortenTo(0.8);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(2.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_NEAR(0.1000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.100000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000009999999989, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.ShortenTo(1.0);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.ShortenTo(0.0);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.ShortenFrom(0.2);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(8.100000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000099999999890, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.ShortenFrom(1.0);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.ShortenFrom(0.0);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.ShortenFrom(VerticalMidPoint1);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(5.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.ShortenFrom(PolySegment1.GetStartPoint());
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.ShortenFrom(PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.ShortenTo(VerticalMidPoint1);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(5.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.ShortenTo(PolySegment1.GetStartPoint());
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.ShortenTo(PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), VerticalMidPoint1);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(5.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.Shorten(VerticalMidPoint1, PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(5.100000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment2;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(10.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    // Test with horizontal PolySegment
    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.Shorten(0.2, 0.8);
    ASSERT_NEAR(2.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(8.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_NEAR(5.1000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(5.1000009999999989, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.Shorten(0.0, 0.8);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(8.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.Shorten(0.2, 1.0);
    ASSERT_NEAR(2.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.Shorten(0.0, 1.0);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.ShortenTo(0.8);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(8.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.099999, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.ShortenTo(1.0);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.ShortenTo(0.0);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.ShortenFrom(0.2);
    ASSERT_NEAR(2.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_NEAR(10.099999, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.100000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.ShortenFrom(1.0);
    ASSERT_NEAR(10.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.ShortenFrom(0.0);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.ShortenFrom(HorizontalMidPoint1);
    ASSERT_NEAR(5.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.ShortenFrom(PolySegment1.GetStartPoint());
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.ShortenFrom(PolySegment1.GetEndPoint());
    ASSERT_NEAR(10.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.ShortenTo(HorizontalMidPoint1);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(5.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.ShortenTo(PolySegment1.GetStartPoint());
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.ShortenTo(PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), HorizontalMidPoint1);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(5.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.Shorten(HorizontalMidPoint1, PolySegment1.GetEndPoint());
    ASSERT_NEAR(5.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment1;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    // Test with inverted horizontal PolySegment
    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.Shorten(0.2, 0.8);
    ASSERT_NEAR(8.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(2.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_NEAR(5.1000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(5.0999990000000004, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.Shorten(0.0, 0.8);
    ASSERT_NEAR(10.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(2.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.Shorten(0.2, 1.0);
    ASSERT_NEAR(8.100000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.Shorten(0.0, 1.0);
    ASSERT_NEAR(10.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.ShortenTo(0.8);
    ASSERT_NEAR(10.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(2.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_NEAR(10.100000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000009999999989, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.ShortenTo(1.0);
    ASSERT_NEAR(10.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.ShortenTo(0.0);
    ASSERT_NEAR(10.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.ShortenFrom(0.2);
    ASSERT_NEAR(8.100000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_NEAR(0.100000999999998900, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.ShortenFrom(1.0);
    ASSERT_NEAR(0.0999999999999996450, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.0999999999999996451, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.ShortenFrom(0.0);
    ASSERT_NEAR(10.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.ShortenFrom(HorizontalMidPoint1);
    ASSERT_NEAR(5.100000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.ShortenFrom(PolySegment1.GetStartPoint());
    ASSERT_NEAR(10.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.ShortenFrom(PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.ShortenTo(HorizontalMidPoint1);
    ASSERT_NEAR(10.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(5.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.ShortenTo(PolySegment1.GetStartPoint());
    ASSERT_NEAR(10.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(10.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.ShortenTo(PolySegment1.GetEndPoint());
    ASSERT_NEAR(10.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), HorizontalMidPoint1);
    ASSERT_NEAR(10.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(5.10, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.Shorten(HorizontalMidPoint1, PolySegment1.GetEndPoint());
    ASSERT_NEAR(5.100000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment2;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), PolySegment1.GetEndPoint());
    ASSERT_NEAR(10.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.099999999999999645, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.100000000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    // Tests with vertical EPSILON sized PolySegment
    PolySegment1 = VerticalPolySegment3;
    PolySegment1.Shorten(0.2, 0.8);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000002, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000008, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_NEAR(0.10000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000005000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000005000001, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.Shorten(0.0, 0.8);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000008, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.Shorten(0.2, 1.0);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000002, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000010, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.Shorten(0.0, 1.0);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000010, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.ShortenTo(0.8);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000008, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_NEAR(0.10000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009999999, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.ShortenTo(1.0);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000001, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.ShortenTo(0.0);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.ShortenFrom(0.2);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000002, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000010, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_NEAR(0.10000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009999999, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000010000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.ShortenFrom(1.0);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000001, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000001, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.ShortenFrom(0.0);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000001, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.ShortenFrom(VerticalMidPoint3);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000005, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000010, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.ShortenFrom(PolySegment1.GetStartPoint());
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000010, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.ShortenFrom(PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000001, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000001, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.ShortenTo(VerticalMidPoint3);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000005, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.ShortenTo(PolySegment1.GetStartPoint());
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.ShortenTo(PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000001, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), VerticalMidPoint3);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000005, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.Shorten(VerticalMidPoint3, PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000005, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000010, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = VerticalPolySegment3;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000001, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    // Tests with horizontal EPSILON sized PolySegment
    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.Shorten(0.2, 0.8);
    ASSERT_NEAR(0.10000002, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000008, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_NEAR(0.10000005000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000005000001, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.Shorten(0.0, 0.8);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000008, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.Shorten(0.2, 1.0);
    ASSERT_NEAR(0.10000002, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000001, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.Shorten(0.0, 1.0);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000010, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.ShortenTo(0.8);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000008, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_NEAR(0.10000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000009999999, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.ShortenTo(1.0);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000001, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.ShortenTo(0.0);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.ShortenFrom(0.2);
    ASSERT_NEAR(0.10000002, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000010, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_NEAR(0.10000009999999, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000010000000, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.ShortenFrom(1.0);
    ASSERT_NEAR(0.1000001, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000001, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.ShortenFrom(0.0);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000001, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.ShortenFrom(HorizontalMidPoint3);
    ASSERT_NEAR(0.10000005, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000010, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.ShortenFrom(PolySegment1.GetStartPoint());
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000001, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.ShortenFrom(PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.1000001, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000001, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.ShortenTo(HorizontalMidPoint3);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000005, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.ShortenTo(PolySegment1.GetStartPoint());
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.ShortenTo(PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000001, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), HorizontalMidPoint3);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000005, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.Shorten(HorizontalMidPoint3, PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.10000005, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000010, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = HorizontalPolySegment3;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1000001, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1000000, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    // Tests with miscalenious EPSILON size PolySegment
    PolySegment1 = MiscPolySegment3;
    PolySegment1.Shorten(0.2, 0.8);
    ASSERT_NEAR(0.10000000432879229, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000001952592015, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000001731516911, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000007810368057, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_NEAR(0.10000001082198071, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000004881480036, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000001082198286, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000004881481012, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.Shorten(0.0, 0.8);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000001731516911, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000007810368057, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.Shorten(0.2, 1.0);
    ASSERT_NEAR(0.10000000432879229, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000001952592015, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000002164396139, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762960071, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.Shorten(0.0, 1.0);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000002164396139, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762960071, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.ShortenTo(0.8);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000001731516911, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000007810368057, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000002164395923, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762959096, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.ShortenTo(1.0);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000002164396139, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762960071, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.ShortenTo(0.0);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.ShortenFrom(0.2);
    ASSERT_NEAR(0.10000000432879229, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000001952592015, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000002164396139, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762960071, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_NEAR(0.10000002164395923, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762959096, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000002164396139, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762960071, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.ShortenFrom(1.0);
    ASSERT_NEAR(0.10000002164396139, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762960071, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000002164396139, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762960071, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.ShortenFrom(0.0);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000002164396139, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762960071, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.ShortenFrom(MiscMidPoint3);
    ASSERT_NEAR(0.10000001082198071, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000004881480036, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000002164396139, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762960071, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.ShortenFrom(PolySegment1.GetStartPoint());
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000002164396139, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762960071, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.ShortenFrom(PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.10000002164396139, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762960071, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000002164396139, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762960071, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.ShortenTo(MiscMidPoint3);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000001082198071, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000004881480036, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.ShortenTo(PolySegment1.GetStartPoint());
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.1, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.ShortenTo(PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000002164396139, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762960071, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), MiscMidPoint3);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000001082198071, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000004881480036, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.Shorten(MiscMidPoint3, PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.10000001082198071, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000004881480036, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000002164396139, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762960071, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    PolySegment1 = MiscPolySegment3;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), PolySegment1.GetEndPoint());
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000000000000000, PolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.10000002164396139, PolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.10000009762960071, PolySegment1.GetEndPoint().GetY(), MYEPSILON);

    // Test with very large PolySegment
    PolySegment1 = LargePolySegment1;
    PolySegment1.Shorten(0.2, 0.8);
    ASSERT_NEAR(1.000E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-13.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(7.000E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(11.00E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.Shorten(0.0, 0.8);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(7.000E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(11.00E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.Shorten(0.2, 1.0);
    ASSERT_NEAR(1.000E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-13.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(9.000E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(19.00E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.Shorten(0.0, 1.0);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(9.000E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(19.00E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.ShortenTo(0.8);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(7.000E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(11.00E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_NEAR(-1.000000000000000E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.00000000000000E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(8.9999989999999997E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(1.8999996000000001E124, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.ShortenTo(1.0);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(9.000E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(19.00E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.ShortenTo(0.0);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.ShortenFrom(0.2);
    ASSERT_NEAR(1.000E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-13.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(9.000E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(19.00E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_NEAR(8.9999989999999997E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(1.8999996000000001E124, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(9.0000000000000000E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(19.000000000000000E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.ShortenFrom(1.0);
    ASSERT_NEAR(9.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(19.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(9.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(19.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.ShortenFrom(0.0);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(9.000E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(19.00E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    // Due to EPSILON problems, some of the following methods used to not work properly
    PolySegment1 = LargePolySegment1;
    PolySegment1.ShortenFrom(LargeMidPoint1);
    ASSERT_NEAR(4.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-1.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(9.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(19.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.ShortenFrom(PolySegment1.GetStartPoint());
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(9.000E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(19.00E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.ShortenFrom(PolySegment1.GetEndPoint());
    ASSERT_NEAR(9.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(19.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(9.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(19.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.ShortenTo(LargeMidPoint1);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(4.000E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.ShortenTo(PolySegment1.GetStartPoint());
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.ShortenTo(PolySegment1.GetEndPoint());
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(9.000E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(19.00E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), LargeMidPoint1);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(4.000E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.Shorten(LargeMidPoint1, PolySegment1.GetEndPoint());
    ASSERT_NEAR(4.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-1.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(9.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(19.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = LargePolySegment1;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), PolySegment1.GetEndPoint());
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(9.000E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(19.00E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    // Test with PolySegments way into positive regions
    PolySegment1 = PositivePolySegment1;
    PolySegment1.Shorten(0.2, 0.8);
    ASSERT_NEAR(3.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(25.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(9.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(37.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.Shorten(0.0, 0.8);
    ASSERT_NEAR(1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(9.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(37.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.Shorten(0.2, 1.0);
    ASSERT_NEAR(3.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(25.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.Shorten(0.0, 1.0);
    ASSERT_NEAR(1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.ShortenTo(0.8);
    ASSERT_NEAR(1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(9.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(37.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_NEAR(1.0000000E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(21.000000E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(1.0999999E124, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(4.0999998E124, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.ShortenTo(1.0);
    ASSERT_NEAR(1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.ShortenTo(0.0);
    ASSERT_NEAR(1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(1.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(21.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.ShortenFrom(0.2);
    ASSERT_NEAR(3.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(25.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_NEAR(1.0999999E124, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(4.0999998E124, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(11.000000E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(41.000000E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.ShortenFrom(1.0);
    ASSERT_NEAR(11E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(41E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(11E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(41E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.ShortenFrom(0.0);
    ASSERT_NEAR(1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.ShortenFrom(PositiveMidPoint1);
    ASSERT_NEAR(6.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(31.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.ShortenFrom(PolySegment1.GetStartPoint());
    ASSERT_NEAR(1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.ShortenFrom(PolySegment1.GetEndPoint());
    ASSERT_NEAR(11E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(41E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(11E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(41E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.ShortenTo(PositiveMidPoint1);
    ASSERT_NEAR(1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(6.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(31.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.ShortenTo(PolySegment1.GetStartPoint());
    ASSERT_NEAR(1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(1.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(21.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.ShortenTo(PolySegment1.GetEndPoint());
    ASSERT_NEAR(1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), PositiveMidPoint1);
    ASSERT_NEAR(1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(6.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(31.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.Shorten(PositiveMidPoint1, PolySegment1.GetEndPoint());
    ASSERT_NEAR(6.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(31.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = PositivePolySegment1;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), PolySegment1.GetEndPoint());
    ASSERT_NEAR(1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.Shorten(0.2, 0.8);
    ASSERT_NEAR(-3E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-25E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-9E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-37E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.Shorten(0.0, 0.8);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-9.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-37.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.Shorten(0.2, 1.0);
    ASSERT_NEAR(-3.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-25.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.Shorten(0.0, 1.0);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.ShortenTo(0.8);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-9.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-37.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_NEAR(-1.0000000E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.000000E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-1.0999999E124, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-4.0999998E124, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.ShortenTo(1.0);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.ShortenTo(0.0);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.ShortenFrom(0.2);
    ASSERT_NEAR(-3.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-25.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_NEAR(-1.0999999E124, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-4.0999998E124, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-11.000000E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-41.000000E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.ShortenFrom(1.0);
    ASSERT_NEAR(-11E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-41E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-11E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-41E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.ShortenFrom(0.0);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.ShortenFrom(NegativeMidPoint1);
    ASSERT_NEAR(-6.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-31.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.ShortenFrom(PolySegment1.GetStartPoint());
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.ShortenFrom(PolySegment1.GetEndPoint());
    ASSERT_NEAR(-11E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-41E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-11E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-41E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.ShortenTo(NegativeMidPoint1);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-6.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-31.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.ShortenTo(PolySegment1.GetStartPoint());
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.ShortenTo(PolySegment1.GetEndPoint());
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), NegativeMidPoint1);
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-6.00E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-31.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.Shorten(NegativeMidPoint1, PolySegment1.GetEndPoint());
    ASSERT_NEAR(-6.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-31.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    PolySegment1 = NegativePolySegment1;
    PolySegment1.Shorten(PolySegment1.GetStartPoint(), PolySegment1.GetEndPoint());
    ASSERT_NEAR(-1.00E123, PolySegment1.GetStartPoint().GetX(), 10E110);
    ASSERT_NEAR(-21.0E123, PolySegment1.GetStartPoint().GetY(), 10E110);
    ASSERT_NEAR(-11.0E123, PolySegment1.GetEndPoint().GetX(), 10E110);
    ASSERT_NEAR(-41.0E123, PolySegment1.GetEndPoint().GetY(), 10E110);

    }

//==================================================================================
// Auto intersection tests
// AutoCrosses()const
// AutoIntersect(HGF2DLocationCollection* po_pPoints) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, AutoIntersectTest) 
    {

    // Test with vertical PolySegment
    ASSERT_FALSE(VerticalPolySegment1.AutoCrosses());

    // Test with inverted vertical PolySegment
    ASSERT_FALSE(VerticalPolySegment2.AutoCrosses());

    // Test with horizontal PolySegment
    ASSERT_FALSE(HorizontalPolySegment1.AutoCrosses());

    // Test with inverted horizontal PolySegment
    ASSERT_FALSE(HorizontalPolySegment2.AutoCrosses());

    // Tests with vertical EPSILON sized PolySegment
    ASSERT_FALSE(VerticalPolySegment3.AutoCrosses());

    // Tests with horizontal EPSILON sized PolySegment
    ASSERT_FALSE(HorizontalPolySegment3.AutoCrosses());

    // Tests with miscalenious EPSILON size PolySegment
    ASSERT_FALSE(MiscPolySegment3.AutoCrosses());

    // Test with very large PolySegment
    ASSERT_FALSE(LargePolySegment1.AutoCrosses());

    // Test with a NULL PolySegment
    ASSERT_FALSE(NullPolySegment1.AutoCrosses());

    }

//==================================================================================
// Closest point calculation test
// CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CalculateClosestPointTest)
    {

    // Test with vertical PolySegment
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalPolySegment1.CalculateClosestPoint(VerticalClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalPolySegment1.CalculateClosestPoint(VerticalClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalPolySegment1.CalculateClosestPoint(VerticalClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(5.000000000000000000, VerticalPolySegment1.CalculateClosestPoint(VerticalClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalPolySegment1.CalculateClosestPoint(VerticalClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(7.000000000000000000, VerticalPolySegment1.CalculateClosestPoint(VerticalClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(0.099999999976716936, VerticalPolySegment1.CalculateClosestPoint(VerticalClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(10.00000000000000000, VerticalPolySegment1.CalculateClosestPoint(VerticalClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalPolySegment1.CalculateClosestPoint(VerticalCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(5.100000000000000000, VerticalPolySegment1.CalculateClosestPoint(VerticalCloseMidPoint1).GetY());

    // Test with inverted vertical PolySegment
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalPolySegment2.CalculateClosestPoint(VerticalClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalPolySegment2.CalculateClosestPoint(VerticalClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalPolySegment2.CalculateClosestPoint(VerticalClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(5.000000000000000000, VerticalPolySegment2.CalculateClosestPoint(VerticalClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalPolySegment2.CalculateClosestPoint(VerticalClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(7.000000000000000000, VerticalPolySegment2.CalculateClosestPoint(VerticalClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(0.099999999976716936, VerticalPolySegment2.CalculateClosestPoint(VerticalClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(10.00000000000000000, VerticalPolySegment2.CalculateClosestPoint(VerticalClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalPolySegment2.CalculateClosestPoint(VerticalCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(5.100000000000000000, VerticalPolySegment2.CalculateClosestPoint(VerticalCloseMidPoint1).GetY());

    // Test with horizontal PolySegment
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalPolySegment1.CalculateClosestPoint(HorizontalClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalPolySegment1.CalculateClosestPoint(HorizontalClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(5.000000000000000000, HorizontalPolySegment1.CalculateClosestPoint(HorizontalClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalPolySegment1.CalculateClosestPoint(HorizontalClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(7.000000000000000000, HorizontalPolySegment1.CalculateClosestPoint(HorizontalClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalPolySegment1.CalculateClosestPoint(HorizontalClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(10.00000000000000000, HorizontalPolySegment1.CalculateClosestPoint(HorizontalClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(0.099999999976716936, HorizontalPolySegment1.CalculateClosestPoint(HorizontalClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(5.100000000000000000, HorizontalPolySegment1.CalculateClosestPoint(HorizontalCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalPolySegment1.CalculateClosestPoint(HorizontalCloseMidPoint1).GetY());

    // Test with inverted horizontal PolySegment
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalPolySegment2.CalculateClosestPoint(HorizontalClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalPolySegment2.CalculateClosestPoint(HorizontalClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(5.000000000000000000, HorizontalPolySegment2.CalculateClosestPoint(HorizontalClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalPolySegment2.CalculateClosestPoint(HorizontalClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(7.000000000000000000, HorizontalPolySegment2.CalculateClosestPoint(HorizontalClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalPolySegment2.CalculateClosestPoint(HorizontalClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(10.00000000000000000, HorizontalPolySegment2.CalculateClosestPoint(HorizontalClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(0.099999999976716936, HorizontalPolySegment2.CalculateClosestPoint(HorizontalClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(5.099999999999999600, HorizontalPolySegment2.CalculateClosestPoint(HorizontalCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalPolySegment2.CalculateClosestPoint(HorizontalCloseMidPoint1).GetY());

    // Tests with vertical EPSILON sized PolySegment
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalPolySegment3.CalculateClosestPoint(VerticalClosePoint3A).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalPolySegment3.CalculateClosestPoint(VerticalClosePoint3A).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalPolySegment3.CalculateClosestPoint(VerticalClosePoint3B).GetX());
    ASSERT_DOUBLE_EQ(0.100000066666666680, VerticalPolySegment3.CalculateClosestPoint(VerticalClosePoint3B).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalPolySegment3.CalculateClosestPoint(VerticalClosePoint3C).GetX());
    ASSERT_DOUBLE_EQ(0.100000033333333340, VerticalPolySegment3.CalculateClosestPoint(VerticalClosePoint3C).GetY());
    ASSERT_DOUBLE_EQ(0.099999999976716936, VerticalPolySegment3.CalculateClosestPoint(VerticalClosePoint3D).GetX());
    ASSERT_DOUBLE_EQ(0.100000050000000000, VerticalPolySegment3.CalculateClosestPoint(VerticalClosePoint3D).GetY());
    ASSERT_DOUBLE_EQ(0.100000000000000000, VerticalPolySegment3.CalculateClosestPoint(VerticalCloseMidPoint3).GetX());
    ASSERT_DOUBLE_EQ(0.100000050000000000, VerticalPolySegment3.CalculateClosestPoint(VerticalCloseMidPoint3).GetY());

    // Tests with horizontal EPSILON sized PolySegment
    ASSERT_DOUBLE_EQ(0.100000066666666680, HorizontalPolySegment3.CalculateClosestPoint(HorizontalClosePoint3B).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalPolySegment3.CalculateClosestPoint(HorizontalClosePoint3A).GetY());
    ASSERT_DOUBLE_EQ(0.100000066666666680, HorizontalPolySegment3.CalculateClosestPoint(HorizontalClosePoint3B).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalPolySegment3.CalculateClosestPoint(HorizontalClosePoint3B).GetY());
    ASSERT_DOUBLE_EQ(0.100000033333333340, HorizontalPolySegment3.CalculateClosestPoint(HorizontalClosePoint3C).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalPolySegment3.CalculateClosestPoint(HorizontalClosePoint3C).GetY());
    ASSERT_DOUBLE_EQ(0.100000050000000000, HorizontalPolySegment3.CalculateClosestPoint(HorizontalClosePoint3D).GetX());
    ASSERT_DOUBLE_EQ(0.099999999976716936, HorizontalPolySegment3.CalculateClosestPoint(HorizontalClosePoint3D).GetY());
    ASSERT_DOUBLE_EQ(0.100000050000000000, HorizontalPolySegment3.CalculateClosestPoint(HorizontalCloseMidPoint3).GetX());
    ASSERT_DOUBLE_EQ(0.100000000000000000, HorizontalPolySegment3.CalculateClosestPoint(HorizontalCloseMidPoint3).GetY());

    // Tests with miscalenious EPSILON size PolySegment
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscPolySegment3.CalculateClosestPoint(MiscClosePoint3A).GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscPolySegment3.CalculateClosestPoint(MiscClosePoint3A).GetY());
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscPolySegment3.CalculateClosestPoint(MiscClosePoint3B).GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscPolySegment3.CalculateClosestPoint(MiscClosePoint3B).GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, MiscPolySegment3.CalculateClosestPoint(MiscClosePoint3C).GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, MiscPolySegment3.CalculateClosestPoint(MiscClosePoint3C).GetY());
    ASSERT_DOUBLE_EQ(0.10000002164396139, MiscPolySegment3.CalculateClosestPoint(MiscClosePoint3D).GetX());
    ASSERT_DOUBLE_EQ(0.10000009762960071, MiscPolySegment3.CalculateClosestPoint(MiscClosePoint3D).GetY());
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscPolySegment3.CalculateClosestPoint(MiscCloseMidPoint3).GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscPolySegment3.CalculateClosestPoint(MiscCloseMidPoint3).GetY());

    // Test with very large PolySegment
    ASSERT_DOUBLE_EQ(4.000000000000000000E123, LargePolySegment1.CalculateClosestPoint(LargeClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(-1.00000000000000000E123, LargePolySegment1.CalculateClosestPoint(LargeClosePoint1A).GetY());
    ASSERT_DOUBLE_EQ(6.94117647058823470E+123, LargePolySegment1.CalculateClosestPoint(LargeClosePoint1B).GetX());
    ASSERT_DOUBLE_EQ(1.07647058823529420E+124, LargePolySegment1.CalculateClosestPoint(LargeClosePoint1B).GetY());
    ASSERT_DOUBLE_EQ(1.05882352941176280E+123, LargePolySegment1.CalculateClosestPoint(LargeClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(-1.2764705882352939E+124, LargePolySegment1.CalculateClosestPoint(LargeClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(4.00023529411764720E+123, LargePolySegment1.CalculateClosestPoint(LargeClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(-9.9905882352941167E+122, LargePolySegment1.CalculateClosestPoint(LargeClosePoint1D).GetY());

    // Test with a NULL PolySegment
    ASSERT_NEAR(0.0, NullPolySegment1.CalculateClosestPoint(NegativeClosePoint1A).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, NullPolySegment1.CalculateClosestPoint(NegativeClosePoint1A).GetY(), MYEPSILON);

    // Tests with special points
    ASSERT_DOUBLE_EQ(10.1, MiscPolySegment1.CalculateClosestPoint(VeryFarPoint).GetX());
    ASSERT_DOUBLE_EQ(10.1, MiscPolySegment1.CalculateClosestPoint(VeryFarPoint).GetY());
    ASSERT_DOUBLE_EQ(5.10, MiscPolySegment1.CalculateClosestPoint(MidPoint).GetX());
    ASSERT_DOUBLE_EQ(5.10, MiscPolySegment1.CalculateClosestPoint(MidPoint).GetY());
    ASSERT_DOUBLE_EQ(0.10, MiscPolySegment1.CalculateClosestPoint(VeryFarNegativePoint).GetX());
    ASSERT_DOUBLE_EQ(0.10, MiscPolySegment1.CalculateClosestPoint(VeryFarNegativePoint).GetY());
    ASSERT_DOUBLE_EQ(10.1, MiscPolySegment1.CalculateClosestPoint(VeryFarAlignedPoint).GetX());
    ASSERT_DOUBLE_EQ(10.1, MiscPolySegment1.CalculateClosestPoint(VeryFarAlignedPoint).GetY());
    ASSERT_DOUBLE_EQ(0.10, MiscPolySegment1.CalculateClosestPoint(VeryFarAlignedNegativePoint).GetX());
    ASSERT_DOUBLE_EQ(0.10, MiscPolySegment1.CalculateClosestPoint(VeryFarAlignedNegativePoint).GetY());
    ASSERT_DOUBLE_EQ(0.10, MiscPolySegment1.CalculateClosestPoint(MiscPolySegment1.GetStartPoint()).GetX());
    ASSERT_DOUBLE_EQ(0.10, MiscPolySegment1.CalculateClosestPoint(MiscPolySegment1.GetStartPoint()).GetY());
    ASSERT_DOUBLE_EQ(10.1, MiscPolySegment1.CalculateClosestPoint(MiscPolySegment1.GetEndPoint()).GetX());
    ASSERT_DOUBLE_EQ(10.1, MiscPolySegment1.CalculateClosestPoint(MiscPolySegment1.GetEndPoint()).GetY());

    }

//==================================================================================
// Intersection test (with other PolySegments only)
// Intersect(const HVE2DVector& pi_rVector, HGF2DLocationCollection* po_pCrossPoints) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, IntersectTest)
    {
     
    HGF2DLocationCollection   DumPoints;

    // Test with extent disjoint PolySegments
    ASSERT_EQ(0,  MiscPolySegment1.Intersect(DisjointPolySegment1, &DumPoints));

    // Test with disjoint but touching by a side PolySegments
    ASSERT_EQ(0,  MiscPolySegment1.Intersect(ContiguousExtentPolySegment1, &DumPoints));

    // Test with disjoint but touching by a tip PolySegments but not linked
    ASSERT_EQ(0,  MiscPolySegment1.Intersect(FlirtingExtentPolySegment1, &DumPoints));

    // Test with disjoint but touching by a tip PolySegments linked
    ASSERT_EQ(0,  MiscPolySegment1.Intersect(FlirtingExtentLinkedPolySegment1, &DumPoints));

    // Test with vertical PolySegment
    ASSERT_EQ(0,  VerticalPolySegment1.Intersect(MiscPolySegment1, &DumPoints));

    // Test with inverted vertical PolySegment
    ASSERT_EQ(0,  VerticalPolySegment2.Intersect(MiscPolySegment1, &DumPoints));

    // Test with close vertical PolySegments
    ASSERT_EQ(0,  VerticalPolySegment1.Intersect(VerticalPolySegment4, &DumPoints));

    // Test with horizontal PolySegment
    ASSERT_EQ(0,  HorizontalPolySegment1.Intersect(MiscPolySegment1, &DumPoints));

    // Test with inverted horizontal PolySegment
    ASSERT_EQ(0,  HorizontalPolySegment2.Intersect(MiscPolySegment1, &DumPoints));

    // Test with parallel PolySegments
    ASSERT_EQ(0,  MiscPolySegment1.Intersect(ParallelPolySegment1, &DumPoints));

    // Test with near parallel PolySegments
    ASSERT_EQ(0,  MiscPolySegment1.Intersect(NearParallelPolySegment1, &DumPoints));

    // Tests with close near parallel PolySegments
    ASSERT_EQ(0,  MiscPolySegment1.Intersect(CloseNearParallelPolySegment1, &DumPoints));

    // Tests with connected PolySegments
    // At start point...
    ASSERT_EQ(0,  MiscPolySegment1.Intersect(ConnectedPolySegment1, &DumPoints));
    ASSERT_EQ(0,  MiscPolySegment1.Intersect(ConnectingPolySegment1, &DumPoints));

    // At end point ...
    ASSERT_EQ(0,  MiscPolySegment1.Intersect(ConnectedPolySegment1A, &DumPoints));
    ASSERT_EQ(0,  MiscPolySegment1.Intersect(ConnectingPolySegment1A, &DumPoints));

    // Tests with linked PolySegments
    ASSERT_EQ(0,  MiscPolySegment1.Intersect(LinkedPolySegment1, &DumPoints));
    ASSERT_EQ(0,  MiscPolySegment1.Intersect(LinkedPolySegment1A, &DumPoints));

    // Tests with vertical EPSILON sized PolySegment
    ASSERT_EQ(0,  VerticalPolySegment3.Intersect(MiscPolySegment1, &DumPoints));

    // Tests with horizontal EPSILON sized PolySegment
    ASSERT_EQ(0,  HorizontalPolySegment3.Intersect(MiscPolySegment1, &DumPoints));

    // Tests with miscalenious EPSILON size PolySegment
    ASSERT_EQ(0,  MiscPolySegment3.Intersect(MiscPolySegment1, &DumPoints));

    // Test with very large PolySegment
    ASSERT_EQ(0,  LargePolySegment1.Intersect(MiscPolySegment1, &DumPoints));

    // Test with PolySegments way into positive regions
    ASSERT_EQ(0,  PositivePolySegment1.Intersect(MiscPolySegment1, &DumPoints));

    // Test with PolySegments way into negative regions
    ASSERT_EQ(0,  NegativePolySegment1.Intersect(MiscPolySegment1, &DumPoints));

    // Test with a NULL PolySegment
    ASSERT_EQ(0,  NullPolySegment1.Intersect(MiscPolySegment1, &DumPoints));

    }

//==================================================================================
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
//==================================================================================
TEST_F (HVE2DPolySegmentTester, ContiguousnessTest)
    {

    HGF2DLocationCollection     DumPoints;
    HGF2DLocation   FirstDumPoint;
    HGF2DLocation   SecondDumPoint;

    // Test with vertical PolySegments
    ASSERT_TRUE(VerticalPolySegment1.AreContiguous(VerticalPolySegment2));

    ASSERT_TRUE(VerticalPolySegment1.AreContiguousAt(VerticalPolySegment2, VerticalMidPoint1));

    ASSERT_EQ(2, VerticalPolySegment1.ObtainContiguousnessPoints(VerticalPolySegment2, &DumPoints));  
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetY());

    VerticalPolySegment1.ObtainContiguousnessPointsAt(VerticalPolySegment2, VerticalMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(0.10, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetY());   

    DumPoints.clear();

    // Test with horizontal PolySegments
    ASSERT_TRUE(HorizontalPolySegment1.AreContiguous(HorizontalPolySegment2));
    ASSERT_TRUE(HorizontalPolySegment1.AreContiguousAt(HorizontalPolySegment2, HorizontalMidPoint1));
    ASSERT_EQ(2, HorizontalPolySegment1.ObtainContiguousnessPoints(HorizontalPolySegment2, &DumPoints));
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[1].GetY());

    HorizontalPolySegment1.ObtainContiguousnessPointsAt(HorizontalPolySegment2, HorizontalMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, SecondDumPoint.GetY());   

    ASSERT_FALSE(HorizontalPolySegment1.AreContiguous(MiscPolySegment1));

    DumPoints.clear();

    // Test with positive slope PolySegments
    ASSERT_TRUE(MiscPolySegment1.AreContiguous(MiscPolySegment2));
    ASSERT_TRUE(MiscPolySegment1.AreContiguousAt(MiscPolySegment2, MiscMidPoint1));
    ASSERT_EQ(2, MiscPolySegment1.ObtainContiguousnessPoints(MiscPolySegment2, &DumPoints));
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.1, DumPoints[1].GetY());

    MiscPolySegment1.ObtainContiguousnessPointsAt(MiscPolySegment2, MiscMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.1, SecondDumPoint.GetY());  

    ASSERT_FALSE(MiscPolySegment1.AreContiguous(LargePolySegment1));

    DumPoints.clear();

    // Tests with negative slope PolySegments
    ASSERT_TRUE(MiscPolySegment6.AreContiguous(MiscPolySegment7));
    ASSERT_TRUE(MiscPolySegment6.AreContiguousAt(MiscPolySegment7, MiscMidPoint6));
    ASSERT_EQ(2, MiscPolySegment6.ObtainContiguousnessPoints(MiscPolySegment7, &DumPoints));
    ASSERT_DOUBLE_EQ(0.10000000000000000, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(-9.8000000000000007, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0000000000000000, DumPoints[1].GetY());

    MiscPolySegment6.ObtainContiguousnessPointsAt(MiscPolySegment7, MiscMidPoint6, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(0.10000000000000000, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(0.10000000000000000, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(-9.8000000000000007, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0000000000000000, SecondDumPoint.GetY());  

    ASSERT_FALSE(MiscPolySegment6.AreContiguous(MiscPolySegment1));

    DumPoints.clear();

    // Tests with vertical EPSILON sized PolySegment
    ASSERT_FALSE(VerticalPolySegment3.AreContiguous(VerticalPolySegment2));
    ASSERT_FALSE(VerticalPolySegment3.AreContiguous(HorizontalPolySegment3));

    DumPoints.clear();

    // Tests with horizontal EPSILON sized PolySegment
    ASSERT_FALSE(HorizontalPolySegment3.AreContiguous(HorizontalPolySegment2));
    ASSERT_FALSE(HorizontalPolySegment3.AreContiguous(MiscPolySegment3));

    DumPoints.clear();

    // Test with a very large PolySegment
    // Precision problem
    ASSERT_TRUE(LargePolySegment1.AreContiguous(LargePolySegment2));
    ASSERT_TRUE(LargePolySegment1.AreContiguousAt(LargePolySegment2, LargeMidPoint1));
    ASSERT_EQ(2, LargePolySegment1.ObtainContiguousnessPoints(LargePolySegment2, &DumPoints));
    ASSERT_DOUBLE_EQ(-1.00E123, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(9.000E123, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(19.00E123, DumPoints[1].GetY());

    LargePolySegment1.ObtainContiguousnessPointsAt(LargePolySegment2, LargeMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(-1.00E123, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(9.000E123, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(19.00E123, SecondDumPoint.GetY());  

    ASSERT_FALSE(LargePolySegment1.AreContiguous(HorizontalPolySegment1));

    DumPoints.clear();

    // Test with a PolySegment way into positive regions
    ASSERT_TRUE(PositivePolySegment1.AreContiguous(PositivePolySegment2));
    ASSERT_TRUE(PositivePolySegment1.AreContiguousAt(PositivePolySegment2, PositiveMidPoint1));
    ASSERT_EQ(2, PositivePolySegment1.ObtainContiguousnessPoints(PositivePolySegment2, &DumPoints));
    ASSERT_DOUBLE_EQ(1.00E123, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(21.0E123, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(11.0E123, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(41.0E123, DumPoints[1].GetY());

    PositivePolySegment1.ObtainContiguousnessPointsAt(PositivePolySegment2, PositiveMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(1.00E123, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(21.0E123, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(11.0E123, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(41.0E123, SecondDumPoint.GetY());   

    ASSERT_FALSE(PositivePolySegment1.AreContiguous(HorizontalPolySegment1));

    DumPoints.clear();

    // Test with PolySegment way into negative PolySegments
    ASSERT_TRUE(NegativePolySegment1.AreContiguous(NegativePolySegment2));
    ASSERT_TRUE(NegativePolySegment1.AreContiguousAt(NegativePolySegment2, NegativeMidPoint1));
    ASSERT_EQ(2, NegativePolySegment1.ObtainContiguousnessPoints(NegativePolySegment2, &DumPoints));
    ASSERT_DOUBLE_EQ(-1.00E123, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, DumPoints[0].GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, DumPoints[1].GetY());

    NegativePolySegment1.ObtainContiguousnessPointsAt(NegativePolySegment2, NegativeMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_DOUBLE_EQ(-1.00E123, FirstDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-21.0E123, FirstDumPoint.GetY());
    ASSERT_DOUBLE_EQ(-11.0E123, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(-41.0E123, SecondDumPoint.GetY());  

    ASSERT_FALSE(NegativePolySegment1.AreContiguous(HorizontalPolySegment1));

    }

//==================================================================================
// Cloning tests
// Clone() const;
// AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CloningTest)
    {
    
    //General Clone Test
    HFCPtr<HVE2DPolySegment> pClone = (HVE2DPolySegment*)MiscPolySegment1.Clone();
    ASSERT_EQ(pWorld, pClone->GetCoordSys());
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(0.1, 0.1, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(10.1, 10.1, pWorld))); 

    // Test with the same coordinate system
    HFCPtr<HVE2DPolySegment> pClone3 = (HVE2DPolySegment*)MiscPolySegment1.AllocateCopyInCoordSys(pWorld);
    ASSERT_EQ(pWorld, pClone3->GetCoordSys());
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(0.1, 0.1, pWorld))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(10.1, 10.1, pWorld)));

    // Test with a translation between systems
    Translation = HGF2DDisplacement (10.0, 10.0);
    HGF2DTranslation myTranslation(Translation);
    HFCPtr<HGF2DCoordSys> pWorldTranslation = new HGF2DCoordSys(myTranslation, pWorld);

    HFCPtr<HVE2DPolySegment> pClone5 = (HVE2DPolySegment*)MiscPolySegment1.AllocateCopyInCoordSys(pWorldTranslation);
    ASSERT_EQ(pWorldTranslation, pClone5->GetCoordSys());
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(-9.9, -9.9, pWorldTranslation))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(0.1, 0.1, pWorldTranslation)));

    // Test with a stretch between systems
    HGF2DStretch myStretch;
    myStretch.SetTranslation(Translation);
    myStretch.SetXScaling(0.5);
    myStretch.SetYScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldStretch = new HGF2DCoordSys(myStretch, pWorld);

    HFCPtr<HVE2DPolySegment> pClone6 = (HVE2DPolySegment*)MiscPolySegment1.AllocateCopyInCoordSys(pWorldStretch);
    ASSERT_EQ(pWorldStretch, pClone6->GetCoordSys());
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(-19.8, -19.8, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(0.2, 0.2, pWorldStretch)));

    // Test with a similitude between systems
    HGF2DSimilitude mySimilitude;
    mySimilitude.SetRotation(PI);
    mySimilitude.SetScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldSimilitude = new HGF2DCoordSys(mySimilitude, pWorld);

    HFCPtr<HVE2DPolySegment> pClone7 = (HVE2DPolySegment*)MiscPolySegment1.AllocateCopyInCoordSys(pWorldSimilitude);
    ASSERT_EQ(pClone7->GetCoordSys(), pWorldSimilitude);
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-0.2,-0.2 , pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-20.2, -20.2, pWorldSimilitude)));

    // Test with a affine between systems
    HGF2DAffine myAffine;
    myAffine.SetTranslation(Translation);
    myAffine.SetRotation(PI);
    myAffine.SetXScaling(0.5);
    myAffine.SetYScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldAffine = new HGF2DCoordSys(myAffine, pWorld);

    HFCPtr<HVE2DPolySegment> pClone8 = (HVE2DPolySegment*)MiscPolySegment1.AllocateCopyInCoordSys(pWorldAffine);
    ASSERT_EQ(pWorldAffine, pClone8->GetCoordSys());
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(19.8, 19.8, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-0.2, -0.2, pWorldAffine)));

    }

//==================================================================================
// Interaction info with other PolySegment
// Crosses(const HVE2DVector& pi_rVector) const;
// AreAdjacent(const HVE2DVector& pi_rVector) const;
// IsPointOn(const HGF2DLocation& pi_rTestPoint) const;
// IsPointOnSCS(const HGF2DLocation& pi_rTestPoint) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, InteractionTest)
    {

    // Tests with a vertical PolySegment
    ASSERT_FALSE(VerticalPolySegment1.Crosses(VerticalPolySegment2));
    ASSERT_TRUE(VerticalPolySegment1.AreAdjacent(VerticalPolySegment2));

    ASSERT_FALSE(VerticalPolySegment1.Crosses(MiscPolySegment1));
    ASSERT_FALSE(VerticalPolySegment1.AreAdjacent(MiscPolySegment1));

    ASSERT_FALSE(VerticalPolySegment1.Crosses(VerticalPolySegment3));
    ASSERT_TRUE(VerticalPolySegment1.AreAdjacent(VerticalPolySegment3));

    ASSERT_FALSE(VerticalPolySegment1.Crosses(LargePolySegment1));
    ASSERT_FALSE(VerticalPolySegment1.AreAdjacent(LargePolySegment1));

    ASSERT_FALSE(VerticalPolySegment1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(VerticalPolySegment1.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(VerticalPolySegment1.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(1.1*MYEPSILON, 0)));
    ASSERT_FALSE(VerticalPolySegment1.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(-1.1*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalPolySegment1.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(0.9*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalPolySegment1.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(-0.9*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalPolySegment1.IsPointOn(VerticalPolySegment1.GetStartPoint()));
    ASSERT_TRUE(VerticalPolySegment1.IsPointOn(VerticalPolySegment1.GetEndPoint()));
    ASSERT_FALSE(VerticalPolySegment1.IsPointOn(VerticalPolySegment1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(VerticalPolySegment1.IsPointOn(VerticalPolySegment1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(VerticalPolySegment1.IsPointOn(VerticalMidPoint1));
    ASSERT_TRUE(VerticalPolySegment1.IsPointOn(VerticalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_FALSE(VerticalPolySegment1.IsPointOnSCS(VerticalPolySegment1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(VerticalPolySegment1.IsPointOnSCS(VerticalPolySegment1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(VerticalPolySegment1.IsPointOnSCS(VerticalMidPoint1));
    ASSERT_TRUE(VerticalPolySegment1.IsPointOnSCS(VerticalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with an inverted vertical PolySegment
    ASSERT_FALSE(VerticalPolySegment2.Crosses(VerticalPolySegment1));
    ASSERT_TRUE(VerticalPolySegment2.AreAdjacent(VerticalPolySegment1));

    ASSERT_FALSE(VerticalPolySegment2.Crosses(MiscPolySegment1));
    ASSERT_FALSE(VerticalPolySegment2.AreAdjacent(MiscPolySegment1));

    ASSERT_FALSE(VerticalPolySegment2.Crosses(VerticalPolySegment3));
    ASSERT_TRUE(VerticalPolySegment2.AreAdjacent(VerticalPolySegment3));

    ASSERT_FALSE(VerticalPolySegment2.Crosses(LargePolySegment1));
    ASSERT_FALSE(VerticalPolySegment2.AreAdjacent(LargePolySegment1));

    ASSERT_FALSE(VerticalPolySegment2.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(VerticalPolySegment2.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(VerticalPolySegment2.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(1.1*MYEPSILON, 0)));
    ASSERT_FALSE(VerticalPolySegment2.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(-1.1*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalPolySegment2.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(0.9*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalPolySegment2.IsPointOn(VerticalMidPoint1-HGF2DDisplacement(-0.9*MYEPSILON, 0)));
    ASSERT_TRUE(VerticalPolySegment2.IsPointOn(VerticalPolySegment2.GetStartPoint()));
    ASSERT_TRUE(VerticalPolySegment2.IsPointOn(VerticalPolySegment2.GetEndPoint()));
    ASSERT_TRUE(VerticalPolySegment2.IsPointOn(VerticalMidPoint1));
    ASSERT_FALSE(VerticalPolySegment2.IsPointOn(VerticalPolySegment2.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(VerticalPolySegment2.IsPointOn(VerticalPolySegment2.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(VerticalPolySegment2.IsPointOn(VerticalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_TRUE(VerticalPolySegment2.IsPointOnSCS(VerticalMidPoint1));
    ASSERT_FALSE(VerticalPolySegment2.IsPointOnSCS(VerticalPolySegment2.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(VerticalPolySegment2.IsPointOnSCS(VerticalPolySegment2.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(VerticalPolySegment2.IsPointOnSCS(VerticalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with a horizontal PolySegment
    ASSERT_FALSE(HorizontalPolySegment1.Crosses(HorizontalPolySegment2));
    ASSERT_TRUE(HorizontalPolySegment1.AreAdjacent(HorizontalPolySegment2));

    ASSERT_FALSE(HorizontalPolySegment1.Crosses(MiscPolySegment1));
    ASSERT_FALSE(HorizontalPolySegment1.AreAdjacent(MiscPolySegment1));

    ASSERT_FALSE(HorizontalPolySegment1.Crosses(HorizontalPolySegment3));
    ASSERT_TRUE(HorizontalPolySegment1.AreAdjacent(HorizontalPolySegment3));

    ASSERT_FALSE(HorizontalPolySegment1.Crosses(LargePolySegment1));
    ASSERT_FALSE(HorizontalPolySegment1.AreAdjacent(LargePolySegment1));

    ASSERT_FALSE(HorizontalPolySegment1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(HorizontalPolySegment1.IsPointOn(HGF2DLocation(0.1-1.1*MYEPSILON, 0.1, pWorld)));
    ASSERT_FALSE(HorizontalPolySegment1.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, 1.1*MYEPSILON)));
    ASSERT_FALSE(HorizontalPolySegment1.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, -1.1*MYEPSILON)));
    ASSERT_TRUE(HorizontalPolySegment1.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, 0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalPolySegment1.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, -0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalPolySegment1.IsPointOn(HorizontalPolySegment1.GetStartPoint()));
    ASSERT_TRUE(HorizontalPolySegment1.IsPointOn(HorizontalPolySegment1.GetEndPoint()));
    ASSERT_TRUE(HorizontalPolySegment1.IsPointOn(HorizontalMidPoint1));
    ASSERT_FALSE(HorizontalPolySegment1.IsPointOn(HorizontalPolySegment1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(HorizontalPolySegment1.IsPointOn(HorizontalPolySegment1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(HorizontalPolySegment1.IsPointOn(HorizontalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_TRUE(HorizontalPolySegment1.IsPointOnSCS(HorizontalMidPoint1));
    ASSERT_FALSE(HorizontalPolySegment1.IsPointOnSCS(HorizontalPolySegment1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(HorizontalPolySegment1.IsPointOnSCS(HorizontalPolySegment1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(HorizontalPolySegment1.IsPointOnSCS(HorizontalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with an inverted horizontal PolySegment
    ASSERT_FALSE(HorizontalPolySegment2.Crosses(HorizontalPolySegment1));
    ASSERT_TRUE(HorizontalPolySegment2.AreAdjacent(HorizontalPolySegment1));

    ASSERT_FALSE(HorizontalPolySegment2.Crosses(MiscPolySegment1));
    ASSERT_FALSE(HorizontalPolySegment2.AreAdjacent(MiscPolySegment1));

    ASSERT_FALSE(HorizontalPolySegment2.Crosses(HorizontalPolySegment3));
    ASSERT_TRUE(HorizontalPolySegment2.AreAdjacent(HorizontalPolySegment3));

    ASSERT_FALSE(HorizontalPolySegment2.Crosses(LargePolySegment1));
    ASSERT_FALSE(HorizontalPolySegment2.AreAdjacent(LargePolySegment1));

    ASSERT_FALSE(HorizontalPolySegment2.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(HorizontalPolySegment2.IsPointOn(HGF2DLocation(0.1-1.1*MYEPSILON, 0.1, pWorld)));
    ASSERT_FALSE(HorizontalPolySegment2.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, 1.1*MYEPSILON)));
    ASSERT_FALSE(HorizontalPolySegment2.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, -1.1*MYEPSILON)));
    ASSERT_TRUE(HorizontalPolySegment2.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, 0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalPolySegment2.IsPointOn(HorizontalMidPoint1-HGF2DDisplacement(0, -0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalPolySegment2.IsPointOn(HorizontalPolySegment2.GetStartPoint()));
    ASSERT_TRUE(HorizontalPolySegment2.IsPointOn(HorizontalPolySegment2.GetEndPoint()));
    ASSERT_TRUE(HorizontalPolySegment2.IsPointOn(HorizontalMidPoint1));
    ASSERT_FALSE(HorizontalPolySegment2.IsPointOn(HorizontalPolySegment2.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(HorizontalPolySegment2.IsPointOn(HorizontalPolySegment2.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(HorizontalPolySegment2.IsPointOn(HorizontalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_TRUE(HorizontalPolySegment2.IsPointOnSCS(HorizontalMidPoint1));
    ASSERT_FALSE(HorizontalPolySegment2.IsPointOnSCS(HorizontalPolySegment2.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(HorizontalPolySegment2.IsPointOnSCS(HorizontalPolySegment2.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(HorizontalPolySegment2.IsPointOnSCS(HorizontalMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with a positive slope PolySegment
    ASSERT_FALSE(MiscPolySegment1.Crosses(MiscPolySegment2));
    ASSERT_TRUE(MiscPolySegment1.AreAdjacent(MiscPolySegment2));

    ASSERT_FALSE(MiscPolySegment1.Crosses(HorizontalPolySegment1));
    ASSERT_FALSE(MiscPolySegment1.AreAdjacent(HorizontalPolySegment1));

    ASSERT_FALSE(MiscPolySegment1.Crosses(MiscPolySegment3));
    ASSERT_FALSE(MiscPolySegment1.AreAdjacent(MiscPolySegment3));

    ASSERT_FALSE(MiscPolySegment1.Crosses(LargePolySegment1));
    ASSERT_FALSE(MiscPolySegment1.AreAdjacent(LargePolySegment1));

    ASSERT_FALSE(MiscPolySegment1.IsPointOn(HGF2DLocation(20, 20, pWorld)));
    ASSERT_FALSE(MiscPolySegment1.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(MiscPolySegment1.IsPointOn(MiscMidPoint1-HGF2DDisplacement(1.1*MYEPSILON, -1.1*MYEPSILON)));
    ASSERT_FALSE(MiscPolySegment1.IsPointOn(MiscMidPoint1-HGF2DDisplacement(-1.1*MYEPSILON, 1.1*MYEPSILON)));
    ASSERT_TRUE(MiscPolySegment1.IsPointOn(MiscMidPoint1-HGF2DDisplacement(0.7*MYEPSILON, -0.7*MYEPSILON)));
    ASSERT_TRUE(MiscPolySegment1.IsPointOn(MiscMidPoint1-HGF2DDisplacement(-0.7*MYEPSILON, 0.7*MYEPSILON)));
    ASSERT_TRUE(MiscPolySegment1.IsPointOn(MiscPolySegment1.GetStartPoint()));
    ASSERT_TRUE(MiscPolySegment1.IsPointOn(MiscPolySegment1.GetEndPoint()));
    ASSERT_TRUE(MiscPolySegment1.IsPointOn(MiscMidPoint1));
    ASSERT_FALSE(MiscPolySegment1.IsPointOn(MiscPolySegment1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(MiscPolySegment1.IsPointOn(MiscPolySegment1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(MiscPolySegment1.IsPointOn(MiscMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_TRUE(MiscPolySegment1.IsPointOnSCS(MiscMidPoint1));
    ASSERT_FALSE(MiscPolySegment1.IsPointOnSCS(MiscPolySegment1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(MiscPolySegment1.IsPointOnSCS(MiscPolySegment1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(MiscPolySegment1.IsPointOnSCS(MiscMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with a negative slope PolySegment
    ASSERT_FALSE(MiscPolySegment2.Crosses(MiscPolySegment1));
    ASSERT_TRUE(MiscPolySegment2.AreAdjacent(MiscPolySegment1));

    ASSERT_FALSE(MiscPolySegment2.IsPointOn(HGF2DLocation(20, 20, pWorld)));
    ASSERT_FALSE(MiscPolySegment2.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(MiscPolySegment2.IsPointOn(MiscMidPoint1-HGF2DDisplacement(1.1*MYEPSILON, -1.1*MYEPSILON)));
    ASSERT_FALSE(MiscPolySegment2.IsPointOn(MiscMidPoint1-HGF2DDisplacement(-1.1*MYEPSILON, 1.1*MYEPSILON)));
    ASSERT_TRUE(MiscPolySegment2.IsPointOn(MiscMidPoint1-HGF2DDisplacement(0.7*MYEPSILON, -0.7*MYEPSILON)));
    ASSERT_TRUE(MiscPolySegment2.IsPointOn(MiscMidPoint1-HGF2DDisplacement(-0.7*MYEPSILON, 0.7*MYEPSILON)));
    ASSERT_TRUE(MiscPolySegment2.IsPointOn(MiscPolySegment1.GetStartPoint()));
    ASSERT_TRUE(MiscPolySegment2.IsPointOn(MiscPolySegment1.GetEndPoint()));
    ASSERT_TRUE(MiscPolySegment2.IsPointOn(MiscMidPoint1));
    ASSERT_FALSE(MiscPolySegment2.IsPointOn(MiscPolySegment1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(MiscPolySegment2.IsPointOn(MiscPolySegment1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(MiscPolySegment2.IsPointOn(MiscMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_TRUE(MiscPolySegment2.IsPointOnSCS(MiscMidPoint1));
    ASSERT_FALSE(MiscPolySegment2.IsPointOnSCS(MiscPolySegment1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(MiscPolySegment2.IsPointOnSCS(MiscPolySegment1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(MiscPolySegment2.IsPointOnSCS(MiscMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    // Tests with a vertical EPSILON sized PolySegment
    ASSERT_FALSE(VerticalPolySegment3.Crosses(VerticalPolySegment1));
    ASSERT_TRUE(VerticalPolySegment3.AreAdjacent(VerticalPolySegment1));

    ASSERT_FALSE(VerticalPolySegment3.Crosses(MiscPolySegment1));
    ASSERT_FALSE(VerticalPolySegment3.AreAdjacent(MiscPolySegment1));

    ASSERT_FALSE(VerticalPolySegment3.Crosses(MiscPolySegment3));
    ASSERT_FALSE(VerticalPolySegment3.AreAdjacent(MiscPolySegment3));

    ASSERT_FALSE(VerticalPolySegment3.Crosses(LargePolySegment1));
    ASSERT_FALSE(VerticalPolySegment3.AreAdjacent(LargePolySegment1));

    ASSERT_FALSE(VerticalPolySegment3.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(VerticalPolySegment3.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(VerticalPolySegment3.IsPointOn(VerticalMidPoint3-HGF2DDisplacement(1.1*MYEPSILON, 1.1*MYEPSILON)));
    ASSERT_FALSE(VerticalPolySegment3.IsPointOn(VerticalMidPoint3-HGF2DDisplacement(-1.1*MYEPSILON, -1.1*MYEPSILON)));
    ASSERT_TRUE(VerticalPolySegment3.IsPointOn(VerticalMidPoint3-HGF2DDisplacement(0.9*MYEPSILON, 0.9*MYEPSILON)));
    ASSERT_TRUE(VerticalPolySegment3.IsPointOn(VerticalMidPoint3-HGF2DDisplacement(-0.9*MYEPSILON, -0.9*MYEPSILON)));
    ASSERT_TRUE(VerticalPolySegment3.IsPointOn(VerticalPolySegment3.GetStartPoint()));
    ASSERT_TRUE(VerticalPolySegment3.IsPointOn(VerticalPolySegment3.GetEndPoint()));
    ASSERT_TRUE(VerticalPolySegment3.IsPointOn(VerticalMidPoint3));

    ASSERT_TRUE(VerticalPolySegment3.IsPointOnSCS(VerticalPolySegment3.GetStartPoint()));
    ASSERT_TRUE(VerticalPolySegment3.IsPointOnSCS(VerticalPolySegment3.GetEndPoint()));
    ASSERT_TRUE(VerticalPolySegment3.IsPointOnSCS(VerticalMidPoint3));

    // Tests with an horizontal EPSILON SIZED PolySegment
    ASSERT_FALSE(HorizontalPolySegment3.Crosses(HorizontalPolySegment1));
    ASSERT_TRUE(HorizontalPolySegment3.AreAdjacent(HorizontalPolySegment1));

    ASSERT_FALSE(HorizontalPolySegment3.Crosses(MiscPolySegment1));
    ASSERT_FALSE(HorizontalPolySegment3.AreAdjacent(MiscPolySegment1));

    ASSERT_FALSE(HorizontalPolySegment3.Crosses(MiscPolySegment3));
    ASSERT_FALSE(HorizontalPolySegment3.AreAdjacent(MiscPolySegment3));

    ASSERT_FALSE(HorizontalPolySegment3.Crosses(LargePolySegment1));
    ASSERT_FALSE(HorizontalPolySegment3.AreAdjacent(LargePolySegment1));

    ASSERT_FALSE(HorizontalPolySegment3.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(HorizontalPolySegment3.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(HorizontalPolySegment3.IsPointOn(HorizontalMidPoint3-HGF2DDisplacement(0, 1.1*MYEPSILON)));
    ASSERT_FALSE(HorizontalPolySegment3.IsPointOn(HorizontalMidPoint3-HGF2DDisplacement(0, -1.1*MYEPSILON)));
    ASSERT_TRUE(HorizontalPolySegment3.IsPointOn(HorizontalMidPoint3-HGF2DDisplacement(0, 0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalPolySegment3.IsPointOn(HorizontalMidPoint3-HGF2DDisplacement(0, -0.9*MYEPSILON)));
    ASSERT_TRUE(HorizontalPolySegment3.IsPointOn(HorizontalPolySegment3.GetStartPoint()));
    ASSERT_TRUE(HorizontalPolySegment3.IsPointOn(HorizontalPolySegment3.GetEndPoint()));
    ASSERT_TRUE(HorizontalPolySegment3.IsPointOn(HorizontalMidPoint3));

    ASSERT_TRUE(HorizontalPolySegment3.IsPointOnSCS(HorizontalPolySegment3.GetStartPoint()));
    ASSERT_TRUE(HorizontalPolySegment3.IsPointOnSCS(HorizontalPolySegment3.GetEndPoint()));
    ASSERT_TRUE(HorizontalPolySegment3.IsPointOnSCS(HorizontalMidPoint3));

    // Tests with a miscalenious EPSILON sized PolySegment
    ASSERT_FALSE(MiscPolySegment3.Crosses(MiscPolySegment1));
    ASSERT_FALSE(MiscPolySegment3.AreAdjacent(MiscPolySegment1));

    ASSERT_FALSE(MiscPolySegment3.Crosses(LargePolySegment1));
    ASSERT_FALSE(MiscPolySegment3.AreAdjacent(LargePolySegment1));

    ASSERT_FALSE(MiscPolySegment3.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(MiscPolySegment3.IsPointOn(HGF2DLocation(0.1, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(MiscPolySegment3.IsPointOn(MiscMidPoint3-HGF2DDisplacement(1.1*MYEPSILON, -1.1*MYEPSILON)));
    ASSERT_FALSE(MiscPolySegment3.IsPointOn(MiscMidPoint3-HGF2DDisplacement(-1.1*MYEPSILON, 1.1*MYEPSILON)));
    ASSERT_TRUE(MiscPolySegment3.IsPointOn(MiscMidPoint3-HGF2DDisplacement(0.9*MYEPSILON, 0.9*MYEPSILON)));
    ASSERT_TRUE(MiscPolySegment3.IsPointOn(MiscMidPoint3-HGF2DDisplacement(-0.9*MYEPSILON, -0.9*MYEPSILON)));
    ASSERT_TRUE(MiscPolySegment3.IsPointOn(MiscPolySegment3.GetStartPoint()));
    ASSERT_TRUE(MiscPolySegment3.IsPointOn(MiscPolySegment3.GetEndPoint()));
    ASSERT_TRUE(MiscPolySegment3.IsPointOn(MiscMidPoint3));

    ASSERT_TRUE(MiscPolySegment3.IsPointOnSCS(MiscPolySegment3.GetStartPoint()));
    ASSERT_TRUE(MiscPolySegment3.IsPointOnSCS(MiscPolySegment3.GetEndPoint()));
    ASSERT_TRUE(MiscPolySegment3.IsPointOnSCS(MiscMidPoint3));

    // Due to precision problems, the following
    // Tests with a very large PolySegment
    ASSERT_FALSE(LargePolySegment1.Crosses(LargePolySegment2));
    ASSERT_TRUE(LargePolySegment1.AreAdjacent(LargePolySegment2));

    ASSERT_FALSE(LargePolySegment1.Crosses(MiscPolySegment1));
    ASSERT_FALSE(LargePolySegment1.AreAdjacent(MiscPolySegment1));

    ASSERT_FALSE(LargePolySegment1.Crosses(PositivePolySegment1));
    ASSERT_FALSE(LargePolySegment1.AreAdjacent(PositivePolySegment1));

    ASSERT_FALSE(LargePolySegment1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_TRUE(LargePolySegment1.IsPointOn(LargePolySegment1.GetStartPoint()));
    ASSERT_TRUE(LargePolySegment1.IsPointOn(LargePolySegment1.GetEndPoint()));
    ASSERT_TRUE(LargePolySegment1.IsPointOn(LargeMidPoint1));

    ASSERT_FALSE(LargePolySegment1.IsPointOnSCS(HGF2DLocation(10, 10, pWorld)));
    ASSERT_TRUE(LargePolySegment1.IsPointOnSCS(LargePolySegment1.GetStartPoint()));
    ASSERT_TRUE(LargePolySegment1.IsPointOnSCS(LargePolySegment1.GetEndPoint()));
    ASSERT_TRUE(LargePolySegment1.IsPointOnSCS(LargeMidPoint1));

    // Tests with a way into positive region PolySegment
    ASSERT_FALSE(PositivePolySegment1.Crosses(PositivePolySegment2));
    ASSERT_TRUE(PositivePolySegment1.AreAdjacent(PositivePolySegment2));

    ASSERT_FALSE(PositivePolySegment1.Crosses(MiscPolySegment1));
    ASSERT_FALSE(PositivePolySegment1.AreAdjacent(MiscPolySegment1));

    ASSERT_FALSE(PositivePolySegment1.Crosses(MiscPolySegment3));
    ASSERT_FALSE(PositivePolySegment1.AreAdjacent(MiscPolySegment3));

    ASSERT_FALSE(PositivePolySegment1.Crosses(LargePolySegment1));
    ASSERT_FALSE(PositivePolySegment1.AreAdjacent(LargePolySegment1));

    ASSERT_FALSE(PositivePolySegment1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_TRUE(PositivePolySegment1.IsPointOn(PositivePolySegment1.GetStartPoint()));
    ASSERT_TRUE(PositivePolySegment1.IsPointOn(PositivePolySegment1.GetEndPoint()));
    ASSERT_TRUE(PositivePolySegment1.IsPointOn(PositiveMidPoint1));

    ASSERT_FALSE(PositivePolySegment1.IsPointOnSCS(HGF2DLocation(10, 10, pWorld)));
    ASSERT_TRUE(PositivePolySegment1.IsPointOnSCS(PositivePolySegment1.GetStartPoint()));
    ASSERT_TRUE(PositivePolySegment1.IsPointOnSCS(PositivePolySegment1.GetEndPoint()));
    ASSERT_TRUE(PositivePolySegment1.IsPointOnSCS(PositiveMidPoint1));

    // Tests with a way into negative region PolySegment
    ASSERT_FALSE(NegativePolySegment1.Crosses(NegativePolySegment2));
    ASSERT_TRUE(NegativePolySegment1.AreAdjacent(NegativePolySegment2));

    ASSERT_FALSE(NegativePolySegment1.Crosses(MiscPolySegment1));
    ASSERT_FALSE(NegativePolySegment1.AreAdjacent(MiscPolySegment1));

    ASSERT_FALSE(NegativePolySegment1.Crosses(MiscPolySegment3));
    ASSERT_FALSE(NegativePolySegment1.AreAdjacent(MiscPolySegment3));

    ASSERT_FALSE(NegativePolySegment1.Crosses(LargePolySegment1));
    ASSERT_FALSE(NegativePolySegment1.AreAdjacent(LargePolySegment1));

    ASSERT_FALSE(NegativePolySegment1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_TRUE(NegativePolySegment1.IsPointOn(NegativePolySegment1.GetStartPoint()));
    ASSERT_TRUE(NegativePolySegment1.IsPointOn(NegativePolySegment1.GetEndPoint()));
    ASSERT_TRUE(NegativePolySegment1.IsPointOn(NegativeMidPoint1));

    ASSERT_FALSE(NegativePolySegment1.IsPointOnSCS(HGF2DLocation(10, 10, pWorld)));
    ASSERT_TRUE(NegativePolySegment1.IsPointOnSCS(NegativePolySegment1.GetStartPoint()));
    ASSERT_TRUE(NegativePolySegment1.IsPointOnSCS(NegativePolySegment1.GetEndPoint()));
    ASSERT_TRUE(NegativePolySegment1.IsPointOnSCS(NegativeMidPoint1));

    // Tests with a NULL PolySegment
    ASSERT_FALSE(NullPolySegment1.Crosses(MiscPolySegment1));
    ASSERT_FALSE(NullPolySegment1.AreAdjacent(MiscPolySegment1));
    ASSERT_FALSE(NullPolySegment1.IsPointOn(HGF2DLocation(10, 10, pWorld)));
    ASSERT_FALSE(NullPolySegment1.IsPointOnSCS(HGF2DLocation(10, 10, pWorld)));

    }

//==================================================================================
// Bearing calculation tests
// CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
//                  HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
// CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
//                              HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, BearingTest)
    {

    // Obtain bearing ALPHA of a vertical PolySegment
    ASSERT_DOUBLE_EQ(4.7123889803846897, VerticalPolySegment1.CalculateBearing(VerticalMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a vertical PolySegment
    ASSERT_DOUBLE_EQ(1.5707963267948966, VerticalPolySegment1.CalculateBearing(VerticalMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted vertical PolySegment
    ASSERT_DOUBLE_EQ(1.5707963267948966, VerticalPolySegment2.CalculateBearing(VerticalMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted vertical PolySegment
    ASSERT_DOUBLE_EQ(4.7123889803846897, VerticalPolySegment2.CalculateBearing(VerticalMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of an horizontal PolySegment
    ASSERT_DOUBLE_EQ(3.1415926535897931, HorizontalPolySegment1.CalculateBearing(HorizontalMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of an horizontal PolySegment
    ASSERT_NEAR(0.0, HorizontalPolySegment1.CalculateBearing(HorizontalMidPoint1, HVE2DVector::BETA).GetAngle(), MYEPSILON);

    // Obtain bearing ALPHA of an inverted horizontal PolySegment
    ASSERT_NEAR(0.0, HorizontalPolySegment2.CalculateBearing(HorizontalMidPoint1, HVE2DVector::ALPHA).GetAngle(), MYEPSILON);

    // Obtain bearing BETA of an inverted horizontal PolySegment
    ASSERT_DOUBLE_EQ(3.14159265358979310, HorizontalPolySegment2.CalculateBearing(HorizontalMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a positive slope PolySegment
    ASSERT_DOUBLE_EQ(-2.3561944901923448, MiscPolySegment1.CalculateBearing(MiscMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a positive slope PolySegment
    ASSERT_DOUBLE_EQ(0.78539816339744828, MiscPolySegment1.CalculateBearing(MiscMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted positive slope PolySegment
    ASSERT_DOUBLE_EQ(0.78539816339744828, MiscPolySegment2.CalculateBearing(MiscMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted positive slope PolySegment
    ASSERT_DOUBLE_EQ(-2.3561944901923448, MiscPolySegment2.CalculateBearing(MiscMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a vertical EPSILON SIZED PolySegment
    ASSERT_DOUBLE_EQ(4.7123889803846897, VerticalPolySegment3.CalculateBearing(VerticalMidPoint3, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a vertical EPSILON SIZED PolySegment
    ASSERT_DOUBLE_EQ(1.5707963267948966, VerticalPolySegment3.CalculateBearing(VerticalMidPoint3, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a horizontal EPSILON SIZED PolySegment
    ASSERT_DOUBLE_EQ(3.1415926535897931, HorizontalPolySegment3.CalculateBearing(HorizontalMidPoint3, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a horizontal EPSILON SIZED PolySegment
    ASSERT_NEAR(0.0, HorizontalPolySegment3.CalculateBearing(HorizontalMidPoint3, HVE2DVector::BETA).GetAngle(), MYEPSILON);

    // Obtain bearing ALPHA of a miscaleniuous EPSILON SIZED PolySegment
    ASSERT_DOUBLE_EQ(-1.7889624832338027, MiscPolySegment3.CalculateBearing(MiscMidPoint3, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a miscaleniuous EPSILON SIZED PolySegment
    ASSERT_DOUBLE_EQ(1.3526301703559906, MiscPolySegment3.CalculateBearing(MiscMidPoint3, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a very large PolySegment
    ASSERT_DOUBLE_EQ(-1.8157749899217608, LargePolySegment1.CalculateBearing(LargeMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a very large PolySegment
    ASSERT_DOUBLE_EQ(1.3258176636680326, LargePolySegment1.CalculateBearing(LargeMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted very large PolySegment
    ASSERT_DOUBLE_EQ(1.3258176636680326, LargePolySegment2.CalculateBearing(LargeMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted very large PolySegment
    ASSERT_DOUBLE_EQ(-1.8157749899217608, LargePolySegment2.CalculateBearing(LargeMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a PolySegment way into negative coordinates
    ASSERT_DOUBLE_EQ(1.1071487177940904, NegativePolySegment1.CalculateBearing(NegativeMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a PolySegment way into negative coordinates
    ASSERT_DOUBLE_EQ(-2.0344439357957027, NegativePolySegment1.CalculateBearing(NegativeMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted PolySegment way into negative coordinates
    ASSERT_DOUBLE_EQ(-2.0344439357957027, NegativePolySegment2.CalculateBearing(NegativeMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted PolySegment way into negative coordinates
    ASSERT_DOUBLE_EQ(1.1071487177940904, NegativePolySegment2.CalculateBearing(NegativeMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a PolySegment way into positive values
    ASSERT_DOUBLE_EQ(-2.0344439357957027, PositivePolySegment1.CalculateBearing(PositiveMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a PolySegment way into positive values
    ASSERT_DOUBLE_EQ(1.1071487177940904, PositivePolySegment1.CalculateBearing(PositiveMidPoint1, HVE2DVector::BETA).GetAngle());

    // Obtain bearing ALPHA of a inverted PolySegment way into positive values
    ASSERT_DOUBLE_EQ(1.1071487177940904, PositivePolySegment2.CalculateBearing(PositiveMidPoint1, HVE2DVector::ALPHA).GetAngle());

    // Obtain bearing BETA of a inverted PolySegment way into positive values
    ASSERT_DOUBLE_EQ(-2.0344439357957027, PositivePolySegment2.CalculateBearing(PositiveMidPoint1, HVE2DVector::BETA).GetAngle());

    // ANGULAR ACCELERATION
    // Obtain angular acceleration ALPHA of a vertical PolySegment
    ASSERT_NEAR(0.0, VerticalPolySegment1.CalculateAngularAcceleration(VerticalMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a vertical PolySegment
    ASSERT_NEAR(0.0, VerticalPolySegment1.CalculateAngularAcceleration(VerticalMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted vertical PolySegment
    ASSERT_NEAR(0.0, VerticalPolySegment2.CalculateAngularAcceleration(VerticalMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted vertical PolySegment
    ASSERT_NEAR(0.0, VerticalPolySegment2.CalculateAngularAcceleration(VerticalMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of an horizontal PolySegment
    ASSERT_NEAR(0.0, HorizontalPolySegment1.CalculateAngularAcceleration(HorizontalMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of an horizontal PolySegment
    ASSERT_NEAR(0.0, HorizontalPolySegment1.CalculateAngularAcceleration(HorizontalMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of an inverted horizontal PolySegment
    ASSERT_NEAR(0.0, HorizontalPolySegment2.CalculateAngularAcceleration(HorizontalMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of an inverted horizontal PolySegment
    ASSERT_NEAR(0.0, HorizontalPolySegment2.CalculateAngularAcceleration(HorizontalMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a positive slope PolySegment
    ASSERT_NEAR(0.0, MiscPolySegment1.CalculateAngularAcceleration(MiscMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a positive slope PolySegment
    ASSERT_NEAR(0.0, MiscPolySegment1.CalculateAngularAcceleration(MiscMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted positive slope PolySegment
    ASSERT_NEAR(0.0, MiscPolySegment2.CalculateAngularAcceleration(MiscMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted positive slope PolySegment
    ASSERT_NEAR(0.0, MiscPolySegment2.CalculateAngularAcceleration(MiscMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a vertical EPSILON SIZED PolySegment
    ASSERT_NEAR(0.0, VerticalPolySegment3.CalculateAngularAcceleration(VerticalMidPoint3, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a vertical EPSILON SIZED PolySegment
    ASSERT_NEAR(0.0, VerticalPolySegment3.CalculateAngularAcceleration(VerticalMidPoint3, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a horizontal EPSILON SIZED PolySegment
    ASSERT_NEAR(0.0, HorizontalPolySegment3.CalculateAngularAcceleration(HorizontalMidPoint3, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a horizontal EPSILON SIZED PolySegment
    ASSERT_NEAR(0.0, HorizontalPolySegment3.CalculateAngularAcceleration(HorizontalMidPoint3, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a miscaleniuous EPSILON SIZED PolySegment
    ASSERT_NEAR(0.0, MiscPolySegment3.CalculateAngularAcceleration(MiscMidPoint3, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a miscaleniuous EPSILON SIZED PolySegment
    ASSERT_NEAR(0.0, MiscPolySegment3.CalculateAngularAcceleration(MiscMidPoint3, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a very large PolySegment
    ASSERT_NEAR(0.0, LargePolySegment1.CalculateAngularAcceleration(LargeMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a very large PolySegment
    ASSERT_NEAR(0.0, LargePolySegment1.CalculateAngularAcceleration(LargeMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted very large PolySegment
    ASSERT_NEAR(0.0, LargePolySegment2.CalculateAngularAcceleration(LargeMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted very large PolySegment
    ASSERT_NEAR(0.0, LargePolySegment2.CalculateAngularAcceleration(LargeMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a PolySegment way into negative coordinates
    ASSERT_NEAR(0.0, NegativePolySegment1.CalculateAngularAcceleration(NegativeMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a PolySegment way into negative coordinates
    ASSERT_NEAR(0.0, NegativePolySegment1.CalculateAngularAcceleration(NegativeMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted PolySegment way into negative coordinates
    ASSERT_NEAR(0.0, NegativePolySegment2.CalculateAngularAcceleration(NegativeMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted PolySegment way into negative coordinates
    ASSERT_NEAR(0.0, NegativePolySegment2.CalculateAngularAcceleration(NegativeMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a PolySegment way into positive values
    ASSERT_NEAR(0.0, PositivePolySegment1.CalculateAngularAcceleration(PositiveMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a PolySegment way into positive values
    ASSERT_NEAR(0.0, PositivePolySegment1.CalculateAngularAcceleration(PositiveMidPoint1, HVE2DVector::BETA), MYEPSILON);

    // Obtain angular acceleration ALPHA of a inverted PolySegment way into positive values
    ASSERT_NEAR(0.0, PositivePolySegment2.CalculateAngularAcceleration(PositiveMidPoint1, HVE2DVector::ALPHA), MYEPSILON);

    // Obtain angular acceleration BETA of a inverted PolySegment way into positive values
    ASSERT_NEAR(0.0, PositivePolySegment2.CalculateAngularAcceleration(PositiveMidPoint1, HVE2DVector::BETA), MYEPSILON);

    }

//==================================================================================
// Extent calculation test
// GetExtent() const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, GetExtentTest)
    {

    // Obtain extent of a vertical PolySegment
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.1, VerticalPolySegment1.GetExtent().GetYMax());

    // Obtain extent of a inverted vertical PolySegment
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment2.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment2.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.10, VerticalPolySegment2.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.1, VerticalPolySegment2.GetExtent().GetYMax());

    // Obtain extent of an horizontal PolySegment
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(10.1, HorizontalPolySegment1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment1.GetExtent().GetYMax());

    // Obtain extent of an inverted horizontal PolySegment
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment2.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment2.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(10.1, HorizontalPolySegment2.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.10, HorizontalPolySegment2.GetExtent().GetYMax());

    // Obtain extent of a positive slope PolySegment
    ASSERT_DOUBLE_EQ(0.10, MiscPolySegment1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10, MiscPolySegment1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(10.1, MiscPolySegment1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.1, MiscPolySegment1.GetExtent().GetYMax());

    // Obtain extent of a vertical EPSILON SIZED PolySegment
    ASSERT_DOUBLE_EQ(0.1000000, VerticalPolySegment3.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.1000000, VerticalPolySegment3.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.1000000, VerticalPolySegment3.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.1000001, VerticalPolySegment3.GetExtent().GetYMax());

    // Obtain extent of a horizontal EPSILON SIZED PolySegment
    ASSERT_DOUBLE_EQ(0.1000000, HorizontalPolySegment3.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.1000000, HorizontalPolySegment3.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.1000001, HorizontalPolySegment3.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.1000000, HorizontalPolySegment3.GetExtent().GetYMax());

    // Obtain extent of a miscaleniuous EPSILON SIZED PolySegment
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscPolySegment3.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(0.10000000000000000, MiscPolySegment3.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(0.10000002164396139, MiscPolySegment3.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(0.10000009762960071, MiscPolySegment3.GetExtent().GetYMax());

    // Obtain extent of a very large PolySegment
    ASSERT_DOUBLE_EQ(-1.00E123, LargePolySegment1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(-21.0E123, LargePolySegment1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(9.000E123, LargePolySegment1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(19.00E123, LargePolySegment1.GetExtent().GetYMax());

    // Obtain extent of a PolySegment way into negative coordinates
    ASSERT_DOUBLE_EQ(-11.0E123, NegativePolySegment1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(-41.0E123, NegativePolySegment1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(-1.00E123, NegativePolySegment1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(-21.0E123, NegativePolySegment1.GetExtent().GetYMax());

    // Obtain extent of a PolySegment way into positive values
    ASSERT_DOUBLE_EQ(10E122, PositivePolySegment1.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(21E123, PositivePolySegment1.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(11E123, PositivePolySegment1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(41E123, PositivePolySegment1.GetExtent().GetYMax());

    }

//==================================================================================
// The following test failed in complex linear testing and is not part of
// the previous tests
//==================================================================================


//==================================================================================
// Additional tests for contiguousness
//==================================================================================
TEST_F (HVE2DPolySegmentTester, AreContiguousTest)
    {

    HVE2DPolySegment    HorizontalPolySegmentSup1(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld));
    HVE2DPolySegment    HorizontalPolySegmentSup2(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(35.0, 10.0, pWorld));

    ASSERT_FALSE(HorizontalPolySegmentSup1.AreContiguous(HorizontalPolySegmentSup2));
    ASSERT_FALSE(HorizontalPolySegmentSup2.AreContiguous(HorizontalPolySegmentSup1));

    HVE2DPolySegment    VerticalPolySegmentSup1(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(10.0, 20.0, pWorld));
    HVE2DPolySegment    VerticalPolySegmentSup2(HGF2DLocation(10.0, 20.0, pWorld), HGF2DLocation(10.0, 35.0, pWorld));

    ASSERT_FALSE(VerticalPolySegmentSup1.AreContiguous(VerticalPolySegmentSup2));
    ASSERT_FALSE(VerticalPolySegmentSup2.AreContiguous(VerticalPolySegmentSup1));

    }
//==================================================================================
// Other test which failed
// all of the following are contiguous
//==================================================================================
TEST_F (HVE2DPolySegmentTester, AreContiguousTest2)
    {
        
    HVE2DPolySegment    HorizontalPolySegmentSup3(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld));
    HVE2DPolySegment    HorizontalPolySegmentSup4(HGF2DLocation(5.0, 10.0, pWorld), HGF2DLocation(35.0, 10.0, pWorld));
    HVE2DPolySegment    HorizontalPolySegmentSup5(HGF2DLocation(5.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld));
    HVE2DPolySegment    HorizontalPolySegmentSup6(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(25.0, 10.0, pWorld));
    HVE2DPolySegment    HorizontalPolySegmentSup7(HGF2DLocation(5.0, 10.0, pWorld), HGF2DLocation(15.0, 10.0, pWorld));
    HVE2DPolySegment    HorizontalPolySegmentSup8(HGF2DLocation(13.0, 10.0, pWorld), HGF2DLocation(25.0, 10.0, pWorld));
    HVE2DPolySegment    HorizontalPolySegmentSup9(HGF2DLocation(12.0, 10.0, pWorld), HGF2DLocation(18.0, 10.0, pWorld));

    HVE2DPolySegment    HorizontalPolySegmentSup10(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HVE2DPolySegment    HorizontalPolySegmentSup11(HGF2DLocation(35.0, 10.0, pWorld), HGF2DLocation(5.0, 10.0, pWorld));
    HVE2DPolySegment    HorizontalPolySegmentSup12(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(5.0, 10.0, pWorld));
    HVE2DPolySegment    HorizontalPolySegmentSup13(HGF2DLocation(25.0, 10.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HVE2DPolySegment    HorizontalPolySegmentSup14(HGF2DLocation(15.0, 10.0, pWorld), HGF2DLocation(5.0, 10.0, pWorld));
    HVE2DPolySegment    HorizontalPolySegmentSup15(HGF2DLocation(25.0, 10.0, pWorld), HGF2DLocation(13.0, 10.0, pWorld));
    HVE2DPolySegment    HorizontalPolySegmentSup16(HGF2DLocation(18.0, 10.0, pWorld), HGF2DLocation(12.0, 10.0, pWorld));

    ASSERT_TRUE(HorizontalPolySegmentSup3.AreContiguous(HorizontalPolySegmentSup4));
    ASSERT_TRUE(HorizontalPolySegmentSup4.AreContiguous(HorizontalPolySegmentSup3));
    ASSERT_TRUE(HorizontalPolySegmentSup3.AreContiguous(HorizontalPolySegmentSup5));
    ASSERT_TRUE(HorizontalPolySegmentSup5.AreContiguous(HorizontalPolySegmentSup3));
    ASSERT_TRUE(HorizontalPolySegmentSup3.AreContiguous(HorizontalPolySegmentSup6));
    ASSERT_TRUE(HorizontalPolySegmentSup6.AreContiguous(HorizontalPolySegmentSup3));
    ASSERT_TRUE(HorizontalPolySegmentSup3.AreContiguous(HorizontalPolySegmentSup7));
    ASSERT_TRUE(HorizontalPolySegmentSup7.AreContiguous(HorizontalPolySegmentSup3));
    ASSERT_TRUE(HorizontalPolySegmentSup3.AreContiguous(HorizontalPolySegmentSup8));
    ASSERT_TRUE(HorizontalPolySegmentSup8.AreContiguous(HorizontalPolySegmentSup3));
    ASSERT_TRUE(HorizontalPolySegmentSup3.AreContiguous(HorizontalPolySegmentSup9));
    ASSERT_TRUE(HorizontalPolySegmentSup9.AreContiguous(HorizontalPolySegmentSup3));
    ASSERT_TRUE(HorizontalPolySegmentSup3.AreContiguous(HorizontalPolySegmentSup10));
    ASSERT_TRUE(HorizontalPolySegmentSup10.AreContiguous(HorizontalPolySegmentSup3));
    ASSERT_TRUE(HorizontalPolySegmentSup3.AreContiguous(HorizontalPolySegmentSup11));
    ASSERT_TRUE(HorizontalPolySegmentSup11.AreContiguous(HorizontalPolySegmentSup3));
    ASSERT_TRUE(HorizontalPolySegmentSup3.AreContiguous(HorizontalPolySegmentSup12));
    ASSERT_TRUE(HorizontalPolySegmentSup12.AreContiguous(HorizontalPolySegmentSup3));
    ASSERT_TRUE(HorizontalPolySegmentSup3.AreContiguous(HorizontalPolySegmentSup13));
    ASSERT_TRUE(HorizontalPolySegmentSup13.AreContiguous(HorizontalPolySegmentSup3));
    ASSERT_TRUE(HorizontalPolySegmentSup3.AreContiguous(HorizontalPolySegmentSup14));
    ASSERT_TRUE(HorizontalPolySegmentSup14.AreContiguous(HorizontalPolySegmentSup3));
    ASSERT_TRUE(HorizontalPolySegmentSup3.AreContiguous(HorizontalPolySegmentSup15));
    ASSERT_TRUE(HorizontalPolySegmentSup15.AreContiguous(HorizontalPolySegmentSup3));
    ASSERT_TRUE(HorizontalPolySegmentSup3.AreContiguous(HorizontalPolySegmentSup16));
    ASSERT_TRUE(HorizontalPolySegmentSup16.AreContiguous(HorizontalPolySegmentSup3));

    ASSERT_TRUE(HorizontalPolySegmentSup4.AreContiguous(HorizontalPolySegmentSup5));
    ASSERT_TRUE(HorizontalPolySegmentSup5.AreContiguous(HorizontalPolySegmentSup4));
    ASSERT_TRUE(HorizontalPolySegmentSup4.AreContiguous(HorizontalPolySegmentSup6));
    ASSERT_TRUE(HorizontalPolySegmentSup6.AreContiguous(HorizontalPolySegmentSup4));
    ASSERT_TRUE(HorizontalPolySegmentSup4.AreContiguous(HorizontalPolySegmentSup7));
    ASSERT_TRUE(HorizontalPolySegmentSup7.AreContiguous(HorizontalPolySegmentSup4));
    ASSERT_TRUE(HorizontalPolySegmentSup4.AreContiguous(HorizontalPolySegmentSup8));
    ASSERT_TRUE(HorizontalPolySegmentSup8.AreContiguous(HorizontalPolySegmentSup4));
    ASSERT_TRUE(HorizontalPolySegmentSup4.AreContiguous(HorizontalPolySegmentSup9));
    ASSERT_TRUE(HorizontalPolySegmentSup9.AreContiguous(HorizontalPolySegmentSup4));
    ASSERT_TRUE(HorizontalPolySegmentSup4.AreContiguous(HorizontalPolySegmentSup10));
    ASSERT_TRUE(HorizontalPolySegmentSup10.AreContiguous(HorizontalPolySegmentSup4));
    ASSERT_TRUE(HorizontalPolySegmentSup4.AreContiguous(HorizontalPolySegmentSup11));
    ASSERT_TRUE(HorizontalPolySegmentSup11.AreContiguous(HorizontalPolySegmentSup4));
    ASSERT_TRUE(HorizontalPolySegmentSup4.AreContiguous(HorizontalPolySegmentSup12));
    ASSERT_TRUE(HorizontalPolySegmentSup12.AreContiguous(HorizontalPolySegmentSup4));
    ASSERT_TRUE(HorizontalPolySegmentSup4.AreContiguous(HorizontalPolySegmentSup13));
    ASSERT_TRUE(HorizontalPolySegmentSup13.AreContiguous(HorizontalPolySegmentSup4));
    ASSERT_TRUE(HorizontalPolySegmentSup4.AreContiguous(HorizontalPolySegmentSup14));
    ASSERT_TRUE(HorizontalPolySegmentSup14.AreContiguous(HorizontalPolySegmentSup4));
    ASSERT_TRUE(HorizontalPolySegmentSup4.AreContiguous(HorizontalPolySegmentSup15));
    ASSERT_TRUE(HorizontalPolySegmentSup15.AreContiguous(HorizontalPolySegmentSup4));
    ASSERT_TRUE(HorizontalPolySegmentSup4.AreContiguous(HorizontalPolySegmentSup16));
    ASSERT_TRUE(HorizontalPolySegmentSup16.AreContiguous(HorizontalPolySegmentSup4));

    ASSERT_TRUE(HorizontalPolySegmentSup5.AreContiguous(HorizontalPolySegmentSup6));
    ASSERT_TRUE(HorizontalPolySegmentSup6.AreContiguous(HorizontalPolySegmentSup5));
    ASSERT_TRUE(HorizontalPolySegmentSup5.AreContiguous(HorizontalPolySegmentSup7));
    ASSERT_TRUE(HorizontalPolySegmentSup7.AreContiguous(HorizontalPolySegmentSup5));
    ASSERT_TRUE(HorizontalPolySegmentSup5.AreContiguous(HorizontalPolySegmentSup8));
    ASSERT_TRUE(HorizontalPolySegmentSup8.AreContiguous(HorizontalPolySegmentSup5));
    ASSERT_TRUE(HorizontalPolySegmentSup5.AreContiguous(HorizontalPolySegmentSup9));
    ASSERT_TRUE(HorizontalPolySegmentSup9.AreContiguous(HorizontalPolySegmentSup5));
    ASSERT_TRUE(HorizontalPolySegmentSup5.AreContiguous(HorizontalPolySegmentSup10));
    ASSERT_TRUE(HorizontalPolySegmentSup10.AreContiguous(HorizontalPolySegmentSup5));
    ASSERT_TRUE(HorizontalPolySegmentSup5.AreContiguous(HorizontalPolySegmentSup11));
    ASSERT_TRUE(HorizontalPolySegmentSup11.AreContiguous(HorizontalPolySegmentSup5));
    ASSERT_TRUE(HorizontalPolySegmentSup5.AreContiguous(HorizontalPolySegmentSup12));
    ASSERT_TRUE(HorizontalPolySegmentSup12.AreContiguous(HorizontalPolySegmentSup5));
    ASSERT_TRUE(HorizontalPolySegmentSup5.AreContiguous(HorizontalPolySegmentSup13));
    ASSERT_TRUE(HorizontalPolySegmentSup13.AreContiguous(HorizontalPolySegmentSup5));
    ASSERT_TRUE(HorizontalPolySegmentSup5.AreContiguous(HorizontalPolySegmentSup14));
    ASSERT_TRUE(HorizontalPolySegmentSup14.AreContiguous(HorizontalPolySegmentSup5));
    ASSERT_TRUE(HorizontalPolySegmentSup5.AreContiguous(HorizontalPolySegmentSup15));
    ASSERT_TRUE(HorizontalPolySegmentSup15.AreContiguous(HorizontalPolySegmentSup5));
    ASSERT_TRUE(HorizontalPolySegmentSup5.AreContiguous(HorizontalPolySegmentSup16));
    ASSERT_TRUE(HorizontalPolySegmentSup16.AreContiguous(HorizontalPolySegmentSup5));

    ASSERT_TRUE(HorizontalPolySegmentSup6.AreContiguous(HorizontalPolySegmentSup7));
    ASSERT_TRUE(HorizontalPolySegmentSup7.AreContiguous(HorizontalPolySegmentSup6));
    ASSERT_TRUE(HorizontalPolySegmentSup6.AreContiguous(HorizontalPolySegmentSup8));
    ASSERT_TRUE(HorizontalPolySegmentSup8.AreContiguous(HorizontalPolySegmentSup6));
    ASSERT_TRUE(HorizontalPolySegmentSup6.AreContiguous(HorizontalPolySegmentSup9));
    ASSERT_TRUE(HorizontalPolySegmentSup9.AreContiguous(HorizontalPolySegmentSup6));
    ASSERT_TRUE(HorizontalPolySegmentSup6.AreContiguous(HorizontalPolySegmentSup10));
    ASSERT_TRUE(HorizontalPolySegmentSup10.AreContiguous(HorizontalPolySegmentSup6));
    ASSERT_TRUE(HorizontalPolySegmentSup6.AreContiguous(HorizontalPolySegmentSup11));
    ASSERT_TRUE(HorizontalPolySegmentSup11.AreContiguous(HorizontalPolySegmentSup6));
    ASSERT_TRUE(HorizontalPolySegmentSup6.AreContiguous(HorizontalPolySegmentSup12));
    ASSERT_TRUE(HorizontalPolySegmentSup12.AreContiguous(HorizontalPolySegmentSup6));
    ASSERT_TRUE(HorizontalPolySegmentSup6.AreContiguous(HorizontalPolySegmentSup13));
    ASSERT_TRUE(HorizontalPolySegmentSup13.AreContiguous(HorizontalPolySegmentSup6));
    ASSERT_TRUE(HorizontalPolySegmentSup6.AreContiguous(HorizontalPolySegmentSup14));
    ASSERT_TRUE(HorizontalPolySegmentSup14.AreContiguous(HorizontalPolySegmentSup6));
    ASSERT_TRUE(HorizontalPolySegmentSup6.AreContiguous(HorizontalPolySegmentSup15));
    ASSERT_TRUE(HorizontalPolySegmentSup15.AreContiguous(HorizontalPolySegmentSup6));
    ASSERT_TRUE(HorizontalPolySegmentSup6.AreContiguous(HorizontalPolySegmentSup16));
    ASSERT_TRUE(HorizontalPolySegmentSup16.AreContiguous(HorizontalPolySegmentSup6));

    ASSERT_TRUE(HorizontalPolySegmentSup7.AreContiguous(HorizontalPolySegmentSup8));
    ASSERT_TRUE(HorizontalPolySegmentSup8.AreContiguous(HorizontalPolySegmentSup7));
    ASSERT_TRUE(HorizontalPolySegmentSup7.AreContiguous(HorizontalPolySegmentSup9));
    ASSERT_TRUE(HorizontalPolySegmentSup9.AreContiguous(HorizontalPolySegmentSup7));
    ASSERT_TRUE(HorizontalPolySegmentSup7.AreContiguous(HorizontalPolySegmentSup10));
    ASSERT_TRUE(HorizontalPolySegmentSup10.AreContiguous(HorizontalPolySegmentSup7));
    ASSERT_TRUE(HorizontalPolySegmentSup7.AreContiguous(HorizontalPolySegmentSup11));
    ASSERT_TRUE(HorizontalPolySegmentSup11.AreContiguous(HorizontalPolySegmentSup7));
    ASSERT_TRUE(HorizontalPolySegmentSup7.AreContiguous(HorizontalPolySegmentSup12));
    ASSERT_TRUE(HorizontalPolySegmentSup12.AreContiguous(HorizontalPolySegmentSup7));
    ASSERT_TRUE(HorizontalPolySegmentSup7.AreContiguous(HorizontalPolySegmentSup13));
    ASSERT_TRUE(HorizontalPolySegmentSup13.AreContiguous(HorizontalPolySegmentSup7));
    ASSERT_TRUE(HorizontalPolySegmentSup7.AreContiguous(HorizontalPolySegmentSup14));
    ASSERT_TRUE(HorizontalPolySegmentSup14.AreContiguous(HorizontalPolySegmentSup7));
    ASSERT_TRUE(HorizontalPolySegmentSup7.AreContiguous(HorizontalPolySegmentSup15));
    ASSERT_TRUE(HorizontalPolySegmentSup15.AreContiguous(HorizontalPolySegmentSup7));
    ASSERT_TRUE(HorizontalPolySegmentSup7.AreContiguous(HorizontalPolySegmentSup16));
    ASSERT_TRUE(HorizontalPolySegmentSup16.AreContiguous(HorizontalPolySegmentSup7));

    ASSERT_TRUE(HorizontalPolySegmentSup8.AreContiguous(HorizontalPolySegmentSup9));
    ASSERT_TRUE(HorizontalPolySegmentSup9.AreContiguous(HorizontalPolySegmentSup8));
    ASSERT_TRUE(HorizontalPolySegmentSup8.AreContiguous(HorizontalPolySegmentSup10));
    ASSERT_TRUE(HorizontalPolySegmentSup10.AreContiguous(HorizontalPolySegmentSup8));
    ASSERT_TRUE(HorizontalPolySegmentSup8.AreContiguous(HorizontalPolySegmentSup11));
    ASSERT_TRUE(HorizontalPolySegmentSup11.AreContiguous(HorizontalPolySegmentSup8));
    ASSERT_TRUE(HorizontalPolySegmentSup8.AreContiguous(HorizontalPolySegmentSup12));
    ASSERT_TRUE(HorizontalPolySegmentSup12.AreContiguous(HorizontalPolySegmentSup8));
    ASSERT_TRUE(HorizontalPolySegmentSup8.AreContiguous(HorizontalPolySegmentSup13));
    ASSERT_TRUE(HorizontalPolySegmentSup13.AreContiguous(HorizontalPolySegmentSup8));
    ASSERT_TRUE(HorizontalPolySegmentSup8.AreContiguous(HorizontalPolySegmentSup14));
    ASSERT_TRUE(HorizontalPolySegmentSup14.AreContiguous(HorizontalPolySegmentSup8));
    ASSERT_TRUE(HorizontalPolySegmentSup8.AreContiguous(HorizontalPolySegmentSup15));
    ASSERT_TRUE(HorizontalPolySegmentSup15.AreContiguous(HorizontalPolySegmentSup8));
    ASSERT_TRUE(HorizontalPolySegmentSup8.AreContiguous(HorizontalPolySegmentSup16));
    ASSERT_TRUE(HorizontalPolySegmentSup16.AreContiguous(HorizontalPolySegmentSup8));

    ASSERT_TRUE(HorizontalPolySegmentSup9.AreContiguous(HorizontalPolySegmentSup10));
    ASSERT_TRUE(HorizontalPolySegmentSup10.AreContiguous(HorizontalPolySegmentSup9));
    ASSERT_TRUE(HorizontalPolySegmentSup9.AreContiguous(HorizontalPolySegmentSup11));
    ASSERT_TRUE(HorizontalPolySegmentSup11.AreContiguous(HorizontalPolySegmentSup9));
    ASSERT_TRUE(HorizontalPolySegmentSup9.AreContiguous(HorizontalPolySegmentSup12));
    ASSERT_TRUE(HorizontalPolySegmentSup12.AreContiguous(HorizontalPolySegmentSup9));
    ASSERT_TRUE(HorizontalPolySegmentSup9.AreContiguous(HorizontalPolySegmentSup13));
    ASSERT_TRUE(HorizontalPolySegmentSup13.AreContiguous(HorizontalPolySegmentSup9));
    ASSERT_TRUE(HorizontalPolySegmentSup9.AreContiguous(HorizontalPolySegmentSup14));
    ASSERT_TRUE(HorizontalPolySegmentSup14.AreContiguous(HorizontalPolySegmentSup9));
    ASSERT_TRUE(HorizontalPolySegmentSup9.AreContiguous(HorizontalPolySegmentSup15));
    ASSERT_TRUE(HorizontalPolySegmentSup15.AreContiguous(HorizontalPolySegmentSup9));
    ASSERT_TRUE(HorizontalPolySegmentSup9.AreContiguous(HorizontalPolySegmentSup16));
    ASSERT_TRUE(HorizontalPolySegmentSup16.AreContiguous(HorizontalPolySegmentSup9));

    ASSERT_TRUE(HorizontalPolySegmentSup10.AreContiguous(HorizontalPolySegmentSup11));
    ASSERT_TRUE(HorizontalPolySegmentSup11.AreContiguous(HorizontalPolySegmentSup10));
    ASSERT_TRUE(HorizontalPolySegmentSup10.AreContiguous(HorizontalPolySegmentSup12));
    ASSERT_TRUE(HorizontalPolySegmentSup12.AreContiguous(HorizontalPolySegmentSup10));
    ASSERT_TRUE(HorizontalPolySegmentSup10.AreContiguous(HorizontalPolySegmentSup13));
    ASSERT_TRUE(HorizontalPolySegmentSup13.AreContiguous(HorizontalPolySegmentSup10));
    ASSERT_TRUE(HorizontalPolySegmentSup10.AreContiguous(HorizontalPolySegmentSup14));
    ASSERT_TRUE(HorizontalPolySegmentSup14.AreContiguous(HorizontalPolySegmentSup10));
    ASSERT_TRUE(HorizontalPolySegmentSup10.AreContiguous(HorizontalPolySegmentSup15));
    ASSERT_TRUE(HorizontalPolySegmentSup15.AreContiguous(HorizontalPolySegmentSup10));
    ASSERT_TRUE(HorizontalPolySegmentSup10.AreContiguous(HorizontalPolySegmentSup16));
    ASSERT_TRUE(HorizontalPolySegmentSup16.AreContiguous(HorizontalPolySegmentSup10));

    ASSERT_TRUE(HorizontalPolySegmentSup11.AreContiguous(HorizontalPolySegmentSup12));
    ASSERT_TRUE(HorizontalPolySegmentSup12.AreContiguous(HorizontalPolySegmentSup11));
    ASSERT_TRUE(HorizontalPolySegmentSup11.AreContiguous(HorizontalPolySegmentSup13));
    ASSERT_TRUE(HorizontalPolySegmentSup13.AreContiguous(HorizontalPolySegmentSup11));
    ASSERT_TRUE(HorizontalPolySegmentSup11.AreContiguous(HorizontalPolySegmentSup14));
    ASSERT_TRUE(HorizontalPolySegmentSup14.AreContiguous(HorizontalPolySegmentSup11));
    ASSERT_TRUE(HorizontalPolySegmentSup11.AreContiguous(HorizontalPolySegmentSup15));
    ASSERT_TRUE(HorizontalPolySegmentSup15.AreContiguous(HorizontalPolySegmentSup11));
    ASSERT_TRUE(HorizontalPolySegmentSup11.AreContiguous(HorizontalPolySegmentSup16));
    ASSERT_TRUE(HorizontalPolySegmentSup16.AreContiguous(HorizontalPolySegmentSup11));

    ASSERT_TRUE(HorizontalPolySegmentSup12.AreContiguous(HorizontalPolySegmentSup13));
    ASSERT_TRUE(HorizontalPolySegmentSup13.AreContiguous(HorizontalPolySegmentSup12));
    ASSERT_TRUE(HorizontalPolySegmentSup12.AreContiguous(HorizontalPolySegmentSup14));
    ASSERT_TRUE(HorizontalPolySegmentSup14.AreContiguous(HorizontalPolySegmentSup12));
    ASSERT_TRUE(HorizontalPolySegmentSup12.AreContiguous(HorizontalPolySegmentSup15));
    ASSERT_TRUE(HorizontalPolySegmentSup15.AreContiguous(HorizontalPolySegmentSup12));
    ASSERT_TRUE(HorizontalPolySegmentSup12.AreContiguous(HorizontalPolySegmentSup16));
    ASSERT_TRUE(HorizontalPolySegmentSup16.AreContiguous(HorizontalPolySegmentSup12));

    ASSERT_TRUE(HorizontalPolySegmentSup13.AreContiguous(HorizontalPolySegmentSup14));
    ASSERT_TRUE(HorizontalPolySegmentSup14.AreContiguous(HorizontalPolySegmentSup13));
    ASSERT_TRUE(HorizontalPolySegmentSup13.AreContiguous(HorizontalPolySegmentSup15));
    ASSERT_TRUE(HorizontalPolySegmentSup15.AreContiguous(HorizontalPolySegmentSup13));
    ASSERT_TRUE(HorizontalPolySegmentSup13.AreContiguous(HorizontalPolySegmentSup16));
    ASSERT_TRUE(HorizontalPolySegmentSup16.AreContiguous(HorizontalPolySegmentSup13));

    ASSERT_TRUE(HorizontalPolySegmentSup14.AreContiguous(HorizontalPolySegmentSup15));
    ASSERT_TRUE(HorizontalPolySegmentSup15.AreContiguous(HorizontalPolySegmentSup14));
    ASSERT_TRUE(HorizontalPolySegmentSup14.AreContiguous(HorizontalPolySegmentSup16));
    ASSERT_TRUE(HorizontalPolySegmentSup16.AreContiguous(HorizontalPolySegmentSup14));

    ASSERT_TRUE(HorizontalPolySegmentSup15.AreContiguous(HorizontalPolySegmentSup16));
    ASSERT_TRUE(HorizontalPolySegmentSup16.AreContiguous(HorizontalPolySegmentSup15));

    }

//==================================================================================
// Other test which failed
// all of the following are contiguous
//==================================================================================
TEST_F (HVE2DPolySegmentTester, AreContiguousTest3)
    {

    HVE2DPolySegment    VerticalPolySegmentSup3(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(10.0, 20.0, pWorld));
    HVE2DPolySegment    VerticalPolySegmentSup4(HGF2DLocation(10.0, 5.0, pWorld), HGF2DLocation(10.0, 35.0, pWorld));
    HVE2DPolySegment    VerticalPolySegmentSup5(HGF2DLocation(10.0, 5.0, pWorld), HGF2DLocation(10.0, 20.0, pWorld));
    HVE2DPolySegment    VerticalPolySegmentSup6(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(10.0, 25.0, pWorld));
    HVE2DPolySegment    VerticalPolySegmentSup7(HGF2DLocation(10.0, 5.0, pWorld), HGF2DLocation(10.0, 15.0, pWorld));
    HVE2DPolySegment    VerticalPolySegmentSup8(HGF2DLocation(10.0, 13.0, pWorld), HGF2DLocation(10.0, 25.0, pWorld));
    HVE2DPolySegment    VerticalPolySegmentSup9(HGF2DLocation(10.0, 12.0, pWorld), HGF2DLocation(10.0, 18.0, pWorld));

    HVE2DPolySegment    VerticalPolySegmentSup10(HGF2DLocation(10.0, 20.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HVE2DPolySegment    VerticalPolySegmentSup11(HGF2DLocation(10.0, 35.0, pWorld), HGF2DLocation(10.0, 5.0, pWorld));
    HVE2DPolySegment    VerticalPolySegmentSup12(HGF2DLocation(10.0, 20.0, pWorld), HGF2DLocation(10.0, 5.0, pWorld));
    HVE2DPolySegment    VerticalPolySegmentSup13(HGF2DLocation(10.0, 25.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    HVE2DPolySegment    VerticalPolySegmentSup14(HGF2DLocation(10.0, 15.0, pWorld), HGF2DLocation(10.0, 5.0, pWorld));
    HVE2DPolySegment    VerticalPolySegmentSup15(HGF2DLocation(10.0, 25.0, pWorld), HGF2DLocation(10.0, 13.0, pWorld));
    HVE2DPolySegment    VerticalPolySegmentSup16(HGF2DLocation(10.0, 18.0, pWorld), HGF2DLocation(10.0, 12.0, pWorld));

    ASSERT_TRUE(VerticalPolySegmentSup3.AreContiguous(VerticalPolySegmentSup4));
    ASSERT_TRUE(VerticalPolySegmentSup4.AreContiguous(VerticalPolySegmentSup3));
    ASSERT_TRUE(VerticalPolySegmentSup3.AreContiguous(VerticalPolySegmentSup5));
    ASSERT_TRUE(VerticalPolySegmentSup5.AreContiguous(VerticalPolySegmentSup3));
    ASSERT_TRUE(VerticalPolySegmentSup3.AreContiguous(VerticalPolySegmentSup6));
    ASSERT_TRUE(VerticalPolySegmentSup6.AreContiguous(VerticalPolySegmentSup3));
    ASSERT_TRUE(VerticalPolySegmentSup3.AreContiguous(VerticalPolySegmentSup7));
    ASSERT_TRUE(VerticalPolySegmentSup7.AreContiguous(VerticalPolySegmentSup3));
    ASSERT_TRUE(VerticalPolySegmentSup3.AreContiguous(VerticalPolySegmentSup8));
    ASSERT_TRUE(VerticalPolySegmentSup8.AreContiguous(VerticalPolySegmentSup3));
    ASSERT_TRUE(VerticalPolySegmentSup3.AreContiguous(VerticalPolySegmentSup9));
    ASSERT_TRUE(VerticalPolySegmentSup9.AreContiguous(VerticalPolySegmentSup3));
    ASSERT_TRUE(VerticalPolySegmentSup3.AreContiguous(VerticalPolySegmentSup10));
    ASSERT_TRUE(VerticalPolySegmentSup10.AreContiguous(VerticalPolySegmentSup3));
    ASSERT_TRUE(VerticalPolySegmentSup3.AreContiguous(VerticalPolySegmentSup11));
    ASSERT_TRUE(VerticalPolySegmentSup11.AreContiguous(VerticalPolySegmentSup3));
    ASSERT_TRUE(VerticalPolySegmentSup3.AreContiguous(VerticalPolySegmentSup12));
    ASSERT_TRUE(VerticalPolySegmentSup12.AreContiguous(VerticalPolySegmentSup3));
    ASSERT_TRUE(VerticalPolySegmentSup3.AreContiguous(VerticalPolySegmentSup13));
    ASSERT_TRUE(VerticalPolySegmentSup13.AreContiguous(VerticalPolySegmentSup3));
    ASSERT_TRUE(VerticalPolySegmentSup3.AreContiguous(VerticalPolySegmentSup14));
    ASSERT_TRUE(VerticalPolySegmentSup14.AreContiguous(VerticalPolySegmentSup3));
    ASSERT_TRUE(VerticalPolySegmentSup3.AreContiguous(VerticalPolySegmentSup15));
    ASSERT_TRUE(VerticalPolySegmentSup15.AreContiguous(VerticalPolySegmentSup3));
    ASSERT_TRUE(VerticalPolySegmentSup3.AreContiguous(VerticalPolySegmentSup16));
    ASSERT_TRUE(VerticalPolySegmentSup16.AreContiguous(VerticalPolySegmentSup3));

    ASSERT_TRUE(VerticalPolySegmentSup4.AreContiguous(VerticalPolySegmentSup5));
    ASSERT_TRUE(VerticalPolySegmentSup5.AreContiguous(VerticalPolySegmentSup4));
    ASSERT_TRUE(VerticalPolySegmentSup4.AreContiguous(VerticalPolySegmentSup6));
    ASSERT_TRUE(VerticalPolySegmentSup6.AreContiguous(VerticalPolySegmentSup4));
    ASSERT_TRUE(VerticalPolySegmentSup4.AreContiguous(VerticalPolySegmentSup7));
    ASSERT_TRUE(VerticalPolySegmentSup7.AreContiguous(VerticalPolySegmentSup4));
    ASSERT_TRUE(VerticalPolySegmentSup4.AreContiguous(VerticalPolySegmentSup8));
    ASSERT_TRUE(VerticalPolySegmentSup8.AreContiguous(VerticalPolySegmentSup4));
    ASSERT_TRUE(VerticalPolySegmentSup4.AreContiguous(VerticalPolySegmentSup9));
    ASSERT_TRUE(VerticalPolySegmentSup9.AreContiguous(VerticalPolySegmentSup4));
    ASSERT_TRUE(VerticalPolySegmentSup4.AreContiguous(VerticalPolySegmentSup10));
    ASSERT_TRUE(VerticalPolySegmentSup10.AreContiguous(VerticalPolySegmentSup4));
    ASSERT_TRUE(VerticalPolySegmentSup4.AreContiguous(VerticalPolySegmentSup11));
    ASSERT_TRUE(VerticalPolySegmentSup11.AreContiguous(VerticalPolySegmentSup4));
    ASSERT_TRUE(VerticalPolySegmentSup4.AreContiguous(VerticalPolySegmentSup12));
    ASSERT_TRUE(VerticalPolySegmentSup12.AreContiguous(VerticalPolySegmentSup4));
    ASSERT_TRUE(VerticalPolySegmentSup4.AreContiguous(VerticalPolySegmentSup13));
    ASSERT_TRUE(VerticalPolySegmentSup13.AreContiguous(VerticalPolySegmentSup4));
    ASSERT_TRUE(VerticalPolySegmentSup4.AreContiguous(VerticalPolySegmentSup14));
    ASSERT_TRUE(VerticalPolySegmentSup14.AreContiguous(VerticalPolySegmentSup4));
    ASSERT_TRUE(VerticalPolySegmentSup4.AreContiguous(VerticalPolySegmentSup15));
    ASSERT_TRUE(VerticalPolySegmentSup15.AreContiguous(VerticalPolySegmentSup4));
    ASSERT_TRUE(VerticalPolySegmentSup4.AreContiguous(VerticalPolySegmentSup16));
    ASSERT_TRUE(VerticalPolySegmentSup16.AreContiguous(VerticalPolySegmentSup4));

    ASSERT_TRUE(VerticalPolySegmentSup5.AreContiguous(VerticalPolySegmentSup6));
    ASSERT_TRUE(VerticalPolySegmentSup6.AreContiguous(VerticalPolySegmentSup5));
    ASSERT_TRUE(VerticalPolySegmentSup5.AreContiguous(VerticalPolySegmentSup7));
    ASSERT_TRUE(VerticalPolySegmentSup7.AreContiguous(VerticalPolySegmentSup5));
    ASSERT_TRUE(VerticalPolySegmentSup5.AreContiguous(VerticalPolySegmentSup8));
    ASSERT_TRUE(VerticalPolySegmentSup8.AreContiguous(VerticalPolySegmentSup5));
    ASSERT_TRUE(VerticalPolySegmentSup5.AreContiguous(VerticalPolySegmentSup9));
    ASSERT_TRUE(VerticalPolySegmentSup9.AreContiguous(VerticalPolySegmentSup5));
    ASSERT_TRUE(VerticalPolySegmentSup5.AreContiguous(VerticalPolySegmentSup10));
    ASSERT_TRUE(VerticalPolySegmentSup10.AreContiguous(VerticalPolySegmentSup5));
    ASSERT_TRUE(VerticalPolySegmentSup5.AreContiguous(VerticalPolySegmentSup11));
    ASSERT_TRUE(VerticalPolySegmentSup11.AreContiguous(VerticalPolySegmentSup5));
    ASSERT_TRUE(VerticalPolySegmentSup5.AreContiguous(VerticalPolySegmentSup12));
    ASSERT_TRUE(VerticalPolySegmentSup12.AreContiguous(VerticalPolySegmentSup5));
    ASSERT_TRUE(VerticalPolySegmentSup5.AreContiguous(VerticalPolySegmentSup13));
    ASSERT_TRUE(VerticalPolySegmentSup13.AreContiguous(VerticalPolySegmentSup5));
    ASSERT_TRUE(VerticalPolySegmentSup5.AreContiguous(VerticalPolySegmentSup14));
    ASSERT_TRUE(VerticalPolySegmentSup14.AreContiguous(VerticalPolySegmentSup5));
    ASSERT_TRUE(VerticalPolySegmentSup5.AreContiguous(VerticalPolySegmentSup15));
    ASSERT_TRUE(VerticalPolySegmentSup15.AreContiguous(VerticalPolySegmentSup5));
    ASSERT_TRUE(VerticalPolySegmentSup5.AreContiguous(VerticalPolySegmentSup16));
    ASSERT_TRUE(VerticalPolySegmentSup16.AreContiguous(VerticalPolySegmentSup5));

    ASSERT_TRUE(VerticalPolySegmentSup6.AreContiguous(VerticalPolySegmentSup7));
    ASSERT_TRUE(VerticalPolySegmentSup7.AreContiguous(VerticalPolySegmentSup6));
    ASSERT_TRUE(VerticalPolySegmentSup6.AreContiguous(VerticalPolySegmentSup8));
    ASSERT_TRUE(VerticalPolySegmentSup8.AreContiguous(VerticalPolySegmentSup6));
    ASSERT_TRUE(VerticalPolySegmentSup6.AreContiguous(VerticalPolySegmentSup9));
    ASSERT_TRUE(VerticalPolySegmentSup9.AreContiguous(VerticalPolySegmentSup6));
    ASSERT_TRUE(VerticalPolySegmentSup6.AreContiguous(VerticalPolySegmentSup10));
    ASSERT_TRUE(VerticalPolySegmentSup10.AreContiguous(VerticalPolySegmentSup6));
    ASSERT_TRUE(VerticalPolySegmentSup6.AreContiguous(VerticalPolySegmentSup11));
    ASSERT_TRUE(VerticalPolySegmentSup11.AreContiguous(VerticalPolySegmentSup6));
    ASSERT_TRUE(VerticalPolySegmentSup6.AreContiguous(VerticalPolySegmentSup12));
    ASSERT_TRUE(VerticalPolySegmentSup12.AreContiguous(VerticalPolySegmentSup6));
    ASSERT_TRUE(VerticalPolySegmentSup6.AreContiguous(VerticalPolySegmentSup13));
    ASSERT_TRUE(VerticalPolySegmentSup13.AreContiguous(VerticalPolySegmentSup6));
    ASSERT_TRUE(VerticalPolySegmentSup6.AreContiguous(VerticalPolySegmentSup14));
    ASSERT_TRUE(VerticalPolySegmentSup14.AreContiguous(VerticalPolySegmentSup6));
    ASSERT_TRUE(VerticalPolySegmentSup6.AreContiguous(VerticalPolySegmentSup15));
    ASSERT_TRUE(VerticalPolySegmentSup15.AreContiguous(VerticalPolySegmentSup6));
    ASSERT_TRUE(VerticalPolySegmentSup6.AreContiguous(VerticalPolySegmentSup16));
    ASSERT_TRUE(VerticalPolySegmentSup16.AreContiguous(VerticalPolySegmentSup6));

    ASSERT_TRUE(VerticalPolySegmentSup7.AreContiguous(VerticalPolySegmentSup8));
    ASSERT_TRUE(VerticalPolySegmentSup8.AreContiguous(VerticalPolySegmentSup7));
    ASSERT_TRUE(VerticalPolySegmentSup7.AreContiguous(VerticalPolySegmentSup9));
    ASSERT_TRUE(VerticalPolySegmentSup9.AreContiguous(VerticalPolySegmentSup7));
    ASSERT_TRUE(VerticalPolySegmentSup7.AreContiguous(VerticalPolySegmentSup10));
    ASSERT_TRUE(VerticalPolySegmentSup10.AreContiguous(VerticalPolySegmentSup7));
    ASSERT_TRUE(VerticalPolySegmentSup7.AreContiguous(VerticalPolySegmentSup11));
    ASSERT_TRUE(VerticalPolySegmentSup11.AreContiguous(VerticalPolySegmentSup7));
    ASSERT_TRUE(VerticalPolySegmentSup7.AreContiguous(VerticalPolySegmentSup12));
    ASSERT_TRUE(VerticalPolySegmentSup12.AreContiguous(VerticalPolySegmentSup7));
    ASSERT_TRUE(VerticalPolySegmentSup7.AreContiguous(VerticalPolySegmentSup13));
    ASSERT_TRUE(VerticalPolySegmentSup13.AreContiguous(VerticalPolySegmentSup7));
    ASSERT_TRUE(VerticalPolySegmentSup7.AreContiguous(VerticalPolySegmentSup14));
    ASSERT_TRUE(VerticalPolySegmentSup14.AreContiguous(VerticalPolySegmentSup7));
    ASSERT_TRUE(VerticalPolySegmentSup7.AreContiguous(VerticalPolySegmentSup15));
    ASSERT_TRUE(VerticalPolySegmentSup15.AreContiguous(VerticalPolySegmentSup7));
    ASSERT_TRUE(VerticalPolySegmentSup7.AreContiguous(VerticalPolySegmentSup16));
    ASSERT_TRUE(VerticalPolySegmentSup16.AreContiguous(VerticalPolySegmentSup7));

    ASSERT_TRUE(VerticalPolySegmentSup8.AreContiguous(VerticalPolySegmentSup9));
    ASSERT_TRUE(VerticalPolySegmentSup9.AreContiguous(VerticalPolySegmentSup8));
    ASSERT_TRUE(VerticalPolySegmentSup8.AreContiguous(VerticalPolySegmentSup10));
    ASSERT_TRUE(VerticalPolySegmentSup10.AreContiguous(VerticalPolySegmentSup8));
    ASSERT_TRUE(VerticalPolySegmentSup8.AreContiguous(VerticalPolySegmentSup11));
    ASSERT_TRUE(VerticalPolySegmentSup11.AreContiguous(VerticalPolySegmentSup8));
    ASSERT_TRUE(VerticalPolySegmentSup8.AreContiguous(VerticalPolySegmentSup12));
    ASSERT_TRUE(VerticalPolySegmentSup12.AreContiguous(VerticalPolySegmentSup8));
    ASSERT_TRUE(VerticalPolySegmentSup8.AreContiguous(VerticalPolySegmentSup13));
    ASSERT_TRUE(VerticalPolySegmentSup13.AreContiguous(VerticalPolySegmentSup8));
    ASSERT_TRUE(VerticalPolySegmentSup8.AreContiguous(VerticalPolySegmentSup14));
    ASSERT_TRUE(VerticalPolySegmentSup14.AreContiguous(VerticalPolySegmentSup8));
    ASSERT_TRUE(VerticalPolySegmentSup8.AreContiguous(VerticalPolySegmentSup15));
    ASSERT_TRUE(VerticalPolySegmentSup15.AreContiguous(VerticalPolySegmentSup8));
    ASSERT_TRUE(VerticalPolySegmentSup8.AreContiguous(VerticalPolySegmentSup16));
    ASSERT_TRUE(VerticalPolySegmentSup16.AreContiguous(VerticalPolySegmentSup8));

    ASSERT_TRUE(VerticalPolySegmentSup9.AreContiguous(VerticalPolySegmentSup10));
    ASSERT_TRUE(VerticalPolySegmentSup10.AreContiguous(VerticalPolySegmentSup9));
    ASSERT_TRUE(VerticalPolySegmentSup9.AreContiguous(VerticalPolySegmentSup11));
    ASSERT_TRUE(VerticalPolySegmentSup11.AreContiguous(VerticalPolySegmentSup9));
    ASSERT_TRUE(VerticalPolySegmentSup9.AreContiguous(VerticalPolySegmentSup12));
    ASSERT_TRUE(VerticalPolySegmentSup12.AreContiguous(VerticalPolySegmentSup9));
    ASSERT_TRUE(VerticalPolySegmentSup9.AreContiguous(VerticalPolySegmentSup13));
    ASSERT_TRUE(VerticalPolySegmentSup13.AreContiguous(VerticalPolySegmentSup9));
    ASSERT_TRUE(VerticalPolySegmentSup9.AreContiguous(VerticalPolySegmentSup14));
    ASSERT_TRUE(VerticalPolySegmentSup14.AreContiguous(VerticalPolySegmentSup9));
    ASSERT_TRUE(VerticalPolySegmentSup9.AreContiguous(VerticalPolySegmentSup15));
    ASSERT_TRUE(VerticalPolySegmentSup15.AreContiguous(VerticalPolySegmentSup9));
    ASSERT_TRUE(VerticalPolySegmentSup9.AreContiguous(VerticalPolySegmentSup16));
    ASSERT_TRUE(VerticalPolySegmentSup16.AreContiguous(VerticalPolySegmentSup9));

    ASSERT_TRUE(VerticalPolySegmentSup10.AreContiguous(VerticalPolySegmentSup11));
    ASSERT_TRUE(VerticalPolySegmentSup11.AreContiguous(VerticalPolySegmentSup10));
    ASSERT_TRUE(VerticalPolySegmentSup10.AreContiguous(VerticalPolySegmentSup12));
    ASSERT_TRUE(VerticalPolySegmentSup12.AreContiguous(VerticalPolySegmentSup10));
    ASSERT_TRUE(VerticalPolySegmentSup10.AreContiguous(VerticalPolySegmentSup13));
    ASSERT_TRUE(VerticalPolySegmentSup13.AreContiguous(VerticalPolySegmentSup10));
    ASSERT_TRUE(VerticalPolySegmentSup10.AreContiguous(VerticalPolySegmentSup14));
    ASSERT_TRUE(VerticalPolySegmentSup14.AreContiguous(VerticalPolySegmentSup10));
    ASSERT_TRUE(VerticalPolySegmentSup10.AreContiguous(VerticalPolySegmentSup15));
    ASSERT_TRUE(VerticalPolySegmentSup15.AreContiguous(VerticalPolySegmentSup10));
    ASSERT_TRUE(VerticalPolySegmentSup10.AreContiguous(VerticalPolySegmentSup16));
    ASSERT_TRUE(VerticalPolySegmentSup16.AreContiguous(VerticalPolySegmentSup10));

    ASSERT_TRUE(VerticalPolySegmentSup11.AreContiguous(VerticalPolySegmentSup12));
    ASSERT_TRUE(VerticalPolySegmentSup12.AreContiguous(VerticalPolySegmentSup11));
    ASSERT_TRUE(VerticalPolySegmentSup11.AreContiguous(VerticalPolySegmentSup13));
    ASSERT_TRUE(VerticalPolySegmentSup13.AreContiguous(VerticalPolySegmentSup11));
    ASSERT_TRUE(VerticalPolySegmentSup11.AreContiguous(VerticalPolySegmentSup14));
    ASSERT_TRUE(VerticalPolySegmentSup14.AreContiguous(VerticalPolySegmentSup11));
    ASSERT_TRUE(VerticalPolySegmentSup11.AreContiguous(VerticalPolySegmentSup15));
    ASSERT_TRUE(VerticalPolySegmentSup15.AreContiguous(VerticalPolySegmentSup11));
    ASSERT_TRUE(VerticalPolySegmentSup11.AreContiguous(VerticalPolySegmentSup16));
    ASSERT_TRUE(VerticalPolySegmentSup16.AreContiguous(VerticalPolySegmentSup11));

    ASSERT_TRUE(VerticalPolySegmentSup12.AreContiguous(VerticalPolySegmentSup13));
    ASSERT_TRUE(VerticalPolySegmentSup13.AreContiguous(VerticalPolySegmentSup12));
    ASSERT_TRUE(VerticalPolySegmentSup12.AreContiguous(VerticalPolySegmentSup14));
    ASSERT_TRUE(VerticalPolySegmentSup14.AreContiguous(VerticalPolySegmentSup12));
    ASSERT_TRUE(VerticalPolySegmentSup12.AreContiguous(VerticalPolySegmentSup15));
    ASSERT_TRUE(VerticalPolySegmentSup15.AreContiguous(VerticalPolySegmentSup12));
    ASSERT_TRUE(VerticalPolySegmentSup12.AreContiguous(VerticalPolySegmentSup16));
    ASSERT_TRUE(VerticalPolySegmentSup16.AreContiguous(VerticalPolySegmentSup12));

    ASSERT_TRUE(VerticalPolySegmentSup13.AreContiguous(VerticalPolySegmentSup14));
    ASSERT_TRUE(VerticalPolySegmentSup14.AreContiguous(VerticalPolySegmentSup13));
    ASSERT_TRUE(VerticalPolySegmentSup13.AreContiguous(VerticalPolySegmentSup15));
    ASSERT_TRUE(VerticalPolySegmentSup15.AreContiguous(VerticalPolySegmentSup13));
    ASSERT_TRUE(VerticalPolySegmentSup13.AreContiguous(VerticalPolySegmentSup16));
    ASSERT_TRUE(VerticalPolySegmentSup16.AreContiguous(VerticalPolySegmentSup13));

    ASSERT_TRUE(VerticalPolySegmentSup14.AreContiguous(VerticalPolySegmentSup15));
    ASSERT_TRUE(VerticalPolySegmentSup15.AreContiguous(VerticalPolySegmentSup14));
    ASSERT_TRUE(VerticalPolySegmentSup14.AreContiguous(VerticalPolySegmentSup16));
    ASSERT_TRUE(VerticalPolySegmentSup16.AreContiguous(VerticalPolySegmentSup14));

    ASSERT_TRUE(VerticalPolySegmentSup15.AreContiguous(VerticalPolySegmentSup16));
    ASSERT_TRUE(VerticalPolySegmentSup16.AreContiguous(VerticalPolySegmentSup15));

    }

//==================================================================================
// Another yet test which did fail
//==================================================================================
TEST_F (HVE2DPolySegmentTester, ObtainContiguousnessPointsTest)
    {
        
    HVE2DPolySegment    HorizontalPolySegmentSup23(HGF2DLocation(20.0, 20.0, pWorld), HGF2DLocation(10.0, 20.0, pWorld));
    HVE2DPolySegment    HorizontalPolySegmentSup24(HGF2DLocation(20.0+0.9*MYEPSILON, 20.0, pWorld), HGF2DLocation(-1.0, 20.0, pWorld));

    HGF2DLocationCollection     Contig23Points;
    ASSERT_EQ(2, HorizontalPolySegmentSup23.ObtainContiguousnessPoints(HorizontalPolySegmentSup24, &Contig23Points));
     
    }

//==================================================================================
// Another yet test which did fail (July 23 1997
//==================================================================================
TEST_F (HVE2DPolySegmentTester, AreContiguousTest4)
    {
   
    HVE2DPolySegment    HorizontalPolySegmentSup1(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(0.0, 1E-6, pWorld));
    HVE2DPolySegment    HorizontalPolySegmentSup2(HGF2DLocation(0.0, 9.999999E-7, pWorld), HGF2DLocation(0.0, 7.0, pWorld));

    ASSERT_FALSE(HorizontalPolySegmentSup1.AreContiguous(HorizontalPolySegmentSup2));

    }

//==================================================================================
// Second part of poly segment testing
// The following function tests the polysegments in their behavior as a
// chain of multiple elements
//==================================================================================


//==================================================================================
// Segment Construction tests
// HVE2DSegment();
// HVE2DSegment(const HGF2DLocation&, const HGF2DLocation&);
// HVE2DSegment(const HGF2DLocation& pi_rStartPoint,
//             const HGF2DDisplacement& pi_rDisplacement);
// HVE2DSegment(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
// HVE2DSegment(const HGF2DSegment&    pi_rObject);
//==================================================================================
TEST_F (HVE2DPolySegmentTester, ConstructionTest2) 
    {

    // Default Constructor
    HVE2DPolySegment    APolySegment1;

    // Constructor with a coordinate system
    HVE2DPolySegment    APolySegment2(pWorld);
    ASSERT_EQ(pWorld, APolySegment2.GetCoordSys());

    // Copy Constructor test
    HVE2DPolySegment    APolySegment3(pWorld);
    APolySegment3.AppendPoint(HorizontalSegment2.GetStartPoint());
    APolySegment3.AppendPoint(MiscSegment1.GetStartPoint());
    APolySegment3.AppendPoint(MiscSegment1.GetEndPoint());

    HVE2DPolySegment    APolySegment4(APolySegment3);
    ASSERT_EQ(pWorld, APolySegment4.GetCoordSys());
    ASSERT_DOUBLE_EQ(10.1, APolySegment4.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, APolySegment4.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, APolySegment4.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, APolySegment4.GetEndPoint().GetY());
    ASSERT_EQ(3, APolySegment4.GetSize());

    }

//==================================================================================
// operator= test
// operator=(const HVE2DPolySegment& pi_rObj);
//==================================================================================
TEST_F (HVE2DPolySegmentTester, OperatorTest2)
    {

    HVE2DPolySegment    APolySegment1(pWorld);
    APolySegment1.AppendPoint(HorizontalSegment2.GetStartPoint());
    APolySegment1.AppendPoint(MiscSegment1.GetStartPoint());
    APolySegment1.AppendPoint(MiscSegment1.GetEndPoint());

    HVE2DPolySegment    APolySegment2(pSys1);
    APolySegment2.AppendPoint(VerticalSegment1.GetStartPoint());
    APolySegment2.AppendPoint(VerticalSegment1.GetEndPoint());

    APolySegment2 = APolySegment1;

    ASSERT_EQ(pWorld, APolySegment2.GetCoordSys());
    ASSERT_DOUBLE_EQ(10.1, APolySegment2.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(0.10, APolySegment2.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(10.1, APolySegment2.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.1, APolySegment2.GetEndPoint().GetY());
    ASSERT_EQ(3, APolySegment2.GetSize());

    }

//==================================================================================
// Type extraction test
// GetType() const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, GetTypeTest2)
    {

    // Basic test
    ASSERT_EQ(HVE2DPolySegment::CLASS_ID, PolySegment1.GetBasicLinearType());
    ASSERT_EQ(HVE2DPolySegment::CLASS_ID, PolySegment2.GetBasicLinearType());
    ASSERT_EQ(HVE2DPolySegment::CLASS_ID, PolySegment3.GetBasicLinearType());

    }

//==================================================================================
// Drop( HGF2DLocationCollection* po_pPoints, const HGFDistance& pi_rTolerance ) const
//==================================================================================
TEST_F (HVE2DPolySegmentTester, DropTest2)
    {

    HGF2DLocationCollection Locations;

    // Test with vertical PolySegment
    PolySegment1.Drop(&Locations, MYEPSILON);
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

    Locations.clear();

    }

//==================================================================================
// AppendPosition(const HGF2DPosition& pi_rNewPoint);
// GetPosition(size_t pi_Index) const;
// GetSize() const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, PositionTest)
    {

    HVE2DPolySegment    APolySegment1(pWorld);

    HGF2DPosition       APosition0(10.0, 10.0);
    HGF2DPosition       APosition1(10.0, 20.0);
    HGF2DPosition       APosition2(20.0, 10.0);
    HGF2DPosition       APosition3(20.0, 20.0);

    APolySegment1.AppendPosition(APosition0);
    APolySegment1.AppendPosition(APosition1);
    APolySegment1.AppendPosition(APosition2);
    APolySegment1.AppendPosition(APosition3);

    ASSERT_TRUE(APolySegment1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(APolySegment1.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(APolySegment1.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(APolySegment1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    ASSERT_DOUBLE_EQ(10.0, APolySegment1.GetPosition(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, APolySegment1.GetPosition(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, APolySegment1.GetPosition(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, APolySegment1.GetPosition(3).GetX());

    ASSERT_DOUBLE_EQ(10.0, APolySegment1.GetPosition(0).GetY());
    ASSERT_DOUBLE_EQ(20.0, APolySegment1.GetPosition(1).GetY());
    ASSERT_DOUBLE_EQ(10.0, APolySegment1.GetPosition(2).GetY());
    ASSERT_DOUBLE_EQ(20.0, APolySegment1.GetPosition(3).GetY());

    ASSERT_EQ(4, APolySegment1.GetSize());

    }

//==================================================================================
// AppendPoint(const HGF2DLocation& pi_rNewPoint);
// GetPoint(size_t pi_Index) const;
// RemovePoint(size_t pi_Index);
//==================================================================================
TEST_F (HVE2DPolySegmentTester, PointTest)
    {

    HVE2DPolySegment    APolySegment1(pWorld);

    HGF2DLocation       ALocation0(10.0, 10.0, pWorld);
    HGF2DLocation       ALocation1(10.0, 20.0, pWorld);
    HGF2DLocation       ALocation2(20.0, 10.0, pWorld);
    HGF2DLocation       ALocation3(20.0, 20.0, pWorld);

    APolySegment1.AppendPoint(ALocation0);
    APolySegment1.AppendPoint(ALocation1);
    APolySegment1.AppendPoint(ALocation2);
    APolySegment1.AppendPoint(ALocation3);

    ASSERT_TRUE(APolySegment1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(APolySegment1.IsPointOn(HGF2DLocation(10.0, 20.0, pWorld)));
    ASSERT_TRUE(APolySegment1.IsPointOn(HGF2DLocation(20.0, 10.0, pWorld)));
    ASSERT_TRUE(APolySegment1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));

    ASSERT_DOUBLE_EQ(10.0, APolySegment1.GetPosition(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, APolySegment1.GetPosition(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, APolySegment1.GetPosition(2).GetX());
    ASSERT_DOUBLE_EQ(20.0, APolySegment1.GetPosition(3).GetX());

    ASSERT_DOUBLE_EQ(10.0, APolySegment1.GetPosition(0).GetY());
    ASSERT_DOUBLE_EQ(20.0, APolySegment1.GetPosition(1).GetY());
    ASSERT_DOUBLE_EQ(10.0, APolySegment1.GetPosition(2).GetY());
    ASSERT_DOUBLE_EQ(20.0, APolySegment1.GetPosition(3).GetY());

    ASSERT_EQ(4, APolySegment1.GetSize());

    APolySegment1.RemovePoint(3);
    ASSERT_EQ(3, APolySegment1.GetSize());
    ASSERT_DOUBLE_EQ(20.0,  APolySegment1.GetPoint(2).GetX());
    ASSERT_DOUBLE_EQ(10.0,  APolySegment1.GetPoint(2).GetY());

    APolySegment1.RemovePoint(2);
    ASSERT_EQ(2, APolySegment1.GetSize());
    ASSERT_DOUBLE_EQ(10.0, APolySegment1.GetPoint(1).GetX());
    ASSERT_DOUBLE_EQ(20.0, APolySegment1.GetPoint(1).GetY());

    APolySegment1.RemovePoint(1);
    ASSERT_EQ(1, APolySegment1.GetSize());
    ASSERT_DOUBLE_EQ(10.0, APolySegment1.GetPoint(0).GetX());
    ASSERT_DOUBLE_EQ(10.0, APolySegment1.GetPoint(0).GetY());

    APolySegment1.RemovePoint(0);
    ASSERT_EQ(0, APolySegment1.GetSize());
    ASSERT_TRUE(APolySegment1.IsNull());

    }

//==================================================================================
// Length calculation test
// CalculateLength() const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CalculateLengthTest2)
    {

    // Test with PolySegment 1
    ASSERT_DOUBLE_EQ(44.142135623730951, PolySegment1.CalculateLength());

    // Test with PolySegment 1
    ASSERT_DOUBLE_EQ(111.76387577639170, PolySegment2.CalculateLength());

    // Test with empty PolySegment
    ASSERT_NEAR(0.0, EmptyPolySegment.CalculateLength(), MYEPSILON);
       
    }

//==================================================================================
// Relative point calculation test
// CalculateRelativePoint(double pi_RelativePos) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CalculateRelativePointTest2)
    {

    // Test with PolySegment 1
    ASSERT_NEAR(0.0, PolySegment1.CalculateRelativePoint(0.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, PolySegment1.CalculateRelativePoint(0.0).GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(3.1213203435596428, PolySegment1.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(3.1213203435596428, PolySegment1.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(17.928932188134524, PolySegment1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, PolySegment1.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(26.757359312880713, PolySegment1.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, PolySegment1.CalculateRelativePoint(0.7).GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999, PolySegment1.CalculateRelativePoint(1.0).GetX());
    ASSERT_NEAR(0.0, PolySegment1.CalculateRelativePoint(1.0).GetY(), MYEPSILON);

    // Test with PolySegment 2
    ASSERT_NEAR(0.0, PolySegment2.CalculateRelativePoint(0.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, PolySegment2.CalculateRelativePoint(0.0).GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(-7.9028994453177486, PolySegment2.CalculateRelativePoint(0.1).GetX());
    ASSERT_DOUBLE_EQ(-7.9028994453177486, PolySegment2.CalculateRelativePoint(0.1).GetY());
    ASSERT_DOUBLE_EQ(20.4988171445599400, PolySegment2.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(1.50354856632018130, PolySegment2.CalculateRelativePoint(0.5).GetY());
    ASSERT_DOUBLE_EQ(18.5291627329175200, PolySegment2.CalculateRelativePoint(0.7).GetX());
    ASSERT_DOUBLE_EQ(15.0000000000000000, PolySegment2.CalculateRelativePoint(0.7).GetY());
    ASSERT_NEAR(0.0, PolySegment2.CalculateRelativePoint(1.0).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, PolySegment2.CalculateRelativePoint(1.0).GetY(), MYEPSILON);

    }

//==================================================================================
// Relative position calculation test
// CalculateRelativePosition(const HGF2DLocation& pi_rPointOnPolySegment) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CalculateRelativePositionTest2)
    {

    // Test with PolySegment1
    ASSERT_NEAR(0.0, PolySegment1.CalculateRelativePosition(PolySegment1Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.099999999999998646, PolySegment1.CalculateRelativePosition(PolySegment1Point0d1));
    ASSERT_DOUBLE_EQ(0.500000000000010770, PolySegment1.CalculateRelativePosition(PolySegment1Point0d5));
    ASSERT_DOUBLE_EQ(1.000000000000000000, PolySegment1.CalculateRelativePosition(PolySegment1Point1d0));

    // Test with PolySegment2
    ASSERT_NEAR(0.0, PolySegment2.CalculateRelativePosition(PolySegment2Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.099999999999999381, PolySegment2.CalculateRelativePosition(PolySegment2Point0d1));
    ASSERT_DOUBLE_EQ(0.500000000000001670, PolySegment2.CalculateRelativePosition(PolySegment2Point0d5));

    // NORMAL Strange behavior provoqued by auto-clossing condition
    ASSERT_NEAR(0.0, PolySegment2.CalculateRelativePosition(PolySegment2Point1d0), MYEPSILON);

    // Test with PolySegment3 (epsilon sized container)
    ASSERT_NEAR(0.0, PolySegment3.CalculateRelativePosition(PolySegment3Point0d0), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.10000000000000007, PolySegment3.CalculateRelativePosition(PolySegment3Point0d1));
    
    #ifdef WIP_IPPTEST_BUG_5 
    // Strange behavior due to epsilon size of PolySegment
    // returned value is 4.90.. and -0.1
    //ASSERT_DOUBLE_EQ(0.5, PolySegment3.CalculateRelativePosition(PolySegment3Point0d5));
    //ASSERT_DOUBLE_EQ(1.0, PolySegment3.CalculateRelativePosition(PolySegment3Point1d0));
    #endif

    }

//==================================================================================
// Rotate( const HGFAngle& pi_rAngle, const HGF2DLocation& pi_rOrigin )
//==================================================================================
TEST_F (HVE2DPolySegmentTester, RotateTest2) 
    {

    HVE2DPolySegment    RotateTest1(pWorld);

    RotateTest1.AppendPoint(HGF2DLocation(1.0, 1.0, pWorld));
    RotateTest1.AppendPoint(HGF2DLocation(2.0, 2.0, pWorld));
    RotateTest1.AppendPoint(HGF2DLocation(3.0, 2.0, pWorld));
    RotateTest1.AppendPoint(HGF2DLocation(3.0, 3.0, pWorld));

    RotateTest1.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld)); 

    ASSERT_TRUE(RotateTest1.IsPointOn(HGF2DLocation(-1.0, -1.0, pWorld)));
    ASSERT_TRUE(RotateTest1.IsPointOn(HGF2DLocation(-2.0, -2.0, pWorld)));
    ASSERT_TRUE(RotateTest1.IsPointOn(HGF2DLocation(-3.0, -2.0, pWorld)));
    ASSERT_TRUE(RotateTest1.IsPointOn(HGF2DLocation(-3.0, -3.0, pWorld)));

    RotateTest1.Rotate(PI, HGF2DLocation(0.0, 0.0, pWorld)); 

    ASSERT_TRUE(RotateTest1.IsPointOn(HGF2DLocation(1.0, 1.0, pWorld)));
    ASSERT_TRUE(RotateTest1.IsPointOn(HGF2DLocation(2.0, 2.0, pWorld)));
    ASSERT_TRUE(RotateTest1.IsPointOn(HGF2DLocation(3.0, 2.0, pWorld)));
    ASSERT_TRUE(RotateTest1.IsPointOn(HGF2DLocation(3.0, 3.0, pWorld)));
    
    RotateTest1.Rotate(PI, HGF2DLocation(1.0, 1.0, pWorld)); 

    ASSERT_TRUE(RotateTest1.IsPointOn(HGF2DLocation(1.0, 1.0, pWorld)));
    ASSERT_TRUE(RotateTest1.IsPointOn(HGF2DLocation(0.0, 0.0, pWorld)));
    ASSERT_TRUE(RotateTest1.IsPointOn(HGF2DLocation(-1.0, 0.0, pWorld)));
    ASSERT_TRUE(RotateTest1.IsPointOn(HGF2DLocation(-1.0, -1.0, pWorld)));

    }

//==================================================================================
// Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin);
//==================================================================================
TEST_F (HVE2DPolySegmentTester, ScaleTest2) 
    {

    HVE2DPolySegment    ScaleTest1(pWorld);

    ScaleTest1.AppendPoint(HGF2DLocation(1.0, 1.0, pWorld));
    ScaleTest1.AppendPoint(HGF2DLocation(2.0, 2.0, pWorld));
    ScaleTest1.AppendPoint(HGF2DLocation(3.0, 2.0, pWorld));
    ScaleTest1.AppendPoint(HGF2DLocation(3.0, 3.0, pWorld));

    ScaleTest1.Scale(5.0, HGF2DLocation(0.0, 0.0, pWorld)); 

    ASSERT_TRUE(ScaleTest1.IsPointOn(HGF2DLocation(5.0, 5.0, pWorld)));
    ASSERT_TRUE(ScaleTest1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(ScaleTest1.IsPointOn(HGF2DLocation(15.0, 10.0, pWorld)));
    ASSERT_TRUE(ScaleTest1.IsPointOn(HGF2DLocation(15.0, 15.0, pWorld)));

    ScaleTest1.Scale(2.0, HGF2DLocation(0.0, 0.0, pWorld)); 

    ASSERT_TRUE(ScaleTest1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(ScaleTest1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(ScaleTest1.IsPointOn(HGF2DLocation(30.0, 20.0, pWorld)));
    ASSERT_TRUE(ScaleTest1.IsPointOn(HGF2DLocation(30.0, 30.0, pWorld)));

    ScaleTest1.Scale(1.0, HGF2DLocation(10.0, 5.0, pWorld)); 

    ASSERT_TRUE(ScaleTest1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_TRUE(ScaleTest1.IsPointOn(HGF2DLocation(20.0, 20.0, pWorld)));
    ASSERT_TRUE(ScaleTest1.IsPointOn(HGF2DLocation(30.0, 20.0, pWorld)));
    ASSERT_TRUE(ScaleTest1.IsPointOn(HGF2DLocation(30.0, 30.0, pWorld)));

    }

//==================================================================================
// RayArea Calculation test
// CalculateRayArea(const HGF2DLocation& pi_rPoint) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CalculateRayAreaTest2)
    {
    
    // Test with PolySegment1
    ASSERT_DOUBLE_EQ(-175.0000015, PolySegment1.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Test with PolySegment2
    ASSERT_DOUBLE_EQ(288.00000000, PolySegment2.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)));

    // Test with PolySegment3 (epsilon sized container)
    ASSERT_NEAR(0.0, PolySegment3.CalculateRayArea(HGF2DLocation(0.0, 0.0, pWorld)), MYEPSILON);

    }

//==================================================================================
// Shortening tests
// Shorten(double pi_StartRelativePos, double pi_EndRelativePos);
// Shorten(const HGF2DLocation& pi_rNewStartPoint,
//         const HGF2DLocation& pi_rNewEndPoint);
// ShortenTo(const HGF2DLocation& pi_rNewEndPoint);
// ShortenTo(double pi_EndRelativePosition);
// ShortenFrom(const HGF2DLocation& pi_rNewStartPoint);
// ShortenFrom(double pi_StartRelativePosition);
//==================================================================================
TEST_F (HVE2DPolySegmentTester, ShorteningTest2)
    {

    // Test with PolySegment1
    HVE2DPolySegment    APolySegment1(PolySegment1);

    APolySegment1.Shorten(0.2, 0.8);
    ASSERT_DOUBLE_EQ(6.2426406871192857, APolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(6.2426406871192857, APolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(30.000000000000000, APolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.8284271247461916, APolySegment1.GetEndPoint().GetY());


    APolySegment1 = PolySegment1;
    APolySegment1.Shorten(0.5, 0.5+MYEPSILON);
    ASSERT_DOUBLE_EQ(17.928936602348085, APolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, APolySegment1.GetEndPoint().GetY());

    APolySegment1 = PolySegment1;
    APolySegment1.Shorten(0.0, 0.8);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.000000000000000, APolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.8284271247461916, APolySegment1.GetEndPoint().GetY());

    APolySegment1 = PolySegment1;
    APolySegment1.Shorten(0.2, 1.0);
    ASSERT_DOUBLE_EQ(6.2426406871192857, APolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(6.2426406871192857, APolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999, APolySegment1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetY(), MYEPSILON);

    APolySegment1 = PolySegment1;
    APolySegment1.Shorten(0.0, 1.0);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, APolySegment1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetY(), MYEPSILON);

    APolySegment1 = PolySegment1;
    APolySegment1.ShortenTo(0.8);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.000000000000000, APolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(8.8284271247461916, APolySegment1.GetEndPoint().GetY());

    APolySegment1 = PolySegment1;
    APolySegment1.ShortenTo(1.0-MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999900000088000, APolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(4.4142135635638624E-6, APolySegment1.GetEndPoint().GetY());

    APolySegment1 = PolySegment1;
    APolySegment1.ShortenTo(1.0);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, APolySegment1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetY(), MYEPSILON);

    APolySegment1 = PolySegment1;
    APolySegment1.ShortenTo(0.0);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetY(), MYEPSILON);

    APolySegment1 = PolySegment1;
    APolySegment1.ShortenFrom(0.2);
    ASSERT_DOUBLE_EQ(6.2426406871192857, APolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(6.2426406871192857, APolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999, APolySegment1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetY(), MYEPSILON);

    APolySegment1 = PolySegment1;
    APolySegment1.ShortenFrom(1.0-MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999900000088000, APolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(4.4142135635638624E-6, APolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999000, APolySegment1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetY(), MYEPSILON);

    APolySegment1 = PolySegment1;
    APolySegment1.ShortenFrom(1.0);
    ASSERT_DOUBLE_EQ(29.999999899999999, APolySegment1.GetStartPoint().GetX());
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, APolySegment1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetY(), MYEPSILON);

    APolySegment1 = PolySegment1;
    APolySegment1.ShortenFrom(0.0);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, APolySegment1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetY(), MYEPSILON);

    APolySegment1 = PolySegment1;
    APolySegment1.ShortenFrom(PolySegmentMidPoint1);
    ASSERT_DOUBLE_EQ(17.928932188135001, APolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, APolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(29.999999900000000, APolySegment1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetY(), MYEPSILON);

    APolySegment1 = PolySegment1;
    APolySegment1.ShortenFrom(APolySegment1.GetStartPoint());
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.9999999, APolySegment1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetY(), MYEPSILON);

    APolySegment1 = PolySegment1;
    APolySegment1.ShortenFrom(APolySegment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(29.999999900000000, APolySegment1.GetStartPoint().GetX());
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, APolySegment1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetY(), MYEPSILON);

    APolySegment1 = PolySegment1;
    APolySegment1.ShortenTo(PolySegmentMidPoint1);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(17.928932188135001, APolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, APolySegment1.GetEndPoint().GetY());

    APolySegment1 = PolySegment1;
    APolySegment1.ShortenTo(APolySegment1.GetStartPoint());
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetY(), MYEPSILON);

    APolySegment1 = PolySegment1;
    APolySegment1.ShortenTo(APolySegment1.GetEndPoint());
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.9999999, APolySegment1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetY(), MYEPSILON);

    APolySegment1 = PolySegment1;
    APolySegment1.Shorten(APolySegment1.GetStartPoint(), PolySegmentMidPoint1);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(17.928932188135001, APolySegment1.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, APolySegment1.GetEndPoint().GetY());

    APolySegment1 = PolySegment1;
    APolySegment1.Shorten(PolySegmentMidPoint1, APolySegment1.GetEndPoint());
    ASSERT_DOUBLE_EQ(17.928932188135001, APolySegment1.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, APolySegment1.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(29.999999899999999, APolySegment1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetY(), MYEPSILON);

    APolySegment1 = PolySegment1;
    APolySegment1.Shorten(APolySegment1.GetStartPoint(), APolySegment1.GetEndPoint());
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, APolySegment1.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.9999999, APolySegment1.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, APolySegment1.GetEndPoint().GetY(), MYEPSILON);

    }

//==================================================================================
// RemovePoint tests
//==================================================================================
TEST_F (HVE2DPolySegmentTester, RemovePointTest2)
    {
   
    // Obtain a duplicate of polysegment
    HVE2DPolySegment TestPolySegment(PolySegment1);

    ASSERT_EQ(7, TestPolySegment.GetSize());
    ASSERT_NEAR(0.0, TestPolySegment.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, TestPolySegment.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.9999999, TestPolySegment.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, TestPolySegment.GetEndPoint().GetY(), MYEPSILON);

    TestPolySegment.RemovePoint(2);
    ASSERT_EQ(6, TestPolySegment.GetSize());
    ASSERT_NEAR(0.0, TestPolySegment.GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, TestPolySegment.GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.9999999, TestPolySegment.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, TestPolySegment.GetEndPoint().GetY(), MYEPSILON);

    TestPolySegment.RemovePoint(0);
    ASSERT_EQ(5, TestPolySegment.GetSize());
    ASSERT_DOUBLE_EQ(10.0000000, TestPolySegment.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0000000, TestPolySegment.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(29.9999999, TestPolySegment.GetEndPoint().GetX());
    ASSERT_NEAR(0.0, TestPolySegment.GetEndPoint().GetY(), MYEPSILON);

    TestPolySegment.RemovePoint(4);
    ASSERT_EQ(4, TestPolySegment.GetSize());
    ASSERT_DOUBLE_EQ(10.0000000, TestPolySegment.GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(10.0000000, TestPolySegment.GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(30.0000000, TestPolySegment.GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(4.99999990, TestPolySegment.GetEndPoint().GetY());

    TestPolySegment.RemovePoint(3);
    ASSERT_EQ(3, TestPolySegment.GetSize());
    TestPolySegment.RemovePoint(2);
    ASSERT_EQ(2, TestPolySegment.GetSize());
    TestPolySegment.RemovePoint(1);
    ASSERT_EQ(1, TestPolySegment.GetSize());
    TestPolySegment.RemovePoint(0);
    ASSERT_EQ(0, TestPolySegment.GetSize());

    }

//==================================================================================
// Auto intersection tests
// AutoCrosses() const;
// AutoIntersect(HGF2DLocationCollection* po_pPoints) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, AutoIntersectTest2)
    {

    HGF2DLocationCollection     DumPoints;

    // Test with a negative
    ASSERT_FALSE(PolySegment1.AutoCrosses());

    // Test with auto closed PolySegment
    ASSERT_FALSE(PolySegment2.AutoCrosses());

    // Test with an auto crossing PolySegment
    ASSERT_TRUE(AutoCrossingPolySegment1.AutoCrosses());
    ASSERT_EQ(4, AutoCrossingPolySegment1.AutoIntersect(&DumPoints));

    ASSERT_TRUE(AutoCrossingPolySegment2.AutoCrosses());

    ASSERT_EQ(1, AutoCrossingPolySegment2.AutoIntersect(&DumPoints));
    AutoCrossingPolySegment2.Reverse();
    ASSERT_TRUE(AutoCrossingPolySegment2.AutoCrosses());
    ASSERT_EQ(1, AutoCrossingPolySegment2.AutoIntersect(&DumPoints));
    AutoCrossingPolySegment2.Reverse();

    // Test with an non auto crossing PolySegment (but passing through extremity)
    ASSERT_FALSE(AutoConnectingPolySegment1.AutoCrosses());

    ASSERT_TRUE(AutoCrossingPolySegment3.AutoCrosses());
    ASSERT_EQ(4, AutoCrossingPolySegment3.AutoIntersect(&DumPoints));
    AutoCrossingPolySegment3.Reverse();
    ASSERT_TRUE(AutoCrossingPolySegment3.AutoCrosses());
    ASSERT_EQ(4, AutoCrossingPolySegment3.AutoIntersect(&DumPoints));
    AutoCrossingPolySegment3.Reverse();

    ASSERT_TRUE(AutoCrossingPolySegment4.AutoCrosses());
    ASSERT_EQ(4, AutoCrossingPolySegment4.AutoIntersect(&DumPoints));
    AutoCrossingPolySegment4.Reverse();
    ASSERT_TRUE(AutoCrossingPolySegment4.AutoCrosses());
    ASSERT_EQ(4, AutoCrossingPolySegment4.AutoIntersect(&DumPoints));
    AutoCrossingPolySegment4.Reverse();

    ASSERT_TRUE(AutoCrossingPolySegment5.AutoCrosses());
    ASSERT_EQ(4, AutoCrossingPolySegment5.AutoIntersect(&DumPoints));
    AutoCrossingPolySegment5.Reverse();
    ASSERT_TRUE(AutoCrossingPolySegment5.AutoCrosses());
    ASSERT_EQ(4, AutoCrossingPolySegment5.AutoIntersect(&DumPoints));
    AutoCrossingPolySegment5.Reverse();

    ASSERT_TRUE(AutoCrossingPolySegment6.AutoCrosses());
    ASSERT_EQ(1, AutoCrossingPolySegment6.AutoIntersect(&DumPoints));
    DumPoints.clear();
    ASSERT_EQ(1, AutoCrossingPolySegment6.AutoIntersect(&DumPoints));
    AutoCrossingPolySegment6.Reverse();
    ASSERT_TRUE(AutoCrossingPolySegment6.AutoCrosses());
    ASSERT_EQ(1, AutoCrossingPolySegment6.AutoIntersect(&DumPoints));
    AutoCrossingPolySegment6.Reverse();

    ASSERT_TRUE(AutoCrossingPolySegment7.AutoCrosses());
    ASSERT_EQ(1, AutoCrossingPolySegment7.AutoIntersect(&DumPoints));
    AutoCrossingPolySegment7.Reverse();
    ASSERT_TRUE(AutoCrossingPolySegment7.AutoCrosses());
    ASSERT_EQ(1, AutoCrossingPolySegment7.AutoIntersect(&DumPoints));
    AutoCrossingPolySegment7.Reverse();

    #ifdef WIP_IPPTEST_BUG_6 

    ASSERT_TRUE(AutoCrossingPolySegment8.AutoCrosses());
//        ASSERT_EQ(1, AutoCrossingPolySegment8.AutoIntersect(&DumPoints)); // BUG #6
    AutoCrossingPolySegment8.Reverse();
    ASSERT_TRUE(AutoCrossingPolySegment8.AutoCrosses());
//        ASSERT_EQ(1, AutoCrossingPolySegment8.AutoIntersect(&DumPoints)); // BUG #6
    AutoCrossingPolySegment8.Reverse();

    ASSERT_TRUE(AutoCrossingPolySegment9.AutoCrosses());
    //    ASSERT_EQ(1, AutoCrossingPolySegment9.AutoIntersect(&DumPoints)); //BUG #6
    AutoCrossingPolySegment9.Reverse();
    ASSERT_TRUE(AutoCrossingPolySegment9.AutoCrosses());
    //    ASSERT_EQ(1, AutoCrossingPolySegment9.AutoIntersect(&DumPoints)); //BUG #6
    AutoCrossingPolySegment9.Reverse();

    ASSERT_TRUE(AutoCrossingPolySegment10.AutoCrosses());
    ASSERT_EQ(1, AutoCrossingPolySegment10.AutoIntersect(&DumPoints));
    AutoCrossingPolySegment10.Reverse();
    ASSERT_TRUE(AutoCrossingPolySegment10.AutoCrosses());
    //    ASSERT_EQ(1, AutoCrossingPolySegment10.AutoIntersect(&DumPoints)); // BUG #6
    AutoCrossingPolySegment10.Reverse();

    ASSERT_TRUE(AutoCrossingPolySegment11.AutoCrosses());
    ASSERT_EQ(4, AutoCrossingPolySegment11.AutoIntersect(&DumPoints));
    AutoCrossingPolySegment11.Reverse();
    ASSERT_TRUE(AutoCrossingPolySegment11.AutoCrosses());
    ASSERT_EQ(4, AutoCrossingPolySegment11.AutoIntersect(&DumPoints));
    AutoCrossingPolySegment11.Reverse();

    ASSERT_TRUE(AutoCrossingPolySegment12.AutoCrosses());
    //    ASSERT_EQ(5, AutoCrossingPolySegment12.AutoIntersect(&DumPoints)); // BUG #6
    AutoCrossingPolySegment12.Reverse();
    ASSERT_TRUE(AutoCrossingPolySegment12.AutoCrosses());
    //   ASSERT_EQ(5, AutoCrossingPolySegment12.AutoIntersect(&DumPoints)); // BUG #6
    AutoCrossingPolySegment12.Reverse();

    ASSERT_TRUE(AutoCrossingPolySegment13.AutoCrosses());
    //    ASSERT_EQ(5, AutoCrossingPolySegment13.AutoIntersect(&DumPoints)); // BUG #6
    AutoCrossingPolySegment13.Reverse();
    ASSERT_TRUE(AutoCrossingPolySegment13.AutoCrosses());
    //    ASSERT_EQ(5, AutoCrossingPolySegment13.AutoIntersect(&DumPoints)); // BUG #6
    AutoCrossingPolySegment13.Reverse();

    #endif

    }

//==================================================================================
// SortPointsAccordingToRelativePosition(HGF2DLocationCollection* pio_pListOfPointsOnLinear) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, SortPointsAccordingToRelativePositionTest2)
    {

    // Create a list of points unordered
    HGF2DLocationCollection MyListOfPoints;
    MyListOfPoints.push_back(PolySegment1.CalculateRelativePoint(1.0));
    MyListOfPoints.push_back(PolySegment1.CalculateRelativePoint(0.0));
    MyListOfPoints.push_back(PolySegment1.CalculateRelativePoint(0.5));

    PolySegment1.SortPointsAccordingToRelativePosition(&MyListOfPoints);

    ASSERT_EQ(3, MyListOfPoints.size());
    HGF2DLocationCollection::iterator Itr;
    Itr = MyListOfPoints.begin();
    ASSERT_NEAR(Itr->GetX(), PolySegment1.CalculateRelativePoint(0.0).GetX(), MYEPSILON); 
    ASSERT_NEAR(Itr->GetY(), PolySegment1.CalculateRelativePoint(0.0).GetY(), MYEPSILON);
    ++Itr;
    ASSERT_DOUBLE_EQ(Itr->GetX(), PolySegment1.CalculateRelativePoint(0.5).GetX());
    ASSERT_DOUBLE_EQ(Itr->GetY(), PolySegment1.CalculateRelativePoint(0.5).GetY());
    ++Itr;
    ASSERT_DOUBLE_EQ(Itr->GetX(), PolySegment1.CalculateRelativePoint(1.0).GetX());
    ASSERT_DOUBLE_EQ(Itr->GetY(), PolySegment1.CalculateRelativePoint(1.0).GetY());

    }

//==================================================================================
// Closest point calculation test
// CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CalculateClosestPointTest2)
    {

    // Test with PolySegment 1
    ASSERT_DOUBLE_EQ(21.100000000000, PolySegment1.CalculateClosestPoint(PolySegmentClosePoint1A).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000, PolySegment1.CalculateClosestPoint(PolySegmentClosePoint1A).GetY());
    ASSERT_NEAR(0.0, PolySegment1.CalculateClosestPoint(PolySegmentClosePoint1B).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, PolySegment1.CalculateClosestPoint(PolySegmentClosePoint1B).GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.000000000000, PolySegment1.CalculateClosestPoint(PolySegmentClosePoint1C).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000, PolySegment1.CalculateClosestPoint(PolySegmentClosePoint1C).GetY());
    ASSERT_DOUBLE_EQ(30.000000000000, PolySegment1.CalculateClosestPoint(PolySegmentClosePoint1D).GetX());
    ASSERT_DOUBLE_EQ(5.1000000000000, PolySegment1.CalculateClosestPoint(PolySegmentClosePoint1D).GetY());
    ASSERT_DOUBLE_EQ(17.928932000000, PolySegment1.CalculateClosestPoint(PolySegmentCloseMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000, PolySegment1.CalculateClosestPoint(PolySegmentCloseMidPoint1).GetY());

    // Test with an empty PolySegment
    ASSERT_NEAR(0.0, EmptyPolySegment.CalculateClosestPoint(PolySegmentClosePoint1A).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, EmptyPolySegment.CalculateClosestPoint(PolySegmentClosePoint1A).GetY(), MYEPSILON);

    // Tests with special points
    ASSERT_DOUBLE_EQ(30.000000000000000, PolySegment1.CalculateClosestPoint(VeryFarPoint).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, PolySegment1.CalculateClosestPoint(VeryFarPoint).GetY());
    ASSERT_DOUBLE_EQ(17.928932188135001, PolySegment1.CalculateClosestPoint(PolySegmentMidPoint1).GetX());
    ASSERT_DOUBLE_EQ(10.000000000000000, PolySegment1.CalculateClosestPoint(PolySegmentMidPoint1).GetY());
    ASSERT_NEAR(0.0, PolySegment1.CalculateClosestPoint(PolySegment1.GetStartPoint()).GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, PolySegment1.CalculateClosestPoint(PolySegment1.GetStartPoint()).GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(29.999999899999999, PolySegment1.CalculateClosestPoint(PolySegment1.GetEndPoint()).GetX());
    ASSERT_NEAR(0.0, PolySegment1.CalculateClosestPoint(PolySegment1.GetEndPoint()).GetY(), MYEPSILON);

    }

//==================================================================================
// Intersection test (with other complex PolySegments only)
// Intersect(const HVE2DVector& pi_rVector, HGF2DLocationCollection* po_pCrossPoints) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, IntersectTest2)
    {

    HGF2DLocationCollection   DumPoints;

    // Test with extent disjoint PolySegments
    ASSERT_EQ(0,  PolySegment1.Intersect(DisjointPolySegment11, &DumPoints));

    // Test with disjoint but touching by a side
    ASSERT_EQ(0,  PolySegment1.Intersect(ContiguousExtentPolySegment11, &DumPoints));

    // Test with disjoint but touching by a tip but not linked
    ASSERT_EQ(0,  PolySegment1.Intersect(FlirtingExtentPolySegment11, &DumPoints));

    // Test with disjoint but touching by a tip segments linked
    ASSERT_EQ(0,  PolySegment1.Intersect(FlirtingExtentLinkedPolySegment11, &DumPoints));

    // Tests with connected PolySegments
    // At start point...
    ASSERT_EQ(0,  PolySegment1.Intersect(ConnectedPolySegment11, &DumPoints));
    ASSERT_EQ(0,  PolySegment1.Intersect(ConnectingPolySegment11, &DumPoints));

    // At end point ...
    ASSERT_EQ(0,  PolySegment1.Intersect(ConnectedPolySegment1A1, &DumPoints));
    ASSERT_EQ(0,  PolySegment1.Intersect(ConnectingPolySegment1A1, &DumPoints));

    // Tests with linked segments
    ASSERT_EQ(0,  PolySegment1.Intersect(LinkedPolySegment11, &DumPoints));
    ASSERT_EQ(0,  PolySegment1.Intersect(LinkedPolySegment1A1, &DumPoints));

    // Tests with EPSILON sized container
    ASSERT_EQ(0,  PolySegment3.Intersect(PolySegment1, &DumPoints));

    // Special cases
    ASSERT_EQ(1, PolySegment1.Intersect(ComplexPolySegmentCase1, &DumPoints));
    ASSERT_DOUBLE_EQ(25.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());

    DumPoints.clear();
    ASSERT_EQ(1, PolySegment1.Intersect(ComplexPolySegmentCase2, &DumPoints));
    ASSERT_DOUBLE_EQ(28.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());

    DumPoints.clear();
    ASSERT_EQ(1, PolySegment1.Intersect(ComplexPolySegmentCase3, &DumPoints));
    ASSERT_DOUBLE_EQ(20.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());

    DumPoints.clear();
    ASSERT_EQ(0,  PolySegment1.Intersect(ComplexPolySegmentCase4, &DumPoints));

    DumPoints.clear();
    ASSERT_EQ(1, PolySegment1.Intersect(ComplexPolySegmentCase5, &DumPoints));
    ASSERT_DOUBLE_EQ(30.0, DumPoints[0].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[0].GetY());

    DumPoints.clear();
    ASSERT_EQ(0,  PolySegment1.Intersect(ComplexPolySegmentCase5A, &DumPoints));

    ASSERT_EQ(0,  PolySegment1.Intersect(ComplexPolySegmentCase6, &DumPoints));
    ASSERT_EQ(0,  PolySegment1.Intersect(ComplexPolySegmentCase7, &DumPoints));

    // Test with a NULL segment
    ASSERT_EQ(0,  EmptyPolySegment.Intersect(PolySegment1, &DumPoints));

    }

//==================================================================================
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
//==================================================================================
TEST_F (HVE2DPolySegmentTester, ContiguousnessTest2)
    {

    HGF2DLocationCollection     DumPoints;
    HGF2DLocation   FirstDumPoint;
    HGF2DLocation   SecondDumPoint;

    // Test with contiguous PolySegments
    ASSERT_TRUE(PolySegment1.AreContiguous(ComplexPolySegmentCase6));

    ASSERT_TRUE(PolySegment1.AreContiguousAt(ComplexPolySegmentCase6, PolySegmentMidPoint1));

    ASSERT_EQ(2, PolySegment1.ObtainContiguousnessPoints(ComplexPolySegmentCase6, &DumPoints));
    ASSERT_NEAR(0.0, DumPoints[0].GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, DumPoints[0].GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, DumPoints[1].GetX());
    ASSERT_DOUBLE_EQ(10.0, DumPoints[1].GetY());

    PolySegment1.ObtainContiguousnessPointsAt(ComplexPolySegmentCase6, PolySegmentMidPoint1, &FirstDumPoint, &SecondDumPoint);
    ASSERT_NEAR(0.0, FirstDumPoint.GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, FirstDumPoint.GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, SecondDumPoint.GetX());
    ASSERT_DOUBLE_EQ(10.0, SecondDumPoint.GetY());

    // Test with non contiguous PolySegments
    ASSERT_FALSE(PolySegment1.AreContiguous(ComplexPolySegmentCase1));

    }

//==================================================================================
// Cloning tests
// Clone() const;
// AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CloningTest2)
    {

    //General Clone Test
    HFCPtr<HVE2DPolySegment> pClone = (HVE2DPolySegment*)PolySegment1.Clone();
    ASSERT_EQ(pWorld, pClone->GetCoordSys());

    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(0.0, 0.0, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(30.0, 10.0, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(30.0, 10.0, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(30.0, 5.0 - MYEPSILON, pWorld))); 
    ASSERT_TRUE(pClone->IsPointOn(HGF2DLocation(30.0 - MYEPSILON, 0.0, pWorld))); 

    // Test with the same coordinate system
    HFCPtr<HVE2DPolySegment> pClone3 = (HVE2DPolySegment*) PolySegment1.AllocateCopyInCoordSys(pWorld);
    ASSERT_EQ(pWorld, pClone3->GetCoordSys());

    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(0.0, 0.0, pWorld))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(10.0, 10.0, pWorld))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(20.0, 10.0, pWorld))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(30.0, 10.0, pWorld))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(30.0, 10.0, pWorld))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(30.0, 5.0 - MYEPSILON, pWorld))); 
    ASSERT_TRUE(pClone3->IsPointOn(HGF2DLocation(30.0 - MYEPSILON, 0.0, pWorld))); 

    // Test with a translation between systems
    Translation = HGF2DDisplacement (10.0, 10.0);
    HGF2DTranslation myTranslation(Translation);
    HFCPtr<HGF2DCoordSys> pWorldTranslation = new HGF2DCoordSys(myTranslation, pWorld);

    HFCPtr<HVE2DPolySegment> pClone5 = (HVE2DPolySegment*) PolySegment1.AllocateCopyInCoordSys(pWorldTranslation);
    ASSERT_EQ(pWorldTranslation, pClone5->GetCoordSys());

    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(-10.0, -10.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(10.0, 0.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(20.0, 0.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(20.0, 0.0, pWorldTranslation))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(20.0, -5.0 - MYEPSILON, pWorldTranslation))); 
    ASSERT_TRUE(pClone5->IsPointOn(HGF2DLocation(20.0 - MYEPSILON, -10.0, pWorldTranslation))); 

    // Test with a stretch between systems
    HGF2DStretch myStretch;
    myStretch.SetTranslation(Translation);
    myStretch.SetXScaling(0.5);
    myStretch.SetYScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldStretch = new HGF2DCoordSys(myStretch, pWorld);

    HFCPtr<HVE2DPolySegment> pClone6 = (HVE2DPolySegment*) PolySegment1.AllocateCopyInCoordSys(pWorldStretch);
    ASSERT_EQ(pWorldStretch, pClone6->GetCoordSys());

    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(-20.0, -20.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(20.0, 0.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(40.0, 0.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(40.0, 0.0, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(40.0, -10.0 - MYEPSILON, pWorldStretch))); 
    ASSERT_TRUE(pClone6->IsPointOn(HGF2DLocation(40.0 - MYEPSILON, -20.0, pWorldStretch))); 

    // Test with a similitude between systems
    HGF2DSimilitude mySimilitude;
    mySimilitude.SetRotation(PI);
    mySimilitude.SetScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldSimilitude = new HGF2DCoordSys(mySimilitude, pWorld);

    HFCPtr<HVE2DPolySegment> pClone7 = (HVE2DPolySegment*) PolySegment1.AllocateCopyInCoordSys(pWorldSimilitude);
    ASSERT_EQ(pWorldSimilitude, pClone7->GetCoordSys());

    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-20.0, -20.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-40.0, -20.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-60.0, -20.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-60.0, -20.0, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-60.0, -10.0 - MYEPSILON, pWorldSimilitude))); 
    ASSERT_TRUE(pClone7->IsPointOn(HGF2DLocation(-60.0 + 2 * MYEPSILON, 0.0, pWorldSimilitude))); 

    // Test with a affine between systems
    HGF2DAffine myAffine;
    myAffine.SetTranslation(Translation);
    myAffine.SetRotation(PI);
    myAffine.SetXScaling(0.5);
    myAffine.SetYScaling(0.5);
    HFCPtr<HGF2DCoordSys> pWorldAffine = new HGF2DCoordSys(myAffine, pWorld);

    HFCPtr<HVE2DPolySegment> pClone8 = (HVE2DPolySegment*) PolySegment1.AllocateCopyInCoordSys(pWorldAffine);
    ASSERT_EQ(pWorldAffine, pClone8->GetCoordSys());

    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(20.0, 20.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(0.0, 0.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-20.0, 0.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-40.0, 0.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-40.0, 0.0, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-40.0, 10.0 - MYEPSILON, pWorldAffine))); 
    ASSERT_TRUE(pClone8->IsPointOn(HGF2DLocation(-40.0 + 2 * MYEPSILON, 20.0, pWorldAffine))); 

    }

//==================================================================================
// Interaction info with other segment
// Crosses(const HVE2DVector& pi_rVector) const;
// AreAdjacent(const HVE2DVector& pi_rVector) const;
// IsPointOn(const HGF2DLocation& pi_rTestPoint) const;
// IsPointOnSCS(const HGF2DLocation& pi_rTestPoint) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, InteractionTest2)
    {

    // Tests with a vertical segment
    ASSERT_TRUE(PolySegment1.Crosses(ComplexPolySegmentCase1));
    ASSERT_FALSE(PolySegment1.AreAdjacent(ComplexPolySegmentCase1));

    ASSERT_TRUE(PolySegment1.Crosses(ComplexPolySegmentCase2));
    ASSERT_TRUE(PolySegment1.AreAdjacent(ComplexPolySegmentCase2));

    ASSERT_TRUE(PolySegment1.Crosses(ComplexPolySegmentCase3));
    ASSERT_FALSE(PolySegment1.AreAdjacent(ComplexPolySegmentCase3));

    ASSERT_FALSE(PolySegment1.Crosses(ComplexPolySegmentCase4));
    ASSERT_TRUE(PolySegment1.AreAdjacent(ComplexPolySegmentCase4));

    ASSERT_TRUE(PolySegment1.Crosses(ComplexPolySegmentCase5));
    ASSERT_TRUE(PolySegment1.AreAdjacent(ComplexPolySegmentCase5));

    ASSERT_FALSE(PolySegment1.Crosses(ComplexPolySegmentCase6));
    ASSERT_TRUE(PolySegment1.AreAdjacent(ComplexPolySegmentCase6));

    ASSERT_FALSE(PolySegment1.Crosses(ComplexPolySegmentCase7));
    ASSERT_FALSE(PolySegment1.AreAdjacent(ComplexPolySegmentCase7));

    ASSERT_TRUE(PolySegment1.IsPointOn(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(PolySegment1.IsPointOn(HGF2DLocation(0.1+1.1*MYEPSILON, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(PolySegment1.IsPointOn(PolySegmentMidPoint1-HGF2DDisplacement(0.0, 1.1*MYEPSILON)));
    ASSERT_FALSE(PolySegment1.IsPointOn(PolySegmentMidPoint1-HGF2DDisplacement(0.0, -1.1*MYEPSILON)));
    ASSERT_TRUE(PolySegment1.IsPointOn(PolySegmentMidPoint1-HGF2DDisplacement(0.0, 0.9*MYEPSILON)));
    ASSERT_TRUE(PolySegment1.IsPointOn(PolySegmentMidPoint1-HGF2DDisplacement(0.0, -0.9*MYEPSILON)));
    ASSERT_TRUE(PolySegment1.IsPointOn(PolySegment1.GetStartPoint()));
    ASSERT_TRUE(PolySegment1.IsPointOn(PolySegment1.GetEndPoint()));
    ASSERT_TRUE(PolySegment1.IsPointOn(PolySegmentMidPoint1));

    ASSERT_FALSE(PolySegment1.IsPointOn(PolySegment1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(PolySegment1.IsPointOn(PolySegment1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(PolySegment1.IsPointOn(PolySegmentMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    ASSERT_TRUE(PolySegment1.IsPointOnSCS(HGF2DLocation(10.0, 10.0, pWorld)));
    ASSERT_FALSE(PolySegment1.IsPointOnSCS(HGF2DLocation(0.1+1.1*MYEPSILON, 0.1-1.1*MYEPSILON, pWorld)));
    ASSERT_FALSE(PolySegment1.IsPointOnSCS(PolySegmentMidPoint1-HGF2DDisplacement(0.0, 1.1*MYEPSILON)));
    ASSERT_FALSE(PolySegment1.IsPointOnSCS(PolySegmentMidPoint1-HGF2DDisplacement(0.0, -1.1*MYEPSILON)));
    ASSERT_TRUE(PolySegment1.IsPointOnSCS(PolySegmentMidPoint1-HGF2DDisplacement(0.0, 0.9*MYEPSILON)));
    ASSERT_TRUE(PolySegment1.IsPointOnSCS(PolySegmentMidPoint1-HGF2DDisplacement(0.0, -0.9*MYEPSILON)));
    ASSERT_TRUE(PolySegment1.IsPointOnSCS(PolySegment1.GetStartPoint()));
    ASSERT_TRUE(PolySegment1.IsPointOnSCS(PolySegment1.GetEndPoint()));
    ASSERT_TRUE(PolySegment1.IsPointOnSCS(PolySegmentMidPoint1));

    ASSERT_FALSE(PolySegment1.IsPointOnSCS(PolySegment1.GetStartPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_FALSE(PolySegment1.IsPointOnSCS(PolySegment1.GetEndPoint(), HVE2DVector::EXCLUDE_EXTREMITIES));
    ASSERT_TRUE(PolySegment1.IsPointOnSCS(PolySegmentMidPoint1, HVE2DVector::EXCLUDE_EXTREMITIES));

    }

//==================================================================================
// Bearing calculation tests
// CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
//                  HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
// CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
//                              HVE2DVector::ArbitraryDirection pi_Direction = HVE2DVector::BETA) const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, BearingTest2)
    {

    // Obtain bearing ALPHA of a vertical segment
    ASSERT_DOUBLE_EQ(-2.3561944901923448, PolySegment1.CalculateBearing(PolySegment1Point0d0, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(0.78539816339744828, PolySegment1.CalculateBearing(PolySegment1Point0d0, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(-2.3561944901923448, PolySegment1.CalculateBearing(PolySegment1Point0d1, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(0.78539816339744828, PolySegment1.CalculateBearing(PolySegment1Point0d1, HVE2DVector::BETA).GetAngle());
    ASSERT_DOUBLE_EQ(3.14159265358979310, PolySegment1.CalculateBearing(PolySegment1Point0d5, HVE2DVector::ALPHA).GetAngle());
    ASSERT_NEAR(0.0, PolySegment1.CalculateBearing(PolySegment1Point0d5, HVE2DVector::BETA).GetAngle(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.57079630679489600, PolySegment1.CalculateBearing(PolySegment1Point1d0, HVE2DVector::ALPHA).GetAngle());
    ASSERT_DOUBLE_EQ(-1.5707963467948973, PolySegment1.CalculateBearing(PolySegment1Point1d0, HVE2DVector::BETA).GetAngle());

    // ANGULAR ACCELERATION
    // Obtain angular acceleration ALPHA of a vertical segment
    ASSERT_NEAR(0.0, PolySegment1.CalculateAngularAcceleration(PolySegment1Point0d0, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, PolySegment1.CalculateAngularAcceleration(PolySegment1Point0d0, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, PolySegment1.CalculateAngularAcceleration(PolySegment1Point0d1, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, PolySegment1.CalculateAngularAcceleration(PolySegment1Point0d1, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, PolySegment1.CalculateAngularAcceleration(PolySegment1Point0d5, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, PolySegment1.CalculateAngularAcceleration(PolySegment1Point0d5, HVE2DVector::BETA), MYEPSILON);
    ASSERT_NEAR(0.0, PolySegment1.CalculateAngularAcceleration(PolySegment1Point1d0, HVE2DVector::ALPHA), MYEPSILON);
    ASSERT_NEAR(0.0, PolySegment1.CalculateAngularAcceleration(PolySegment1Point1d0, HVE2DVector::BETA), MYEPSILON);

    }

//==================================================================================
// Extent calculation test
// GetExtent() const;
//==================================================================================
TEST_F (HVE2DPolySegmentTester, GetExtentTest2)
    {

    // Obtain extent of PolySegment 1
    ASSERT_NEAR(0.0, PolySegment1.GetExtent().GetXMin(), MYEPSILON);
    ASSERT_NEAR(0.0, PolySegment1.GetExtent().GetYMin(), MYEPSILON);
    ASSERT_DOUBLE_EQ(30.0, PolySegment1.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(10.0, PolySegment1.GetExtent().GetYMax());

    // Obtain extent of PolySegment 2
    ASSERT_DOUBLE_EQ(-10.0, PolySegment2.GetExtent().GetXMin());
    ASSERT_DOUBLE_EQ(-10.0, PolySegment2.GetExtent().GetYMin());
    ASSERT_DOUBLE_EQ(24.00, PolySegment2.GetExtent().GetXMax());
    ASSERT_DOUBLE_EQ(15.00, PolySegment2.GetExtent().GetYMax());

    // Obtain extent of an epsilon container
    ASSERT_NEAR(0.0, PolySegment3.GetExtent().GetXMin(), MYEPSILON);
    ASSERT_NEAR(0.0, PolySegment3.GetExtent().GetYMin(), MYEPSILON);
    ASSERT_NEAR(0.0, PolySegment3.GetExtent().GetXMax(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.9999999999999999E-7, PolySegment3.GetExtent().GetYMax());

    // Obtain extent of an empty PolySegment
    HGF2DExtent EmptyExtent(EmptyPolySegment.GetExtent());
    ASSERT_FALSE(EmptyExtent.IsDefined());

    }

//==================================================================================
// Test which previously failed
//==================================================================================
TEST_F (HVE2DPolySegmentTester, ModifyShapeTest2)
    {

    HGF2DLocationCollection     CCPoints;
        
    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(10.0, 20.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(20.0, 20.0, pWorld));

    HVE2DPolySegment  AddPolySegment2(pWorld);
    AddPolySegment2.AppendPoint(HGF2DLocation(5.0, 10.0, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(15.0, 15.0, pWorld));

    ASSERT_EQ(1, AddPolySegment1.Intersect(AddPolySegment2, &CCPoints));
    ASSERT_TRUE(AddPolySegment1.Crosses(AddPolySegment2));

    CCPoints.clear();
    HVE2DPolySegment  AddPolySegment3(pWorld);
    AddPolySegment3.AppendPoint(HGF2DLocation(12.0, 12.0, pWorld));
    AddPolySegment3.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AddPolySegment3.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));
    AddPolySegment3.AppendPoint(HGF2DLocation(15.0, 15.0, pWorld));

    ASSERT_EQ(0,  AddPolySegment1.Intersect(AddPolySegment3, &CCPoints));
    ASSERT_FALSE(AddPolySegment1.Crosses(AddPolySegment3));

    }

//==================================================================================
// Additional test for possible new CrossesPolysegmentSCS method
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CrossesPolysegmentSCSTest2)
    {

    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(30.0, 10.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(30.0, 20.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(20.0, 20.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(20.0, 30.0, pWorld));

    HVE2DPolySegment  AddPolySegment2(pWorld);
    AddPolySegment2.AppendPoint(HGF2DLocation(20.0, 5.0, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(30.0, 10.0, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(30.0, 20.0, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(25.0, 25.0, pWorld));

    ASSERT_FALSE(AddPolySegment1.Crosses(AddPolySegment2));
   
    }

//==================================================================================
// Additional test for possible new CrossesPolysegmentSCS method
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CrossesPolysegmentSCS2Test2)
    {

    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(0.0, 10.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(10.0, 0.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(20.0, 0.0, pWorld));

    HVE2DPolySegment  AddPolySegment2(pWorld);
    AddPolySegment2.AppendPoint(HGF2DLocation(0.0, 5.0, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(20.0, 5.0, pWorld));

    HVE2DPolySegment  AddPolySegment3(pWorld);
    AddPolySegment3.AppendPoint(HGF2DLocation(0.0, 5.0, pWorld));
    AddPolySegment3.AppendPoint(HGF2DLocation(10.0, 5.0, pWorld));
    AddPolySegment3.AppendPoint(HGF2DLocation(20.0, 5.0, pWorld));

    HVE2DPolySegment  AddPolySegment4(pWorld);
    AddPolySegment4.AppendPoint(HGF2DLocation(0.0, 10.0, pWorld));
    AddPolySegment4.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AddPolySegment4.AppendPoint(HGF2DLocation(10.0, 0.0, pWorld));
    AddPolySegment4.AppendPoint(HGF2DLocation(10.0, 5.0, pWorld));
    AddPolySegment4.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AddPolySegment4.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));
    AddPolySegment4.AppendPoint(HGF2DLocation(20.0, 0.0, pWorld));

    ASSERT_TRUE(AddPolySegment1.Crosses(AddPolySegment2));
    ASSERT_TRUE(AddPolySegment1.Crosses(AddPolySegment3));
    ASSERT_TRUE(AddPolySegment4.Crosses(AddPolySegment3));
      
    }

//==================================================================================
// Additional test for possible new CrossesPolysegmentSCS method
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CrossesPolysegmentSCS3Test2)
    {

    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(0.0, 20.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(20.0, 20.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(15.0, 10.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(20.0, 0.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));

    HVE2DPolySegment  AddPolySegment2(pWorld);
    AddPolySegment2.AppendPoint(HGF2DLocation(0.0 + 1e-9, 0.0 + 1e-9, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(0.0 + 1e-9, 20.0 + 1e-9, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(20.0 + 1e-9, 20.0 + 1e-9, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(25.0 + 1e-9, 10.0 + 1e-9, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(20.0 + 1e-9, 0.0 + 1e-9, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(0.0 + 1e-9, 0.0 + 1e-9, pWorld));

    ASSERT_FALSE(AddPolySegment1.Crosses(AddPolySegment2));
    
    }

//==================================================================================
// Additional test for possible new CrossesPolysegmentSCS method
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CrossesPolysegmentSCS4Test2)
    {
    
    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(0.0, 10.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(30.0, 20.0, pWorld));

    HVE2DPolySegment  AddPolySegment2(pWorld);
    AddPolySegment2.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(30.0, 10.0, pWorld));

    ASSERT_FALSE(AddPolySegment1.Crosses(AddPolySegment2));
       
    }

//==================================================================================
// Additional test for possible new CrossesPolysegmentSCS method
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CrossesPolysegmentSCS5Test2)
    {

    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(0.0, 0.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(30.0, 10.0, pWorld));

    HVE2DPolySegment  AddPolySegment2(pWorld);
    AddPolySegment2.AppendPoint(HGF2DLocation(0.0, 10.0, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(30.0, 20.0, pWorld));

    ASSERT_FALSE(AddPolySegment1.Crosses(AddPolySegment2));

    }

//==================================================================================
// Test crossing at start-end point
//==================================================================================
TEST_F (HVE2DPolySegmentTester, CrossesTest2)
    {
    
    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(10.0, 20.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(20.0, 20.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(20.0, 10.0, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(10.0, 10.0, pWorld));

    HVE2DPolySegment  AddPolySegment2(pWorld);
    AddPolySegment2.AppendPoint(HGF2DLocation(5.0, 5.0, pWorld));
    AddPolySegment2.AppendPoint(HGF2DLocation(15.0, 15.0, pWorld));

    ASSERT_FALSE(AddPolySegment1.Crosses(AddPolySegment2));
   
    }

//==================================================================================
// Test that failed Aug 14, 2000
//==================================================================================
TEST_F (HVE2DPolySegmentTester, IsAutoContiguousTest2)
    {

    HVE2DPolySegment  AddPolySegment1(pWorld);
    AddPolySegment1.AppendPoint(HGF2DLocation(249.984375000170640 , -0.000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(249.984375000170640 , 0.000000594766647, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(0.000000000000000 , 0.000000595476536, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(0.000000000000000 , 187.500000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(250.000000000000000 , 187.500000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(250.000000000000000 , 187.499999933650140, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(249.984374999393650 , 187.499999933650200, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(249.984375000608570 , 0.000000067755764, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(250.000000000000000 , 0.000000067755688, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(250.000000000000000 , -0.000000000000000, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(249.984375000170640 , -0.000000000000000, pWorld));

    ASSERT_TRUE(AddPolySegment1.IsAutoContiguous());
      
    }

//==================================================================================
// Test that failed Nov 17, 2000
//==================================================================================
TEST_F (HVE2DPolySegmentTester, IsAutoContiguous2Test2)
    {

    HVE2DPolySegment  AddPolySegment1(pWorld);

    AddPolySegment1.AppendPoint(HGF2DLocation(9.543589853241450 , 0.008890985395661, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(18.699148524608169 , 0.008891351467961, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(69.791914208400826 , 0.008886207735458, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(69.791914181781692 , 0.008885749281632, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(9.543589853241450 , 0.008890985395661, pWorld));

    AddPolySegment1.SetAutoToleranceActive(false);
    AddPolySegment1.SetTolerance(0.001);

    ASSERT_TRUE(AddPolySegment1.IsAutoContiguous());

    HVE2DPolySegment OriginalPolySegment(AddPolySegment1);

    AddPolySegment1.RemoveAutoContiguousNeedles(true);

    HVE2DPolySegment NewPolySegment(AddPolySegment1);
    NewPolySegment.RemoveAutoContiguousNeedles(true);
    ASSERT_FALSE(AddPolySegment1.IsAutoContiguous() || NewPolySegment.IsAutoContiguous());

    }

//==================================================================================
// Test that failed Nov 17, 2000
//==================================================================================
TEST_F (HVE2DPolySegmentTester, IsAutoContiguous3Test2)
    {

    HVE2DPolySegment  AddPolySegment1(pWorld);

    AddPolySegment1.AppendPoint(HGF2DLocation(238.000903774127350 , 0.006040800313726, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(238.000899078842170 , 35.162291219526061, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(238.000905904020160 , 63.023962408213322, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(238.000904373795270 , 0.172818264379531, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(238.000903915592720 , 0.006040760326914, pWorld));
    AddPolySegment1.AppendPoint(HGF2DLocation(238.000903774127350 , 0.006040800313726, pWorld));

    AddPolySegment1.SetAutoToleranceActive(false);
    AddPolySegment1.SetTolerance(0.00001);

    ASSERT_TRUE(AddPolySegment1.IsAutoContiguous());

    HVE2DPolySegment OriginalPolySegment(AddPolySegment1);

    AddPolySegment1.RemoveAutoContiguousNeedles(true);

    HVE2DPolySegment NewPolySegment(AddPolySegment1);
    NewPolySegment.RemoveAutoContiguousNeedles(true);
    ASSERT_FALSE(AddPolySegment1.IsAutoContiguous() || NewPolySegment.IsAutoContiguous());

    }