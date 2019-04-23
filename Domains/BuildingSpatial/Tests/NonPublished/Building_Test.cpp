/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/Building_Test.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <Bentley\BeTest.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <BeSQLite/BeSQLite.h>
#include "BuildingSpatialTestFixtureBase.h"
#include <BuildingSpatial/Elements/Building.h>
#include <BuildingSpatial/Handlers/BuildingHandler.h>
#include <BuildingSpatial/Domain/BuildingSpatialMacros.h>
#include <DgnPlatform\DgnElement.h>
#include <DgnPlatform/DgnCoreAPI.h>

USING_NAMESPACE_BUILDINGSPATIAL
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_BENTLEY_SQLITE


#define TOLERANCE 0.1
#define RETURN_ERROR_IF_EQ(a, b) if (a == b) return BentleyStatus::ERROR
#define RETURN_ERROR_IF_FALSE(condition) if (!(condition)) return BentleyStatus::ERROR

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

TEST_F(BuildingTestFixture, SetFootprintShape)
{
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    BuildingPtr building = BuildingSpatial::Building::Create(*m_model);
    ASSERT_TRUE(building->Insert().IsValid());

    double area = building->GetFootprintArea();
    ASSERT_EQ(0, area);

    CurveVectorPtr rectCurve = CurveVector::CreateRectangle(0, 0, 10, 10, 0);

    DPoint3d curveCentroid;
    double curveArea;
    rectCurve->CentroidAreaXY(curveCentroid, curveArea);

    building->SetFootprintShape(rectCurve);

    area = building->GetFootprintArea();
    ASSERT_EQ(curveArea, area);

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
}
