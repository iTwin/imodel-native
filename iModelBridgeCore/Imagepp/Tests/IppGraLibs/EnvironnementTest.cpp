//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/EnvironnementTest.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

 #include "../imagepptestpch.h"
 #include "EnvironnementTest.h"
 
 EnvironnementTest::EnvironnementTest() 
    { 

    // TRANSFORMATION MODELS
    Translation = HGF2DDisplacement(10.0, 30.48);
    Rotation = (PI/4);
    ScalingX = 1.00001;
    ScalingY = 2.00001;
    Anortho = -0.000001;
    Affine1 = HGF2DAffine(Translation, Rotation, ScalingX, ScalingY, Anortho);

    // COORDINATE SYSTEMS
    pWorld = new HGF2DCoordSys();
    pSys1 = new HGF2DCoordSys(Affine1, pWorld);
    pNoLinear = new HGF2DCoordSys(Projective, pSys1);

    // VERTICAL SEGMENT
    VerticalSegment1 = HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(0.1, 10.1, pWorld));
    VerticalSegment2 = HVE2DSegment(HGF2DLocation(0.1, 10.1, pWorld), HGF2DLocation(0.1, 0.1, pWorld));
    VerticalSegment3 = HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(0.1, 0.1 + MYEPSILON, pWorld));
    VerticalSegment4 = HVE2DSegment(HGF2DLocation(0.1 + MYEPSILON, 0.1, pWorld), HGF2DLocation(0.1+MYEPSILON, 10.1, pWorld));
    VerticalSegment5 = HVE2DSegment(HGF2DLocation(-10.0, -10.0, pWorld), HGF2DLocation(-10.0, 0.0, pWorld));
    CloseVerticalSegment1 = HVE2DSegment(HGF2DLocation(0.1+MYEPSILON, 0.1, pWorld), HGF2DLocation(0.1, 10.1, pWorld));

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

    // HORIZONTAL SEGMENT
    HorizontalSegment1 = HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(10.1, 0.1, pWorld));
    HorizontalSegment2 = HVE2DSegment(HGF2DLocation(10.1, 0.1, pWorld), HGF2DLocation(0.1, 0.1, pWorld));
    HorizontalSegment3 = HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(0.1 + MYEPSILON, 0.1, pWorld));
    HorizontalSegment5 = HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(10.1 + MYEPSILON, 0.1, pWorld));

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
    HorizontalClosePoint3D = HGF2DLocation (0.1 + (MYEPSILON/2), 1000000.0, pWorld);
    HorizontalCloseMidPoint3 = HGF2DLocation(0.1 + (MYEPSILON/2), 0.0, pWorld);

    // MISC SEGMENTS
    MiscSegment1 = HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(10.1, 10.1, pWorld));
    MiscSegment2 = HVE2DSegment(HGF2DLocation(10.1, 10.1, pWorld), HGF2DLocation(0.1, 0.1, pWorld));
    MiscSegment3 = HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DDisplacement(HGFBearing(77.5 * PI/180), MYEPSILON));
    MiscSegment4 = HVE2DSegment(HGF2DLocation(0.2, 0.1, pWorld), HGF2DDisplacement((77.5), MYEPSILON));
    MiscSegment6 = HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(-9.9, 10.1, pWorld));
    MiscSegment7 = HVE2DSegment(HGF2DLocation(0.2, 0.0, pWorld), HGF2DLocation(-9.8, 10.0, pWorld));
    DisjointSegment1 = HVE2DSegment(HGF2DLocation(-0.1, -0.1, pWorld), HGF2DLocation(-10.1, -10.24, pWorld));
    ContiguousExtentSegment1 = HVE2DSegment(HGF2DLocation(10.1, 0.1, pWorld), HGF2DLocation(20.1, 10.1, pWorld));
    FlirtingExtentSegment1 = HVE2DSegment(HGF2DLocation(10.1, 0.1, pWorld), HGF2DLocation(20.1, -10.1, pWorld));
    FlirtingExtentLinkedSegment1 = HVE2DSegment(HGF2DLocation(10.1, 10.1, pWorld), HGF2DLocation(20.1, 0.1, pWorld));
    ParallelSegment1 = HVE2DSegment(HGF2DLocation(0.2, 0.1, pWorld), HGF2DLocation(10.2, 10.1, pWorld));
    LinkedParallelSegment1 = HVE2DSegment(HGF2DLocation(10.1, 10.1, pWorld), HGF2DLocation(20.1, 20.1, pWorld));
    NearParallelSegment1 = HVE2DSegment(HGF2DLocation(0.2, 0.1, pWorld), HGF2DLocation(10.2, 10.1+MYEPSILON, pWorld));
    CloseNearParallelSegment1 = HVE2DSegment(HGF2DLocation(0.1+MYEPSILON, 0.1, pWorld), HGF2DLocation(10.1+MYEPSILON, 10.1+MYEPSILON, pWorld));
    ConnectedSegment1 = HVE2DSegment(HGF2DLocation(0.2, 0.0, pWorld), HGF2DLocation(0.0, 0.2, pWorld));
    ConnectingSegment1 = HVE2DSegment(HGF2DLocation(10.2, 0.0, pWorld), HGF2DLocation(2.0, 2.0, pWorld));
    ConnectedSegment1A = HVE2DSegment(HGF2DLocation(20.2, 0.0, pWorld), HGF2DLocation(0.0, 20.2, pWorld));
    ConnectingSegment1A = HVE2DSegment(HGF2DLocation(2.0, 2.0, pWorld), HGF2DLocation(10.2, 0.0, pWorld));
    LinkedSegment1 = HVE2DSegment(HGF2DLocation(0.1, 0.1, pWorld), HGF2DLocation(10.0, 3.2, pWorld));
    LinkedSegment1A = HVE2DSegment(HGF2DLocation(10.1, 10.1, pWorld), HGF2DLocation(10.0, 3.2, pWorld));

    MiscSegment3A = HVE2DSegment(HGF2DLocation(0.1+MYEPSILON, 0.1, pWorld), HGF2DDisplacement(HGFBearing(77.5 * PI/180), MYEPSILON));

    Misc3Point0d0 = HGF2DLocation(0.1, 0.1, pWorld);
    Misc3Point0d1 = HGF2DLocation(0.1, 0.1, pWorld) + HGF2DDisplacement(HGFBearing(77.5 * PI/180), MYEPSILON * 0.1);
    Misc3Point0d5 = HGF2DLocation(0.1, 0.1, pWorld) + HGF2DDisplacement(HGFBearing(77.5* PI/180), MYEPSILON * 0.5);
    Misc3Point1d0 = HGF2DLocation(0.1, 0.1, pWorld) + HGF2DDisplacement(HGFBearing(77.5* PI/180), MYEPSILON);
    MiscMidPoint1 = HGF2DLocation(5.1, 5.1, pWorld);

    MiscMidPoint3 = HGF2DLocation(0.1, 0.1, pWorld)+ HGF2DDisplacement(HGFBearing(77.5), MYEPSILON / 2);
    MiscClosePoint3A = HGF2DLocation(0.0, 0.0, pWorld);
    MiscClosePoint3B = HGF2DLocation(0.0, 0.1 + (2*MYEPSILON/3), pWorld);
    MiscClosePoint3C = HGF2DLocation(0.2, 0.1 + (MYEPSILON/3), pWorld);
    MiscClosePoint3D = HGF2DLocation(1000000.0, 0.1 + (MYEPSILON/2), pWorld);
    MiscCloseMidPoint3 = HGF2DLocation(0.0, 0.1 + (MYEPSILON/2), pWorld);

    VeryFarAlignedPoint = HGF2DLocation(21E123, 21E123, pWorld);
    VeryFarAlignedNegativePoint = HGF2DLocation(-21E123, -21E123, pWorld);
    VeryFarNegativePoint = HGF2DLocation(-1E123, -21E123, pWorld);
    MidPoint = HGF2DLocation(5.1, 5.1, pWorld);

    MiscMidPoint6 = HGF2DLocation(-4.9, 5.1, pWorld);

    // LARGE SEGMENTS
    LargeSegment1 = HVE2DSegment(HGF2DLocation(-1E123, -21E123, pWorld), HGF2DLocation(9E123, 19E123, pWorld));
    LargeSegment2 =  HVE2DSegment(HGF2DLocation(9E123, 19E123, pWorld), HGF2DLocation(-1E123, -21E123, pWorld));
    ParallelLargeSegment1 = HVE2DSegment(HGF2DLocation(-1.000000001E123, -21E123, pWorld), HGF2DLocation(9.000000001E123, 19E123, pWorld));

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

    // POSITIVE SEGMENTS
    PositiveSegment1 = HVE2DSegment(HGF2DLocation(1E123, 21E123, pWorld), HGF2DLocation(11E123, 41E123, pWorld));
    PositiveSegment2 = HVE2DSegment(HGF2DLocation(11E123, 41E123, pWorld), HGF2DLocation(1E123, 21E123, pWorld));
    ParallelPositiveSegment1 = HVE2DSegment(HGF2DLocation(1.000001E123, 21E123, pWorld), HGF2DLocation(11.000001E123, 41E123, pWorld));

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

    // NEGATIVE SEGMENTS
    NegativeSegment1 = HVE2DSegment(HGF2DLocation(-1E123, -21E123, pWorld), HGF2DLocation(-11E123, -41E123, pWorld));
    NegativeSegment2 = HVE2DSegment(HGF2DLocation(-11E123, -41E123, pWorld), HGF2DLocation(-1E123, -21E123, pWorld));
    ParallelNegativeSegment1 = HVE2DSegment(HGF2DLocation(-1.000001E123, -21E123, pWorld), HGF2DLocation(-11.000001E123, -41E123, pWorld));

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

    // NULL SEGMENTS
    NullSegment1 = HVE2DSegment(pWorld);
    NullSegment2 = HVE2DSegment(pWorld);

    // Special SEGMENTS
    SegmentInvalid = HVE2DSegment(HGF2DLocation(0.0, 0.0, pNoLinear), HGF2DLocation(10.0, 10.0, pNoLinear));
    SegmentPSys1 = HVE2DSegment(HGF2DLocation(0.0, 0.0, pSys1), HGF2DLocation(10.0, 10.0, pSys1));

    //Identity
    EmptyLinear = HVE2DComplexLinear(pWorld);

     // Complex Linears
    Linear1Segment1 = HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    Linear1Segment2 = HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld));
    Linear1Segment3 = HVE2DSegment(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(30.0, 10.0, pWorld));
    Linear1Segment4 = HVE2DSegment(HGF2DLocation(30.0, 10.0, pWorld), HGF2DLocation(30.0, 5.0, pWorld));
    Linear1Segment5 = HVE2DSegment(HGF2DLocation(30.0, 5.0, pWorld), HGF2DLocation(30.0, 5.0-MYEPSILON, pWorld));
    Linear1Segment6 = HVE2DSegment(HGF2DLocation(30.0, 5.0-MYEPSILON, pWorld), HGF2DLocation(30.0-MYEPSILON, 0.0, pWorld));
    Linear1 = HVE2DComplexLinear(pWorld); // Open
    Linear1.AppendLinear(Linear1Segment1);
    Linear1.AppendLinear(Linear1Segment2);
    Linear1.AppendLinear(Linear1Segment3);
    Linear1.AppendLinear(Linear1Segment4);
    Linear1.AppendLinear(Linear1Segment5);
    Linear1.AppendLinear(Linear1Segment6);

    Linear1Point0d0 = HGF2DLocation(0.0, 0.0, pWorld);
    Linear1Point0d1 = HGF2DLocation(3.1213203435596, 3.1213203435596, pWorld);
    Linear1Point0d5 = HGF2DLocation(17.928932188135, 10.0, pWorld);
    Linear1Point1d0 = HGF2DLocation(30.0-MYEPSILON, 0.0, pWorld);
    LinearMidPoint1 = HGF2DLocation(17.928932188135, 10.0, pWorld);

    LinearClosePoint1A = HGF2DLocation(21.1, 10.1, pWorld);
    LinearClosePoint1B = HGF2DLocation(-10.0, 10.0, pWorld);
    LinearClosePoint1C = HGF2DLocation(30.0, 10.1, pWorld);
    LinearClosePoint1D = HGF2DLocation(32.1, 5.1, pWorld);
    LinearCloseMidPoint1 = HGF2DLocation(17.928932, 9.1, pWorld);

    Linear2Segment1 = HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(-10.0, -10.0, pWorld));
    Linear2Segment2 = HVE2DSegment(HGF2DLocation(-10.0, -10.0, pWorld), HGF2DLocation(18.0, 9.0, pWorld));
    Linear2Segment3 = HVE2DSegment(HGF2DLocation(18.0, 9.0, pWorld), HGF2DLocation(21.0, 0.0, pWorld));
    Linear2Segment4 = HVE2DSegment(HGF2DLocation(21.0, 0.0, pWorld), HGF2DLocation(24.0, 15.0, pWorld));
    Linear2Segment5 = HVE2DSegment(HGF2DLocation(24.0, 15.0, pWorld), HGF2DLocation(0.0, 15.0, pWorld));
    Linear2Segment6 = HVE2DSegment(HGF2DLocation(0.0, 15.0, pWorld), HGF2DLocation(0.0, 0.0, pWorld));
    Linear2 = HVE2DComplexLinear(pWorld);  // AutoClosed
    Linear2.AppendLinear(Linear2Segment1);
    Linear2.AppendLinear(Linear2Segment2);
    Linear2.AppendLinear(Linear2Segment3);
    Linear2.AppendLinear(Linear2Segment4);
    Linear2.AppendLinear(Linear2Segment5);
    Linear2.AppendLinear(Linear2Segment6);

    Linear2Point0d0 = HGF2DLocation(0.0, 0.0, pWorld);
    Linear2Point0d1 = HGF2DLocation(-7.9028994453177, -7.9028994453177, pWorld);
    Linear2Point0d5 = HGF2DLocation(20.498817144560, 1.5035485663202, pWorld);
    Linear2Point1d0 = HGF2DLocation(0.0, 0.0, pWorld);

    LinearMidPoint2 = HGF2DLocation(20.498817144560, 1.5035485663202, pWorld);

    LinearClosePoint2A = HGF2DLocation(21.1, 10.1, pWorld);
    LinearClosePoint2B = HGF2DLocation(21.1, 10.1, pWorld);
    LinearClosePoint2C = HGF2DLocation(21.1, 10.1, pWorld);
    LinearClosePoint2D = HGF2DLocation(21.1, 10.1, pWorld);
    LinearCloseMidPoint2 = HGF2DLocation(21.1, 10.1, pWorld);
   
    // epsilon size container
    Linear3Segment1 = HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(0.0-MYEPSILON, 0.0, pWorld));
    Linear3Segment2 = HVE2DSegment(HGF2DLocation(0.0-MYEPSILON, 0.0, pWorld), HGF2DLocation(0.0-MYEPSILON, 0.0+MYEPSILON, pWorld));
    Linear3Segment3 = HVE2DSegment(HGF2DLocation(0.0-MYEPSILON, 0.0+MYEPSILON, pWorld), HGF2DLocation(0.0, 0.0+MYEPSILON, pWorld));
    Linear3Segment4 = HVE2DSegment(HGF2DLocation(0.0, 0.0+MYEPSILON, pWorld), HGF2DLocation(0.0+MYEPSILON, 0.0+2*MYEPSILON, pWorld));
    Linear3Segment5 = HVE2DSegment(HGF2DLocation(0.0+MYEPSILON, 0.0+2*MYEPSILON, pWorld), HGF2DLocation(0.0+MYEPSILON, 0.0, pWorld));
    Linear3 = HVE2DComplexLinear(pWorld); // Open
    Linear3.AppendLinear(Linear3Segment1);
    Linear3.AppendLinear(Linear3Segment2);
    Linear3.AppendLinear(Linear3Segment3);
    Linear3.AppendLinear(Linear3Segment4);
    Linear3.AppendLinear(Linear3Segment5);

    Linear3Point0d0 = HGF2DLocation(0.0, 0.0, pWorld);
    Linear3Point0d1 = HGF2DLocation(-0.64142135623731 *MYEPSILON, 0.0, pWorld);
    Linear3Point0d5 = HGF2DLocation(0.14644660940673*MYEPSILON, 1.1464466094067*MYEPSILON, pWorld);
    Linear3Point1d0 = HGF2DLocation(0.0+MYEPSILON, 0.0, pWorld);

    LinearMidPoint3 = HGF2DLocation(1.4644660940673e-009, 1.1464466094067*MYEPSILON, pWorld);

    LinearClosePoint3A = HGF2DLocation(21.1, 10.1, pWorld);
    LinearClosePoint3B = HGF2DLocation(21.1, 10.1, pWorld);
    LinearClosePoint3C = HGF2DLocation(21.1, 10.1, pWorld);
    LinearClosePoint3D = HGF2DLocation(21.1, 10.1, pWorld);
    LinearCloseMidPoint3 = HGF2DLocation(21.1, 10.1, pWorld);


    // Used for intersection tests
    Case1Segment1 = HVE2DSegment(HGF2DLocation(5.0, 12.0, pWorld), HGF2DLocation(15.0, 15.0, pWorld));
    ComplexLinearCase1 = HVE2DComplexLinear(pWorld);
    ComplexLinearCase1.AppendLinear(Case1Segment1);

    Case2Segment1 = HVE2DSegment(HGF2DLocation(5.0, 5.0, pWorld), HGF2DLocation(10.0, 12.0, pWorld));
    Case2Segment2 = HVE2DSegment(HGF2DLocation(10.0, 12.0, pWorld), HGF2DLocation(10.0, 15.0, pWorld));
    Case2Segment3 = HVE2DSegment(HGF2DLocation(10.0, 15.0, pWorld), HGF2DLocation(15.0, 17.0, pWorld));
    ComplexLinearCase2 = HVE2DComplexLinear(pWorld);
    ComplexLinearCase2.AppendLinear(Case2Segment1);
    ComplexLinearCase2.AppendLinear(Case2Segment2);
    ComplexLinearCase2.AppendLinear(Case2Segment3);

    Case3Segment1 = HVE2DSegment(HGF2DLocation(5.0, 5.0, pWorld), HGF2DLocation(15.0, 15.0, pWorld));
    ComplexLinearCase3 = HVE2DComplexLinear(pWorld);
    ComplexLinearCase3.AppendLinear(Case3Segment1);

    Case4Segment1 = HVE2DSegment(HGF2DLocation(5.0, 10.0, pWorld), HGF2DLocation(35.0, 10.0, pWorld));
    ComplexLinearCase4 = HVE2DComplexLinear(pWorld);
    ComplexLinearCase4.AppendLinear(Case4Segment1);

    Case5Segment1 = HVE2DSegment(HGF2DLocation(5.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld));
    Case5Segment2 = HVE2DSegment(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(15.0, 15.0, pWorld));
    ComplexLinearCase5 = HVE2DComplexLinear(pWorld);
    ComplexLinearCase5.AppendLinear(Case5Segment1);
    ComplexLinearCase5.AppendLinear(Case5Segment2);

    Case5ASegment1 = HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld));
    Case5ASegment2 = HVE2DSegment(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(15.0, 15.0, pWorld));
    ComplexLinearCase5A = HVE2DComplexLinear(pWorld);
    ComplexLinearCase5A.AppendLinear(Case5ASegment1);
    ComplexLinearCase5A.AppendLinear(Case5ASegment2);

    Case6Segment1 = HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld));
    Case6Segment2 = HVE2DSegment(HGF2DLocation(20.0, 10.0, pWorld), HGF2DLocation(20.0, 20.0, pWorld));
    Case6Segment3 = HVE2DSegment(HGF2DLocation(20.0, 20.0, pWorld), HGF2DLocation(10.0, 20.0, pWorld));
    ComplexLinearCase6 = HVE2DComplexLinear(pWorld);
    ComplexLinearCase6.AppendLinear(Case6Segment1);
    ComplexLinearCase6.AppendLinear(Case6Segment2);
    ComplexLinearCase6.AppendLinear(Case6Segment3);

    Case7Segment1 = HVE2DSegment(HGF2DLocation(5.0, 5.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    ComplexLinearCase7 = HVE2DComplexLinear(pWorld);
    ComplexLinearCase7.AppendLinear(Case7Segment1);

    AutoCrossingLinear1Segment1 = HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingLinear1Segment2 = HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingLinear1Segment3 = HVE2DSegment(HGF2DLocation(15.0, 5.0, pWorld), HGF2DLocation(0.0, 5.0, pWorld));
    AutoCrossingLinear1Segment4 = HVE2DSegment(HGF2DLocation(0.0, 5.0, pWorld), HGF2DLocation(5.0, 0.0, pWorld));
    AutoCrossingLinear1Segment5 = HVE2DSegment(HGF2DLocation(5.0, 0.0, pWorld), HGF2DLocation(20.0, 20.0, pWorld));
    AutoCrossingLinear1 = HVE2DComplexLinear(pWorld);
    AutoCrossingLinear1.AppendLinear(AutoCrossingLinear1Segment1);
    AutoCrossingLinear1.AppendLinear(AutoCrossingLinear1Segment2);
    AutoCrossingLinear1.AppendLinear(AutoCrossingLinear1Segment3);
    AutoCrossingLinear1.AppendLinear(AutoCrossingLinear1Segment4);
    AutoCrossingLinear1.AppendLinear(AutoCrossingLinear1Segment5);

    AutoCrossingLinear2Segment1 = HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    AutoCrossingLinear2Segment2 = HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(15.0, 5.0, pWorld));
    AutoCrossingLinear2Segment3 = HVE2DSegment(HGF2DLocation(15.0, 5.0, pWorld), HGF2DLocation(10.0, 5.0, pWorld));
    AutoCrossingLinear2Segment4 = HVE2DSegment(HGF2DLocation(10.0, 5.0, pWorld), HGF2DLocation(10.0, 15.0, pWorld));
    AutoCrossingLinear2 = HVE2DComplexLinear(pWorld);
    AutoCrossingLinear2.AppendLinear(AutoCrossingLinear2Segment1);
    AutoCrossingLinear2.AppendLinear(AutoCrossingLinear2Segment2);
    AutoCrossingLinear2.AppendLinear(AutoCrossingLinear2Segment3);
    AutoCrossingLinear2.AppendLinear(AutoCrossingLinear2Segment4);

    AutoConnectingLinear1Segment1 = HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(10.0, 10.0, pWorld));
    AutoConnectingLinear1Segment2 = HVE2DSegment(HGF2DLocation(10.0, 10.0, pWorld), HGF2DLocation(15.0, 5.0, pWorld));
    AutoConnectingLinear1Segment3 = HVE2DSegment(HGF2DLocation(15.0, 5.0, pWorld), HGF2DLocation(10.0, 0.0, pWorld));
    AutoConnectingLinear1Segment4 = HVE2DSegment(HGF2DLocation(10.0, 0.0, pWorld), HGF2DLocation(-10.0, 0.0, pWorld));
    AutoConnectingLinear1 = HVE2DComplexLinear(pWorld);
    AutoConnectingLinear1.AppendLinear(AutoConnectingLinear1Segment1);
    AutoConnectingLinear1.AppendLinear(AutoConnectingLinear1Segment2);
    AutoConnectingLinear1.AppendLinear(AutoConnectingLinear1Segment3);
    AutoConnectingLinear1.AppendLinear(AutoConnectingLinear1Segment4);

    DisjointLinear1Segment1 = HVE2DSegment(HGF2DLocation(-10.0, -10.0, pWorld), HGF2DLocation(-20.0, -10.0, pWorld));
    DisjointLinear1Segment2 = HVE2DSegment(HGF2DLocation(-20.0, -10.0, pWorld), HGF2DLocation(-30.0, -10.0, pWorld));
    DisjointLinear1 = HVE2DComplexLinear(pWorld); // Open
    DisjointLinear1.AppendLinear(DisjointLinear1Segment1);
    DisjointLinear1.AppendLinear(DisjointLinear1Segment2);

    ContiguousExtentLinear1Segment1 = HVE2DSegment(HGF2DLocation(-10.0, 0.0, pWorld), HGF2DLocation(0.0, 10.0, pWorld));
    ContiguousExtentLinear1Segment2 = HVE2DSegment(HGF2DLocation(0.0, 10.0, pWorld), HGF2DLocation(-10.0, 10.0, pWorld));
    ContiguousExtentLinear1 = HVE2DComplexLinear(pWorld); // Open
    ContiguousExtentLinear1.AppendLinear(ContiguousExtentLinear1Segment1);
    ContiguousExtentLinear1.AppendLinear(ContiguousExtentLinear1Segment2);

    FlirtingExtentLinear1Segment1 = HVE2DSegment(HGF2DLocation(-10.0, 0.0, pWorld), HGF2DLocation(0.0, -10.0, pWorld));
    FlirtingExtentLinear1Segment2 = HVE2DSegment(HGF2DLocation(0.0, -10.0, pWorld), HGF2DLocation(-10.0, -10.0, pWorld));
    FlirtingExtentLinear1 = HVE2DComplexLinear(pWorld); // Open
    FlirtingExtentLinear1.AppendLinear(FlirtingExtentLinear1Segment1);
    FlirtingExtentLinear1.AppendLinear(FlirtingExtentLinear1Segment2);

    FlirtingExtentLinkedLinear1Segment1 = HVE2DSegment(HGF2DLocation(-10.0, 0.0, pWorld), HGF2DLocation(0.0, -10.0, pWorld));
    FlirtingExtentLinkedLinear1Segment2 = HVE2DSegment(HGF2DLocation(0.0, -10.0, pWorld), HGF2DLocation(0.0, 0.0, pWorld));
    FlirtingExtentLinkedLinear1 = HVE2DComplexLinear(pWorld); // Open
    FlirtingExtentLinkedLinear1.AppendLinear(FlirtingExtentLinkedLinear1Segment1);
    FlirtingExtentLinkedLinear1.AppendLinear(FlirtingExtentLinkedLinear1Segment2);

    ConnectedLinear1Segment1 = HVE2DSegment(HGF2DLocation(-10.0, 10.0, pWorld), HGF2DLocation(10.0, -10.0, pWorld));
    ConnectedLinear1Segment2 = HVE2DSegment(HGF2DLocation(10.0, -10.0, pWorld), HGF2DLocation(0.0, -10.0, pWorld));
    ConnectedLinear1 = HVE2DComplexLinear(pWorld); // Open
    ConnectedLinear1.AppendLinear(ConnectedLinear1Segment1);
    ConnectedLinear1.AppendLinear(ConnectedLinear1Segment2);

    ConnectingLinear1Segment1 = HVE2DSegment(HGF2DLocation(5.0, 5.0, pWorld), HGF2DLocation(-5.0, 10.0, pWorld));
    ConnectingLinear1Segment2 = HVE2DSegment(HGF2DLocation(-5.0, 10.0, pWorld), HGF2DLocation(-15.0, 0.0, pWorld));
    ConnectingLinear1 = HVE2DComplexLinear(pWorld);
    ConnectingLinear1.AppendLinear(ConnectingLinear1Segment1);
    ConnectingLinear1.AppendLinear(ConnectingLinear1Segment2);

    ConnectedLinear1ASegment1 = HVE2DSegment(HGF2DLocation(40.0, 0.0, pWorld), HGF2DLocation(10.0, 0.0, pWorld));
    ConnectedLinear1ASegment2 = HVE2DSegment(HGF2DLocation(10.0, 0.0, pWorld), HGF2DLocation(11.0, -10.0, pWorld));
    ConnectedLinear1A = HVE2DComplexLinear(pWorld); // Open
    ConnectedLinear1A.AppendLinear(ConnectedLinear1ASegment1);
    ConnectedLinear1A.AppendLinear(ConnectedLinear1ASegment2);

    ConnectingLinear1ASegment1 = HVE2DSegment(HGF2DLocation(21.0, 20.0, pWorld), HGF2DLocation(20.0, 15.0, pWorld));
    ConnectingLinear1ASegment2 = HVE2DSegment(HGF2DLocation(20.0, 15.0, pWorld), HGF2DLocation(20.0, 10.0, pWorld));
    ConnectingLinear1A = HVE2DComplexLinear(pWorld);
    ConnectingLinear1A.AppendLinear(ConnectingLinear1ASegment1);
    ConnectingLinear1A.AppendLinear(ConnectingLinear1ASegment2);

    LinkedLinear1Segment1 = HVE2DSegment(HGF2DLocation(0.0, 0.0, pWorld), HGF2DLocation(20.0, 4.0, pWorld));
    LinkedLinear1Segment2 = HVE2DSegment(HGF2DLocation(20.0, 4.0, pWorld), HGF2DLocation(21, 0.0, pWorld));
    LinkedLinear1 = HVE2DComplexLinear(pWorld);
    LinkedLinear1.AppendLinear(LinkedLinear1Segment1);
    LinkedLinear1.AppendLinear(LinkedLinear1Segment2);

    LinkedLinear1ASegment1 = HVE2DSegment(HGF2DLocation(11.0, 3.0, pWorld), HGF2DLocation(20.0, 4.0, pWorld));
    LinkedLinear1ASegment2 = HVE2DSegment(HGF2DLocation(20.0, 4.0, pWorld), HGF2DLocation(30.0-MYEPSILON, 0.0, pWorld));
    LinkedLinear1A = HVE2DComplexLinear(pWorld);
    LinkedLinear1A.AppendLinear(LinkedLinear1ASegment1);
    LinkedLinear1A.AppendLinear(LinkedLinear1ASegment2);

    // Shapes
    Rect1 = HVE2DRectangle(10.0, 10.0, 20.0, 20.0, pWorld);

    NorthContiguousRect = HVE2DRectangle(10.0, 20.0, 20.0, 30.0, pWorld);
    EastContiguousRect = HVE2DRectangle(20.0, 10.0, 30.0, 20.0, pWorld);
    WestContiguousRect = HVE2DRectangle(0.0, 10.0, 10.0, 20.0, pWorld);
    SouthContiguousRect = HVE2DRectangle(10.0, 0.0, 20.0, 10.0, pWorld);

    NETipRect = HVE2DRectangle(20.0, 20.0, 30.0, 30.0, pWorld);
    NWTipRect = HVE2DRectangle(0.0, 20.0, 10.0, 30.0, pWorld);
    SETipRect = HVE2DRectangle(20.0, 0.0, 30.0, 10.0, pWorld);
    SWTipRect = HVE2DRectangle(0.0, 0.0, 10.0, 10.0, pWorld);

    VerticalFitRect = HVE2DRectangle(15.0, 10.0, 25.0, 20.0, pWorld);
    HorizontalFitRect = HVE2DRectangle(10.0, 15.0, 20.0, 25.0, pWorld);

    DisjointRect = HVE2DRectangle(-10.0, -10.0, 0.0, 0.0, pWorld);
    NegativeRect = HVE2DRectangle(-20.0, -20.0, -10.0, -10.0, pWorld);

    MiscRect1 = HVE2DRectangle(5.0, 5.0, 15.0, 15.0, pWorld);

    EnglobRect1 = HVE2DRectangle(10.0, 10.0, 30.0, 30.0, pWorld);
    EnglobRect2 = HVE2DRectangle(0.0, 0.0, 30.0, 30.0, pWorld);
    EnglobRect3 = HVE2DRectangle(10.0, 10.0, 20.0, 30.0, pWorld);

    IncludedRect1 = HVE2DRectangle(10.0, 10.0, 15.0, 15.0, pWorld);
    IncludedRect2 = HVE2DRectangle(15.0, 10.0, 20.0, 15.0, pWorld);
    IncludedRect3 = HVE2DRectangle(15.0, 15.0, 20.0, 20.0, pWorld);
    IncludedRect4 = HVE2DRectangle(10.0, 15.0, 15.0, 20.0, pWorld);
    IncludedRect5 = HVE2DRectangle(12.0, 12.0, 18.0, 18.0, pWorld);
    IncludedRect6 = HVE2DRectangle(10.0, 10.0, 20.0, 15.0, pWorld);
    IncludedRect7 = HVE2DRectangle(10.0, 10.0, 15.0, 20.0, pWorld);
    IncludedRect8 = HVE2DRectangle(15.0, 10.0, 20.0, 20.0, pWorld);
    IncludedRect9 = HVE2DRectangle(10.0, 15.0, 20.0, 20.0, pWorld);

    RectPSys1 = HVE2DRectangle(10.0, 10.0, 15.0, 15.0, pSys1);

    SWCuttingSegment = HVE2DSegment(HGF2DLocation(5.0, 5.0, pWorld), HGF2DLocation(15.0, 15.0, pWorld));
    NECuttingSegment =  HVE2DSegment(HGF2DLocation(15.0, 15.0, pWorld), HGF2DLocation(25.0, 25.0, pWorld));
    NWCuttingSegment = HVE2DSegment(HGF2DLocation(5.0, 25.0, pWorld), HGF2DLocation(15.0, 15.0, pWorld));
    SECuttingSegment = HVE2DSegment(HGF2DLocation(15.0, 15.0, pWorld), HGF2DLocation(25.0, 5.0, pWorld));

    NorthContiguousSegment = HVE2DSegment(HGF2DLocation(0.0, 20.0, pWorld), HGF2DLocation(30.0, 20.0, pWorld));
    SouthContiguousSegment = HVE2DSegment(HGF2DLocation(0.0, 10.0, pWorld), HGF2DLocation(30.0, 10.0, pWorld));
    EastContiguousSegment = HVE2DSegment(HGF2DLocation(20.0, 0.0, pWorld), HGF2DLocation(20.0, 30.0, pWorld));
    WestContiguousSegment = HVE2DSegment(HGF2DLocation(10.0, 0.0, pWorld), HGF2DLocation(10.0, 30.0, pWorld));

    RectClosePoint1A = HGF2DLocation(21.1, 10.1, pWorld);
    RectClosePoint1B = HGF2DLocation(9.0, 9.0, pWorld);
    RectClosePoint1C = HGF2DLocation(19.9, 10.0, pWorld);
    RectClosePoint1D = HGF2DLocation(0.1, 15.0, pWorld);
    RectCloseMidPoint1 = HGF2DLocation(15.0, 20.1, pWorld);

    VeryFarPoint = HGF2DLocation(10000.0, 10000.1, pWorld);

    Rect1Point0d0 = HGF2DLocation(10.0, 10.0, pWorld);
    Rect1Point0d1 = HGF2DLocation(15.0, 10.0, pWorld);
    Rect1Point0d5 = HGF2DLocation(20.0, 20.0, pWorld);
    Rect1Point1d0 = HGF2DLocation(10.0, 10.0+1.1*MYEPSILON, pWorld);

    RectMidPoint1 = HGF2DLocation(15.0, 20.0, pWorld);
    
	// ====================================================================================================================================
	// SECTION START FOR LITE VERSIONS (non-HVE)
  	// ====================================================================================================================================

    // VERTICAL SEGMENT
    VerticalSegment1A = HGF2DSegment(HGF2DPosition(0.1, 0.1), HGF2DPosition(0.1, 10.1));
    VerticalSegment2A = HGF2DSegment(HGF2DPosition(0.1, 10.1), HGF2DPosition(0.1, 0.1));
    VerticalSegment3A = HGF2DSegment(HGF2DPosition(0.1, 0.1), HGF2DPosition(0.1, 0.1 + MYEPSILON));
    VerticalSegment4A = HGF2DSegment(HGF2DPosition(0.1 + MYEPSILON, 0.1), HGF2DPosition(0.1+MYEPSILON, 10.1));
    VerticalSegment5A = HGF2DSegment(HGF2DPosition(-10.0, -10.0), HGF2DPosition(-10.0, 0.0));
    CloseVerticalSegment1A = HGF2DSegment(HGF2DPosition(0.1+MYEPSILON, 0.1), HGF2DPosition(0.1, 10.1));

    VerticalPoint0d0A = HGF2DPosition(0.1, 0.1);
    VerticalPoint0d1A = HGF2DPosition(0.1, 1.1);
    VerticalPoint0d5A = HGF2DPosition(0.1, 5.1);
    VerticalPoint1d0A = HGF2DPosition(0.1, 10.1);

    Vertical3Point0d0A = HGF2DPosition(0.1, 0.1);
    Vertical3Point0d1A = HGF2DPosition(0.1, 0.1 + (0.1 * MYEPSILON));
    Vertical3Point0d5A = HGF2DPosition(0.1, 0.1 + (0.5 * MYEPSILON));
    Vertical3Point1d0A = HGF2DPosition(0.1, 0.1 + MYEPSILON);

    VerticalMidPoint1A = HGF2DPosition(0.1, 5.1);
    VerticalMidPoint3A = HGF2DPosition(0.1, 0.1 + (MYEPSILON/2));

    VerticalClosePoint1AA = HGF2DPosition(0.0, 0.0);
    VerticalClosePoint1BA = HGF2DPosition(0.0, 5.0);
    VerticalClosePoint1CA = HGF2DPosition(0.2, 7.0);
    VerticalClosePoint1DA = HGF2DPosition(1000000.0, 10.0);
    VerticalCloseMidPoint1A = HGF2DPosition(0.0, 5.1);

    VerticalClosePoint3AA = HGF2DPosition(0.0, 0.0);
    VerticalClosePoint3BA = HGF2DPosition(0.0, 0.1 + (2*MYEPSILON/3));
    VerticalClosePoint3CA = HGF2DPosition(0.2, 0.1 + (MYEPSILON/3));
    VerticalClosePoint3DA = HGF2DPosition(1000000.0, 0.1 + (MYEPSILON/2));
    VerticalCloseMidPoint3A = HGF2DPosition(0.0, 0.1 + (MYEPSILON/2));

    // HORIZONTAL SEGMENT
    HorizontalSegment1A = HGF2DSegment(HGF2DPosition(0.1, 0.1), HGF2DPosition(10.1, 0.1));
    HorizontalSegment2A = HGF2DSegment(HGF2DPosition(10.1, 0.1), HGF2DPosition(0.1, 0.1));
    HorizontalSegment3A = HGF2DSegment(HGF2DPosition(0.1, 0.1), HGF2DPosition(0.1 + MYEPSILON, 0.1));
    HorizontalSegment5A = HGF2DSegment(HGF2DPosition(0.1, 0.1), HGF2DPosition(10.1 + MYEPSILON, 0.1));

    HorizontalPoint0d0A = HGF2DPosition(0.1, 0.1);
    HorizontalPoint0d1A = HGF2DPosition(1.1, 0.1);
    HorizontalPoint0d5A = HGF2DPosition(5.1, 0.1);
    HorizontalPoint1d0A = HGF2DPosition(10.1, 0.1);

    Horizontal3Point0d0A = HGF2DPosition(0.1, 0.1);
    Horizontal3Point0d1A = HGF2DPosition(0.1 + (0.1 * MYEPSILON), 0.1);
    Horizontal3Point0d5A = HGF2DPosition(0.1 + (0.5 * MYEPSILON), 0.1);
    Horizontal3Point1d0A = HGF2DPosition(0.1 + MYEPSILON, 0.1);

    HorizontalMidPoint1A = HGF2DPosition(5.1, 0.1);
    HorizontalMidPoint3A = HGF2DPosition(0.1 + (MYEPSILON/2), 0.1);

    HorizontalClosePoint1AA = HGF2DPosition(0.0, 0.0);
    HorizontalClosePoint1BA = HGF2DPosition(5.0, 0.0);
    HorizontalClosePoint1CA = HGF2DPosition(7.0, 0.2);
    HorizontalClosePoint1DA = HGF2DPosition(10.0, 1000000.0);
    HorizontalCloseMidPoint1A = HGF2DPosition(5.1, 0.0);

    HorizontalClosePoint3AA = HGF2DPosition(0.0, 0.0);
    HorizontalClosePoint3BA = HGF2DPosition(0.1 + (2*MYEPSILON/3), 0.0);
    HorizontalClosePoint3CA = HGF2DPosition(0.1 + (MYEPSILON/3), 0.2);
    HorizontalClosePoint3DA = HGF2DPosition (0.1 + (MYEPSILON/2), 1000000.0);
    HorizontalCloseMidPoint3A = HGF2DPosition(0.1 + (MYEPSILON/2), 0.0);

    // MISC SEGMENTS
    MiscSegment1A = HGF2DSegment(HGF2DPosition(0.1, 0.1), HGF2DPosition(10.1, 10.1));
    MiscSegment2A = HGF2DSegment(HGF2DPosition(10.1, 10.1), HGF2DPosition(0.1, 0.1));
    MiscSegment3AA = HGF2DSegment(HGF2DPosition(0.1, 0.1), HGF2DDisplacement(HGFBearing(77.5 * PI/180), MYEPSILON));
    MiscSegment4A = HGF2DSegment(HGF2DPosition(0.2, 0.1), HGF2DDisplacement((77.5), MYEPSILON));
    MiscSegment6A = HGF2DSegment(HGF2DPosition(0.1, 0.1), HGF2DPosition(-9.9, 10.1));
    MiscSegment7A = HGF2DSegment(HGF2DPosition(0.2, 0.0), HGF2DPosition(-9.8, 10.0));
    DisjointSegment1A = HGF2DSegment(HGF2DPosition(-0.1, -0.1), HGF2DPosition(-10.1, -10.24));
    ContiguousExtentSegment1A = HGF2DSegment(HGF2DPosition(10.1, 0.1), HGF2DPosition(20.1, 10.1));
    FlirtingExtentSegment1A = HGF2DSegment(HGF2DPosition(10.1, 0.1), HGF2DPosition(20.1, -10.1));
    FlirtingExtentLinkedSegment1A = HGF2DSegment(HGF2DPosition(10.1, 10.1), HGF2DPosition(20.1, 0.1));
    ParallelSegment1A = HGF2DSegment(HGF2DPosition(0.2, 0.1), HGF2DPosition(10.2, 10.1));
    LinkedParallelSegment1A = HGF2DSegment(HGF2DPosition(10.1, 10.1), HGF2DPosition(20.1, 20.1));
    NearParallelSegment1A = HGF2DSegment(HGF2DPosition(0.2, 0.1), HGF2DPosition(10.2, 10.1+MYEPSILON));
    CloseNearParallelSegment1A = HGF2DSegment(HGF2DPosition(0.1+MYEPSILON, 0.1), HGF2DPosition(10.1+MYEPSILON, 10.1+MYEPSILON));
    ConnectedSegment1B = HGF2DSegment(HGF2DPosition(0.2, 0.0), HGF2DPosition(0.0, 0.2));
    ConnectingSegment1B = HGF2DSegment(HGF2DPosition(10.2, 0.0), HGF2DPosition(2.0, 2.0));
    ConnectedSegment1AA = HGF2DSegment(HGF2DPosition(20.2, 0.0), HGF2DPosition(0.0, 20.2));
    ConnectingSegment1AA = HGF2DSegment(HGF2DPosition(2.0, 2.0), HGF2DPosition(10.2, 0.0));
    LinkedSegment1B = HGF2DSegment(HGF2DPosition(0.1, 0.1), HGF2DPosition(10.0, 3.2));
    LinkedSegment1AA = HGF2DSegment(HGF2DPosition(10.1, 10.1), HGF2DPosition(10.0, 3.2));

    MiscSegment3AA = HGF2DSegment(HGF2DPosition(0.1+MYEPSILON, 0.1), HGF2DDisplacement(HGFBearing(77.5 * PI/180), MYEPSILON));

    Misc3Point0d0A = HGF2DPosition(0.1, 0.1);
    Misc3Point0d1A = HGF2DPosition(0.1, 0.1) + HGF2DDisplacement(HGFBearing(77.5 * PI/180), MYEPSILON * 0.1);
    Misc3Point0d5A = HGF2DPosition(0.1, 0.1) + HGF2DDisplacement(HGFBearing(77.5* PI/180), MYEPSILON * 0.5);
    Misc3Point1d0A = HGF2DPosition(0.1, 0.1) + HGF2DDisplacement(HGFBearing(77.5* PI/180), MYEPSILON);
    MiscMidPoint1A = HGF2DPosition(5.1, 5.1);

    MiscMidPoint3A = HGF2DPosition(0.1, 0.1)+ HGF2DDisplacement(HGFBearing(77.5), MYEPSILON / 2);
    MiscClosePoint3AA = HGF2DPosition(0.0, 0.0);
    MiscClosePoint3BA = HGF2DPosition(0.0, 0.1 + (2*MYEPSILON/3));
    MiscClosePoint3CA = HGF2DPosition(0.2, 0.1 + (MYEPSILON/3));
    MiscClosePoint3DA = HGF2DPosition(1000000.0, 0.1 + (MYEPSILON/2));
    MiscCloseMidPoint3A = HGF2DPosition(0.0, 0.1 + (MYEPSILON/2));

    VeryFarAlignedPointA = HGF2DPosition(21E123, 21E123);
    VeryFarAlignedNegativePointA = HGF2DPosition(-21E123, -21E123);
    VeryFarNegativePointA = HGF2DPosition(-1E123, -21E123);
    MidPointA = HGF2DPosition(5.1, 5.1);

    MiscMidPoint6A = HGF2DPosition(-4.9, 5.1);

    // LARGE SEGMENTS
    LargeSegment1A = HGF2DSegment(HGF2DPosition(-1E123, -21E123), HGF2DPosition(9E123, 19E123));
    LargeSegment2A =  HGF2DSegment(HGF2DPosition(9E123, 19E123), HGF2DPosition(-1E123, -21E123));
    ParallelLargeSegment1A = HGF2DSegment(HGF2DPosition(-1.000000001E123, -21E123), HGF2DPosition(9.000000001E123, 19E123));

    LargePoint0d0A = HGF2DPosition(-1E123, -21E123);
    LargePoint0d1A = HGF2DPosition(0.0, -17E123);
    LargePoint0d5A = HGF2DPosition(4E123, -1E123);
    LargePoint1d0A = HGF2DPosition(9E123, 19E123);

    LargeMidPoint1A = HGF2DPosition(4E123, -1E123);

    LargeClosePoint1AA = HGF2DPosition(0.0, 0.0);
    LargeClosePoint1BA = HGF2DPosition(1E124, 1E124);
    LargeClosePoint1CA = HGF2DPosition(-1E124, -1E124);
    LargeClosePoint1DA = HGF2DPosition(10.0, 1E120);
    LargeCloseMidPoint1A = HGF2DPosition(4.0001E123, -1.0002E123);

    // POSITIVE SEGMENTS
    PositiveSegment1A = HGF2DSegment(HGF2DPosition(1E123, 21E123), HGF2DPosition(11E123, 41E123));
    PositiveSegment2A = HGF2DSegment(HGF2DPosition(11E123, 41E123), HGF2DPosition(1E123, 21E123));
    ParallelPositiveSegment1A = HGF2DSegment(HGF2DPosition(1.000001E123, 21E123), HGF2DPosition(11.000001E123, 41E123));

    PositivePoint0d0A = HGF2DPosition(1E123, 21E123);
    PositivePoint0d1A = HGF2DPosition(2E123, 23E123);
    PositivePoint0d5A = HGF2DPosition(6E123, 31E123);
    PositivePoint1d0A = HGF2DPosition(11E123, 41E123);

    PositiveMidPoint1A = HGF2DPosition(6E123, 31E123);

    PositiveClosePoint1AA = HGF2DPosition(0.0, 0.0);
    PositiveClosePoint1BA = HGF2DPosition(1E124, 1E124);
    PositiveClosePoint1CA = HGF2DPosition(-1E124, -1E124);
    PositiveClosePoint1DA = HGF2DPosition(10.0, 1E120);
    PositiveCloseMidPoint1A = HGF2DPosition(6.0001E123, 31.0002E123);

    // NEGATIVE SEGMENTS
    NegativeSegment1A = HGF2DSegment(HGF2DPosition(-1E123, -21E123), HGF2DPosition(-11E123, -41E123));
    NegativeSegment2A = HGF2DSegment(HGF2DPosition(-11E123, -41E123), HGF2DPosition(-1E123, -21E123));
    ParallelNegativeSegment1A = HGF2DSegment(HGF2DPosition(-1.000001E123, -21E123), HGF2DPosition(-11.000001E123, -41E123));

    NegativePoint0d0A = HGF2DPosition(-1E123, -21E123);
    NegativePoint0d1A = HGF2DPosition(-2E123, -23E123);
    NegativePoint0d5A = HGF2DPosition(-6E123, -31E123);
    NegativePoint1d0A = HGF2DPosition(-11E123, -41E123);

    NegativeMidPoint1A = HGF2DPosition(-6E123, -31E123);

    NegativeClosePoint1AA = HGF2DPosition(0.0, 0.0);
    NegativeClosePoint1BA = HGF2DPosition(1E124, 1E124);
    NegativeClosePoint1CA = HGF2DPosition(-1E124, -1E124);
    NegativeClosePoint1DA = HGF2DPosition(10.0, 1E120);
    NegativeCloseMidPoint1A = HGF2DPosition(-6.0001E123, -31.0002E123);

    // NULL SEGMENTS
    NullSegment1A = HGF2DSegment();
    NullSegment2A = HGF2DSegment();

    // Special SEGMENTS
    SegmentInvalidA = HGF2DSegment(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 10.0));
    SegmentPSys1A = HGF2DSegment(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 10.0));

     // Complex Linears
    Linear1Segment1A = HGF2DSegment(HGF2DPosition(0.0, 0.0), HGF2DPosition(10.0, 10.0));
    Linear1Segment2A = HGF2DSegment(HGF2DPosition(10.0, 10.0), HGF2DPosition(20.0, 10.0));
    Linear1Segment3A = HGF2DSegment(HGF2DPosition(20.0, 10.0), HGF2DPosition(30.0, 10.0));
    Linear1Segment4A = HGF2DSegment(HGF2DPosition(30.0, 10.0), HGF2DPosition(30.0, 5.0));
    Linear1Segment5A = HGF2DSegment(HGF2DPosition(30.0, 5.0), HGF2DPosition(30.0, 5.0-MYEPSILON));
    Linear1Segment6A = HGF2DSegment(HGF2DPosition(30.0, 5.0-MYEPSILON), HGF2DPosition(30.0-MYEPSILON, 0.0));
    Linear1A = HGF2DPolySegment(); // Open
    Linear1A.AppendPoint(HGF2DPosition(0.0, 0.0));
    Linear1A.AppendPoint(HGF2DPosition(10.0, 10.0));
    Linear1A.AppendPoint(HGF2DPosition(20.0, 10.0));
    Linear1A.AppendPoint(HGF2DPosition(30.0, 10.0));
    Linear1A.AppendPoint(HGF2DPosition(30.0, 5.0));
    Linear1A.AppendPoint(HGF2DPosition(30.0, 5.0-MYEPSILON));
    Linear1A.AppendPoint(HGF2DPosition(30.0-MYEPSILON, 0.0));

    Linear1Point0d0A = HGF2DPosition(0.0, 0.0);
    Linear1Point0d1A = HGF2DPosition(3.1213203435596, 3.1213203435596);
    Linear1Point0d5A = HGF2DPosition(17.928932188135, 10.0);
    Linear1Point1d0A = HGF2DPosition(30.0-MYEPSILON, 0.0);
    LinearMidPoint1A = HGF2DPosition(17.928932188135, 10.0);

    LinearClosePoint1AA = HGF2DPosition(21.1, 10.1);
    LinearClosePoint1BA = HGF2DPosition(-10.0, 10.0);
    LinearClosePoint1CA = HGF2DPosition(30.0, 10.1);
    LinearClosePoint1DA = HGF2DPosition(32.1, 5.1);
    LinearCloseMidPoint1A = HGF2DPosition(17.928932, 9.1);

    Linear2Segment1A = HGF2DSegment(HGF2DPosition(0.0, 0.0), HGF2DPosition(-10.0, -10.0));
    Linear2Segment2A = HGF2DSegment(HGF2DPosition(-10.0, -10.0), HGF2DPosition(18.0, 9.0));
    Linear2Segment3A = HGF2DSegment(HGF2DPosition(18.0, 9.0), HGF2DPosition(21.0, 0.0));
    Linear2Segment4A = HGF2DSegment(HGF2DPosition(21.0, 0.0), HGF2DPosition(24.0, 15.0));
    Linear2Segment5A = HGF2DSegment(HGF2DPosition(24.0, 15.0), HGF2DPosition(0.0, 15.0));
    Linear2Segment6A = HGF2DSegment(HGF2DPosition(0.0, 15.0), HGF2DPosition(0.0, 0.0));

    Linear2A = HGF2DPolySegment();  // AutoClosed
    Linear2A.AppendPoint(HGF2DPosition(0.0, 0.0));
    Linear2A.AppendPoint(HGF2DPosition(-10.0, -10.0));
    Linear2A.AppendPoint(HGF2DPosition(18.0, 9.0));
    Linear2A.AppendPoint(HGF2DPosition(21.0, 0.0));
    Linear2A.AppendPoint(HGF2DPosition(24.0, 15.0));
    Linear2A.AppendPoint(HGF2DPosition(0.0, 15.0));
    Linear2A.AppendPoint(HGF2DPosition(0.0, 0.0));

    Linear2Point0d0A = HGF2DPosition(0.0, 0.0);
    Linear2Point0d1A = HGF2DPosition(-7.9028994453177, -7.9028994453177);
    Linear2Point0d5A = HGF2DPosition(20.498817144560, 1.5035485663202);
    Linear2Point1d0A = HGF2DPosition(0.0, 0.0);

    LinearMidPoint2A = HGF2DPosition(20.498817144560, 1.5035485663202);

    LinearClosePoint2AA = HGF2DPosition(21.1, 10.1);
    LinearClosePoint2BA = HGF2DPosition(21.1, 10.1);
    LinearClosePoint2CA = HGF2DPosition(21.1, 10.1);
    LinearClosePoint2DA = HGF2DPosition(21.1, 10.1);
    LinearCloseMidPoint2A = HGF2DPosition(21.1, 10.1);
   
    // epsilon size container
    Linear3Segment1A = HGF2DSegment(HGF2DPosition(0.0, 0.0), HGF2DPosition(0.0-MYEPSILON, 0.0));
    Linear3Segment2A = HGF2DSegment(HGF2DPosition(0.0-MYEPSILON, 0.0), HGF2DPosition(0.0-MYEPSILON, 0.0+MYEPSILON));
    Linear3Segment3A = HGF2DSegment(HGF2DPosition(0.0-MYEPSILON, 0.0+MYEPSILON), HGF2DPosition(0.0, 0.0+MYEPSILON));
    Linear3Segment4A = HGF2DSegment(HGF2DPosition(0.0, 0.0+MYEPSILON), HGF2DPosition(0.0+MYEPSILON, 0.0+2*MYEPSILON));
    Linear3Segment5A = HGF2DSegment(HGF2DPosition(0.0+MYEPSILON, 0.0+2*MYEPSILON), HGF2DPosition(0.0+MYEPSILON, 0.0));
    Linear3A = HGF2DPolySegment(); // Open
    Linear3A.AppendPoint(HGF2DPosition(0.0-MYEPSILON, 0.0));
    Linear3A.AppendPoint(HGF2DPosition(0.0-MYEPSILON, 0.0));
    Linear3A.AppendPoint(HGF2DPosition(0.0-MYEPSILON, 0.0+MYEPSILON));
    Linear3A.AppendPoint(HGF2DPosition(0.0, 0.0+MYEPSILON));
    Linear3A.AppendPoint(HGF2DPosition(0.0+MYEPSILON, 0.0+2*MYEPSILON));
    Linear3A.AppendPoint(HGF2DPosition(HGF2DPosition(0.0+MYEPSILON, 0.0)));

    Linear3Point0d0A = HGF2DPosition(0.0, 0.0);
    Linear3Point0d1A = HGF2DPosition(-0.64142135623731 *MYEPSILON, 0.0);
    Linear3Point0d5A = HGF2DPosition(0.14644660940673*MYEPSILON, 1.1464466094067*MYEPSILON);
    Linear3Point1d0A = HGF2DPosition(0.0+MYEPSILON, 0.0);

    LinearMidPoint3A = HGF2DPosition(1.4644660940673e-009, 1.1464466094067*MYEPSILON);

    LinearClosePoint3AA = HGF2DPosition(21.1, 10.1);
    LinearClosePoint3BA = HGF2DPosition(21.1, 10.1);
    LinearClosePoint3CA = HGF2DPosition(21.1, 10.1);
    LinearClosePoint3DA = HGF2DPosition(21.1, 10.1);
    LinearCloseMidPoint3A = HGF2DPosition(21.1, 10.1);

    // Used for intersection tests
    ComplexLinearCase1A = HGF2DPolySegment();
    ComplexLinearCase1A.AppendPoint(HGF2DPosition(5.0, 12.0));
    ComplexLinearCase1A.AppendPoint(HGF2DPosition(15.0, 15.0));

    ComplexLinearCase2A = HGF2DPolySegment();
    ComplexLinearCase2A.AppendPoint(HGF2DPosition(5.0, 5.0));
    ComplexLinearCase2A.AppendPoint(HGF2DPosition(10.0, 12.0));
    ComplexLinearCase2A.AppendPoint(HGF2DPosition(10.0, 15.0));
    ComplexLinearCase2A.AppendPoint(HGF2DPosition(15.0, 17.0));

    ComplexLinearCase3A = HGF2DPolySegment();
    ComplexLinearCase3A.AppendPoint(HGF2DPosition(5.0, 5.0));
    ComplexLinearCase3A.AppendPoint(HGF2DPosition(15.0, 15.0));

    ComplexLinearCase4A = HGF2DPolySegment();
    ComplexLinearCase4A.AppendPoint(HGF2DPosition(5.0, 10.0));
    ComplexLinearCase4A.AppendPoint(HGF2DPosition(35.0, 10.0));

    ComplexLinearCase5B = HGF2DPolySegment();
    ComplexLinearCase5B.AppendPoint(HGF2DPosition(5.0, 10.0));
    ComplexLinearCase5B.AppendPoint(HGF2DPosition(20.0, 10.0));
    ComplexLinearCase5B.AppendPoint(HGF2DPosition(15.0, 15.0));

    ComplexLinearCase5AA = HGF2DPolySegment();
    ComplexLinearCase5AA.AppendPoint(HGF2DPosition(10.0, 10.0));
    ComplexLinearCase5AA.AppendPoint(HGF2DPosition(20.0, 10.0));
    ComplexLinearCase5AA.AppendPoint(HGF2DPosition(15.0, 15.0));

    ComplexLinearCase6A = HGF2DPolySegment();
    ComplexLinearCase6A.AppendPoint(HGF2DPosition(10.0, 10.0));
    ComplexLinearCase6A.AppendPoint(HGF2DPosition(20.0, 10.0));
    ComplexLinearCase6A.AppendPoint(HGF2DPosition(20.0, 20.0));
    ComplexLinearCase6A.AppendPoint(HGF2DPosition(10.0, 20.0));

    ComplexLinearCase7A = HGF2DPolySegment();
    ComplexLinearCase7A.AppendPoint(HGF2DPosition(5.0, 5.0));
    ComplexLinearCase7A.AppendPoint(HGF2DPosition(10.0, 10.0));

    AutoCrossingLinear1A = HGF2DPolySegment();
    AutoCrossingLinear1A.AppendPoint(HGF2DPosition(0.0, 0.0));
    AutoCrossingLinear1A.AppendPoint(HGF2DPosition(10.0, 10.0));
    AutoCrossingLinear1A.AppendPoint(HGF2DPosition(15.0, 5.0));
    AutoCrossingLinear1A.AppendPoint(HGF2DPosition(0.0, 5.0));
    AutoCrossingLinear1A.AppendPoint(HGF2DPosition(5.0, 0.0));
    AutoCrossingLinear1A.AppendPoint(HGF2DPosition(20.0, 20.0));

    AutoCrossingLinear2A = HGF2DPolySegment();
    AutoCrossingLinear2A.AppendPoint(HGF2DPosition(0.0, 0.0));
    AutoCrossingLinear2A.AppendPoint(HGF2DPosition(10.0, 10.0));
    AutoCrossingLinear2A.AppendPoint(HGF2DPosition(15.0, 5.0));
    AutoCrossingLinear2A.AppendPoint(HGF2DPosition(10.0, 5.0));
    AutoCrossingLinear2A.AppendPoint(HGF2DPosition(10.0, 15.0));

    AutoConnectingLinear1A = HGF2DPolySegment();
    AutoConnectingLinear1A.AppendPoint(HGF2DPosition(0.0, 0.0));
    AutoConnectingLinear1A.AppendPoint(HGF2DPosition(10.0, 10.0));
    AutoConnectingLinear1A.AppendPoint(HGF2DPosition(15.0, 5.0));
    AutoConnectingLinear1A.AppendPoint(HGF2DPosition(10.0, 0.0));
    AutoConnectingLinear1A.AppendPoint(HGF2DPosition(-10.0, 0.0));

    DisjointLinear1A = HGF2DPolySegment();
    DisjointLinear1A.AppendPoint(HGF2DPosition(-10.0, -10.0));
    DisjointLinear1A.AppendPoint(HGF2DPosition(-20.0, -10.0));
    DisjointLinear1A.AppendPoint(HGF2DPosition(-30.0, -10.0));

    ContiguousExtentLinear1A = HGF2DPolySegment();
    ContiguousExtentLinear1A.AppendPoint(HGF2DPosition(-10.0, 0.0));
    ContiguousExtentLinear1A.AppendPoint(HGF2DPosition(0.0, 10.0));
    ContiguousExtentLinear1A.AppendPoint(HGF2DPosition(-10.0, 10.0));

    FlirtingExtentLinear1A = HGF2DPolySegment();
    FlirtingExtentLinear1A.AppendPoint(HGF2DPosition(-10.0, 0.0));
    FlirtingExtentLinear1A.AppendPoint(HGF2DPosition(0.0, -10.0));
    FlirtingExtentLinear1A.AppendPoint(HGF2DPosition(-10.0, -10.0));

    FlirtingExtentLinkedLinear1A = HGF2DPolySegment();
    FlirtingExtentLinkedLinear1A.AppendPoint(HGF2DPosition(-10.0, 0.0));
    FlirtingExtentLinkedLinear1A.AppendPoint(HGF2DPosition(0.0, -10.0));
    FlirtingExtentLinkedLinear1A.AppendPoint(HGF2DPosition(0.0, 0.0));

    ConnectedLinear1B = HGF2DPolySegment();
    ConnectedLinear1B.AppendPoint(HGF2DPosition(-10.0, 10.0));
    ConnectedLinear1B.AppendPoint(HGF2DPosition(10.0, -10.0));
    ConnectedLinear1B.AppendPoint(HGF2DPosition(0.0, -10.0));

    ConnectingLinear1B = HGF2DPolySegment();
    ConnectingLinear1B.AppendPoint(HGF2DPosition(5.0, 5.0));
    ConnectingLinear1B.AppendPoint(HGF2DPosition(-5.0, 10.0));
    ConnectingLinear1B.AppendPoint(HGF2DPosition(-15.0, 0.0));

    ConnectedLinear1AA = HGF2DPolySegment();
    ConnectedLinear1AA.AppendPoint(HGF2DPosition(40.0, 0.0));
    ConnectedLinear1AA.AppendPoint(HGF2DPosition(10.0, 0.0));
    ConnectedLinear1AA.AppendPoint(HGF2DPosition(11.0, -10.0));

    ConnectingLinear1AA = HGF2DPolySegment();
    ConnectingLinear1AA.AppendPoint(HGF2DPosition(21.0, 20.0));
    ConnectingLinear1AA.AppendPoint(HGF2DPosition(20.0, 15.0));
    ConnectingLinear1AA.AppendPoint(HGF2DPosition(20.0, 10.0));

    LinkedLinear1B = HGF2DPolySegment();
    LinkedLinear1B.AppendPoint(HGF2DPosition(21.0, 20.0));
    LinkedLinear1B.AppendPoint(HGF2DPosition(20.0, 15.0));
    LinkedLinear1B.AppendPoint(HGF2DPosition(21, 0.0));

    LinkedLinear1AA = HGF2DPolySegment();
    LinkedLinear1AA.AppendPoint(HGF2DPosition(11.0, 3.0));
    LinkedLinear1AA.AppendPoint(HGF2DPosition(20.0, 4.0));
    LinkedLinear1AA.AppendPoint(HGF2DPosition(30.0-MYEPSILON, 0.0));

    // Shapes
    Rect1A = HGF2DRectangle(10.0, 10.0, 20.0, 20.0);

    NorthContiguousRectA = HGF2DRectangle(10.0, 20.0, 20.0, 30.0);
    EastContiguousRectA = HGF2DRectangle(20.0, 10.0, 30.0, 20.0);
    WestContiguousRectA = HGF2DRectangle(0.0, 10.0, 10.0, 20.0);
    SouthContiguousRectA = HGF2DRectangle(10.0, 0.0, 20.0, 10.0);

    NETipRectA = HGF2DRectangle(20.0, 20.0, 30.0, 30.0);
    NWTipRectA = HGF2DRectangle(0.0, 20.0, 10.0, 30.0);
    SETipRectA = HGF2DRectangle(20.0, 0.0, 30.0, 10.0);
    SWTipRectA = HGF2DRectangle(0.0, 0.0, 10.0, 10.0);

    VerticalFitRectA = HGF2DRectangle(15.0, 10.0, 25.0, 20.0);
    HorizontalFitRectA = HGF2DRectangle(10.0, 15.0, 20.0, 25.0);

    DisjointRectA = HGF2DRectangle(-10.0, -10.0, 0.0, 0.0);
    NegativeRectA = HGF2DRectangle(-20.0, -20.0, -10.0, -10.0);

    MiscRect1A = HGF2DRectangle(5.0, 5.0, 15.0, 15.0);

    EnglobRect1A = HGF2DRectangle(10.0, 10.0, 30.0, 30.0);
    EnglobRect2A = HGF2DRectangle(0.0, 0.0, 30.0, 30.0);
    EnglobRect3A = HGF2DRectangle(10.0, 10.0, 20.0, 30.0);

    IncludedRect1A = HGF2DRectangle(10.0, 10.0, 15.0, 15.0);
    IncludedRect2A = HGF2DRectangle(15.0, 10.0, 20.0, 15.0);
    IncludedRect3A = HGF2DRectangle(15.0, 15.0, 20.0, 20.0);
    IncludedRect4A = HGF2DRectangle(10.0, 15.0, 15.0, 20.0);
    IncludedRect5A = HGF2DRectangle(12.0, 12.0, 18.0, 18.0);
    IncludedRect6A = HGF2DRectangle(10.0, 10.0, 20.0, 15.0);
    IncludedRect7A = HGF2DRectangle(10.0, 10.0, 15.0, 20.0);
    IncludedRect8A = HGF2DRectangle(15.0, 10.0, 20.0, 20.0);
    IncludedRect9A = HGF2DRectangle(10.0, 15.0, 20.0, 20.0);

    RectPSys1A = HGF2DRectangle(10.0, 10.0, 15.0, 15.0);

    SWCuttingSegmentA = HGF2DSegment(HGF2DPosition(5.0, 5.0), HGF2DPosition(15.0, 15.0));
    NECuttingSegmentA =  HGF2DSegment(HGF2DPosition(15.0, 15.0), HGF2DPosition(25.0, 25.0));
    NWCuttingSegmentA = HGF2DSegment(HGF2DPosition(5.0, 25.0), HGF2DPosition(15.0, 15.0));
    SECuttingSegmentA = HGF2DSegment(HGF2DPosition(15.0, 15.0), HGF2DPosition(25.0, 5.0));

    NorthContiguousSegmentA = HGF2DSegment(HGF2DPosition(0.0, 20.0), HGF2DPosition(30.0, 20.0));
    SouthContiguousSegmentA = HGF2DSegment(HGF2DPosition(0.0, 10.0), HGF2DPosition(30.0, 10.0));
    EastContiguousSegmentA = HGF2DSegment(HGF2DPosition(20.0, 0.0), HGF2DPosition(20.0, 30.0));
    WestContiguousSegmentA = HGF2DSegment(HGF2DPosition(10.0, 0.0), HGF2DPosition(10.0, 30.0));

    RectClosePoint1AA = HGF2DPosition(21.1, 10.1);
    RectClosePoint1BA = HGF2DPosition(9.0, 9.0);
    RectClosePoint1CA = HGF2DPosition(19.9, 10.0);
    RectClosePoint1DA = HGF2DPosition(0.1, 15.0);
    RectCloseMidPoint1A = HGF2DPosition(15.0, 20.1);

    VeryFarPointA = HGF2DPosition(10000.0, 10000.1);

    Rect1Point0d0A = HGF2DPosition(10.0, 10.0);
    Rect1Point0d1A = HGF2DPosition(15.0, 10.0);
    Rect1Point0d5A = HGF2DPosition(20.0, 20.0);
    Rect1Point1d0A = HGF2DPosition(10.0, 10.0+1.1*MYEPSILON);

    RectMidPoint1A = HGF2DPosition(15.0, 20.0);
    

    }