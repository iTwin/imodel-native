/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/Connection.h>
#include "../../../Source/Shared/Queries/QueryExecutor.h"
#include "../../../Source/Shared/ECSchemaHelper.h"
#include "../../../Source/Content/ContentQuery.h"
#include "../../../Source/Hierarchies/NavigationQuery.h"
#include "../Helpers/ECDbTestProject.h"
#include "../Helpers/TestHelpers.h"
#include "../Helpers/TestNavNode.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryBuilderTest : ECPresentationTest
{
    DECLARE_SCHEMA_REGISTRY(QueryBuilderTest)
    static ECDbTestProject* s_project;

private:
    std::shared_ptr<ECSchemaHelper> m_schemaHelper;
    TestConnectionManager m_connections;
    IConnectionPtr m_connection;
    SimpleCancelationTokenPtr m_cancellationToken;
    std::shared_ptr<CustomFunctionsInjector> m_customFunctions;
    bvector<PresentationKey const*> m_presentationRulesToDelete;

protected:
    template<typename TRule> TRule& RegisterForDelete(TRule& spec) {m_presentationRulesToDelete.push_back(&spec); return spec;}

public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;

    ECDbR GetDb() {return s_project->GetECDb();}
    IConnectionCR GetConnection() {return *m_connection;}
    IConnectionManagerCR GetConnections() {return m_connections;}
    SimpleCancelationToken& GetCancellationToken() const {return *m_cancellationToken;}
    ECSchemaHelper& GetSchemaHelper() {return *m_schemaHelper;}

    ECClassP GetECClassP(Utf8CP schemaName, Utf8CP className);
    ECClassCP GetECClass(Utf8CP schemaName, Utf8CP className);
    ECClassCP GetECClass(Utf8CP className);
    ECSchemaCP GetECSchema(Utf8CP schemaName);
    ECSchemaCP GetECSchema();
    bvector<ECClassCP> GetECClasses(Utf8CP schemaName);

    void PrepareQuery(Utf8StringCR queryString, Utf8StringCR queryName = BeTest::GetNameOfCurrentTest());

    PresentationQueryContractFieldPtr CreateDisplayLabelField(SelectClass<ECClass> const&, bvector<RelatedClassPath> const& = {}, bvector<InstanceLabelOverrideValueSpecification*> const& = {});
    };

#define DEFINE_SCHEMA(name, schema_xml) DEFINE_REGISTRY_SCHEMA(QueryBuilderTest, name, schema_xml)

END_ECPRESENTATIONTESTS_NAMESPACE
