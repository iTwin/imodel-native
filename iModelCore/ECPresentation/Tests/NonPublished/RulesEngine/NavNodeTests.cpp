/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+===============+===============+===============+===============+===============+======*/
struct NavNodeTests : ECPresentationTest
{
    static ECDbTestProject* s_project;

    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("NavNodeTests", "RulesEngineTest.01.00.ecschema.xml");
        }

    void TearDown() override
        {
        s_project->GetECDb().AbandonChanges();
        }
};
ECDbTestProject* NavNodeTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodeTests, NavNodeExtendedData_SetRelatedInstanceKey)
    {
    ECClassCP widget = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    NavNodeExtendedData::RelatedInstanceKey* relatedKey = new NavNodeExtendedData::RelatedInstanceKey(ECInstanceKey(widget->GetId(), ECInstanceId(BeInt64Id(123))), "key");
    NavNodeExtendedData extendedData;
    extendedData.SetRelatedInstanceKey(*relatedKey);

    EXPECT_EQ(1, extendedData.GetRelatedInstanceKeys().size());
    EXPECT_STREQ(relatedKey->GetAlias(), extendedData.GetRelatedInstanceKeys()[0].GetAlias());
    EXPECT_EQ(relatedKey->GetInstanceKey(), extendedData.GetRelatedInstanceKeys()[0].GetInstanceKey());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NavNodeTests, NavNodeExtendedData_SetSkippedInstanceKey)
    {
    ECClassCP widget = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECInstanceKey* key = new ECInstanceKey(widget->GetId(), ECInstanceId(BeInt64Id(123)));
    NavNodeExtendedData extendedData;
    extendedData.SetSkippedInstanceKey(*key);

    EXPECT_EQ(1, extendedData.GetSkippedInstanceKeys().size());
    EXPECT_EQ(key->GetClassId(), extendedData.GetSkippedInstanceKeys()[0].GetClassId());
    EXPECT_EQ(key->GetInstanceId(), extendedData.GetSkippedInstanceKeys()[0].GetInstanceId());
    }
