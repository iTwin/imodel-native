/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "NestedStructArrayTestSchemaHelper.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct ECInstanceFinderTests : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECInstanceFinderTests, FindRelatedInstances_MultipleSeedInstancesOfSameClass_AllRelatedInstances)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("FindRelatedInstances.ecdb", SchemaItem::CreateForFile("ECSqlStatementTests.01.00.00.ecschema.xml")));
    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);

    ECSqlStatement stmt;
    ECInstanceKeyMultiMap seedInstances;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECClassId, ECInstanceId FROM ECST.Customer"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    seedInstances.insert({stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1)});
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    seedInstances.insert({stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1)});
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    // Skipping this instance to make sure that results are filtered by seed instances
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    seedInstances.insert({stmt.GetValueId<ECClassId>(0), stmt.GetValueId<ECInstanceId>(1)});
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    auto instanceRange = seedInstances.equal_range(seedInstances.begin().key());
    // Assert that all customer instances belong to the same ECClass
    ASSERT_EQ(seedInstances.size(), std::distance(instanceRange.first, instanceRange.second));

    ECInstanceKeyMultiMap relatedInstances;
    ASSERT_EQ(SUCCESS, ECInstanceFinder(m_ecdb).FindRelatedInstances
    (
        &relatedInstances,
        nullptr,
        seedInstances,
        ECInstanceFinder::RelatedDirection::RelatedDirection_All
    ));

    EXPECT_EQ(6, relatedInstances.size());
    }

END_ECDBUNITTESTS_NAMESPACE
