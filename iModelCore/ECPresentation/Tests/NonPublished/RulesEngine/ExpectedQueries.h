/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/Connection.h>
#include "../../../Source/RulesDriven/RulesEngine/QueryExecutor.h"
#include "../../../Source/RulesDriven/RulesEngine/ECSchemaHelper.h"
#include "ECDbTestProject.h"
#include "TestHelpers.h"
#include "TestNavNode.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

#define SCHEMA_BASIC_1  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                                                                                            \
                        "<ECSchema schemaName=\"Basic1\" alias=\"b1\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"              \
                        "    <ECEntityClass typeName=\"Class1A\">"                                                                                              \
                        "        <ECProperty propertyName=\"Name\" typeName=\"int\" />"                                                                        \
                        "    </ECEntityClass>"                                                                                                                  \
                        "    <ECEntityClass typeName=\"Class1B\">"                                                                                              \
                        "        <ECProperty propertyName=\"Name\" typeName=\"double\" />"                                                                     \
                        "    </ECEntityClass>"                                                                                                                  \
                        "    <ECEntityClass typeName=\"Class2\">"                                                                                               \
                        "        <ECProperty propertyName=\"Name\" typeName=\"int\" />"                                                                        \
                        "        <ECProperty propertyName=\"CategorizedProperty\" typeName=\"int\" category=\"CategoryName\" />"                                \
                        "    </ECEntityClass>"                                                                                                                  \
                        "    <PropertyCategory typeName=\"CategoryName\" displayLabel=\"Category Label\" description=\"Category description\" priority=\"1\" />"\
                        "</ECSchema>"

#define SCHEMA_BASIC_2  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                                                                                            \
                        "<ECSchema schemaName=\"Basic2\" alias=\"b2\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"              \
                        "    <ECEntityClass typeName=\"Class2\">"                                                                                               \
                        "       <ECProperty propertyName=\"Name\" typeName=\"string\" priority=\"1200\"/>"                                                      \
                        "       <ECProperty propertyName=\"Hidden\" typeName=\"string\">"                                                                       \
                        "           <ECCustomAttributes>"                                                                                                       \
                        "               <HiddenProperty xmlns=\"CoreCustomAttributes.01.00\"/>"                                                                \
                        "           </ECCustomAttributes>"                                                                                                      \
                        "       </ECProperty>"                                                                                                                  \
                        "    </ECEntityClass>"                                                                                                                  \
                        "</ECSchema>"

#define SCHEMA_BASIC_3  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                                                                                            \
                        "<ECSchema schemaName=\"Basic3\" alias=\"b3\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"              \
                        "    <ECEntityClass typeName=\"Class3\" displayLabel=\"Test Class 3\">"                                                                 \
                        "        <ECProperty propertyName=\"SomeProperty\" typeName=\"string\" />"                                                              \
                        "    </ECEntityClass>"                                                                                                                  \
                        "</ECSchema>"

#define SCHEMA_BASIC_4  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                                                                                            \
                        "<ECSchema schemaName=\"Basic4\" alias=\"b4\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"              \
                        "    <ECEntityClass typeName=\"ClassA\">"                                                                                               \
                        "        <ECProperty propertyName=\"SomeProperty\" typeName=\"int\" />"                                                                 \
                        "        <ECProperty propertyName=\"Description\" typeName=\"string\" />"                                                               \
                        "    </ECEntityClass>"                                                                                                                  \
                        "    <ECEntityClass typeName=\"ClassB\">"                                                                                               \
                        "        <BaseClass>ClassA</BaseClass>"                                                                                                 \
                        "    </ECEntityClass>"                                                                                                                  \
                        "    <ECEntityClass typeName=\"ClassC\">"                                                                                               \
                        "        <BaseClass>ClassB</BaseClass>"                                                                                                 \
                        "        <ECNavigationProperty propertyName=\"B\" relationshipName=\"ClassBHasClassC\" direction=\"Backward\" />"                       \
                        "    </ECEntityClass>"                                                                                                                  \
                        "    <ECRelationshipClass typeName=\"ClassBHasClassC\" strength=\"referencing\" strengthDirection=\"forward\" modifier=\"None\">"       \
                        "        <Source multiplicity=\"(1..1)\" roleLabel=\"ClassB Has ClassC\" polymorphic=\"False\">"                                        \
                        "            <Class class=\"ClassB\" />"                                                                                                \
                        "        </Source>"                                                                                                                     \
                        "        <Target multiplicity=\"(1..*)\" roleLabel=\"ClassB Has ClassC\" polymorphic=\"False\">"                                        \
                        "            <Class class=\"ClassC\" />"                                                                                                \
                        "        </Target>"                                                                                                                     \
                        "    </ECRelationshipClass>"                                                                                                            \
                        "</ECSchema>"

