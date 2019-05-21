/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "QuantityTakeoffsAspectsTestFixtureBase.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <BeSQLite/BeSQLite.h>
#include <QuantityTakeoffsAspects/Elements/PileAspect.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
// Sets up environment for BuildingSpatial testing.
// @bsiclass                                    Jonas.Valiunas                  10/2017
//=======================================================================================
struct PileAspectTestFixture : public QuantityTakeoffsAspectsTestFixtureBase
    {
    public:
        PileAspectTestFixture() {};
        ~PileAspectTestFixture() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PileAspectTestFixture, Create)
    {
    DgnDbR db = GetDgnDb();

    PileAspectPtr aspect = PileAspect::Create();

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE