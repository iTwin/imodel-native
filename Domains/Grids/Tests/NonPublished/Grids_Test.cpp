/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/Grids_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BeXml\BeXml.h>
#include <Bentley\BeTest.h>
#include <DgnPlatform\UnitTests\DgnDbTestUtils.h>
#include <DgnPlatform\UnitTests\ScopedDgnHost.h>
#include <Grids/GridsApi.h>
#include <DgnPlatform\FunctionalDomain.h>
#include "GridsTestFixtureBase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING
USING_NAMESPACE_CONSTRAINTMODEL
USING_NAMESPACE_GRIDS
USING_NAMESPACE_BENTLEY_SQLITE

//=======================================================================================
// Sets up environment for Grids testing.
// @bsiclass                                    Jonas.Valiunas                  10/2017
//=======================================================================================
struct GridsTestFixture : public GridsTestFixtureBase
    {
    public:
        GridsTestFixture() {};
        ~GridsTestFixture() {};

        void SetUp() override;
        void TearDown() override;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
void GridsTestFixture::SetUp()
    {
    GridsTestFixtureBase::SetUp ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
void GridsTestFixture::TearDown()
    {
    GridsTestFixtureBase::TearDown ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Jonas.Valiunas                  10/2017
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F(GridsTestFixture, CreateOrthogonalGrid)
    {

    }


