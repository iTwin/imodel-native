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

extern bool InsertInstance(ECDbR db, ECClassCR ecClass, IECInstanceR ecInstance);
extern IECRelationshipInstancePtr CreateRelationship(ECRelationshipClassCR relationshipClass, IECInstanceR source, IECInstanceR target);
extern ECInstanceId InstanceToId(IECInstanceCR ecInstance);
extern IECInstancePtr CreatePerson(ECClassCR ecClass, Utf8CP firstName, Utf8CP lastName);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DeleteInstance(IECInstanceCR instance, ECDbR ecdb)
    {
    ECClassCR ecClass = instance.GetClass();
    ECInstanceDeleter deleter(ecdb, ecClass);

    ASSERT_TRUE(deleter.IsValid()) << "Invaild Deleter for ecClass : " << ecClass.GetName().c_str();

    auto status = deleter.Delete(instance);
    ASSERT_TRUE(status == SUCCESS) << "Instance Deletion failed for ecClass : " << ecClass.GetName().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool HasInstance(IECInstanceCR instance, ECDbR ecdb)
    {
    Utf8String ecsql;
    ecsql.Sprintf("SELECT NULL FROM ONLY %s WHERE ECInstanceId=%llu",
                  instance.GetClass().GetECSqlName().c_str(), InstanceToId(instance).GetValue());

    ECSqlStatement statement;
    EXPECT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, ecsql.c_str()));
    return statement.Step() == BE_SQLITE_ROW;
    }

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/2014
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipStrength, BackwardEmbedding)
    {
    ECDbR ecdb = SetupECDb("BackwardRelationshipStrengthTest.ecdb", BeFileName(L"RelationshipStrengthTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());
    /*
    *                                           SingleParent
    *                                                 |
    *                                                 | ChildrenHasSingleParent (Backward EMBEDDING)
    *         ________________________________________|______________________________________
    *        |                                        |                                      |
    *      Child1                                   Child2                                 Child3
    */
    ECClassCP personClass = ecdb.Schemas().GetECClass("RelationshipStrengthTest", "Person");
    IECInstancePtr child1 = CreatePerson(*personClass, "First", "Child");
    InsertInstance(ecdb, *personClass, *child1);
    IECInstancePtr child2 = CreatePerson(*personClass, "Second", "Child");
    InsertInstance(ecdb, *personClass, *child2);
    IECInstancePtr child3 = CreatePerson(*personClass, "Third", "Child");
    InsertInstance(ecdb, *personClass, *child3);
    IECInstancePtr singleParent = CreatePerson(*personClass, "Only", "singleParent");
    InsertInstance(ecdb, *personClass, *singleParent);

    //Backward Embedding relationship (SingleParent -> Child1, Child2)
    ECRelationshipClassCP ChildrenHasSingleParent = ecdb.Schemas().GetECClass("RelationshipStrengthTest", "ChildrenHasSingleParent")->GetRelationshipClassCP();
    IECRelationshipInstancePtr child1HasSingleParent;
    child1HasSingleParent = CreateRelationship(*ChildrenHasSingleParent, *child1, *singleParent);
    InsertInstance(ecdb, *ChildrenHasSingleParent, *child1HasSingleParent);
    IECRelationshipInstancePtr child2HasSingleParent;
    child2HasSingleParent = CreateRelationship(*ChildrenHasSingleParent, *child2, *singleParent);
    InsertInstance(ecdb, *ChildrenHasSingleParent, *child2HasSingleParent);
    IECRelationshipInstancePtr child3HasSingleParent;
    child3HasSingleParent = CreateRelationship(*ChildrenHasSingleParent, *child3, *singleParent);
    InsertInstance(ecdb, *ChildrenHasSingleParent, *child3HasSingleParent);

    /*
    * Test 1: Delete Child1
    * Validate child1HasSingleParent,  child1 have been deleted
    * Validate singleParent, child2HasSingleParent, child3HasSingleParent, child2, child3 are still there
    */
    DeleteInstance(*child1, ecdb);
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(*child1, ecdb));
    ASSERT_FALSE(HasInstance(*child1HasSingleParent, ecdb));

    ASSERT_TRUE(HasInstance(*singleParent, ecdb));
    ASSERT_TRUE(HasInstance(*child2HasSingleParent, ecdb));
    ASSERT_TRUE(HasInstance(*child3HasSingleParent, ecdb));
    ASSERT_TRUE(HasInstance(*child2, ecdb));
    ASSERT_TRUE(HasInstance(*child3, ecdb));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelationshipStrength, RelationshipTest)
    {
    ECDbR ecdb = SetupECDb("RelationshipStrengthTest.ecdb", BeFileName(L"RelationshipStrengthTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    /*
     *          Create the following relationship hierarchy
     *
     * GrandParent1  <- ParentHasSpouse (REFERENCING) -> GrandParent2
     *     |__________________________________________________|
     *                             |
     *                             | ManyParentsHaveChildren (REFERENCING)
     *                             |
     *                         SingleParent
     *                             |
     *                             | SingleParentHasChildren (EMBEDDING)
     *      _______________________|__________________________
     *     |                                                  |
     *   Child1                                             Child2
     *
     */

    ECClassCP personClass = ecdb.Schemas().GetECClass("RelationshipStrengthTest", "Person");
    IECInstancePtr grandParent1 = CreatePerson(*personClass, "First", "GrandParent");
    InsertInstance(ecdb, *personClass, *grandParent1);
    IECInstancePtr grandParent2 = CreatePerson(*personClass, "Second", "GrandParent");
    InsertInstance(ecdb, *personClass, *grandParent2);
    IECInstancePtr singleParent = CreatePerson(*personClass, "Only", "SingleParent");
    InsertInstance(ecdb, *personClass, *singleParent);
    IECInstancePtr child1 = CreatePerson(*personClass, "First", "Child");
    InsertInstance(ecdb, *personClass, *child1);
    IECInstancePtr child2 = CreatePerson(*personClass, "Second", "Child");
    InsertInstance(ecdb, *personClass, *child2);

    // Referencing relationship (GrandParent1, GrandParent2 -> SingleParent)
    ECRelationshipClassCP manyParentsHaveChildren = ecdb.Schemas().GetECClass("RelationshipStrengthTest", "ManyParentsHaveChildren")->GetRelationshipClassCP();
    IECRelationshipInstancePtr grandParent1HasSingleParent;
    grandParent1HasSingleParent = CreateRelationship(*manyParentsHaveChildren, *grandParent1, *singleParent);
    InsertInstance(ecdb, *manyParentsHaveChildren, *grandParent1HasSingleParent);
    IECRelationshipInstancePtr grandParent2HasSingleParent;
    grandParent2HasSingleParent = CreateRelationship(*manyParentsHaveChildren, *grandParent2, *singleParent);
    InsertInstance(ecdb, *manyParentsHaveChildren, *grandParent2HasSingleParent);

    // Embedding relationship (SingleParent -> Child1, Child2)
    ECRelationshipClassCP singleParentHasChildren = ecdb.Schemas().GetECClass("RelationshipStrengthTest", "SingleParentHasChildren")->GetRelationshipClassCP();
    IECRelationshipInstancePtr singleParentHasChild1;
    singleParentHasChild1 = CreateRelationship(*singleParentHasChildren, *singleParent, *child1);
    InsertInstance(ecdb, *singleParentHasChildren, *singleParentHasChild1);
    IECRelationshipInstancePtr singleParentHasChild2;
    singleParentHasChild2 = CreateRelationship(*singleParentHasChildren, *singleParent, *child2);
    InsertInstance(ecdb, *singleParentHasChildren, *singleParentHasChild2);

    // Referencing relationship (GrandParent1 <-> GrandParent2)
    ECRelationshipClassCP parentHasSpouse = ecdb.Schemas().GetECClass("RelationshipStrengthTest", "ParentHasSpouse")->GetRelationshipClassCP();
    IECRelationshipInstancePtr grandParent1HasSpouse;
    grandParent1HasSpouse = CreateRelationship(*parentHasSpouse, *grandParent1, *grandParent2);
    InsertInstance(ecdb, *parentHasSpouse, *grandParent1HasSpouse);
    IECRelationshipInstancePtr grandParent2HasSpouse;
    grandParent2HasSpouse = CreateRelationship(*parentHasSpouse, *grandParent2, *grandParent1);
    InsertInstance(ecdb, *parentHasSpouse, *grandParent2HasSpouse);

    ecdb.SaveChanges();

    //Verify instances before deletion
    ASSERT_TRUE(HasInstance(*grandParent1HasSpouse, ecdb));
    ASSERT_TRUE(HasInstance(*grandParent2HasSpouse, ecdb));

    /*
    * Test 1: Delete GrandParent1
    * Validate grandParent1HasSpouse, grandParent2HasSpouse, grandParent1HasSingleParent have been deleted (orphaned relationships)
    * Validate singleParent is still around (referencing relationship with one parent remaining)
    */
    DeleteInstance(*grandParent1, ecdb);
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(*grandParent1, ecdb));
    ASSERT_FALSE(HasInstance(*grandParent1HasSpouse, ecdb));
    ASSERT_FALSE(HasInstance(*grandParent2HasSpouse, ecdb));
    ASSERT_FALSE(HasInstance(*grandParent1HasSingleParent, ecdb));

    ASSERT_TRUE(HasInstance(*singleParent, ecdb));

    /*
    * Test 2: Delete GrandParent2
    * Validate grandParent2HasSingleParent has been deleted (orphaned relationship), *Validate singeParent has been deleted (held instance with no parents remaining)
    */
    DeleteInstance(*grandParent2, ecdb);
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(*grandParent2, ecdb));
    ASSERT_FALSE(HasInstance(*grandParent2HasSingleParent, ecdb));

    ASSERT_TRUE(HasInstance(*singleParent, ecdb));
    ASSERT_TRUE(HasInstance(*singleParentHasChild1, ecdb));
    ASSERT_TRUE(HasInstance(*singleParentHasChild2, ecdb));
    ASSERT_TRUE(HasInstance(*child1, ecdb));
    ASSERT_TRUE(HasInstance(*child2, ecdb));
    }

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/2014
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipStrength, BackwardHoldingForwardEmbedding)
    {
    ECDbR ecdb = SetupECDb("BackwardRelationshipStrengthTest.ecdb", BeFileName(L"RelationshipStrengthTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    /*
    *          Create the following relationship hierarchy
    *
    * GrandParent1  <- ParentHasSpouse (REFERENCING) -> GrandParent2
    *     |__________________________________________________|
    *                             |
    *                             | ChildrenHaveManyParents.( Backward REFERENCING)
    *                             |
    *                         SingleParent
    *                             |
    *                             | SingleParentHasChildren.( Forward EMBEDDING)
    *      _______________________|__________________________
    *     |                                                  |
    *   Child1                                             Child2
    *
    */
    ECClassCP personClass = ecdb.Schemas().GetECClass("RelationshipStrengthTest", "Person");
    IECInstancePtr child1 = CreatePerson(*personClass, "First", "Child");
    InsertInstance(ecdb, *personClass, *child1);
    IECInstancePtr child2 = CreatePerson(*personClass, "Second", "Child");
    InsertInstance(ecdb, *personClass, *child2);
    IECInstancePtr singleParent = CreatePerson(*personClass, "Only", "singleParent");
    InsertInstance(ecdb, *personClass, *singleParent);
    IECInstancePtr grandParent1 = CreatePerson(*personClass, "First", "GrandParent");
    InsertInstance(ecdb, *personClass, *grandParent1);
    IECInstancePtr grandParent2 = CreatePerson(*personClass, "Second", "GrandParent");
    InsertInstance(ecdb, *personClass, *grandParent2);

    // Backward referencing relationship (GrandParent1, GrandParent2 <- SingleParent)
    ECRelationshipClassCP ChildrenHaveManyParents = ecdb.Schemas().GetECClass("RelationshipStrengthTest", "ChildrenHaveManyParents")->GetRelationshipClassCP();
    IECRelationshipInstancePtr singleParentHasGrandParent1;
    singleParentHasGrandParent1 = CreateRelationship(*ChildrenHaveManyParents, *singleParent, *grandParent1);
    InsertInstance(ecdb, *ChildrenHaveManyParents, *singleParentHasGrandParent1);
    IECRelationshipInstancePtr singleParentHasGrandParent2;
    singleParentHasGrandParent2 = CreateRelationship(*ChildrenHaveManyParents, *singleParent, *grandParent2);
    InsertInstance(ecdb, *ChildrenHaveManyParents, *singleParentHasGrandParent2);

    //Forward Embedding relationship (SingleParent -> Child1, Child2)
    ECRelationshipClassCP SingleParentHasChildren = ecdb.Schemas().GetECClass("RelationshipStrengthTest", "SingleParentHasChildren")->GetRelationshipClassCP();
    IECRelationshipInstancePtr singleParentHasChild1;
    singleParentHasChild1 = CreateRelationship(*SingleParentHasChildren, *singleParent, *child1);
    InsertInstance(ecdb, *SingleParentHasChildren, *singleParentHasChild1);
    IECRelationshipInstancePtr singleParentHasChild2;
    singleParentHasChild2 = CreateRelationship(*SingleParentHasChildren, *singleParent, *child2);
    InsertInstance(ecdb, *SingleParentHasChildren, *singleParentHasChild2);

    //Backward Referencing relationship (GrandParent1 <-> GrandParent2)
    ECRelationshipClassCP parentHasSpouse = ecdb.Schemas().GetECClass("RelationshipStrengthTest", "ParentHasSpouse_backward")->GetRelationshipClassCP();
    IECRelationshipInstancePtr grandParent1HasSpouse;
    grandParent1HasSpouse = CreateRelationship(*parentHasSpouse, *grandParent1, *grandParent2);
    InsertInstance(ecdb, *parentHasSpouse, *grandParent1HasSpouse);
    IECRelationshipInstancePtr grandParent2HasSpouse;
    grandParent2HasSpouse = CreateRelationship(*parentHasSpouse, *grandParent2, *grandParent1);
    InsertInstance(ecdb, *parentHasSpouse, *grandParent2HasSpouse);

    ecdb.SaveChanges();

    //Validate Instance exists before deletion
    ASSERT_TRUE(HasInstance(*singleParentHasChild1, ecdb));

    /*
    * Test 1: Delete Child1
    * Validate Child1 and singleParentHasChild1 have been deleted
    * Validate Child2 is Still there
    */
    DeleteInstance(*child1, ecdb);
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(*child1, ecdb));
    ASSERT_FALSE(HasInstance(*singleParentHasChild1, ecdb));

    ASSERT_TRUE(HasInstance(*child2, ecdb));

    /*
    * Test 2: Delete Child2
    * Validate Child2 and singleParentHasChild2 have been deleted
    * Validate singleParent is still around (relationship grand parents remaining)
    */
    DeleteInstance(*child2, ecdb);
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(*child2, ecdb));
    ASSERT_FALSE(HasInstance(*singleParentHasChild2, ecdb));

    ASSERT_TRUE(HasInstance(*singleParent, ecdb));

    /*
    * Test 3: Delete GrandParent1
    * Validate GrandParent1, grandParent1HasSpouse, grandParent2HasSpouse, singleParentHasGrandParent1 have been deleted
    * Validate singleParent is still around (referencing relationship with one parent remaining)
    */
    DeleteInstance(*grandParent1, ecdb);
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(*grandParent1, ecdb));
    ASSERT_FALSE(HasInstance(*grandParent1HasSpouse, ecdb));
    ASSERT_FALSE(HasInstance(*grandParent2HasSpouse, ecdb));
    ASSERT_FALSE(HasInstance(*singleParentHasGrandParent1, ecdb));

    ASSERT_TRUE(HasInstance(*singleParent, ecdb));

    /*
    * Test 4: Delete GrandParent2
    * Validate GrandParent2, singleParentHasGrandParent2 have been deleted, * Single parent has been deleted too as no parent exists anymore
    */
    DeleteInstance(*grandParent2, ecdb);
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(*grandParent2, ecdb));
    ASSERT_FALSE(HasInstance(*singleParentHasGrandParent2, ecdb));
    ASSERT_TRUE(HasInstance(*singleParent, ecdb));
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
        ECEntityClassP GetEntityClass(Utf8CP className);

        RelationshipCardinalityCR GetClassCardinality(Cardinality classCardinality);

        ECRelatedInstanceDirection GetRelationDirection(Direction direction);

    protected:
        ECSchemaPtr testSchema = nullptr;

        void CreateSchema(Utf8CP schemaName, Utf8CP schemaNamePrefix);

        //Adding a Class automatically adds a Property of Type string with Name "SqlPrintfString ("%sProp", className)" to the class.
        void AddEntityClass(Utf8CP className);

        void AddRelationShipClass(Cardinality SourceClassCardinality, Cardinality targetClassCardinality, StrengthType strengthType, Direction direction, Utf8CP relationshipClassName, Utf8CP sourceClass, Utf8CP targetClass, bool isPolymorphic);

        void AssertSchemaImport(bool isSchemaImportExpectedToSucceed);

        void InsertEntityClassInstances(Utf8CP className, Utf8CP propName, int numberOfInstances, std::vector<ECInstanceKey>& classKeys);

        void InsertRelationshipInstances(Utf8CP relationshipClass, std::vector<ECInstanceKey> const& sourceKeys, std::vector<ECInstanceKey>const& targetKeys, std::vector<DbResult> const& expected, size_t& rowInserted) const;

        size_t GetInsertedRelationshipsCount(Utf8CP relationshipClass) const;

        ECClassId GetRelationShipClassId(Utf8CP className);

        bool InstanceExists(Utf8CP classExp, ECInstanceKey const& key) const;

        bool RelationshipExists(Utf8CP relClassExp, ECInstanceKey const& sourceKey, ECInstanceKey const& targetKey) const;
    };

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
ECEntityClassP ECDbRelationshipsIntegrityTests::GetEntityClass(Utf8CP className)
    {
    return testSchema->GetClassP(className)->GetEntityClassP();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
RelationshipCardinalityCR ECDbRelationshipsIntegrityTests::GetClassCardinality(Cardinality classCardinality)
    {
    if (classCardinality == Cardinality::ZeroOne)
        return RelationshipCardinality::ZeroOne();
    else if (classCardinality == Cardinality::ZeroMany)
        return RelationshipCardinality::ZeroMany();
    else if (classCardinality == Cardinality::OneOne)
        return RelationshipCardinality::OneOne();
    else
        return RelationshipCardinality::OneMany();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
ECRelatedInstanceDirection ECDbRelationshipsIntegrityTests::GetRelationDirection(Direction direction)
    {
    if (direction == Direction::Forward)
        return ECRelatedInstanceDirection::Forward;
    else
        return ECRelatedInstanceDirection::Backward;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::CreateSchema(Utf8CP schemaName, Utf8CP schemaNamePrefix)
    {
    ECSchema::CreateSchema(testSchema, schemaName, 1, 0);
    ASSERT_TRUE(testSchema.IsValid());

    testSchema->SetNamespacePrefix(schemaNamePrefix);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::AssertSchemaImport(bool isSchemaImportExpectedToSucceed)
    {

    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    EXPECT_EQ(ECObjectsStatus::Success, readContext->AddSchema(*testSchema));
    if (isSchemaImportExpectedToSucceed)
        {
        EXPECT_EQ(SUCCESS, m_ecdb.Schemas().ImportECSchemas(readContext->GetCache()));
        }
    else
        {
        EXPECT_EQ(ERROR, m_ecdb.Schemas().ImportECSchemas(readContext->GetCache()));
        }
    m_ecdb.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::AddEntityClass(Utf8CP className)
    {
    ECEntityClassP testClass = nullptr;
    EXPECT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(testClass, className));
    PrimitiveECPropertyP prim = nullptr;
    EXPECT_EQ(ECObjectsStatus::Success, testClass->CreatePrimitiveProperty(prim, SqlPrintfString("%sProp", className).GetUtf8CP()));
    EXPECT_EQ(ECObjectsStatus::Success, prim->SetType(PrimitiveType::PRIMITIVETYPE_String));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::AddRelationShipClass(Cardinality SourceClassCardinality, Cardinality targetClassCardinality, StrengthType strengthType, Direction direction, Utf8CP relationshipClassName, Utf8CP sourceClass, Utf8CP targetClass, bool isPolymorphic)
    {
    ECRelationshipClassP testRelationshipClass = nullptr;
    EXPECT_EQ(ECObjectsStatus::Success, testSchema->CreateRelationshipClass(testRelationshipClass, relationshipClassName));
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->SetStrength(strengthType));
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->SetStrengthDirection(GetRelationDirection(direction)));

    //Set Relstionship Source Class and Cardinality
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->GetSource().AddClass(*GetEntityClass(sourceClass)));
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->GetSource().SetIsPolymorphic(true));
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->GetSource().SetCardinality(GetClassCardinality(SourceClassCardinality)));

    //Set Relstionship Target Class and Cardinality
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->GetTarget().AddClass(*GetEntityClass(targetClass)));
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->GetTarget().SetIsPolymorphic(true));
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->GetTarget().SetCardinality(GetClassCardinality(targetClassCardinality)));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::InsertEntityClassInstances(Utf8CP className, Utf8CP propName, int numberOfInstances, std::vector<ECInstanceKey>& classKeys)
    {
    ECSqlStatement stmt;

    SqlPrintfString insertECSql = SqlPrintfString("INSERT INTO %s(%s) VALUES(?)", GetEntityClass(className)->GetECSqlName().c_str(), propName);

    ASSERT_EQ(stmt.Prepare(m_ecdb, insertECSql.GetUtf8CP()), ECSqlStatus::Success);
    for (int i = 0; i < numberOfInstances; i++)
        {
        ECInstanceKey key;
        SqlPrintfString textVal = SqlPrintfString("%s_%d", className, i);
        ASSERT_EQ(stmt.Reset(), ECSqlStatus::Success);
        ASSERT_EQ(stmt.ClearBindings(), ECSqlStatus::Success);
        ASSERT_EQ(stmt.BindText(1, textVal.GetUtf8CP(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(stmt.Step(key), BE_SQLITE_DONE);
        classKeys.push_back(key);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::InsertRelationshipInstances(Utf8CP relationshipClass, std::vector<ECInstanceKey> const& sourceKeys, std::vector<ECInstanceKey>const& targetKeys, std::vector<DbResult> const& expected, size_t& rowInserted) const
    {
    ECSqlStatement stmt;
    SqlPrintfString sql = SqlPrintfString("INSERT INTO %s (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(?,?,?,?)", relationshipClass);

    ASSERT_EQ(stmt.Prepare(m_ecdb, sql.GetUtf8CP()), ECSqlStatus::Success);
    ASSERT_EQ(expected.size(), sourceKeys.size() * targetKeys.size());
    int n = 0;
    for (auto& sourceKey : sourceKeys)
        {
        for (auto& targetKey : targetKeys)
            {
            stmt.Reset();
            ASSERT_EQ(ECSqlStatus::Success, stmt.ClearBindings());
            stmt.BindId(1, sourceKey.GetECInstanceId());
            stmt.BindId(2, sourceKey.GetECClassId());
            stmt.BindId(3, targetKey.GetECInstanceId());
            stmt.BindId(4, targetKey.GetECClassId());

            if (expected[n] != BE_SQLITE_DONE)
                ASSERT_NE(BE_SQLITE_DONE, stmt.Step());
            else
                {
                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
                rowInserted++;
                }

            n = n + 1;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
size_t ECDbRelationshipsIntegrityTests::GetInsertedRelationshipsCount(Utf8CP relationshipClass) const
    {
    ECSqlStatement stmt;
    auto sql = SqlPrintfString("SELECT COUNT(*) FROM ONLY ts.Foo JOIN ts.Goo USING %s", relationshipClass);
    if (stmt.Prepare(m_ecdb, sql.GetUtf8CP()) == ECSqlStatus::Success)
        {
        if (stmt.Step() == BE_SQLITE_ROW)
            return static_cast<size_t>(stmt.GetValueInt(0));
        }

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
ECClassId ECDbRelationshipsIntegrityTests::GetRelationShipClassId(Utf8CP className)
    {
    return testSchema->GetClassP(className)->GetEntityClassP()->GetId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbRelationshipsIntegrityTests::InstanceExists(Utf8CP classExp, ECInstanceKey const& key) const
    {
    Utf8String ecsql;
    ecsql.Sprintf("SELECT NULL FROM %s WHERE ECInstanceId=?", classExp);
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), ecsql.c_str())) << ecsql.c_str();
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetECInstanceId()));

    DbResult stat = stmt.Step();
    EXPECT_TRUE(stat == BE_SQLITE_ROW || stat == BE_SQLITE_DONE);
    return stat == BE_SQLITE_ROW;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbRelationshipsIntegrityTests::RelationshipExists(Utf8CP relClassExp, ECInstanceKey const& sourceKey, ECInstanceKey const& targetKey) const
    {
    Utf8String ecsql;
    ecsql.Sprintf("SELECT NULL FROM %s WHERE SourceECInstanceId=? AND SourceECClassId=? AND TargetECInstanceId=? AND TargetECClassId=?", relClassExp);
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), ecsql.c_str())) << ecsql.c_str();
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(1, sourceKey.GetECInstanceId()));
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(2, sourceKey.GetECClassId()));
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(3, targetKey.GetECInstanceId()));
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(4, targetKey.GetECClassId()));

    DbResult stat = stmt.Step();
    EXPECT_TRUE(stat == BE_SQLITE_ROW || stat == BE_SQLITE_DONE);
    return stat == BE_SQLITE_ROW;
    };

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbRelationshipsIntegrityTests, ForwardEmbeddingRelationshipsTest)
    {
    SetupECDb("forwardEmbeddingRelationshipsTest.ecdb");
    ASSERT_TRUE(GetECDb().IsDbOpen());

    CreateSchema("testSchema", "ts");
    AddEntityClass("Foo");
    AddEntityClass("Goo");
    AddRelationShipClass(Cardinality::ZeroOne, Cardinality::OneOne, StrengthType::Embedding, Direction::Forward, "FooOwnsGoo", "Foo", "Goo", true);
    AddRelationShipClass(Cardinality::ZeroOne, Cardinality::OneMany, StrengthType::Embedding, Direction::Forward, "FooOwnsManyGoo", "Foo", "Goo", true);
    AssertSchemaImport(true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> FooOwnsGooResult;
    std::vector<DbResult> FooOwnsManyGooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(0,1), Target(1,1)
            if (f == g)
                FooOwnsGooResult.push_back(BE_SQLITE_DONE);
            else
                FooOwnsGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(0,1), Target(1,N)
            if (f == 0)
                FooOwnsManyGooResult.push_back(BE_SQLITE_DONE);
            else
                FooOwnsManyGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);
            }
        }

    //1-1............................
    PersistedMapStrategy mapStrategy;
    size_t count_FooOwnsGoo = 0;
    {
    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, m_ecdb, GetRelationShipClassId("FooOwnsGoo")));
    ASSERT_EQ(PersistedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooOwnsGoo", fooKeys, gooKeys, FooOwnsGooResult, count_FooOwnsGoo);
    }
    ASSERT_EQ(count_FooOwnsGoo, GetInsertedRelationshipsCount("ts.FooOwnsGoo"));

    //1-N............................
    size_t count_FooOwnsManyGoo = 0;
    {
    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, m_ecdb, GetRelationShipClassId("FooOwnsManyGoo")));
    ASSERT_EQ(PersistedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooOwnsManyGoo", fooKeys, gooKeys, FooOwnsManyGooResult, count_FooOwnsManyGoo);
    }
    ASSERT_EQ(count_FooOwnsManyGoo, GetInsertedRelationshipsCount("ts.FooOwnsManyGoo"));

    m_ecdb.Schemas().CreateECClassViewsInDb();

    //Delete fooKeys[0]
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "DELETE FROM ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[0].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[0]));

    //fooKeys[1] and fooKeys[2] which are in same table have no impact
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));

    //fooKeys[0] embedds gooKeys, so all gooKeys will get deleted.
    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[2]));

    //As a result all the relationships will also get deleted.
    ASSERT_FALSE(RelationshipExists("ts.FooOwnsGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooOwnsGoo", fooKeys[1], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooOwnsGoo", fooKeys[2], gooKeys[2]));

    ASSERT_FALSE(RelationshipExists("ts.FooOwnsManyGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooOwnsManyGoo", fooKeys[0], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooOwnsManyGoo", fooKeys[0], gooKeys[2]));
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbRelationshipsIntegrityTests, BackwardEmbeddingRelationshipsTest)
    {
    SetupECDb("backwardEmbeddingRelationshipTest.ecdb");
    ASSERT_TRUE(GetECDb().IsDbOpen());

    CreateSchema("testSchema", "ts");
    AddEntityClass("Foo");
    AddEntityClass("Goo");
    AddRelationShipClass(Cardinality::ZeroOne, Cardinality::ZeroOne, StrengthType::Embedding, Direction::Backward, "FooOwnedByGoo", "Foo", "Goo", true);
    AddRelationShipClass(Cardinality::OneMany, Cardinality::ZeroOne, StrengthType::Embedding, Direction::Backward, "FooOwnedByManyGoo", "Foo", "Goo", true);
    AssertSchemaImport(true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> FooOwnedByGooResult;
    std::vector<DbResult> FooOwnedByManyGooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(1,1), Target(0,1)
            if (f == g)
                FooOwnedByGooResult.push_back(BE_SQLITE_DONE);
            else
                FooOwnedByGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(1,N), Target(0,1)
            if (g == 0)
                FooOwnedByManyGooResult.push_back(BE_SQLITE_DONE);
            else
                FooOwnedByManyGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);
            }
        }

    //1-1...........................
    PersistedMapStrategy mapStrategy;
    size_t count_FooOwnedByGoo = 0;
    {
    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, m_ecdb, GetRelationShipClassId("FooOwnedByGoo")));
    ASSERT_EQ(PersistedMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooOwnedByGoo", fooKeys, gooKeys, FooOwnedByGooResult, count_FooOwnedByGoo);
    }
    ASSERT_EQ(count_FooOwnedByGoo, GetInsertedRelationshipsCount("ts.FooOwnedByGoo"));

    //1-N..........................
    size_t count_FooOwnedByManyGoo = 0;
    {
    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, m_ecdb, GetRelationShipClassId("FooOwnedByManyGoo")));
    ASSERT_EQ(PersistedMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooOwnedByManyGoo", fooKeys, gooKeys, FooOwnedByManyGooResult, count_FooOwnedByManyGoo);
    }
    ASSERT_EQ(count_FooOwnedByManyGoo, GetInsertedRelationshipsCount("ts.FooOwnedByManyGoo"));

    m_ecdb.Schemas().CreateECClassViewsInDb();

    //Delete gooKeys[0]
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "DELETE FROM ts.Goo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKeys[0].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[0]));

    //gooKeys[1] and gooKeys[2] will have no impact
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));

    //fooKeys[0] embedds fooKeys, so all fooKeys will get deleted.
    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[0]));
    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[2]));

    //As a result all the relationships will also get deleted.
    ASSERT_FALSE(RelationshipExists("ts.FooOwnedByGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooOwnedByGoo", fooKeys[1], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooOwnedByGoo", fooKeys[2], gooKeys[2]));

    ASSERT_FALSE(RelationshipExists("ts.FooOwnedByManyGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooOwnedByManyGoo", fooKeys[0], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooOwnedByManyGoo", fooKeys[0], gooKeys[2]));
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbRelationshipsIntegrityTests, ForwardReferencingRelationshipsTest)
    {
    SetupECDb("forwardReferencingRelationshipsTest.ecdb");
    ASSERT_TRUE(GetECDb().IsDbOpen());

    CreateSchema("testSchema", "ts");
    AddEntityClass("Foo");
    AddEntityClass("Goo");
    AddRelationShipClass(Cardinality::ZeroOne, Cardinality::ZeroOne, StrengthType::Referencing, Direction::Forward, "FooHasGoo", "Foo", "Goo", true);
    AddRelationShipClass(Cardinality::ZeroOne, Cardinality::ZeroMany, StrengthType::Referencing, Direction::Forward, "FooHasManyGoo", "Foo", "Goo", true);
    AddRelationShipClass(Cardinality::ZeroMany, Cardinality::ZeroMany, StrengthType::Referencing, Direction::Forward, "ManyFoohaveManyGoo", "Foo", "Goo", true);
    AssertSchemaImport(true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances("Goo", "GooProp", maxGooInstances, gooKeys);

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
                FooOwnsGooResult.push_back(BE_SQLITE_DONE);
            else
                FooOwnsGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(0,1), Target(0,N)
            if (f == 0)
                FooOwnsManyGooResult.push_back(BE_SQLITE_DONE);
            else
                FooOwnsManyGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(0,N), Target(0,N)
            ManyFooOwnManyGooResult.push_back(BE_SQLITE_DONE);
            }
        }

    //1-1............................
    PersistedMapStrategy mapStrategy;
    size_t count_FooHasGoo = 0;
    {
    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, m_ecdb, GetRelationShipClassId("FooHasGoo")));
    ASSERT_EQ(PersistedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooHasGoo", fooKeys, gooKeys, FooOwnsGooResult, count_FooHasGoo);
    }
    ASSERT_EQ(count_FooHasGoo, GetInsertedRelationshipsCount("ts.FooHasGoo"));

    //1-N...........................
    size_t count_FooHasManyGoo = 0;
    {
    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, m_ecdb, GetRelationShipClassId("FooHasManyGoo")));
    ASSERT_EQ(PersistedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooHasManyGoo", fooKeys, gooKeys, FooOwnsManyGooResult, count_FooHasManyGoo);
    }
    ASSERT_EQ(count_FooHasManyGoo, GetInsertedRelationshipsCount("ts.FooHasManyGoo"));

    //N-N...........................
    size_t count_ManyFoohaveManyGoo = 0;
    {
    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, m_ecdb, GetRelationShipClassId("ManyFoohaveManyGoo")));
    ASSERT_EQ(PersistedMapStrategy::Strategy::OwnTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.ManyFoohaveManyGoo", fooKeys, gooKeys, ManyFooOwnManyGooResult, count_ManyFoohaveManyGoo);
    }
    ASSERT_EQ(count_ManyFoohaveManyGoo, GetInsertedRelationshipsCount("ts.ManyFoohaveManyGoo"));

    m_ecdb.Schemas().CreateECClassViewsInDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "DELETE FROM ts.Foo WHERE ECInstanceId=?"));
    //Delete fooKeys[0]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[0].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[1]));//fooKeys[1] and fooKeys[2] which are in same table have no impact
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooHasGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooHasManyGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooHasManyGoo", fooKeys[0], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooHasManyGoo", fooKeys[0], gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.ManyFoohaveManyGoo", fooKeys[0], gooKeys[0]));

    ASSERT_TRUE(RelationshipExists("ts.FooHasGoo", fooKeys[1], gooKeys[1]));
    ASSERT_TRUE(RelationshipExists("ts.ManyFoohaveManyGoo", fooKeys[1], gooKeys[1]));
    }

    //Delete fooKeys[1]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[1].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2])); //fooKeys[2] which is in same table has no impact
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooHasGoo", fooKeys[1], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.ManyFoohaveManyGoo", fooKeys[1], gooKeys[1]));

    ASSERT_TRUE(RelationshipExists("ts.FooHasGoo", fooKeys[2], gooKeys[2]));
    ASSERT_TRUE(RelationshipExists("ts.ManyFoohaveManyGoo", fooKeys[2], gooKeys[2]));
    }

    //Delete fooKeys[2]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[2].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHasGoo", fooKeys[2], gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.ManyFoohaveManyGoo", fooKeys[2], gooKeys[2]));

    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));//all gooKeys must exist, refered instances never get deleted.
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbRelationshipsIntegrityTests, BackwardReferencingRelationshipsTest)
    {
    SetupECDb("backwardReferencingRelationshipsTest.ecdb");
    ASSERT_TRUE(GetECDb().IsDbOpen());

    CreateSchema("testSchema", "ts");
    AddEntityClass("Foo");
    AddEntityClass("Goo");
    AddRelationShipClass(Cardinality::ZeroOne, Cardinality::ZeroOne, StrengthType::Referencing, Direction::Backward, "GooHasFoo", "Foo", "Goo", true);
    AddRelationShipClass(Cardinality::ZeroMany, Cardinality::ZeroOne, StrengthType::Referencing, Direction::Backward, "GooHasManyFoo", "Foo", "Goo", true);
    AddRelationShipClass(Cardinality::ZeroMany, Cardinality::ZeroMany, StrengthType::Referencing, Direction::Backward, "ManyGooHaveManyFoo", "Foo", "Goo", true);
    AssertSchemaImport(true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances("Goo", "GooProp", maxGooInstances, gooKeys);

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
                GooOwnsFooResult.push_back(BE_SQLITE_DONE);
            else
                GooOwnsFooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(0,N), Target(0,1)
            if (g == 0)
                GooOwnsManyFooResult.push_back(BE_SQLITE_DONE);
            else
                GooOwnsManyFooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(0,N), Target(0,N)
            ManyGooOwnManyFooResult.push_back(BE_SQLITE_DONE);
            }
        }

    //1-1............................
    PersistedMapStrategy mapStrategy;
    size_t count_GooHasFoo = 0;
    {
    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, m_ecdb, GetRelationShipClassId("GooHasFoo")));
    ASSERT_EQ(PersistedMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.GooHasFoo", fooKeys, gooKeys, GooOwnsFooResult, count_GooHasFoo);
    }
    ASSERT_EQ(count_GooHasFoo, GetInsertedRelationshipsCount("ts.GooHasFoo"));

    //1-N...........................
    size_t count_GooHasManyFoo = 0;
    {
    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, m_ecdb, GetRelationShipClassId("GooHasManyFoo")));
    ASSERT_EQ(PersistedMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.GooHasManyFoo", fooKeys, gooKeys, GooOwnsManyFooResult, count_GooHasManyFoo);
    }
    ASSERT_EQ(count_GooHasManyFoo, GetInsertedRelationshipsCount("ts.GooHasManyFoo"));

    //N-N...........................
    size_t count_ManyGooHaveManyFoo = 0;
    {
    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, m_ecdb, GetRelationShipClassId("ManyGooHaveManyFoo")));
    ASSERT_EQ(PersistedMapStrategy::Strategy::OwnTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.ManyGooHaveManyFoo", fooKeys, gooKeys, ManyGooOwnManyFooResult, count_ManyGooHaveManyFoo);
    }
    ASSERT_EQ(count_ManyGooHaveManyFoo, GetInsertedRelationshipsCount("ts.ManyGooHaveManyFoo"));

    m_ecdb.Schemas().CreateECClassViewsInDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "DELETE FROM ts.Goo WHERE ECInstanceId=?"));
    //Delete gooKeys[0]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKeys[0].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.GooHasFoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.GooHasManyFoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.GooHasManyFoo", fooKeys[0], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.GooHasManyFoo", fooKeys[0], gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.ManyGooHaveManyFoo", fooKeys[0], gooKeys[0]));

    ASSERT_TRUE(RelationshipExists("ts.GooHasFoo", fooKeys[1], gooKeys[1]));
    ASSERT_TRUE(RelationshipExists("ts.ManyGooHaveManyFoo", fooKeys[1], gooKeys[1]));
    }

    //Delete gooKeys[1]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKeys[1].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.GooHasFoo", fooKeys[1], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.ManyGooHaveManyFoo", fooKeys[1], gooKeys[1]));

    ASSERT_TRUE(RelationshipExists("ts.GooHasFoo", fooKeys[2], gooKeys[2]));
    ASSERT_TRUE(RelationshipExists("ts.ManyGooHaveManyFoo", fooKeys[2], gooKeys[2]));
    }

    //Delete gooKeys[2]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKeys[2].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.GooHasFoo", fooKeys[2], gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.ManyGooHaveManyFoo", fooKeys[2], gooKeys[2]));

    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[0]));//all fooKeys must exist, refered instances never get deleted.
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbRelationshipsIntegrityTests, ForwardHoldingOneToOne)
    {
    SetupECDb("forwardHoldingRelationshipsTest.ecdb");
    ASSERT_TRUE(GetECDb().IsDbOpen());

    CreateSchema("testSchema", "ts");
    AddEntityClass("Foo");
    AddEntityClass("Goo");
    AddRelationShipClass(Cardinality::ZeroOne, Cardinality::OneOne, StrengthType::Holding, Direction::Forward, "FooHoldsGoo", "Foo", "Goo", true);
    AssertSchemaImport(true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> FooHoldsGooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(1,1), Target(1,1)
            if (f == g)
                FooHoldsGooResult.push_back(BE_SQLITE_DONE);
            else
                FooHoldsGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);
            }
        }

    //1-1............................
    PersistedMapStrategy mapStrategy;
    size_t count_FooHoldsGoo = 0;
    {
    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, m_ecdb, GetRelationShipClassId("FooHoldsGoo")));
    ASSERT_EQ(PersistedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooHoldsGoo", fooKeys, gooKeys, FooHoldsGooResult, count_FooHoldsGoo);
    }
    ASSERT_EQ(count_FooHoldsGoo, GetInsertedRelationshipsCount("ts.FooHoldsGoo"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "DELETE FROM ts.Foo WHERE ECInstanceId=?"));
    //Delete fooKeys[0]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[0].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldsGoo", fooKeys[0], gooKeys[0]));

    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));

    //gooKeys[1] and gooKeys[2] are still held by fooKeys[1] and fooKeys[2] respectively in 1-1 relationship
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));

    ASSERT_TRUE(RelationshipExists("ts.FooHoldsGoo", fooKeys[1], gooKeys[1]));
    ASSERT_TRUE(RelationshipExists("ts.FooHoldsGoo", fooKeys[2], gooKeys[2]));
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbRelationshipsIntegrityTests, ForwardHoldingRelationshipsTest)
    {
    SetupECDb("forwardHoldingRelationshipsTest.ecdb");
    ASSERT_TRUE(GetECDb().IsDbOpen());

    CreateSchema("testSchema", "ts");
    AddEntityClass("Foo");
    AddEntityClass("Goo");
    AddRelationShipClass(Cardinality::ZeroOne, Cardinality::OneOne, StrengthType::Holding, Direction::Forward, "FooHoldsGoo", "Foo", "Goo", true);
    AddRelationShipClass(Cardinality::ZeroOne, Cardinality::OneMany, StrengthType::Holding, Direction::Forward, "FooHoldManyGoo", "Foo", "Goo", true);
    AssertSchemaImport(true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> FooHoldsGooResult;
    std::vector<DbResult> FooHoldsManyGooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(1,1), Target(1,1)
            if (f == g)
                FooHoldsGooResult.push_back(BE_SQLITE_DONE);
            else
                FooHoldsGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(1,1), Target(1,N)
            if (f == 0)
                FooHoldsManyGooResult.push_back(BE_SQLITE_DONE);
            else
                FooHoldsManyGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);
            }
        }

    //1-1............................
    PersistedMapStrategy mapStrategy;
    size_t count_FooHoldsGoo = 0;
    {
    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, m_ecdb, GetRelationShipClassId("FooHoldsGoo")));
    ASSERT_EQ(PersistedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooHoldsGoo", fooKeys, gooKeys, FooHoldsGooResult, count_FooHoldsGoo);
    }
    ASSERT_EQ(count_FooHoldsGoo, GetInsertedRelationshipsCount("ts.FooHoldsGoo"));

    //1-N............................
    size_t count_FooHoldManyGoo = 0;
    {
    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, m_ecdb, GetRelationShipClassId("FooHoldManyGoo")));
    ASSERT_EQ(PersistedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooHoldManyGoo", fooKeys, gooKeys, FooHoldsManyGooResult, count_FooHoldManyGoo);
    }
    ASSERT_EQ(count_FooHoldManyGoo, GetInsertedRelationshipsCount("ts.FooHoldManyGoo"));

    m_ecdb.Schemas().CreateECClassViewsInDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "DELETE FROM ts.Foo WHERE ECInstanceId=?"));
    //Delete fooKeys[0]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[0].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldsGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldManyGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldManyGoo", fooKeys[0], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldManyGoo", fooKeys[0], gooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));//gooKeys[0] is still held by fooKeys[1] and fooKeys[2]
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));

    ASSERT_TRUE(RelationshipExists("ts.FooHoldsGoo", fooKeys[1], gooKeys[1]));
    ASSERT_TRUE(RelationshipExists("ts.FooHoldsGoo", fooKeys[2], gooKeys[2]));
    }

    //Delete fooKeys[1]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[1].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldsGoo", fooKeys[1], gooKeys[1]));
    }

    //Delete fooKeys[2]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[2].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldsGoo", fooKeys[2], gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.ManyFooHoldManyGoo", fooKeys[2], gooKeys[2]));
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbRelationshipsIntegrityTests, BackwardHoldingRelationshipsTest)
    {
    SetupECDb("backwardHoldingRelationshipsTest.ecdb");
    ASSERT_TRUE(GetECDb().IsDbOpen());

    CreateSchema("testSchema", "ts");
    AddEntityClass("Foo");
    AddEntityClass("Goo");
    AddRelationShipClass(Cardinality::ZeroOne, Cardinality::ZeroOne, StrengthType::Holding, Direction::Backward, "FooHeldByGoo", "Foo", "Goo", true);
    AddRelationShipClass(Cardinality::OneMany, Cardinality::ZeroOne, StrengthType::Holding, Direction::Backward, "FooHeldByManyGoo", "Foo", "Goo", true);
    AssertSchemaImport(true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> fooHeldByGooResult;
    std::vector<DbResult> fooHeldByManyGooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(1,1), Target(1,1)
            if (f == g)
                fooHeldByGooResult.push_back(BE_SQLITE_DONE);
            else
                fooHeldByGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(1,N), Target(1,1)
            if (g == 0)
                fooHeldByManyGooResult.push_back(BE_SQLITE_DONE);
            else
                fooHeldByManyGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);
            }
        }

    //1-1............................
    PersistedMapStrategy mapStrategy;
    size_t count_FooHeldByGoo = 0;
    {
    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, m_ecdb, GetRelationShipClassId("FooHeldByGoo")));
    ASSERT_EQ(PersistedMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooHeldByGoo", fooKeys, gooKeys, fooHeldByGooResult, count_FooHeldByGoo);
    }
    ASSERT_EQ(count_FooHeldByGoo, GetInsertedRelationshipsCount("ts.FooHeldByGoo"));

    //1-N...........................
    size_t count_FooHeldByManyGoo = 0;
    {
    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, m_ecdb, GetRelationShipClassId("FooHeldByManyGoo")));
    ASSERT_EQ(PersistedMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooHeldByManyGoo", fooKeys, gooKeys, fooHeldByManyGooResult, count_FooHeldByManyGoo);
    }
    ASSERT_EQ(count_FooHeldByManyGoo, GetInsertedRelationshipsCount("ts.FooHeldByManyGoo"));

    m_ecdb.Schemas().CreateECClassViewsInDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "DELETE FROM ts.Goo WHERE ECInstanceId=?"));
    //Delete gooKeys[0]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKeys[0].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHeldByGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooHeldByManyGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooHeldByManyGoo", fooKeys[0], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooHeldByManyGoo", fooKeys[0], gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.ManyFooHeldByManyGoo", fooKeys[0], gooKeys[0]));
    }

    //Delete gooKeys[1]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKeys[1].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHeldByGoo", fooKeys[1], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.ManyFooHeldByManyGoo", fooKeys[1], gooKeys[1]));
    }

    //Delete gooKeys[2]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKeys[2].GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHeldByGoo", fooKeys[2], gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.ManyFooHeldByManyGoo", fooKeys[2], gooKeys[2]));
    }
    }

END_ECDBUNITTESTS_NAMESPACE
