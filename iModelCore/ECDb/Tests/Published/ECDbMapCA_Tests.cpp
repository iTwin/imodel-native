/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbMapCA_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct ECDbMapCATests : public ::testing::Test
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


    //This is a mirror of the internal MapStrategy used by ECDb and persisted in the DB.
    //The values can change, so in that case this struct needs to be updated accordingly.
    struct ResolvedMapStrategy
        {
        enum class Strategy
            {
            NotMapped,
            OwnTable,
            SharedTable,
            ExistingTable,

            ForeignKeyRelationshipInTargetTable = 100,
            ForeignKeyRelationshipInSourceTable = 101
            };

        enum class Options
            {
            None = 0,
            Readonly = 1,
            SharedColumns = 2
            };

        Strategy m_strategy;
        Options m_options;
        bool m_isPolymorphic;

        ResolvedMapStrategy() : m_strategy(Strategy::NotMapped), m_options(Options::None), m_isPolymorphic(false) {}
        ResolvedMapStrategy(Strategy strategy, Options options, bool isPolymorphic) : m_strategy(strategy), m_options(options), m_isPolymorphic(isPolymorphic) {}
        };

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                   Affan.Khan                         02/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    ResolvedMapStrategy GetMapStrategy(ECDbR ecdb, ECClassId ClassId)
        {
        Statement stmt;
        stmt.Prepare (ecdb, "SELECT MapStrategy, MapStrategyOptions, IsMapStrategyPolymorphic FROM ec_ClassMap WHERE ClassId = ?");
        stmt.BindInt64 (1, ClassId);
        if (stmt.Step() == BE_SQLITE_ROW)
            {
            ResolvedMapStrategy::Strategy strat = (ResolvedMapStrategy::Strategy) stmt.GetValueInt(0);
            ResolvedMapStrategy::Options options = stmt.IsColumnNull(1) ? ResolvedMapStrategy::Options::None : (ResolvedMapStrategy::Options) stmt.GetValueInt(1);
            bool isPolymorphic = stmt.GetValueInt(2) != 0;
            return ResolvedMapStrategy(strat, options, isPolymorphic);
            }

        return ResolvedMapStrategy();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                   Affan.Khan                         02/15
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ExecuteRelationshipInsertionIntegrityTest (ECDbR ecdb, bool allowDuplicateRelationships, bool allowForeignKeyConstraint, bool schemaImportExpectedToSucceed)
        {
        ECSchemaPtr testSchema;
        ECClassP foo = nullptr, goo = nullptr;
        ECRelationshipClassP oneFooHasOneGoo = nullptr, oneFooHasManyGoo = nullptr, manyFooHasManyGoo = nullptr;
        PrimitiveECPropertyP prim;
        auto readContext = ECSchemaReadContext::CreateContext ();
        readContext->AddSchemaLocater (ecdb.GetSchemaLocater ());
        auto ecdbmapKey = SchemaKey ("ECDbMap", 1, 0);
        auto ecdbmapSchema = readContext->LocateSchema (ecdbmapKey, SchemaMatchType::SCHEMAMATCHTYPE_LatestCompatible);
        ASSERT_TRUE (ecdbmapSchema.IsValid ());

        ECSchema::CreateSchema (testSchema, "TestSchema", 1, 0);
        ASSERT_TRUE (testSchema.IsValid ());

        testSchema->SetNamespacePrefix ("ts");
        testSchema->AddReferencedSchema (*ecdbmapSchema);

        testSchema->CreateClass (foo, "Foo");
        testSchema->CreateClass (goo, "Goo");

        testSchema->CreateRelationshipClass (oneFooHasOneGoo, "OneFooHasOneGoo");
        testSchema->CreateRelationshipClass (oneFooHasManyGoo, "OneFooHasManyGoo");
        testSchema->CreateRelationshipClass (manyFooHasManyGoo, "ManyFooHasManyGoo");

        ASSERT_TRUE (foo != nullptr);
        ASSERT_TRUE (foo != nullptr);
        ASSERT_TRUE (oneFooHasOneGoo != nullptr);
        ASSERT_TRUE (oneFooHasManyGoo != nullptr);
        ASSERT_TRUE (manyFooHasManyGoo != nullptr);

        prim = nullptr;
        foo->CreatePrimitiveProperty (prim, "fooProp");
        prim->SetType (PrimitiveType::PRIMITIVETYPE_String);
        ASSERT_TRUE (prim != nullptr);

        prim = nullptr;
        goo->CreatePrimitiveProperty (prim, "gooProp");
        prim->SetType (PrimitiveType::PRIMITIVETYPE_String);
        ASSERT_TRUE (prim != nullptr);

        oneFooHasOneGoo->GetSource ().AddClass (*foo);
        oneFooHasOneGoo->GetSource ().SetCardinality ("1");
        oneFooHasOneGoo->GetTarget ().AddClass (*goo);
        oneFooHasOneGoo->GetTarget ().SetCardinality ("1");

        oneFooHasManyGoo->GetSource ().AddClass (*foo);
        oneFooHasManyGoo->GetSource ().SetCardinality ("1");
        oneFooHasManyGoo->GetTarget ().AddClass (*goo);
        oneFooHasManyGoo->GetTarget ().SetCardinality ("N");

        manyFooHasManyGoo->GetSource ().AddClass (*foo);
        manyFooHasManyGoo->GetSource ().SetCardinality ("N");
        manyFooHasManyGoo->GetTarget ().AddClass (*goo);
        manyFooHasManyGoo->GetTarget ().SetCardinality ("N");
        Backdoor::ECObjects::ECSchemaReadContext::AddSchema(*readContext, *testSchema);

        if (allowDuplicateRelationships)
            {
            auto caInstClass = ecdbmapSchema->GetClassCP ("LinkTableRelationshipMap");
            ASSERT_TRUE (caInstClass != nullptr);
            auto caInst = caInstClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
            ASSERT_TRUE (caInst != nullptr);
            ASSERT_TRUE (caInst->SetValue ("AllowDuplicateRelationships", ECValue (true)) == ECOBJECTS_STATUS_Success);
            ASSERT_TRUE (manyFooHasManyGoo->SetCustomAttribute (*caInst) == ECOBJECTS_STATUS_Success);
            }

        if (allowForeignKeyConstraint)
            {
            auto fkMapClass = ecdbmapSchema->GetClassCP ("ForeignKeyRelationshipMap");
            ASSERT_TRUE (fkMapClass != nullptr);
            auto caInst = fkMapClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
            ASSERT_TRUE (caInst != nullptr);
            const Utf8CP enforceReferentialIntegrityProperty = "CreateConstraint";
            ASSERT_TRUE (caInst->SetValue (enforceReferentialIntegrityProperty, ECValue (true)) == ECOBJECTS_STATUS_Success);
            ASSERT_TRUE(oneFooHasOneGoo->SetCustomAttribute(*caInst) == ECOBJECTS_STATUS_Success);
            ASSERT_TRUE (oneFooHasManyGoo->SetCustomAttribute (*caInst) == ECOBJECTS_STATUS_Success);
            }

        if (schemaImportExpectedToSucceed)
            ASSERT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (readContext->GetCache ()));
        else
            {
            ASSERT_EQ(ERROR, ecdb.Schemas().ImportECSchemas(readContext->GetCache()));
            return;
            }

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
                //1:1 is not effected with AllowDuplicateRelationships
                if (f == g)
                    oneFooHasOneGooResult.push_back (ECSqlStepStatus::Done);
                else
                    oneFooHasOneGooResult.push_back (ECSqlStepStatus::Error);

                //1:N is effected with AllowDuplicateRelationships
                if (f == 0)
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

        ResolvedMapStrategy mapStrategy = GetMapStrategy(ecdb, oneFooHasOneGoo->GetId());
        ASSERT_EQ(ResolvedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);
        ASSERT_EQ (count_OneFooHasOneGoo, GetRelationshipInstanceCount (ecdb, "ts.OneFooHasOneGoo"));

        //1:N--------------------------------
        size_t count_OneFooHasManyGoo = 0;
        VerifyRelationshipInsertionIntegrity (ecdb, "ts.OneFooHasManyGoo", fooKeys, gooKeys, oneFooHasManyGooResult, count_OneFooHasManyGoo);

        mapStrategy = GetMapStrategy(ecdb, oneFooHasManyGoo->GetId());
        ASSERT_EQ(ResolvedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);
        ASSERT_EQ (count_OneFooHasManyGoo, GetRelationshipInstanceCount (ecdb, "ts.OneFooHasManyGoo"));

        //N:N--------------------------------
        size_t count_ManyFooHasManyGoo = 0;
        VerifyRelationshipInsertionIntegrity (ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, manyFooHasManyGooResult, count_ManyFooHasManyGoo);
        if (allowDuplicateRelationships)
            VerifyRelationshipInsertionIntegrity (ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, reinsertResultDone, count_ManyFooHasManyGoo);
        else
            VerifyRelationshipInsertionIntegrity (ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, reinsertResultError, count_ManyFooHasManyGoo);

        mapStrategy = GetMapStrategy(ecdb, manyFooHasManyGoo->GetId());

        ASSERT_EQ(ResolvedMapStrategy::Strategy::OwnTable, mapStrategy.m_strategy);
        ASSERT_EQ(ResolvedMapStrategy::Options::None, mapStrategy.m_options);
        ASSERT_FALSE(mapStrategy.m_isPolymorphic);
        ASSERT_EQ (count_ManyFooHasManyGoo, GetRelationshipInstanceCount (ecdb, "ts.ManyFooHasManyGoo"));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECDbMapCATests, ForeignKeyRelationshipMap_EnforceReferentialIntegrity)
    {
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("ForeignKeyRelationshipMap_EnforceReferentialIntegrity.ecdb");
    ExecuteRelationshipInsertionIntegrityTest (ecdb, false, true, true);
    //when AllowDuplicate is turned of, OneFooHasManyGoo will also be mapped as endtable therefore ReferentialIntegrityCheck will be performed for it, so there will be two rows in the ForeignKey table
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasOneGoo"));
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasManyGoo"));

    BeSQLite::Statement sqlStatment;
    auto stat = sqlStatment.Prepare (ecdb, "SELECT ec_Column.Name FROM ec_Column JOIN ec_ForeignKey ON ec_ForeignKey.TableId = ec_Column.[TableId] JOIN ec_ForeignKeyColumn ON ec_ForeignKeyColumn.ColumnId = ec_Column.Id WHERE ec_ForeignKey.Id = 1");
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
TEST_F (ECDbMapCATests, ForeignKeyConstraint_EnforceReferentialIntegrityCheck_AllowDuplicateRelation)
    {
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("ForeignKeyConstraint_EnforceReferentialIntegrityCheck_AllowDuplicateRelation.ecdb");
    ExecuteRelationshipInsertionIntegrityTest (ecdb, true, true, true);
    //when AllowDuplicate is turned on, OneFooHasManyGoo will also be mapped as endtable therefore there will be only one row in the ForeignKey table
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
* @bsimethod                                   Affan.Khan                         02/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECDbMapCATests, RelationshipTest_DoNotAllowDuplicateRelationships)
    {
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("RelationshipCardinalityTest.ecdb");
    ExecuteRelationshipInsertionIntegrityTest (ecdb, false, false, true);
    ASSERT_TRUE (ecdb.TableExists ("ts_Foo"));
    ASSERT_TRUE (ecdb.TableExists ("ts_Goo"));
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasOneGoo"));
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasManyGoo"));
    ASSERT_TRUE (ecdb.TableExists ("ts_ManyFooHasManyGoo"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         02/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECDbMapCATests, RelationshipTest_AllowDuplicateRelationships)
    {
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("RelationshipCardinalityTest_AllowDuplicateRelationships.ecdb");
    ExecuteRelationshipInsertionIntegrityTest (ecdb, true, false, true);
    ASSERT_TRUE (ecdb.TableExists ("ts_Foo"));
    ASSERT_TRUE (ecdb.TableExists ("ts_Goo"));
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasOneGoo"));
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasManyGoo"));
    ASSERT_TRUE (ecdb.TableExists ("ts_ManyFooHasManyGoo"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (SchemaImportTestFixture, AbstractClassWithPolymorphicAndNonPolymorphicSharedTable)
    {
    TestItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestAbstractClasses' nameSpacePrefix='tac' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='AbstractBaseClass' isDomainClass='False'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                  <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P1' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='ChildDomainClassA' isDomainClass='True'>"
        "        <BaseClass>AbstractBaseClass</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='ChildDomainClassB' isDomainClass='True'>"
        "        <BaseClass>AbstractBaseClass</BaseClass>"
        "        <ECProperty propertyName='P3' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='SharedTable' isDomainClass='False'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                  <IsPolymorphic>False</IsPolymorphic>"
        "                </MapStrategy>"
        "                <TableName>SharedTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P1' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='SharedTable1' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                  <IsPolymorphic>False</IsPolymorphic>"
        "                </MapStrategy>"
        "                <TableName>SharedTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P2' typeName='string' />"
        "    </ECClass>"
        "</ECSchema>",
        true,"");

    ECDb db;
    bool asserted = false;
    AssertSchemaImport(db, asserted, testItem, "abstractclass.ecdb");
    ASSERT_FALSE(asserted);

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
TEST_F(SchemaImportTestFixture, PolymorphicSharedTable_SharedColumns)
    {
    TestItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='SchemaWithSharedColumns' nameSpacePrefix='rc' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='BaseClass' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                  <Options>SharedColumnsForSubclasses</Options>"
        "                  <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P1' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='ChildDomainClassA' isDomainClass='True'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                  <Options>SharedColumnsForSubclasses</Options>"
        "                  <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P2' typeName='double' />"
        "    </ECClass>"
        "    <ECClass typeName='ChildDomainClassB' isDomainClass='True'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='DerivedA' isDomainClass='True'>"
        "        <BaseClass>ChildDomainClassA</BaseClass>"
        "        <ECProperty propertyName='P4' typeName='double' />"
        "    </ECClass>"
        "    <ECClass typeName='DerivedB' isDomainClass='True'>"
        "        <BaseClass>ChildDomainClassA</BaseClass>"
        "        <ECProperty propertyName='P5' typeName='string' />"
        "    </ECClass>"
        "</ECSchema>", true, "");

    ECDb db;
    bool asserted = false;
    AssertSchemaImport(db, asserted, testItem, "sharedcolumns.ecdb");
    ASSERT_FALSE(asserted);

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
TEST_F(SchemaImportTestFixture, PolymorphicSharedTable_SharedColumns_DisableSharedColumns)
    {
    TestItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='SchemaWithSharedColumns' nameSpacePrefix='rc' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='BaseClass' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                  <Options>SharedColumnsForSubclasses</Options>"
        "                  <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P1' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='ChildDomainClassA' isDomainClass='True'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Options>DisableSharedColumns</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P2' typeName='double' />"
        "    </ECClass>"
        "    <ECClass typeName='ChildDomainClassB' isDomainClass='True'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Options>DisableSharedColumns</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='DerivedA' isDomainClass='True'>"
        "        <BaseClass>ChildDomainClassA</BaseClass>"
        "        <ECProperty propertyName='P4' typeName='double' />"
        "    </ECClass>"
        "    <ECClass typeName='DerivedB' isDomainClass='True'>"
        "        <BaseClass>ChildDomainClassA</BaseClass>"
        "        <ECProperty propertyName='P5' typeName='string' />"
        "    </ECClass>"
        "</ECSchema>", true, "");

    ECDb db;
    bool asserted = false;
    AssertSchemaImport(db, asserted, testItem, "sharedcolumnstest.ecdb");
    ASSERT_FALSE(asserted);

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
    const int expectedColCount = 6;
    Statement statement;
    ASSERT_EQ (statement.Prepare (db, "SELECT * FROM rc_BaseClass"), DbResult::BE_SQLITE_OK);
    ASSERT_EQ (statement.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ(expectedColCount, statement.GetColumnCount());

    //verify that the columns generated are same as expected
    Utf8String expectedColumnNames = "ECInstanceIdECClassIdP1P2x01P3";
    Utf8String actualColumnNames;
    for (int i = 0; i < expectedColCount; i++)
        {
        actualColumnNames.append (statement.GetColumnName (i));
        }
    ASSERT_STREQ (expectedColumnNames.c_str(), actualColumnNames.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaImportTestFixture, ECDbMapCATests)
    {
    //TODO: Please complete this test with all other invalid combinations

    std::vector<TestItem> testItems {
        TestItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='ClassA' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy><Strategy>bla</Strategy></MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "    <ECClass typeName='ClassAB' isDomainClass='True'>"
        "        <BaseClass>ClassA</BaseClass>"
        "    </ECClass>"
        "</ECSchema>", false, ""),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='ClassA' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>OwnTable</Strategy>"
        "                   <Options>SharedColumns</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, ""),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>ExistingTable</Strategy>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy ExistingTable expects TableName to be set"),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>ExistingTable</Strategy>"
        "                </MapStrategy>"
        "                <TableName>idontexist</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy ExistingTable expects table specified by TableName to preexist"),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>OwnTable</Strategy>"
        "                   <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "                <TableName>bla</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy OwnTable doesn't allow TableName to be set."),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>OwnTable</Strategy>"
        "                   <IsPolymorphic>False</IsPolymorphic>"
        "                </MapStrategy>"
        "                <TableName>bla</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy OwnTable doesn't allow TableName to be set."),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "                <TableName>bla</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy SharedTable, polymorphic doesn't allow TableName to be set."),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <IsPolymorphic>False</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy SharedTable, non-polymorphic expects TableName to be set."),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <IsPolymorphic>False</IsPolymorphic>"
        "                </MapStrategy>"
        "                <TableName>idontexistyet</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", true, "MapStrategy SharedTable, non-polymorphic doesn't expect table specified in TableName to be set."),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>NotMapped</Strategy>"
        "                   <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "                <TableName>bla</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy NotMapped, polymorphic doesn't allow TableName to be set."),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>NotMapped</Strategy>"
        "                   <IsPolymorphic>False</IsPolymorphic>"
        "                </MapStrategy>"
        "                <TableName>bla</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy NotMapped, non-polymorphic doesn't allow TableName to be set."),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='A' isDomainClass='True'>"
        "        <ECProperty propertyName='AA' typeName='double' />"
        "    </ECClass>"
        "    <ECClass typeName='B' isDomainClass='True'>"
        "        <ECProperty propertyName='BB' typeName='double' />"
        "    </ECClass>"
        "    <ECRelationshipClass typeName='Rel' isDomainClass='True' strength='referencing'>"
        "        <ECCustomAttributes>"
        "            <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
        "            </ForeignKeyRelationshipMap>"
        "        </ECCustomAttributes>"
        "       <Source cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='B' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>", false, ""),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TeststructClassInPolymorphicSharedTable' nameSpacePrefix='tph' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='BaseClass' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='p1' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='CustomAttributeClass' isDomainClass='False' isCustomAttributeClass='True'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECProperty propertyName='p2' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='isStructClass' isStruct='True' isDomainClass='True'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECProperty propertyName='p3' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='NonDomainClass' isDomainClass='False'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECProperty propertyName='p4' typeName='string' />"
        "    </ECClass>"
        "</ECSchema>", false, "Struct in class hierarchy with SharedTable (polymorphic) map strategy is expected to be not supported.")
        };

    for (TestItem const& item : testItems)
        {
        AssertSchemaImport(item, "ecdbmapcatests.ecdb");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbMapCATests, UniqueIndexesSupportFor1to1RelationshipWithTablePerHierarchy)
    {
    ECDbTestProject testproject;
    ECDbR ecdb = testproject.Create ("PartialIndexWithMultipleRelationshipInSameTable_Db.ecdb", L"TablePerHierarchy.01.00.ecschema.xml", false);

    BeSQLite::Statement stmt;
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (ecdb, "SELECT Id from ec_Class where ec_Class.[Name] = ?"));
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindText (1, "TPHhasClassA", Statement::MakeCopy::No));
    ASSERT_EQ (DbResult::BE_SQLITE_ROW, stmt.Step ());

    Utf8String whereClauseValue;
    ASSERT_EQ (SUCCESS, whereClauseValue.Sprintf ("ECClassId = %d", stmt.GetValueInt (0)));
    stmt.Finalize ();

    //verify that entry in the ec_Index table exists for relationship table TPHhasClassA
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (ecdb, "SELECT ec_Index.[Name], ec_Index.[IsUnique] from ec_Index where ec_Index.[WhereClause] = ?"));
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindText (1, whereClauseValue, Statement::MakeCopy::No));
    while (DbResult::BE_SQLITE_ROW == stmt.Step ())
        {
        ASSERT_EQ (1, stmt.GetValueInt (1)) << "Index value for 1:1 Relationship is not Unique";
        Utf8String indexName = stmt.GetValueText (0);
        ASSERT_TRUE (indexName == "idx_ECRel_Source_Unique_tph_TPHOwnsTPH" || "idx_ECRel_Target_Unique_tph_TPHOwnsTPH");
        }
    stmt.Finalize ();

    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (ecdb, "SELECT Id from ec_Class where ec_Class.[Name] = ?"));
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindText (1, "TPHhasClassB", Statement::MakeCopy::No));
    ASSERT_EQ (DbResult::BE_SQLITE_ROW, stmt.Step ());

    ASSERT_EQ (SUCCESS, whereClauseValue.Sprintf ("ECClassId = %d", stmt.GetValueInt (0)));
    stmt.Finalize ();

    //verify that entry in ec_Index table also exists for relationship table TPHhasClassB
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (ecdb, "SELECT ec_Index.[Name], ec_Index.[IsUnique] from ec_Index where ec_Index.[WhereClause] = ?"));
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindText (1, whereClauseValue, Statement::MakeCopy::No));
    while (DbResult::BE_SQLITE_ROW == stmt.Step ())
        {
        ASSERT_EQ (1, stmt.GetValueInt (1)) << "Index value for 1:1 Relationship is not Unique";
        Utf8String indexName = stmt.GetValueText (0);
        ASSERT_TRUE (indexName == "uix_unique_tph_TPHhasClassB_Source" || "uix_unique_tph_TPHhasClassB_Target");
        }
    stmt.Finalize ();

    ecdb.CloseDb ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbMapCATests, InstanceInsertionForRelationshipsMappedToSameTable)
    {
    ECDbTestProject testproject;
    ECDbR ecdb = testproject.Create ("RelationshipsWithTablePerHierarchy.ecdb", L"TablePerHierarchy.01.00.ecschema.xml", false);

    ASSERT_TRUE (ecdb.TableExists ("tph_TPHOwnsTPH"));
    ASSERT_FALSE (ecdb.TableExists ("tph_TPHhasClassA"));
    ASSERT_FALSE (ecdb.TableExists ("tph_TPHhasClassB"));

    ECSchemaCP schema = ecdb.Schemas ().GetECSchema ("TablePerHierarchy", true);
    ASSERT_TRUE (schema != nullptr) << "Couldn't locate Schema TablerPerHierarchy";

    ECClassCP TPH = schema->GetClassCP ("TPH");
    ASSERT_TRUE (TPH != nullptr) << "Couldn't locate class TPH from schema";
    ECClassCP classA = schema->GetClassCP ("ClassA");
    ASSERT_TRUE (classA != nullptr) << "Couldn't locate classA from Schema";
    ECClassCP classB = schema->GetClassCP ("ClassB");
    ASSERT_TRUE (classB != nullptr) << "Couldn't locate classB from Schema";

    //Insert Instances for class TPH
    ECN::StandaloneECInstancePtr TPHInstance1 = TPH->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ECN::StandaloneECInstancePtr TPHInstance2 = TPH->GetDefaultStandaloneEnabler ()->CreateInstance ();

    TPHInstance1->SetValue ("TPH", ECValue ("tph_string1"));
    TPHInstance2->SetValue ("TPH", ECValue ("tph_string2"));

    ECInstanceInserter inserter (ecdb, *TPH);
    ASSERT_TRUE (inserter.IsValid ());

    auto stat = inserter.Insert (*TPHInstance1, true);
    ASSERT_TRUE (stat == SUCCESS);
    stat = inserter.Insert (*TPHInstance2, true);
    ASSERT_TRUE (stat == SUCCESS);

    //Insert Instances for ClassA
    ECN::StandaloneECInstancePtr ClassAInstance1 = classA->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ECN::StandaloneECInstancePtr ClassAInstance2 = classA->GetDefaultStandaloneEnabler ()->CreateInstance ();

    ClassAInstance1->SetValue ("ClassA", ECValue ("ClassA_string1"));
    ClassAInstance2->SetValue ("ClassA", ECValue ("ClassA_string2"));

    ECInstanceInserter classAinserter (ecdb, *classA);
    ASSERT_TRUE (classAinserter.IsValid ());

    stat = classAinserter.Insert (*ClassAInstance1, true);
    ASSERT_TRUE (stat == SUCCESS);
    stat = classAinserter.Insert (*ClassAInstance2, true);
    ASSERT_TRUE (stat == SUCCESS);

    //Insert Instances for ClassB
    ECN::StandaloneECInstancePtr ClassBInstance1 = classB->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ECN::StandaloneECInstancePtr ClassBInstance2 = classB->GetDefaultStandaloneEnabler ()->CreateInstance ();

    ClassBInstance1->SetValue ("ClassB", ECValue ("ClassB_string1"));
    ClassBInstance2->SetValue ("ClassB", ECValue ("ClassB_string2"));

    ECInstanceInserter classBinserter (ecdb, *classB);
    ASSERT_TRUE (classBinserter.IsValid ());

    stat = classBinserter.Insert (*ClassBInstance1, true);
    ASSERT_TRUE (stat == SUCCESS);
    stat = classBinserter.Insert (*ClassBInstance2, true);
    ASSERT_TRUE (stat == SUCCESS);

    //Get Relationship Classes
    ECRelationshipClassCP TPHownsTPH = schema->GetClassCP ("TPHOwnsTPH")->GetRelationshipClassCP ();
    ASSERT_TRUE (TPHownsTPH != nullptr);
    ECRelationshipClassCP TPHhasClassA = schema->GetClassCP ("TPHhasClassA")->GetRelationshipClassCP ();
    ASSERT_TRUE (TPHhasClassA != nullptr);
    ECRelationshipClassCP TPHhasClassB = schema->GetClassCP ("TPHhasClassB")->GetRelationshipClassCP ();
    ASSERT_TRUE (TPHhasClassB != nullptr);

        {//Insert Instances for Relationship TPHOwnsTPH
        ECN::StandaloneECRelationshipInstancePtr relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*TPHownsTPH)->CreateRelationshipInstance ();
        ECInstanceInserter relationshipinserter (ecdb, *TPHownsTPH);
        ASSERT_TRUE (relationshipinserter.IsValid ());

            {//Inserting 1st Instance
            relationshipInstance->SetSource (TPHInstance1.get ());
            relationshipInstance->SetTarget (TPHInstance2.get ());
            relationshipInstance->SetInstanceId ("source->target");
            ASSERT_EQ (SUCCESS, relationshipinserter.Insert (*relationshipInstance));
            }
                {//Inserting 2nd Instance
                relationshipInstance->SetSource (TPHInstance2.get ());
                relationshipInstance->SetTarget (TPHInstance1.get ());
                relationshipInstance->SetInstanceId ("source->target");
                ASSERT_EQ (SUCCESS, relationshipinserter.Insert (*relationshipInstance));
                    }
        }

            {//Insert Instances for Relationship TPHhasClassA
            ECN::StandaloneECRelationshipInstancePtr relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*TPHhasClassA)->CreateRelationshipInstance ();
            ECInstanceInserter relationshipinserter (ecdb, *TPHhasClassA);
            ASSERT_TRUE (relationshipinserter.IsValid ());

                {//Inserting 1st Instance
                relationshipInstance->SetSource (TPHInstance1.get ());
                relationshipInstance->SetTarget (ClassAInstance1.get ());
                relationshipInstance->SetInstanceId ("source->target");
                ASSERT_EQ (SUCCESS, relationshipinserter.Insert (*relationshipInstance));
                }
                    {//Inserting 2nd Instance
                    relationshipInstance->SetSource (TPHInstance2.get ());
                    relationshipInstance->SetTarget (ClassAInstance2.get ());
                    relationshipInstance->SetInstanceId ("source->target");
                    ASSERT_EQ (SUCCESS, relationshipinserter.Insert (*relationshipInstance));
                        }
                }

                {//Insert Instances for Relationship TPHhasClassB
                ECN::StandaloneECRelationshipInstancePtr relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*TPHhasClassB)->CreateRelationshipInstance ();
                ECInstanceInserter relationshipinserter (ecdb, *TPHhasClassB);
                ASSERT_TRUE (relationshipinserter.IsValid ());

                    {//Inserting 1st Instance
                    relationshipInstance->SetSource (TPHInstance1.get ());
                    relationshipInstance->SetTarget (ClassBInstance1.get ());
                    relationshipInstance->SetInstanceId ("source->target");
                    ASSERT_EQ (SUCCESS, relationshipinserter.Insert (*relationshipInstance));
                    }
                        {//Inserting 2nd Instance
                        relationshipInstance->SetSource (TPHInstance2.get ());
                        relationshipInstance->SetTarget (ClassBInstance2.get ());
                        relationshipInstance->SetInstanceId ("source->target");
                        ASSERT_EQ (SUCCESS, relationshipinserter.Insert (*relationshipInstance));
                            }
            }
    ECSqlStatement stmt;
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT * FROM tph.TPH"));
    int rowCount = 0;
    while (stmt.Step () != ECSqlStepStatus::Done)
        {
        rowCount++;
        }
    ASSERT_EQ (rowCount, 6) << "Insert count doesn't match the No of rows returned";
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT * FROM tph.TPHOwnsTPH"));
    rowCount = 0;
    while (stmt.Step () != ECSqlStepStatus::Done)
        {
        rowCount++;
        }
    ASSERT_EQ (rowCount, 6) << "No of row returned doesn't match the no of Instances inserted for Relationship Classes";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
