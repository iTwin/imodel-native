//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HVE2DPolygonOfSegmentsTesterWithEpsilon.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "EnvironnementTest.h"
#include "HVE2DPolygonOfSegmentsTester.h"

// SPECIAL TESTS
// The following are all case which failed with previous library

//==================================================================================
//IntersectShapeTestWithPointer
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointer)
    {

    HVE2DSegment    Segment1A(HGF2DLocation(-1.7E38, -1.7E38, pWorld), HGF2DLocation(-1.7E38, 1.70E38, pWorld));
    HVE2DSegment    Segment2A(HGF2DLocation(-1.7E38, 1.70E38, pWorld), HGF2DLocation(1.70E38, 1.70E38, pWorld));
    HVE2DSegment    Segment3A(HGF2DLocation(1.70E38, 1.70E38, pWorld), HGF2DLocation(1.70E38, -1.7E38, pWorld));
    HVE2DSegment    Segment4A(HGF2DLocation(1.70E38, -1.7E38, pWorld), HGF2DLocation(-1.7E38, -1.7E38, pWorld));
    
    HVE2DComplexLinear  MyLinear1(pWorld);
    MyLinear1.AppendLinear(Segment1A);
    MyLinear1.AppendLinear(Segment2A);
    MyLinear1.AppendLinear(Segment3A);
    MyLinear1.AppendLinear(Segment4A);

    HVE2DPolygonOfSegments    Poly1A(MyLinear1);

    HVE2DSegment    Segment1B(HGF2DLocation(0.000000000000, 0.0000000000000, pWorld), HGF2DLocation(395.9575494628, -76.96025049963, pWorld));
    HVE2DSegment    Segment2B(HGF2DLocation(395.9575494628, -76.96025049963, pWorld), HGF2DLocation(471.1935301076, 315.54848345850, pWorld));
    HVE2DSegment    Segment3B(HGF2DLocation(471.1935301076, 315.54848345850, pWorld), HGF2DLocation(76.23598064480, 392.50873395810, pWorld));
    HVE2DSegment    Segment4B(HGF2DLocation(76.23598064480, 392.50873395810, pWorld), HGF2DLocation(0.000000000000, 0.0000000000000, pWorld));
    
    HVE2DComplexLinear  MyLinear2(pWorld);
    MyLinear2.AppendLinear(Segment1B);
    MyLinear2.AppendLinear(Segment2B);
    MyLinear2.AppendLinear(Segment3B);
    MyLinear2.AppendLinear(Segment4B);

    HVE2DPolygonOfSegments    Poly1B(MyLinear2);

    HFCPtr<HVE2DPolygonOfSegments>     pResult1A = (HVE2DPolygonOfSegments*) Poly1A.IntersectShape(Poly1B);
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult1A->GetShapeType());
    ASSERT_TRUE(pResult1A->IsSimple());

    HVE2DComplexLinear  AComp1A;
    AComp1A = pResult1A->GetLinear(HVE2DSimpleShape::CW);      
    ASSERT_EQ(4, AComp1A.GetNumberOfLinears());
    ASSERT_DOUBLE_EQ(161049.202116080094, pResult1A->CalculateArea());

    ASSERT_NEAR(0.0, AComp1A.GetLinear(0).GetStartPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp1A.GetLinear(0).GetStartPoint().GetY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(76.23598064480, AComp1A.GetLinear(0).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(392.5087339581, AComp1A.GetLinear(0).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(76.23598064480, AComp1A.GetLinear(1).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(392.5087339581, AComp1A.GetLinear(1).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(471.1935301076, AComp1A.GetLinear(1).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(315.5484834585, AComp1A.GetLinear(1).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(471.19353010760, AComp1A.GetLinear(2).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(315.54848345850, AComp1A.GetLinear(2).GetStartPoint().GetY());
    ASSERT_DOUBLE_EQ(395.95754946280, AComp1A.GetLinear(2).GetEndPoint().GetX());
    ASSERT_DOUBLE_EQ(-76.96025049963, AComp1A.GetLinear(2).GetEndPoint().GetY());

    ASSERT_DOUBLE_EQ(395.95754946280, AComp1A.GetLinear(3).GetStartPoint().GetX());
    ASSERT_DOUBLE_EQ(-76.96025049963, AComp1A.GetLinear(3).GetStartPoint().GetY());
    ASSERT_NEAR(0.0, AComp1A.GetLinear(3).GetEndPoint().GetX(), MYEPSILON);
    ASSERT_NEAR(0.0, AComp1A.GetLinear(3).GetEndPoint().GetY(), MYEPSILON);

    }

//==================================================================================
// Test which failed on 21 may 1997
// The coordinate systems are different (but identity)
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointer2)
    {

    HFCPtr<HGF2DCoordSys> pCoordSysIdent = new HGF2DCoordSys(HGF2DIdentity(), pWorld);
    HFCPtr<HVE2DRectangle> pShape1 = new HVE2DRectangle (0.0, 0.0, 415.0, 409.0, pWorld);

    HVE2DComplexLinear  TheLinear(pCoordSysIdent);

    TheLinear.AppendLinear(HVE2DSegment(HGF2DLocation(0.00 , 256.0, pCoordSysIdent), HGF2DLocation(83.0 , 256.0, pCoordSysIdent)));
    TheLinear.AppendLinear(HVE2DSegment(HGF2DLocation(83.0 , 256.0, pCoordSysIdent), HGF2DLocation(83.0 , 0.000, pCoordSysIdent)));
    TheLinear.AppendLinear(HVE2DSegment(HGF2DLocation(83.0 , 0.000, pCoordSysIdent), HGF2DLocation(0.00 , 0.000, pCoordSysIdent)));
    TheLinear.AppendLinear(HVE2DSegment(HGF2DLocation(0.00 , 0.000, pCoordSysIdent), HGF2DLocation(0.00 , 256.0, pCoordSysIdent)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(21248.0, pResult->CalculateArea());
    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult->GetShapeType());
      
    }

//==================================================================================
// Another test which failed on 22 may 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeTestWithGetRectangle)
    {
    
    HFCPtr<HGF2DCoordSys> pCoordSysIdent = new HGF2DCoordSys(HGF2DIdentity(), pWorld);
              
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000 , 0.000, pWorld), HGF2DLocation(256.0 , 0.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 0.000, pWorld), HGF2DLocation(256.0 , 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 256.0, pWorld), HGF2DLocation(0.000 , 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000 , 256.0, pWorld), HGF2DLocation(0.000 , 0.000, pWorld)));

    HFCPtr<HVE2DPolygonOfSegments> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pCoordSysIdent);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 105.0, pCoordSysIdent), HGF2DLocation(256.0 , 0.000, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 0.000, pCoordSysIdent), HGF2DLocation(0.000 , 0.000, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000 , 0.000, pCoordSysIdent), HGF2DLocation(0.000 , 105.0, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000 , 105.0, pCoordSysIdent), HGF2DLocation(256.0 , 105.0, pCoordSysIdent)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DRectangle> pResult = (HVE2DRectangle*) pShape1->IntersectShape(*pShape2);

    ASSERT_DOUBLE_EQ(26880.0, pResult->CalculateArea());
    ASSERT_EQ(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult->GetShapeType());
    ASSERT_TRUE(pResult->IsSimple());

    double RectXMin;
    double RectXMax;
    double RectYMin;
    double RectYMax;

    pResult->GetRectangle(&RectXMin, &RectYMin, &RectXMax, &RectYMax);

    ASSERT_NEAR(0.0, RectXMin, MYEPSILON);
    ASSERT_DOUBLE_EQ(256.0, RectXMax);
    ASSERT_NEAR(0.0, RectYMin, MYEPSILON);
    ASSERT_DOUBLE_EQ(105.0, RectYMax);

    }

//==================================================================================
// Test which failed on may 28 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointer3)
    {
    
    HFCPtr<HGF2DCoordSys> pCoordSysIdent = new HGF2DCoordSys(HGF2DIdentity(), pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0000000000 , 331.0680836798, pWorld), 
                                         HGF2DLocation(172.5086905196 , 370.7988489798, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(172.5086905196 , 370.7988489798, pWorld), 
                                         HGF2DLocation(71.45071697720 , 153.0000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(71.45071697720 , 153.0000000000, pWorld), 
                                         HGF2DLocation(256.0000000000 , 153.0000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0000000000 , 153.0000000000, pWorld), 
                                         HGF2DLocation(256.0000000000 , 331.0680836798, pWorld)));

    HFCPtr<HVE2DPolygonOfSegments> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pCoordSysIdent);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0000000000 , 331.0680836798, pCoordSysIdent), 
                                         HGF2DLocation(172.5086905196 , 370.7988489798, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(172.5086905196 , 370.7988489798, pCoordSysIdent), 
                                         HGF2DLocation(53.76108564268 , 114.7988489798, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(53.76108564268 , 114.7988489798, pCoordSysIdent), 
                                         HGF2DLocation(256.0000000000 , 114.7988489798, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0000000000 , 114.7988489798, pCoordSysIdent), 
                                         HGF2DLocation(256.0000000000 , 331.0680836798, pCoordSysIdent)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DPolygonOfSegments> pResult = (HVE2DPolygonOfSegments*) pShape1->IntersectShape(*pShape2);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult->GetShapeType());
    ASSERT_TRUE(pResult->IsSimple());

    HVE2DComplexLinear  AComp;
    AComp = pResult->GetLinear(HVE2DSimpleShape::CW);
    ASSERT_EQ(4, AComp.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(27527.5828776401322, pResult->CalculateArea());

    }

//==================================================================================
// Test which failed on may 30 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointer4)
    {
    HFCPtr<HGF2DCoordSys> pCoordSysIdent = new HGF2DCoordSys(HGF2DIdentity(), pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000 , 0.000, pWorld), HGF2DLocation(0.000 , 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000 , 256.0, pWorld), HGF2DLocation(256.0 , 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 256.0, pWorld), HGF2DLocation(256.0 , 0.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 0.000, pWorld), HGF2DLocation(0.000 , 0.000, pWorld)));

    HFCPtr<HVE2DPolygonOfSegments> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pCoordSysIdent);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0000000000 , 38.73076530008, pCoordSysIdent), 
                                         HGF2DLocation(172.5086905196 , 0.000000000000, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(172.5086905196 , 0.000000000000, pCoordSysIdent), 
                                         HGF2DLocation(53.76108564268 , 256.0000000000, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(53.76108564268 , 256.0000000000, pCoordSysIdent), 
                                         HGF2DLocation(256.0000000000 , 256.0000000000, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.0000000000 , 256.0000000000, pCoordSysIdent), 
                                         HGF2DLocation(256.0000000000 , 38.73076530008, pCoordSysIdent)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DPolygonOfSegments> pResult = (HVE2DPolygonOfSegments*) pShape1->IntersectShape(*pShape2);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult->GetShapeType());
    ASSERT_TRUE(pResult->IsSimple());
      
    HVE2DComplexLinear  AComp;
    AComp = pResult->GetLinear(HVE2DSimpleShape::CW);
       
    ASSERT_EQ(4, AComp.GetNumberOfLinears());

    ASSERT_DOUBLE_EQ(34956.6274951873056, pResult->CalculateArea());

    }

