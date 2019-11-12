/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
//
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>


USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL




static double s_noGapTol = 1.0e-10;
static double s_bigGapDirect = 100.0;
//static double s_smallGapDirect = 0.01;
static double s_gapAlong = 100.0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(GapCleanup, Test1)
    {

    CurveVectorPtr sticks = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (1,0,0, 99,0,0)));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (100,1,0, 100,99,0)));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (99,100,0, 1,100,0)));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (0,99,0, 0,1,0)));
    CurveGapOptions options (s_noGapTol, s_bigGapDirect, s_gapAlong);
    CurveVectorPtr fixed = sticks->CloneWithGapsClosed (options);

    if (!Check::Near (0.0, fixed->MaxGapWithinPath (), "LineLine gap cleanup"))
        {
        Check::Print (*sticks, "sticks");
        Check::Print (*fixed, "fixed");
        }
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(GapCleanup, Test2)
    {
    CurveVectorPtr sticks = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (1,0,0, 99,0,0)));
    sticks->push_back (
        ICurvePrimitive::CreateArc (DEllipse3d::FromPointsOnArc (
                        DPoint3d::From (100,1,0),
                        DPoint3d::From (110,50,0),
                        DPoint3d::From (100,99,0)
                        )));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (99,100,0, 1,100,0)));
    sticks->push_back (
        ICurvePrimitive::CreateArc (DEllipse3d::FromPointsOnArc (
                        DPoint3d::From (0,99,0),
                        DPoint3d::From (5,50,0),
                        DPoint3d::From (0,1,0)
                        )));
    CurveGapOptions options (s_noGapTol, s_bigGapDirect, s_gapAlong);
    CurveVectorPtr fixed = sticks->CloneWithGapsClosed (options);
    if (!Check::Near (0.0, fixed->MaxGapWithinPath (), "LineArc gap cleanup"))
        {
        Check::Print (*sticks, "sticks");
        Check::Print (*fixed, "fixed");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(GapCleanup, Test3)
    {
    CurveVectorPtr sticks = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From (100,0,0));
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (0,100,0));
    points.push_back (DPoint3d::From (100,100,0));
    
    sticks->push_back (ICurvePrimitive::CreateLineString (points));
    double d0 = 93.0;
    sticks->push_back (
        ICurvePrimitive::CreateArc (
            DEllipse3d::FromVectors (
                        DPoint3d::From (100,50,0),
                        DVec3d::From (49,0,0),
                        DVec3d::From (0,49,0),
                        Angle::DegreesToRadians (d0),
                        Angle::DegreesToRadians (-2.0 * d0)
                        )));

    CurveGapOptions options (s_noGapTol, s_bigGapDirect, s_gapAlong);
    CurveVectorPtr fixed = sticks->CloneWithGapsClosed (options);
    if (!Check::Near (0.0, fixed->MaxGapWithinPath (), "LineArc gap cleanup"))
        {
        Check::Print (*sticks, "sticks");
        Check::Print (*fixed, "fixed");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(GapCleanup, Test4)
    {
    CurveVectorPtr sticks = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    DPoint3d pointA = DPoint3d::From (0,0,0);
    DPoint3d pointB = DPoint3d::From (99,0,0);
    DPoint3d pointC = DPoint3d::From (100,1,0);
    DPoint3d pointD = DPoint3d::From (100,100,0);
    
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (pointA, pointB)));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (pointB, pointC)));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (pointC, pointD)));
        
    sticks->at(1)->SetMarkerBit (ICurvePrimitive::CURVE_PRIMITIVE_BIT_GapCurve, true);
    CurveGapOptions options (s_noGapTol, s_bigGapDirect, s_gapAlong);
    CurveVectorPtr fixed = sticks->CloneWithGapsClosed (options);

    if (!Check::Near (0.0, fixed->MaxGapWithinPath (), "LineLine gap cleanup"))
        {
        Check::Print (*sticks, "sticks");
        Check::Print (*fixed, "fixed");
        }

    Check::Size (3, sticks->size (), "Size with gap");
    Check::Size (2, fixed->size (), "Size after extend");
    Check::True (fixed->Length () > sticks->Length () + 0.5, "Length with extension instead of gap");
    }