//Base class has mapping TablerPerHierarchy and Derived Classes also implement TablerPerHierarchy
TEST_F (ECDbMapCATests, TablePerHierarchy)
    {
    const int rowsPerInstances = 3;
    ECDbTestProject testproject;
    auto& ecdb = testproject.Create ("tableperhierarchy.ecdb", L"TablePerHierarchy.01.00.ecschema.xml", rowsPerInstances);

    ASSERT_TRUE (ecdb.TableExists ("tph_TPH"));
    ASSERT_FALSE (ecdb.TableExists ("tph_DerivesTablePerHierarchy"));
    ASSERT_FALSE (ecdb.TableExists ("tph_ClassA"));
    ASSERT_FALSE (ecdb.TableExists ("tph_DerivesTablePerHierarchy1"));
    ASSERT_FALSE (ecdb.TableExists ("tph_ClassB"));

    //Instance count should be equal to 15
    ECSqlStatement stmt;
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT * FROM tph.TPH"));
    size_t rowCount = 0;
    while (stmt.Step () != ECSqlStepStatus::Done)
        {
        rowCount++;
        }
    ASSERT_TRUE (rowCount == 15) << "No of Instances in the Base Class is not 15";
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT * FROM ONLY tph.DerivesTablePerHierarchy1"));
    rowCount = 0;
    while (stmt.Step () != ECSqlStepStatus::Done)
        {
        rowCount++;
        }
    ASSERT_TRUE (rowCount == 3) << "No of Instances in the Derived Class residing in the same base table is not 3";
    stmt.Finalize ();
    ecdb.CloseDb ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
//Base class has mapping TablerPerHierarchy and Derived Classes implement DoNotMap and DoNotMapHierarchy
TEST_F (ECDbMapCATests, TablePerHierarchy_DoNotMapHierarchy)
    {
    const int rowsPerInstances = 3;
    ECDbTestProject testproject;
    auto& ecdb = testproject.Create ("TablePerHierarchy_DoNotMapHierarchy.ecdb", L"TablePerHierarchy.01.00.ecschema.xml", rowsPerInstances);

    ASSERT_TRUE (ecdb.TableExists ("tph_TPH1"));
    EXPECT_TRUE (ecdb.TableExists ("tph_ClassD"));/*this class should be mapped along the base class*/
    ASSERT_FALSE (ecdb.TableExists ("tph_DerivesDoNotMapHierarchy"));
    ASSERT_FALSE (ecdb.TableExists ("tph_ClassC"));

    ECSqlStatement stmt;
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT * FROM ONLY tph.TPH1"));
    size_t rowCount = 0;
    while (stmt.Step () != ECSqlStepStatus::Done)
        {
        rowCount++;
        }
    ASSERT_TRUE (rowCount == 3);
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT * FROM ONLY tph.ClassD"));
    rowCount = 0;
    while (stmt.Step () != ECSqlStepStatus::Done)
        {
        rowCount++;
        }
    ASSERT_TRUE (rowCount == 3);
    stmt.Finalize ();

    ecdb.CloseDb ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
//Base class has mapping TablerPerHierarchy and Derived Classes implement TableForThisClass and TablePerClass
TEST_F (ECDbMapCATests, TablePerHierarchy_TablePerClass)
    {
    const int rowsPerInstances = 3;
    ECDbTestProject testproject;
    auto& ecdb = testproject.Create ("TablePerHierarchy_TablePerClass.ecdb", L"TablePerHierarchy.01.00.ecschema.xml", rowsPerInstances);

    ASSERT_TRUE (ecdb.TableExists ("tph_TPH2"));
    EXPECT_FALSE (ecdb.TableExists ("tph_DerivesTableForThisClass"));/*Seperate table should exist for this class*/
    EXPECT_FALSE (ecdb.TableExists ("ClassF"));/*should be mapped to base class*/
    EXPECT_FALSE (ecdb.TableExists ("tph_DerivesTablePerClass"));/*Seperate table should exist for this class*/
    EXPECT_FALSE (ecdb.TableExists ("tph_ClassE"));/*Seperate table should exist for this class*/

    ecdb.CloseDb ();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbMapCATests, RetrieveConstraintClassInstanceBeforeAfterInsertingRelationshipInstance)
    {
    ECDbTestProject testproject;
    ECDbR ecdb = testproject.Create ("RelationshipsWithTablePerHierarchy.ecdb", L"TablePerHierarchy.01.00.ecschema.xml", false);

    ASSERT_TRUE (ecdb.TableExists ("tph_TPHOwnsTPH"));
    ASSERT_FALSE (ecdb.TableExists ("tph_TPHhasClassA"));
    ASSERT_FALSE (ecdb.TableExists ("tph_TPHhasClassB"));

    ECSqlStatement insertStatement;
    ECInstanceKey TPHKey1;
    ECInstanceKey TPHKey2;
    ASSERT_EQ (ECSqlStatus::Success, insertStatement.Prepare (ecdb, "INSERT INTO tph.TPH (TPH) VALUES ('tph_string1')"));
    ASSERT_EQ (ECSqlStepStatus::Done, insertStatement.Step (TPHKey1));
    ASSERT_TRUE (TPHKey1.IsValid ());
    insertStatement.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, insertStatement.Prepare (ecdb, "INSERT INTO tph.TPH (TPH) VALUES ('tph_string2')"));
    ASSERT_EQ (ECSqlStepStatus::Done, insertStatement.Step (TPHKey2));
    ASSERT_TRUE (TPHKey2.IsValid ());
    insertStatement.Finalize ();

    ECInstanceKey classAKey1;
    ECInstanceKey classAKey2;
    ASSERT_EQ (ECSqlStatus::Success, insertStatement.Prepare (ecdb, "INSERT INTO tph.ClassA (ClassA) VALUES ('ClassA_string1')"));
    ASSERT_EQ (ECSqlStepStatus::Done, insertStatement.Step (classAKey1));
    ASSERT_TRUE (classAKey1.IsValid ());
    insertStatement.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, insertStatement.Prepare (ecdb, "INSERT INTO tph.ClassA (ClassA) VALUES ('ClassA_string2')"));
    ASSERT_EQ (ECSqlStepStatus::Done, insertStatement.Step (classAKey2));
    ASSERT_TRUE (classAKey2.IsValid ());
    insertStatement.Finalize ();

    //retrieve ECInstance from Db before inserting Relationship Instance, based on ECInstanceId, verify ECInstance is valid
    ECSqlStatement selectStmt;
    ASSERT_EQ (ECSqlStatus::Success, selectStmt.Prepare (ecdb, "SELECT * FROM tph.TPH WHERE ECInstanceId = ?"));
    selectStmt.BindId (1, TPHKey1.GetECInstanceId ());
    ASSERT_EQ (ECSqlStepStatus::HasRow, selectStmt.Step ());
    ECInstanceECSqlSelectAdapter TPHadapter (selectStmt);
    IECInstancePtr readInstance = TPHadapter.GetInstance ();
    ASSERT_TRUE (readInstance.IsValid ());
    selectStmt.Finalize ();

    ECSqlStatement relationStmt;
    ASSERT_EQ (relationStmt.Prepare (ecdb, "INSERT INTO tph.TPHhasClassA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    relationStmt.BindId (1, TPHKey1.GetECInstanceId ());
    relationStmt.BindInt64 (2, TPHKey1.GetECClassId ());
    relationStmt.BindId (3, classAKey1.GetECInstanceId ());
    relationStmt.BindInt64 (4, classAKey1.GetECClassId ());
    ASSERT_EQ (relationStmt.Step (), ECSqlStepStatus::Done);
    relationStmt.Finalize ();

    //try to insert Duplicate relationship step() should return error
    ASSERT_EQ (relationStmt.Prepare (ecdb, "INSERT INTO tph.TPHhasClassA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    relationStmt.BindId (1, TPHKey1.GetECInstanceId ());
    relationStmt.BindInt64 (2, TPHKey1.GetECClassId ());
    relationStmt.BindId (3, classAKey1.GetECInstanceId ());
    relationStmt.BindInt64 (4, classAKey1.GetECClassId ());
    ASSERT_EQ (relationStmt.Step (), ECSqlStepStatus::Error);
    relationStmt.Finalize ();

    //retrieve ECInstance from Db After Inserting Relationship Instance, based on ECInstanceId, verify ECInstance is valid
    ASSERT_EQ (ECSqlStatus::Success, selectStmt.Prepare (ecdb, "SELECT * FROM tph.ClassA WHERE ECInstanceId = ?"));
    selectStmt.BindId (1, classAKey1.GetECInstanceId ());
    ASSERT_EQ (ECSqlStepStatus::HasRow, selectStmt.Step ());
    ECInstanceECSqlSelectAdapter ClassAadapter (selectStmt);
    readInstance = ClassAadapter.GetInstance ();
    ASSERT_TRUE (readInstance.IsValid ());
    selectStmt.Finalize ();

    ecdb.CloseDb ();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMapCATests, EnforceLinkTableFor11Relationship)
    {
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create ("relationshipTestDb.ecdb", L"SampleDgnDbEditor.01.00.ecschema.xml", false);
    ECSchemaCP schema = ecdbr.Schemas ().GetECSchema ("SampleDgnDbEditor", true);
    ASSERT_TRUE (schema != nullptr);
    ASSERT_TRUE (ecdbr.TableExists ("sdde_ArchWithHVACStorey"));
    ASSERT_TRUE (ecdbr.TableExists ("sdde_ArchStoreyWithElements"));
    }
END_ECDBUNITTESTS_NAMESPACE