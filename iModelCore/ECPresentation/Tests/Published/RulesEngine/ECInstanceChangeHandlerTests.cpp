/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/RulesEngine/ECInstanceChangeHandlerTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentation/RulesDriven/ECInstanceChangeHandlers.h>
#include "../../NonPublished/RulesEngine/ECDbTestProject.h"
#include "../../NonPublished/RulesEngine/TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2017
+===============+===============+===============+===============+===============+======*/
struct DefaultECInstanceChangeHandlerTests : ::testing::Test
    {
    static ECDbTestProject* s_project;
    RefCountedPtr<DefaultECInstanceChangeHandler> m_handler;
    
    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("DefaultECInstanceChangeHandlerTests", "RulesEngineTest.01.00.ecschema.xml");
        }

    static void TearDownTestCase()
        {
        DELETE_AND_CLEAR(s_project);
        }

    void SetUp() override
        {
        s_project->GetECDb().AbandonChanges();
        m_handler = DefaultECInstanceChangeHandler::Create();
        }

    ECValue GetInstanceValue(ECInstanceKeyCR key, Utf8CP propertyAccessor)
        {
        ECClassCP ecClass = s_project->GetECDb().Schemas().GetClass(key.GetClassId());

        ECSqlStatement stmt;
        stmt.Prepare(s_project->GetECDb(), Utf8String("SELECT * FROM ").append(ecClass->GetECSqlName()).c_str());
        stmt.Step();

        ECInstanceECSqlSelectAdapter adapter(stmt);
        IECInstancePtr instance = adapter.GetInstance();

        ECValue v;
        instance->GetValue(v, propertyAccessor);
        return v;
        }
    };
ECDbTestProject* DefaultECInstanceChangeHandlerTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultECInstanceChangeHandlerTests, ChangesPrimaryInstanceValue)
    {
    ECEntityClassCP widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget")->GetEntityClassCP();
    
    // insert an empty widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *widgetClass);
    ECInstanceKey key = RulesEngineTestHelpers::GetInstanceKey(*widget);

    // verify the value is null
    Utf8CP propertyAccessor = "IntProperty";
    EXPECT_TRUE(GetInstanceValue(key, propertyAccessor).IsNull());

    // change the value
    ECValue valueAfter(123);
    ECInstanceChangeResult result = m_handler->Change(s_project->GetECDb(), ChangedECInstanceInfo(*widgetClass, key.GetInstanceId()), propertyAccessor, valueAfter);

    // verify
    EXPECT_EQ(SUCCESS, result.GetStatus());
    EXPECT_EQ(valueAfter, result.GetChangedValue());
    EXPECT_EQ(valueAfter, GetInstanceValue(key, propertyAccessor));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultECInstanceChangeHandlerTests, ChangesRelatedInstanceValue)
    {
    ECEntityClassCP widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget")->GetEntityClassCP();
    ECEntityClassCP gadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget")->GetEntityClassCP();
    ECRelationshipClassCP rel = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    RelatedClassPath relationshipPath = {RelatedClass(*widgetClass, *gadgetClass, *rel, true)};
        
    // insert an empty widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *rel, *widget, *gadget);
    ECInstanceKey widgetKey = RulesEngineTestHelpers::GetInstanceKey(*widget);
    ECInstanceKey gadgetKey = RulesEngineTestHelpers::GetInstanceKey(*gadget);

    // verify the value is null
    Utf8CP propertyAccessor = "IntProperty";
    EXPECT_TRUE(GetInstanceValue(widgetKey, propertyAccessor).IsNull());

    // change the value
    ECValue valueAfter(123);
    ChangedECInstanceInfo changeInfo(*gadgetClass, gadgetKey.GetInstanceId(), *widgetClass, widgetKey.GetInstanceId(), relationshipPath);
    ECInstanceChangeResult result = m_handler->Change(s_project->GetECDb(), changeInfo, propertyAccessor, valueAfter);

    // verify
    EXPECT_EQ(SUCCESS, result.GetStatus());
    EXPECT_EQ(valueAfter, result.GetChangedValue());
    EXPECT_EQ(valueAfter, GetInstanceValue(widgetKey, propertyAccessor));
    }