#define SCHEMA_COMPLEX_1 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                                                                                           \
                        "<ECSchema schemaName=\"SchemaComplex\" alias=\"sc\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"       \
                        "    <ECEntityClass typeName=\"Class1\">"                                                                                               \
                        "        <ECProperty propertyName=\"PropertyA\" typeName=\"string\" />"                                                                 \
                        "    </ECEntityClass>"                                                                                                                  \
                        "    <ECEntityClass typeName=\"BaseOf2And3\" modifier=\"Abstract\">"                                                                    \
                        "        <ECCustomAttributes>"                                                                                                          \
                        "            <ClassMap xmlns=\"ECDbMap.02.00\">"                                                                                        \
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"                                                                          \
                        "            </ClassMap>"                                                                                                               \
                        "        </ECCustomAttributes>"                                                                                                         \
                        "    </ECEntityClass>"                                                                                                                  \
                        "    <ECEntityClass typeName=\"Class2\">"                                                                                               \
                        "        <BaseClass>BaseOf2And3</BaseClass>"                                                                                            \
                        "        <ECProperty propertyName=\"PropertyB\" typeName=\"string\" />"                                                                 \
                        "        <ECNavigationProperty propertyName=\"C1\" relationshipName=\"Class1HasClass2And3\" direction=\"Backward\" />"                  \
                        "    </ECEntityClass>"                                                                                                                  \
                        "    <ECEntityClass typeName=\"Class3\">"                                                                                               \
                        "        <BaseClass>BaseOf2And3</BaseClass>"                                                                                            \
                        "        <ECProperty propertyName=\"PropertyC\" typeName=\"string\" />"                                                                 \
                        "        <ECProperty propertyName=\"PropertyD\" typeName=\"string\" />"                                                                 \
                        "        <ECNavigationProperty propertyName=\"C1\" relationshipName=\"Class1HasClass2And3\" direction=\"Backward\" />"                  \
                        "    </ECEntityClass>"                                                                                                                  \
                        "    <ECRelationshipClass typeName=\"Class1HasClass2And3\" strength=\"referencing\" strengthDirection=\"forward\" modifier=\"None\">"   \
                        "        <Source multiplicity=\"(1..1)\" roleLabel=\"Class1 Has Class2 And 3\" polymorphic=\"True\">"                                   \
                        "            <Class class=\"Class1\" />"                                                                                                \
                        "        </Source>"                                                                                                                     \
                        "        <Target multiplicity=\"(1..*)\" roleLabel=\"Class1 Has Class2 And 3 (reversed)\" polymorphic=\"True\" abstractConstraint=\"BaseOf2And3\">" \
                        "            <Class class=\"Class2\" />"                                                                                                \
                        "            <Class class=\"Class3\" />"                                                                                                \
                        "        </Target>"                                                                                                                     \
                        "    </ECRelationshipClass>"                                                                                                            \
                        "</ECSchema>"

