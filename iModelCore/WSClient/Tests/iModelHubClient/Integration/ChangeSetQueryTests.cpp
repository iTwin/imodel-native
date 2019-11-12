/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "Helpers.h"
#include <WebServices/iModelHub/Client/ChangeSetQuery.h>
#include <Bentley/BeTest.h>
#include <WebServices/iModelHub/Utils.h>

USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_IMODELHUB

using namespace ::testing;
using namespace ::std;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChangeSetQueryTests : public Test {};

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChangeSetQueryTests, SelectBridgeProperties)
    {
    ChangeSetQuery query;
    query.SelectBridgeProperties();

    Utf8String expectedSelect("*,HasBridgeProperties-forward-BridgeProperties.*");
    ASSERT_EQ(expectedSelect, query.GetWSQuery().GetSelect());
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChangeSetQueryTests, FilterChangeSetsBetweenAndById)
    {
    ChangeSetQuery query;
    query.FilterChangeSetsBetween("changeSet1", "changeSet2");
    query.FilterById("changeSet1");

    Utf8String expectedFilter("((CumulativeChangeSet-backward-ChangeSet.Id+eq+'changeSet1'+and+FollowingChangeSet-backward-ChangeSet.Id+eq+'changeSet2')+or+(CumulativeChangeSet-backward-ChangeSet.Id+eq+'changeSet2'+and+FollowingChangeSet-backward-ChangeSet.Id+eq+'changeSet1'))+and+$id+in+['changeSet1']");
    ASSERT_EQ(expectedFilter, query.GetWSQuery().GetFilter());
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChangeSetQueryTests, FilterChangeSetsByIds)
    {
    ChangeSetQuery query;
    std::deque<ObjectId> filterIds;
    filterIds.push_back(ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet, "changeSet1"));
    filterIds.push_back(ObjectId(ServerSchema::Schema::iModel, ServerSchema::Class::ChangeSet, "changeSet2"));
    query.FilterByIds(filterIds);

    Utf8String expectedFilter("$id+in+['changeSet1','changeSet2']");
    ASSERT_EQ(expectedFilter, query.GetWSQuery().GetFilter());
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChangeSetQueryTests, FilterChangeSetsAfterId)
    {
    ChangeSetQuery query;
    query.FilterChangeSetsAfterId("changeSet2");

    Utf8String expectedFilter("FollowingChangeSet-backward-ChangeSet.Id+eq+'changeSet2'");
    ASSERT_EQ(expectedFilter, query.GetWSQuery().GetFilter());
    }