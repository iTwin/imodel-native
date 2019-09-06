/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
|
+--------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

USING_NAMESPACE_BUILDING_SHARED

struct GeometryUtilsTests : public BuildingSharedTestFixtureBase
    {
    public:
        GeometryUtilsTests() {}
        ~GeometryUtilsTests() {};
    };

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                09/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, AddVertex_LinearCurveVector)
    {
    CurveVectorPtr cv = CurveVector::CreateLinear({ { 0,0,0 },{ 200,0,0 },{ 200,200,0 } });

    GeometryUtils::AddVertex(*cv, { 100,0,0 });

    CurveVectorPtr expectedCV = CurveVector::CreateLinear({ { 0,0,0 }, { 100,0,0 }, { 200,0,0 }, { 200,200,0 } });
    ASSERT_TRUE(cv->IsSameStructureAndGeometry(*expectedCV));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                09/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, AddVertex_LinearCurveVector_VertexAlreadyExists)
    {
    CurveVectorPtr cv = CurveVector::CreateLinear({ { 0,0,0 },{ 200,0,0 },{ 200,200,0 } });

    GeometryUtils::AddVertex(*cv, { 200,0,0 });

    CurveVectorPtr expectedCV = CurveVector::CreateLinear({ { 0,0,0 },{ 200,0,0 },{ 200,200,0 } });
    ASSERT_TRUE(cv->IsSameStructureAndGeometry(*expectedCV));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, AddVertex_OnArc)
    {
    CurveVectorPtr cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    cv->Add(ICurvePrimitive::CreateArc(DEllipse3d::FromVectors({ 0,0,0 }, DVec3d::From(1, 0, 0), DVec3d::From(0, 1, 0), 0, Angle::DegreesToRadians(270))));

    GeometryUtils::AddVertex(*cv, { -1,0,0 });

    CurveVectorPtr expectedCV = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    expectedCV->Add(ICurvePrimitive::CreateArc(DEllipse3d::FromVectors({ 0,0,0 }, DVec3d::From(1, 0, 0), DVec3d::From(0, 1, 0), 0, Angle::DegreesToRadians(180))));
    expectedCV->Add(ICurvePrimitive::CreateArc(DEllipse3d::FromVectors({ 0,0,0 }, DVec3d::From(1, 0, 0), DVec3d::From(0, 1, 0), Angle::DegreesToRadians(180), Angle::DegreesToRadians(90))));
    ASSERT_TRUE(cv->IsSameStructureAndGeometry(*expectedCV));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, AddVertex_OpenType_ClosedShape)
    {
    CurveVectorPtr cv = CurveVector::CreateLinear({ {0,0,0}, {0,200,0}, {200,200,0}, {200,0,0}, {0,0,0} }, CurveVector::BOUNDARY_TYPE_Open);

    GeometryUtils::AddVertex(*cv, { 100,0,0 });

    CurveVectorPtr expectedCV = CurveVector::CreateLinear({ {0,0,0},{0,200,0},{200,200,0},{200,0,0},{100,0,0},{0,0,0} }, CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_TRUE(cv->IsSameStructureAndGeometry(*expectedCV));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, AddVertex_OnInnerShape)
    {
    CurveVectorPtr cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
    cv->Add(CurveVector::CreateLinear({ {0,0,0}, {300,0,0}, {300,300,0}, {0,300,0}, {0,0,0} }, CurveVector::BOUNDARY_TYPE_Outer));
    cv->Add(CurveVector::CreateLinear({ {100,100,0}, {100,200,0}, {200,200,0}, {200,100,0}, {100,100,0} }, CurveVector::BOUNDARY_TYPE_Inner));

    GeometryUtils::AddVertex(*cv, { 150,100,0 });

    CurveVectorPtr expectedCV = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
    expectedCV->Add(CurveVector::CreateLinear({ {0,0,0}, {300,0,0}, {300,300,0}, {0,300,0}, {0,0,0} }, CurveVector::BOUNDARY_TYPE_Outer));
    expectedCV->Add(CurveVector::CreateLinear({ {100,100,0}, {100,200,0}, {200,200,0}, {200,100,0}, {150,100,0}, {100,100,0} }, CurveVector::BOUNDARY_TYPE_Inner));
    ASSERT_TRUE(cv->IsSameStructureAndGeometry(*expectedCV));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, AddVertex_OuterCurveVector)
    {
    CurveVectorPtr cv = CurveVector::CreateLinear({ {0,0,0},{200,0,0},{200,200,0},{0,200,0},{0,0,0} }, CurveVector::BOUNDARY_TYPE_Outer);

    GeometryUtils::AddVertex(*cv, { 100,0,0 });

    CurveVectorPtr expectedCV = CurveVector::CreateLinear({ {0,0,0},{100,0,0},{200,0,0},{200,200,0},{0,200,0},{0,0,0} }, CurveVector::BOUNDARY_TYPE_Outer);
    ASSERT_TRUE(cv->IsSameStructureAndGeometry(*expectedCV));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                10/2017
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(GeometryUtilsTests, AllPointsInOrOnShape)
    {
    bvector<DPoint3d> shapePoints {{0,0,0}, {100,0,0}, {100,100,0}, {0,100,0}, {0,0,0}};
    bvector<DPoint3d> pointsToTest {{0,0,0}, {50,50,0}, {99,0,0}};

    ASSERT_TRUE(GeometryUtils::AllPointsInOrOnShape(shapePoints, pointsToTest));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                10/2017
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(GeometryUtilsTests, AllPointsInOrOnShape_1IsOut)
    {
    bvector<DPoint3d> shapePoints {{0,0,0}, {100,0,0}, {100,100,0}, {0,100,0}, {0,0,0}};
    bvector<DPoint3d> pointsToTest {{0,0,0}, {50,50,0}, {200,0,0}};

    ASSERT_FALSE(GeometryUtils::AllPointsInOrOnShape(shapePoints, pointsToTest));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                09/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GeometryUtilsTests, GetConvexPolygonVertices_KeepArea)
    {
    DPoint3d center {0,0,0};
    double radius = 50;
    int nVertices = 32;
    double expectedArea = PI * radius * radius;

    bvector<DPoint3d> polygonVertices = GeometryUtils::GetConvexPolygonVertices(nVertices, radius, center, true);
    ASSERT_EQ(polygonVertices.size(), nVertices);
    ASSERT_TRUE(PolygonOps::IsConvex(polygonVertices));

    CurveVectorPtr polygon = CurveVector::CreateLinear(polygonVertices, CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    double actualArea;
    DPoint3d centroid;
    DVec3d normal;
    polygon->CentroidNormalArea(centroid, normal, actualArea);
    ASSERT_DOUBLE_EQ(actualArea, expectedArea);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                10/2017
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(GeometryUtilsTests, GetCircularArcPoints)
    {
    double radius = 10;
    DPoint3d center {0,0,0};
    DEllipse3d arc = DEllipse3d::FromCenterRadiusXY(center, radius);

    bvector<DPoint3d> pointsOnArc = GeometryUtils::GetCircularArcPoints(arc, PI / 4, false);

    ASSERT_EQ(pointsOnArc.size(), 81);
    ASSERT_TRUE(pointsOnArc.front().AlmostEqual(pointsOnArc.back()));
    for (DPoint3d const& p : pointsOnArc)
        {
        ASSERT_DOUBLE_EQ(p.Distance(center), radius);
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                10/2017
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(GeometryUtilsTests, GetCircularArcPoints_KeepArea)
    {
    double radius = 10;
    double expectedArea = PI * radius * radius;
    DEllipse3d arc = DEllipse3d::FromCenterRadiusXY({0,0,0}, radius);

    bvector<DPoint3d> points = GeometryUtils::GetCircularArcPoints(arc, PI / 2, true);

    CurveVectorPtr shape = CurveVector::CreateLinear(points, CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    double actualArea;
    DPoint3d centroid;
    DVec3d normal;
    shape->CentroidNormalArea(centroid, normal, actualArea);

    ASSERT_DOUBLE_EQ(actualArea, expectedArea);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                10/2017
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(GeometryUtilsTests, GetCircularArcPoints_QuarterSection_KeepArea)
    {
    double radius = 10;
    double expectedArea = PI * radius * radius / 4;
    DPoint3d center {0,0,0};
    DEllipse3d arc = DEllipse3d::FromArcCenterStartEnd(center, {radius,0,0}, {0,radius,0});

    bvector<DPoint3d> points = GeometryUtils::GetCircularArcPoints(arc, PI / 4, true);

    bvector<DPoint3d> sectionPoints = {center};
    sectionPoints.insert(sectionPoints.end(), points.begin(), points.end());
    CurveVectorPtr shape = CurveVector::CreateLinear(sectionPoints, CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    double actualArea;
    DPoint3d centroid;
    DVec3d normal;
    shape->CentroidNormalArea(centroid, normal, actualArea);

    ASSERT_DOUBLE_EQ(actualArea, expectedArea);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                10/2017
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(GeometryUtilsTests, GetCircularArcPoints_MaxEdgeLengthEqualToArcLength)
    {
    double radius = 10;
    DPoint3d start {radius,0,0};
    DPoint3d end {0,radius,0};
    DEllipse3d arc = DEllipse3d::FromArcCenterStartEnd({0,0,0}, start, end);
    double maxEdgeLength = arc.ArcLength();

    bvector<DPoint3d> points = GeometryUtils::GetCircularArcPoints(arc, maxEdgeLength, false);

    ASSERT_EQ(points.size(), 2);
    ASSERT_TRUE(points[0].AlmostEqual(start));
    ASSERT_TRUE(points[1].AlmostEqual(end));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                10/2017
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(GeometryUtilsTests, GetCircularArcPoints_MaxEdgeLengthEqualTo3QuartersOfArcLength)
    {
    double radius = 10;
    DPoint3d start {radius,0,0};
    DPoint3d mid {cos(PI / 4)*radius, sin(PI / 4)*radius, 0};
    DPoint3d end {0,radius,0};
    DEllipse3d arc = DEllipse3d::FromArcCenterStartEnd({0,0,0}, start, end);
    double maxEdgeLength = arc.ArcLength() * 3 / 4;

    bvector<DPoint3d> points = GeometryUtils::GetCircularArcPoints(arc, maxEdgeLength, false);

    ASSERT_EQ(points.size(), 3);
    ASSERT_TRUE(points[0].AlmostEqual(start));
    ASSERT_TRUE(points[1].AlmostEqual(mid));
    ASSERT_TRUE(points[2].AlmostEqual(end));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                10/2017
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(GeometryUtilsTests, GetCircularArcPoints_NonCircularArcProducesEmptyCollectionOfPoints)
    {
    DEllipse3d arc = DEllipse3d::FromVectors(DPoint3d {0,0,0}, DVec3d::From(10, 0, 0), DVec3d::From(0, 5, 0), 0, PI);

    bvector<DPoint3d> points = GeometryUtils::GetCircularArcPoints(arc);

    ASSERT_EQ(points.size(), 0);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                10/2017
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(GeometryUtilsTests, GetCurvePrimitivePoints_StrokesNotAddedToStraightSegments)
    {
    ICurvePrimitivePtr primitive = ICurvePrimitive::CreateLineString({{0,0,0},{10,0,0},{10,10,0}});

    bvector<DPoint3d> points = GeometryUtils::GetCurvePrimitivePoints(*primitive, 5);

    ASSERT_EQ(points.size(), 3);
    ASSERT_TRUE(points[0].AlmostEqual({0,0,0}));
    ASSERT_TRUE(points[1].AlmostEqual({10,0,0}));
    ASSERT_TRUE(points[2].AlmostEqual({10,10,0}));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                10/2017
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(GeometryUtilsTests, GetCurvePrimitivePoints_StrokesNotAddedToLine)
    {
    ICurvePrimitivePtr primitive = ICurvePrimitive::CreateLine(DSegment3d::From({0,0,0}, {10,0,0}));

    bvector<DPoint3d> points = GeometryUtils::GetCurvePrimitivePoints(*primitive, 5);

    ASSERT_EQ(points.size(), 2);
    ASSERT_TRUE(points[0].AlmostEqual({0,0,0}));
    ASSERT_TRUE(points[1].AlmostEqual({10,0,0}));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                11/2017
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(GeometryUtilsTests, IsSameSingleLoopGeometry)
    {
    ICurvePrimitivePtr curve1 = ICurvePrimitive::CreateLine(DSegment3d::From({-5,0,0}, {5,0,0}));
    ICurvePrimitivePtr curve2 = ICurvePrimitive::CreateArc(DEllipse3d::FromVectors(DPoint3d {0,0,0}, DVec3d::From(5, 0, 0), DVec3d::From(0, 5, 0), 0, PI));

    CurveVectorPtr cv1 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    CurveVectorPtr cv2 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);

    cv1->Add(curve1);
    cv1->Add(curve2);

    cv2->Add(curve1);
    cv2->Add(curve2);

    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*cv1, *cv2));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                11/2017
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(GeometryUtilsTests, IsSameSingleLoopGeometry_DifferentOrder)
    {
    ICurvePrimitivePtr curve1 = ICurvePrimitive::CreateLine(DSegment3d::From({-5,0,0}, {5,0,0}));
    ICurvePrimitivePtr curve2 = ICurvePrimitive::CreateArc(DEllipse3d::FromVectors(DPoint3d {0,0,0}, DVec3d::From(5, 0, 0), DVec3d::From(0, 5, 0), 0, PI));

    CurveVectorPtr cv1 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    CurveVectorPtr cv2 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);

    cv1->Add(curve1);
    cv1->Add(curve2);

    cv2->Add(curve2);
    cv2->Add(curve1);

    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*cv1, *cv2));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                11/2017
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(GeometryUtilsTests, IsSameSingleLoopGeometry_DifferentGeometry)
    {
    ICurvePrimitivePtr curve1 = ICurvePrimitive::CreateLine(DSegment3d::From({-5,0,0}, {5,0,0}));
    ICurvePrimitivePtr curve2 = ICurvePrimitive::CreateArc(DEllipse3d::FromVectors(DPoint3d {0,0,0}, DVec3d::From(5, 0, 0), DVec3d::From(0, 5, 0), 0, PI));
    ICurvePrimitivePtr curve3 = ICurvePrimitive::CreateLineString({{-5,0,0},{0,-5,0},{5,0,0}});

    CurveVectorPtr cv1 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    CurveVectorPtr cv2 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);

    cv1->Add(curve1);
    cv1->Add(curve2);

    cv2->Add(curve2);
    cv2->Add(curve3);

    ASSERT_FALSE(GeometryUtils::IsSameSingleLoopGeometry(*cv1, *cv2));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, IsSameSingleLoopGeometry_FullArcsWithDifferentStarts)
    {
    DEllipse3d arc1 = DEllipse3d::FromVectors({0,0,0}, DVec3d::From(1, 0, 0), DVec3d::From(0, 1, 0), 0, Angle::TwoPi());
    DEllipse3d arc2 = DEllipse3d::FromVectors({0,0,0}, DVec3d::From(1, 0, 0), DVec3d::From(0, 1, 0), Angle::Pi(), Angle::TwoPi());

    CurveVectorPtr cv1 = CurveVector::Create(ICurvePrimitive::CreateArc(arc1), CurveVector::BOUNDARY_TYPE_Outer);
    CurveVectorPtr cv2 = CurveVector::Create(ICurvePrimitive::CreateArc(arc2), CurveVector::BOUNDARY_TYPE_Outer);

    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*cv1, *cv2));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, IsSameSingleLoopGeometry_DifferentEllipses)
    {
    DEllipse3d ellipse1 = DEllipse3d::FromVectors({0,0,0}, DVec3d::From(1, 0, 0), DVec3d::From(0, 2, 0), 0, Angle::TwoPi());
    DEllipse3d ellipse2 = DEllipse3d::FromVectors({0,0,0}, DVec3d::From(2, 0, 0), DVec3d::From(0, 1, 0), 0, Angle::TwoPi());

    CurveVectorPtr cv1 = CurveVector::Create(ICurvePrimitive::CreateArc(ellipse1), CurveVector::BOUNDARY_TYPE_Outer);
    CurveVectorPtr cv2 = CurveVector::Create(ICurvePrimitive::CreateArc(ellipse2), CurveVector::BOUNDARY_TYPE_Outer);

    ASSERT_FALSE(GeometryUtils::IsSameSingleLoopGeometry(*cv1, *cv2));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, IsSameSingleLoopGeometry_EqualEllipsesWithDifferentStarts)
    {
    DEllipse3d ellipse1 = DEllipse3d::FromVectors({0,0,0}, DVec3d::From(1, 0, 0), DVec3d::From(0, 2, 0), 0, Angle::TwoPi());
    DEllipse3d ellipse2 = DEllipse3d::FromVectors({0,0,0}, DVec3d::From(1, 0, 0), DVec3d::From(0, 2, 0), Angle::Pi(), Angle::TwoPi());

    CurveVectorPtr cv1 = CurveVector::Create(ICurvePrimitive::CreateArc(ellipse1), CurveVector::BOUNDARY_TYPE_Outer);
    CurveVectorPtr cv2 = CurveVector::Create(ICurvePrimitive::CreateArc(ellipse2), CurveVector::BOUNDARY_TYPE_Outer);

    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*cv1, *cv2));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, ProjectCurveOntoZeroPlane)
    {
    STDVectorDPoint3d points{{10, 0, 0}, {10, 10, 10}, {0, 10, 10}, {0, 0, 0}};
    CurveVectorPtr rectCurve = CurveVector::CreateLinear(points, CurveVector::BOUNDARY_TYPE_Outer);

    double area1 = GeometryUtils::GetCurveArea(*rectCurve);
    CurveVectorPtr curve = GeometryUtils::ProjectCurveOntoZeroPlane(*rectCurve);

    double area2 = GeometryUtils::GetCurveArea(*curve);

    ASSERT_EQ(100, area2);
    ASSERT_NE(area1, area2);
    }

//-------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                06/2019
//--------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, ProjectCurveOntoPlane)
    {
    bvector<DPoint3d> initCurvePoints{{0, 0, 0}, {10, 0, 10}, {10, 10, 10}, {0, 10, 0}};
    CurveVectorPtr initCurve = CurveVector::CreateLinear(initCurvePoints, CurveVector::BOUNDARY_TYPE_Outer);

    bvector<DPoint3d> expectedCurvePoints{{5, 0, 0}, {5, 0, 10}, {5, 10, 10}, {5, 10, 0}};
    CurveVectorPtr expectedCurve = CurveVector::CreateLinear(expectedCurvePoints, CurveVector::BOUNDARY_TYPE_Outer);

    DPlane3d plane = DPlane3d::FromOriginAndNormal(DPoint3d::From(5, 0, 0), DVec3d::From(1, 0, 0));
    CurveVectorPtr actualCurve = GeometryUtils::ProjectCurveOntoPlane(*initCurve, plane);

    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*actualCurve, *expectedCurve));
    }

//-------------------------------------------------------------------------------------
// @betest                                      Mindaugas.Butkus                06/2019
//--------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, ProjectPointsOntoPlane)
    {
    bvector<DPoint3d> initPoints{{0, 0, 0}, {10, 0, 10}, {10, 10, 10}, {0, 10, 0}};
    bvector<DPoint3d> expectedPoints{{4, 0, 0}, {4, 0, 10}, {4, 10, 10}, {4, 10, 0}};
    DPlane3d plane = DPlane3d::FromOriginAndNormal(DPoint3d::From(4, 0, 0), DVec3d::From(1, 0, 0));
    bvector<DPoint3d> actualPoints = GeometryUtils::ProjectPointsOntoPlane(initPoints, plane);

    ASSERT_TRUE(DPoint3d::AlmostEqual(actualPoints, expectedPoints, DoubleOps::SmallCoordinateRelTol()));
    }