#define SCHEMA_COMPLEX_3 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                                                                                           \
                        "<ECSchema schemaName=\"SchemaComplex3\" alias=\"sc3\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"     \
                        "    <ECEntityClass typeName=\"GroupingClass\">"                                                                                        \
                        "    </ECEntityClass>"                                                                                                                  \
                        "    <ECEntityClass typeName=\"ChildClassWithNavigationProperty\">"                                                                     \
                        "        <ECNavigationProperty propertyName=\"Group\" relationshipName=\"NavigationGrouping\" direction=\"Backward\" />"                \
                        "    </ECEntityClass>"                      \
                        "    <ECRelationshipClass typeName=\"NavigationGrouping\" strength=\"referencing\" strengthDirection=\"forward\" modifier=\"None\">"    \
                        "        <Source multiplicity=\"(0..1)\" roleLabel=\"Label\" polymorphic=\"True\">"                                                     \
                        "            <Class class=\"GroupingClass\" />"                                                                                         \
                        "        </Source>"                                                                                                                     \
                        "        <Target multiplicity=\"(0..*)\" roleLabel=\"Label\" polymorphic=\"True\">"                                                     \
                        "            <Class class=\"ChildClassWithNavigationProperty\" />"                                                                      \
                        "        </Target>"                                                                                                                     \
                        "    </ECRelationshipClass>"         \
                        "</ECSchema>"

#define SEARCH_NODE_QUERY "SELECT MyID, IntProperty FROM [RulesEngineTest].[Widget] WHERE [Widget].[ECInstanceId] > 0"
#define SEARCH_NODE_QUERY_PROCESSED "SELECT MyID, IntProperty, ECInstanceId AS [" \
                                    SEARCH_QUERY_FIELD_ECInstanceId "], ECClassId AS [" \
                                    SEARCH_QUERY_FIELD_ECClassId "] FROM [RulesEngineTest].[Widget] WHERE [Widget].[ECInstanceId] > 0"

#define TABLE_ALIAS(prefix, ecclass, counter) \
    Utf8PrintfString("%s_%s_%s_%d", prefix, (ecclass).GetSchema().GetAlias().c_str(), (ecclass).GetName().c_str(), counter).c_str()

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct ExpectedQueries
{
    DECLARE_SCHEMA_REGISTRY(ExpectedQueries)

private:
    bmap<Utf8String, NavigationQueryCPtr> m_navigationQueries;
    bmap<Utf8String, ContentQueryCPtr> m_contentQueries;
    ECDbTestProject m_project;
    ECSchemaHelper* m_schemaHelper;
    bvector<PropertyGroupCP> m_propertyGroupsToDelete;
    DefaultCategorySupplier m_categorySupplier;
    TestConnectionManager m_connections;
    IConnectionPtr m_connection;

private:
    ExpectedQueries(BeTest::Host& host)
        {
        PrepareSchemaContext(); 
        RegisterExpectedQueries();
        }
    ~ExpectedQueries();
    void PrepareSchemaContext();
    void RegisterExpectedQueries();
    void RegisterForDelete(PropertyGroupCR spec) {m_propertyGroupsToDelete.push_back(&spec);}
    
public:
    static ExpectedQueries& GetInstance(BeTest::Host&);

    void RegisterQuery(Utf8CP name, NavigationQuery const&);
    void RegisterQuery(Utf8CP name, ContentQuery const&);
    NavigationQueryCPtr GetNavigationQuery(Utf8CP name, ChildNodeSpecificationCR spec) const;
    ContentQueryCPtr GetContentQuery(Utf8CP name) const;
    bmap<Utf8String, NavigationQueryCPtr> const& GetNavigationQueries() const;
    bmap<Utf8String, ContentQueryCPtr> const& GetContentQueries() const;
    ContentDescriptorPtr GetEmptyContentDescriptor(Utf8CP displayType) const;

    ECClassP GetECClassP(Utf8CP schemaName, Utf8CP className);
    ECClassCP GetECClass(Utf8CP schemaName, Utf8CP className);
    ECSchemaCP GetECSchema(Utf8CP schemaName);
    bvector<ECClassCP> GetECClasses(Utf8CP schemaName);
    ECDbR GetDb() {return m_project.GetECDb();}
    IConnectionCR GetConnection() {return *m_connection;}
    IConnectionManagerCR GetConnections() {return m_connections;}
    };

END_ECPRESENTATIONTESTS_NAMESPACE
