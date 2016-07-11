/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbSymbolProviderTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ECObjects/ECExpressions.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                      Grigas.Petraitis              02/2016
//+===============+===============+===============+===============+===============+======
struct ECDbSymbolProviderTests : ECDbTestFixture
    {
    virtual void SetUp() override
        {
        SetupECDb("ECDbExpressionSymbolProviderTests.ecdb");
        }
    };

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSymbolProviderTests, Path)
    {
    Utf8CP schemaXml = ""
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test2\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "    <ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.13\" prefix=\"bcs\" />"
        "    <ECEntityClass typeName=\"TestClass\" displayLabel=\"Class With Calculated Property\">"
        "        <ECProperty propertyName=\"label\" typeName=\"string\">"
        "            <ECCustomAttributes>"
        "                <CalculatedECPropertySpecification xmlns=\"Bentley_Standard_CustomAttributes.01.00\">"
        "                    <ECExpression>ECDb.Path</ECExpression>"
        "                    <FailureValue>Unknown</FailureValue>"
        "                </CalculatedECPropertySpecification>"
        "            </ECCustomAttributes>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(GetECDb().GetSchemaLocater());
    ECSchema::ReadFromXmlString(schema, schemaXml, *ctx);
    GetECDb().Schemas().ImportECSchemas(ctx->GetCache());

    ECClassCP testClass = GetECDb().Schemas().GetECClass("TestSchema", "TestClass");
    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ECValue value;
    instance->GetValue(value, "label");
    EXPECT_TRUE(value.IsString());
    EXPECT_STREQ(GetECDb().GetDbFileName(), value.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSymbolProviderTests, Name)
    {
    Utf8CP schemaXml = ""
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test2\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "    <ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.13\" prefix=\"bcs\" />"
        "    <ECEntityClass typeName=\"TestClass\" displayLabel=\"Class With Calculated Property\">"
        "        <ECProperty propertyName=\"label\" typeName=\"string\">"
        "            <ECCustomAttributes>"
        "                <CalculatedECPropertySpecification xmlns=\"Bentley_Standard_CustomAttributes.01.00\">"
        "                    <ECExpression>ECDb.Name</ECExpression>"
        "                    <FailureValue>Unknown</FailureValue>"
        "                </CalculatedECPropertySpecification>"
        "            </ECCustomAttributes>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(GetECDb().GetSchemaLocater());
    ECSchema::ReadFromXmlString(schema, schemaXml, *ctx);
    GetECDb().Schemas().ImportECSchemas(ctx->GetCache());

    ECClassCP testClass = GetECDb().Schemas().GetECClass("TestSchema", "TestClass");
    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();

    BeFileName fullPath(GetECDb().GetDbFileName(), true);

    ECValue value;
    instance->GetValue(value, "label");
    EXPECT_TRUE(value.IsString());
    EXPECT_STREQ(fullPath.GetFileNameWithoutExtension().c_str(), value.GetWCharCP());
    }

//=======================================================================================
// @bsiclass                                      Grigas.Petraitis              07/2016
//+===============+===============+===============+===============+===============+======
struct ECDbExpressionSymbolContextTests : ECDbSymbolProviderTests
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual Utf8String GetTestSchemaXMLString() const
        {
        return
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
            "    <ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.00\" prefix=\"bcs\" />"
            "    <ECEntityClass typeName=\"ClassA\" displayLabel=\"Class A\">"
            "        <ECProperty propertyName=\"label\" typeName=\"string\" />"
            "    </ECEntityClass>"
            "    <ECEntityClass typeName=\"ClassB\" displayLabel=\"Class B\" >"
            "        <ECProperty propertyName=\"label\" typeName=\"string\" />"
            "    </ECEntityClass>"
            "    <ECRelationshipClass typeName=\"Rel\" strength=\"referencing\" strengthDirection=\"forward\" modifier='Sealed'>"
            "        <Source cardinality=\"(0,1)\" roleLabel=\"A has B\" polymorphic=\"True\">"
            "           <Class class=\"ClassA\" />"
            "        </Source>"
            "        <Target cardinality=\"(0,N)\" roleLabel=\"B belongs to A\" polymorphic=\"True\">"
            "           <Class class=\"ClassB\" />"
            "        </Target>"
            "    </ECRelationshipClass>"
            "</ECSchema>";
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void SetUp() override
        {
        ECDbSymbolProviderTests::SetUp();

        ECSchemaPtr schema;
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        ECSchema::ReadFromXmlString(schema, GetTestSchemaXMLString().c_str(), *context);
        GetECDb().Schemas().ImportECSchemas(context->GetCache());
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool EvaluateECExpression(ECValueR result, Utf8StringCR expression, ExpressionContextR context)
        {
        NodePtr node = ECEvaluator::ParseValueExpressionAndCreateTree(expression.c_str());

        ValueResultPtr valueResult;
        if (ExpressionStatus::Success != node->GetValue(valueResult, context))
            return false;

        if (ExpressionStatus::Success != valueResult->GetECValue(result))
            return false;

        return true;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsitest                                       Grigas.Petraitis              07/2016
    //+---------------+---------------+---------------+---------------+---------------+------
    static ExpressionContextPtr CreateContext(IECInstanceCR instance)
        {
        InstanceExpressionContextPtr instanceContext = InstanceExpressionContext::Create(nullptr);
        instanceContext->SetInstance(instance);
        SymbolExpressionContextPtr rootCtx = SymbolExpressionContext::Create(bvector<Utf8String>(), nullptr);
        rootCtx->AddSymbol(*ContextSymbol::CreateContextSymbol("this", *instanceContext));
        return rootCtx;
        }
    };

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, HasRelatedInstance_ReturnsFalseWhenDoesntHaveAnyRelatedInstances)
    {
    ECClassCP classA = GetECDb().Schemas().GetECClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA);
    
    ECDbExpressionSymbolContext ecdbContext(GetECDb());
    ExpressionContextPtr exprContext = CreateContext(*instanceA);
    
    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.HasRelatedInstance(\"Rel:0:ClassB\")", *exprContext));
    ASSERT_TRUE(value.IsBoolean());
    ASSERT_FALSE(value.GetBoolean());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, HasRelatedInstance_ReturnsTrueWhenHasOneRelatedInstance)
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
    stmt.BindId(2, classA->GetId());
    stmt.BindText(3, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(4, classB->GetId());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ECDbExpressionSymbolContext ecdbContext(GetECDb());
    ExpressionContextPtr exprContext = CreateContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.HasRelatedInstance(\"Rel:0:ClassB\")", *exprContext));
    ASSERT_TRUE(value.IsBoolean());
    ASSERT_TRUE(value.GetBoolean());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, HasRelatedInstance_ReturnsTrueWhenHasMultipleRelatedInstances)
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
    stmt.BindId(2, classA->GetId());
    stmt.BindText(3, instanceB1->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(4, classB->GetId());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Reset();
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(2, classA->GetId());
    stmt.BindText(3, instanceB2->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(4, classB->GetId());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ECDbExpressionSymbolContext ecdbContext(GetECDb());
    ExpressionContextPtr exprContext = CreateContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.HasRelatedInstance(\"Rel:0:ClassB\")", *exprContext));
    ASSERT_TRUE(value.IsBoolean());
    ASSERT_TRUE(value.GetBoolean());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedInstance_FollowsForwardRelationshipWithSingleInstance)
    {
    ECClassCP classA = GetECDb().Schemas().GetECClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA);

    ECClassCP classB = GetECDb().Schemas().GetECClass("TestSchema", "ClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceB->SetValue("label", ECValue("B"));
    ECInstanceInserter(GetECDb(), *classB).Insert(*instanceB);

    ECSqlStatement stmt;
    stmt.Prepare(GetECDb(), "INSERT INTO [test].[Rel] (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)");
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(2, classA->GetId());
    stmt.BindText(3, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(4, classB->GetId());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    
    ECDbExpressionSymbolContext ecdbContext(GetECDb());
    ExpressionContextPtr exprContext = CreateContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedInstance(\"Rel:0:ClassB\").label", *exprContext));
    ASSERT_TRUE(value.IsString());
    ASSERT_STREQ("B", value.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedInstance_FollowsBackwardRelationshipWithSingleInstance)
    {
    ECClassCP classA = GetECDb().Schemas().GetECClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceA->SetValue("label", ECValue("A"));
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA);

    ECClassCP classB = GetECDb().Schemas().GetECClass("TestSchema", "ClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classB).Insert(*instanceB);

    ECSqlStatement stmt;
    stmt.Prepare(GetECDb(), "INSERT INTO [test].[Rel] (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)");
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(2, classA->GetId());
    stmt.BindText(3, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(4, classB->GetId());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    
    ECDbExpressionSymbolContext ecdbContext(GetECDb());
    ExpressionContextPtr exprContext = CreateContext(*instanceB);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedInstance(\"Rel:1:ClassA\").label", *exprContext));
    ASSERT_TRUE(value.IsString());
    ASSERT_STREQ("A", value.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedInstance_FollowsForwardRelationshipWithMultipleInstances)
    {
    ECClassCP classA = GetECDb().Schemas().GetECClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA);

    ECClassCP classB = GetECDb().Schemas().GetECClass("TestSchema", "ClassB");
    IECInstancePtr instanceB1 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceB1->SetValue("label", ECValue("B1"));
    ECInstanceInserter(GetECDb(), *classB).Insert(*instanceB1);
    IECInstancePtr instanceB2 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceB2->SetValue("label", ECValue("B2"));
    ECInstanceInserter(GetECDb(), *classB).Insert(*instanceB2);

    ECSqlStatement stmt;
    stmt.Prepare(GetECDb(), "INSERT INTO [test].[Rel] (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)");
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(2, classA->GetId());
    stmt.BindText(3, instanceB2->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(4, classB->GetId());
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    
    ECDbExpressionSymbolContext ecdbContext(GetECDb());
    ExpressionContextPtr exprContext = CreateContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedInstance(\"Rel:0:ClassB\").label", *exprContext));
    ASSERT_TRUE(value.IsString());
    ASSERT_STREQ("B2", value.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedInstance_FollowsBackwardRelationshipWithMultipleInstances)
    {
    ECClassCP classA = GetECDb().Schemas().GetECClass("TestSchema", "ClassA");
    IECInstancePtr instanceA1 = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceA1->SetValue("label", ECValue("A1"));
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA1);
    IECInstancePtr instanceA2 = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceA2->SetValue("label", ECValue("A2"));
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA2);

    ECClassCP classB = GetECDb().Schemas().GetECClass("TestSchema", "ClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classB).Insert(*instanceB);

    ECSqlStatement stmt;
    stmt.Prepare(GetECDb(), "INSERT INTO [test].[Rel] (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)");
    stmt.BindText(1, instanceA2->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(2, classA->GetId());
    stmt.BindText(3, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(4, classB->GetId());
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    
    ECDbExpressionSymbolContext ecdbContext(GetECDb());
    ExpressionContextPtr exprContext = CreateContext(*instanceB);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedInstance(\"Rel:1:ClassA\").label", *exprContext));
    ASSERT_TRUE(value.IsString());
    ASSERT_STREQ("A2", value.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedInstance_FollowsRelationshipWhenRelationshipAndRelatedClassAreInDifferentSchema)
    {
    Utf8CP differentSchemaXml = ""
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"DifferentSchema\" nameSpacePrefix=\"test2\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "    <ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.00\" prefix=\"bcs\" />"
        "    <ECSchemaReference name=\"TestSchema\" version=\"01.00\" prefix=\"test\" />"
        "    <ECEntityClass typeName=\"DifferentClassB\" displayLabel=\"Different Class B\">"
        "        <ECProperty propertyName=\"label\" typeName=\"string\" />"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName=\"DifferentRelationship\" strength=\"referencing\" strengthDirection=\"forward\" modifier='Sealed'>"
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
    instanceA->SetValue("label", ECValue("A"));
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA);

    ECClassCP classB = GetECDb().Schemas().GetECClass("DifferentSchema", "DifferentClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classB).Insert(*instanceB);

    ECSqlStatement stmt;
    stmt.Prepare(GetECDb(), "INSERT INTO [test2].[DifferentRelationship] (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)");
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(2, classA->GetId());
    stmt.BindText(3, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(4, classB->GetId());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    
    ECDbExpressionSymbolContext ecdbContext(GetECDb());
    ExpressionContextPtr exprContext = CreateContext(*instanceB);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedInstance(\"DifferentRelationship:1:ClassA\").label", *exprContext));
    ASSERT_TRUE(value.IsString());
    ASSERT_STREQ("A", value.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, SymbolsAreInjectedWhenImportingSchemas)
    {
    Utf8CP schemaXml = ""
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"DifferentSchema\" nameSpacePrefix=\"test2\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "    <ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.00\" prefix=\"bcs\" />"
        "    <ECSchemaReference name=\"TestSchema\" version=\"01.00\" prefix=\"test\" />"
        "    <ECEntityClass typeName=\"ClassC\" displayLabel=\"Class With Calculated Property\">"
        "        <ECProperty propertyName=\"label\" typeName=\"string\">"
        "            <ECCustomAttributes>"
        "                <CalculatedECPropertySpecification xmlns=\"Bentley_Standard_CustomAttributes.01.00\">"
        "                    <ECExpression>this.GetRelatedInstance(\"CHasA:0:ClassA\").label</ECExpression>"
        "                    <FailureValue>Unknown</FailureValue>"
        "                </CalculatedECPropertySpecification>"
        "            </ECCustomAttributes>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName=\"CHasA\" strength=\"referencing\" strengthDirection=\"forward\" modifier='Sealed'>"
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
    stmt.BindId(2, classC->GetId());
    stmt.BindText(3, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(4, classA->GetId());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ECValue v;
    instanceC->GetValue(v, "label");
    EXPECT_STREQ("ClassA Label", v.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              02/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, SymbolsAreInjectedWhenDeserializingSchemas)
    {
    Utf8CP schemaXml = ""
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"DifferentSchema\" nameSpacePrefix=\"test2\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "    <ECSchemaReference name=\"Bentley_Standard_CustomAttributes\" version=\"01.00\" prefix=\"bcs\" />"
        "    <ECSchemaReference name=\"TestSchema\" version=\"01.00\" prefix=\"test\" />"
        "    <ECEntityClass typeName=\"ClassC\" displayLabel=\"Class With Calculated Property\">"
        "        <ECProperty propertyName=\"label\" typeName=\"string\">"
        "            <ECCustomAttributes>"
        "                <CalculatedECPropertySpecification xmlns=\"Bentley_Standard_CustomAttributes.01.00\">"
        "                    <ECExpression>this.GetRelatedInstance(\"CHasA:0:ClassA\").label</ECExpression>"
        "                    <FailureValue>Unknown</FailureValue>"
        "                </CalculatedECPropertySpecification>"
        "            </ECCustomAttributes>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName=\"CHasA\" strength=\"referencing\" strengthDirection=\"forward\" modifier='Sealed'>"
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
    insertStmt.BindId(2, classC->GetId());
    insertStmt.BindText(3, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    insertStmt.BindId(4, classA->GetId());
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    ASSERT_EQ(BE_SQLITE_OK, GetECDb().SaveChanges());

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

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedValue_ReturnsNullWhenThereAreNoRelatedInstances)
    {
    ECClassCP classA = GetECDb().Schemas().GetECClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA);
    
    ECDbExpressionSymbolContext ecdbContext(GetECDb());
    ExpressionContextPtr exprContext = CreateContext(*instanceA);
    
    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedValue(\"Rel:0:ClassB\", \"label\")", *exprContext));
    ASSERT_TRUE(value.IsNull());
    }

//---------------------------------------------------------------------------------------
// @bsitest                                       Grigas.Petraitis              07/2016
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedValue_ReturnsRelatedInstanceValue)
    {
    ECClassCP classA = GetECDb().Schemas().GetECClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(GetECDb(), *classA).Insert(*instanceA);

    ECClassCP classB = GetECDb().Schemas().GetECClass("TestSchema", "ClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceB->SetValue("label", ECValue("test label"));
    ECInstanceInserter(GetECDb(), *classB).Insert(*instanceB);
    
    ECSqlStatement stmt;
    stmt.Prepare(GetECDb(), "INSERT INTO [test].[Rel] (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)");
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(2, classA->GetId());
    stmt.BindText(3, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindId(4, classB->GetId());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ECDbExpressionSymbolContext ecdbContext(GetECDb());
    ExpressionContextPtr exprContext = CreateContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedValue(\"Rel:0:ClassB\", \"label\")", *exprContext));
    ASSERT_TRUE(value.IsString());
    ASSERT_STREQ("test label", value.GetUtf8CP());
    }

END_ECDBUNITTESTS_NAMESPACE

