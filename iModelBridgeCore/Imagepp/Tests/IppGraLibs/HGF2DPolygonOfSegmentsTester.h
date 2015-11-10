//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DPolygonOfSegmentsTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DPolygonOfSegmentsTester
//-----------------------------------------------------------------------------

#pragma once
 
// Preparation of required environement
class HGF2DPolygonOfSegmentsTester : public EnvironnementTest 
    {   
protected:

    HGF2DPolygonOfSegmentsTester();

    // Polygons
    HGF2DPolygonOfSegments      Poly1A;

    HGF2DPolygonOfSegments      NorthContiguousPolyA;
    HGF2DPolygonOfSegments      EastContiguousPolyA;
    HGF2DPolygonOfSegments      WestContiguousPolyA;
    HGF2DPolygonOfSegments      SouthContiguousPolyA;

    HGF2DPolygonOfSegments      NETipPolyA;
    HGF2DPolygonOfSegments      NWTipPolyA;
    HGF2DPolygonOfSegments      SETipPolyA;
    HGF2DPolygonOfSegments      SWTipPolyA;

    HGF2DPolygonOfSegments      VerticalFitPolyA;
    HGF2DPolygonOfSegments      HorizontalFitPolyA;

    HGF2DPolygonOfSegments      DisjointPolyA;
    HGF2DPolygonOfSegments      NegativePolyA;

    HGF2DPolygonOfSegments      MiscPoly1A;

    HGF2DPolygonOfSegments      EnglobPoly1A;
    HGF2DPolygonOfSegments      EnglobPoly2A;
    HGF2DPolygonOfSegments      EnglobPoly3A;

    HGF2DPolygonOfSegments      IncludedPoly1A;
    HGF2DPolygonOfSegments      IncludedPoly2A;
    HGF2DPolygonOfSegments      IncludedPoly3A;
    HGF2DPolygonOfSegments      IncludedPoly4A;
    HGF2DPolygonOfSegments      IncludedPoly5A;
    HGF2DPolygonOfSegments      IncludedPoly6A;
    HGF2DPolygonOfSegments      IncludedPoly7A;
    HGF2DPolygonOfSegments      IncludedPoly8A;
    HGF2DPolygonOfSegments      IncludedPoly9A;

    HGF2DPosition   PolyClosePoint1AA;
    HGF2DPosition   PolyClosePoint1BA;
    HGF2DPosition   PolyClosePoint1CA;
    HGF2DPosition   PolyClosePoint1DA;
    HGF2DPosition   PolyCloseMidPoint1A;

    HGF2DPosition   Poly1Point0d0A;
    HGF2DPosition   Poly1Point0d1A;
    HGF2DPosition   Poly1Point0d5A;
    HGF2DPosition   Poly1Point1d0A;

    HGF2DPosition   PolyMidPoint1A;

    //Array
    double DblArray[1000];
    size_t MyPolyCount;

    };