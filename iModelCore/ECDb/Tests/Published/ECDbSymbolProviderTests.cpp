/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbSymbolProviderTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                      Grigas.Petraitis              02/2016
//+===============+===============+===============+===============+===============+======
struct ECDbExpressionSymbolProviderTests : ECDbTestFixture
    {
    bvector<Utf8String> m_reqestedSymbols;

    virtual void SetUp() override
        {
        SetupECDb("ECDbExpressionSymbolProviderTests.ecdb");
        }

    virtual void _PublishSymbols(SymbolExpressionContextR context)
        {
        //ECDbExpressionSymbolProvider provider(GetECDb());
        //provider.PublishSymbols(context, m_reqestedSymbols);
        }

    ExpressionStatus EvaluateExpression(EvaluationResult& result, Utf8CP expr)
        {
        SymbolExpressionContextPtr context = SymbolExpressionContext::Create(nullptr);
        _PublishSymbols(*context);

        NodePtr tree = ECEvaluator::ParseValueExpressionAndCreateTree (expr);
        EXPECT_TRUE(tree.IsValid());
        return tree->GetValue (result, *context);
        }
    };

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolProviderTests, Path)
    {
    EvaluationResult result;
    ASSERT_EQ(ExpressionStatus::Success, EvaluateExpression(result, "ECDb.Path"));
    ASSERT_TRUE(result.IsECValue());
    ASSERT_STREQ(GetECDb().GetDbFileName(), result.GetECValue()->GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolProviderTests, Name)
    {
    BeFileName fullPath(GetECDb().GetDbFileName(), true);
    EvaluationResult result;
    ASSERT_EQ(ExpressionStatus::Success, EvaluateExpression(result, "ECDb.Name"));
    ASSERT_TRUE(result.IsECValue());
    ASSERT_STREQ(fullPath.GetFileNameWithoutExtension().c_str(), result.GetECValue()->GetWCharCP());
    }

//=======================================================================================
// @bsiclass                                      Grigas.Petraitis              02/2016
//+===============+===============+===============+===============+===============+======
struct ECDbExpressionSymbolProviderRelatedInstanceTests : ECDbExpressionSymbolProviderTests
    {
    Utf8String GetTestSchemaXMLString() const
        {
        return
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
            "    <ECEntityClass typeName=\"ClassA\" displayLabel=\"Class A\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"label\" typeName=\"string\" />"
            "    </ECEntityClass>"
            "    <ECEntityClass typeName=\"ClassB\" displayLabel=\"Class B\" isDomainClass=\"True\">"
            "        <ECProperty propertyName=\"label\" typeName=\"string\" />"
            "    </ECEntityClass>"
            "    <ECRelationshipClass typeName=\"Rel\" isDomainClass=\"True\" strength=\"referencing\" strengthDirection=\"forward\">"
            "        <Source cardinality=\"(0,1)\" roleLabel=\"A has B\" polymorphic=\"True\">"
            "           <Class class=\"ClassA\" />"
            "        </Source>"
            "        <Target cardinality=\"(0,N)\" roleLabel=\"B belongs to A\" polymorphic=\"True\">"
            "           <Class class=\"ClassB\" />"
            "        </Target>"
            "    </ECRelationshipClass>"
            "</ECSchema>";
        }

    virtual void SetUp() override
        {
        ECDbExpressionSymbolProviderTests::SetUp();

        ECSchemaPtr schema;
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        ECSchema::ReadFromXmlString(schema, GetTestSchemaXMLString().c_str(), *context);
        GetECDb().Schemas().ImportECSchemas(context->GetCache());
        }

    virtual void _PublishSymbols(SymbolExpressionContextR context) override
        {
        //ECDbExpressionSymbolProvider provider(GetECDb());
        //provider.PublishSymbols(context, m_reqestedSymbols);
        }

    ExpressionStatus EvaluateExpression(EvaluationResult& result, Utf8CP expr, InstanceListExpressionContextR instanceContext)
        {
        SymbolExpressionContextPtr context = SymbolExpressionContext::Create(nullptr);
        ContextSymbolPtr instanceSymbol = ContextSymbol::CreateContextSymbol("this", instanceContext);
        context->AddSymbol(*instanceSymbol);
        _PublishSymbols(*context);
        NodePtr tree = ECEvaluator::ParseValueExpressionAndCreateTree(expr);
        EXPECT_TRUE(tree.IsValid());
        return tree->GetValue(result, *context);
        }

    ExpressionStatus EvaluateExpression(EvaluationResult& result, Utf8CP expr, IECInstanceR instance)
        {
        InstanceExpressionContextPtr context = InstanceExpressionContext::Create(nullptr);
        context->SetInstance(instance);
        return EvaluateExpression(result, expr, *context);
        }

    ExpressionStatus EvaluateExpression(EvaluationResult& result, Utf8CP expr, ECInstanceListCR instances)
        {
        InstanceListExpressionContextPtr context = InstanceListExpressionContext::Create(instances, nullptr);
        return EvaluateExpression(result, expr, *context);
        }
    };

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolProviderRelatedInstanceTests, FollowsForwardRelationshipWithSingleInstance)
    {
    ECClassCP classA = GetECDb().Schemas().GetECClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA);

    ECClassCP classB = GetECDb().Schemas().GetECClass("TestSchema", "ClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classB).Insert(*instanceB);

    ECSqlStatement stmt;
    stmt.Prepare(GetECDb(), "INSERT INTO [test].[Rel] (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)");
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt64(2, classA->GetId());
    stmt.BindText(3, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt64(4, classB->GetId());
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());

    EvaluationResult result;
    ASSERT_EQ(ExpressionStatus::Success, EvaluateExpression(result, "this.GetRelatedInstance(\"Rel:0:ClassB\")", *instanceA));
    ASSERT_TRUE(result.IsInstanceList());
    ASSERT_EQ(1, result.GetInstanceList()->size());
    ASSERT_STREQ(instanceB->GetInstanceId().c_str(), (*result.GetInstanceList())[0]->GetInstanceId().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolProviderRelatedInstanceTests, FollowsBackwardRelationshipWithSingleInstance)
    {
    ECClassCP classA = GetECDb().Schemas().GetECClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA);

    ECClassCP classB = GetECDb().Schemas().GetECClass("TestSchema", "ClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classB).Insert(*instanceB);

    ECSqlStatement stmt;
    stmt.Prepare(GetECDb(), "INSERT INTO [test].[Rel] (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)");
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt64(2, classA->GetId());
    stmt.BindText(3, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt64(4, classB->GetId());
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());

    EvaluationResult result;
    ASSERT_EQ(ExpressionStatus::Success, EvaluateExpression(result, "this.GetRelatedInstance(\"Rel:1:ClassA\")", *instanceB));
    ASSERT_TRUE(result.IsInstanceList());
    ASSERT_EQ(1, result.GetInstanceList()->size());
    ASSERT_STREQ(instanceA->GetInstanceId().c_str(), (*result.GetInstanceList())[0]->GetInstanceId().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolProviderRelatedInstanceTests, FollowsForwardRelationshipWithMultipleInstances)
    {
    ECClassCP classA = GetECDb().Schemas().GetECClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA);

    ECClassCP classB = GetECDb().Schemas().GetECClass("TestSchema", "ClassB");
    IECInstancePtr instanceB1 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classB).Insert(*instanceB1);
    IECInstancePtr instanceB2 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classB).Insert(*instanceB2);

    ECSqlStatement stmt;
    stmt.Prepare(GetECDb(), "INSERT INTO [test].[Rel] (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)");
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt64(2, classA->GetId());
    stmt.BindText(3, instanceB2->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt64(4, classB->GetId());
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());

    EvaluationResult result;
    ASSERT_EQ(ExpressionStatus::Success, EvaluateExpression(result, "this.GetRelatedInstance(\"Rel:0:ClassB\")", *instanceA));
    ASSERT_TRUE(result.IsInstanceList());
    ASSERT_EQ(1, result.GetInstanceList()->size());
    ASSERT_STREQ(instanceB2->GetInstanceId().c_str(), (*result.GetInstanceList())[0]->GetInstanceId().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolProviderRelatedInstanceTests, FollowsBackwardRelationshipWithMultipleInstances)
    {
    ECClassCP classA = GetECDb().Schemas().GetECClass("TestSchema", "ClassA");
    IECInstancePtr instanceA1 = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA1);
    IECInstancePtr instanceA2 = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA2);

    ECClassCP classB = GetECDb().Schemas().GetECClass("TestSchema", "ClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classB).Insert(*instanceB);

    ECSqlStatement stmt;
    stmt.Prepare(GetECDb(), "INSERT INTO [test].[Rel] (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)");
    stmt.BindText(1, instanceA2->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt64(2, classA->GetId());
    stmt.BindText(3, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt64(4, classB->GetId());
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());

    EvaluationResult result;
    ASSERT_EQ(ExpressionStatus::Success, EvaluateExpression(result, "this.GetRelatedInstance(\"Rel:1:ClassA\")", *instanceB));
    ASSERT_TRUE(result.IsInstanceList());
    ASSERT_EQ(1, result.GetInstanceList()->size());
    ASSERT_STREQ(instanceA2->GetInstanceId().c_str(), (*result.GetInstanceList())[0]->GetInstanceId().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolProviderRelatedInstanceTests, FollowsRelationshipFromDifferentSchema)
    {
    Utf8CP differentSchemaXml = ""
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"DifferentSchema\" nameSpacePrefix=\"test2\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "    <ECSchemaReference name=\"TestSchema\" version=\"01.00\" prefix=\"test\" />"
        "    <ECRelationshipClass typeName=\"DifferentRelationship\" isDomainClass=\"True\" strength=\"referencing\" strengthDirection=\"forward\">"
        "        <Source cardinality=\"(0,1)\" roleLabel=\"A has B\" polymorphic=\"True\">"
        "           <Class class=\"test:ClassA\" />"
        "        </Source>"
        "        <Target cardinality=\"(0,N)\" roleLabel=\"B belongs to A\" polymorphic=\"True\">"
        "           <Class class=\"test:ClassB\" />"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(GetECDb().GetSchemaLocater());
    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, differentSchemaXml, *ctx);
    ASSERT_TRUE(schema.IsValid());
    GetECDb().Schemas().ImportECSchemas(ctx->GetCache());

    ECClassCP classA = GetECDb().Schemas().GetECClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA);

    ECClassCP classB = GetECDb().Schemas().GetECClass("TestSchema", "ClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classB).Insert(*instanceB);

    ECSqlStatement stmt;
    stmt.Prepare(GetECDb(), "INSERT INTO [test2].[DifferentRelationship] (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)");
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt64(2, classA->GetId());
    stmt.BindText(3, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt64(4, classB->GetId());
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());

    EvaluationResult result;
    ASSERT_EQ(ExpressionStatus::Success, EvaluateExpression(result, "this.GetRelatedInstance(\"DifferentRelationship:0:ClassB\")", *instanceA));
    ASSERT_TRUE(result.IsInstanceList());
    ASSERT_EQ(1, result.GetInstanceList()->size());
    ASSERT_STREQ(instanceB->GetInstanceId().c_str(), (*result.GetInstanceList())[0]->GetInstanceId().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolProviderRelatedInstanceTests, FollowsRelationshipWhenRelationshipAndRelatedClassAreInDifferentSchema)
    {
    Utf8CP differentSchemaXml = ""
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"DifferentSchema\" nameSpacePrefix=\"test2\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "    <ECSchemaReference name=\"TestSchema\" version=\"01.00\" prefix=\"test\" />"
        "    <ECEntityClass typeName=\"DifferentClassB\" displayLabel=\"Different Class B\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"label\" typeName=\"string\" />"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName=\"DifferentRelationship\" isDomainClass=\"True\" strength=\"referencing\" strengthDirection=\"forward\">"
        "        <Source cardinality=\"(0,1)\" roleLabel=\"A has B\" polymorphic=\"True\">"
        "           <Class class=\"test:ClassA\" />"
        "        </Source>"
        "        <Target cardinality=\"(0,N)\" roleLabel=\"B belongs to A\" polymorphic=\"True\">"
        "           <Class class=\"DifferentClassB\" />"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(GetECDb().GetSchemaLocater());
    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, differentSchemaXml, *ctx);
    ASSERT_TRUE(schema.IsValid());
    GetECDb().Schemas().ImportECSchemas(ctx->GetCache());

    ECClassCP classA = GetECDb().Schemas().GetECClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA);

    ECClassCP classB = GetECDb().Schemas().GetECClass("DifferentSchema", "DifferentClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classB).Insert(*instanceB);

    ECSqlStatement stmt;
    stmt.Prepare(GetECDb(), "INSERT INTO [test2].[DifferentRelationship] (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)");
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt64(2, classA->GetId());
    stmt.BindText(3, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt64(4, classB->GetId());
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());

    EvaluationResult result;
    ASSERT_EQ(ExpressionStatus::Success, EvaluateExpression(result, "this.GetRelatedInstance(\"DifferentRelationship:0:DifferentClassB\")", *instanceA));
    ASSERT_TRUE(result.IsInstanceList());
    ASSERT_EQ(1, result.GetInstanceList()->size());
    ASSERT_STREQ(instanceB->GetInstanceId().c_str(), (*result.GetInstanceList())[0]->GetInstanceId().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolProviderRelatedInstanceTests, SymbolsAreInjectedWhenImportingSchemas)
    {
    Utf8CP schemaXml = ""
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"DifferentSchema\" nameSpacePrefix=\"test2\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "    <ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.00\" prefix=\"bcs\" />"
        "    <ECSchemaReference name=\"TestSchema\" version=\"01.00\" prefix=\"test\" />"
        "    <ECEntityClass typeName=\"ClassC\" displayLabel=\"Class With Calculated Property\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"label\" typeName=\"string\">"
        "            <ECCustomAttributes>"
        "                <CalculatedECPropertySpecification xmlns=\"Bentley_Standard_CustomAttributes.01.00\">"
        "                    <ECExpression>this.GetRelatedInstance(\"CHasA:0:ClassA\").label</ECExpression>"
        "                    <FailureValue>Unknown</FailureValue>"
        "                </CalculatedECPropertySpecification>"
        "            </ECCustomAttributes>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName=\"CHasA\" isDomainClass=\"True\" strength=\"referencing\" strengthDirection=\"forward\">"
        "        <Source cardinality=\"(0,1)\" roleLabel=\"C has A\" polymorphic=\"True\">"
        "           <Class class=\"ClassC\" />"
        "        </Source>"
        "        <Target cardinality=\"(0,N)\" roleLabel=\"A belongs to C\" polymorphic=\"True\">"
        "           <Class class=\"test:ClassA\" />"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr differentSchema;
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(GetECDb().GetSchemaLocater());
    ECSchema::ReadFromXmlString(differentSchema, schemaXml, *ctx);
    GetECDb().Schemas().ImportECSchemas(ctx->GetCache());

    ECClassCP classA = GetECDb().Schemas().GetECClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceA->SetValue("label", ECValue("ClassA Label"));
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA);

    ECClassCP classC = differentSchema->GetClassCP("ClassC");
    IECInstancePtr instanceC = classC->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classC).Insert(*instanceC);

    ECSqlStatement stmt;
    stmt.Prepare(GetECDb(), "INSERT INTO [test2].[CHasA] (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)");
    stmt.BindText(1, instanceC->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt64(2, classC->GetId());
    stmt.BindText(3, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindInt64(4, classA->GetId());
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());

    ECValue v;
    instanceC->GetValue(v, "label");
    EXPECT_STREQ("ClassA Label", v.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolProviderRelatedInstanceTests, SymbolsAreInjectedWhenDeserializingSchemas)
    {
    Utf8CP schemaXml = ""
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"DifferentSchema\" nameSpacePrefix=\"test2\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "    <ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.00\" prefix=\"bcs\" />"
        "    <ECSchemaReference name=\"TestSchema\" version=\"01.00\" prefix=\"test\" />"
        "    <ECEntityClass typeName=\"ClassC\" displayLabel=\"Class With Calculated Property\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"label\" typeName=\"string\">"
        "            <ECCustomAttributes>"
        "                <CalculatedECPropertySpecification xmlns=\"Bentley_Standard_CustomAttributes.01.00\">"
        "                    <ECExpression>this.GetRelatedInstance(\"CHasA:0:ClassA\").label</ECExpression>"
        "                    <FailureValue>Unknown</FailureValue>"
        "                </CalculatedECPropertySpecification>"
        "            </ECCustomAttributes>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName=\"CHasA\" isDomainClass=\"True\" strength=\"referencing\" strengthDirection=\"forward\">"
        "        <Source cardinality=\"(0,1)\" roleLabel=\"C has A\" polymorphic=\"True\">"
        "           <Class class=\"ClassC\" />"
        "        </Source>"
        "        <Target cardinality=\"(0,N)\" roleLabel=\"A belongs to C\" polymorphic=\"True\">"
        "           <Class class=\"test:ClassA\" />"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(GetECDb().GetSchemaLocater());
    ECSchema::ReadFromXmlString(schema, schemaXml, *ctx);
    GetECDb().Schemas().ImportECSchemas(ctx->GetCache());

    ECClassCP classA = GetECDb().Schemas().GetECClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceA->SetValue("label", ECValue("ClassA Label"));
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA);

    ECClassCP classC = GetECDb().Schemas().GetECClass("DifferentSchema", "ClassC");
    IECInstancePtr instanceC = classC->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classC).Insert(*instanceC);

    ECSqlStatement insertStmt;
    insertStmt.Prepare(GetECDb(), "INSERT INTO [test2].[CHasA] (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)");
    insertStmt.BindText(1, instanceC->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    insertStmt.BindInt64(2, classC->GetId());
    insertStmt.BindText(3, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    insertStmt.BindInt64(4, classA->GetId());
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, insertStmt.Step());
    ASSERT_EQ(DbResult::BE_SQLITE_OK, GetECDb().SaveChanges());

    // reopen ECDb
    BeFileName ecdbPath(GetECDb().GetDbFileName(), true);
    GetECDb().CloseDb();
    GetECDb().OpenBeSQLiteDb(ecdbPath, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::Readonly));

    ECSqlStatement selectStmt;
    ASSERT_TRUE(selectStmt.Prepare(GetECDb(), "SELECT * FROM test2.ClassC WHERE ECInstanceId = ?").IsSuccess());
    ASSERT_TRUE(selectStmt.BindText(1, instanceC->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No).IsSuccess());

    ECInstanceECSqlSelectAdapter adapter(selectStmt);
    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_ROW, selectStmt.Step());
    IECInstancePtr selectedInstanceC = adapter.GetInstance();
    ASSERT_TRUE(selectedInstanceC.IsValid());

    ECValue v;
    selectedInstanceC->GetValue(v, "label");
    EXPECT_STREQ("ClassA Label", v.GetUtf8CP());
    }

END_ECDBUNITTESTS_NAMESPACE

