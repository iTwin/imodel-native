/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../../Helpers/TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavNodeTests : ECPresentationTest
{
    static ECDbTestProject* s_project;

    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("NavNodeTests", "RulesEngineTest.01.00.ecschema.xml");
        }

    static void TearDownTestCase()
        {
        DELETE_AND_CLEAR(s_project);
        }

    void TearDown() override
        {
        s_project->GetECDb().AbandonChanges();
        }
};
ECDbTestProject* NavNodeTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodeTests, NavNodeExtendedData_SetRelatedInstanceKey)
    {
    ECClassCP widget = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    NavNodeExtendedData::RelatedInstanceKey relatedKey(ECInstanceKey(widget->GetId(), ECInstanceId(BeInt64Id(123))), "key");
    NavNodeExtendedData extendedData;
    extendedData.SetRelatedInstanceKey(relatedKey);

    EXPECT_EQ(1, extendedData.GetRelatedInstanceKeys().size());
    EXPECT_STREQ(relatedKey.GetAlias(), extendedData.GetRelatedInstanceKeys()[0].GetAlias());
    EXPECT_EQ(relatedKey.GetInstanceKey(), extendedData.GetRelatedInstanceKeys()[0].GetInstanceKey());
    }