size_t CountBoundaryType (CurveVectorCR source, CurveVector::BoundaryType targetType)
    {
    size_t count = 0;
    for (size_t i = 0; i < source.size (); i++)
        {
        CurveVector::BoundaryType btype;
        if (source.GetChildBoundaryType (i, btype) && btype == targetType)
            count++;
        }
    return count;
    }

#if defined (_WIN32) && !defined(BENTLEY_WINRT)

static void Announce (Json::Value & value, IGeometryPtr &g)
    {
    Json::StyledWriter writer;
    Utf8String s = writer.write (value);
    GEOMAPI_PRINTF ("%s\n", s.c_str());
    Json::FastWriter fastWriter;
    Utf8String s1 = fastWriter.write (value);
    if (s1.size () < 2000)
      GEOMAPI_PRINTF ("\n%s\n", s1.c_str());
#ifdef verifyStringToJson
    Json::Value value1;
    bool b = Json::Reader::Parse (s1, value1, false);
    Utf8String s2 = fastWriter.write (value1);
    GEOMAPI_PRINTF ("s2 %s\n", s.c_str());
    Check::True (b);
#endif
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Chaining, Test0)
    {
    static bool s_doPrint = false;
    CurveVectorPtr sticks = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    DPoint3d pointA = DPoint3d::From (0,0,0);
    DPoint3d pointB = DPoint3d::From (100,0,0);
    DPoint3d pointC = DPoint3d::From (100,100,0);
    
    DPoint3d pointD = DPoint3d::From (200,0,0);
    DPoint3d pointE = DPoint3d::From (300,0,0);
    
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (pointA, pointB)));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (pointB, pointC)));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (pointD, pointE)));
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (pointC, pointA)));

    CurveVectorPtr chains = sticks->AssembleChains ();
    Check::Size (2, chains->size (), "Triangle + Line = 2 chains");
    Check::Size (1, CountBoundaryType (*chains, CurveVector::BOUNDARY_TYPE_Outer), "Triangle+Line has one outer");
    Check::Size (1, CountBoundaryType (*chains, CurveVector::BOUNDARY_TYPE_Open), "Triangle+Line has one open");
    if (s_doPrint)
        Check::Print (chains, "Triangle + islandLine");
    sticks->push_back (
        ICurvePrimitive::CreateLine (DSegment3d::From (pointB, pointD)));
    chains = sticks->AssembleChains ();
    if (s_doPrint)
        Check::Print (chains, "Triangle + penninsulaLine");

    Check::Size (2, chains->size (), "Triangle + Line = 2 chains");
    Check::Size (2, CountBoundaryType (*chains, CurveVector::BOUNDARY_TYPE_Open), "Triangle+LineConnected has two chain");
    Check::Size (0, CountBoundaryType (*chains, CurveVector::BOUNDARY_TYPE_Outer), "Triangle+Line has one outer");



    }

#if defined (_WIN32) && !defined(BENTLEY_WINRT)