//==================================================================================
// Test which failed on june 4 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointer5)
    {
    HFCPtr<HGF2DCoordSys> pCoordSysIdent = new HGF2DCoordSys(HGF2DIdentity(), pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000 , 0.0000000000000, pWorld), 
                                         HGF2DLocation(172.5086905196 , 370.79884897980, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(172.5086905196 , 370.79884897980, pWorld), 
                                         HGF2DLocation(548.6860067216 , 195.93306275580, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(548.6860067216 , 195.93306275580, pWorld),
                                         HGF2DLocation(376.1773162020 , -175.8657862240, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(376.1773162020 , -175.8657862240, pWorld),
                                         HGF2DLocation(0.000000000000 , 0.0000000000000, pWorld)));

    HFCPtr<HVE2DPolygonOfSegments> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pCoordSysIdent);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(77.00000000000, 165.2703287922, pCoordSysIdent), 
                                         HGF2DLocation(77.00000000000, 114.7988489798, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(77.00000000000, 114.7988489798, pCoordSysIdent), 
                                         HGF2DLocation(53.76108564268, 114.7988489798, pCoordSysIdent)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(53.76108564268, 114.7988489798, pCoordSysIdent), 
                                         HGF2DLocation(77.00000000000, 165.2703287922, pCoordSysIdent)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DPolygonOfSegments> pResult = (HVE2DPolygonOfSegments*) pShape1->IntersectShape(*pShape2);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult->GetShapeType());
    ASSERT_TRUE(pResult->IsSimple());
        
    HVE2DComplexLinear  AComp;
    AComp = pResult->GetLinear(HVE2DSimpleShape::CW);
       
    ASSERT_EQ(3, AComp.GetNumberOfLinears());
    ASSERT_DOUBLE_EQ(586.451198423784262, pResult->CalculateArea());

    }

