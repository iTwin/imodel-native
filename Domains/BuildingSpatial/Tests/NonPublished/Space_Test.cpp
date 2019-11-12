/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <BeSQLite/BeSQLite.h>
#include "BuildingSpatialTestFixtureBase.h"
#include <BuildingSpatial/Elements/Space.h>
#include <BuildingShared/BuildingSharedApi.h>

USING_NAMESPACE_BUILDINGSPATIAL
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING_SHARED

//=======================================================================================
// Sets up environment for BuildingSpatial testing.
// @bsiclass                                    Jonas.Valiunas                  10/2017
//=======================================================================================
struct SpaceTestFixture : public BuildingSpatialTestFixtureBase
    {
    public:
        SpaceTestFixture() {};
        ~SpaceTestFixture() {};
    };

TEST_F(SpaceTestFixture, SpaceIsInserted)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();
    SpacePtr space = BuildingSpatial::Space::Create(*m_model);
    ASSERT_TRUE(space->Insert().IsValid());
    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(SpaceTestFixture, SetFootprintShape)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    SpacePtr space = BuildingSpatial::Space::Create(*m_model);
    ASSERT_TRUE(space->Insert().IsValid());

    ASSERT_EQ(0, space->GetFootprintArea());

    CurveVectorPtr rectCurve = CurveVector::CreateRectangle(0, 0, 10, 10, 0);

    space->SetFootprintShape(rectCurve);

    CurveVectorPtr spaceShape = DgnGeometryUtils::GetBaseShape(*space);

    ASSERT_TRUE(spaceShape.IsValid());
    ASSERT_EQ(GeometryUtils::GetCurveArea(*rectCurve), GeometryUtils::GetCurveArea(*spaceShape));

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }
