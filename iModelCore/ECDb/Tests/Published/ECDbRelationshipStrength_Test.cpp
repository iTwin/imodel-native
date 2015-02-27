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

extern bool InsertInstance (ECDbR db, ECClassCR ecClass, IECInstanceR ecInstance);
extern IECRelationshipInstancePtr CreateRelationship (ECRelationshipClassCR relationshipClass, IECInstanceR source, IECInstanceR target);
extern ECInstanceId InstanceToId (IECInstanceCR ecInstance);
extern IECInstancePtr CreatePerson (ECClassCR ecClass, WCharCP firstName, WCharCP lastName);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
int DeleteInstance (IECInstanceCR instance, ECDbR ecDb)
    {
    ECClassCR ecClass = instance.GetClass();

    ECInstanceDeleter deleter (ecDb, ecClass);

    if (!deleter.IsValid ())
        return -1;

    auto status = deleter.Delete (instance);
    if (status != SUCCESS)
        return -1;

    return deleter.GetDefaultEventHandler().GetInstancesAffectedCount ();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool HasInstance (IECInstanceCR instance, ECDbR ecDb)
    {
    ECSqlSelectBuilder builder;
    ECClassCR ecClass = instance.GetClass();
    Utf8PrintfString whereECInstanceId ("ECInstanceId = %lld", InstanceToId (instance).GetValue());
    builder.From (ecClass, false).SelectAll().Where (whereECInstanceId.c_str());

    ECSqlStatement statement;
    auto stat = statement.Prepare (ecDb, builder.ToString ().c_str ());
    BeAssert (stat == ECSqlStatus::Success);

    return statement.Step() == ECSqlStepStatus::HasRow;
    }

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/2014
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbInstances, RelationshipStrengthBackwardEmbedding)
    {
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create("BackwardRelationshipStrengthTest.ecdb", L"RelationshipStrengthBackwardTest.01.00.ecschema.xml", false);
    /*
    *                         SingleParent
    *                             |
    *                             | ChildrenHasSingleParent (Backward EMBEDDING)
    *      _______________________|__________________________________________________________________
    *     |                                                  |                                      |
    *   Child1                                             Child2                                 Child3
    */
    ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP(L"Person");
    IECInstancePtr child1 = CreatePerson(*personClass, L"First", L"Child");
    InsertInstance(ecdbr, *personClass, *child1);
    IECInstancePtr child2 = CreatePerson(*personClass, L"Second", L"Child");
    InsertInstance(ecdbr, *personClass, *child2);
    IECInstancePtr child3 = CreatePerson(*personClass, L"Third", L"Child");
    InsertInstance(ecdbr, *personClass, *child3);
    IECInstancePtr singleParent = CreatePerson(*personClass, L"Only", L"singleParent");
    InsertInstance(ecdbr, *personClass, *singleParent);

    //Backward Embedding relationship (SingleParent -> Child1, Child2)
    ECRelationshipClassP ChildrenHasSingleParent = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP(L"ChildrenHasSingleParent")->GetRelationshipClassP();
    IECRelationshipInstancePtr child1HasSingleParent;
    child1HasSingleParent = CreateRelationship (*ChildrenHasSingleParent, *child1, *singleParent);
    InsertInstance(ecdbr, *ChildrenHasSingleParent, *child1HasSingleParent);
    IECRelationshipInstancePtr child2HasSingleParent;
    child2HasSingleParent = CreateRelationship (*ChildrenHasSingleParent, *child2, *singleParent);
    InsertInstance(ecdbr, *ChildrenHasSingleParent, *child2HasSingleParent);
    IECRelationshipInstancePtr child3HasSingleParent;
    child3HasSingleParent = CreateRelationship (*ChildrenHasSingleParent, *child3, *singleParent);
    InsertInstance(ecdbr, *ChildrenHasSingleParent, *child3HasSingleParent);
    /*
    * Test 1: Delete Child1
    * Validate child1HasSingleParent,  child1 has been deleted
    */
    int numDeleted = DeleteInstance(*child1, ecdbr);
    ASSERT_EQ (2, numDeleted);
    ASSERT_FALSE (HasInstance (*child1, ecdbr));
    ASSERT_TRUE (HasInstance (*singleParent, ecdbr));
    ASSERT_FALSE(HasInstance(*child1HasSingleParent, ecdbr));
    ASSERT_TRUE (HasInstance(*child2HasSingleParent, ecdbr));
    ASSERT_TRUE(HasInstance(*child3HasSingleParent, ecdbr));
    ASSERT_TRUE(HasInstance(*child2, ecdbr));
    ASSERT_TRUE(HasInstance(*child3, ecdbr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, RelationshipStrength)
    {
    ECDbTestProject testProject;
    ECDbR ecDb = testProject.Create ("RelationshipStrengthTest.ecdb", L"RelationshipStrengthTest.01.00.ecschema.xml", false);

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

    ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP (L"Person");
    IECInstancePtr grandParent1 = CreatePerson (*personClass, L"First", L"GrandParent");
    InsertInstance (ecDb, *personClass, *grandParent1);
    IECInstancePtr grandParent2 = CreatePerson (*personClass, L"Second", L"GrandParent");
    InsertInstance (ecDb, *personClass, *grandParent2);
    IECInstancePtr singleParent = CreatePerson (*personClass, L"Only", L"SingleParent");
    InsertInstance (ecDb, *personClass, *singleParent);
    IECInstancePtr child1 = CreatePerson (*personClass, L"First", L"Child");
    InsertInstance (ecDb, *personClass, *child1);
    IECInstancePtr child2 = CreatePerson (*personClass, L"Second", L"Child");
    InsertInstance (ecDb, *personClass, *child2);

    // Holding relationship (GrandParent1, GrandParent2 -> SingleParent)
    ECRelationshipClassP manyParentsHaveChildren = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP (L"ManyParentsHaveChildren")->GetRelationshipClassP();
    IECRelationshipInstancePtr grandParent1HasSingleParent;
    grandParent1HasSingleParent = CreateRelationship (*manyParentsHaveChildren, *grandParent1, *singleParent);
    InsertInstance (ecDb, *manyParentsHaveChildren, *grandParent1HasSingleParent);
    IECRelationshipInstancePtr grandParent2HasSingleParent;
    grandParent2HasSingleParent = CreateRelationship (*manyParentsHaveChildren, *grandParent2, *singleParent);
    InsertInstance (ecDb, *manyParentsHaveChildren, *grandParent2HasSingleParent);

    // Embedding relationship (SingleParent -> Child1, Child2)
    ECRelationshipClassP singleParentHasChildren = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP (L"SingleParentHasChildren")->GetRelationshipClassP();
    IECRelationshipInstancePtr singleParentHasChild1;
    singleParentHasChild1 = CreateRelationship (*singleParentHasChildren, *singleParent, *child1);
    InsertInstance (ecDb, *singleParentHasChildren, *singleParentHasChild1);
    IECRelationshipInstancePtr singleParentHasChild2;
    singleParentHasChild2 = CreateRelationship (*singleParentHasChildren, *singleParent, *child2);
    InsertInstance (ecDb, *singleParentHasChildren, *singleParentHasChild2);

    // Referencing relationship (GrandParent1 <-> GrandParent2)
    ECRelationshipClassP parentHasSpouse = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP (L"ParentHasSpouse")->GetRelationshipClassP();
    IECRelationshipInstancePtr grandParent1HasSpouse;
    grandParent1HasSpouse = CreateRelationship (*parentHasSpouse, *grandParent1, *grandParent2);
    InsertInstance (ecDb, *parentHasSpouse, *grandParent1HasSpouse);
    IECRelationshipInstancePtr grandParent2HasSpouse;
    grandParent2HasSpouse = CreateRelationship (*parentHasSpouse, *grandParent2, *grandParent1);
    InsertInstance (ecDb, *parentHasSpouse, *grandParent2HasSpouse);

    ecDb.SaveChanges();

    /*
    * Test 1: Delete GrandParent1
    * Validate grandParent1HasSpouse, grandParent2HasSpouse, grandParent1HasSingleParent have been deleted (orphaned relationships)
    * Validate singleParent is still around (holding relationship with one parent remaining)
    */
    ASSERT_TRUE(HasInstance(*grandParent1HasSpouse, ecDb));
    ASSERT_TRUE(HasInstance(*grandParent2HasSpouse, ecDb));

    int numDeleted = DeleteInstance(*grandParent1, ecDb);
    ASSERT_EQ(4, numDeleted);

    ASSERT_FALSE(HasInstance(*grandParent1, ecDb));
    ASSERT_FALSE(HasInstance(*grandParent1HasSpouse, ecDb));
    ASSERT_FALSE(HasInstance(*grandParent2HasSpouse, ecDb));
    ASSERT_FALSE(HasInstance(*grandParent1HasSingleParent, ecDb));
    ASSERT_TRUE(HasInstance(*singleParent, ecDb));

    /*
    * Test 2: Delete GrandParent2
    * Validate grandParent2HasSingleParent has been deleted (orphaned relationship)
    * Validate singeParent has been deleted (held instance with no parents remaining)
    */
    numDeleted = DeleteInstance(*grandParent2, ecDb);
    ASSERT_EQ(7, numDeleted);

    ASSERT_FALSE(HasInstance(*grandParent2, ecDb));
    ASSERT_FALSE(HasInstance(*grandParent2HasSingleParent, ecDb));
    ASSERT_FALSE(HasInstance(*singleParent, ecDb));
    ASSERT_FALSE(HasInstance(*singleParentHasChild1, ecDb));
    ASSERT_FALSE(HasInstance(*singleParentHasChild2, ecDb));
    ASSERT_FALSE(HasInstance(*child1, ecDb));
    ASSERT_FALSE(HasInstance(*child2, ecDb));
    }

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/2014
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbInstances, RelationshipStrengthBackwardHoldingForwardEmbedding)
{
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create("BackwardRelationshipStrengthTest.ecdb", L"RelationshipStrengthBackwardTest.01.00.ecschema.xml", false);

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
    ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP(L"Person");
    IECInstancePtr child1 = CreatePerson(*personClass, L"First", L"Child");
    InsertInstance(ecdbr, *personClass, *child1);
    IECInstancePtr child2 = CreatePerson(*personClass, L"Second", L"Child");
    InsertInstance(ecdbr, *personClass, *child2);
    IECInstancePtr singleParent = CreatePerson(*personClass, L"Only", L"singleParent");
    InsertInstance(ecdbr, *personClass, *singleParent);
    IECInstancePtr grandParent1 = CreatePerson(*personClass, L"First", L"GrandParent");
    InsertInstance(ecdbr, *personClass, *grandParent1);
    IECInstancePtr grandParent2 = CreatePerson(*personClass, L"Second", L"GrandParent");
    InsertInstance(ecdbr, *personClass, *grandParent2);

    // Backward Holding relationship (GrandParent1, GrandParent2 <- SingleParent)
    ECRelationshipClassP ChildrenHaveManyParents = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP(L"ChildrenHaveManyParents")->GetRelationshipClassP();
    IECRelationshipInstancePtr singleParentHasGrandParent1;
    singleParentHasGrandParent1 = CreateRelationship (*ChildrenHaveManyParents, *singleParent, *grandParent1);
    InsertInstance(ecdbr, *ChildrenHaveManyParents, *singleParentHasGrandParent1);
    IECRelationshipInstancePtr singleParentHasGrandParent2;
    singleParentHasGrandParent2 = CreateRelationship (*ChildrenHaveManyParents, *singleParent, *grandParent2);
    InsertInstance(ecdbr, *ChildrenHaveManyParents, *singleParentHasGrandParent2);

    //Forward Embedding relationship (SingleParent -> Child1, Child2)
    ECRelationshipClassP SingleParentHasChildren = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP(L"SingleParentHasChildren")->GetRelationshipClassP();
    IECRelationshipInstancePtr singleParentHasChild1;
    singleParentHasChild1 = CreateRelationship(*SingleParentHasChildren, *singleParent, *child1);
    InsertInstance(ecdbr, *SingleParentHasChildren, *singleParentHasChild1);
    IECRelationshipInstancePtr singleParentHasChild2;
    singleParentHasChild2 = CreateRelationship(*SingleParentHasChildren, *singleParent, *child2);
    InsertInstance(ecdbr, *SingleParentHasChildren, *singleParentHasChild2);

    //Backward Referencing relationship (GrandParent1 <-> GrandParent2)
    ECRelationshipClassP parentHasSpouse = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP(L"ParentHasSpouse")->GetRelationshipClassP();
    IECRelationshipInstancePtr grandParent1HasSpouse;
    grandParent1HasSpouse = CreateRelationship(*parentHasSpouse, *grandParent1, *grandParent2);
    InsertInstance(ecdbr, *parentHasSpouse, *grandParent1HasSpouse);
    IECRelationshipInstancePtr grandParent2HasSpouse;
    grandParent2HasSpouse = CreateRelationship(*parentHasSpouse, *grandParent2, *grandParent1);
    InsertInstance(ecdbr, *parentHasSpouse, *grandParent2HasSpouse);

    ecdbr.SaveChanges();

    /*
    * Test 1: Delete Child1
    * Validate Child1 and singleParentHasChild1 have been deleted
    */
    ASSERT_TRUE(HasInstance(*singleParentHasChild1, ecdbr));

    int numDeleted = DeleteInstance(*child1, ecdbr);
    ASSERT_EQ(2, numDeleted);

    ASSERT_FALSE(HasInstance(*child1, ecdbr));
    ASSERT_FALSE(HasInstance(*singleParentHasChild1, ecdbr));
    ASSERT_TRUE(HasInstance(*child2, ecdbr));

    /*
    * Test 2: Delete Child2
    * Validate Child2 and singleParentHasChild2 have been deleted
    * Validate singleParent is still around (relationship grand parents remaining)
    */
    ASSERT_TRUE(HasInstance(*singleParentHasChild2, ecdbr));

    numDeleted = DeleteInstance(*child2, ecdbr);
    ASSERT_EQ(2, numDeleted);

    ASSERT_FALSE(HasInstance(*child2, ecdbr));
    ASSERT_FALSE(HasInstance(*singleParentHasChild2, ecdbr));
    ASSERT_TRUE(HasInstance(*singleParent, ecdbr));

    /*
    * Test 3: Delete GrandParent1
    * Validate GrandParent1, grandParent1HasSpouse, grandParent2HasSpouse, singleParentHasGrandParent1 have been deleted
    * Validate singleParent is still around (holding relationship with one parent remaining)
    */
    numDeleted = DeleteInstance(*grandParent1, ecdbr);
    ASSERT_EQ(4, numDeleted);
    ASSERT_FALSE(HasInstance(*grandParent1, ecdbr));
    ASSERT_FALSE(HasInstance(*grandParent1HasSpouse, ecdbr));
    ASSERT_FALSE(HasInstance(*grandParent2HasSpouse, ecdbr));
    ASSERT_FALSE(HasInstance(*singleParentHasGrandParent1, ecdbr));
    ASSERT_TRUE(HasInstance(*singleParent, ecdbr));

    /*
    * Test 3: Delete GrandParent2
    * Validate GrandParent2, singleParentHasGrandParent2 have been deleted.
    * Single parent has been deleted too as no parent exists anymore
    */
    numDeleted = DeleteInstance(*grandParent2, ecdbr);
    ASSERT_EQ(3, numDeleted);
    ASSERT_FALSE(HasInstance(*singleParentHasGrandParent2, ecdbr));
    ASSERT_FALSE (HasInstance(*singleParent, ecdbr));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances, RelationshipStrengthAnyClass)
    {
    ECDbTestProject testProject;
    ECDbR ecDb = testProject.Create ("RelationshipStrengthAnyClassTest.ecdb", L"RelationshipStrengthTest.01.00.ecschema.xml", false);

    /*
     *          Create the following relationship hierarchy
     * 
     * GrandParent1                                      GrandParent2
     *     |__________________________________________________|
     *                             | 
     *                             | ParentsHaveAnyChildren (HOLDING)
     *                             |
     *                         SingleParent
     *                             | 
     *                             | AnyChildHasParent (EMBEDDING)
     *      _______________________|__________________________ 
     *     |                                                  |
     *   Child1                                             Child2
     * 
     */

    ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP (L"Person");
    IECInstancePtr grandParent1 = CreatePerson (*personClass, L"First", L"GrandParent");
    InsertInstance (ecDb, *personClass, *grandParent1);
    IECInstancePtr grandParent2 = CreatePerson (*personClass, L"Second", L"GrandParent");
    InsertInstance (ecDb, *personClass, *grandParent2);
    IECInstancePtr singleParent = CreatePerson (*personClass, L"Only", L"SingleParent");
    InsertInstance (ecDb, *personClass, *singleParent);
    IECInstancePtr child1 = CreatePerson (*personClass, L"First", L"Child");
    InsertInstance (ecDb, *personClass, *child1);
    IECInstancePtr child2 = CreatePerson (*personClass, L"Second", L"Child");
    InsertInstance (ecDb, *personClass, *child2);

    // Holding relationship (GrandParent1, GrandParent2 -> SingleParent)
    ECRelationshipClassP parentsHaveAnyChildren = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP (L"ParentsHaveAnyChildren")->GetRelationshipClassP();
    IECRelationshipInstancePtr grandParent1HasSingleParent;
    grandParent1HasSingleParent = CreateRelationship (*parentsHaveAnyChildren, *grandParent1, *singleParent);
    InsertInstance (ecDb, *parentsHaveAnyChildren, *grandParent1HasSingleParent);
    IECRelationshipInstancePtr grandParent2HasSingleParent;
    grandParent2HasSingleParent = CreateRelationship (*parentsHaveAnyChildren, *grandParent2, *singleParent);
    InsertInstance (ecDb, *parentsHaveAnyChildren, *grandParent2HasSingleParent);

    // Embedding relationship (SingleParent -> Child1, Child2)
    ECRelationshipClassP parentHasAnyChild = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP (L"ParentHasAnyChild")->GetRelationshipClassP();
    IECRelationshipInstancePtr singleParentHasChild1;
    singleParentHasChild1 = CreateRelationship (*parentHasAnyChild, *singleParent, *child1);
    InsertInstance (ecDb, *parentHasAnyChild, *singleParentHasChild1);
    IECRelationshipInstancePtr singleParentHasChild2;
    singleParentHasChild2 = CreateRelationship (*parentHasAnyChild, *singleParent, *child2);
    InsertInstance (ecDb, *parentHasAnyChild, *singleParentHasChild2);

    ecDb.SaveChanges();

    /*
     * Test 1: Delete GrandParent1
     * Validate grandParent1HasSingleParent have been deleted (orphaned relationships)
     * Validate singleParent is still around (holding relationship with one parent remaining)
     */
    int numDeleted = DeleteInstance (*grandParent1, ecDb);
    ASSERT_EQ (2, numDeleted);

    ASSERT_FALSE (HasInstance (*grandParent1, ecDb));
    ASSERT_FALSE (HasInstance (*grandParent1HasSingleParent, ecDb));
    ASSERT_TRUE (HasInstance (*singleParent, ecDb));
    
    /*
     * Test 2: Delete GrandParent2
     * Validate grandParent2HasSingleParent has been deleted (orphaned relationship)
     * Validate singeParent has been deleted (held instance with no parents remaining)
     */
    numDeleted = DeleteInstance (*grandParent2, ecDb);
    ASSERT_EQ (7, numDeleted);

    ASSERT_FALSE (HasInstance (*grandParent2, ecDb));
    ASSERT_FALSE (HasInstance (*grandParent2HasSingleParent, ecDb));
    ASSERT_FALSE (HasInstance (*singleParent, ecDb));
    ASSERT_FALSE (HasInstance (*singleParentHasChild1, ecDb));
    ASSERT_FALSE (HasInstance (*singleParentHasChild2, ecDb));
    ASSERT_FALSE (HasInstance (*child1, ecDb));
    ASSERT_FALSE (HasInstance (*child2, ecDb));
    }

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/2014
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbInstances, RelationshipStrengthAnyClassBackwardHoldingBackwardEmbedding)
{
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create("RelationshipStrengthAnyClassTest.ecdb", L"RelationshipStrengthBackwardTest.01.00.ecschema.xml", false);

    /*
    *          Create the following relationship hierarchy
    *
    * GrandParent1                                      GrandParent2
    *     |__________________________________________________|
    *                             |
    *                             | AnyChildrenHaveParents (Backward HOLDING)
    *                             |
    *                         SingleParent
    *                             |
    *                             | ParentHasAnyChild (Backward EMBEDDING)
    *      _______________________|__________________________
    *     |                                                  |
    *   Child1                                             Child2
    *
    */

    ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP(L"Person");
    IECInstancePtr child1 = CreatePerson(*personClass, L"First", L"Child");
    InsertInstance(ecdbr, *personClass, *child1);
    IECInstancePtr child2 = CreatePerson(*personClass, L"Second", L"Child");
    InsertInstance(ecdbr, *personClass, *child2);
    IECInstancePtr singleParent = CreatePerson(*personClass, L"Only", L"singleParent");
    InsertInstance(ecdbr, *personClass, *singleParent);
    IECInstancePtr grandParent1 = CreatePerson(*personClass, L"First", L"GrandParent");
    InsertInstance(ecdbr, *personClass, *grandParent1);
    IECInstancePtr grandParent2 = CreatePerson(*personClass, L"Second", L"GrandParent");
    InsertInstance(ecdbr, *personClass, *grandParent2);

    //Backward Holding relationship (GrandParent1, GrandParent2 <- SingleParent)
    ECRelationshipClassP AnyChildrenHaveParents = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP(L"AnyChildrenHaveParents")->GetRelationshipClassP();
    IECRelationshipInstancePtr singleParentHasGrandParent1;
    singleParentHasGrandParent1 = CreateRelationship(*AnyChildrenHaveParents, *grandParent1, *singleParent);
    InsertInstance(ecdbr, *AnyChildrenHaveParents, *singleParentHasGrandParent1);
    IECRelationshipInstancePtr singleParentHasGrandParent2;
    singleParentHasGrandParent2 = CreateRelationship(*AnyChildrenHaveParents, *grandParent2, *singleParent);
    InsertInstance(ecdbr, *AnyChildrenHaveParents, *singleParentHasGrandParent2);

    // Embedding relationship (SingleParent <- Child1, Child2)
    ECRelationshipClassP AnyChildHasParent = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP(L"AnyChildHasParent")->GetRelationshipClassP();
    IECRelationshipInstancePtr child1HasSingleParent;
    child1HasSingleParent = CreateRelationship(*AnyChildHasParent, *singleParent, *child1);
    InsertInstance(ecdbr, *AnyChildHasParent, *child1HasSingleParent);
    IECRelationshipInstancePtr child2HasSingleParent;
    child2HasSingleParent = CreateRelationship(*AnyChildHasParent, *singleParent, *child2);
    InsertInstance(ecdbr, *AnyChildHasParent, *child2HasSingleParent);

    ecdbr.SaveChanges();

    /*
    * Test 1: Delete singleParent
    * Validate child1HasSingleParent, child2HasSingleParent, singleParentHasGrandParent1, singleParentHasGrandParent2, grandParent1, grandParent2 have been deleted
    * Validate child1 and child2 are still around (Backward Embedding relationship with child1 and child2)
    */
    int numDeleted = DeleteInstance(*singleParent, ecdbr);
    ASSERT_EQ(7, numDeleted);
    ASSERT_FALSE (HasInstance(*child1HasSingleParent, ecdbr));
    ASSERT_FALSE (HasInstance(*child2HasSingleParent, ecdbr));
    ASSERT_FALSE (HasInstance(*singleParentHasGrandParent1, ecdbr));
    ASSERT_FALSE (HasInstance(*singleParentHasGrandParent2, ecdbr));
    ASSERT_FALSE (HasInstance(*grandParent1, ecdbr));
    ASSERT_FALSE (HasInstance(*grandParent2, ecdbr));
    ASSERT_TRUE (HasInstance(*child2, ecdbr));
    ASSERT_TRUE(HasInstance(*child1, ecdbr));
}
END_ECDBUNITTESTS_NAMESPACE