void ReadCG (Utf8CP xmlString, int noisy = 0, bool expectFailure = false)
    {
    bvector<IGeometryPtr> geometry, geometryA;
    if (noisy > 10)
        GEOMAPI_PRINTF ("CG to geometry: %s\n", xmlString);
    bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
    BeXmlCGStreamReader::TryParse (xmlString, geometry, extendedData, 2);
    //BeXmlCGReader::TryParse (xmlString, geometryA, 2);
    if (noisy > 10)
        {
        GEOMAPI_PRINTF ("(geometry.size () %d\n", (int)geometry.size ());
        if (geometry.size () == 0)
            {
            if (!expectFailure)
                GEOMAPI_PRINTF ("\n\n  ************** RoundTrip failure?? ************** \n\n");
            else
                GEOMAPI_PRINTF ("nonCG data rejected as expected\n");
            }
        }
    if (noisy > 10)
        {
        Utf8String beXmlA;
        for (IGeometryPtr g : geometry)
             {
             BeXmlCGWriter::Write(beXmlA, *g);
             GEOMAPI_PRINTF ("(\nBeXmlCGWriter XML\n%s\n)\n", beXmlA.c_str());
            }

        for (IGeometryPtr g : geometry)
            {
            Json::Value jsonValue;
            if (BentleyGeometryJson::TryGeometryToJsonValue (jsonValue, *g))
                {
                Announce (jsonValue, g);
                bvector<IGeometryPtr> g1;
                bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
                if (!BentleyGeometryJson::TryJsonValueToGeometry (jsonValue, g1))
                    {
                    GEOMAPI_PRINTF (" ????????? TryJsonValueToGeometry failed\n");
                    }
                else
                    {
                    if (Check::True (g1.size () > 0 && g1.front ().IsValid ()))
                        {
                        bool b = g1.size () == 1
                              && g->IsSameStructureAndGeometry(*g1.front ());
                        if (!b)
                            {
                            GEOMAPI_PRINTF (" **********  returned from json reader (%d)\n", (int)g1.size ());
                            for (size_t i = 0; i < g1.size (); i++)
                                {
                                Json::Value jsonValue1;
                                if (BentleyGeometryJson::TryGeometryToJsonValue (jsonValue1, *g1[i]))
                                    Announce (jsonValue1, g);
                                }
                            }
                        Check::True (b, "json round trip");
                        }
                    else
                        {
                        GEOMAPI_PRINTF (" ????????? no valid geometry on json readback\n");
                        }
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,LineSegment)
    {
    ReadCG("<LineSegment><startPoint>0,0,0</startPoint><endPoint>1,1,1</endPoint></LineSegment>", 10);
    }

static Utf8CP s_circularArc = "\
<CircularArc>\
  <placement>\
    <origin>1,2,3</origin>\
    <vectorZ>0,0,1</vectorZ>\
    <vectorX>1,0,0</vectorX>\
  </placement>\
  <radius>10</radius>\
  <startAngle>20</startAngle>\
  <sweepAngle>30</sweepAngle>\
</CircularArc>";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,CircularArc)
    {
    ReadCG(s_circularArc, 10);
    }

static Utf8CP s_ellipticArc = "\
<EllipticArc>\
  <placement>\
    <origin>1,2,3</origin>\
    <vectorZ>0,0,1</vectorZ>\
    <vectorX>1,0,0</vectorX>\
  </placement>\
  <radiusA>10</radiusA>\
  <radiusB>15</radiusB>\
  <startAngle>20</startAngle>\
  <sweepAngle>30</sweepAngle>\
</EllipticArc>";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,ellipticArc)
    {
    ReadCG(s_ellipticArc, 10);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,NonCG)
    {
    ReadCG("<Hello></Hello>", 10, true);
    ReadCG("<Hello a=\"1\"></Hello>", 10, true);
    ReadCG("<Hello a=\"1\"><how><then></then></how></Hello>", 10, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,Sphere)
    {
    ReadCG(
"<Sphere>\
  <placement>\
    <origin>0,0,0</origin>\
    <vectorZ>0,0,1</vectorZ>\
    <vectorX>1,0,0</vectorX>\
  </placement>\
  <radius>10</radius>\
</Sphere>\
", 10);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,TorusPipe)
    {
    ReadCG(
"<TorusPipe>\
  <placement>\
    <origin>0,0,0</origin>\
    <vectorZ>0,0,1</vectorZ>\
    <vectorX>1,0,0</vectorX>\
  </placement>\
  <radiusA>2</radiusA>\
  <radiusB>1</radiusB>\
  <startAngle>0</startAngle>\
  <sweepAngle>90</sweepAngle>\
  <bSolidFlag>false</bSolidFlag>\
</TorusPipe>\
", 10);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,SkewedCone)
    {
    ReadCG(
"<SkewedCone>\
  <placement>\
    <origin>0,0,0</origin>\
    <vectorZ>0,0,1</vectorZ>\
    <vectorX>1,0,0</vectorX>\
  </placement>\
  <centerB>0,0,0</centerB>\
  <radiusA>0</radiusA>\
  <radiusB>0</radiusB>\
  <bSolidFlag>false</bSolidFlag>\
</SkewedCone>\
", 10);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,Block)
    {
    ReadCG(
"<Block>\
  <placement>\
    <origin>0,0,0</origin>\
    <vectorZ>0,0,1</vectorZ>\
    <vectorX>1,0,0</vectorX>\
  </placement>\
  <cornerA>2,1,-1</cornerA>\
  <cornerB>5,3,4</cornerB>\
  <bSolidFlag>true</bSolidFlag>\
</Block>\
", 10);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,CircularCone)
    {
    ReadCG(
"<CircularCone>\
  <placement>\
    <origin>0,0,0</origin>\
    <vectorZ>0,0,1</vectorZ>\
    <vectorX>1,0,0</vectorX>\
  </placement>\
  <height>2</height>\
  <radiusA>3</radiusA>\
  <radiusB>4</radiusB>\
  <bSolidFlag>false</bSolidFlag>\
</CircularCone>\
", 10);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,CircularDisk)
    {
    ReadCG(
"<CircularDisk xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\">\
  <placement>\
    <origin>0,0,0</origin>\
    <vectorZ>0,0,1</vectorZ>\
    <vectorX>1,0,0</vectorX>\
  </placement>\
  <radius>0</radius>\
</CircularDisk>\
", 10);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,EllipticDisk)
    {
    ReadCG(
"<EllipticDisk xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\">\
  <placement>\
    <origin>0,0,0</origin>\
    <vectorZ>0,0,1</vectorZ>\
    <vectorX>1,0,0</vectorX>\
  </placement>\
  <radiusA>0</radiusA>\
  <radiusB>0</radiusB>\
</EllipticDisk>\
", 10);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,LineString)
    {
    ReadCG(
"<LineString>\
  <ListOfPoint>\
    <xyz>0,0,0</xyz>\
    <xyz>0,0,1</xyz>\
    <xyz>1,0,0</xyz>\
  </ListOfPoint>\
</LineString>\
", 10);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,IndexedMesh)
    {
    ReadCG(
"<IndexedMesh>\
  <ListOfCoord>\
    <Coord>1,2,3</Coord>\
    <Coord>1,2,4</Coord>\
    <Coord>5,2,1</Coord>\
  </ListOfCoord>\
  <ListOfCoordIndex>\
    <id>1</id>\
    <id>2</id>\
    <id>3</id>\
    <id>0</id>\
  </ListOfCoordIndex>\
  <ListOfParam></ListOfParam>\
  <ListOfParamIndex></ListOfParamIndex>\
  <ListOfNormal></ListOfNormal>\
  <ListOfNormalIndex></ListOfNormalIndex>\
</IndexedMesh>\
", 10);
    }
static int s_readerDebug = 10;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,IndexedMeshWithVariantNullList)
    {
    ReadCG(
"<IndexedMesh>\n\
  <ListOfCoord>\
    <Coord>1,2,3</Coord>\
    <Coord>1,2,4</Coord>\
    <Coord>5,2,1</Coord>\
  </ListOfCoord>\n\
  <ListOfCoordIndex>\
    <id>1</id>\
    <id>2</id>\
    <id>3</id>\
    <id>0</id>\
  </ListOfCoordIndex>\n\
  <ListOfParam></ListOfParam>\n\
  <ListOfParamIndex></ListOfParamIndex>\n\
  <ListOfNormal/>\n\
  <ListOfNormalIndex/>\n\
</IndexedMesh>\
", s_readerDebug);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,LineSegmentMulti)
    {
    ReadCG("\
<MyWrapper>\
<LineSegment><startPoint>0,0,0</startPoint><endPoint>1,1,1</endPoint></LineSegment>\
<LineSegment><startPoint>1,0,0</startPoint><endPoint>1,1,1</endPoint></LineSegment>\
<LineSegment><startPoint>2,0,0</startPoint><endPoint>1,1,1</endPoint></LineSegment>\
</MyWrapper>\
", 10);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,CurveChain)
    {
    ReadCG("\
<CurveChain>\
<ListOfCurve>\
<LineSegment><startPoint>0,0,0</startPoint><endPoint>1,1,1</endPoint></LineSegment>\
<LineSegment><startPoint>1,0,0</startPoint><endPoint>1,1,1</endPoint></LineSegment>\
<LineSegment><startPoint>2,0,0</startPoint><endPoint>1,1,1</endPoint></LineSegment>\
</ListOfCurve>\
</CurveChain>\
", 10);
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,BsplineCurve)
    {
    ReadCG("\
<BsplineCurve>\
      <Order>4</Order>\
      <Closed>false</Closed>\
      <ListOfControlPoint>\
        <ControlPoint>10241.640620058,10283.3998390257,40.9999850090838</ControlPoint>\
        <ControlPoint>10242.640620058,10283.3998390257,41.99999506277</ControlPoint>\
        <ControlPoint>10243.640620058,10283.3998390257,42.0000051164561</ControlPoint>\
        <ControlPoint>10244.640620058,10283.3998390257,43.0000151701423</ControlPoint>\
      </ListOfControlPoint>\
    </BsplineCurve>\
", 10);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,BsplineSurface)
    {
    ReadCG("\
    <BsplineSurface>\
      <OrderU>3</OrderU>\
      <numUControlPoint>3</numUControlPoint>\
      <ClosedU>false</ClosedU>\
      <OrderV>2</OrderV>\
      <numVControlPoint>2</numVControlPoint>\
      <ClosedV>false</ClosedV>\
      <ListOfControlPoint>\
        <ControlPoint>10046.5226852909,10368.3040382963,38.6411181401554</ControlPoint>\
        <ControlPoint>10046.4861510507,10368.1339816238,38.662359806822</ControlPoint>\
        <ControlPoint>10046.635753121,10368.1801101284,38.9341764734887</ControlPoint>\
        <ControlPoint>10047.0115462541,10368.1197467764,40.104534806822</ControlPoint>\
        <ControlPoint>10047.0476370631,10368.2452301926,40.2901931401554</ControlPoint>\
        <ControlPoint>10047.0198922956,10368.4029496106,40.2890764734887</ControlPoint>\
      </ListOfControlPoint>\
    </BsplineSurface>\
", 10);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,Polygon)
{
ReadCG("\
<Polygon xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\">\
  <ListOfPoint>\
        <Point>10046.5226852909,10368.3040382963,38.6411181401554</Point>\
        <Point>10046.4861510507,10368.1339816238,38.662359806822</Point>\
        <Point>10046.635753121,10368.1801101284,38.9341764734887</Point>\
        <Point>10047.0115462541,10368.1197467764,40.104534806822</Point>\
        <Point>10047.0476370631,10368.2452301926,40.2901931401554</Point>\
        <Point>10047.0198922956,10368.4029496106,40.2890764734887</Point>\
  </ListOfPoint>\
</Polygon>\
", 10);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,DgnExtrusionA)
{
ReadCG("\
<DgnExtrusion>\
<BaseGeometry>\
<Polygon>\
  <ListOfPoint>\
        <Point>1,2,1</Point>\
        <Point>2,2,1</Point>\
        <Point>2,3,1</Point>\
        <Point>1,3,1</Point>\
  </ListOfPoint>\
</Polygon>\
</BaseGeometry>\
<ExtrusionVector>0,0,1</ExtrusionVector>\
<capped>true</capped>\
</DgnExtrusion>\
", 10);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,DgnRotationalSweep)
{
ReadCG("\
<DgnRotationalSweep>\
<BaseGeometry>\
<Polygon>\
  <ListOfPoint>\
        <Point>1,2,1</Point>\
        <Point>2,2,1</Point>\
        <Point>2,3,1</Point>\
        <Point>1,3,1</Point>\
  </ListOfPoint>\
</Polygon>\
</BaseGeometry>\
<center>0,0,0</center>\
<axis>1,0,0</axis>\
<sweepAngle>90</sweepAngle>\
<capped>true</capped>\
</DgnRotationalSweep>\
", 10);
}



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,SurfacePatch)
{
ReadCG("\
<SurfacePatch>\
  <ExteriorLoop>\
    <CurveChain>\
      <ListOfCurve>\
        <CircularArc>\
          <placement>\
            <origin>32.3455,112.74600000000001,-5.7592</origin>\
            <vectorZ>0,1,0</vectorZ>\
            <vectorX>1,-0,0</vectorX>\
          </placement>\
          <radius>0.2</radius>\
          <startAngle>-90</startAngle>\
          <sweepAngle>180</sweepAngle>\
        </CircularArc>\
        <LineSegment>\
          <startPoint>32.3455,112.74600000000001,-5.5592</startPoint>\
          <endPoint>32.3455,112.74600000000001,-5.9592</endPoint>\
        </LineSegment>\
      </ListOfCurve>\
    </CurveChain>\
  </ExteriorLoop>\
  <ListOfHoleLoop />\
</SurfacePatch>\
", 10);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,SurfacePatch1)
{
ReadCG("\
<SurfacePatch>\
<exteriorLoop>\
  <CurveChain>\
    <ListOfCurve>\
      <LineString>\
        <ListOfPoint>\
          <Point>0,0,0</Point>\
          <Point>6,0,0</Point>\
          <Point>6,4,0</Point>\
          <Point>0,4,0</Point>\
          <Point>0,0,0</Point>\
          </ListOfPoint>\
      </LineString>\
    </ListOfCurve>\
  </CurveChain>\
</exteriorLoop>\
<ListOfHoleLoop>\
  <CurveChain>\
    <ListOfCurve>\
      <LineString>\
        <ListOfPoint>\
          <Point>1,2,0</Point>\
          <Point>2,2,0</Point>\
          <Point>2,3,0</Point>\
          <Point>1,3,0</Point>\
          <Point>1,2,0</Point>\
        </ListOfPoint>\
      </LineString>\
    </ListOfCurve>\
  </CurveChain>\
  <CurveChain>\
    <ListOfCurve>\
      <LineString>\
        <ListOfPoint>\
          <Point>3,2,0</Point>\
          <Point>4,2,0</Point>\
          <Point>4,3,0</Point>\
        </ListOfPoint>\
      </LineString>\
      <LineString>\
        <ListOfPoint>\
          <Point>4,3,0</Point>\
          <Point>3,3,0</Point>\
          <Point>3,2,0</Point>\
        </ListOfPoint>\
      </LineString>\
    </ListOfCurve>\
  </CurveChain>\
</ListOfHoleLoop>\
</SurfacePatch>\
", 10);
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,DgnBox)
{
ReadCG("\
<DgnBox>\
  <baseOrigin>1,2,1</baseOrigin>\
  <topOrigin>1,2,5</topOrigin>\
  <vectorX>1,0,0</vectorX>\
  <vectorY>0,1,0</vectorY>\
  <baseX>3</baseX>\
  <baseY>4</baseY>\
  <topX>3</topX>\
  <topY>4</topY>\
  <capped>true</capped>\
</DgnBox>\
", 10);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,SolidBySweptSurface)
{
ReadCG("\
<SolidBySweptSurface>\
  <BaseGeometry>\
    <Polygon>\
      <ListOfPoint>\
        <xyz>1,2,1</xyz>\
        <xyz>2,2,1</xyz>\
        <xyz>2,3,1</xyz>\
        <xyz>1,3,1</xyz>\
        <xyz>1,2,1</xyz>\
      </ListOfPoint>\
    </Polygon>\
  </BaseGeometry>\
  <RailCurve>\
    <LineSegment>\
      <startPoint>1,2,1</startPoint>\
      <endPoint>1,2,2</endPoint>\
    </LineSegment>\
  </RailCurve>\
</SolidBySweptSurface>\
", 10);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,DgnSphere)
{
ReadCG("\
<DgnSphere>\
<center>1,2,3</center>\
<vectorX>0,1,0</vectorX>\
<vectorZ>1,0,0</vectorZ>\
<radiusXY>4</radiusXY>\
<radiusZ>4</radiusZ>\
<startLatitude>0</startLatitude>\
<latitudeSweep>40</latitudeSweep>\
<capped>true</capped>\
</DgnSphere>\
", 10);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,DgnRuledSweep)
{
#ifdef UseMidLevelLabels
ReadCG("\
<DgnRuledSweep>\
 <ListOfContour>\
    <Contour><LineSegment><startPoint>0,0,0</startPoint><endPoint>1,0,0)</endPoint></LineSegment></Contour>\
    <Contour><LineSegment><startPoint>0,0,1</startPoint><endPoint>1,0,1)</endPoint></LineSegment></Contour>\
    <Contour><LineSegment><startPoint>0,0,2</startPoint><endPoint>1,0,2)</endPoint></LineSegment></Contour>\
 </ListOfContour>\
<capped>true</capped>\
 </DgnRuledSweep>\
 ", 10);
#else
ReadCG("\
<DgnRuledSweep>\
 <ListOfContour>\
    <LineSegment><startPoint>0,0,0</startPoint><endPoint>1,0,0)</endPoint></LineSegment>\
    <LineSegment><startPoint>0,0,1</startPoint><endPoint>1,0,1)</endPoint></LineSegment>\
    <LineSegment><startPoint>0,0,2</startPoint><endPoint>1,0,2)</endPoint></LineSegment>\
 </ListOfContour>\
<capped>true</capped>\
 </DgnRuledSweep>\
 ", 10);
#endif
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,SurfaceByRuledSweep)
{
#ifdef UseMidLevelLabels
ReadCG("\
<SurfaceByRuledSweep>\
 <ListOfSection>\
    <Section><LineSegment><startPoint>0,0,0</startPoint><endPoint>1,0,0)</endPoint></LineSegment></Section>\
    <Section><LineSegment><startPoint>0,0,1</startPoint><endPoint>1,0,1)</endPoint></LineSegment></Section>\
    <Section><LineSegment><startPoint>0,0,2</startPoint><endPoint>1,0,2)</endPoint></LineSegment></Section>\
 </ListOfSection>\
 </SurfaceByRuledSweep>\
 ", 10);
#else
ReadCG("\
<SurfaceByRuledSweep>\
 <ListOfSection>\
    <LineSegment><startPoint>0,0,0</startPoint><endPoint>1,0,0)</endPoint></LineSegment>\
    <LineSegment><startPoint>0,0,1</startPoint><endPoint>1,0,1)</endPoint></LineSegment>\
    <LineSegment><startPoint>0,0,2</startPoint><endPoint>1,0,2)</endPoint></LineSegment>\
 </ListOfSection>\
 </SurfaceByRuledSweep>\
 ", 10);
#endif
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,SolidByRuledSweep)
{
#ifdef UseMidLevelLabels
ReadCG("\
<SolidByRuledSweep>\
 <ListOfSection>\
    <Section><CircularDisk>\
      <placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radius>0</radius>\
    </CircularDisk></Section>\
    <Section><CircularDisk>\
      <placement><origin>0,0,3</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radius>0</radius>\
    </CircularDisk></Section>\
 </ListOfSection>\
 </SolidByRuledSweep>\
 ", 10);
#else
ReadCG("\
<SolidByRuledSweep>\
 <ListOfSection>\
    <CircularDisk>\
      <placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radius>0</radius>\
    </CircularDisk>\
    <CircularDisk>\
      <placement><origin>0,0,3</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radius>0</radius>\
    </CircularDisk>\
 </ListOfSection>\
 </SolidByRuledSweep>\
 ", 10);

#endif
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeXmlStreamReader,Coordinate)
{
ReadCG("\
<Coordinate>\
<xyz>0,0,0</xyz>\
 </Coordinate>\
 ", 10);
}

#endif

