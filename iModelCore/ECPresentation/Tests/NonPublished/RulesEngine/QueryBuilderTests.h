/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/QueryBuilderTests.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
struct NavigationQueryBuilderTests : ::testing::Test
    {
    ECSchemaHelper* m_schemaHelper;
    PresentationRuleSetPtr m_ruleset;
    RootNodeRuleP m_rootNodeRule;
    ChildNodeRuleP m_childNodeRule;
    NavigationQueryBuilder* m_builder;
    TestUserSettings m_settings;
    RelatedPathsCache m_relatedPathsCache;
    ECExpressionsCache m_expressionsCache;
    RuleSetLocaterManager m_locaterManager;
    TestNodesCache m_nodesCache;

    NavigationQueryBuilderTests() : m_builder(nullptr) {}
    
    void SetUp() override;
    void TearDown() override;

    NavigationQueryBuilder& GetBuilder() {return *m_builder;}
    ECClassCP GetECClass(Utf8CP schemaName, Utf8CP className);
    bvector<ECClassCP> GetECClasses(Utf8CP schemaName);
    Utf8String GetInstanceIdWithFormat(Utf8CP format, NavNodeCR node);
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
* @bsiclass                                     Grigas.Petraitis                07/2017
+===============+===============+===============+===============+===============+======*/
struct TestParsedSelectionInfo : IParsedSelectionInfo
{
private:
    bvector<ECClassCP> m_classes;
    bmap<ECClassCP, bvector<ECInstanceId>> m_instanceIds;
protected:
    bvector<ECClassCP> const& _GetClasses() const override {return m_classes;}
    bvector<ECInstanceId> const& _GetInstanceIds(ECClassCR ecClass) const override
        {
        auto iter = m_instanceIds.find(&ecClass);
        if (m_instanceIds.end() != iter)
            return iter->second;
        static bvector<ECInstanceId> s_empty;
        return s_empty;
        }
public:
    TestParsedSelectionInfo() {}
    TestParsedSelectionInfo(ECClassCR ecClass, ECInstanceId instanceId)
        {
        m_classes.push_back(&ecClass);
        m_instanceIds[&ecClass].push_back(instanceId);
        }
    TestParsedSelectionInfo(ECClassCR ecClass, bvector<ECInstanceId> instanceIds)
        {
        m_classes.push_back(&ecClass);
        m_instanceIds[&ecClass] = instanceIds;
        }
    TestParsedSelectionInfo(bvector<bpair<ECClassCP, ECInstanceId>> pairs)
        {
        bset<ECClassCP> used;
        for (auto pair : pairs)
            {
            if (used.end() == used.find(pair.first))
                {
                m_classes.push_back(pair.first);
                used.insert(pair.first);
                }
            m_instanceIds[pair.first].push_back(pair.second);
            }
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct ContentQueryBuilderTests : ::testing::Test
    {    
    ECSchemaHelper* m_schemaHelper;
    PresentationRuleSetPtr m_ruleset;
    ContentDescriptorBuilder* m_descriptorBuilder;
    ContentQueryBuilder* m_queryBuilder;
    TestUserSettings m_settings;
    RuleSetLocaterManager m_locaterManager;
    TestNodeLocater m_nodesLocater;
    TestLocalizationProvider m_localizationProvider;
    DefaultCategorySupplier m_categorySupplier;
    ECExpressionsCache m_expressionsCache;
    RelatedPathsCache m_relatedPathsCache;

    ContentQueryBuilderTests() : m_descriptorBuilder(nullptr), m_queryBuilder(nullptr) {}
    
    void SetUp() override;
    void TearDown() override;

    ContentDescriptorBuilder& GetDescriptorBuilder() {return *m_descriptorBuilder;}
    ContentQueryBuilder& GetQueryBuilder() {return *m_queryBuilder;}
    ECClassCP GetECClass(Utf8CP schemaName, Utf8CP className);
    ECClassCP GetECClass(Utf8CP className);
    ContentQueryCPtr GetExpectedQuery();
    };

END_ECPRESENTATIONTESTS_NAMESPACE