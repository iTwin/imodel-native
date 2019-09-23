/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <UnitTests/BackDoor/ECPresentation/TestRuleSetLocater.h>
#include "../BackDoor/PublicAPI/BackDoor/ECPresentation/Localization.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

#define LOGGER_NAMESPACE "Performance.RulesEngine"

/*=================================================================================**//**
* Abstract base test class for all tests that use rules driven presentation manager
* @bsiclass                                     Mantas.Kontrimas                07/2018
+===============+===============+===============+===============+===============+======*/
struct RulesEngineTests : ECPresentationTest
{
protected:
    TestRuleSetLocaterPtr m_locater;
    SQLangLocalizationProvider m_localizationProvider;
    RulesDrivenECPresentationManager* m_manager;

protected:
    RulesEngineTests() : m_manager(nullptr) {}
    static void SetUpTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
    virtual void _SetupRulesets() = 0;
    virtual void _SetupProjects() = 0;
    virtual void _ConfigureManagerParams(RulesDrivenECPresentationManager::Params&);
    void OpenProject(ECDbR project, BeFileNameCR projectPath);
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct Timer : StopWatch
    {
    Timer(Utf8CP name = nullptr);
    ~Timer();
    void Finish();
    };

/*=================================================================================**//**
* Abstract base class for tests that use a single ECDb project
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct RulesEngineSingleProjectTests : RulesEngineTests
{
protected:
    ECDb m_project;

protected:
    static void SetUpTestCase();
    void _SetupRulesets() override;
    void _SetupProjects() override;
    virtual BeFileName _SupplyProjectPath() const = 0;
    virtual PresentationRuleSetPtr _SupplyRuleset() const = 0;
    RulesDrivenECPresentationManager::ContentOptions CreateContentOptions() const;
};

END_ECPRESENTATIONTESTS_NAMESPACE