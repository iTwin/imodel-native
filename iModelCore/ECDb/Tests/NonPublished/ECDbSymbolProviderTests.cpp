/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ECObjects/ECExpressions.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECDbExpressionSymbolContextTests : ECDbTestFixture
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual Utf8String GetTestSchemaXMLString() const
        {
        return
            R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" alias="test" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="ClassA" displayLabel="Class A">
                    <ECProperty propertyName="label" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="ClassB" displayLabel="Class B" >
                    <ECProperty propertyName="label" typeName="string" />
                    <ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" strength="referencing" strengthDirection="forward" modifier="Sealed">
                    <Source multiplicity="(0..1)" roleLabel="A has B" polymorphic="True">
                       <Class class="ClassA" />
                    </Source>
                    <Target multiplicity="(0..*)" roleLabel="B belongs to A" polymorphic="True">
                       <Class class="ClassB" />
                    </Target>
                </ECRelationshipClass>
                <ECRelationshipClass typeName="Rel2"  strength="referencing" strengthDirection="forward" modifier="None">
                    <Source multiplicity="(0..1)" roleLabel="A has B" polymorphic="False">
                        <Class class="ClassA" />
                    </Source>
                    <Target multiplicity="(0..1)" roleLabel="B belongs to A" polymorphic="True">
                        <Class class="ClassB" />
                    </Target>
                    <ECProperty propertyName="Priority" typeName="int" />
                </ECRelationshipClass>
            </ECSchema>)xml";
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetUp() override
        {
        SetupECDb("ECDbExpressionSymbolContextTests.ecdb");

        ECSchemaPtr schema;
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        ECSchema::ReadFromXmlString(schema, GetTestSchemaXMLString().c_str(), *context);
        m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas());
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
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
    // @bsitest
    //+---------------+---------------+---------------+---------------+---------------+------
    static SymbolExpressionContextPtr CreateRootContext()
        {
        return SymbolExpressionContext::Create(bvector<Utf8String>(), nullptr);
        }

    //---------------------------------------------------------------------------------------
    // @bsitest
    //+---------------+---------------+---------------+---------------+---------------+------
    static ExpressionContextPtr CreateInstanceContext(IECInstanceCR instance)
        {
        InstanceExpressionContextPtr instanceContext = InstanceExpressionContext::Create(nullptr);
        instanceContext->SetInstance(instance);
        SymbolExpressionContextPtr rootCtx = CreateRootContext();
        rootCtx->AddSymbol(*ContextSymbol::CreateContextSymbol("this", *instanceContext));
        return rootCtx;
        }
    };

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, Path)
    {
    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "ECDb.Path", *CreateRootContext()));
    EXPECT_TRUE(value.IsString());
    EXPECT_STREQ(m_ecdb.GetDbFileName(), value.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, Name)
    {
    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "ECDb.Name", *CreateRootContext()));
    EXPECT_TRUE(value.IsString());
    EXPECT_STREQ(BeFileName(m_ecdb.GetDbFileName(), true).GetFileNameWithoutExtension().c_str(), value.GetWCharCP());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetECClassId)
    {
    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "GetECClassId(\"ClassA\", \"TestSchema\")", *CreateRootContext()));
    ASSERT_TRUE(value.IsLong());
    ASSERT_EQ(testClass->GetId().GetValue(), value.GetLong());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, HasRelatedInstance_3args_ReturnsFalseWhenDoesntHaveAnyRelatedInstances)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);
    
    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);
    
    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.HasRelatedInstance(\"TestSchema:Rel\", \"Forward\", \"TestSchema:ClassB\")", *exprContext));
    ASSERT_TRUE(value.IsBoolean());
    ASSERT_FALSE(value.GetBoolean());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, HasRelatedInstance_3args_ReturnsTrueWhenHasOneRelatedInstance)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECClassCP classB = m_ecdb.Schemas().GetClass("TestSchema", "ClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB);
    
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE TestSchema.ClassB SET A.Id = ? WHERE ECInstanceId=?"));
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.HasRelatedInstance(\"TestSchema:Rel\", \"Forward\", \"TestSchema:ClassB\")", *exprContext));
    ASSERT_TRUE(value.IsBoolean());
    ASSERT_TRUE(value.GetBoolean());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, HasRelatedInstance_3args_ReturnsTrueWhenHasMultipleRelatedInstances)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECClassCP classB = m_ecdb.Schemas().GetClass("TestSchema", "ClassB");
    IECInstancePtr instanceB1 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB1);
    IECInstancePtr instanceB2 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB2);
    
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE TestSchema.ClassB SET A.Id = ? WHERE ECInstanceId=?"));
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB1->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Reset();
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB2->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.HasRelatedInstance(\"TestSchema:Rel\", \"Forward\", \"TestSchema:ClassB\")", *exprContext));
    ASSERT_TRUE(value.IsBoolean());
    ASSERT_TRUE(value.GetBoolean());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, HasRelatedInstance_2args_ReturnsFalseWhenDoesntHaveAnyRelatedInstances)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.HasRelatedInstance(\"TestSchema:ClassB\", b => b.A.Id = this.ECInstanceId)", *exprContext));
    ASSERT_TRUE(value.IsBoolean());
    ASSERT_FALSE(value.GetBoolean());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, HasRelatedInstance_2args_ReturnsTrueWhenHasOneRelatedInstance)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECClassCP classB = m_ecdb.Schemas().GetClass("TestSchema", "ClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE TestSchema.ClassB SET A.Id = ? WHERE ECInstanceId=?"));
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.HasRelatedInstance(\"TestSchema:ClassB\", b => b.A.Id = this.ECInstanceId)", *exprContext));
    ASSERT_TRUE(value.IsBoolean());
    ASSERT_TRUE(value.GetBoolean());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, HasRelatedInstance_2args_ReturnsTrueWhenHasMultipleRelatedInstances)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECClassCP classB = m_ecdb.Schemas().GetClass("TestSchema", "ClassB");
    IECInstancePtr instanceB1 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB1);
    IECInstancePtr instanceB2 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB2);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE TestSchema.ClassB SET A.Id = ? WHERE ECInstanceId=?"));
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB1->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Reset();
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB2->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.HasRelatedInstance(\"TestSchema:ClassB\", b => b.A.Id = this.ECInstanceId)", *exprContext));
    ASSERT_TRUE(value.IsBoolean());
    ASSERT_TRUE(value.GetBoolean());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedInstancesCount_3args_Returns0WhenThereAreNoRelatedInstances)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECClassCP classB = m_ecdb.Schemas().GetClass("TestSchema", "ClassB");
    IECInstancePtr instanceB1 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB1);
    IECInstancePtr instanceB2 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB2);
    

    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedInstancesCount(\"TestSchema:Rel\", \"Forward\", \"TestSchema:ClassB\")", *exprContext));
    ASSERT_TRUE(value.IsLong());
    ASSERT_EQ(0, value.GetLong());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedInstancesCount_3args_Returns2WhenThereAre2RelatedInstances)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECClassCP classB = m_ecdb.Schemas().GetClass("TestSchema", "ClassB");
    IECInstancePtr instanceB1 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB1);
    IECInstancePtr instanceB2 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB2);
    
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE TestSchema.ClassB SET A.Id = ? WHERE ECInstanceId=?"));

    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB1->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Reset();
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB2->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedInstancesCount(\"TestSchema:Rel\", \"Forward\", \"TestSchema:ClassB\")", *exprContext));
    ASSERT_TRUE(value.IsLong());
    ASSERT_EQ(2, value.GetLong());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedInstancesCount_2args_Returns0WhenThereAreNoRelatedInstances)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECClassCP classB = m_ecdb.Schemas().GetClass("TestSchema", "ClassB");
    IECInstancePtr instanceB1 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB1);
    IECInstancePtr instanceB2 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB2);


    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedInstancesCount(\"TestSchema:ClassB\", b => b.A.Id = this.ECInstanceId)", *exprContext));
    ASSERT_TRUE(value.IsLong());
    ASSERT_EQ(0, value.GetLong());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedInstancesCount_2args_Returns2WhenThereAre2RelatedInstances)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECClassCP classB = m_ecdb.Schemas().GetClass("TestSchema", "ClassB");
    IECInstancePtr instanceB1 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB1);
    IECInstancePtr instanceB2 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB2);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE TestSchema.ClassB SET A.Id = ? WHERE ECInstanceId=?"));

    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB1->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Reset();
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB2->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedInstancesCount(\"TestSchema:ClassB\", b => b.A.Id = this.ECInstanceId)", *exprContext));
    ASSERT_TRUE(value.IsLong());
    ASSERT_EQ(2, value.GetLong());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedInstance_FollowsForwardRelationshipWithSingleInstance)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECClassCP classB = m_ecdb.Schemas().GetClass("TestSchema", "ClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceB->SetValue("label", ECValue("B"));
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE TestSchema.ClassB SET A.Id = ? WHERE ECInstanceId=?"));
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    
    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedInstance(\"Rel:0:ClassB\").label", *exprContext));
    ASSERT_TRUE(value.IsString());
    ASSERT_STREQ("B", value.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedInstance_FollowsBackwardRelationshipWithSingleInstance)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceA->SetValue("label", ECValue("A"));
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECClassCP classB = m_ecdb.Schemas().GetClass("TestSchema", "ClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE TestSchema.ClassB SET A.Id = ? WHERE ECInstanceId=?"));
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    
    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceB);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedInstance(\"Rel:1:ClassA\").label", *exprContext));
    ASSERT_TRUE(value.IsString());
    ASSERT_STREQ("A", value.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedInstance_FollowsForwardRelationshipWithMultipleInstances)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECClassCP classB = m_ecdb.Schemas().GetClass("TestSchema", "ClassB");
    IECInstancePtr instanceB1 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceB1->SetValue("label", ECValue("B1"));
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB1);
    IECInstancePtr instanceB2 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceB2->SetValue("label", ECValue("B2"));
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB2);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE TestSchema.ClassB SET A.Id = ? WHERE ECInstanceId=?"));
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB2->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    
    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedInstance(\"Rel:0:ClassB\").label", *exprContext));
    ASSERT_TRUE(value.IsString());
    ASSERT_STREQ("B2", value.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedInstance_FollowsBackwardRelationshipWithMultipleInstances)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA1 = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceA1->SetValue("label", ECValue("A1"));
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA1);
    IECInstancePtr instanceA2 = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceA2->SetValue("label", ECValue("A2"));
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA2);

    ECClassCP classB = m_ecdb.Schemas().GetClass("TestSchema", "ClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE TestSchema.ClassB SET A.Id = ? WHERE ECInstanceId=?"));
    stmt.BindText(1, instanceA2->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    
    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceB);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedInstance(\"Rel:1:ClassA\").label", *exprContext));
    ASSERT_TRUE(value.IsString());
    ASSERT_STREQ("A2", value.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedInstance_FollowsRelationshipWhenRelationshipAndRelatedClassAreInDifferentSchema)
    {
    Utf8CP differentSchemaXml = ""
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"DifferentSchema\" alias=\"test2\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
        "    <ECSchemaReference name=\"TestSchema\" version=\"01.00\" alias=\"test\" />"
        "    <ECEntityClass typeName=\"DifferentClassB\" displayLabel=\"Different Class B\">"
        "        <ECProperty propertyName=\"label\" typeName=\"string\" />"
        "        <ECNavigationProperty propertyName=\"A\" relationshipName=\"DifferentRelationship\" direction=\"Backward\"/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName=\"DifferentRelationship\" strength=\"referencing\" strengthDirection=\"forward\" modifier='Sealed'>"
        "        <Source multiplicity=\"(0..1)\" roleLabel=\"A has B\" polymorphic=\"True\">"
        "           <Class class=\"test:ClassA\" />"
        "        </Source>"
        "        <Target multiplicity=\"(0..*)\" roleLabel=\"B belongs to A\" polymorphic=\"True\">"
        "           <Class class=\"DifferentClassB\" />"
        "        </Target>"
        "    </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, differentSchemaXml, *ctx);
    ASSERT_TRUE(schema.IsValid());
    m_ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas());

    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceA->SetValue("label", ECValue("A"));
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECClassCP classB = m_ecdb.Schemas().GetClass("DifferentSchema", "DifferentClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE DifferentSchema.DifferentClassB SET A.Id = ? WHERE ECInstanceId=?"));
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    
    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceB);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedInstance(\"DifferentRelationship:1:ClassA\").label", *exprContext));
    ASSERT_TRUE(value.IsString());
    ASSERT_STREQ("A", value.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedValue_4args_ReturnsNullWhenThereAreNoRelatedInstances)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);
    
    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);
    
    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedValue(\"TestSchema:Rel\", \"Forward\", \"TestSchema:ClassB\", \"label\")", *exprContext));
    ASSERT_TRUE(value.IsNull());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedValue_4args_ReturnsRelatedInstanceValue_WithNavigationPropertyRelationship)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECClassCP classB = m_ecdb.Schemas().GetClass("TestSchema", "ClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceB->SetValue("label", ECValue("test label"));
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB);
    
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE TestSchema.ClassB SET A.Id = ? WHERE ECInstanceId=?"));
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedValue(\"TestSchema:Rel\", \"Forward\", \"TestSchema:ClassB\", \"label\")", *exprContext));
    ASSERT_TRUE(value.IsString());
    ASSERT_STREQ("test label", value.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedValue_4args_ReturnsRelatedInstanceValue_WithLinkTableRelationship)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECClassCP classB = m_ecdb.Schemas().GetClass("TestSchema", "ClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceB->SetValue("label", ECValue("test label"));
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB);
    
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO TestSchema.Rel2 (SourceECInstanceId, TargetECInstanceId) VALUES (?, ?)"));
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    
    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedValue(\"TestSchema:Rel2\", \"Forward\", \"TestSchema:ClassB\", \"label\")", *exprContext));
    ASSERT_TRUE(value.IsString());
    ASSERT_STREQ("test label", value.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedValue_3args_ReturnsNullWhenThereAreNoRelatedInstances)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedValue(\"TestSchema:ClassB\", b => b.A.Id = this.ECInstanceId, \"label\")", *exprContext));
    ASSERT_TRUE(value.IsNull());
    }

