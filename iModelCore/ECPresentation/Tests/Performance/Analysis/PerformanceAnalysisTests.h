/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationManager.h>
#include "../PerformanceTests.h"
#include "Reporters.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

#define REPORT_FIELD_Dataset                        "Dataset"
#define REPORT_FIELD_RulesetId                      "RulesetId"
#define REPORT_FIELD_TimeToLoadAllSchemas           "Time to load all schemas"

#define RULESETS_CONFIG_FILENAME                   L"Rulesets.config"

/*=================================================================================**//**
* Configuration for RulesEnginePerformanceAnalysisTests.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesEnginePerformanceAnalysisReporterConfig
    {
    Utf8String fileName = "Report";
    bvector<Utf8String> fieldNames;
    bvector<Utf8String> jsonGroupingFieldNames;
    bvector<Utf8String> csvTestNameFieldNames;
    bvector<Utf8String> csvExportedFieldNames;
    };

/*=================================================================================**//**
* Configuration for RulesEnginePerformanceAnalysisTests.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesEnginePerformanceAnalysisTestsConfig
    {
    int threadsCount = 1;
    Utf8String rulesetsSubDirectory;
    };

/*=================================================================================**//**
* Abstract base class for performance analysis tests
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesEnginePerformanceAnalysisTests : ECPresentationTest
{
    struct RulesetConfig
        {
        int maxDepth = -1;
        };

protected:
    RulesEnginePerformanceAnalysisReporterConfig m_reporterConfig;
    std::unique_ptr<Reporter> m_reporter;
    bmap<Utf8String, RulesetConfig> m_rulesetConfigs;

private:
    void SaveReport();
    void LoadRulesetConfigs();
    BentleyStatus ParseRulesetConfig(JsonValueCR, RulesetConfig&);

protected:
    static void SetUpTestCase();
    virtual void SetUp();
    virtual void TearDown();

    virtual Reporter& _GetJsonReporter();
    virtual Reporter& _GetCsvReporter();
};

/*=================================================================================**//**
* Abstract base class for performance analysis tests that uses only one presentation
* manager.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SingleManagerRulesEnginePerformanceAnalysisTests : RulesEnginePerformanceAnalysisTests
{
protected:
    RulesEnginePerformanceAnalysisTestsConfig m_config;
    ECPresentationManager* m_manager = nullptr;
    RuntimeJsonLocalState m_localState;
    TestRuleSetLocaterPtr m_locater;

protected:
    virtual void SetUp();
    virtual void TearDown();

    void Reset(ECDb* project = nullptr);
    void ForEachDatasetAndRuleset(std::function<void(Reporter&, ECDbR, Utf8StringCR)> testCaseRunner);
};

END_ECPRESENTATIONTESTS_NAMESPACE
