/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/RulesEngine/PresentationManagerTests.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <UnitTests/BackDoor/ECPresentation/TestRuleSetLocater.h>
#include "../../NonPublished/RulesEngine/TestHelpers.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;

    TestConnectionManager m_connections;
    RulesDrivenECPresentationManager* m_manager;
    TestRuleSetLocaterPtr m_locater;
    
    static void SetUpTestCase();
    static void TearDownTestCase();
    static void InitTestL10N();
    static void ShutDownTestL10N();
    static void RegisterSchemaXml(Utf8String name, Utf8String schemaXml);
    
    RulesDrivenECPresentationManagerTests() : m_manager(nullptr) {}
    virtual void SetUp() override;
    virtual void TearDown() override;

    ECSchemaCP GetSchema();
    ECClassCP GetClass(Utf8CP name);
    ECClassCP GetClass(Utf8CP schemaName, Utf8CP className);
    ECRelationshipClassCP GetRelationshipClass(Utf8CP name);
    ECRelationshipClassCP GetRelationshipClass(Utf8CP schemaName, Utf8CP className);
    Utf8String GetClassNamesList(bvector<ECClassCP> const& classes);
    Utf8String GetDisplayLabel(IECInstanceCR instance);
    };

#define DEFINE_SCHEMA(name, schema_xml) DEFINE_REGISTRY_SCHEMA(RulesDrivenECPresentationManagerTests, name, schema_xml)

END_ECPRESENTATIONTESTS_NAMESPACE