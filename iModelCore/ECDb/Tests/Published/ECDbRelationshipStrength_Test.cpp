/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbRelationshipStrength_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

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

END_ECDBUNITTESTS_NAMESPACE
