/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/ECDb/ECDbHints_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UnitTests/NonPublished/ECDb/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECDbHintTests : public ::testing::Test
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                   Affan.Khan                         02/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyRelationshipInsertionIntegrity (ECDbCR ecdb, Utf8CP relationshipClass, std::vector<ECInstanceKey> const& sourceKeys, std::vector<ECInstanceKey>const& targetKeys, std::vector<ECSqlStepStatus> const& expected, size_t& rowInserted)
        {
        ECSqlStatement stmt;
        auto sql = SqlPrintfString ("INSERT INTO %s (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(?,?,?,?)", relationshipClass);
        ASSERT_EQ (stmt.Prepare (ecdb, sql.GetUtf8CP ()), ECSqlStatus::Success);
        ASSERT_EQ (expected.size (), sourceKeys.size () * targetKeys.size ());
        int n = 0;
        for (auto& fooKey : sourceKeys)
            {
            for (auto& gooKey : targetKeys)
                {
                stmt.Reset ();
                ASSERT_EQ (static_cast<int>(stmt.ClearBindings ()), static_cast<int>(ECSqlStatus::Success));
                stmt.BindId (1, fooKey.GetECInstanceId ());
                stmt.BindInt64 (2, fooKey.GetECClassId ());
                stmt.BindId (3, gooKey.GetECInstanceId ());
                stmt.BindInt64 (4, gooKey.GetECClassId ());
                ASSERT_EQ (static_cast<int>(stmt.Step ()), static_cast<int>(expected[n]));
                if (expected[n] == ECSqlStepStatus::Done)
                    rowInserted++;

                n = n + 1;
                }
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                   Affan.Khan                         02/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    size_t GetRelationshipInstanceCount (ECDbCR ecdb, Utf8CP relationshipClass)
        {
        ECSqlStatement stmt;
        auto sql = SqlPrintfString ("SELECT COUNT(*) FROM ONLY ts.Foo JOIN ts.Goo USING %s", relationshipClass);
        if (stmt.Prepare (ecdb, sql.GetUtf8CP ()) == ECSqlStatus::Success)
            {
            if (stmt.Step () == ECSqlStepStatus::HasRow)
                return static_cast<size_t>(stmt.GetValueInt (0));
            }

        return 0;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                   Affan.Khan                         02/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    enum  MapStrategy : uint32_t
        {
        MapStrategy_DoNotMap = 0x1,
        MapStrategy_DoNotMapHierarchy = 0x2,
        MapStrategy_TablePerHierarchy = 0x4,
        MapStrategy_TablePerClass = 0x8,
        MapStrategy_SharedTableForThisClass = 0x10,
        MapStrategy_MapToExistingTable = 0x20,
        MapStrategy_TableForThisClass = 40,
        MapStrategy_WithReuseColumns = 0x100,
        MapStrategy_WithExclusivelyStoredInThisTable = 0x200,
        MapStrategy_WithReadonly = 0x400,
        MapStrategy_NoHint = 0x0,
        MapStrategy_InParentTable = 0x1000,
        MapStrategy_RelationshipTargetTable = 0x2000,
        MapStrategy_RelationshipSourceTable = 0x4000,
        };

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                   Affan.Khan                         02/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    uint32_t GetMapStrategy (ECDbR ecdb, ECClassId ecClassId)
        {
        Statement stmt;
        stmt.Prepare (ecdb, "SELECT MapStrategy FROM ec_ClassMap WHERE ECClassId = ?");
        stmt.BindInt64 (1, ecClassId);
        if (stmt.Step () == BE_SQLITE_ROW)
            return stmt.GetValueInt (0);

        return 0;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                   Affan.Khan                         02/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExecuteRelationshipInsertionIntegrityTest (ECDbR ecdb, bool allowDuplicateRelationships, bool allowForeignKeyConstraint)
        {
        ECSchemaPtr testSchema;
        ECClassP foo = nullptr, goo = nullptr;
        ECRelationshipClassP oneFooHasOneGoo = nullptr, oneFooHasManyGoo = nullptr, manyFooHasManyGoo = nullptr;
        PrimitiveECPropertyP prim;
        auto readContext = ECSchemaReadContext::CreateContext ();
        readContext->AddSchemaLocater (ecdb.GetSchemaLocater ());
        auto bscaKey = SchemaKey (L"Bentley_Standard_CustomAttributes", 1, 11);
        auto bscaSchema = readContext->LocateSchema (bscaKey, SchemaMatchType::SCHEMAMATCHTYPE_LatestCompatible);
        ASSERT_TRUE (bscaSchema.IsValid ());

        ECSchema::CreateSchema (testSchema, L"TestSchema", 1, 0);
        ASSERT_TRUE (testSchema.IsValid ());

        testSchema->SetNamespacePrefix (L"ts");
        testSchema->AddReferencedSchema (*bscaSchema);

        testSchema->CreateClass (foo, L"Foo");
        testSchema->CreateClass (goo, L"Goo");

        testSchema->CreateRelationshipClass (oneFooHasOneGoo, L"OneFooHasOneGoo");
        testSchema->CreateRelationshipClass (oneFooHasManyGoo, L"OneFooHasManyGoo");
        testSchema->CreateRelationshipClass (manyFooHasManyGoo, L"ManyFooHasManyGoo");

        ASSERT_TRUE (foo != nullptr);
        ASSERT_TRUE (foo != nullptr);
        ASSERT_TRUE (oneFooHasOneGoo != nullptr);
        ASSERT_TRUE (oneFooHasManyGoo != nullptr);
        ASSERT_TRUE (manyFooHasManyGoo != nullptr);

        prim = nullptr;
        foo->CreatePrimitiveProperty (prim, L"fooProp");
        prim->SetType (PrimitiveType::PRIMITIVETYPE_String);
        ASSERT_TRUE (prim != nullptr);

        prim = nullptr;
        goo->CreatePrimitiveProperty (prim, L"gooProp");
        prim->SetType (PrimitiveType::PRIMITIVETYPE_String);
        ASSERT_TRUE (prim != nullptr);

        oneFooHasOneGoo->GetSource ().AddClass (*foo);
        oneFooHasOneGoo->GetSource ().SetCardinality (L"1");
        oneFooHasOneGoo->GetTarget ().AddClass (*goo);
        oneFooHasOneGoo->GetTarget ().SetCardinality (L"1");

        oneFooHasManyGoo->GetSource ().AddClass (*foo);
        oneFooHasManyGoo->GetSource ().SetCardinality (L"1");
        oneFooHasManyGoo->GetTarget ().AddClass (*goo);
        oneFooHasManyGoo->GetTarget ().SetCardinality (L"N");

        manyFooHasManyGoo->GetSource ().AddClass (*foo);
        manyFooHasManyGoo->GetSource ().SetCardinality (L"N");
        manyFooHasManyGoo->GetTarget ().AddClass (*goo);
        manyFooHasManyGoo->GetTarget ().SetCardinality (L"N");
        readContext->AddSchema (*testSchema);

        if (allowDuplicateRelationships)
            {
            auto hintClass = bscaSchema->GetClassCP (L"ECDbRelationshipClassHint");
            ASSERT_TRUE (hintClass != nullptr);
            auto hint = hintClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
            ASSERT_TRUE (hint != nullptr);
            ASSERT_TRUE (hint->SetValue (L"AllowDuplicateRelationships", ECValue (true)) == ECOBJECTS_STATUS_Success);
            ASSERT_TRUE (oneFooHasOneGoo->SetCustomAttribute (*hint) == ECOBJECTS_STATUS_Success);
            ASSERT_TRUE (oneFooHasManyGoo->SetCustomAttribute (*hint) == ECOBJECTS_STATUS_Success);
            ASSERT_TRUE (manyFooHasManyGoo->SetCustomAttribute (*hint) == ECOBJECTS_STATUS_Success);
            }

        if (allowForeignKeyConstraint)
            {
            auto constraintClassHint = bscaSchema->GetClassCP (L"ECDbRelationshipConstraintHint");
            ASSERT_TRUE (constraintClassHint != nullptr);
            auto hint = constraintClassHint->GetDefaultStandaloneEnabler ()->CreateInstance ();
            ASSERT_TRUE (hint != nullptr);
            const WCharCP enforceReferentialIntegrityProperty = L"ForeignKeyConstraint.EnforceReferentialIntegrityCheck";
            ASSERT_TRUE (hint->SetValue (enforceReferentialIntegrityProperty, ECValue (true)) == ECOBJECTS_STATUS_Success);

            ECRelationshipConstraintR targetOnetoOne = oneFooHasOneGoo->GetTarget ();
            ASSERT_TRUE (targetOnetoOne.SetCustomAttribute (*hint) == ECOBJECTS_STATUS_Success);

            ECRelationshipConstraintR targetOneToMany = oneFooHasManyGoo->GetTarget ();
            ASSERT_TRUE (targetOneToMany.SetCustomAttribute (*hint) == ECOBJECTS_STATUS_Success);

            ECRelationshipConstraintR targetManyToMany = manyFooHasManyGoo->GetTarget ();
            ASSERT_TRUE (targetManyToMany.SetCustomAttribute (*hint) == ECOBJECTS_STATUS_Success);
            }

        ASSERT_EQ (ecdb.Schemas ().ImportECSchemas (readContext->GetCache ()), BentleyStatus::SUCCESS);

        std::vector<ECInstanceKey> fooKeys, gooKeys;
        const int maxFooInstances = 3;
        const int maxGooInstances = 3;

        ECSqlStatement fooStmt;
        ASSERT_EQ (fooStmt.Prepare (ecdb, "INSERT INTO ts.Foo(fooProp) VALUES(?)"), ECSqlStatus::Success);
        for (auto i = 0; i < maxFooInstances; i++)
            {
            ECInstanceKey out;
            ASSERT_EQ (fooStmt.Reset (), ECSqlStatus::Success);
            ASSERT_EQ (fooStmt.ClearBindings (), ECSqlStatus::Success);
            ASSERT_EQ (fooStmt.BindText (1, SqlPrintfString ("foo_%d", i), IECSqlBinder::MakeCopy::Yes), ECSqlStatus::Success);
            ASSERT_EQ (fooStmt.Step (out), ECSqlStepStatus::Done);
            fooKeys.push_back (out);
            }

        ECSqlStatement gooStmt;
        ASSERT_EQ (gooStmt.Prepare (ecdb, "INSERT INTO ts.Goo(gooProp) VALUES(?)"), ECSqlStatus::Success);
        for (auto i = 0; i < maxGooInstances; i++)
            {
            ECInstanceKey out;
            ASSERT_EQ (gooStmt.Reset (), ECSqlStatus::Success);
            ASSERT_EQ (gooStmt.ClearBindings (), ECSqlStatus::Success);
            ASSERT_EQ (gooStmt.BindText (1, SqlPrintfString ("goo_%d", i), IECSqlBinder::MakeCopy::Yes), ECSqlStatus::Success);
            ASSERT_EQ (gooStmt.Step (out), ECSqlStepStatus::Done);
            gooKeys.push_back (out);
            }

        //Compute what are the right valid permutation
        std::vector<ECSqlStepStatus> oneFooHasOneGooResult;
        std::vector<ECSqlStepStatus> oneFooHasManyGooResult;
        std::vector<ECSqlStepStatus> manyFooHasManyGooResult;
        std::vector<ECSqlStepStatus> reinsertResultError;
        std::vector<ECSqlStepStatus> reinsertResultDone;
        for (auto f = 0; f < maxFooInstances; f++)
            {
            for (auto g = 0; g < maxGooInstances; g++)
                {
                //1:1 is not effected AllowDuplicateRelationships
                if (f == g)
                    oneFooHasOneGooResult.push_back (ECSqlStepStatus::Done);
                else
                    oneFooHasOneGooResult.push_back (ECSqlStepStatus::Error);

                //1:N is effected AllowDuplicateRelationships
                if (f == 0 || allowDuplicateRelationships)
                    oneFooHasManyGooResult.push_back (ECSqlStepStatus::Done);
                else
                    oneFooHasManyGooResult.push_back (ECSqlStepStatus::Error);

                manyFooHasManyGooResult.push_back (ECSqlStepStatus::Done);
                reinsertResultError.push_back (ECSqlStepStatus::Error);
                reinsertResultDone.push_back (ECSqlStepStatus::Done);
                }
            }

        //1:1--------------------------------
        size_t count_OneFooHasOneGoo = 0;
        VerifyRelationshipInsertionIntegrity (ecdb, "ts.OneFooHasOneGoo", fooKeys, gooKeys, oneFooHasOneGooResult, count_OneFooHasOneGoo);
        VerifyRelationshipInsertionIntegrity (ecdb, "ts.OneFooHasOneGoo", fooKeys, gooKeys, reinsertResultError, count_OneFooHasOneGoo);
        if (allowDuplicateRelationships)
            {
            ASSERT_EQ (GetMapStrategy (ecdb, oneFooHasManyGoo->GetId ()), MapStrategy_TableForThisClass);
            }
        else
            {
            ASSERT_EQ (GetMapStrategy (ecdb, oneFooHasManyGoo->GetId ()), MapStrategy_RelationshipTargetTable);
            }
        ASSERT_EQ (count_OneFooHasOneGoo, GetRelationshipInstanceCount (ecdb, "ts.OneFooHasOneGoo"));

        //1:N--------------------------------
        size_t count_OneFooHasManyGoo = 0;
        VerifyRelationshipInsertionIntegrity (ecdb, "ts.OneFooHasManyGoo", fooKeys, gooKeys, oneFooHasManyGooResult, count_OneFooHasManyGoo);
        if (allowDuplicateRelationships)
            {
            VerifyRelationshipInsertionIntegrity (ecdb, "ts.OneFooHasManyGoo", fooKeys, gooKeys, reinsertResultDone, count_OneFooHasManyGoo);
            ASSERT_EQ (GetMapStrategy (ecdb, oneFooHasManyGoo->GetId ()), MapStrategy_TableForThisClass);
            }
        else
            {
            VerifyRelationshipInsertionIntegrity (ecdb, "ts.OneFooHasManyGoo", fooKeys, gooKeys, reinsertResultError, count_OneFooHasManyGoo);
            ASSERT_EQ (GetMapStrategy (ecdb, oneFooHasManyGoo->GetId ()), MapStrategy_RelationshipTargetTable);
            }
        ASSERT_EQ (count_OneFooHasManyGoo, GetRelationshipInstanceCount (ecdb, "ts.OneFooHasManyGoo"));

        //N:N--------------------------------
        size_t count_ManyFooHasManyGoo = 0;
        VerifyRelationshipInsertionIntegrity (ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, manyFooHasManyGooResult, count_ManyFooHasManyGoo);
        if (allowDuplicateRelationships)
            VerifyRelationshipInsertionIntegrity (ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, reinsertResultDone, count_ManyFooHasManyGoo);
        else
            VerifyRelationshipInsertionIntegrity (ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, reinsertResultError, count_ManyFooHasManyGoo);

        ASSERT_EQ (GetMapStrategy (ecdb, manyFooHasManyGoo->GetId ()), MapStrategy_TableForThisClass);
        ASSERT_EQ (count_ManyFooHasManyGoo, GetRelationshipInstanceCount (ecdb, "ts.ManyFooHasManyGoo"));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECDbHintTests, ForeignKeyConstraint_EnforceReferentialIntegrityCheck)
    {
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("ForeignKeyConstraint_EnforceReferentialIntegrityCheck.ecdb");
    ExecuteRelationshipInsertionIntegrityTest (ecdb, false, true);
    //when AllowDuplicate is turned of, OneFooHasManyGoo will also be mapped as endtable therefore ReferentialIntegrityCheck will be performed for it, so there will be two rows in the ForeignKey table
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasOneGoo"));
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasManyGoo"));

    BeSQLite::Statement sqlStatment;
    auto stat = sqlStatment.Prepare (ecdb, "SELECT ec_Column.[Name] FROM ec_Column JOIN ec_ForeignKey ON ec_ForeignKey.[TableId] = ec_Column.[TableId] JOIN ec_ForeignKeyColumn ON ec_ForeignKeyColumn.[ColumnId] = ec_Column.[Id] WHERE ec_ForeignKey.[Id] = 1");
    ASSERT_EQ (stat, DbResult::BE_SQLITE_OK);
    size_t rowCount = 0;
    while (sqlStatment.Step () != DbResult::BE_SQLITE_DONE)
        {
        rowCount++;
        }
    ASSERT_EQ (2, rowCount);

    sqlStatment.Finalize ();
    ecdb.CloseDb ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECDbHintTests, ForeignKeyConstraint_EnforceReferentialIntegrityCheck_AllowDuplicateRelation)
    {
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("ForeignKeyConstraint_EnforceReferentialIntegrityCheck_AllowDuplicateRelation.ecdb");
    ExecuteRelationshipInsertionIntegrityTest (ecdb, true, true);
    //when AllowDuplicate is turned on, OneFooHasManyGoo will also be mapped as linktable therefore ReferentialIntegrityCheck will not be performed for it, so there will be only one row in the ForeignKey table
    ASSERT_TRUE (ecdb.TableExists ("ts_OneFooHasOneGoo"));
    ASSERT_TRUE (ecdb.TableExists ("ts_OneFooHasManyGoo"));

    BeSQLite::Statement sqlStatment;
    auto stat = sqlStatment.Prepare (ecdb, "SELECT ec_Column.[Name] FROM ec_Column JOIN ec_ForeignKey ON ec_ForeignKey.[TableId] = ec_Column.[TableId] JOIN ec_ForeignKeyColumn ON ec_ForeignKeyColumn.[ColumnId] = ec_Column.[Id] WHERE ec_ForeignKey.[Id] = 1");
    ASSERT_EQ (stat, DbResult::BE_SQLITE_OK);
    size_t rowCount = 0;
    while (sqlStatment.Step () != DbResult::BE_SQLITE_DONE)
        {
        rowCount++;
        }
    ASSERT_EQ (0, rowCount);

    sqlStatment.Finalize ();
    ecdb.CloseDb ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         02/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECDbHintTests, RelationshipTest_DoNotAllowDupilcateRelationships)
    {
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("RelationshipCardinalityTest.ecdb");
    ExecuteRelationshipInsertionIntegrityTest (ecdb, false, false);
    ASSERT_TRUE (ecdb.TableExists ("ts_Foo"));
    ASSERT_TRUE (ecdb.TableExists ("ts_Goo"));
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasOneGoo"));
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasManyGoo"));
    ASSERT_TRUE (ecdb.TableExists ("ts_ManyFooHasManyGoo"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         02/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECDbHintTests, RelationshipTest_AllowDuplicateRelationships)
    {
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("RelationshipCardinalityTest_AllowDuplicateRelationships.ecdb");
    ExecuteRelationshipInsertionIntegrityTest (ecdb, true, false);
    ASSERT_TRUE (ecdb.TableExists ("ts_Foo"));
    ASSERT_TRUE (ecdb.TableExists ("ts_Goo"));
    ASSERT_TRUE (ecdb.TableExists ("ts_OneFooHasOneGoo"));
    ASSERT_TRUE (ecdb.TableExists ("ts_OneFooHasManyGoo"));
    ASSERT_TRUE (ecdb.TableExists ("ts_ManyFooHasManyGoo"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbHintTests, AbstractClassWithTablePerHierarchyAndSharedTableForThisClass)
    {
    auto const schema =
        L"<?xml version='1.0' encoding='utf-8'?>"
        L"<ECSchema schemaName='TestAbstractClasses' nameSpacePrefix='tac' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        L"    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        L"    <ECClass typeName='AbstractBaseClass' isDomainClass='False'>"
        L"        <ECCustomAttributes>"
        L"            <ECDbClassHint xmlns='Bentley_Standard_CustomAttributes.01.00'>"
        L"                <Indexes />"
        L"                <MapStrategy>TablePerHierarchy</MapStrategy>"
        L"            </ECDbClassHint>"
        L"        </ECCustomAttributes>"
        L"        <ECProperty propertyName='P1' typeName='string' />"
        L"    </ECClass>"
        L"    <ECClass typeName='ChildDomainClassA' isDomainClass='True'>"
        L"        <BaseClass>AbstractBaseClass</BaseClass>"
        L"        <ECProperty propertyName='P2' typeName='string' />"
        L"    </ECClass>"
        L"    <ECClass typeName='ChildDomainClassB' isDomainClass='True'>"
        L"        <BaseClass>AbstractBaseClass</BaseClass>"
        L"        <ECProperty propertyName='P3' typeName='string' />"
        L"    </ECClass>"
        L"    <ECClass typeName='SharedTable' isDomainClass='False'>"
        L"        <ECCustomAttributes>"
        L"            <ECDbClassHint xmlns='Bentley_Standard_CustomAttributes.01.00'>"
        L"                <Indexes />"
        L"                <MapStrategy>SharedTableForThisClass</MapStrategy>"
        L"                <TableName>SharedTable</TableName>"
        L"            </ECDbClassHint>"
        L"        </ECCustomAttributes>"
        L"        <ECProperty propertyName='P1' typeName='string' />"
        L"    </ECClass>"
        L"    <ECClass typeName='SharedTable1' isDomainClass='True'>"
        L"        <ECCustomAttributes>"
        L"            <ECDbClassHint xmlns='Bentley_Standard_CustomAttributes.01.00'>"
        L"                <MapStrategy>SharedTableForThisClass</MapStrategy>"
        L"                <TableName>SharedTable</TableName>"
        L"            </ECDbClassHint>"
        L"        </ECCustomAttributes>"
        L"        <ECProperty propertyName='P2' typeName='string' />"
        L"    </ECClass>"
        L"</ECSchema>";

    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create ("abstractClassTest.ecdb");
    ECSchemaPtr testSchema;
    auto readContext = ECSchemaReadContext::CreateContext ();
    ECSchema::ReadFromXmlString (testSchema, schema, *readContext);
    ASSERT_TRUE (testSchema != nullptr);
    auto importStatus = db.Schemas ().ImportECSchemas (readContext->GetCache ());
    ASSERT_TRUE (importStatus == BentleyStatus::SUCCESS);

    //verify tables
    ASSERT_TRUE (db.TableExists ("SharedTable"));
    ASSERT_TRUE (db.TableExists ("tac_AbstractBaseClass"));
    ASSERT_FALSE (db.TableExists ("tac_ChildDomainClassA"));
    ASSERT_FALSE (db.TableExists ("tac_ChildDomainClassB"));
    ASSERT_FALSE (db.TableExists ("tac_SharedTable"));
    ASSERT_FALSE (db.TableExists ("tac_SharedTable1"));

    //verify ECSqlStatement
    ECSqlStatement s1, s2, s3, s4, s5;
    ASSERT_EQ (s1.Prepare (db, "INSERT INTO tac.AbstractBaseClass (P1) VALUES('Hello')"), ECSqlStatus::InvalidECSql);
    ASSERT_EQ (s4.Prepare (db, "INSERT INTO tac.SharedTable (P1) VALUES('Hello')"), ECSqlStatus::InvalidECSql);
    //Noabstract classes
    ASSERT_EQ (s2.Prepare (db, "INSERT INTO tac.ChildDomainClassA (P1, P2) VALUES('Hello', 'World')"), ECSqlStatus::Success);
    ASSERT_EQ (s2.Step (), ECSqlStepStatus::Done);
    ASSERT_EQ (s3.Prepare (db, "INSERT INTO tac.ChildDomainClassB (P1, P3) VALUES('Hello', 'World')"), ECSqlStatus::Success);
    ASSERT_EQ (s3.Step (), ECSqlStepStatus::Done);
    ASSERT_EQ (s5.Prepare (db, "INSERT INTO tac.SharedTable1 (P2) VALUES('Hello')"), ECSqlStatus::Success);
    ASSERT_EQ (s5.Step (), ECSqlStepStatus::Done);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbHintTests, TablePerHierarchy_WithReuseColumns)
    {
    auto const schema =
        L"<?xml version='1.0' encoding='utf-8'?>"
        L"<ECSchema schemaName='SchemaWithReuseColumn' nameSpacePrefix='rc' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        L"    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        L"    <ECClass typeName='BaseClass' isDomainClass='True'>"
        L"        <ECCustomAttributes>"
        L"            <ECDbClassHint xmlns='Bentley_Standard_CustomAttributes.01.00'>"
        L"                <Indexes />"
        L"                <MapStrategy>TablePerHierarchy | ReuseColumns</MapStrategy>"
        L"            </ECDbClassHint>"
        L"        </ECCustomAttributes>"
        L"        <ECProperty propertyName='P1' typeName='string' />"
        L"    </ECClass>"
        L"    <ECClass typeName='ChildDomainClassA' isDomainClass='True'>"
        L"        <BaseClass>BaseClass</BaseClass>"
        L"        <ECCustomAttributes>"
        L"            <ECDbClassHint xmlns='Bentley_Standard_CustomAttributes.01.00'>"
        L"                <Indexes />"
        L"                <MapStrategy>TablePerHierarchy | ReuseColumns</MapStrategy>"
        L"            </ECDbClassHint>"
        L"        </ECCustomAttributes>"
        L"        <ECProperty propertyName='P2' typeName='double' />"
        L"    </ECClass>"
        L"    <ECClass typeName='ChildDomainClassB' isDomainClass='True'>"
        L"        <BaseClass>BaseClass</BaseClass>"
        L"        <ECProperty propertyName='P3' typeName='int' />"
        L"    </ECClass>"
        L"    <ECClass typeName='DerivedA' isDomainClass='True'>"
        L"        <BaseClass>ChildDomainClassA</BaseClass>"
        L"        <ECProperty propertyName='P4' typeName='double' />"
        L"    </ECClass>"
        L"    <ECClass typeName='DerivedB' isDomainClass='True'>"
        L"        <BaseClass>ChildDomainClassA</BaseClass>"
        L"        <ECProperty propertyName='P5' typeName='string' />"
        L"    </ECClass>"
        L"</ECSchema>";

    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create ("columnReuseTest.ecdb");
    ECSchemaPtr testSchema;
    auto readContext = ECSchemaReadContext::CreateContext ();
    ECSchema::ReadFromXmlString (testSchema, schema, *readContext);
    ASSERT_TRUE (testSchema != nullptr);
    auto importStatus = db.Schemas ().ImportECSchemas (readContext->GetCache ());
    ASSERT_TRUE (importStatus == BentleyStatus::SUCCESS);

    //verify tables
    ASSERT_TRUE (db.TableExists ("rc_BaseClass"));
    ASSERT_FALSE (db.TableExists ("rc_ChildDomainClassA"));
    ASSERT_FALSE (db.TableExists ("rc_ChildDomainClassB"));
    ASSERT_FALSE (db.TableExists ("rc_DerivedA"));
    ASSERT_FALSE (db.TableExists ("rc_DerivedB"));

    //verify ECSqlStatments
    ECSqlStatement s1, s2, s3, s4, s5;
    ASSERT_EQ (s1.Prepare (db, "INSERT INTO rc.BaseClass (P1) VALUES('HelloWorld')"), ECSqlStatus::Success);
    ASSERT_EQ (s1.Step (), ECSqlStepStatus::Done);
    ASSERT_EQ (s2.Prepare (db, "INSERT INTO rc.ChildDomainClassA (P1, P2) VALUES('ChildClassA', 10.002)"), ECSqlStatus::Success);
    ASSERT_EQ (s2.Step (), ECSqlStepStatus::Done);
    ASSERT_EQ (s3.Prepare (db, "INSERT INTO rc.ChildDomainClassB (P1, P3) VALUES('ChildClassB', 2)"), ECSqlStatus::Success);
    ASSERT_EQ (s3.Step (), ECSqlStepStatus::Done);
    ASSERT_EQ (s4.Prepare (db, "INSERT INTO rc.DerivedA (P1, P2, P4) VALUES('DerivedA', 11.003, 12.004)"), ECSqlStatus::Success);
    ASSERT_EQ (s4.Step (), ECSqlStepStatus::Done);
    ASSERT_EQ (s5.Prepare (db, "INSERT INTO rc.DerivedB (P1, P2, P5) VALUES('DerivedB', 11.003, 'DerivedB')"), ECSqlStatus::Success);
    ASSERT_EQ (s5.Step (), ECSqlStepStatus::Done);

    //verify No of Columns in BaseClass
    Statement statement;
    ASSERT_EQ (statement.Prepare(db, "SELECT * FROM rc_BaseClass"), DbResult::BE_SQLITE_OK);
    ASSERT_EQ (statement.Step(), DbResult::BE_SQLITE_ROW);
    size_t columnCount = statement.GetColumnCount ();
    ASSERT_EQ (columnCount, 5);

    //verify that the columns generated are same as expected
    Utf8String expectedColumnNames = "ECInstanceIdECClassIdP1x01x02";
    Utf8String actualColumnNames;
    for (int i = 0; i < 5; i++)
        {
        actualColumnNames.append (statement.GetColumnName (i));
        }
    ASSERT_EQ (expectedColumnNames, actualColumnNames);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbHintTests, TablePerHierarchy_ReuseColumns_DisableReuseColumnsForThisClass)
    {
    auto const schema =
        L"<?xml version='1.0' encoding='utf-8'?>"
        L"<ECSchema schemaName='SchemaWithReuseColumn' nameSpacePrefix='rc' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        L"    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        L"    <ECClass typeName='BaseClass' isDomainClass='True'>"
        L"        <ECCustomAttributes>"
        L"            <ECDbClassHint xmlns='Bentley_Standard_CustomAttributes.01.00'>"
        L"                <Indexes />"
        L"                <MapStrategy>TablePerHierarchy | ReuseColumns</MapStrategy>"
        L"            </ECDbClassHint>"
        L"        </ECCustomAttributes>"
        L"        <ECProperty propertyName='P1' typeName='string' />"
        L"    </ECClass>"
        L"    <ECClass typeName='ChildDomainClassA' isDomainClass='True'>"
        L"        <BaseClass>BaseClass</BaseClass>"
        L"        <ECCustomAttributes>"
        L"            <ECDbClassHint xmlns='Bentley_Standard_CustomAttributes.01.00'>"
        L"                <Indexes />"
        L"                <MapStrategy>DisableReuseColumnsForThisClass</MapStrategy>"
        L"            </ECDbClassHint>"
        L"        </ECCustomAttributes>"
        L"        <ECProperty propertyName='P2' typeName='double' />"
        L"    </ECClass>"
        L"    <ECClass typeName='ChildDomainClassB' isDomainClass='True'>"
        L"        <BaseClass>BaseClass</BaseClass>"
        L"        <ECCustomAttributes>"
        L"            <ECDbClassHint xmlns='Bentley_Standard_CustomAttributes.01.00'>"
        L"                <Indexes />"
        L"                <MapStrategy>DisableReuseColumnsForThisClass</MapStrategy>"
        L"            </ECDbClassHint>"
        L"        </ECCustomAttributes>"
        L"        <ECProperty propertyName='P3' typeName='int' />"
        L"    </ECClass>"
        L"    <ECClass typeName='DerivedA' isDomainClass='True'>"
        L"        <BaseClass>ChildDomainClassA</BaseClass>"
        L"        <ECProperty propertyName='P4' typeName='double' />"
        L"    </ECClass>"
        L"    <ECClass typeName='DerivedB' isDomainClass='True'>"
        L"        <BaseClass>ChildDomainClassA</BaseClass>"
        L"        <ECProperty propertyName='P5' typeName='string' />"
        L"    </ECClass>"
        L"</ECSchema>";

    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create ("columnReuseTest.ecdb");
    ECSchemaPtr testSchema;
    auto readContext = ECSchemaReadContext::CreateContext ();
    ECSchema::ReadFromXmlString (testSchema, schema, *readContext);
    ASSERT_TRUE (testSchema != nullptr);
    auto importStatus = db.Schemas ().ImportECSchemas (readContext->GetCache ());
    ASSERT_TRUE (importStatus == BentleyStatus::SUCCESS);

    //verify tables
    ASSERT_TRUE (db.TableExists ("rc_BaseClass"));
    ASSERT_FALSE (db.TableExists ("rc_ChildDomainClassA"));
    ASSERT_FALSE (db.TableExists ("rc_ChildDomainClassB"));
    ASSERT_FALSE (db.TableExists ("rc_DerivedA"));
    ASSERT_FALSE (db.TableExists ("rc_DerivedB"));

    //verify ECSqlStatments
    ECSqlStatement s1, s2, s3, s4, s5;
    ASSERT_EQ (s1.Prepare (db, "INSERT INTO rc.BaseClass (P1) VALUES('HelloWorld')"), ECSqlStatus::Success);
    ASSERT_EQ (s1.Step (), ECSqlStepStatus::Done);
    ASSERT_EQ (s2.Prepare (db, "INSERT INTO rc.ChildDomainClassA (P1, P2) VALUES('ChildClassA', 10.002)"), ECSqlStatus::Success);
    ASSERT_EQ (s2.Step (), ECSqlStepStatus::Done);
    ASSERT_EQ (s3.Prepare (db, "INSERT INTO rc.ChildDomainClassB (P1, P3) VALUES('ChildClassB', 2)"), ECSqlStatus::Success);
    ASSERT_EQ (s3.Step (), ECSqlStepStatus::Done);
    ASSERT_EQ (s4.Prepare (db, "INSERT INTO rc.DerivedA (P1, P2, P4) VALUES('DerivedA', 11.003, 12.004)"), ECSqlStatus::Success);
    ASSERT_EQ (s4.Step (), ECSqlStepStatus::Done);
    ASSERT_EQ (s5.Prepare (db, "INSERT INTO rc.DerivedB (P1, P2, P5) VALUES('DerivedB', 11.003, 'DerivedB')"), ECSqlStatus::Success);
    ASSERT_EQ (s5.Step (), ECSqlStepStatus::Done);

    //verify No of Columns in BaseClass
    Statement statement;
    ASSERT_EQ (statement.Prepare (db, "SELECT * FROM rc_BaseClass"), DbResult::BE_SQLITE_OK);
    ASSERT_EQ (statement.Step (), DbResult::BE_SQLITE_ROW);
    size_t columnCount = statement.GetColumnCount ();
    ASSERT_EQ (columnCount, 6);

    //verify that the columns generated are same as expected
    Utf8String expectedColumnNames = "ECInstanceIdECClassIdP1P2x01P3";
    Utf8String actualColumnNames;
    for (int i = 0; i < 6; i++)
        {
        actualColumnNames.append (statement.GetColumnName (i));
        }
    ASSERT_EQ (expectedColumnNames, actualColumnNames);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbHintTests, TestInValidMapStrategyValue)
    {
    auto const schema =
        L"<?xml version='1.0' encoding='utf-8'?>"
        L"<ECSchema schemaName='SchemaWithReuseColumn' nameSpacePrefix='rc' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        L"    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        L"    <ECClass typeName='ClassA' isDomainClass='True'>"
        L"        <ECCustomAttributes>"
        L"            <ECDbClassHint xmlns='Bentley_Standard_CustomAttributes.01.00'>"
        L"                <Indexes />"
        L"                <MapStrategy>TablePerHierarchy | ReuseColumn</MapStrategy>"
        L"            </ECDbClassHint>"
        L"        </ECCustomAttributes>"
        L"        <ECProperty propertyName='Price' typeName='double' />"
        L"    </ECClass>"
        L"    <ECClass typeName='ClassAB' isDomainClass='True'>"
        L"        <BaseClass>ClassA</BaseClass>"
        L"    </ECClass>"
        L"</ECSchema>";

    ECDbTestProject::Initialize ();
    ECDb ecdb;
    ASSERT_EQ (BE_SQLITE_OK, ECDbTestUtility::CreateECDb (ecdb, nullptr, L"SchemaHintDb.ecdb"))<<"ECDb couldn't be created";

    ECSchemaPtr testSchema;
    auto readContext = ECSchemaReadContext::CreateContext ();
    ECSchema::ReadFromXmlString (testSchema, schema, *readContext);
    ASSERT_TRUE (testSchema != nullptr);
    auto importStatus = ecdb.Schemas ().ImportECSchemas (readContext->GetCache ());
    ASSERT_TRUE (importStatus == BentleyStatus::ERROR)<< "Schema import successful instead of returning error";
    }
END_ECDBUNITTESTS_NAMESPACE