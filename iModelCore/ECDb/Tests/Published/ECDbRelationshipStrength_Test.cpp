/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbRelationshipStrength_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

struct RelationshipStrength : ECDbTestFixture {};

extern bool InsertInstance (ECDbR db, ECClassCR ecClass, IECInstanceR ecInstance);
extern IECRelationshipInstancePtr CreateRelationship (ECRelationshipClassCR relationshipClass, IECInstanceR source, IECInstanceR target);
extern ECInstanceId InstanceToId (IECInstanceCR ecInstance);
extern IECInstancePtr CreatePerson (ECClassCR ecClass, Utf8CP firstName, Utf8CP lastName);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DeleteInstance (IECInstanceCR instance, ECDbR ecdb)
    {
    ECClassCR ecClass = instance.GetClass ();
    ECInstanceDeleter deleter (ecdb, ecClass);

    ASSERT_TRUE (deleter.IsValid ()) << "Invaild Deleter for ecClass : " << ecClass.GetName ().c_str ();

    auto status = deleter.Delete (instance);
    ASSERT_TRUE (status == SUCCESS) << "Instance Deletion failed for ecClass : " << ecClass.GetName ().c_str ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool HasInstance (IECInstanceCR instance, ECDbR ecdb)
    {
    ECSqlSelectBuilder builder;
    ECClassCR ecClass = instance.GetClass ();
    Utf8PrintfString whereECInstanceId ("ECInstanceId = %lld", InstanceToId (instance).GetValue ());
    builder.From (ecClass, false).SelectAll ().Where (whereECInstanceId.c_str ());

    ECSqlStatement statement;
    auto stat = statement.Prepare (ecdb, builder.ToString ().c_str ());
    BeAssert (stat == ECSqlStatus::Success);

    return statement.Step () == BE_SQLITE_ROW;
    }

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/2014
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (RelationshipStrength, BackwardEmbedding)
    {
    ECDbR ecdb = SetupECDb ("BackwardRelationshipStrengthTest.ecdb", BeFileName (L"RelationshipStrengthTest.01.00.ecschema.xml"));
    /*
    *                                           SingleParent
    *                                                 |
    *                                                 | ChildrenHasSingleParent (Backward EMBEDDING)
    *         ________________________________________|______________________________________
    *        |                                        |                                      |
    *      Child1                                   Child2                                 Child3
    */
    ECClassCP personClass = ecdb.Schemas ().GetECClass ("RelationshipStrengthTest", "Person");
    IECInstancePtr child1 = CreatePerson (*personClass, "First", "Child");
    InsertInstance (ecdb, *personClass, *child1);
    IECInstancePtr child2 = CreatePerson (*personClass, "Second", "Child");
    InsertInstance (ecdb, *personClass, *child2);
    IECInstancePtr child3 = CreatePerson (*personClass, "Third", "Child");
    InsertInstance (ecdb, *personClass, *child3);
    IECInstancePtr singleParent = CreatePerson (*personClass, "Only", "singleParent");
    InsertInstance (ecdb, *personClass, *singleParent);

    //Backward Embedding relationship (SingleParent -> Child1, Child2)
    ECRelationshipClassCP ChildrenHasSingleParent = ecdb.Schemas ().GetECClass ("RelationshipStrengthTest", "ChildrenHasSingleParent")->GetRelationshipClassCP ();
    IECRelationshipInstancePtr child1HasSingleParent;
    child1HasSingleParent = CreateRelationship (*ChildrenHasSingleParent, *child1, *singleParent);
    InsertInstance (ecdb, *ChildrenHasSingleParent, *child1HasSingleParent);
    IECRelationshipInstancePtr child2HasSingleParent;
    child2HasSingleParent = CreateRelationship (*ChildrenHasSingleParent, *child2, *singleParent);
    InsertInstance (ecdb, *ChildrenHasSingleParent, *child2HasSingleParent);
    IECRelationshipInstancePtr child3HasSingleParent;
    child3HasSingleParent = CreateRelationship (*ChildrenHasSingleParent, *child3, *singleParent);
    InsertInstance (ecdb, *ChildrenHasSingleParent, *child3HasSingleParent);

    /*
    * Test 1: Delete Child1
    * Validate child1HasSingleParent,  child1 have been deleted
    * Validate singleParent, child2HasSingleParent, child3HasSingleParent, child2, child3 are still there
    */
    DeleteInstance (*child1, ecdb);
    ecdb.SaveChanges ();

    ASSERT_FALSE (HasInstance (*child1, ecdb));
    ASSERT_FALSE (HasInstance (*child1HasSingleParent, ecdb));

    ASSERT_TRUE (HasInstance (*singleParent, ecdb));
    ASSERT_TRUE (HasInstance (*child2HasSingleParent, ecdb));
    ASSERT_TRUE (HasInstance (*child3HasSingleParent, ecdb));
    ASSERT_TRUE (HasInstance (*child2, ecdb));
    ASSERT_TRUE (HasInstance (*child3, ecdb));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RelationshipStrength, RelationshipTest)
    {
    ECDbR ecdb = SetupECDb ("RelationshipStrengthTest.ecdb", BeFileName (L"RelationshipStrengthTest.01.00.ecschema.xml"));

    /*
     *          Create the following relationship hierarchy
     *
     * GrandParent1  <- ParentHasSpouse (REFERENCING) -> GrandParent2
     *     |__________________________________________________|
     *                             |
     *                             | ManyParentsHaveChildren (HOLDING)
     *                             |
     *                         SingleParent
     *                             |
     *                             | SingleParentHasChildren (EMBEDDING)
     *      _______________________|__________________________
     *     |                                                  |
     *   Child1                                             Child2
     *
     */

    ECClassCP personClass = ecdb.Schemas ().GetECClass ("RelationshipStrengthTest", "Person");
    IECInstancePtr grandParent1 = CreatePerson (*personClass, "First", "GrandParent");
    InsertInstance (ecdb, *personClass, *grandParent1);
    IECInstancePtr grandParent2 = CreatePerson (*personClass, "Second", "GrandParent");
    InsertInstance (ecdb, *personClass, *grandParent2);
    IECInstancePtr singleParent = CreatePerson (*personClass, "Only", "SingleParent");
    InsertInstance (ecdb, *personClass, *singleParent);
    IECInstancePtr child1 = CreatePerson (*personClass, "First", "Child");
    InsertInstance (ecdb, *personClass, *child1);
    IECInstancePtr child2 = CreatePerson (*personClass, "Second", "Child");
    InsertInstance (ecdb, *personClass, *child2);

    // Holding relationship (GrandParent1, GrandParent2 -> SingleParent)
    ECRelationshipClassCP manyParentsHaveChildren = ecdb.Schemas ().GetECClass ("RelationshipStrengthTest", "ManyParentsHaveChildren")->GetRelationshipClassCP ();
    IECRelationshipInstancePtr grandParent1HasSingleParent;
    grandParent1HasSingleParent = CreateRelationship (*manyParentsHaveChildren, *grandParent1, *singleParent);
    InsertInstance (ecdb, *manyParentsHaveChildren, *grandParent1HasSingleParent);
    IECRelationshipInstancePtr grandParent2HasSingleParent;
    grandParent2HasSingleParent = CreateRelationship (*manyParentsHaveChildren, *grandParent2, *singleParent);
    InsertInstance (ecdb, *manyParentsHaveChildren, *grandParent2HasSingleParent);

    // Embedding relationship (SingleParent -> Child1, Child2)
    ECRelationshipClassCP singleParentHasChildren = ecdb.Schemas ().GetECClass ("RelationshipStrengthTest", "SingleParentHasChildren")->GetRelationshipClassCP ();
    IECRelationshipInstancePtr singleParentHasChild1;
    singleParentHasChild1 = CreateRelationship (*singleParentHasChildren, *singleParent, *child1);
    InsertInstance (ecdb, *singleParentHasChildren, *singleParentHasChild1);
    IECRelationshipInstancePtr singleParentHasChild2;
    singleParentHasChild2 = CreateRelationship (*singleParentHasChildren, *singleParent, *child2);
    InsertInstance (ecdb, *singleParentHasChildren, *singleParentHasChild2);

    // Referencing relationship (GrandParent1 <-> GrandParent2)
    ECRelationshipClassCP parentHasSpouse = ecdb.Schemas ().GetECClass ("RelationshipStrengthTest", "ParentHasSpouse")->GetRelationshipClassCP ();
    IECRelationshipInstancePtr grandParent1HasSpouse;
    grandParent1HasSpouse = CreateRelationship (*parentHasSpouse, *grandParent1, *grandParent2);
    InsertInstance (ecdb, *parentHasSpouse, *grandParent1HasSpouse);
    IECRelationshipInstancePtr grandParent2HasSpouse;
    grandParent2HasSpouse = CreateRelationship (*parentHasSpouse, *grandParent2, *grandParent1);
    InsertInstance (ecdb, *parentHasSpouse, *grandParent2HasSpouse);

    ecdb.SaveChanges ();

    //Verify instances before deletion
    ASSERT_TRUE (HasInstance (*grandParent1HasSpouse, ecdb));
    ASSERT_TRUE (HasInstance (*grandParent2HasSpouse, ecdb));

    /*
    * Test 1: Delete GrandParent1
    * Validate grandParent1HasSpouse, grandParent2HasSpouse, grandParent1HasSingleParent have been deleted (orphaned relationships)
    * Validate singleParent is still around (holding relationship with one parent remaining)
    */
    DeleteInstance (*grandParent1, ecdb);
    ecdb.SaveChanges ();

    ASSERT_FALSE (HasInstance (*grandParent1, ecdb));
    ASSERT_FALSE (HasInstance (*grandParent1HasSpouse, ecdb));
    ASSERT_FALSE (HasInstance (*grandParent2HasSpouse, ecdb));
    ASSERT_FALSE (HasInstance (*grandParent1HasSingleParent, ecdb));

    ASSERT_TRUE (HasInstance (*singleParent, ecdb));

    /*
    * Test 2: Delete GrandParent2
    * Validate grandParent2HasSingleParent has been deleted (orphaned relationship), *Validate singeParent has been deleted (held instance with no parents remaining)
    */
    DeleteInstance (*grandParent2, ecdb);
    ecdb.SaveChanges ();

    ASSERT_FALSE (HasInstance (*grandParent2HasSingleParent, ecdb));
    ASSERT_FALSE (HasInstance (*grandParent2, ecdb));
    ASSERT_FALSE (HasInstance (*singleParent, ecdb));
    ASSERT_FALSE (HasInstance (*singleParentHasChild1, ecdb));
    ASSERT_FALSE (HasInstance (*singleParentHasChild2, ecdb));
    ASSERT_FALSE (HasInstance (*child1, ecdb));
    ASSERT_FALSE (HasInstance (*child2, ecdb));
    }

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/2014
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (RelationshipStrength, BackwardHoldingForwardEmbedding)
    {
    ECDbR ecdb = SetupECDb ("BackwardRelationshipStrengthTest.ecdb", BeFileName (L"RelationshipStrengthTest.01.00.ecschema.xml"));

    /*
    *          Create the following relationship hierarchy
    *
    * GrandParent1  <- ParentHasSpouse (REFERENCING) -> GrandParent2
    *     |__________________________________________________|
    *                             |
    *                             | ChildrenHaveManyParents.( Backward HOLDING)
    *                             |
    *                         SingleParent
    *                             |
    *                             | SingleParentHasChildren.( Forward EMBEDDING)
    *      _______________________|__________________________
    *     |                                                  |
    *   Child1                                             Child2
    *
    */
    ECClassCP personClass = ecdb.Schemas ().GetECClass ("RelationshipStrengthTest", "Person");
    IECInstancePtr child1 = CreatePerson (*personClass, "First", "Child");
    InsertInstance (ecdb, *personClass, *child1);
    IECInstancePtr child2 = CreatePerson (*personClass, "Second", "Child");
    InsertInstance (ecdb, *personClass, *child2);
    IECInstancePtr singleParent = CreatePerson (*personClass, "Only", "singleParent");
    InsertInstance (ecdb, *personClass, *singleParent);
    IECInstancePtr grandParent1 = CreatePerson (*personClass, "First", "GrandParent");
    InsertInstance (ecdb, *personClass, *grandParent1);
    IECInstancePtr grandParent2 = CreatePerson (*personClass, "Second", "GrandParent");
    InsertInstance (ecdb, *personClass, *grandParent2);

    // Backward Holding relationship (GrandParent1, GrandParent2 <- SingleParent)
    ECRelationshipClassCP ChildrenHaveManyParents = ecdb.Schemas ().GetECClass ("RelationshipStrengthTest", "ChildrenHaveManyParents")->GetRelationshipClassCP ();
    IECRelationshipInstancePtr singleParentHasGrandParent1;
    singleParentHasGrandParent1 = CreateRelationship (*ChildrenHaveManyParents, *singleParent, *grandParent1);
    InsertInstance (ecdb, *ChildrenHaveManyParents, *singleParentHasGrandParent1);
    IECRelationshipInstancePtr singleParentHasGrandParent2;
    singleParentHasGrandParent2 = CreateRelationship (*ChildrenHaveManyParents, *singleParent, *grandParent2);
    InsertInstance (ecdb, *ChildrenHaveManyParents, *singleParentHasGrandParent2);

    //Forward Embedding relationship (SingleParent -> Child1, Child2)
    ECRelationshipClassCP SingleParentHasChildren = ecdb.Schemas ().GetECClass ("RelationshipStrengthTest", "SingleParentHasChildren")->GetRelationshipClassCP ();
    IECRelationshipInstancePtr singleParentHasChild1;
    singleParentHasChild1 = CreateRelationship (*SingleParentHasChildren, *singleParent, *child1);
    InsertInstance (ecdb, *SingleParentHasChildren, *singleParentHasChild1);
    IECRelationshipInstancePtr singleParentHasChild2;
    singleParentHasChild2 = CreateRelationship (*SingleParentHasChildren, *singleParent, *child2);
    InsertInstance (ecdb, *SingleParentHasChildren, *singleParentHasChild2);

    //Backward Referencing relationship (GrandParent1 <-> GrandParent2)
    ECRelationshipClassCP parentHasSpouse = ecdb.Schemas ().GetECClass ("RelationshipStrengthTest", "ParentHasSpouse_backward")->GetRelationshipClassCP ();
    IECRelationshipInstancePtr grandParent1HasSpouse;
    grandParent1HasSpouse = CreateRelationship (*parentHasSpouse, *grandParent1, *grandParent2);
    InsertInstance (ecdb, *parentHasSpouse, *grandParent1HasSpouse);
    IECRelationshipInstancePtr grandParent2HasSpouse;
    grandParent2HasSpouse = CreateRelationship (*parentHasSpouse, *grandParent2, *grandParent1);
    InsertInstance (ecdb, *parentHasSpouse, *grandParent2HasSpouse);

    ecdb.SaveChanges ();

    //Validate Instance exists before deletion
    ASSERT_TRUE (HasInstance (*singleParentHasChild1, ecdb));

    /*
    * Test 1: Delete Child1
    * Validate Child1 and singleParentHasChild1 have been deleted
    * Validate Child2 is Still there
    */
    DeleteInstance (*child1, ecdb);
    ecdb.SaveChanges ();

    ASSERT_FALSE (HasInstance (*child1, ecdb));
    ASSERT_FALSE (HasInstance (*singleParentHasChild1, ecdb));

    ASSERT_TRUE (HasInstance (*child2, ecdb));

    /*
    * Test 2: Delete Child2
    * Validate Child2 and singleParentHasChild2 have been deleted
    * Validate singleParent is still around (relationship grand parents remaining)
    */
    DeleteInstance (*child2, ecdb);
    ecdb.SaveChanges ();

    ASSERT_FALSE (HasInstance (*child2, ecdb));
    ASSERT_FALSE (HasInstance (*singleParentHasChild2, ecdb));

    ASSERT_TRUE (HasInstance (*singleParent, ecdb));

    /*
    * Test 3: Delete GrandParent1
    * Validate GrandParent1, grandParent1HasSpouse, grandParent2HasSpouse, singleParentHasGrandParent1 have been deleted
    * Validate singleParent is still around (holding relationship with one parent remaining)
    */
    DeleteInstance (*grandParent1, ecdb);
    ecdb.SaveChanges ();

    ASSERT_FALSE (HasInstance (*grandParent1, ecdb));
    ASSERT_FALSE (HasInstance (*grandParent1HasSpouse, ecdb));
    ASSERT_FALSE (HasInstance (*grandParent2HasSpouse, ecdb));
    ASSERT_FALSE (HasInstance (*singleParentHasGrandParent1, ecdb));

    ASSERT_TRUE (HasInstance (*singleParent, ecdb));

    /*
    * Test 4: Delete GrandParent2
    * Validate GrandParent2, singleParentHasGrandParent2 have been deleted, * Single parent has been deleted too as no parent exists anymore
    */
    DeleteInstance (*grandParent2, ecdb);
    ecdb.SaveChanges ();

    ASSERT_FALSE (HasInstance (*grandParent2, ecdb));
    ASSERT_FALSE (HasInstance (*singleParentHasGrandParent2, ecdb));
    ASSERT_FALSE (HasInstance (*singleParent, ecdb));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                       Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
struct ECDbRelationshipsIntegrityTests : ECDbMappingTestFixture
    {
    enum class Cardinality
        {
        ZeroOne,
        ZeroMany,
        OneOne,
        OneMany
        };

    enum class Direction
        {
        Forward,
        Backward
        };

    private:
        ECEntityClassP GetEntityClass (Utf8CP className);

        RelationshipCardinalityCR GetClassCardinality (Cardinality classCardinality);

        ECRelatedInstanceDirection GetRelationDirection (Direction direction);

    protected:
        ECSchemaPtr testSchema = nullptr;
        ECSchemaReadContextPtr readContext = nullptr;

        void CreateSchema (Utf8CP schemaName, Utf8CP schemaNamePrefix);

        //Adding a Class automatically adds a Property of Type string with Name "SqlPrintfString ("%sProp", className)" to the class.
        void AddEntityClass (Utf8CP className);

        void AddRelationShipClass (Cardinality SourceClassCardinality, Cardinality targetClassCardinality, StrengthType strengthType, Direction direction, Utf8CP relationshipClassName, Utf8CP sourceClass, Utf8CP targetClass, bool isPolymorphic);

        void AssertSchemaImport (bool isSchemaImportExpectedToSucceed);

        void InsertEntityClassInstances (Utf8CP className, Utf8CP propName, int numberOfInstances, std::vector<ECInstanceKey>& classKeys);

        void InsertRelationshipInstances (Utf8CP relationshipClass, std::vector<ECInstanceKey> const& sourceKeys, std::vector<ECInstanceKey>const& targetKeys, std::vector<DbResult> const& expected, size_t& rowInserted) const;

        size_t GetInsertedRelationshipsCount (Utf8CP relationshipClass) const;

        ECClassId GetRelationShipClassId (Utf8CP className);
    };

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
ECEntityClassP ECDbRelationshipsIntegrityTests::GetEntityClass (Utf8CP className)
    {
    return testSchema->GetClassP (className)->GetEntityClassP ();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
RelationshipCardinalityCR ECDbRelationshipsIntegrityTests::GetClassCardinality (Cardinality classCardinality)
    {
    if (classCardinality == Cardinality::ZeroOne)
        return RelationshipCardinality::ZeroOne ();
    else if (classCardinality == Cardinality::ZeroMany)
        return RelationshipCardinality::ZeroMany ();
    else if (classCardinality == Cardinality::OneOne)
        return RelationshipCardinality::OneOne ();
    else
        return RelationshipCardinality::OneMany ();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
ECRelatedInstanceDirection ECDbRelationshipsIntegrityTests::GetRelationDirection (Direction direction)
    {
    if (direction == Direction::Forward)
        return ECRelatedInstanceDirection::Forward;
    else
        return ECRelatedInstanceDirection::Backward;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::CreateSchema (Utf8CP schemaName, Utf8CP schemaNamePrefix)
    {
    readContext = ECSchemaReadContext::CreateContext ();
    readContext->AddSchemaLocater (m_ecdb.GetSchemaLocater ());
    SchemaKey ecdbmapKey = SchemaKey ("ECDbMap", 1, 0);
    ECSchemaPtr ecdbmapSchema = readContext->LocateSchema (ecdbmapKey, SchemaMatchType::SCHEMAMATCHTYPE_LatestCompatible);
    ASSERT_TRUE (ecdbmapSchema.IsValid ());

    ECSchema::CreateSchema (testSchema, schemaName, 1, 0);
    ASSERT_TRUE (testSchema.IsValid ());

    testSchema->SetNamespacePrefix (schemaNamePrefix);
    testSchema->AddReferencedSchema (*ecdbmapSchema);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::AssertSchemaImport (bool isSchemaImportExpectedToSucceed)
    {
    EXPECT_EQ (ECObjectsStatus::Success, readContext->AddSchema (*testSchema));
    if (isSchemaImportExpectedToSucceed)
        {
        EXPECT_EQ (SUCCESS, m_ecdb.Schemas ().ImportECSchemas (readContext->GetCache ()));
        }
    else
        {
        EXPECT_EQ (ERROR, m_ecdb.Schemas ().ImportECSchemas (readContext->GetCache ()));
        }
    m_ecdb.SaveChanges ();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::AddEntityClass (Utf8CP className)
    {
    ECEntityClassP testClass = nullptr;
    EXPECT_EQ (ECObjectsStatus::Success, testSchema->CreateEntityClass (testClass, className));
    PrimitiveECPropertyP prim = nullptr;
    EXPECT_EQ (ECObjectsStatus::Success, testClass->CreatePrimitiveProperty (prim, SqlPrintfString ("%sProp", className).GetUtf8CP ()));
    EXPECT_EQ (ECObjectsStatus::Success, prim->SetType (PrimitiveType::PRIMITIVETYPE_String));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::AddRelationShipClass (Cardinality SourceClassCardinality, Cardinality targetClassCardinality, StrengthType strengthType, Direction direction, Utf8CP relationshipClassName, Utf8CP sourceClass, Utf8CP targetClass, bool isPolymorphic)
    {
    ECRelationshipClassP testRelationshipClass = nullptr;
    EXPECT_EQ (ECObjectsStatus::Success, testSchema->CreateRelationshipClass (testRelationshipClass, relationshipClassName));
    EXPECT_EQ (ECObjectsStatus::Success, testRelationshipClass->SetStrength (strengthType));
    EXPECT_EQ (ECObjectsStatus::Success, testRelationshipClass->SetStrengthDirection (GetRelationDirection (direction)));

    //Set Relstionship Source Class and Cardinality
    EXPECT_EQ (ECObjectsStatus::Success, testRelationshipClass->GetSource ().AddClass (*GetEntityClass (sourceClass)));
    EXPECT_EQ (ECObjectsStatus::Success, testRelationshipClass->GetSource ().SetIsPolymorphic (true));
    EXPECT_EQ (ECObjectsStatus::Success, testRelationshipClass->GetSource ().SetCardinality (GetClassCardinality (SourceClassCardinality)));

    //Set Relstionship Target Class and Cardinality
    EXPECT_EQ (ECObjectsStatus::Success, testRelationshipClass->GetTarget ().AddClass (*GetEntityClass (targetClass)));
    EXPECT_EQ (ECObjectsStatus::Success, testRelationshipClass->GetTarget ().SetIsPolymorphic (true));
    EXPECT_EQ (ECObjectsStatus::Success, testRelationshipClass->GetTarget ().SetCardinality (GetClassCardinality (targetClassCardinality)));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::InsertEntityClassInstances (Utf8CP className, Utf8CP propName, int numberOfInstances, std::vector<ECInstanceKey>& classKeys)
    {
    ECSqlStatement stmt;

    SqlPrintfString insertECSql = SqlPrintfString ("INSERT INTO %s(%s) VALUES(?)", GetEntityClass (className)->GetECSqlName ().c_str (), propName);

    ASSERT_EQ (stmt.Prepare (m_ecdb, insertECSql.GetUtf8CP ()), ECSqlStatus::Success);
    for (int i = 0; i < numberOfInstances; i++)
        {
        ECInstanceKey key;
        ASSERT_EQ (stmt.Reset (), ECSqlStatus::Success);
        ASSERT_EQ (stmt.ClearBindings (), ECSqlStatus::Success);
        ASSERT_EQ (stmt.BindText (1, SqlPrintfString ("%s_%d", className, i).GetUtf8CP (), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ (stmt.Step (key), BE_SQLITE_DONE);
        classKeys.push_back (key);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::InsertRelationshipInstances (Utf8CP relationshipClass, std::vector<ECInstanceKey> const& sourceKeys, std::vector<ECInstanceKey>const& targetKeys, std::vector<DbResult> const& expected, size_t& rowInserted) const
    {
    ECSqlStatement stmt;
    SqlPrintfString sql = SqlPrintfString ("INSERT INTO %s (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(?,?,?,?)", relationshipClass);

    ASSERT_EQ (stmt.Prepare (m_ecdb, sql.GetUtf8CP ()), ECSqlStatus::Success);
    ASSERT_EQ (expected.size (), sourceKeys.size () * targetKeys.size ());
    int n = 0;
    for (auto& sourceKey : sourceKeys)
        {
        for (auto& targetKey : targetKeys)
            {
            stmt.Reset ();
            ASSERT_EQ (ECSqlStatus::Success, stmt.ClearBindings ());
            stmt.BindId (1, sourceKey.GetECInstanceId ());
            stmt.BindInt64 (2, sourceKey.GetECClassId ());
            stmt.BindId (3, targetKey.GetECInstanceId ());
            stmt.BindInt64 (4, targetKey.GetECClassId ());

            if (expected[n] != BE_SQLITE_DONE)
                ASSERT_NE (BE_SQLITE_DONE, stmt.Step ());
            else
                {
                ASSERT_EQ (BE_SQLITE_DONE, stmt.Step ());
                rowInserted++;
                }

            n = n + 1;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
size_t ECDbRelationshipsIntegrityTests::GetInsertedRelationshipsCount (Utf8CP relationshipClass) const
    {
    ECSqlStatement stmt;
    auto sql = SqlPrintfString ("SELECT COUNT(*) FROM ONLY ts.Foo JOIN ts.Goo USING %s", relationshipClass);
    if (stmt.Prepare (m_ecdb, sql.GetUtf8CP ()) == ECSqlStatus::Success)
        {
        if (stmt.Step () == BE_SQLITE_ROW)
            return static_cast<size_t>(stmt.GetValueInt (0));
        }

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
ECClassId ECDbRelationshipsIntegrityTests::GetRelationShipClassId (Utf8CP className)
    {
    return testSchema->GetClassP (className)->GetEntityClassP ()->GetId();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbRelationshipsIntegrityTests, ForwardEmbeddingRelationshipsTest)
    {
    SetupECDb ("forwardEmbeddingRelationshipsTest.ecdb");
    CreateSchema ("testSchema", "ts");
    AddEntityClass ("Foo");
    AddEntityClass ("Goo");
    AddRelationShipClass (Cardinality::ZeroOne, Cardinality::OneOne, StrengthType::Embedding, Direction::Forward, "FooOwnsGoo", "Foo", "Goo", true);
    AddRelationShipClass (Cardinality::ZeroOne, Cardinality::OneMany, StrengthType::Embedding, Direction::Forward, "FooOwnsManyGoo", "Foo", "Goo", true);
    AddRelationShipClass (Cardinality::ZeroMany, Cardinality::OneMany, StrengthType::Embedding, Direction::Forward, "ManyFooOwnManyGoo", "Foo", "Goo", true);
    AssertSchemaImport (true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances ("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances ("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> FooOwnsGooResult;
    std::vector<DbResult> FooOwnsManyGooResult;
    std::vector<DbResult> ManyFooOwnManyGooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(0,1), Target(1,1)
            if (f == g)
                FooOwnsGooResult.push_back (BE_SQLITE_DONE);
            else
                FooOwnsGooResult.push_back (BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(0,1), Target(1,N)
            if (f == 0)
                FooOwnsManyGooResult.push_back (BE_SQLITE_DONE);
            else
                FooOwnsManyGooResult.push_back (BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(0,N), Target(1,N)
            ManyFooOwnManyGooResult.push_back (BE_SQLITE_DONE);
            }
        }

    //1-1............................
    PersistedMapStrategy mapStrategy;
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("FooOwnsGoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    size_t count_FooOwnsGoo = 0;
    InsertRelationshipInstances ("ts.FooOwnsGoo", fooKeys, gooKeys, FooOwnsGooResult, count_FooOwnsGoo);
    ASSERT_EQ (count_FooOwnsGoo, GetInsertedRelationshipsCount ("ts.FooOwnsGoo"));
    
    //1-N............................
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("FooOwnsManyGoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    size_t count_FooOwnsManyGoo = 0;
    InsertRelationshipInstances ("ts.FooOwnsManyGoo", fooKeys, gooKeys, FooOwnsManyGooResult, count_FooOwnsManyGoo);
    ASSERT_EQ (count_FooOwnsManyGoo, GetInsertedRelationshipsCount ("ts.FooOwnsManyGoo"));

    //N-N............................
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("ManyFooOwnManyGoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::OwnTable, mapStrategy.m_strategy);

    size_t count_ManyGooOwnManyGoo = 0;
    InsertRelationshipInstances ("ts.ManyFooOwnManyGoo", fooKeys, gooKeys, ManyFooOwnManyGooResult, count_ManyGooOwnManyGoo);
    ASSERT_EQ (count_ManyGooOwnManyGoo, GetInsertedRelationshipsCount ("ts.ManyFooOwnManyGoo"));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbRelationshipsIntegrityTests, BackwardEmbeddingRelationshipsTest)
    {
    SetupECDb ("backwardEmbeddingRelationshipTest.ecdb");
    CreateSchema ("testSchema", "ts");
    AddEntityClass ("Foo");
    AddEntityClass ("Goo");
    AddRelationShipClass (Cardinality::OneOne, Cardinality::ZeroOne, StrengthType::Embedding, Direction::Backward, "GooOwnsFoo", "Foo", "Goo", true);
    AddRelationShipClass (Cardinality::OneMany, Cardinality::ZeroOne, StrengthType::Embedding, Direction::Backward, "GooOwnsManyFoo", "Foo", "Goo", true);
    AddRelationShipClass (Cardinality::OneMany, Cardinality::ZeroMany, StrengthType::Embedding, Direction::Backward, "ManyGooOwnManyFoo", "Foo", "Goo", true);
    AssertSchemaImport (true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances ("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances ("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> GooOwnsFooResult;
    std::vector<DbResult> GooOwnsManyFooResult;
    std::vector<DbResult> ManyGooOwnManyFooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(1,1), Target(0,1)
            if (f == g)
                GooOwnsFooResult.push_back (BE_SQLITE_DONE);
            else
                GooOwnsFooResult.push_back (BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(1,N), Target(0,1)
            if (g == 0)
                GooOwnsManyFooResult.push_back (BE_SQLITE_DONE);
            else
                GooOwnsManyFooResult.push_back (BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(1,N), Target(0,N)
            ManyGooOwnManyFooResult.push_back (BE_SQLITE_DONE);
            }
        }

    //1-1...........................
    PersistedMapStrategy mapStrategy;
#ifdef TFS361480
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("GooOwnsFoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);
#endif

    size_t count_FooOwnsGoo = 0;
    InsertRelationshipInstances ("ts.GooOwnsFoo", fooKeys, gooKeys, GooOwnsFooResult, count_FooOwnsGoo);
    ASSERT_EQ (count_FooOwnsGoo, GetInsertedRelationshipsCount ("ts.GooOwnsFoo"));

    //1-N..........................
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("GooOwnsManyFoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);

    size_t count_FooOwnsManyGoo = 0;
    InsertRelationshipInstances ("ts.GooOwnsManyFoo", fooKeys, gooKeys, GooOwnsManyFooResult, count_FooOwnsManyGoo);
    ASSERT_EQ (count_FooOwnsManyGoo, GetInsertedRelationshipsCount ("ts.GooOwnsManyFoo"));

    //N-N..........................
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("ManyGooOwnManyFoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::OwnTable, mapStrategy.m_strategy);

    size_t count_ManyGooOwnManyGoo = 0;
    InsertRelationshipInstances ("ts.ManyGooOwnManyFoo", fooKeys, gooKeys, ManyGooOwnManyFooResult, count_ManyGooOwnManyGoo);
    ASSERT_EQ (count_ManyGooOwnManyGoo, GetInsertedRelationshipsCount ("ts.ManyGooOwnManyFoo"));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbRelationshipsIntegrityTests, ForwardReferencingRelationshipsTest)
    {
    SetupECDb ("forwardReferencingRelationshipTest.ecdb");
    CreateSchema ("testSchema", "ts");
    AddEntityClass ("Foo");
    AddEntityClass ("Goo");
    AddRelationShipClass (Cardinality::ZeroOne, Cardinality::ZeroOne, StrengthType::Referencing, Direction::Forward, "FooHasGoo", "Foo", "Goo", true);
    AddRelationShipClass (Cardinality::ZeroOne, Cardinality::ZeroMany, StrengthType::Referencing, Direction::Forward, "FooHasManyGoo", "Foo", "Goo", true);
    AddRelationShipClass (Cardinality::ZeroMany, Cardinality::ZeroMany, StrengthType::Referencing, Direction::Forward, "ManyFoohaveManyGoo", "Foo", "Goo", true);
    AssertSchemaImport (true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances ("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances ("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> FooOwnsGooResult;
    std::vector<DbResult> FooOwnsManyGooResult;
    std::vector<DbResult> ManyFooOwnManyGooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(0,1), Target(0,1)
            if (f == g)
                FooOwnsGooResult.push_back (BE_SQLITE_DONE);
            else
                FooOwnsGooResult.push_back (BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(0,1), Target(0,N)
            if (f == 0)
                FooOwnsManyGooResult.push_back (BE_SQLITE_DONE);
            else
                FooOwnsManyGooResult.push_back (BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(0,N), Target(0,N)
            ManyFooOwnManyGooResult.push_back (BE_SQLITE_DONE);
            }
        }

    //1-1............................
    PersistedMapStrategy mapStrategy;
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("FooHasGoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    size_t count_FooOwnsGoo = 0;
    InsertRelationshipInstances ("ts.FooHasGoo", fooKeys, gooKeys, FooOwnsGooResult, count_FooOwnsGoo);
    ASSERT_EQ (count_FooOwnsGoo, GetInsertedRelationshipsCount ("ts.FooHasGoo"));

    //1-N...........................
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("FooHasManyGoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    size_t count_FooOwnsManyGoo = 0;
    InsertRelationshipInstances ("ts.FooHasManyGoo", fooKeys, gooKeys, FooOwnsManyGooResult, count_FooOwnsManyGoo);
    ASSERT_EQ (count_FooOwnsManyGoo, GetInsertedRelationshipsCount ("ts.FooHasManyGoo"));

    //N-N...........................
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("ManyFoohaveManyGoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::OwnTable, mapStrategy.m_strategy);

    size_t count_ManyGooOwnManyGoo = 0;
    InsertRelationshipInstances ("ts.ManyFoohaveManyGoo", fooKeys, gooKeys, ManyFooOwnManyGooResult, count_ManyGooOwnManyGoo);
    ASSERT_EQ (count_ManyGooOwnManyGoo, GetInsertedRelationshipsCount ("ts.ManyFoohaveManyGoo"));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbRelationshipsIntegrityTests, BackwardReferencingRelationshipsTest)
    {
    SetupECDb ("backwardReferencingRelationshipsTest.ecdb");
    CreateSchema ("testSchema", "ts");
    AddEntityClass ("Foo");
    AddEntityClass ("Goo");
    AddRelationShipClass (Cardinality::ZeroOne, Cardinality::ZeroOne, StrengthType::Referencing, Direction::Backward, "GooHasFoo", "Foo", "Goo", true);
    AddRelationShipClass (Cardinality::ZeroMany, Cardinality::ZeroOne, StrengthType::Referencing, Direction::Backward, "GooHasManyFoo", "Foo", "Goo", true);
    AddRelationShipClass (Cardinality::ZeroMany, Cardinality::ZeroMany, StrengthType::Referencing, Direction::Backward, "ManyGooHaveManyFoo", "Foo", "Goo", true);
    AssertSchemaImport (true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances ("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances ("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> GooOwnsFooResult;
    std::vector<DbResult> GooOwnsManyFooResult;
    std::vector<DbResult> ManyGooOwnManyFooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(0,1), Target(0,1)
            if (f == g)
                GooOwnsFooResult.push_back (BE_SQLITE_DONE);
            else
                GooOwnsFooResult.push_back (BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(0,N), Target(0,1)
            if (g == 0)
                GooOwnsManyFooResult.push_back (BE_SQLITE_DONE);
            else
                GooOwnsManyFooResult.push_back (BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(0,N), Target(0,N)
            ManyGooOwnManyFooResult.push_back (BE_SQLITE_DONE);
            }
        }

    //1-1............................
    PersistedMapStrategy mapStrategy;
#ifdef TFS361480
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("GooHasFoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);
#endif

    size_t count_FooOwnsGoo = 0;
    InsertRelationshipInstances ("ts.GooHasFoo", fooKeys, gooKeys, GooOwnsFooResult, count_FooOwnsGoo);
    ASSERT_EQ (count_FooOwnsGoo, GetInsertedRelationshipsCount ("ts.GooHasFoo"));

    //1-N...........................
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("GooHasManyFoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);

    size_t count_FooOwnsManyGoo = 0;
    InsertRelationshipInstances ("ts.GooHasManyFoo", fooKeys, gooKeys, GooOwnsManyFooResult, count_FooOwnsManyGoo);
    ASSERT_EQ (count_FooOwnsManyGoo, GetInsertedRelationshipsCount ("ts.GooHasManyFoo"));

    //N-N...........................
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("ManyGooHaveManyFoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::OwnTable, mapStrategy.m_strategy);

    size_t count_ManyGooOwnManyGoo = 0;
    InsertRelationshipInstances ("ts.ManyGooHaveManyFoo", fooKeys, gooKeys, ManyGooOwnManyFooResult, count_ManyGooOwnManyGoo);
    ASSERT_EQ (count_ManyGooOwnManyGoo, GetInsertedRelationshipsCount ("ts.ManyGooHaveManyFoo"));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbRelationshipsIntegrityTests, ForwardHoldingRelationshipsTest)
    {
    SetupECDb ("forwardHoldingRelationshipsTest.ecdb");
    CreateSchema ("testSchema", "ts");
    AddEntityClass ("Foo");
    AddEntityClass ("Goo");
    AddRelationShipClass (Cardinality::OneOne, Cardinality::OneOne, StrengthType::Holding, Direction::Forward, "FooOwnsGoo", "Foo", "Goo", true);
    AddRelationShipClass (Cardinality::OneOne, Cardinality::OneMany, StrengthType::Holding, Direction::Forward, "FooOwnsManyGoo", "Foo", "Goo", true);
    AddRelationShipClass (Cardinality::OneMany, Cardinality::OneMany, StrengthType::Holding, Direction::Forward, "ManyFooOwnManyGoo", "Foo", "Goo", true);
    AssertSchemaImport (true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances ("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances ("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> FooOwnsGooResult;
    std::vector<DbResult> FooOwnsManyGooResult;
    std::vector<DbResult> ManyFooOwnManyGooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(1,1), Target(1,1)
            if (f == g)
                FooOwnsGooResult.push_back (BE_SQLITE_DONE);
            else
                FooOwnsGooResult.push_back (BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(1,1), Target(1,N)
            if (f == 0)
                FooOwnsManyGooResult.push_back (BE_SQLITE_DONE);
            else
                FooOwnsManyGooResult.push_back (BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(1,N), Target(1,N)
            ManyFooOwnManyGooResult.push_back (BE_SQLITE_DONE);
            }
        }

    //1-1............................
    PersistedMapStrategy mapStrategy;
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("FooOwnsGoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    size_t count_FooOwnsGoo = 0;
    InsertRelationshipInstances ("ts.FooOwnsGoo", fooKeys, gooKeys, FooOwnsGooResult, count_FooOwnsGoo);
    ASSERT_EQ (count_FooOwnsGoo, GetInsertedRelationshipsCount ("ts.FooOwnsGoo"));

    //1-N............................
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("FooOwnsManyGoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    size_t count_FooOwnsManyGoo = 0;
    InsertRelationshipInstances ("ts.FooOwnsManyGoo", fooKeys, gooKeys, FooOwnsManyGooResult, count_FooOwnsManyGoo);
    ASSERT_EQ (count_FooOwnsManyGoo, GetInsertedRelationshipsCount ("ts.FooOwnsManyGoo"));

    //N-N...........................
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("ManyFooOwnManyGoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::OwnTable, mapStrategy.m_strategy);

    size_t count_ManyGooOwnManyGoo = 0;
    InsertRelationshipInstances ("ts.ManyFooOwnManyGoo", fooKeys, gooKeys, ManyFooOwnManyGooResult, count_ManyGooOwnManyGoo);
    ASSERT_EQ (count_ManyGooOwnManyGoo, GetInsertedRelationshipsCount ("ts.ManyFooOwnManyGoo"));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbRelationshipsIntegrityTests, BackwardHoldingRelationshipsTest)
    {
    SetupECDb ("backwardHoldingRelationshipsTest.ecdb");
    CreateSchema ("testSchema", "ts");
    AddEntityClass ("Foo");
    AddEntityClass ("Goo");
    AddRelationShipClass (Cardinality::OneOne, Cardinality::OneOne, StrengthType::Holding, Direction::Backward, "GooOwnsFoo", "Foo", "Goo", true);
    AddRelationShipClass (Cardinality::OneMany, Cardinality::OneOne, StrengthType::Holding, Direction::Backward, "GooOwnsManyFoo", "Foo", "Goo", true);
    AddRelationShipClass (Cardinality::OneMany, Cardinality::OneMany, StrengthType::Holding, Direction::Backward, "ManyGooOwnManyFoo", "Foo", "Goo", true);
    AssertSchemaImport (true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances ("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances ("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> GooOwnsFooResult;
    std::vector<DbResult> GooOwnsManyFooResult;
    std::vector<DbResult> ManyGooOwnManyFooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(1,1), Target(1,1)
            if (f == g)
                GooOwnsFooResult.push_back (BE_SQLITE_DONE);
            else
                GooOwnsFooResult.push_back (BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(1,N), Target(1,1)
            if (g == 0)
                GooOwnsManyFooResult.push_back (BE_SQLITE_DONE);
            else
                GooOwnsManyFooResult.push_back (BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(1,N), Target(1,N)
            ManyGooOwnManyFooResult.push_back (BE_SQLITE_DONE);
            }
        }

    //1-1............................
    PersistedMapStrategy mapStrategy;
#ifdef TFS361480
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("GooOwnsFoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);
#endif // TFS361480

    size_t count_FooOwnsGoo = 0;
    InsertRelationshipInstances ("ts.GooOwnsFoo", fooKeys, gooKeys, GooOwnsFooResult, count_FooOwnsGoo);
    ASSERT_EQ (count_FooOwnsGoo, GetInsertedRelationshipsCount ("ts.GooOwnsFoo"));

    //1-N...........................
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("GooOwnsManyFoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);

    size_t count_FooOwnsManyGoo = 0;
    InsertRelationshipInstances ("ts.GooOwnsManyFoo", fooKeys, gooKeys, GooOwnsManyFooResult, count_FooOwnsManyGoo);
    ASSERT_EQ (count_FooOwnsManyGoo, GetInsertedRelationshipsCount ("ts.GooOwnsManyFoo"));

    //N-N...........................
    ASSERT_TRUE (TryGetPersistedMapStrategy (mapStrategy, m_ecdb, GetRelationShipClassId ("ManyGooOwnManyFoo")));
    ASSERT_EQ (PersistedMapStrategy::Strategy::OwnTable, mapStrategy.m_strategy);

    size_t count_ManyGooOwnManyGoo = 0;
    InsertRelationshipInstances ("ts.ManyGooOwnManyFoo", fooKeys, gooKeys, ManyGooOwnManyFooResult, count_ManyGooOwnManyGoo);
    ASSERT_EQ (count_ManyGooOwnManyGoo, GetInsertedRelationshipsCount ("ts.ManyGooOwnManyFoo"));
    }

END_ECDBUNITTESTS_NAMESPACE
