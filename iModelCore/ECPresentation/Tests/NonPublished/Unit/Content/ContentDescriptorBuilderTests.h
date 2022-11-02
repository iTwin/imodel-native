/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <UnitTests/ECPresentation/ECPresentationTest.h>
#include "../../../../Source/Content/ContentQueryBuilder.h"
#include "../../Helpers/TestHelpers.h"
#include "../../Helpers/TestNavNode.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentDescriptorBuilderTests : ECPresentationTest
    {
    DECLARE_SCHEMA_REGISTRY(ContentDescriptorBuilderTests);
    static std::unique_ptr<ECDbTestProject> s_project;

    TestConnectionManager m_connections;
    IConnectionPtr m_connection;
    SimpleCancelationTokenPtr m_cancellationToken;
    std::unique_ptr<ECSchemaHelper> m_schemaHelper;
    PresentationRuleSetPtr m_ruleset;
    std::unique_ptr<CustomFunctionsInjector> m_customFunctions;
    std::unique_ptr<ContentDescriptorBuilder::Context> m_context;
    std::unique_ptr<ContentDescriptorBuilder> m_descriptorBuilder;
    RulesetVariables m_rulesetVariables;
    DefaultCategorySupplier m_categorySupplier;
    std::unique_ptr<IRulesPreprocessor> m_rulesPreprocessor;

    ContentDescriptorBuilderTests() : m_descriptorBuilder(nullptr) {}

    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();

    ContentDescriptorBuilder& GetDescriptorBuilder() {return *m_descriptorBuilder;}
    ECSchemaCP GetSchema();
    ECClassCP GetClass(Utf8CP schemaName, Utf8CP className);
    ECClassCP GetClass(Utf8CP className);
    };

END_ECPRESENTATIONTESTS_NAMESPACE