//---------------------------------------------------------------------------------------
// @bsitest
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbExpressionSymbolContextTests, GetRelatedValue_3args_ReturnsRelatedInstanceValue)
    {
    ECClassCP classA = m_ecdb.Schemas().GetClass("TestSchema", "ClassA");
    IECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECInstanceInserter(m_ecdb, *classA, nullptr).Insert(*instanceA);

    ECClassCP classB = m_ecdb.Schemas().GetClass("TestSchema", "ClassB");
    IECInstancePtr instanceB = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceB->SetValue("label", ECValue("test label"));
    ECInstanceInserter(m_ecdb, *classB, nullptr).Insert(*instanceB);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE TestSchema.ClassB SET A.Id = ? WHERE ECInstanceId=?"));
    stmt.BindText(1, instanceA->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    stmt.BindText(2, instanceB->GetInstanceId().c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ECDbExpressionSymbolContext ecdbContext(m_ecdb);
    ExpressionContextPtr exprContext = CreateInstanceContext(*instanceA);

    ECValue value;
    ASSERT_TRUE(EvaluateECExpression(value, "this.GetRelatedValue(\"TestSchema:ClassB\", b => b.A.Id = this.ECInstanceId, \"label\")", *exprContext));
    ASSERT_TRUE(value.IsString());
    ASSERT_STREQ("test label", value.GetUtf8CP());
    }

END_ECDBUNITTESTS_NAMESPACE