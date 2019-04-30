/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/DgnGeometryUtils_Test.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BuildingShared/BuildingSharedApi.h>
#include "BuildingSharedTestFixtureBase.h"
#include <DgnPlatform/GenericDomain.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING
USING_NAMESPACE_BUILDING_SHARED

struct DgnGeometryUtilsTests : public BuildingSharedTestFixtureBase
    {
    public:
        DgnGeometryUtilsTests() {}
        ~DgnGeometryUtilsTests() {};
    };

//--------------------------------------------------------------------------------------
// @betest                                       Mindaugas Butkus                08/2017
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnGeometryUtilsTests, GetXYCrossSection_LowestCrossSectionParallelToXYPlane)
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
TEST_F(DgnGeometryUtilsTests, GetXYCrossSection_TopmostCrossSectionParallelToXYPlane)
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
TEST_F(DgnGeometryUtilsTests, GetXYCrossSection_TryGetCrossSectionBelowSolid)
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
TEST_F(DgnGeometryUtilsTests, GetXYCrossSection_TryGetCrossSectionAboveSolid)
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
TEST_F(DgnGeometryUtilsTests, GetXYCrossSection_ReturnsPointStringWhenSinglePointOnPlane)
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
TEST_F(DgnGeometryUtilsTests, GetXYCrossSection_GetLowestPointOfSolidWithRoundNotXYOrientedBase)
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
TEST_F(DgnGeometryUtilsTests, GetXYCrossSection_BoxUnionTouchesAndIntersects)
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
// @betest                                       Mindaugas Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnGeometryUtilsTests, GetBodySlice)
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
TEST_F(DgnGeometryUtilsTests, GetUpwardSlice)
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
TEST_F(DgnGeometryUtilsTests, GetDownwardSlice)
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

SpatialCategoryCPtr createAndInsertCategory(DgnDbR db, Utf8StringCR name)
    {
    Dgn::DgnSubCategory::Appearance appearance;
    appearance.SetColor(Dgn::ColorDef::White());
    Dgn::SpatialCategory category(db.GetDictionaryModel(), name, Dgn::DgnCategory::Rank::Domain);
    return category.Insert(appearance);
    }

PhysicalModelPtr createAndInsertModel(DgnDbR db, Utf8StringCR name)
    {
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*rootSubject, name);
    db.Elements().Insert(*partition);
    return PhysicalModel::CreateAndInsert(*partition);
    }

GenericPhysicalObjectPtr createAndInsertObject(DgnDbR db)
    {
    PhysicalModelPtr model = createAndInsertModel(db, "TestModel");
    if(model.IsNull())
        return nullptr;

    SpatialCategoryCPtr category = createAndInsertCategory(db, "TestObject");
    if (category.IsNull())
        return nullptr;

    GenericPhysicalObjectPtr object = GenericPhysicalObject::Create(*model, category->GetCategoryId());
    if(object->Insert().IsNull())
        return nullptr;

    return object;
    }

//--------------------------------------------------------------------------------------
// @betest                                       Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnGeometryUtilsTests, ExtractBottomFaceShape_EmptyGeometry_ReturnsNullptr)
    {
    DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = createAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    GeometryBuilderPtr builder = GeometryBuilder::Create(*object);
    builder->Finish(*object);

    CurveVectorPtr shape = DgnGeometryUtils::ExtractBottomFaceShape(*object);
    ASSERT_TRUE(shape.IsNull());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }



