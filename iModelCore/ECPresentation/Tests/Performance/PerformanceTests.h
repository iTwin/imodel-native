/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationManager.h>
#include <UnitTests/ECPresentation/ECPresentationTest.h>
#include <UnitTests/ECPresentation/TestRuleSetLocater.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

#define LOGGER_NAMESPACE "Performance.RulesEngine"

/*=================================================================================**//**
* Abstract base test class for all tests that use rules driven presentation manager
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesEngineTests : ECPresentationTest
{
protected:
    TestRuleSetLocaterPtr m_locater;
    ECPresentationManager* m_manager;

protected:
    RulesEngineTests() : m_manager(nullptr) {}
    static void SetUpTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
    virtual void _SetupRulesets() = 0;
    virtual void _SetupProjects() = 0;
    virtual void _ConfigureManagerParams(ECPresentationManager::Params&);
    void OpenProject(ECDbR project, BeFileNameCR projectPath);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct Timer : StopWatch
    {
    Timer(Utf8CP name = nullptr);
    ~Timer();
    void Finish();
    };

/*=================================================================================**//**
* Abstract base class for tests that use a single ECDb project
* @bsiclass
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
    Utf8StringCR GetRulesetId() const;
};

END_ECPRESENTATIONTESTS_NAMESPACE
