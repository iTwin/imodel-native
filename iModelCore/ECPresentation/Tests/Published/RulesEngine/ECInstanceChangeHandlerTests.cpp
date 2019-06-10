/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
struct DefaultECInstanceChangeHandlerTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    ECDbR m_db;
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

    DefaultECInstanceChangeHandlerTests() : m_db(s_project->GetECDb()) {}

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        s_project->GetECDb().AbandonChanges();
        m_handler = DefaultECInstanceChangeHandler::Create();
        }

    ECValue GetInstanceValue(ECClassInstanceKeyCR key, Utf8CP propertyAccessor)
        {
        ECSqlStatement stmt;
        stmt.Prepare(m_db, Utf8String("SELECT * FROM ").append(key.GetClass()->GetECSqlName()).c_str());
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
    ECEntityClassCP widgetClass = m_db.Schemas().GetClass("RulesEngineTest", "Widget")->GetEntityClassCP();

    EXPECT_TRUE(m_handler->CanHandle(s_project->GetECDb(), *widgetClass));
    
    // insert an empty widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *widgetClass);
    ECClassInstanceKey key = RulesEngineTestHelpers::GetInstanceKey(*widget);

    // verify the value is null
    Utf8CP propertyAccessor = "IntProperty";
    EXPECT_TRUE(GetInstanceValue(key, propertyAccessor).IsNull());

    // change the value
    ECValue valueAfter(123);
    ECInstanceChangeResult result = m_handler->Change(m_db, ChangedECInstanceInfo(*widgetClass, key.GetId()), propertyAccessor, valueAfter);

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
    ECEntityClassCP widgetClass = m_db.Schemas().GetClass("RulesEngineTest", "Widget")->GetEntityClassCP();
    ECEntityClassCP gadgetClass = m_db.Schemas().GetClass("RulesEngineTest", "Gadget")->GetEntityClassCP();
    ECRelationshipClassCP rel = m_db.Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets")->GetRelationshipClassCP();
    RelatedClassPath relationshipPath = {RelatedClass(*widgetClass, *gadgetClass, *rel, true)};
    
    EXPECT_TRUE(m_handler->CanHandle(s_project->GetECDb(), *widgetClass));
        
    // insert an empty widget
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(m_db, *gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(m_db, *rel, *widget, *gadget);
    ECClassInstanceKey widgetKey = RulesEngineTestHelpers::GetInstanceKey(*widget);
    ECClassInstanceKey gadgetKey = RulesEngineTestHelpers::GetInstanceKey(*gadget);

    // verify the value is null
    Utf8CP propertyAccessor = "IntProperty";
    EXPECT_TRUE(GetInstanceValue(widgetKey, propertyAccessor).IsNull());

    // change the value
    ECValue valueAfter(123);
    ChangedECInstanceInfo changeInfo(*gadgetClass, gadgetKey.GetId(), *widgetClass, widgetKey.GetId(), relationshipPath);
    ECInstanceChangeResult result = m_handler->Change(m_db, changeInfo, propertyAccessor, valueAfter);

    // verify
    EXPECT_EQ(SUCCESS, result.GetStatus());
    EXPECT_EQ(valueAfter, result.GetChangedValue());
    EXPECT_EQ(valueAfter, GetInstanceValue(widgetKey, propertyAccessor));
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2017
+===============+===============+===============+===============+===============+======*/
struct CustomECDb : ECDb
    {
    CustomECDb() {ApplyECDbSettings(true, false);}
    };
/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultECInstanceChangeHandlerTests, CantHandleECDbsWhichRequireWriteToken)
    {
    ECDbTestProject project(new CustomECDb());
    project.Create("CantHandleECDbsWhichRequireWriteToken", "RulesEngineTest.01.00.ecschema.xml");
    ECEntityClassCP widgetClass = project.GetECDb().Schemas().GetClass("RulesEngineTest", "Widget")->GetEntityClassCP();
    EXPECT_FALSE(m_handler->CanHandle(project.GetECDb(), *widgetClass));
    }