/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/GeometryUtils_Test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING
USING_NAMESPACE_BUILDING_SHARED

struct GeometryUtilsTests : public BuildingSharedTestFixtureBase
    {
    public:
        GeometryUtilsTests() {}
        ~GeometryUtilsTests() {};

    };


//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, GetXYCrossSection_LowestCrossSectionParallelToXYPlane)
    {
    DgnConeDetail body = DgnConeDetail({ 0, 0, 0 }, { 0, 0, 20 }, 20, 10, true);
    DEllipse3d expectedEllipse;
    body.FractionToSection(0, expectedEllipse);
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(body);
    Dgn::IBRepEntityPtr solid = nullptr;
    DgnGeometryUtils::CreateBodyFromGeometricPrimitive(solid, geometry, true);
    ASSERT_TRUE(solid.IsValid()) << "Failed to create solid";

    CurveVectorPtr crossSection = DgnGeometryUtils::GetXYCrossSection(*solid, solid->GetEntityRange().low.z);
    ASSERT_TRUE(crossSection.IsValid());
    ASSERT_EQ(crossSection->size(), 1) << "Cross-section should contain 1 CurvePrimitive";
    ASSERT_EQ(crossSection->at(0)->GetCurvePrimitiveType(), ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Arc);
    ASSERT_TRUE(crossSection->at(0)->GetArcCP()->IsAlmostEqual(expectedEllipse, DoubleOps::SmallCoordinateRelTol()));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, GetXYCrossSection_TopmostCrossSectionParallelToXYPlane)
    {
    DgnConeDetail body = DgnConeDetail({ 0, 0, 0 }, { 0, 0, 20 }, 20, 10, true);
    DEllipse3d expectedEllipse;
    body.FractionToSection(1, expectedEllipse);
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(body);
    Dgn::IBRepEntityPtr solid = nullptr;
    DgnGeometryUtils::CreateBodyFromGeometricPrimitive(solid, geometry, true);
    ASSERT_TRUE(solid.IsValid()) << "Failed to create solid";

    CurveVectorPtr crossSection = DgnGeometryUtils::GetXYCrossSection(*solid, solid->GetEntityRange().high.z);
    ASSERT_TRUE(crossSection.IsValid());
    ASSERT_EQ(crossSection->size(), 1) << "Cross-section should contain 1 CurvePrimitive";
    ASSERT_EQ(crossSection->at(0)->GetCurvePrimitiveType(), ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Arc);
    ASSERT_TRUE(crossSection->at(0)->GetArcCP()->IsAlmostEqual(expectedEllipse, DoubleOps::SmallCoordinateRelTol()));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, GetXYCrossSection_TryGetCrossSectionBelowSolid)
    {
    DgnConeDetail body = DgnConeDetail({ 0, 0, 0 }, { 0, 0, 20 }, 20, 10, true);
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(body);
    Dgn::IBRepEntityPtr solid = nullptr;
    DgnGeometryUtils::CreateBodyFromGeometricPrimitive(solid, geometry, true);
    ASSERT_TRUE(solid.IsValid()) << "Failed to create solid";

    CurveVectorPtr crossSection = DgnGeometryUtils::GetXYCrossSection(*solid, solid->GetEntityRange().low.z - 1);
    ASSERT_FALSE(crossSection.IsValid());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, GetXYCrossSection_TryGetCrossSectionAboveSolid)
    {
    DgnConeDetail body = DgnConeDetail({ 0, 0, 0 }, { 0, 0, 20 }, 20, 10, true);
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(body);
    Dgn::IBRepEntityPtr solid = nullptr;
    DgnGeometryUtils::CreateBodyFromGeometricPrimitive(solid, geometry, true);
    ASSERT_TRUE(solid.IsValid()) << "Failed to create solid";

    CurveVectorPtr crossSection = DgnGeometryUtils::GetXYCrossSection(*solid, solid->GetEntityRange().high.z + 1);
    ASSERT_FALSE(crossSection.IsValid());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, GetXYCrossSection_ReturnsPointStringWhenSinglePointOnPlane)
    {
    DgnConeDetail body = DgnConeDetail({ 0, 0, 0 }, { 0, 0, 20 }, 20, 0, true);
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(body);
    Dgn::IBRepEntityPtr solid = nullptr;
    DgnGeometryUtils::CreateBodyFromGeometricPrimitive(solid, geometry, true);
    ASSERT_TRUE(solid.IsValid()) << "Failed to create solid";

    CurveVectorPtr crossSection = DgnGeometryUtils::GetXYCrossSection(*solid, solid->GetEntityRange().high.z);
    ASSERT_TRUE(crossSection.IsValid());
    ASSERT_EQ(crossSection->size(), 1);
    ASSERT_EQ(crossSection->at(0)->GetCurvePrimitiveType(), ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_PointString);
    ASSERT_EQ(crossSection->at(0)->GetPointStringCP()->size(), 1);
    ASSERT_TRUE(DPoint3d::From(0, 0, 20).AlmostEqual(crossSection->at(0)->GetPointStringCP()->at(0)));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, GetXYCrossSection_GetLowestPointOfSolidWithRoundNotXYOrientedBase)
    {
    DgnConeDetail body = DgnConeDetail({ 0, 0, 0 }, { 20, 0, 20 }, 20 * sqrt(2), 0, true);
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(body);
    Dgn::IBRepEntityPtr solid = nullptr;
    DgnGeometryUtils::CreateBodyFromGeometricPrimitive(solid, geometry, true);
    ASSERT_TRUE(solid.IsValid()) << "Failed to create solid";

    CurveVectorPtr crossSection = DgnGeometryUtils::GetXYCrossSection(*solid, solid->GetEntityRange().low.z);
    ASSERT_TRUE(crossSection.IsValid());
    ASSERT_EQ(crossSection->size(), 1);
    ASSERT_EQ(crossSection->at(0)->GetCurvePrimitiveType(), ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_PointString);
    ASSERT_EQ(crossSection->at(0)->GetPointStringCP()->size(), 1);
    ASSERT_TRUE(DPoint3d::From(20, 0, -20).AlmostEqual(crossSection->at(0)->GetPointStringCP()->at(0)));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, GetXYCrossSection_BoxUnionTouchesAndIntersects)
    {
    DgnBoxDetail box1Detail = DgnBoxDetail(DPoint3d::From( 0, 0, 0 ), DPoint3d::From( 0, 0, 100 ), DVec3d::UnitX(), DVec3d::UnitY(),
                                           200, 200, 200, 200, true);
    DgnBoxDetail box2Detail = DgnBoxDetail(DPoint3d::From( 150, 150, -20 ), DPoint3d::From( 150, 150, 10 ), DVec3d::UnitX(), DVec3d::UnitY(),
                                           50, 50, 50, 50, true);

    ISolidPrimitivePtr box1SolidPrimitive = ISolidPrimitive::CreateDgnBox(box1Detail);
    ISolidPrimitivePtr box2SolidPrimitive = ISolidPrimitive::CreateDgnBox(box2Detail);

    GeometricPrimitivePtr box1GeomPrimitive = GeometricPrimitive::Create(box1SolidPrimitive);
    GeometricPrimitivePtr box2GeomPrimitive = GeometricPrimitive::Create(box2SolidPrimitive);

    IBRepEntityPtr box1Solid;
    IBRepEntityPtr box2Solid;
    DgnGeometryUtils::CreateBodyFromGeometricPrimitive(box1Solid, box1GeomPrimitive, true);
    DgnGeometryUtils::CreateBodyFromGeometricPrimitive(box2Solid, box2GeomPrimitive, true);

    BRepUtil::Modify::BooleanOperation(box1Solid, box2Solid, BRepUtil::Modify::BooleanMode::Unite);
    ASSERT_TRUE(box1Solid.IsValid()) << "Failed to create solid";
    CurveVectorPtr crossSection = DgnGeometryUtils::GetXYCrossSection(*box1Solid, 0);
    ASSERT_TRUE(crossSection.IsValid());
    CurveVectorPtr expectedCV = CurveVector::CreateLinear({ {0,0,0}, {200,0,0}, {200, 200, 0}, {0, 200, 0} }, CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    ASSERT_TRUE(GeometryUtils::IsSameSingleLoopGeometry(*crossSection, *expectedCV));
    }

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
    cv->Add(ICurvePrimitive::CreateArc(DEllipse3d::FromVectors({0,0,0}, DVec3d::From(1, 0, 0), DVec3d::From(0, 1, 0), 0, Angle::DegreesToRadians(270))));

    GeometryUtils::AddVertex(*cv, {-1,0,0});

    CurveVectorPtr expectedCV = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    expectedCV->Add(ICurvePrimitive::CreateArc(DEllipse3d::FromVectors({0,0,0}, DVec3d::From(1, 0, 0), DVec3d::From(0, 1, 0), 0, Angle::DegreesToRadians(180))));
    expectedCV->Add(ICurvePrimitive::CreateArc(DEllipse3d::FromVectors({0,0,0}, DVec3d::From(1, 0, 0), DVec3d::From(0, 1, 0), Angle::DegreesToRadians(180), Angle::DegreesToRadians(90))));
    ASSERT_TRUE(cv->IsSameStructureAndGeometry(*expectedCV));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, AddVertex_OpenType_ClosedShape)
    {
    CurveVectorPtr cv = CurveVector::CreateLinear({{0,0,0}, {0,200,0}, {200,200,0}, {200,0,0}, {0,0,0}}, CurveVector::BOUNDARY_TYPE_Open);

    GeometryUtils::AddVertex(*cv, {100,0,0});

    CurveVectorPtr expectedCV = CurveVector::CreateLinear({{0,0,0},{0,200,0},{200,200,0},{200,0,0},{100,0,0},{0,0,0}}, CurveVector::BOUNDARY_TYPE_Open);
    ASSERT_TRUE(cv->IsSameStructureAndGeometry(*expectedCV));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, AddVertex_OnInnerShape)
    {
    CurveVectorPtr cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
    cv->Add(CurveVector::CreateLinear({{0,0,0}, {300,0,0}, {300,300,0}, {0,300,0}, {0,0,0}}, CurveVector::BOUNDARY_TYPE_Outer));
    cv->Add(CurveVector::CreateLinear({{100,100,0}, {100,200,0}, {200,200,0}, {200,100,0}, {100,100,0}}, CurveVector::BOUNDARY_TYPE_Inner));

    GeometryUtils::AddVertex(*cv, {150,100,0});

    CurveVectorPtr expectedCV = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
    expectedCV->Add(CurveVector::CreateLinear({{0,0,0}, {300,0,0}, {300,300,0}, {0,300,0}, {0,0,0}}, CurveVector::BOUNDARY_TYPE_Outer));
    expectedCV->Add(CurveVector::CreateLinear({{100,100,0}, {100,200,0}, {200,200,0}, {200,100,0}, {150,100,0}, {100,100,0}}, CurveVector::BOUNDARY_TYPE_Inner));
    ASSERT_TRUE(cv->IsSameStructureAndGeometry(*expectedCV));
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, AddVertex_OuterCurveVector)
    {
    CurveVectorPtr cv = CurveVector::CreateLinear({{0,0,0},{200,0,0},{200,200,0},{0,200,0},{0,0,0}}, CurveVector::BOUNDARY_TYPE_Outer);

    GeometryUtils::AddVertex(*cv, {100,0,0});

    CurveVectorPtr expectedCV = CurveVector::CreateLinear({{0,0,0},{100,0,0},{200,0,0},{200,200,0},{0,200,0},{0,0,0}}, CurveVector::BOUNDARY_TYPE_Outer);
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
// @betest                                       Mindaugas Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, GetBodySlice)
    {
    DgnBoxDetail boxDetail = DgnBoxDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, 100), DVec3d::UnitX(), DVec3d::UnitY(),
                                           200, 200, 200, 200, true);
    ISolidPrimitivePtr boxSolidPrimitive = ISolidPrimitive::CreateDgnBox(boxDetail);
    ASSERT_TRUE(boxSolidPrimitive.IsValid());
    GeometricPrimitivePtr boxGeomPrimitive = GeometricPrimitive::Create(boxSolidPrimitive);
    ASSERT_TRUE(boxGeomPrimitive.IsValid());
    IBRepEntityPtr boxSolid;
    DgnGeometryUtils::CreateBodyFromGeometricPrimitive(boxSolid, boxGeomPrimitive, true);
    ASSERT_TRUE(boxSolid.IsValid());

    if (true)
        {
        IBRepEntityPtr slice = DgnGeometryUtils::GetBodySlice(*boxSolid, 10, 20);
        ASSERT_TRUE(slice.IsValid());
        DRange3d sliceRange = slice->GetEntityRange();
        ASSERT_DOUBLE_EQ(sliceRange.low.z, 10);
        ASSERT_DOUBLE_EQ(sliceRange.high.z, 20);
        }

    if (true)
        {
        IBRepEntityPtr slice = DgnGeometryUtils::GetBodySlice(*boxSolid, -5, 10);
        ASSERT_TRUE(slice.IsValid());
        DRange3d sliceRange = slice->GetEntityRange();
        ASSERT_DOUBLE_EQ(sliceRange.low.z, 0);
        ASSERT_DOUBLE_EQ(sliceRange.high.z, 10);
        }

    if (true)
        {
        IBRepEntityPtr slice = DgnGeometryUtils::GetBodySlice(*boxSolid, 90, 110);
        ASSERT_TRUE(slice.IsValid());
        DRange3d sliceRange = slice->GetEntityRange();
        ASSERT_DOUBLE_EQ(sliceRange.low.z, 90);
        ASSERT_DOUBLE_EQ(sliceRange.high.z, 100);
        }

    if (true)
        {
        IBRepEntityPtr slice = DgnGeometryUtils::GetBodySlice(*boxSolid, -5, 105);
        ASSERT_TRUE(slice.IsValid());
        DRange3d sliceRange = slice->GetEntityRange();
        ASSERT_DOUBLE_EQ(sliceRange.low.z, 0);
        ASSERT_DOUBLE_EQ(sliceRange.high.z, 100);
        }

    if (true)
        {
        IBRepEntityPtr slice = DgnGeometryUtils::GetBodySlice(*boxSolid, 50, 50);
        ASSERT_TRUE(slice.IsValid());
        DRange3d sliceRange = slice->GetEntityRange();
        ASSERT_DOUBLE_EQ(sliceRange.low.z, 50);
        ASSERT_DOUBLE_EQ(sliceRange.high.z, 50);
        }
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, GetUpwardSlice)
    {
    DgnBoxDetail boxDetail = DgnBoxDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, 100), DVec3d::UnitX(), DVec3d::UnitY(),
                                          200, 200, 200, 200, true);
    ISolidPrimitivePtr boxSolidPrimitive = ISolidPrimitive::CreateDgnBox(boxDetail);
    ASSERT_TRUE(boxSolidPrimitive.IsValid());
    GeometricPrimitivePtr boxGeomPrimitive = GeometricPrimitive::Create(boxSolidPrimitive);
    ASSERT_TRUE(boxGeomPrimitive.IsValid());
    IBRepEntityPtr boxSolid;
    DgnGeometryUtils::CreateBodyFromGeometricPrimitive(boxSolid, boxGeomPrimitive, true);
    ASSERT_TRUE(boxSolid.IsValid());

    if (true)
        {
        IBRepEntityPtr slice = DgnGeometryUtils::GetUpwardSlice(*boxSolid, 10);
        ASSERT_TRUE(slice.IsValid());
        DRange3d sliceRange = slice->GetEntityRange();
        ASSERT_DOUBLE_EQ(sliceRange.low.z, 10);
        ASSERT_DOUBLE_EQ(sliceRange.high.z, 100);
        }

    if (true)
        {
        IBRepEntityPtr slice = DgnGeometryUtils::GetUpwardSlice(*boxSolid, -5);
        ASSERT_TRUE(slice.IsValid());
        DRange3d sliceRange = slice->GetEntityRange();
        ASSERT_DOUBLE_EQ(sliceRange.low.z, 0);
        ASSERT_DOUBLE_EQ(sliceRange.high.z, 100);
        }

    if (true)
        {
        IBRepEntityPtr slice = DgnGeometryUtils::GetUpwardSlice(*boxSolid, 105);
        ASSERT_TRUE(slice.IsNull());
        }

    if (true)
        {
        IBRepEntityPtr slice = DgnGeometryUtils::GetUpwardSlice(*boxSolid, 100);
        ASSERT_TRUE(slice.IsValid());
        DRange3d sliceRange = slice->GetEntityRange();
        ASSERT_DOUBLE_EQ(sliceRange.low.z, 100);
        ASSERT_DOUBLE_EQ(sliceRange.high.z, 100);
        }
    }

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeometryUtilsTests, GetDownwardSlice)
    {
    DgnBoxDetail boxDetail = DgnBoxDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, 100), DVec3d::UnitX(), DVec3d::UnitY(),
                                          200, 200, 200, 200, true);
    ISolidPrimitivePtr boxSolidPrimitive = ISolidPrimitive::CreateDgnBox(boxDetail);
    ASSERT_TRUE(boxSolidPrimitive.IsValid());
    GeometricPrimitivePtr boxGeomPrimitive = GeometricPrimitive::Create(boxSolidPrimitive);
    ASSERT_TRUE(boxGeomPrimitive.IsValid());
    IBRepEntityPtr boxSolid;
    DgnGeometryUtils::CreateBodyFromGeometricPrimitive(boxSolid, boxGeomPrimitive, true);
    ASSERT_TRUE(boxSolid.IsValid());

    if (true)
        {
        IBRepEntityPtr slice = DgnGeometryUtils::GetDownwardSlice(*boxSolid, 10);
        ASSERT_TRUE(slice.IsValid());
        DRange3d sliceRange = slice->GetEntityRange();
        ASSERT_DOUBLE_EQ(sliceRange.low.z, 0);
        ASSERT_DOUBLE_EQ(sliceRange.high.z, 10);
        }

    if (true)
        {
        IBRepEntityPtr slice = DgnGeometryUtils::GetDownwardSlice(*boxSolid, 105);
        ASSERT_TRUE(slice.IsValid());
        DRange3d sliceRange = slice->GetEntityRange();
        ASSERT_DOUBLE_EQ(sliceRange.low.z, 0);
        ASSERT_DOUBLE_EQ(sliceRange.high.z, 100);
        }

    if (true)
        {
        IBRepEntityPtr slice = DgnGeometryUtils::GetDownwardSlice(*boxSolid, -5);
        ASSERT_TRUE(slice.IsNull());
        }

    if (true)
        {
        IBRepEntityPtr slice = DgnGeometryUtils::GetDownwardSlice(*boxSolid, 0);
        ASSERT_TRUE(slice.IsValid());
        DRange3d sliceRange = slice->GetEntityRange();
        ASSERT_DOUBLE_EQ(sliceRange.low.z, 0);
        ASSERT_DOUBLE_EQ(sliceRange.high.z, 0);
        }
    }