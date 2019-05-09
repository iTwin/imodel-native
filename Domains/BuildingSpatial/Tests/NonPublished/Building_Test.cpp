/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <BeSQLite/BeSQLite.h>
#include "BuildingSpatialTestFixtureBase.h"
#include <BuildingSpatial/Elements/Building.h>
#include <BuildingShared/BuildingSharedApi.h>

USING_NAMESPACE_BUILDINGSPATIAL
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING_SHARED

//=======================================================================================
// Sets up environment for BuildingSpatial testing.
// @bsiclass                                    Jonas.Valiunas                  10/2017
//=======================================================================================
struct BuildingTestFixture : public BuildingSpatialTestFixtureBase
    {
    public:
        BuildingTestFixture() {};
        ~BuildingTestFixture() {};
    };

TEST_F(BuildingTestFixture, BuildingIsInserted)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();
    BuildingPtr building = BuildingSpatial::Building::Create(*m_model);
    ASSERT_TRUE(building->Insert().IsValid());
    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Elonas.Seviakovas               04/2019
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(BuildingTestFixture, SetFootprintShape)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    BuildingPtr building = BuildingSpatial::Building::Create(*m_model);
    ASSERT_TRUE(building->Insert().IsValid());

    ASSERT_EQ(0, building->GetFootprintArea());

    CurveVectorPtr rectCurve = CurveVector::CreateRectangle(0, 0, 10, 10, 0);

    building->SetFootprintShape(rectCurve);
    
    CurveVectorPtr buildingShape = DgnGeometryUtils::GetBaseShape(*building);

    ASSERT_TRUE(buildingShape.IsValid());
    ASSERT_EQ(GeometryUtils::GetCurveArea(*rectCurve), GeometryUtils::GetCurveArea(*buildingShape));

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Elonas.Seviakovas               04/2019
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(BuildingTestFixture, GetFootprintArea_HorizontalPlaneAtZero_ReturnsAreaOfThePlane)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    BuildingPtr building = BuildingSpatial::Building::Create(*m_model);
    ASSERT_TRUE(building->Insert().IsValid());

    ASSERT_EQ(0, building->GetFootprintArea());

    CurveVectorPtr rectCurve = CurveVector::CreateRectangle(0, 0, 10, 10, 0);

    building->SetFootprintShape(rectCurve, false);
    building->Update();

    ASSERT_EQ(GeometryUtils::GetCurveArea(*rectCurve), building->GetFootprintArea());
    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Elonas.Seviakovas               04/2019
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(BuildingTestFixture, GetFootprintArea_HorizontalPlaneAtNotZero_ReturnsZero)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    BuildingPtr building = BuildingSpatial::Building::Create(*m_model);
    ASSERT_TRUE(building->Insert().IsValid());

    ASSERT_EQ(0, building->GetFootprintArea());

    CurveVectorPtr rectCurve = CurveVector::CreateRectangle(0, 0, 10, 10, 1);

    building->SetFootprintShape(rectCurve, false);
    building->Update();

    ASSERT_EQ(0, building->GetFootprintArea());
    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Elonas.Seviakovas               04/2019
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(BuildingTestFixture, GetFootprintArea_AngledPlane_ReturnsZero)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    BuildingPtr building = BuildingSpatial::Building::Create(*m_model);
    ASSERT_TRUE(building->Insert().IsValid());

    ASSERT_EQ(0, building->GetFootprintArea());

    STDVectorDPoint3d points{ {10, 0, 0}, {10, 10, 10}, {0, 10, 10}, {0, 0, 0} };
    CurveVectorPtr rectCurve = CurveVector::CreateLinear(points, CurveVector::BOUNDARY_TYPE_Outer);

    building->SetFootprintShape(rectCurve, false);
    building->Update();

    ASSERT_EQ(0, building->GetFootprintArea());
    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Elonas.Seviakovas               04/2019
//--------------+---------------+---------------+---------------+---------------+--------
TEST_F(BuildingTestFixture, GetFootprintArea_BuildingIntersectsLocalPlane_ReturnsAreaOfSlice)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    BuildingPtr building = BuildingSpatial::Building::Create(*m_model);
    ASSERT_TRUE(building->Insert().IsValid());

    ASSERT_EQ(0, building->GetFootprintArea());

    CurveVectorPtr rectCurve = CurveVector::CreateRectangle(0, 0, 10, 10, -5);

    CurveVectorPtr selectedShape = rectCurve->Clone();
    selectedShape->ConsolidateAdjacentPrimitives(true);
    DVec3d zvector = DVec3d::From(0.0f, 0.0f, 10.0f);
    DgnExtrusionDetail extrusionDetail(selectedShape, zvector, true);
    DgnGeometryUtils::SetDgnExtrusionDetail(*building, nullptr, DgnSubCategoryId(), extrusionDetail);

    building->Update();

    ASSERT_EQ(GeometryUtils::GetCurveArea(*rectCurve), building->GetFootprintArea());
    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }
