/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include "../../../Source/RulesDriven/RulesEngine/QueryBuilder.h"
#include "TestNavNode.h"
#include "ExpectedQueries.h"
#include "TestHelpers.h"
#include "TestLocalizationProvider.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

#define DEFINE_SCHEMA(name, schema_xml) DEFINE_REGISTRY_SCHEMA(ExpectedQueries, name, schema_xml)

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryBuilderTests : ECPresentationTest
    {
    ECSchemaHelper* m_schemaHelper;
    PresentationRuleSetPtr m_ruleset;
    RootNodeRuleP m_rootNodeRule;
    ChildNodeRuleP m_childNodeRule;
    NavigationQueryBuilder* m_builder;
    RulesetVariables m_rulesetVariables;
    IConnectionCPtr m_connection;
    RuleSetLocaterManager m_locaterManager;
    TestNodesCache m_nodesCache;

    NavigationQueryBuilderTests() 
        : m_builder(nullptr), m_locaterManager(ExpectedQueries::GetInstance(BeTest::GetHost()).GetConnections()), m_nodesCache(ExpectedQueries::GetInstance(BeTest::GetHost()).GetConnections())
        {}
    
    void SetUp() override;
    void TearDown() override;

    NavigationQueryBuilder& GetBuilder() {return *m_builder;}
    ECSchemaCP GetECSchema();
    ECClassCP GetECClass(Utf8CP schemaName, Utf8CP className);
    ECClassCP GetECClass(Utf8CP className);
    bvector<ECClassCP> GetECClasses(Utf8CP schemaName);
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryBuilder_MultiLevelGroupingTests : NavigationQueryBuilderTests
    {
    DEFINE_T_SUPER(NavigationQueryBuilderTests);

    AllInstanceNodesSpecificationP m_specification1;
    InstanceNodesOfSpecificClassesSpecificationP m_specification2;
    AllRelatedInstanceNodesSpecificationP m_specification3;
    
    void SetUp() override;
    void TearDown() override;
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct ContentQueryBuilderTests : ECPresentationTest
    {    
    ECSchemaHelper* m_schemaHelper;
    PresentationRuleSetPtr m_ruleset;
    ContentDescriptorBuilder::Context* m_context;
    ContentDescriptorBuilder* m_descriptorBuilder;
    ContentQueryBuilder* m_queryBuilder;
    RulesetVariables m_rulesetVariables;
    RuleSetLocaterManager m_locaterManager;
    TestNodeLocater m_nodesLocater;
    TestLocalizationProvider m_localizationProvider;
    DefaultCategorySupplier m_categorySupplier;

    ContentQueryBuilderTests() 
        : m_descriptorBuilder(nullptr), m_queryBuilder(nullptr), m_locaterManager(ExpectedQueries::GetInstance(BeTest::GetHost()).GetConnections())
        {}
    
    void SetUp() override;
    void TearDown() override;

    ContentDescriptorBuilder& GetDescriptorBuilder() {return *m_descriptorBuilder;}
    ContentQueryBuilder& GetQueryBuilder() {return *m_queryBuilder;}
    ECSchemaCP GetECSchema();
    ECClassCP GetECClass(Utf8CP schemaName, Utf8CP className);
    ECClassCP GetECClass(Utf8CP className);
    ContentQueryCPtr GetExpectedQuery();
    };

END_ECPRESENTATIONTESTS_NAMESPACE
