/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/RulesEngine/PresentationManagerTests.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <UnitTests/BackDoor/ECPresentation/TestRuleSetLocater.h>
#include "../../NonPublished/RulesEngine/TestHelpers.h"

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerTests : ::testing::Test
    {
    static ECDbTestProject* s_project;

    RulesDrivenECPresentationManager* m_manager;
    TestRuleSetLocaterPtr m_locater;
    
    static void SetUpTestCase();
    static void TearDownTestCase();
    static void InitTestL10N();
    static void ShutDownTestL10N();

    RulesDrivenECPresentationManagerTests() : m_manager(nullptr) {}
    Utf8String GetDisplayLabel(IECInstanceCR instance);
    void SetUp() override;
    void TearDown() override;
    };