//==================================================================================
// Test which failed on july 9 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShapeTestWithPointer6)
    {
    HFCPtr<HGF2DCoordSys> pCoordSysIdent = new HGF2DCoordSys(HGF2DIdentity(), pWorld);
      
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000000 , -56.5383363659600, pWorld), 
                                         HGF2DLocation(256.00000000000000 , 49.42061113575000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000000 , 49.42061113575000, pWorld), 
                                         HGF2DLocation(189.52802192110000 , 80.13421377599000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(189.52802192110000 , 80.13421377599000, pWorld), 
                                         HGF2DLocation(37.906557384530807, 80.134213775989394, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(37.906557384530807, 80.134213775989394, pWorld), 
                                         HGF2DLocation(0.0000000000000000, 0.0000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000, 0.0000000000000000, pWorld), 
                                         HGF2DLocation(232.14793481380000, -108.9027500563000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(232.14793481380000, -108.9027500563000, pWorld), 
                                         HGF2DLocation(256.00000000000000, -56.53833636596000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000000, -119.7476048769000, pWorld), 
                                         HGF2DLocation(0.0000000000000000, 0.0000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000, 0.0000000000000000, pWorld),
                                         HGF2DLocation(37.906557384530799, 80.134213775989536, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(37.906557384530799, 80.134213775989536, pWorld),
                                         HGF2DLocation(256.00000000000000, 80.134213775990000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000000, 80.134213775990000, pWorld),
                                         HGF2DLocation(256.00000000000000, -119.7476048769000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DPolygonOfSegments> pResult = (HVE2DPolygonOfSegments*) pShape2->IntersectShape(*pShape1);

    ASSERT_NE(static_cast<HGF2DShapeTypeId>(HVE2DRectangle::CLASS_ID), pResult->GetShapeType());
    ASSERT_TRUE(pResult->IsSimple());
    
    ASSERT_DOUBLE_EQ(32552.3193134339599, pResult->CalculateArea());

    HVE2DComplexLinear  AComp;
    AComp = pResult->GetLinear(HVE2DSimpleShape::CW);

    }

//==================================================================================
// Test which failed on aug 26, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  GenerateScanLinesTest)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 175.000000000, pWorld), HGF2DLocation(256.0 , 30.0000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 30.0000000000, pWorld), HGF2DLocation(230.0 , 30.0000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(230.0 , 30.0000000000, pWorld), HGF2DLocation(230.0 , 30.0000000001, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(230.0 , 30.0000000001, pWorld), HGF2DLocation(230.0 , 30.0000000002, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(230.0 , 30.0000000002, pWorld), HGF2DLocation(230.0 , 175.000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(230.0 , 175.000000000, pWorld), HGF2DLocation(256.0 , 175.000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    // Create shape of same size
    HVEShape  MyShape(pShape1->GetExtent());

    // Generate scanlines for this dum shape (this insures creation of Y)
    HGFScanLines  TheScanLines(false, pWorld);

    MyShape.GenerateScanLines(TheScanLines);

    }

//==================================================================================
// Test which failed on aug 26, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  GenerateScanLinesTest2)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000 , 234.8339996446, pWorld), HGF2DLocation(243.500 , 237.5000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(243.500 , 237.5000000000, pWorld), HGF2DLocation(243.625 , 238.0000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(243.625 , 238.0000000000, pWorld), HGF2DLocation(256.000 , 238.0000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000 , 238.0000000000, pWorld), HGF2DLocation(256.000 , 234.8339996446, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    // Create shape of same size
    HVEShape  MyShape(pShape1->GetExtent());

    // Generate scanlines for this dum shape (this insures creation of Y)
    HGFScanLines TheScanLines(false, pWorld);

    // Generate scanlines for our shape
    MyShape.GenerateScanLines(TheScanLines);

    }

//==================================================================================
// Test which failed on aug 26, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  GenerateScanLinesTest3)
    {
 
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00 , 163.61, pWorld), HGF2DLocation(250.25 , 154.50, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(250.25 , 154.50, pWorld), HGF2DLocation(256.00 , 150.05, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00 , 150.05, pWorld), HGF2DLocation(256.00 , 122.01, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00 , 122.01, pWorld), HGF2DLocation(53.254 , 79.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(53.254 , 79.000, pWorld), HGF2DLocation(15.029 , 256.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15.029 , 256.00, pWorld), HGF2DLocation(256.00 , 256.00, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00 , 256.00, pWorld), HGF2DLocation(256.00 , 163.61, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    // Create shape of same size
    HVEShape  MyShape(pShape1->GetExtent());

    // Generate scanlines for this dum shape (this insures creation of Y)
    HGFScanLines  TheScanLines(false, pWorld);

    // Generate scanlines for our shape
    MyShape.GenerateScanLines(TheScanLines);

    }

//==================================================================================
// Test which failed on aug 27, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest)
    {

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2048.00000000000000 , 1097.039892433902, pWorld),
                                         HGF2DLocation(2168.02336591538600 , 986.0000000000011, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2168.02336591538600 , 986.0000000000011, pWorld),
                                         HGF2DLocation(2470.00000000000000 , 986.0000000000011, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2470.00000000000000 , 986.0000000000011, pWorld),
                                         HGF2DLocation(2470.00000000000000 , 704.8262289325814, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2470.00000000000000 , 704.8262289325814, pWorld),
                                          HGF2DLocation(2569.2192862548840 , 611.7017455707912, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2569.21928625488400 , 611.7017455707912, pWorld),
                                         HGF2DLocation(2048.00000000000000 , 611.7017455707912, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2048.00000000000000 , 611.7017455707912, pWorld),
                                         HGF2DLocation(2048.00000000000000 , 1097.039892433902, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1279.05405332091600 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(1280.00000000000000 , 0.00000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1280.00000000000000 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(1280.00000000000000 , 1024.00000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1280.00000000000000 , 1024.00000000000000, pWorld),
                                         HGF2DLocation(1248.98025825285000 , 1024.00000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1248.98025825285000 , 1024.00000000000000, pWorld),
                                         HGF2DLocation(1657.11823802983100 , 1462.06362048594600, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1657.11823802983100 , 1462.06362048594600, pWorld),
                                         HGF2DLocation(2168.02336591538600 , 986.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2168.02336591538600 , 986.000000000000000, pWorld),
                                         HGF2DLocation(1396.00000000000000 , 986.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1396.00000000000000 , 986.000000000000000, pWorld),
                                         HGF2DLocation(1396.00000000000000 , 136.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1396.00000000000000 , 136.000000000000000, pWorld),
                                         HGF2DLocation(2233.00842751780000 , 136.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2233.00842751780000 , 136.000000000000000, pWorld),
                                         HGF2DLocation(1721.75008346975300 , -412.29825442920880, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1721.75008346975300 , -412.29825442920880, pWorld),
                                         HGF2DLocation(1279.05405332091600 , 0.00000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(6663.69082039965724, pResult->CalculateArea());

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(778011.542183140292, pResult->CalculateArea());

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(162573.735755132045, pResult->CalculateArea());

    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(608774.115607607527, pResult->CalculateArea());

    }

//==================================================================================
// Test which failed on sep 15, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest2)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000, 0.00000, pWorld), HGF2DLocation(0.00000, 0.00001, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000, 0.00001, pWorld), HGF2DLocation(0.00001, 0.00001, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00001, 0.00001, pWorld), HGF2DLocation(0.00001, 0.00000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00001, 0.00000, pWorld), HGF2DLocation(0.00000, 0.00000, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , -229.49314192432380, pWorld),
                                         HGF2DLocation(0.0000000000000000 , 453.173524742341900, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 453.173524742341900, pWorld),
                                         HGF2DLocation(682.66666666666630 , 453.173524742341900, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(682.66666666666630 , 453.173524742341900, pWorld),
                                         HGF2DLocation(682.66666666666630 , -229.49314192432380, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(682.66666666666630 , -229.49314192432380, pWorld),
                                         HGF2DLocation(0.0000000000000000 , -229.49314192432380, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(466033.77777777688, pResult->CalculateArea());

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);

    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(466033.77777777682, pResult->CalculateArea());

    }

//==================================================================================
// Test which failed on sep 25, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest3)
    {
 
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 0.000, pWorld), HGF2DLocation(0.000, 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 256.0, pWorld), HGF2DLocation(256.0, 256.0, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 256.0, pWorld), HGF2DLocation(256.0, 0.000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 0.000, pWorld), HGF2DLocation(0.000, 0.000, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 192.000000000000000, pWorld),
                                         HGF2DLocation(26.000000000000430 , 253.521615114176500, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(26.000000000000430 , 253.521615114176500, pWorld),
                                         HGF2DLocation(26.000000000000430 , 409.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(26.000000000000430 , 409.000000000000000, pWorld),
                                         HGF2DLocation(441.00000000000060 , 409.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(441.00000000000060 , 409.000000000000000, pWorld),
                                         HGF2DLocation(441.00000000000060 , 0.00000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(441.00000000000060 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(26.000000000000430 , 0.00000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(26.000000000000430 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(26.000000000000430 , 180.636547785502500, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(26.000000000000430 , 180.636547785502500, pWorld),
                                         HGF2DLocation(0.0000000000000000 , 192.000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(59827.5058752726617, pResult->CalculateArea());

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(176391.00000000017, pResult->CalculateArea());

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(5708.49412472733274, pResult->CalculateArea());

    HVE2DRectangle  Rect1(0.0, 0.0, 256.0, 256.0, pWorld);

    pResult = Rect1.DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(5708.49412472733274, pResult->CalculateArea());

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);

    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(110855.0000000002, pResult->CalculateArea());

    }

//==================================================================================
// Test which failed on sep 26, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest4)
    {
   
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(126.00002333325810 , 374.00007290433640, pWorld),
                                         HGF2DLocation(382.00007298135420 , 374.00007365927220, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007298135420 , 374.00007365927220, pWorld),
                                         HGF2DLocation(382.00007373628990 , 118.00002401117640, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007373628990 , 118.00002401117640, pWorld),
                                         HGF2DLocation(126.00002408819400 , 118.00002325624050, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(126.00002408819400 , 118.00002325624050, pWorld),
                                         HGF2DLocation(126.00002333325810 , 374.00007290433640, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(126.00002405280640 , 130.00002633843170, pWorld),
                                         HGF2DLocation(382.00007298135420 , 130.00002709336770, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007298135420 , 130.00002709336770, pWorld),
                                         HGF2DLocation(382.00007301674200 , 118.00002401117640, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007301674200 , 118.00002401117640, pWorld),
                                         HGF2DLocation(126.00002408819400 , 118.00002325624050, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(126.00002408819400 , 118.00002325624050, pWorld),
                                         HGF2DLocation(126.00002405280640 , 130.00002633843170, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(3072.00156987197760, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0258063547807, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(62464.0240436438907, pResult->CalculateArea());  

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);   

    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    }

//==================================================================================
// Test which failed on sep 26, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest5)
    {

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(126.00002408819410 , 118.00002325624000, pWorld),
                                         HGF2DLocation(382.00007298135470 , 118.00002401117580, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007298135470 , 118.00002401117580, pWorld),
                                         HGF2DLocation(382.00007370090320 , -126.0000233096655, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007370090320 , -126.0000233096655, pWorld),
                                         HGF2DLocation(126.00002480774240 , -126.0000240646014, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(126.00002480774240 , -126.0000240646014, pWorld),
                                         HGF2DLocation(126.00002408819410 , 118.00002325624000, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(126.00002257832220 , -126.0000240646011, pWorld),
                                         HGF2DLocation(126.00002257832220 , 130.00002633843110, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(126.00002257832220 , 130.00002633843110, pWorld),
                                         HGF2DLocation(382.00007298135420 , 130.00002633843110, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007298135420 , 130.00002633843110, pWorld),
                                         HGF2DLocation(382.00007298135420 , -126.0000240646011, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007298135420 , -126.0000240646011, pWorld),
                                         HGF2DLocation(126.00002257832220 , -126.0000240646011, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(62464.0242373325090, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.025898457272, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(8.7784996139817E-5, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);   

    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(3072.0018500711267, pResult->CalculateArea());  

    }

//==================================================================================
// Test which failed on sep 29, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest6)
    {
 
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(600.6736535497 , -864.478207542, pWorld),
                                         HGF2DLocation(598.1357904592, -16.48934551255, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(598.1357904592, -16.48934551255, pWorld),
                                         HGF2DLocation(1210.105822119, -14.33701979160, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1210.105822119, -14.33701979160, pWorld),
                                         HGF2DLocation(1212.643685210, -862.2954950332, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1212.643685210, -862.2954950332, pWorld),
                                         HGF2DLocation(600.6736535497 , -864.478207542, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0213688201825, 0.0000000000000, pWorld),
                                         HGF2DLocation(0.0000000000000, 160.53759742730, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000, 160.53759742730, pWorld),
                                         HGF2DLocation(115.99175663380, 160.16150983050, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(115.99175663380, 160.16150983050, pWorld),
                                         HGF2DLocation(116.01312545400, 0.6239124032448, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(116.01312545400, 0.6239124032448, pWorld),
                                         HGF2DLocation(0.0213688201825, 0.0000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(537503.018409437267, pResult->CalculateArea());

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(518939.973708531179, pResult->CalculateArea());

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);

    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(18563.0447009061026, pResult->CalculateArea());
    
    }
    
//==================================================================================
// Test which failed on sep 29, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest7)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(126.00002257832220 , 630.00012255243180, pWorld),
                                         HGF2DLocation(382.00007222641780 , 630.00012330736810, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007222641780 , 630.00012330736810, pWorld),
                                         HGF2DLocation(382.00007298135420 , 374.00007365927160, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007298135420 , 374.00007365927160, pWorld),
                                         HGF2DLocation(126.00002333325810 , 374.00007290433590, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(126.00002333325810 , 374.00007290433590, pWorld),
                                         HGF2DLocation(126.00002257832220 , 630.00012255243180, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(126.00002329787050 , 386.00007674146330, pWorld),
                                         HGF2DLocation(382.00007298135420 , 386.00007749639910, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007298135420 , 386.00007749639910, pWorld),
                                         HGF2DLocation(382.00007301674200 , 374.00007365927160, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007301674200 , 374.00007365927160, pWorld),
                                         HGF2DLocation(126.00002333325810 , 374.00007290433590, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(126.00002333325810 , 374.00007290433590, pWorld),
                                         HGF2DLocation(126.00002329787050 , 386.00007674146330, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HGF2DLiteSegment LiteSegment1(HGF2DPosition(382.0000729814, 386.0000774964), HGF2DPosition(382.0000729814, 374.0000736593));
    HGF2DLiteSegment LiteSegment2(HGF2DPosition(382.0000729814, 386.0000774964), HGF2DPosition(382.0000730167, 374.0000736593));

    // Resumate of problem
    ASSERT_FALSE(LiteSegment1.Crosses(LiteSegment2));

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(3072.00177219486386, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.02581541425, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(62464.024219213774, pResult->CalculateArea());  

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest8)
    {

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(244.00000000000000 , -0.000001474484110, pWorld),
                                         HGF2DLocation(-12.00000000000014 , -0.000000719548210, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-12.00000000000014 , -0.000000719548210, pWorld),
                                         HGF2DLocation(-11.99999926865598 , 248.00000075493490, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-11.99999926865598 , 248.00000075493490, pWorld),
                                         HGF2DLocation(244.00000073134430 , 247.99999999999890, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(244.00000073134430 , 247.99999999999890, pWorld),
                                         HGF2DLocation(244.00000000000000 , -0.000001474484110, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , -0.000000000000570, pWorld),
                                         HGF2DLocation(0.000000000000000 , 247.99999999999940, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 247.99999999999940, pWorld),
                                         HGF2DLocation(244.0000000000000 , 247.99999999999940, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(244.0000000000000 , 247.99999999999940, pWorld),
                                         HGF2DLocation(244.0000000000000 , -0.000000000000570, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(244.0000000000000 , -0.000000000000570, pWorld),
                                         HGF2DLocation(0.000000000000000 , -0.000000000000570, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HGF2DLiteSegment LiteSegment1(HGF2DPosition(-11.99999926865598, 248.00000075493490), HGF2DPosition(244.00000073134430, 247.99999999999890));
    HGF2DLiteSegment LiteSegment2(HGF2DPosition(0.0000000000000000, 247.99999999999940), HGF2DPosition(244.00000000000000, 247.99999999999940));

    // Resumate of problem
    ASSERT_FALSE(LiteSegment1.Crosses(LiteSegment2));

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(60512.000000000000, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(63488.000377467717, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(2976.0003774677243, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest9)
    {
           
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00002509535310 , -374.00007328770210, pWorld),
                                         HGF2DLocation(382.00007298135420 , -374.00007365927220, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007298135420 , -374.00007365927220, pWorld),
                                         HGF2DLocation(382.00007308751720 , -337.99994959696720, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007308751720 , -337.99994959696720, pWorld),
                                         HGF2DLocation(256.00002520151610 , -337.99994922539710, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00002520151610 , -337.99994922539710, pWorld),
                                         HGF2DLocation(256.00002509535310 , -374.00007328770210, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.00002520151610 , -593.99997479848390, pWorld),
                                         HGF2DLocation(256.00002520151610 , -337.99994959696780, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.00002520151610 , -337.99994959696780, pWorld),
                                         HGF2DLocation(512.00005040303220 , -337.99994959696780, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(512.00005040303220 , -337.99994959696780, pWorld),
                                         HGF2DLocation(512.00005040303220 , -593.99997479848390, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(512.00005040303220 , -593.99997479848390, pWorld),
                                         HGF2DLocation(256.00002520151610 , -593.99997479848390, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    // Resumate of problem

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(4536.01735575240491, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.012928496857, pResult->CalculateArea());   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(2.3408963415317511E-5, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(60999.995572744461, pResult->CalculateArea());
      
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest10) 
    {

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(198.00002966624810 , 748.00013416875620, pWorld),
                                         HGF2DLocation(349.83085012412430 , 748.00013461576940, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(349.83085012412430 , 748.00013461576940, pWorld),
                                         HGF2DLocation(349.83085029172540 , 691.66168779997070, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(349.83085029172540 , 691.66168779997070, pWorld),
                                         HGF2DLocation(198.00002983384950 , 691.66168735295740, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(198.00002983384950 , 691.66168735295740, pWorld),
                                         HGF2DLocation(198.00002966624810 , 748.00013416875620, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 691.66168779997180, pWorld),
                                         HGF2DLocation(0.0000000000000000 , 1040.4925379240950, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 1040.4925379240950, pWorld),
                                         HGF2DLocation(349.83085012412430 , 1040.4925379240950, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(349.83085012412430 , 1040.4925379240950, pWorld),
                                         HGF2DLocation(349.83085012412430 , 691.66168779997180, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(349.83085012412430 , 691.66168779997180, pWorld),
                                         HGF2DLocation(0.0000000000000000 , 691.66168779997180, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(8553.91260336497543, pResult->CalculateArea());  

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(122031.79291161057, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(3.3935279405168E-5, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(113477.880283734411, pResult->CalculateArea());    
     
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest11)
    {
  
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(243.99999999999970 , -0.00000150987170, pWorld),
                                         HGF2DLocation(-267.9999511068415 , 0.000000000000080, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-267.9999511068415 , 0.000000000000080, pWorld),
                                         HGF2DLocation(-267.9999499685396 , 385.9999522451429, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-267.9999499685396 , 385.9999522451429, pWorld),
                                         HGF2DLocation(244.00000113830140 , 385.9999507352709, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(244.00000113830140 , 385.9999507352709, pWorld),
                                         HGF2DLocation(243.99999999999970 , -0.00000150987170, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 0.00000000000014, pWorld),
                                         HGF2DLocation(0.000000000000000 , 256.000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 256.000000000000, pWorld),
                                         HGF2DLocation(244.0000000000000 , 256.000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(244.0000000000000 , 256.000000000000, pWorld),
                                         HGF2DLocation(244.0000000000000 , 0.00000000000014, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(244.0000000000000 , 0.00000000000014, pWorld),
                                         HGF2DLocation(0.000000000000000 , 0.00000000000014, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(62463.999999999964, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(197631.95667675603, pResult->CalculateArea());   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(135167.95667675606, pResult->CalculateArea());   

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest12)
    {
         
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(78.50000752018806 , 15.50000145759122, pWorld),
                                         HGF2DLocation(78.50000756663425 , 0.000000185785010, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(78.50000756663425 , 0.000000185785010, pWorld),
                                         HGF2DLocation(15.50000150403761 , 0.000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15.50000150403761 , 0.000000000000000, pWorld),
                                         HGF2DLocation(15.50000150403761 , 15.50000145759122, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15.50000150403761 , 15.50000145759122, pWorld),
                                         HGF2DLocation(0.000000000000000 , 15.50000145759122, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 15.50000145759122, pWorld),
                                         HGF2DLocation(0.000000000000000 , 121.9999909979776, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 121.9999909979776, pWorld),
                                         HGF2DLocation(134.4999867839708 , 121.9999909979776, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(134.4999867839708 , 121.9999909979776, pWorld),
                                         HGF2DLocation(134.4999867839708 , 15.50000145759122, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(134.4999867839708 , 15.50000145759122, pWorld),
                                         HGF2DLocation(78.50000752018806 , 15.50000145759122, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(0.00000000000000 , 663.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000 , 663.000000000000000, pWorld),
                                         HGF2DLocation(841.000000000000 , 663.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(841.000000000000 , 663.000000000000000, pWorld),
                                         HGF2DLocation(841.000000000000 , 0.00000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(841.000000000000 , 0.00000000000000000, pWorld),
                                         HGF2DLocation(0.00000000000000 , 0.00000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(15300.7473652613134, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(557583.000000000000, pResult->CalculateArea());   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(542282.252634738688, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest13)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1.4309074174435E-5, 439.0001100173, pWorld),
                                         HGF2DLocation(256.00006395720000, 439.0001107722, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00006395720000, 439.0001107722, pWorld),
                                         HGF2DLocation(256.00006467670000, 194.9999970023, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00006467670000, 194.9999970023, pWorld),
                                         HGF2DLocation(1.5028622470936E-5, 194.9999962474, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1.5028622470936E-5, 194.9999962474, pWorld),
                                         HGF2DLocation(1.4309074174435E-5, 439.0001100173, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000, 194.9999970023, pWorld),
                                         HGF2DLocation(0.000000000000000, 707.0000306043, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000, 707.0000306043, pWorld),
                                         HGF2DLocation(512.0000336020000, 707.0000306043, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(512.0000336020000, 707.0000306043, pWorld),
                                         HGF2DLocation(512.0000336020000, 194.9999970023, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(512.0000336020000, 194.9999970023, pWorld),
                                         HGF2DLocation(0.000000000000000, 194.9999970023, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(62464.041414800828, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(262144.03460170358, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(9.6627220595548361E-5, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(199679.9932658395, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on sep 30, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest14)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007298135420 , 374.00007365927220, pWorld),
                                         HGF2DLocation(630.00012107794650 , 374.00007439061620, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(630.00012107794650 , 374.00007439061620, pWorld),
                                         HGF2DLocation(630.00012179749550 , 130.00002633843140, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(630.00012179749550 , 130.00002633843140, pWorld),
                                         HGF2DLocation(382.00007370090260 , 130.00002560708710, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007370090260 , 130.00002560708710, pWorld),
                                         HGF2DLocation(382.00007298135420 , 374.00007365927220, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007298135470 , 130.00002633843110, pWorld),
                                         HGF2DLocation(382.00007298135470 , 386.00007674146330, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(382.00007298135470 , 386.00007674146330, pWorld),
                                         HGF2DLocation(638.00012338438640 , 386.00007674146330, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(638.00012338438640 , 386.00007674146330, pWorld),
                                         HGF2DLocation(638.00012338438640 , 130.00002633843110, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(638.00012338438640 , 130.00002633843110, pWorld),
                                         HGF2DLocation(382.00007298135470 , 130.00002633843110, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(60512.023828082718, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.025899966946, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(9.0686672104080078E-5, pResult->CalculateArea());   

    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(5024.0022445288269, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on oct 1, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest15)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(6.5414858785482000 , 204.15769530740250, pWorld),
                                         HGF2DLocation(256.00000000000000 , 204.15769457208940, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000000 , 204.15769457208940, pWorld),
                                         HGF2DLocation(256.00000015211980 , 256.00000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000015211980 , 256.00000000000000, pWorld),
                                         HGF2DLocation(6.5414860306680100 , 256.00000073531340, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(6.5414860306680100 , 256.00000073531340, pWorld),
                                         HGF2DLocation(6.5414858785482000 , 204.15769530740250, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 0.000000000000000, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 256.0000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 256.0000000000000, pWorld),
                                         HGF2DLocation(256.000000000000000 , 256.0000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000 , 256.0000000000000, pWorld),
                                         HGF2DLocation(256.000000000000000 , 0.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000 , 0.000000000000000, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 0.000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(12932.5044806770511, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0000956582080, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(9.5658213467686437E-5, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(52603.4956149811259, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on oct 1, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest16)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(243.99999924491790 , -192.00000147448450, pWorld),
                                         HGF2DLocation(-12.00000151016373 , -192.00000071954870, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-12.00000151016373 , -192.00000071954870, pWorld),
                                         HGF2DLocation(-12.00000075522783 , 64.0000000355332500, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-12.00000075522783 , 64.0000000355332500, pWorld),
                                         HGF2DLocation(243.99999999985360 , 63.9999992805972900, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(243.99999999985360 , 63.9999992805972900, pWorld),
                                         HGF2DLocation(243.99999924491790 , -192.00000147448450, pWorld)));

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-0.00000000000057 , 0.000000000000000, pWorld),
                                         HGF2DLocation(-0.00000000000057 , 64.00000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-0.00000000000057 , 64.00000000000000, pWorld),
                                         HGF2DLocation(63.99999999999943 , 64.00000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(63.99999999999943 , 64.00000000000000, pWorld),
                                         HGF2DLocation(63.99999999999943 , 0.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(63.99999999999943 , 0.000000000000000, pWorld),
                                         HGF2DLocation(-0.00000000000057 , 0.000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);
    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(4096.0, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.000393758528, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(61440.000392631118, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->DifferentiateFromShapeSCS(*pShape2);
    ASSERT_DOUBLE_EQ(6.0382653828128241E-6, pResult->CalculateArea());  

    }

//==================================================================================
// Test which failed on oct 3, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  GenerateScanLines4)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0000000000000 , 38.81924348805597, pWorld),
                                         HGF2DLocation(194.7608117356506 , 0.000000000000570, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(194.7608117356506 , 0.000000000000570, pWorld),
                                         HGF2DLocation(51.08487400820127 , 0.000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(51.08487400820127 , 0.000000000000000, pWorld),
                                         HGF2DLocation(0.000000000000000 , 82.10911284920758, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 82.10911284920758, pWorld),
                                         HGF2DLocation(0.000000000001140 , 180.8504152071451, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000001140 , 180.8504152071451, pWorld),
                                         HGF2DLocation(11.00000000000000 , 187.5860439171718, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(11.00000000000000 , 187.5860439171718, pWorld),
                                         HGF2DLocation(11.00000000000000 , 71.00000000000085, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(11.00000000000000 , 71.00000000000085, pWorld),
                                         HGF2DLocation(223.5000000000011 , 71.00000000000085, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(223.5000000000011 , 71.00000000000085, pWorld),
                                         HGF2DLocation(223.5000000000011 , 190.0000000000009, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(223.5000000000011 , 190.0000000000009, pWorld),
                                         HGF2DLocation(16.47350767121384 , 190.0000000000006, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(16.47350767121384 , 190.0000000000006, pWorld),
                                         HGF2DLocation(121.6926241931026 , 256.0000000000006, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(121.6926241931026 , 256.0000000000006, pWorld),
                                         HGF2DLocation(193.1160457272902 , 256.0000000000011, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(193.1160457272902 , 256.0000000000011, pWorld),
                                         HGF2DLocation(256.0000000000000 , 156.1773003917556, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0000000000000 , 156.1773003917556, pWorld),
                                         HGF2DLocation(256.0000000000000 , 38.81924348805597, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    // Create shape of same size
    HVEShape  MyShape(*pShape1);

    // Generate scanlines for this dum shape (this insures creation of Y)
    HGFScanLines  TheScanLines(false, pWorld);

    MyShape.GenerateScanLines(TheScanLines);

    }

//==================================================================================
// Test which failed on oct 6, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  CalculateSpatialPositionOfNonCrossingLinearTest)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(200554.36158946010000 , -6398.04597202595300, pWorld),
                                         HGF2DLocation(196150.83067803180000 , -53764.2531169768300, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(196150.83067803180000 , -53764.2531169768300, pWorld),
                                         HGF2DLocation(172671.65121527440000 , -51581.2596558242800, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(172671.65121527440000 , -51581.2596558242800, pWorld),
                                         HGF2DLocation(172671.65121527440000 , -6398.04597202595300, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(172671.65121527440000 , -6398.04597202595300, pWorld),
                                         HGF2DLocation(200554.36158946010000 , -6398.04597202595300, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DSegment    MySegment(HGF2DLocation(200554.36158945980000 , -6398.04597202595300, pWorld),
                              HGF2DLocation(198455.29354262730000 , -28973.6121477180200, pWorld));

    pShape1->CalculateSpatialPositionOfNonCrossingLinear(MySegment);

    }

//==================================================================================
// Test which failed on dec 16, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest17)
    {
        
    HVE2DRectangle  Shape1(-3579929.748470003, 85666005.23929002, 797561.8597685, 90043496.84753, pWorld);

    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-3220838.030607, 85666005.23929, pWorld),
                                         HGF2DLocation(-3579929.748470, 85666005.23929, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-3579929.748470, 85666005.23929, pWorld),
                                         HGF2DLocation(-3579929.748470, 90043496.84753, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-3579929.748470, 90043496.84753, pWorld),
                                         HGF2DLocation(-3220838.030607, 90043496.84753, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-3220838.030607, 90043496.84753, pWorld),
                                         HGF2DLocation(-3220838.030607, 85666005.23929, pWorld)));

    HVE2DPolygonOfSegments  Shape2(TheLinear1);

    HFCPtr<HVE2DShape> pResult = Shape1.IntersectShape(Shape2);
    ASSERT_DOUBLE_EQ(1.57192098153377148E12, pResult->CalculateArea());
    
    pResult = Shape1.DifferentiateShape(Shape2);
    ASSERT_DOUBLE_EQ(1.75905117986713203E13, pResult->CalculateArea());
    
    pResult = Shape1.UnifyShape(Shape2);
    ASSERT_DOUBLE_EQ(1.91624327802050390E13, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on dec 19, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest18)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-3579929.748469676, 76911022.02281535, pWorld),
                                         HGF2DLocation(-3579929.748469676, 81288513.63105357, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-3579929.748469676, 81288513.63105357, pWorld),
                                         HGF2DLocation(797561.85976853970, 81288513.63105357, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(797561.85976853970, 81288513.63105357, pWorld),
                                         HGF2DLocation(797561.85976853970, 76911022.02281535, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(797561.85976853970, 76911022.02281535, pWorld),
                                         HGF2DLocation(-3579929.748469676, 76911022.02281535, pWorld)));

    HVE2DPolygonOfSegments  Shape1(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-3220838.030606692, 76911022.02281520, pWorld),
                                         HGF2DLocation(-3579929.748469797, 76911022.02281520, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-3579929.748469797, 76911022.02281520, pWorld),
                                         HGF2DLocation(-3579929.748469797, 81288513.63105103, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-3579929.748469797, 81288513.63105103, pWorld),
                                         HGF2DLocation(-3220838.030606692, 81288513.63105103, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-3220838.030606692, 81288513.63105103, pWorld),
                                         HGF2DLocation(-3220838.030606692, 76911022.02281520, pWorld)));

    HVE2DPolygonOfSegments  Shape2(TheLinear2);

    HFCPtr<HVE2DShape> pResult = Shape1.IntersectShape(Shape2);
    ASSERT_DOUBLE_EQ(1.57192098153273168E12, pResult->CalculateArea());    

    pResult = Shape1.DifferentiateShape(Shape2);
    ASSERT_DOUBLE_EQ(1.75905117986635605E13, pResult->CalculateArea());
    
    pResult = Shape1.UnifyShape(Shape2);
    ASSERT_DOUBLE_EQ(19162432780191.0660000, pResult->CalculateArea());    

    }

//==================================================================================
// Test which failed on Jan 16 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  IntersectShape3)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(343186.75207202940000, 5067813.55207243700000, pWorld),
                                         HGF2DLocation(343186.75207202940000, 5062784.82890859200000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(343186.75207202940000, 5062784.82890859200000, pWorld),
                                         HGF2DLocation(343666.32587660310000, 5062784.82890859200000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(343666.32587660310000, 5062784.82890859200000, pWorld),
                                         HGF2DLocation(343656.75207202940000, 5067814.82890859200000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(343656.75207202940000, 5067814.82890859200000, pWorld),
                                         HGF2DLocation(343656.75206638850000, 5067814.82890858300000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(343656.75206638850000, 5067814.82890858300000, pWorld),
                                         HGF2DLocation(343186.75207202940000, 5067813.55207243700000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(343656.7520720294, 5067814.828908592, pWorld),
                                         HGF2DLocation(343186.7520720294, 5067813.552072437, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(343186.7520720294, 5067813.552072437, pWorld),
                                         HGF2DLocation(343186.7520720294, 5062784.828908592, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(343186.7520720294, 5062784.828908592, pWorld),
                                         HGF2DLocation(343666.3258766031, 5062784.828908592, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(343666.3258766031, 5062784.828908592, pWorld),
                                         HGF2DLocation(343656.7520720294, 5067814.828908592, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_DOUBLE_EQ(2387878.06200763490, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(2387878.06200623512, pResult->CalculateArea());  

    }

//==================================================================================
// Test which failed on Jan 19 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest20)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2029951690.00000200000000 , 2069995506.9999980000000, pWorld),
                                         HGF2DLocation(2029909908.05370600000000 , 2070045299.8880250000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2029909908.05370600000000 , 2070045299.8880250000000, pWorld),
                                         HGF2DLocation(2029978852.05251400000000 , 2070103150.7367420000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2029978852.05251400000000 , 2070103150.7367420000000, pWorld),
                                         HGF2DLocation(2030020633.99881000000000 , 2070053357.8487150000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2030020633.99881000000000 , 2070053357.8487150000000, pWorld),
                                         HGF2DLocation(2029951690.00000200000000 , 2069995506.9999980000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029951690.00000200000000 , 2069995506.9999980000000, pWorld),
                                         HGF2DLocation(2029945974.05895500000000 , 2070002317.8393110000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029945974.05895500000000 , 2070002317.8393110000000, pWorld),
                                         HGF2DLocation(2029945974.05895500000000 , 2070031573.0052470000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029945974.05895500000000 , 2070031573.0052470000000, pWorld),
                                         HGF2DLocation(2029982041.06420400000000 , 2070031573.0052470000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029982041.06420400000000 , 2070031573.0052470000000, pWorld),
                                         HGF2DLocation(2029982041.06420400000000 , 2070020974.1829270000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029982041.06420400000000 , 2070020974.1829270000000, pWorld),
                                         HGF2DLocation(2029951690.00000200000000 , 2069995506.9999980000000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_DOUBLE_EQ(894841033.535156250, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(894841033.531250000, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(5850050404.10156250, pResult->CalculateArea());  

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(5850050404.09375000, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(4955200833.53515625, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(8537.03906250000000, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(894841033.531250000, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on Jan 20 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest21)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000 , -1.49011611938480E-7, pWorld), HGF2DLocation(0.000 , 255.9999998509884010, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000 , 255.9999998509884010, pWorld), HGF2DLocation(256.0 , 255.9999998509884010, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , 255.9999998509884010, pWorld), HGF2DLocation(256.0 , -1.49011611938480E-7, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0 , -1.49011611938480E-7, pWorld), HGF2DLocation(0.000 , -1.49011611938480E-7 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( -1.4901161193848E-7, 237.1464065313000000, pWorld),
                                         HGF2DLocation( -1.4901161193848E-7, 255.9999997020000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( -1.4901161193848E-7, 255.9999997020000000, pWorld),
                                         HGF2DLocation( 256.000000000000000, 255.9999998509884010, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 256.000000000000000, 255.9999998509884010, pWorld),
                                         HGF2DLocation( 256.000000000000000, -1.49011611938480E-7, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 256.000000000000000, -1.49011611938480E-7, pWorld),
                                         HGF2DLocation( 198.628075212200000, -1.49011611938480E-7, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 198.628075212200000, -1.49011611938480E-7, pWorld),
                                         HGF2DLocation( -1.4901161193848E-7, 237.1464065313000000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_DOUBLE_EQ(41984.0328473983172, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(41984.0328473983172, pResult->CalculateArea());  

    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(65536.0000028094070, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0000028094070, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(23551.9671335311686, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(2.8094043224535217E-6, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(41984.0328473983099, pResult->CalculateArea());
        
    }

//==================================================================================
// Test which failed on Jan 20 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest22)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000000 , 0.00000000000000000, pWorld), 
                                         HGF2DLocation(-1.4901161193848E-7 , 256.000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-1.4901161193848E-7 , 256.000000000000000, pWorld),
                                         HGF2DLocation(256.000000000000000 , 256.000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000 , 256.000000000000000, pWorld),
                                         HGF2DLocation(256.000000000000000 , -1.4901161193848E-7, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.000000000000000 , -1.4901161193848E-7, pWorld),
                                         HGF2DLocation(0.00000000000000000 , 0.00000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 0.000, 0.000, pWorld), HGF2DLocation( 0.000, 256.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 0.000, 256.0, pWorld), HGF2DLocation( 256.0, 256.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 256.0, 256.0, pWorld), HGF2DLocation( 256.0, 0.000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 256.0, 0.000, pWorld), HGF2DLocation( 0.000, 0.000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_DOUBLE_EQ(65536.0000000000000, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0000000000000, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(65536.000038146973, pResult->CalculateArea());  

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.000038146973, pResult->CalculateArea());  

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0,pResult->CalculateArea(), MYEPSILON);  

    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0,pResult->CalculateArea(), MYEPSILON);    

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0000000000000, pResult->CalculateArea());   

    }

//==================================================================================
// Test which failed on Jan 20 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest23)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-1.49011611938480E-7, -1.49011611938480E-7, pWorld),
                                         HGF2DLocation(-1.49011611938480E-7, 255.9999998509884010, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-1.49011611938480E-7, 255.9999998509884010, pWorld),
                                         HGF2DLocation(255.9999998509884010, 256.0000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(255.9999998509884010, 256.0000000000000000, pWorld),
                                         HGF2DLocation(255.9999998509884010, 0.000000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(255.9999998509884010, 0.000000000000000000, pWorld),
                                         HGF2DLocation(-1.49011611938480E-7, -1.49011611938480E-7, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-1.49012748806850E-7, -2.2737367544323E-12, pWorld),
                                         HGF2DLocation(-1.49012748806850E-7, 256.0000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-1.49012748806850E-7, 256.0000000000000000, pWorld),
                                         HGF2DLocation(255.9999998509884010, 256.0000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(255.9999998509884010, 256.0000000000000000, pWorld),
                                         HGF2DLocation(255.9999998509884010, 0.000000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(255.9999998509884010, 0.000000000000000000, pWorld),
                                         HGF2DLocation(-1.49012748806850E-7, -2.2737367544323E-12, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_DOUBLE_EQ(65536.0000000000000, pResult->CalculateArea());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.000000000582, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(65536.000000000582, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.0000000000000, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    }

//==================================================================================
// Test which failed on Jan 20 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest24)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000, -9.31322574615480E-8, pWorld),
                                         HGF2DLocation(0.000000000000, 255.9999998509884010, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000, 255.9999998509884010, pWorld),
                                         HGF2DLocation(63.77044698596, 255.9999999255000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(63.77044698596, 255.9999999255000000, pWorld),
                                         HGF2DLocation(90.83728918433, -3.72529029846190E-8, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(90.83728918433, -3.72529029846190E-8, pWorld),
                                         HGF2DLocation(0.000000000000, -9.31322574615480E-8, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 0.000, 0.000, pWorld), HGF2DLocation( 0.000, 256.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 0.000, 256.0, pWorld), HGF2DLocation( 256.0, 256.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 256.0, 256.0, pWorld), HGF2DLocation( 256.0, 0.000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation( 256.0, 0.000, pWorld), HGF2DLocation( 0.000, 0.000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_DOUBLE_EQ(19789.7902270799204, pResult->CalculateArea());  

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(19789.7902270799204, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(65536.000023841858, pResult->CalculateArea());  

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(65536.000000000000, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(45746.2097676311241, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on Jan 21 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest25)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2029947377.103, 2070045465.066, pWorld),
                                         HGF2DLocation(2029947377.103, 2070057954.083, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2029947377.103, 2070057954.083, pWorld),
                                         HGF2DLocation(2029959866.120, 2070057954.083, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2029959866.120, 2070057954.083, pWorld),
                                         HGF2DLocation(2029959866.120, 2070045465.066, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2029959866.120, 2070045465.066, pWorld),
                                         HGF2DLocation(2029947377.103, 2070045465.066, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029947377.103, 2070045465.066 , pWorld),
                                         HGF2DLocation(2029947377.103, 2070057954.083 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029947377.103, 2070057954.083 , pWorld),
                                         HGF2DLocation(2029959866.120, 2070057954.083 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029959866.120, 2070057954.083 , pWorld),
                                         HGF2DLocation(2029959866.120, 2070045465.066 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2029959866.120, 2070045465.066 , pWorld),
                                         HGF2DLocation(2029947377.103, 2070045465.066 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(155975545.625288516, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(155975545.625000000, pResult->CalculateArea());    

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(155975545.625000000, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);    

    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    }

//==================================================================================
// Test which failed on aug 26, 1997
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  RasterizeTest)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000000 , 85.07206146931276, pWorld),
                                         HGF2DLocation(153.57488331542120 , 149.9849718410987, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(153.57488331542120 , 149.9849718410987, pWorld),
                                         HGF2DLocation(111.24160724152170 , 84.32326121302322, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(111.24160724152170 , 84.32326121302322, pWorld),
                                         HGF2DLocation(98.837432157219150 , 94.74092901172116, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(98.837432157219150 , 94.74092901172116, pWorld),
                                         HGF2DLocation(178.61339551983110 , 222.7580927154049, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(178.61339551983110 , 222.7580927154049, pWorld),
                                         HGF2DLocation(256.00000000000000 , 173.4535306419712, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.00000000000000 , 173.4535306419712, pWorld),
                                         HGF2DLocation(256.00000000000000 , 85.07206146931276, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    // Create shape of same size
    HVEShape  MyShape(pShape1->GetExtent());

    // Generate scanlines for this dum shape (this insures creation of Y)
    HGFScanLines  TheScanLines(false, pWorld);

    pShape1->Rasterize(TheScanLines);

    }

//==================================================================================
// Test which failed on Feb 09 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest26)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15282.44160147476000 , -10973.174321865150 , pWorld),
                                         HGF2DLocation(28082.24664637727000 , -10996.576352700050 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(28082.24664637727000 , -10996.576352700050 , pWorld),
                                         HGF2DLocation(28104.64867721215000 , 1803.22869220245000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(28104.64867721215000 , 1803.22869220245000 , pWorld),
                                         HGF2DLocation(15304.84363230965000 , 1826.63072303734600 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(15304.84363230965000 , 1826.63072303734600 , pWorld),
                                         HGF2DLocation(15282.44160147476000 , -10973.174321865150 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(15282.44160147572000 , -11041.3617278322900 , pWorld),
                                         HGF2DLocation(15282.44160147572000 , 14603.05242364347000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(15282.44160147572000 , 14603.05242364347000 , pWorld),
                                         HGF2DLocation(40926.85575295152000 , 14603.05242364347000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(40926.85575295152000 , 14603.05242364347000 , pWorld),
                                         HGF2DLocation(40926.85575295152000 , -11041.3617278322900 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(40926.85575295152000 , -11041.3617278322900 , pWorld),
                                         HGF2DLocation(15282.44160147572000 , -11041.3617278322900 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(163835533.440527886, pResult->CalculateArea());    

    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(657635977.172411203, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(657635977.172411203, pResult->CalculateArea());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(493800443.731883287, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on August 18 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest28)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-4.8885340220295E-11 , 0.000000000000 , pWorld),
                                         HGF2DLocation(110.8485265112000000 , 63.47435208261 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(110.8485265112000000 , 63.47435208261 , pWorld),
                                         HGF2DLocation(-2.8421709430404E-13 , 256.0000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-2.8421709430404E-13 , 256.0000000000 , pWorld),
                                         HGF2DLocation(-256.000000000000000 , 256.0000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-256.000000000000000 , 256.0000000000 , pWorld),
                                         HGF2DLocation(-256.000000000000000 , 0.000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-256.000000000000000 , 0.000000000000 , pWorld),
                                         HGF2DLocation(-4.8885340220295E-11 , 0.000000000000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 0.000, pWorld), HGF2DLocation(0.000, 256.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 256.0, pWorld), HGF2DLocation(128.0, 256.0, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(128.0, 256.0, pWorld), HGF2DLocation(128.0, 0.000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(128.0, 0.000, pWorld), HGF2DLocation(0.000, 0.000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(14188.6113934383083, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(98304.0000000000000, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(98304.0000000000000, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(65535.999999999927, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(18579.3886065664264, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on October 7 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest29)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.97254913127000 , 36528.98034829492000 , pWorld),
                                         HGF2DLocation(-10362.97254332386000 , 36528.98035521596000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.97254332386000 , 36528.98035521596000 , pWorld),
                                         HGF2DLocation(-10362.97254999999000 , 36528.98034999997000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.97254999999000 , 36528.98034999997000 , pWorld),
                                         HGF2DLocation(-65772.30034309317000 , 107449.6585960491000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-65772.30034309317000 , 107449.6585960491000 , pWorld),
                                         HGF2DLocation(-14551.31049872443000 , 147467.6175577276000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-14551.31049872443000 , 147467.6175577276000 , pWorld),
                                         HGF2DLocation(36652.408752940860000 , 81930.06325922805000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(36652.408752940860000 , 81930.06325922805000 , pWorld),
                                         HGF2DLocation(100362.97255166530000 , 28471.01965472511000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(100362.97255166530000 , 28471.01965472511000 , pWorld),
                                         HGF2DLocation(58581.026255414790000 , -21321.8683726104500, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(58581.026255414790000 , -21321.8683726104500 , pWorld),
                                         HGF2DLocation(-10362.97254455088000 , 36528.98033930533000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.97254455088000 , 36528.98033930533000 , pWorld),
                                         HGF2DLocation(30496.477215592730000 , -43661.8914195308200, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(30496.477215592730000 , -43661.8914195308200 , pWorld),
                                         HGF2DLocation(-27418.76350684626000 , -73170.7162502366700, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-27418.76350684626000 , -73170.7162502366700, pWorld),
                                         HGF2DLocation(-68278.21327243885000 , 7019.155519294152000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-68278.21327243885000 , 7019.155519294152000, pWorld),
                                         HGF2DLocation(-10362.97255000004000 , 36528.98034999997000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.97255000004000 , 36528.98034999997000, pWorld),
                                         HGF2DLocation(-10362.97254913127000 , 36528.98034829492000, pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-87507.54318190165000 , 82882.24776904780000, pWorld),
                                         HGF2DLocation(-10362.97254999990000 , 36528.98035000004000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.97254999990000 , 36528.98035000004000, pWorld),
                                         HGF2DLocation(-68278.21327243885000 , 7019.155519294152000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-68278.21327243885000 , 7019.155519294152000, pWorld),
                                         HGF2DLocation(-59816.42063631593000 , -9587.65065051119000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-59816.42063631593000 , -9587.65065051119000, pWorld),
                                         HGF2DLocation(-120985.2918734361000 , 27166.50231267436000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-120985.2918734361000 , 27166.50231267436000, pWorld),
                                         HGF2DLocation(-87507.54318190165000 , 82882.24776904780000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(21744448777.670792, pResult->CalculateArea());   

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(21744448777.670792, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(17105949296.653196, pResult->CalculateArea());   

    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(4638498952.5197973, pResult->CalculateArea());  

    }

//==================================================================================
// Another Test which failed on October 7 1998
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest30)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.97255000001000 , 36528.98034999997000 , pWorld),
                                         HGF2DLocation(-97689.12619839687000 , 58301.68638970102000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-97689.12619839687000 , 58301.68638970102000 , pWorld),
                                         HGF2DLocation(-81964.89405861273000 , 121371.9084690989000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-81964.89405861273000 , 121371.9084690989000 , pWorld),
                                         HGF2DLocation(5362.2595897841020000 , 99598.20242939773000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(5362.2595897841020000 , 99598.20242939773000 , pWorld),
                                         HGF2DLocation(-10362.97255000001000 , 36528.98034999997000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.97254836835000 , 36528.98034920412000 , pWorld),
                                         HGF2DLocation(-10362.97254332386000 , 36528.98035521596000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.97254332386000 , 36528.98035521596000 , pWorld),
                                         HGF2DLocation(-10362.97254999999000 , 36528.98034999997000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.97254999999000 , 36528.98034999997000 , pWorld),
                                         HGF2DLocation(-65772.30034309317000 , 107449.6585960491000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-65772.30034309317000 , 107449.6585960491000 , pWorld),
                                         HGF2DLocation(-14551.31049872443000 , 147467.6175577276000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-14551.31049872443000 , 147467.6175577276000 , pWorld),
                                         HGF2DLocation(36652.408752940860000 , 81930.06325922805000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(36652.408752940860000 , 81930.06325922805000 , pWorld),
                                         HGF2DLocation(100362.97255166530000 , 28471.01965472511000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(100362.97255166530000 , 28471.01965472511000 , pWorld),
                                         HGF2DLocation(65684.981222158590000 , -12856.2808520035500 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(65684.981222158590000 , -12856.2808520035500 , pWorld),
                                         HGF2DLocation(42034.423706349950000 , -61346.1818546281700 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(42034.423706349950000 , -61346.1818546281700 , pWorld),
                                         HGF2DLocation(18406.218307773900000 , -49821.6501835301300 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(18406.218307773900000 , -49821.6501835301300 , pWorld),
                                         HGF2DLocation(-27418.76350684626000 , -73170.7162502366700 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-27418.76350684626000 , -73170.7162502366700 , pWorld),
                                         HGF2DLocation(-59816.42063631593000 , -9587.65065051119000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-59816.42063631593000 , -9587.65065051119000 , pWorld),
                                         HGF2DLocation(-120985.2918734361000 , 27166.50231267436000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-120985.2918734361000 , 27166.50231267436000 , pWorld),
                                         HGF2DLocation(-87507.54318190165000 , 82882.24776904780000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-87507.54318190165000 , 82882.24776904780000 , pWorld),
                                         HGF2DLocation(-10362.97254999990000 , 36528.98035000004000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.97254999990000 , 36528.98035000004000 , pWorld),
                                         HGF2DLocation(-10362.97254836835000 , 36528.98034920412000 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(3819660456.68523979, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(25949063880.0947456, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(25949063880.0947456, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(2030385151.8416114, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(20099018271.567894, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on January 4 1999
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest31)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(251.38915521104350 , 1878.28020202368500 , pWorld),
                                         HGF2DLocation(0.0000000000000000 , 2306.38940237835000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.0000000000000000 , 2306.38940237835000 , pWorld),
                                         HGF2DLocation(556.08400000003170 , 2619.05740238167300 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(556.08400000003170 , 2619.05740238167300 , pWorld),
                                         HGF2DLocation(597.64199999975970 , 2533.97540238313400 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(597.64199999975970 , 2533.97540238313400 , pWorld),
                                         HGF2DLocation(1157.6920000000390 , 2942.62340237759100 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1157.6920000000390 , 2942.62340237759100 , pWorld),
                                         HGF2DLocation(1360.9571153306170 , 2597.07270631380400 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1360.9571153306170 , 2597.07270631380400 , pWorld),
                                         HGF2DLocation(1363.2963474688590 , 2598.42491989955300 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1363.2963474688590 , 2598.42491989955300 , pWorld),
                                         HGF2DLocation(1736.4059474691750 , 2032.88579989969700 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1736.4059474691750 , 2032.88579989969700 , pWorld),
                                         HGF2DLocation(1709.1182893974470 , 1793.80524571239900 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1709.1182893974470 , 1793.80524571239900 , pWorld),
                                         HGF2DLocation(1945.2163260177480 , 1445.51002201065400 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1945.2163260177480 , 1445.51002201065400 , pWorld),
                                         HGF2DLocation(1695.0075410180730 , 1287.39685107581300 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1695.0075410180730 , 1287.39685107581300 , pWorld),
                                         HGF2DLocation(1697.7009916447570 , 1281.23749805241800 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1697.7009916447570 , 1281.23749805241800 , pWorld),
                                         HGF2DLocation(1737.9155931496060 , 1230.36334732361100 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1737.9155931496060 , 1230.36334732361100 , pWorld),
                                         HGF2DLocation(1973.0554095498520 , 1395.65000000037300 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1973.0554095498520 , 1395.65000000037300 , pWorld),
                                         HGF2DLocation(2247.8054095502010 , 986.100000005215400 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2247.8054095502010 , 986.100000005215400 , pWorld),
                                         HGF2DLocation(2179.4054095501780 , 927.000000007450600 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2179.4054095501780 , 927.000000007450600 , pWorld),
                                         HGF2DLocation(2382.6054095502480 , 644.950000004842900 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(2382.6054095502480 , 644.950000004842900 , pWorld),
                                         HGF2DLocation(1269.0054095501550 , 0.00000000000000000, pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1269.0054095501550 , 0.00000000000000000 , pWorld),
                                         HGF2DLocation(797.20540954999160 , 708.200000001117600 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(797.20540954999160 , 708.200000001117600 , pWorld),
                                         HGF2DLocation(917.36555435531770 , 794.533854080364100 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(917.36555435531770 , 794.533854080364100 , pWorld),
                                         HGF2DLocation(839.13632601790600 , 745.530022010207200 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(839.13632601790600 , 745.530022010207200 , pWorld),
                                         HGF2DLocation(406.49632601789200 , 1436.33002200909000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(406.49632601789200 , 1436.33002200909000 , pWorld),
                                         HGF2DLocation(478.21462259441610 , 1490.73787791281900 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(478.21462259441610 , 1490.73787791281900 , pWorld),
                                         HGF2DLocation(236.09282746911050 , 1869.89251990057500 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(236.09282746911050 , 1869.89251990057500 , pWorld),
                                         HGF2DLocation(251.38915521104350 , 1878.28020202368500 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(251.38915521104350 , 1878.28020202368500 , pWorld),
                                         HGF2DLocation(941.91618072113490 , 789.439447365701200 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(941.91618072113490 , 789.439447365701200 , pWorld),
                                         HGF2DLocation(990.43891932512630 , 826.438629571348400 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(990.43891932512630 , 826.438629571348400 , pWorld),
                                         HGF2DLocation(986.69358094688500 , 837.717088313773300 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(986.69358094688500 , 837.717088313773300 , pWorld),
                                         HGF2DLocation(932.55795740464240 , 804.527957096695900 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(932.55795740464240 , 804.527957096695900 , pWorld),
                                         HGF2DLocation(251.38915521104350 , 1878.28020202368500 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 0.000000000000000, pWorld),
                                         HGF2DLocation(0.000000000000000 , 2942.000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.000000000000000 , 2942.000000000000, pWorld),
                                         HGF2DLocation(2382.000000000000 , 2942.000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2382.000000000000 , 2942.000000000000, pWorld),
                                         HGF2DLocation(2382.000000000000 , 0.000000000000000, pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(2382.000000000000 , 0.000000000000000, pWorld),
                                         HGF2DLocation(0.000000000000000 , 0.000000000000000, pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_NEAR(3447356.8918827991, pResult->CalculateArea(), 10E-3);
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_NEAR(3447356.8918827991, pResult->CalculateArea(), 10E-3);
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_NEAR(7007844.7411207762, pResult->CalculateArea(), 10E-3);   

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_NEAR(7007844.7411207771, pResult->CalculateArea(), 10E-3);
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_NEAR(0.74112077634504203, pResult->CalculateArea(), 10E-12);
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_NEAR(3560487.1081172004, pResult->CalculateArea(), 10E-3);
    
    }

//==================================================================================
// Test which failed on January 8 1999
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest32)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.97254836834000 , 36528.98034920412000 , pWorld),
                                         HGF2DLocation(-10362.97254332393000 , 36528.98035521588000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.97254332393000 , 36528.98035521588000 , pWorld),
                                         HGF2DLocation(-10362.97255000001000 , 36528.98034999997000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.97255000001000 , 36528.98034999997000 , pWorld),
                                         HGF2DLocation(-65772.30034309332000 , 107449.6585960491000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-65772.30034309332000 , 107449.6585960491000 , pWorld),
                                         HGF2DLocation(-14551.31049872444000 , 147467.6175577276000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-14551.31049872444000 , 147467.6175577276000 , pWorld),
                                         HGF2DLocation(36652.408752940640000 , 81930.06325922819000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(36652.408752940640000 , 81930.06325922819000 , pWorld),
                                         HGF2DLocation(100362.97255166530000 , 28471.01965472511000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(100362.97255166530000 , 28471.01965472511000 , pWorld),
                                         HGF2DLocation(65684.981222158590000 , -12856.2808520035500 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(65684.981222158590000 , -12856.2808520035500 , pWorld),
                                         HGF2DLocation(42034.423706349950000 , -61346.1818546281700 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(42034.423706349950000 , -61346.1818546281700 , pWorld),
                                         HGF2DLocation(18406.218307773830000 , -49821.6501835301300 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(18406.218307773830000 , -49821.6501835301300 , pWorld),
                                         HGF2DLocation(-27418.76350684630000 , -73170.7162502366700 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-27418.76350684630000 , -73170.7162502366700 , pWorld),
                                         HGF2DLocation(-59816.42063631586000 , -9587.65065051140800 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-59816.42063631586000 , -9587.65065051140800 , pWorld),
                                         HGF2DLocation(-120985.2918734361000 , 27166.50231267429000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-120985.2918734361000 , 27166.50231267429000 , pWorld),
                                         HGF2DLocation(-87507.54318190165000 , 82882.24776904780000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-87507.54318190165000 , 82882.24776904780000 , pWorld),
                                         HGF2DLocation(-10362.97255000001000 , 36528.98034999997000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.97255000001000 , 36528.98034999997000 , pWorld),
                                         HGF2DLocation(-10362.97254836834000 , 36528.98034920412000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-94435.20360731255000 , 71353.18749675452000 , pWorld),
                                         HGF2DLocation(-81964.89405861273000 , 121371.9084690987000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-81964.89405861273000 , 121371.9084690987000 , pWorld),
                                         HGF2DLocation(-56181.62813618503000 , 114942.6005061449000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-56181.62813618503000 , 114942.6005061449000 , pWorld),
                                         HGF2DLocation(-65772.30034309332000 , 107449.6585960491000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-65772.30034309332000 , 107449.6585960491000 , pWorld),
                                         HGF2DLocation(-10362.97255000001000 , 36528.98034999997000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-10362.97255000001000 , 36528.98034999997000 , pWorld),
                                         HGF2DLocation(-87507.54318190165000 , 82882.24776904780000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-87507.54318190165000 , 82882.24776904780000 , pWorld),
                                         HGF2DLocation(-94435.20360731255000 , 71353.18749675452000 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    
    pResult = pShape2->UnifyShape(*pShape1);
    
    pResult = pShape1->UnifyShape(*pShape2);
    
    #ifdef WIP_IPPTEST_BUG_4 
    pResult = pShape1->DifferentiateShape(*pShape2); 
    pResult = pShape2->DifferentiateShape(*pShape1);
    #endif

    }

//==================================================================================
// Test which failed on May 18 1999
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest33)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1397168087.98626900000000 , -1597261746.48763900000000 , pWorld),
                                         HGF2DLocation(1396690582.11111700000000 , -1630473344.53076400000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1396690582.11111700000000 , -1630473344.53076400000000 , pWorld),
                                         HGF2DLocation(1427712484.92282800000000 , -1630473344.53076400000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1427712484.92282800000000 , -1630473344.53076400000000 , pWorld),
                                         HGF2DLocation(1427712484.92282800000000 , -1597700911.89259000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(1427712484.92282800000000 , -1597700911.89259000000000 , pWorld),
                                         HGF2DLocation(1397168087.98626900000000 , -1597261746.48763900000000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1397168087.98626700000000 , -1597261746.48764200000000 , pWorld),
                                         HGF2DLocation(1394500885.87970100000000 , -1782771904.54012900000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1394500885.87970100000000 , -1782771904.54012900000000 , pWorld),
                                         HGF2DLocation(1557455317.14201400000000 , -1785114850.54406700000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1557455317.14201400000000 , -1785114850.54406700000000 , pWorld),
                                         HGF2DLocation(1560122518.24858000000000 , -1599604692.49158000000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(1560122518.24858000000000 , -1599604692.49158000000000 , pWorld),
                                         HGF2DLocation(1397168087.98626700000000 , -1597261746.48764200000000 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_DOUBLE_EQ(1015650565817576.0, pResult->CalculateArea());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_DOUBLE_EQ(30235951328494196.0, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_DOUBLE_EQ(30235951328494416.0, pResult->CalculateArea());   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_DOUBLE_EQ(13078664.0000000000, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_DOUBLE_EQ(29220300749597976.0, pResult->CalculateArea());
    
    }

//==================================================================================
// Test which failed on May 19 1999
// 3 april 2000. We do not support autocontiguous polygons
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest34)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(444.10497237569010 , 222.55248618784530 , pWorld),
                                         HGF2DLocation(444.10497237569010 , 444.10497237569060 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(444.10497237569010 , 444.10497237569060 , pWorld),
                                         HGF2DLocation(666.65745856353510 , 444.10497237569060 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(666.65745856353510 , 444.10497237569060 , pWorld),
                                         HGF2DLocation(666.65745856353510 , 222.55248618784530 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(666.65745856353510 , 222.55248618784530 , pWorld),
                                         HGF2DLocation(444.10497237569010 , 222.55248618784530 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(639.99999999999770 , 222.55248618784530 , pWorld),
                                         HGF2DLocation(639.99999999999770 , 256.00000000000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(639.99999999999770 , 256.00000000000000 , pWorld),
                                         HGF2DLocation(640.00000000000000 , 256.00000000000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(640.00000000000000 , 256.00000000000000 , pWorld),
                                         HGF2DLocation(640.00000000000000 , 222.55248618784530 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(640.00000000000000 , 222.55248618784530 , pWorld),
                                         HGF2DLocation(639.99999999999770 , 222.55248618784530 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_TRUE(pResult->IsEmpty());
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);   

    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(49307.056622203163, pResult->CalculateArea());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(49307.056622203163, pResult->CalculateArea());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    ASSERT_DOUBLE_EQ(49307.056622203163, pResult->CalculateArea());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);    

    }

//==================================================================================
// Test which failed on June 9 1999
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest35)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(34844.16927999996000 , 44491.57767857141000 , pWorld),
                                         HGF2DLocation(41887.33631999997000 , 44491.57767857141000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(41887.33631999997000 , 44491.57767857141000 , pWorld),
                                         HGF2DLocation(41887.33631999990000 , -2278.75032142859700 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(41887.33631999990000 , -2278.75032142859700 , pWorld),
                                         HGF2DLocation(34844.16927999996000 , -2278.75032142859700 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(34844.16927999996000 , -2278.75032142859700 , pWorld),
                                         HGF2DLocation(34844.16927999996000 , 44491.57767857141000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(24059.97599999984000 , 26663.21735845723000 , pWorld),
                                         HGF2DLocation(6231.615679885634000 , 44491.57767857141000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(6231.615679885634000 , 44491.57767857141000 , pWorld),
                                         HGF2DLocation(41887.33632011399000 , 44491.57767857134000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(41887.33632011399000 , 44491.57767857134000 , pWorld),
                                         HGF2DLocation(24059.97599999984000 , 26663.21735845723000 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());   

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());   

    }

//==================================================================================
// Test which failed on June 14 1999
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest36)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 0.00 , pWorld), HGF2DLocation(10.01, 0.10 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.01, 0.10 , pWorld), HGF2DLocation(10.02, 10.2 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(10.02, 10.2 , pWorld), HGF2DLocation(-0.10, 10.1 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(-0.10, 10.1 , pWorld), HGF2DLocation(0.000, 0.00 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HFCPtr<HVE2DShape> pShape2 = new HVE2DRectangle(5.0, -1.0, 5.00000000001, 11.0, pWorld);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);    

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_TRUE(pResult->IsEmpty());
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_TRUE(pResult->IsEmpty());
    ASSERT_NEAR(0.0, pResult->CalculateArea(), MYEPSILON);
    
    }

//==================================================================================
// Test which failed on Sept 21, 1999
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest37)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 0.000 , pWorld), HGF2DLocation(0.000, 256.0 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.000, 256.0 , pWorld), HGF2DLocation(256.0, 256.0 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 256.0 , pWorld), HGF2DLocation(256.0, 0.000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(256.0, 0.000 , pWorld), HGF2DLocation(0.000, 0.000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(255.9999998156, 166.9374991798 , pWorld),
                                         HGF2DLocation(343.1683259336, 363.8989382296 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(343.1683259336, 363.8989382296 , pWorld),
                                         HGF2DLocation(511.0000000000, 304.0000000000 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(511.0000000000, 304.0000000000 , pWorld),
                                         HGF2DLocation(424.8316736055, 107.8989382327 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(424.8316736055, 107.8989382327 , pWorld),
                                         HGF2DLocation(255.9999998156, 166.9374991798 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());   

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());    

    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());    

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());    

    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());   

    }

//==================================================================================
// Test which failed on Oct 14, 1999
//==================================================================================
TEST_F(HVE2DPolygonOfSegmentsTester,  ModifyShapeTest38)
    {
        
    HVE2DComplexLinear  TheLinear1(pWorld);

    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000 , 256.00000000000000 , pWorld),
                                         HGF2DLocation(0.00739457713053 , 256.00000000000000 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00739457713053 , 256.00000000000000 , pWorld),
                                         HGF2DLocation(0.00000000000000 , 255.92716162332320 , pWorld)));
    TheLinear1.AppendLinear(HVE2DSegment(HGF2DLocation(0.00000000000000 , 255.92716162332320 , pWorld),
                                         HGF2DLocation(0.00000000000000 , 256.00000000000000 , pWorld)));

    HFCPtr<HVE2DShape> pShape1 = new HVE2DPolygonOfSegments(TheLinear1);

    HVE2DComplexLinear  TheLinear2(pWorld);

    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-255.85432279390440 , 256.07283860515550 , pWorld),
                                         HGF2DLocation(-254.70864558780860 , 511.92716140115580 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-254.70864558780860 , 511.92716140115580 , pWorld),
                                         HGF2DLocation(0.14567720555641000 , 511.78148419505400 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(0.14567720555641000 , 511.78148419505400 , pWorld),
                                         HGF2DLocation(-0.0000000005394400 , 255.92716139905340 , pWorld)));
    TheLinear2.AppendLinear(HVE2DSegment(HGF2DLocation(-0.0000000005394400 , 255.92716139905340 , pWorld),
                                         HGF2DLocation(-255.85432279390440 , 256.07283860515550 , pWorld)));

    HFCPtr<HVE2DShape> pShape2 = new HVE2DPolygonOfSegments(TheLinear2);

    HFCPtr<HVE2DShape> pResult = pShape2->IntersectShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());    

    pResult = pShape1->IntersectShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());
    
    pResult = pShape2->UnifyShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());    

    pResult = pShape1->UnifyShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());    

    pResult = pShape1->DifferentiateShape(*pShape2);
    ASSERT_FALSE(pResult->IsEmpty());   

    pResult = pShape2->DifferentiateShape(*pShape1);
    ASSERT_FALSE(pResult->IsEmpty());    

    } 