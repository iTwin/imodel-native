//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HVE2DPolygonOfSegmentsTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DPolygonOfSegmentsTester
//-----------------------------------------------------------------------------

#pragma once
 
// Preparation of required environement
class HVE2DPolygonOfSegmentsTester : public EnvironnementTest 
    {   
protected:

    HVE2DPolygonOfSegmentsTester();

    // Polygons
    HVE2DPolygonOfSegments      Poly1;

    HVE2DPolygonOfSegments      NorthContiguousPoly;
    HVE2DPolygonOfSegments      EastContiguousPoly;
    HVE2DPolygonOfSegments      WestContiguousPoly;
    HVE2DPolygonOfSegments      SouthContiguousPoly;

    HVE2DPolygonOfSegments      NETipPoly;
    HVE2DPolygonOfSegments      NWTipPoly;
    HVE2DPolygonOfSegments      SETipPoly;
    HVE2DPolygonOfSegments      SWTipPoly;

    HVE2DPolygonOfSegments      VerticalFitPoly;
    HVE2DPolygonOfSegments      HorizontalFitPoly;

    HVE2DPolygonOfSegments      DisjointPoly;
    HVE2DPolygonOfSegments      NegativePoly;

    HVE2DPolygonOfSegments      MiscPoly1;

    HVE2DPolygonOfSegments      EnglobPoly1;
    HVE2DPolygonOfSegments      EnglobPoly2;
    HVE2DPolygonOfSegments      EnglobPoly3;

    HVE2DPolygonOfSegments      IncludedPoly1;
    HVE2DPolygonOfSegments      IncludedPoly2;
    HVE2DPolygonOfSegments      IncludedPoly3;
    HVE2DPolygonOfSegments      IncludedPoly4;
    HVE2DPolygonOfSegments      IncludedPoly5;
    HVE2DPolygonOfSegments      IncludedPoly6;
    HVE2DPolygonOfSegments      IncludedPoly7;
    HVE2DPolygonOfSegments      IncludedPoly8;
    HVE2DPolygonOfSegments      IncludedPoly9;

    HGF2DLocation   PolyClosePoint1A;
    HGF2DLocation   PolyClosePoint1B;
    HGF2DLocation   PolyClosePoint1C;
    HGF2DLocation   PolyClosePoint1D;
    HGF2DLocation   PolyCloseMidPoint1;

    HGF2DLocation   Poly1Point0d0;
    HGF2DLocation   Poly1Point0d1;
    HGF2DLocation   Poly1Point0d5;
    HGF2DLocation   Poly1Point1d0;

    HGF2DLocation   PolyMidPoint1;

    //Array
    double DblArray[1000];
    size_t MyPolyCount;

    };