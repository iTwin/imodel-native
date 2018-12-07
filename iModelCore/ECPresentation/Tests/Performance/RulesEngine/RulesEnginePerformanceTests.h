/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/RulesEngine/RulesEnginePerformanceTests.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <UnitTests/BackDoor/ECPresentation/TestRuleSetLocater.h>
#include "../../BackDoor/PublicAPI/BackDoor/ECPresentation/Localization.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

#define LOGGER_NAMESPACE "Performance.RulesEngine"

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct RulesEnginePerformanceTests : ECPresentationTest
{
    struct Timer : StopWatch
        {
        Timer(Utf8CP name = nullptr);
        ~Timer();
        void Finish();
        };

private:
    TestRuleSetLocaterPtr m_locater;

protected:
    ECDb m_project;
    ConnectionManager m_connections;
    RulesDrivenECPresentationManager* m_manager;
    
protected:
    RulesEnginePerformanceTests() : m_manager(nullptr) {}
    static void SetUpTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
    virtual BeFileName _SupplyProjectPath() const = 0;
    virtual PresentationRuleSetPtr _SupplyRuleset() const = 0;
    RulesDrivenECPresentationManager::ContentOptions CreateContentOptions() const;
};

END_ECPRESENTATIONTESTS_NAMESPACE