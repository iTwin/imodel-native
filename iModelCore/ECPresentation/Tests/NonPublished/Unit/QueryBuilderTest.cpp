/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "QueryBuilderTest.h"
#include "../Helpers/TestHelpers.h"
#include "../../../Source/Shared/Queries/CustomFunctions.h"
#include "../../../Source/Shared/ECSchemaHelper.h"
#include "../../../Source/Shared/RulesPreprocessor.h"
#include "../../../Source/Content/ContentQueryBuilder.h"
#include "../../../Source/Hierarchies/NavigationQueryBuilder.h"
#include <Bentley/Logging.h>

DEFINE_SCHEMA_REGISTRY(QueryBuilderTest)
ECDbTestProject* QueryBuilderTest::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderTest::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("QueryBuilderTest");
    INIT_SCHEMA_REGISTRY(s_project->GetECDb());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderTest::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderTest::SetUp()
    {
    ECPresentationTest::SetUp();
    m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
    m_cancellationToken = SimpleCancelationToken::Create();
    m_schemaHelper = std::make_shared<ECSchemaHelper>(*m_connection, nullptr, nullptr);
    m_customFunctions = std::make_shared<CustomFunctionsInjector>(GetConnections(), GetConnection());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderTest::TearDown()
    {
    for (PresentationKey const* rule : m_presentationRulesToDelete)
        delete rule;
    m_presentationRulesToDelete.clear();
    GetDb().AbandonChanges();
    ECPresentationTest::TearDown();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP QueryBuilderTest::GetECClassP(Utf8CP schemaName, Utf8CP className)
    {
    return const_cast<ECClassP>(m_schemaHelper->GetECClass(schemaName, className));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP QueryBuilderTest::GetECClass(Utf8CP schemaName, Utf8CP className) {return GetECClassP(schemaName, className);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP QueryBuilderTest::GetECClass(Utf8CP className) {return GetECClassP(BeTest::GetNameOfCurrentTest(), className);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP QueryBuilderTest::GetECSchema(Utf8CP schemaName) {return m_schemaHelper->GetSchema(schemaName, true);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP QueryBuilderTest::GetECSchema() {return m_schemaHelper->GetSchema(BeTest::GetNameOfCurrentTest(), true);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECClassCP> QueryBuilderTest::GetECClasses(Utf8CP schemaName)
    {
    bvector<ECClassCP> classes;
    bset<ECClassCP> set;
    ECSchemaCP schema = GetDb().Schemas().GetSchema(schemaName);
    if (nullptr != schema)
        {
        ECClassContainerCR classContainer = schema->GetClasses();
        for (ECClassCP ecClass : classContainer)
            {
            if (set.end() != set.find(ecClass))
                continue;

            classes.push_back(ecClass);
            set.insert(ecClass);
            }
        }
    return classes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderTest::PrepareQuery(Utf8StringCR queryString, Utf8StringCR queryName)
    {
    ECSqlStatement statement;
    ECSqlStatus status = statement.Prepare(GetDb(), queryString.c_str());
    EXPECT_EQ(ECSqlStatus::Success, status)
        << "Failure: " << GetDb().GetLastError() << "\r\n"
        << "Failed query: " << queryName.c_str() << "\r\n" << queryString.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldPtr QueryBuilderTest::CreateDisplayLabelField(SelectClass<ECClass> const& selectClass,
    bvector<RelatedClassPath> const& relatedInstancePaths, bvector<InstanceLabelOverrideValueSpecification*> const& instanceLabelOverrides)
    {
    return RulesEngineTestHelpers::CreateDisplayLabelField(*m_schemaHelper, selectClass, relatedInstancePaths,
        ContainerHelpers::TransformContainer<bvector<InstanceLabelOverrideValueSpecification const*>>(instanceLabelOverrides));
    }