//--------------------------------------------------------------------------------------
// @betest                                       Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnGeometryUtilsTests, ExtractBottomFaceShape_UnclosedCurveAsGeometry_ReturnsNullptr)
    {
    DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = createAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    STDVectorDPoint3d points{ {0, 10}, {10, 10}, {10, 0}, {0, 0} };

    CurveVectorPtr curve = CurveVector::CreateLinear(points, CurveVector::BOUNDARY_TYPE_Open);

    GeometryBuilderPtr builder = GeometryBuilder::Create(*object);
    builder->Append(*curve);
    builder->Finish(*object);

    CurveVectorPtr shape = DgnGeometryUtils::ExtractBottomFaceShape(*object);
    ASSERT_TRUE(shape.IsNull());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnGeometryUtilsTests, ExtractBottomFaceShape_SingeCurveAsGeometry_ReturnsCurve)
    {
    DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = createAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    CurveVectorPtr rectCurve = CurveVector::CreateRectangle(0, 0, 10, 10, 0);

    GeometryBuilderPtr builder = GeometryBuilder::Create(*object);
    builder->Append(*rectCurve);
    builder->Finish(*object);

    CurveVectorPtr shape = DgnGeometryUtils::ExtractBottomFaceShape(*object);
    ASSERT_TRUE(shape.IsValid());

    ASSERT_TRUE(shape->CountPrimitivesBelow() == 1);
    ASSERT_EQ(GeometryUtils::GetCurveArea(*rectCurve), GeometryUtils::GetCurveArea(*shape));

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnGeometryUtilsTests, GetBaseShape_ExtrusionGeometry_ReturnsExtrusionBottomShape)
    {
    DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    CurveVectorPtr rectCurve = CurveVector::CreateRectangle(0, 0, 10, 10, 0);

    GenericPhysicalObjectPtr object = createAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    DgnGeometryUtils::InitiateExtrusionGeometry(*object, nullptr, DgnSubCategoryId(), rectCurve, 10);

    CurveVectorPtr shape = DgnGeometryUtils::GetBaseShape(*object);
    ASSERT_TRUE(shape.IsValid());

    DPoint3d curveCenter;
    double curveArea;
    rectCurve->CentroidAreaXY(curveCenter, curveArea);

    DPoint3d shapeCenter;
    double shapeArea;
    shape->CentroidAreaXY(shapeCenter, shapeArea);

    ASSERT_EQ(curveCenter, shapeCenter);
    ASSERT_EQ(curveArea, shapeArea);

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnGeometryUtilsTests, ExtractBottomFaceShape_MultipleCurvesAsGeometry_ReturnsNestedCurve)
    {
    DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = createAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    CurveVectorPtr rectCurve = CurveVector::CreateRectangle(0, 0, 10, 10, 0);
    CurveVectorPtr rectCurve2 = CurveVector::CreateRectangle(20, 20, 30, 30, 0);

    GeometryBuilderPtr builder = GeometryBuilder::Create(*object);
    builder->Append(*rectCurve);
    builder->Append(*rectCurve2);
    builder->Finish(*object);

    CurveVectorPtr shape = DgnGeometryUtils::ExtractBottomFaceShape(*object);
    ASSERT_TRUE(shape.IsValid());

    ASSERT_TRUE(shape->CountPrimitivesBelow() == 2);
    ASSERT_EQ(GeometryUtils::GetCurveArea(*rectCurve) + GeometryUtils::GetCurveArea(*rectCurve2), GeometryUtils::GetCurveArea(*shape));

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnGeometryUtilsTests, ExtractBottomFaceShape_CurveAndBRepEntityAsGeometry_ReturnsNestedCurve)
    {
    DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = createAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    CurveVectorPtr rectCurve = CurveVector::CreateRectangle(0, 0, 10, 10, 0);

    DgnBoxDetail boxDetail(DPoint3d::From(20, 20, 0), DPoint3d::From(20, 20, 10), DVec3d::UnitX(), DVec3d::UnitY(),
        10, 10, 10, 10, true);

    ISolidPrimitivePtr boxSolidPrimitive = ISolidPrimitive::CreateDgnBox(boxDetail);

    IBRepEntityPtr boxSolid;
    Dgn::BRepUtil::Create::BodyFromSolidPrimitive(boxSolid, *boxSolidPrimitive);

    GeometryBuilderPtr builder = GeometryBuilder::Create(*object);
    builder->Append(*rectCurve);
    builder->Append(*boxSolid);
    builder->Finish(*object);

    CurveVectorPtr shape = DgnGeometryUtils::ExtractBottomFaceShape(*object);
    ASSERT_TRUE(shape.IsValid());

    ASSERT_TRUE(shape->CountPrimitivesBelow() == 2);
    ASSERT_EQ(200, GeometryUtils::GetCurveArea(*shape));

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

//--------------------------------------------------------------------------------------
// @betest                                       Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnGeometryUtilsTests, GetBaseShape_RectangularPlaneGeometry_ReturnsUnchangedCurve)
    {
    DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    CurveVectorPtr rectCurve = CurveVector::CreateRectangle(0, 0, 10, 10, 0);

    GenericPhysicalObjectPtr object = createAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    GeometryBuilderPtr builder = GeometryBuilder::Create(*object);
    builder->Append(*rectCurve);
    builder->Finish(*object);

    CurveVectorPtr shape = DgnGeometryUtils::GetBaseShape(*object);
    ASSERT_TRUE(shape.IsValid());

    DPoint3d curveCenter;
    double curveArea;
    rectCurve->CentroidAreaXY(curveCenter, curveArea);

    DPoint3d shapeCenter;
    double shapeArea;
    shape->CentroidAreaXY(shapeCenter, shapeArea);

    ASSERT_EQ(curveCenter, shapeCenter);
    ASSERT_EQ(curveArea, shapeArea);

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }