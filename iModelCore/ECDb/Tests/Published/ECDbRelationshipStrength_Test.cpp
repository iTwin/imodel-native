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
extern IECInstancePtr CreatePerson (ECClassCR ecClass, Utf8CP firstName, Utf8CP lastName);

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
    //ECDB_AFFECTEDROWS
    //return deleter.GetDefaultEventHandler().GetInstancesAffectedCount ();
    return 0;
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
    ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("Person");
    IECInstancePtr child1 = CreatePerson(*personClass, "First", "Child");
    InsertInstance(ecdbr, *personClass, *child1);
    IECInstancePtr child2 = CreatePerson(*personClass, "Second", "Child");
    InsertInstance(ecdbr, *personClass, *child2);
    IECInstancePtr child3 = CreatePerson(*personClass, "Third", "Child");
    InsertInstance(ecdbr, *personClass, *child3);
    IECInstancePtr singleParent = CreatePerson(*personClass, "Only", "singleParent");
    InsertInstance(ecdbr, *personClass, *singleParent);

    //Backward Embedding relationship (SingleParent -> Child1, Child2)
    ECRelationshipClassP ChildrenHasSingleParent = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ChildrenHasSingleParent")->GetRelationshipClassP();
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
    /*int numDeleted = */ DeleteInstance(*child1, ecdbr);
    //ASSERT_EQ (2, numDeleted);
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

    ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP ("Person");
    IECInstancePtr grandParent1 = CreatePerson (*personClass, "First", "GrandParent");
    InsertInstance (ecDb, *personClass, *grandParent1);
    IECInstancePtr grandParent2 = CreatePerson (*personClass, "Second", "GrandParent");
    InsertInstance (ecDb, *personClass, *grandParent2);
    IECInstancePtr singleParent = CreatePerson (*personClass, "Only", "SingleParent");
    InsertInstance (ecDb, *personClass, *singleParent);
    IECInstancePtr child1 = CreatePerson (*personClass, "First", "Child");
    InsertInstance (ecDb, *personClass, *child1);
    IECInstancePtr child2 = CreatePerson (*personClass, "Second", "Child");
    InsertInstance (ecDb, *personClass, *child2);

    // Holding relationship (GrandParent1, GrandParent2 -> SingleParent)
    ECRelationshipClassP manyParentsHaveChildren = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP ("ManyParentsHaveChildren")->GetRelationshipClassP();
    IECRelationshipInstancePtr grandParent1HasSingleParent;
    grandParent1HasSingleParent = CreateRelationship (*manyParentsHaveChildren, *grandParent1, *singleParent);
    InsertInstance (ecDb, *manyParentsHaveChildren, *grandParent1HasSingleParent);
    IECRelationshipInstancePtr grandParent2HasSingleParent;
    grandParent2HasSingleParent = CreateRelationship (*manyParentsHaveChildren, *grandParent2, *singleParent);
    InsertInstance (ecDb, *manyParentsHaveChildren, *grandParent2HasSingleParent);

    // Embedding relationship (SingleParent -> Child1, Child2)
    ECRelationshipClassP singleParentHasChildren = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP ("SingleParentHasChildren")->GetRelationshipClassP();
    IECRelationshipInstancePtr singleParentHasChild1;
    singleParentHasChild1 = CreateRelationship (*singleParentHasChildren, *singleParent, *child1);
    InsertInstance (ecDb, *singleParentHasChildren, *singleParentHasChild1);
    IECRelationshipInstancePtr singleParentHasChild2;
    singleParentHasChild2 = CreateRelationship (*singleParentHasChildren, *singleParent, *child2);
    InsertInstance (ecDb, *singleParentHasChildren, *singleParentHasChild2);

    // Referencing relationship (GrandParent1 <-> GrandParent2)
    ECRelationshipClassP parentHasSpouse = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP ("ParentHasSpouse")->GetRelationshipClassP();
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

    /*int numDeleted = */ DeleteInstance(*grandParent1, ecDb);
    //ASSERT_EQ(4, numDeleted);

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
    /*numDeleted = */DeleteInstance(*grandParent2, ecDb);
    //ASSERT_EQ(7, numDeleted);
    ecDb.SaveChanges ();
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
    ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("Person");
    IECInstancePtr child1 = CreatePerson(*personClass, "First", "Child");
    InsertInstance(ecdbr, *personClass, *child1);
    IECInstancePtr child2 = CreatePerson(*personClass, "Second", "Child");
    InsertInstance(ecdbr, *personClass, *child2);
    IECInstancePtr singleParent = CreatePerson(*personClass, "Only", "singleParent");
    InsertInstance(ecdbr, *personClass, *singleParent);
    IECInstancePtr grandParent1 = CreatePerson(*personClass, "First", "GrandParent");
    InsertInstance(ecdbr, *personClass, *grandParent1);
    IECInstancePtr grandParent2 = CreatePerson(*personClass, "Second", "GrandParent");
    InsertInstance(ecdbr, *personClass, *grandParent2);

    // Backward Holding relationship (GrandParent1, GrandParent2 <- SingleParent)
    ECRelationshipClassP ChildrenHaveManyParents = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ChildrenHaveManyParents")->GetRelationshipClassP();
    IECRelationshipInstancePtr singleParentHasGrandParent1;
    singleParentHasGrandParent1 = CreateRelationship (*ChildrenHaveManyParents, *singleParent, *grandParent1);
    InsertInstance(ecdbr, *ChildrenHaveManyParents, *singleParentHasGrandParent1);
    IECRelationshipInstancePtr singleParentHasGrandParent2;
    singleParentHasGrandParent2 = CreateRelationship (*ChildrenHaveManyParents, *singleParent, *grandParent2);
    InsertInstance(ecdbr, *ChildrenHaveManyParents, *singleParentHasGrandParent2);

    //Forward Embedding relationship (SingleParent -> Child1, Child2)
    ECRelationshipClassP SingleParentHasChildren = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("SingleParentHasChildren")->GetRelationshipClassP();
    IECRelationshipInstancePtr singleParentHasChild1;
    singleParentHasChild1 = CreateRelationship(*SingleParentHasChildren, *singleParent, *child1);
    InsertInstance(ecdbr, *SingleParentHasChildren, *singleParentHasChild1);
    IECRelationshipInstancePtr singleParentHasChild2;
    singleParentHasChild2 = CreateRelationship(*SingleParentHasChildren, *singleParent, *child2);
    InsertInstance(ecdbr, *SingleParentHasChildren, *singleParentHasChild2);

    //Backward Referencing relationship (GrandParent1 <-> GrandParent2)
    ECRelationshipClassP parentHasSpouse = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ParentHasSpouse")->GetRelationshipClassP();
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

    /*int numDeleted =*/ DeleteInstance(*child1, ecdbr);
    //ASSERT_EQ(2, numDeleted);

    ASSERT_FALSE(HasInstance(*child1, ecdbr));
    ASSERT_FALSE(HasInstance(*singleParentHasChild1, ecdbr));
    ASSERT_TRUE(HasInstance(*child2, ecdbr));

    /*
    * Test 2: Delete Child2
    * Validate Child2 and singleParentHasChild2 have been deleted
    * Validate singleParent is still around (relationship grand parents remaining)
    */
    ASSERT_TRUE(HasInstance(*singleParentHasChild2, ecdbr));

    /*numDeleted = */ DeleteInstance(*child2, ecdbr);
    //ASSERT_EQ(2, numDeleted);

    ASSERT_FALSE(HasInstance(*child2, ecdbr));
    ASSERT_FALSE(HasInstance(*singleParentHasChild2, ecdbr));
    ASSERT_TRUE(HasInstance(*singleParent, ecdbr));

    /*
    * Test 3: Delete GrandParent1
    * Validate GrandParent1, grandParent1HasSpouse, grandParent2HasSpouse, singleParentHasGrandParent1 have been deleted
    * Validate singleParent is still around (holding relationship with one parent remaining)
    */
    /*numDeleted =*/ DeleteInstance(*grandParent1, ecdbr);
    //ASSERT_EQ(4, numDeleted);
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
    /*numDeleted = */ DeleteInstance(*grandParent2, ecdbr);
    //ASSERT_EQ(3, numDeleted);
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
     *                             | ParentHasAnyChild (EMBEDDING)
     *      _______________________|__________________________ 
     *     |                                                  |
     *   Child1                                             Child2
     * 
     */

    ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP ("Person");
    IECInstancePtr grandParent1 = CreatePerson (*personClass, "First", "GrandParent");
    InsertInstance (ecDb, *personClass, *grandParent1);
    IECInstancePtr grandParent2 = CreatePerson (*personClass, "Second", "GrandParent");
    InsertInstance (ecDb, *personClass, *grandParent2);
    IECInstancePtr singleParent = CreatePerson (*personClass, "Only", "SingleParent");
    InsertInstance (ecDb, *personClass, *singleParent);
    IECInstancePtr child1 = CreatePerson (*personClass, "First", "Child");
    InsertInstance (ecDb, *personClass, *child1);
    IECInstancePtr child2 = CreatePerson (*personClass, "Second", "Child");
    InsertInstance (ecDb, *personClass, *child2);

    // Holding relationship (GrandParent1, GrandParent2 -> SingleParent)
    ECRelationshipClassP parentsHaveAnyChildren = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP ("ParentsHaveAnyChildren")->GetRelationshipClassP();
    IECRelationshipInstancePtr grandParent1HasSingleParent;
    grandParent1HasSingleParent = CreateRelationship (*parentsHaveAnyChildren, *grandParent1, *singleParent);
    InsertInstance (ecDb, *parentsHaveAnyChildren, *grandParent1HasSingleParent);
    IECRelationshipInstancePtr grandParent2HasSingleParent;
    grandParent2HasSingleParent = CreateRelationship (*parentsHaveAnyChildren, *grandParent2, *singleParent);
    InsertInstance (ecDb, *parentsHaveAnyChildren, *grandParent2HasSingleParent);

    // Embedding relationship (SingleParent -> Child1, Child2)
    ECRelationshipClassP parentHasAnyChild = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP ("ParentHasAnyChild")->GetRelationshipClassP();
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
    /*int numDeleted =*/ DeleteInstance (*grandParent1, ecDb);
    //ASSERT_EQ (2, numDeleted);

    ASSERT_FALSE (HasInstance (*grandParent1, ecDb));
    ASSERT_FALSE (HasInstance (*grandParent1HasSingleParent, ecDb));
    ASSERT_TRUE (HasInstance (*singleParent, ecDb));
    
    /*
     * Test 2: Delete GrandParent2
     * Validate grandParent2HasSingleParent has been deleted (orphaned relationship)
     * Validate singeParent has been deleted (held instance with no parents remaining)
     */
    /*numDeleted =*/ DeleteInstance (*grandParent2, ecDb);
    //ASSERT_EQ (7, numDeleted);

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
    *                             | AnyChildrenHaveParents (0:N Backward HOLDING)
    *                             |
    *                         SingleParent
    *                             |
    *                             | AnyChildHasParent (N:1 Backward EMBEDDING)
    *      _______________________|__________________________
    *     |                                                  |
    *   Child1                                             Child2
    *
    */

    ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("Person");
    IECInstancePtr child1 = CreatePerson(*personClass, "First", "Child");
    InsertInstance(ecdbr, *personClass, *child1);
    IECInstancePtr child2 = CreatePerson(*personClass, "Second", "Child");
    InsertInstance(ecdbr, *personClass, *child2);
    IECInstancePtr singleParent = CreatePerson(*personClass, "Only", "singleParent");
    InsertInstance(ecdbr, *personClass, *singleParent);
    IECInstancePtr grandParent1 = CreatePerson(*personClass, "First", "GrandParent");
    InsertInstance(ecdbr, *personClass, *grandParent1);
    IECInstancePtr grandParent2 = CreatePerson(*personClass, "Second", "GrandParent");
    InsertInstance(ecdbr, *personClass, *grandParent2);

    //Backward Holding relationship (GrandParent1, GrandParent2 <- SingleParent)
    ECRelationshipClassP AnyChildrenHaveParents = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("AnyChildrenHaveParents")->GetRelationshipClassP();
    IECRelationshipInstancePtr singleParentHasGrandParent1;
    singleParentHasGrandParent1 = CreateRelationship(*AnyChildrenHaveParents, *singleParent, *grandParent1);
    InsertInstance(ecdbr, *AnyChildrenHaveParents, *singleParentHasGrandParent1);
    IECRelationshipInstancePtr singleParentHasGrandParent2;
    singleParentHasGrandParent2 = CreateRelationship(*AnyChildrenHaveParents, *singleParent, *grandParent2);
    InsertInstance(ecdbr, *AnyChildrenHaveParents, *singleParentHasGrandParent2);

    // Embedding relationship (SingleParent <- Child1, Child2)
    ECRelationshipClassP AnyChildHasParent = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("AnyChildHasParent")->GetRelationshipClassP();
    IECRelationshipInstancePtr child1HasSingleParent;
    child1HasSingleParent = CreateRelationship(*AnyChildHasParent, *child1, *singleParent);
    InsertInstance(ecdbr, *AnyChildHasParent, *child1HasSingleParent);
    IECRelationshipInstancePtr child2HasSingleParent;
    child2HasSingleParent = CreateRelationship(*AnyChildHasParent, *child2, *singleParent);
    InsertInstance(ecdbr, *AnyChildHasParent, *child2HasSingleParent);

    ecdbr.SaveChanges();

    /*
    * Test 1: Delete singleParent
    * Validate grandParent1 and grandParent2 are still around (all others deleted)
    */
    /*int numDeleted = */DeleteInstance(*singleParent, ecdbr);
    //ASSERT_EQ(7, numDeleted);
    ASSERT_FALSE (HasInstance(*child1HasSingleParent, ecdbr));
    ASSERT_FALSE (HasInstance(*child2HasSingleParent, ecdbr));
    ASSERT_FALSE (HasInstance(*singleParentHasGrandParent1, ecdbr));
    ASSERT_FALSE (HasInstance(*singleParentHasGrandParent2, ecdbr));
    ASSERT_TRUE (HasInstance(*grandParent1, ecdbr));
    ASSERT_TRUE(HasInstance(*grandParent2, ecdbr));
    ASSERT_FALSE (HasInstance(*child2, ecdbr));
    ASSERT_FALSE(HasInstance(*child1, ecdbr));
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Rafay.Muneeb	                     03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstancesRelationshipStrength, ForwardHoldingByMultipleDeleteTest)
{
    /*
    Create the following relationship hierarchy


    GrandParent1 --►--►---►(Embedding)►-----►------►----Pet1
    ↓													↑
    ↓													↑
    (HOLDING)											(Embedding)
    ↓													↑
    Parent1-----►--►---►(Embedding)►-----►------►----Child1
                                                        ↑
                                                        ↑
                                                        (Embedding)
                                                        ↑
                                                        GrandParent2
    */

	//int numDeleted = 0;
	bool InsertionStatus;
	ECDbTestProject testProject;
	ECDbR ecDb = testProject.Create("RelationshipStrengthDifferentLevelHierarchyTest.ecdb", L"RelationshipStrengthTest.01.00.ecschema.xml", false);

	//grandParent1
	ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("Person");
	IECInstancePtr grandParent1 = CreatePerson(*personClass, "First", "GrandParent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent1));
	//grandParent2
	IECInstancePtr grandParent2 = CreatePerson(*personClass, "Second", "GrandParent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent2));
	//Parent1
	IECInstancePtr Parent1 = CreatePerson(*personClass, "First", "Parent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent1));
	//Parent2
	IECInstancePtr Parent2 = CreatePerson(*personClass, "Second", "Parent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent2));
	//child1    
	IECInstancePtr child1 = CreatePerson(*personClass, "First", "Child");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child1));
	//child2
	IECInstancePtr child2 = CreatePerson(*personClass, "Second", "Child");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child2));
	//pet1
	IECInstancePtr pet1 = CreatePerson(*personClass, "First", "pet");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet1));
	//pet2
	IECInstancePtr pet2 = CreatePerson(*personClass, "Second", "pet");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet2));


	// Holding relationship (GrandParent2 -> Parent2 -> child2 -> pet2 )
	ECRelationshipClassP HoldingRelationShipClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ParentsHaveAnyChildren")->GetRelationshipClassP();

	//GrandParent1->Parent1
	IECRelationshipInstancePtr RelationInstanceGP1_P1 = CreateRelationship(*HoldingRelationShipClass, *grandParent1, *Parent1);
	InsertionStatus = InsertInstance(ecDb, *HoldingRelationShipClass, *RelationInstanceGP1_P1);
	ASSERT_TRUE(InsertionStatus);

	//Parent1->child1
	IECRelationshipInstancePtr RelationInstanceC1_P1 = CreateRelationship(*HoldingRelationShipClass, *Parent1, *child1);
	InsertionStatus = InsertInstance(ecDb, *HoldingRelationShipClass, *RelationInstanceC1_P1);
	ASSERT_TRUE(InsertionStatus);

	//child1->pet1
	IECRelationshipInstancePtr RelationInstancePET1_C1 = CreateRelationship(*HoldingRelationShipClass, *child1, *pet1);
	InsertionStatus = InsertInstance(ecDb, *HoldingRelationShipClass, *RelationInstancePET1_C1);
	ASSERT_TRUE(InsertionStatus);

	//GrandParent1->pet1
	IECRelationshipInstancePtr RelationInstanceGP1_PET1 = CreateRelationship(*HoldingRelationShipClass, *grandParent1, *pet1);
	InsertionStatus = InsertInstance(ecDb, *HoldingRelationShipClass, *RelationInstanceGP1_PET1);
	ASSERT_TRUE(InsertionStatus);

	//GrandParent1->child1
	IECRelationshipInstancePtr RelationInstanceGP2_C1 = CreateRelationship(*HoldingRelationShipClass, *grandParent2, *child1);
	InsertionStatus = InsertInstance(ecDb, *HoldingRelationShipClass, *RelationInstanceGP2_C1);
	ASSERT_TRUE(InsertionStatus);



	ecDb.SaveChanges();
	/*
	Test 1: Delete grandParent1
	Validate:Parent1 should exist which has holding relationship from 2 direct path and 1 indirect path
	*/
	/*numDeleted =*/ DeleteInstance(*grandParent1, ecDb);
	//ASSERT_EQ(5, numDeleted);
	ASSERT_TRUE(HasInstance(*pet1, ecDb));
	ASSERT_TRUE(HasInstance(*child1, ecDb));

	ASSERT_FALSE(HasInstance(*Parent1, ecDb));

}
/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Rafay.Muneeb	                     03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstancesRelationshipStrength, ForwardHoldingMultipleLinkedObjectDeleteTest)
{
    /*
    Create the following relationship hierarchy


    GrandParent1 --►--►---►(Embedding)►-----►------►----Pet1
    ↓													↑
    ↓													↑
    (HOLDING)											(Embedding)
    ↓													↑
    Parent1-----►--►---►(Embedding)►-----►------►----Child1
                                                        ↑
                                                        ↑
                                                        (Embedding)
                                                        ↑
                                                        GrandParent2
    */

	//int numDeleted = 0;
	bool InsertionStatus;
	ECDbTestProject testProject;
	ECDbR ecDb = testProject.Create("RelationshipStrengthDifferentLevelHierarchyTest.ecdb", L"RelationshipStrengthTest.01.00.ecschema.xml", false);

	//grandParent1
	ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("Person");
	IECInstancePtr grandParent1 = CreatePerson(*personClass, "First", "GrandParent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent1));
	//grandParent2
	IECInstancePtr grandParent2 = CreatePerson(*personClass, "Second", "GrandParent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent2));
	//Parent1
	IECInstancePtr Parent1 = CreatePerson(*personClass, "First", "Parent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent1));
	//Parent2
	IECInstancePtr Parent2 = CreatePerson(*personClass, "Second", "Parent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent2));
	//child1    
	IECInstancePtr child1 = CreatePerson(*personClass, "First", "Child");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child1));
	//child2
	IECInstancePtr child2 = CreatePerson(*personClass, "Second", "Child");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child2));
	//pet1
	IECInstancePtr pet1 = CreatePerson(*personClass, "First", "pet");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet1));
	//pet2
	IECInstancePtr pet2 = CreatePerson(*personClass, "Second", "pet");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet2));


	// Holding relationship (GrandParent2 -> Parent2 -> child2 -> pet2 )
	ECRelationshipClassP HoldingRelationShipClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ParentsHaveAnyChildren")->GetRelationshipClassP();

	//GrandParent1->Parent1
	IECRelationshipInstancePtr RelationInstanceGP1_P1 = CreateRelationship(*HoldingRelationShipClass, *grandParent1, *Parent1);
	InsertionStatus = InsertInstance(ecDb, *HoldingRelationShipClass, *RelationInstanceGP1_P1);
	ASSERT_TRUE(InsertionStatus);

	//Parent1->child1
	IECRelationshipInstancePtr RelationInstanceC1_P1 = CreateRelationship(*HoldingRelationShipClass, *Parent1, *child1);
	InsertionStatus = InsertInstance(ecDb, *HoldingRelationShipClass, *RelationInstanceC1_P1);
	ASSERT_TRUE(InsertionStatus);

	//child1->pet1
	IECRelationshipInstancePtr RelationInstancePET1_C1 = CreateRelationship(*HoldingRelationShipClass, *child1, *pet1);
	InsertionStatus = InsertInstance(ecDb, *HoldingRelationShipClass, *RelationInstancePET1_C1);
	ASSERT_TRUE(InsertionStatus);

	//GrandParent1->pet1
	IECRelationshipInstancePtr RelationInstanceGP1_PET1 = CreateRelationship(*HoldingRelationShipClass, *grandParent1, *pet1);
	InsertionStatus = InsertInstance(ecDb, *HoldingRelationShipClass, *RelationInstanceGP1_PET1);
	ASSERT_TRUE(InsertionStatus);

	//GrandParent1->child1
	IECRelationshipInstancePtr RelationInstanceGP2_C1 = CreateRelationship(*HoldingRelationShipClass, *grandParent2, *child1);
	InsertionStatus = InsertInstance(ecDb, *HoldingRelationShipClass, *RelationInstanceGP2_C1);
	ASSERT_TRUE(InsertionStatus);

	ecDb.SaveChanges();
	/*
	Test 1: Delete Pet1
	Validate:Pet1 should delete
	*/
	/*numDeleted =*/ DeleteInstance(*pet1, ecDb);
	//ASSERT_EQ(3, numDeleted);

	ASSERT_FALSE(HasInstance(*pet1, ecDb));

	ASSERT_TRUE(HasInstance(*child1, ecDb));
	ASSERT_TRUE(HasInstance(*grandParent1, ecDb));

}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Rafay.Muneeb	                     03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstancesRelationshipStrength, ForwardEmbeddingByMultiplePathDeleteTest)
    {
    //int numDeleted = 0;
    bool InsertionStatus;
    ECDbTestProject testProject;
    ECDbR ecDb = testProject.Create("RelationshipStrengthDifferentLevelHierarchyTest.ecdb", L"RelationshipStrengthTest.01.00.ecschema.xml", false);

    //grandParent1
    ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("Person");
    IECInstancePtr grandParent1 = CreatePerson(*personClass, "First", "GrandParent");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent1));
    //grandParent2
    IECInstancePtr grandParent2 = CreatePerson(*personClass, "Second", "GrandParent");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent2));
    //Parent1
    IECInstancePtr Parent1 = CreatePerson(*personClass, "First", "Parent");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent1));
    //Parent2
    IECInstancePtr Parent2 = CreatePerson(*personClass, "Second", "Parent");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent2));
    //child1    
    IECInstancePtr child1 = CreatePerson(*personClass, "First", "Child");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child1));
    //child2
    IECInstancePtr child2 = CreatePerson(*personClass, "Second", "Child");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child2));
    //pet1
    IECInstancePtr pet1 = CreatePerson(*personClass, "First", "pet");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet1));
    //pet2
    IECInstancePtr pet2 = CreatePerson(*personClass, "Second", "pet");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet2));


    ECRelationshipClassP EmbeddingRelationShipClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ParentHasAnyChild")->GetRelationshipClassP();

    IECRelationshipInstancePtr RelationInstanceGP1_P1 = CreateRelationship(*EmbeddingRelationShipClass, *grandParent1, *Parent1);
    InsertionStatus = InsertInstance(ecDb, *EmbeddingRelationShipClass, *RelationInstanceGP1_P1);
    ASSERT_TRUE(InsertionStatus);

    IECRelationshipInstancePtr RelationInstanceC1_P1 = CreateRelationship(*EmbeddingRelationShipClass, *Parent1, *child1);
    InsertionStatus = InsertInstance(ecDb, *EmbeddingRelationShipClass, *RelationInstanceC1_P1);
    ASSERT_TRUE(InsertionStatus);

    IECRelationshipInstancePtr RelationInstancePET1_C1 = CreateRelationship(*EmbeddingRelationShipClass, *child1, *pet1);
    InsertionStatus = InsertInstance(ecDb, *EmbeddingRelationShipClass, *RelationInstancePET1_C1);
    ASSERT_TRUE(InsertionStatus);

    ecDb.SaveChanges();

    /*numDeleted =*/ DeleteInstance(*grandParent1, ecDb);
    //ASSERT_EQ(7, numDeleted);

    ASSERT_TRUE(HasInstance(*grandParent2, ecDb));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Rafay.Muneeb	                     03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstancesRelationshipStrength, ForwardEmbeddingMiddleInstancePathDeleteTest)
    {
    //int numDeleted = 0;
    bool InsertionStatus;
    ECDbTestProject testProject;
    ECDbR ecDb = testProject.Create("RelationshipStrengthDifferentLevelHierarchyTest.ecdb", L"RelationshipStrengthTest.01.00.ecschema.xml", false);

    //grandParent1
    ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("Person");
    IECInstancePtr grandParent1 = CreatePerson(*personClass, "First", "GrandParent");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent1));
    //grandParent2
    IECInstancePtr grandParent2 = CreatePerson(*personClass, "Second", "GrandParent");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent2));
    //Parent1
    IECInstancePtr Parent1 = CreatePerson(*personClass, "First", "Parent");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent1));
    //Parent2
    IECInstancePtr Parent2 = CreatePerson(*personClass, "Second", "Parent");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent2));
    //child1    
    IECInstancePtr child1 = CreatePerson(*personClass, "First", "Child");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child1));
    //child2
    IECInstancePtr child2 = CreatePerson(*personClass, "Second", "Child");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child2));
    //pet1
    IECInstancePtr pet1 = CreatePerson(*personClass, "First", "pet");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet1));
    //pet2
    IECInstancePtr pet2 = CreatePerson(*personClass, "Second", "pet");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet2));


    ECRelationshipClassP EmbeddingRelationShipClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ParentHasAnyChild")->GetRelationshipClassP();

    IECRelationshipInstancePtr RelationInstanceGP1_P1 = CreateRelationship(*EmbeddingRelationShipClass, *grandParent1, *Parent1);
    InsertionStatus = InsertInstance(ecDb, *EmbeddingRelationShipClass, *RelationInstanceGP1_P1);
    ASSERT_TRUE(InsertionStatus);

    IECRelationshipInstancePtr RelationInstanceC1_P1 = CreateRelationship(*EmbeddingRelationShipClass, *Parent1, *child1);
    InsertionStatus = InsertInstance(ecDb, *EmbeddingRelationShipClass, *RelationInstanceC1_P1);
    ASSERT_TRUE(InsertionStatus);

    IECRelationshipInstancePtr RelationInstancePET1_C1 = CreateRelationship(*EmbeddingRelationShipClass, *child1, *pet1);
    InsertionStatus = InsertInstance(ecDb, *EmbeddingRelationShipClass, *RelationInstancePET1_C1);
    ASSERT_TRUE(InsertionStatus);


    ecDb.SaveChanges();

    /*numDeleted =*/ DeleteInstance(*Parent1, ecDb);
    //ASSERT_EQ(6, numDeleted);

    ASSERT_TRUE(HasInstance(*grandParent1, ecDb));
    ASSERT_FALSE(HasInstance(*Parent1, ecDb));
    ASSERT_FALSE(HasInstance(*child1, ecDb));
    ASSERT_FALSE(HasInstance(*pet1, ecDb));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Rafay.Muneeb	                     03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstancesRelationshipStrength, ForwardEmbeddingMultipleLinkedObjectPathDeleteTest)
    {
    //int numDeleted = 0;
    bool InsertionStatus;
    ECDbTestProject testProject;
    ECDbR ecDb = testProject.Create("RelationshipStrengthDifferentLevelHierarchyTest.ecdb", L"RelationshipStrengthTest.01.00.ecschema.xml", false);

    //grandParent1
    ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("Person");
    IECInstancePtr grandParent1 = CreatePerson(*personClass, "First", "GrandParent");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent1));
    //grandParent2
    IECInstancePtr grandParent2 = CreatePerson(*personClass, "Second", "GrandParent");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent2));
    //Parent1
    IECInstancePtr Parent1 = CreatePerson(*personClass, "First", "Parent");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent1));
    //Parent2
    IECInstancePtr Parent2 = CreatePerson(*personClass, "Second", "Parent");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent2));
    //child1    
    IECInstancePtr child1 = CreatePerson(*personClass, "First", "Child");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child1));
    //child2
    IECInstancePtr child2 = CreatePerson(*personClass, "Second", "Child");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child2));
    //pet1
    IECInstancePtr pet1 = CreatePerson(*personClass, "First", "pet");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet1));
    //pet2
    IECInstancePtr pet2 = CreatePerson(*personClass, "Second", "pet");
    ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet2));

    ECRelationshipClassP EmbeddingRelationShipClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ParentHasAnyChild")->GetRelationshipClassP();

    IECRelationshipInstancePtr RelationInstanceGP1_P1 = CreateRelationship(*EmbeddingRelationShipClass, *grandParent1, *Parent1);
    InsertionStatus = InsertInstance(ecDb, *EmbeddingRelationShipClass, *RelationInstanceGP1_P1);
    ASSERT_TRUE(InsertionStatus);

    IECRelationshipInstancePtr RelationInstanceC1_P1 = CreateRelationship(*EmbeddingRelationShipClass, *Parent1, *child1);
    InsertionStatus = InsertInstance(ecDb, *EmbeddingRelationShipClass, *RelationInstanceC1_P1);
    ASSERT_TRUE(InsertionStatus);

    IECRelationshipInstancePtr RelationInstancePET1_C1 = CreateRelationship(*EmbeddingRelationShipClass, *child1, *pet1);
    InsertionStatus = InsertInstance(ecDb, *EmbeddingRelationShipClass, *RelationInstancePET1_C1);
    ASSERT_TRUE(InsertionStatus);

    ecDb.SaveChanges();
    /*
    Test 1: Delete pet1
    Validate: only pet1 should delete with its relations
    */
    /*numDeleted =*/ DeleteInstance(*pet1, ecDb);
    //ASSERT_EQ(2, numDeleted);

    ASSERT_FALSE(HasInstance(*pet1, ecDb));
    }

/***********************************************************************************************
            Create the following relationship hierarchy (Below 5 Test use this relationship model)
            GrandParent1                                      GrandParent2
            ↓                                                   ↓
            ↓                                                   ↓
            AnyChildHasParent (EMBEDDING)					ParentsHaveAnyChildren (HOLDING)
            ↓													↓
            Parent1  --►------►--------►(EMBEDDING)►-----►----► Parent2
            ↓                                                   ↓
            AnyChildHasParent (EMBEDDING)	 				ParentsHaveAnyChildren (HOLDING)
            ↓                                                   ↓
            ↓                                                   ↓
            Child1	◄-------◄------◄(HOLDING)◄----◄----◄--  Child2
            ↓                                                   ↓
            AnyChildHasParent (EMBEDDING)					ParentsHaveAnyChildren (HOLDING)
            ↓                                                   ↓
            Pet1                                               Pet2
*/
/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                   Rafay.Muneeb	                     03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstancesRelationshipStrength, DirectEmbeddedChildDeleteTest)
{
	//int numDeleted = 0;
	bool InsertionStatus;
	ECDbTestProject testProject;
	ECDbR ecDb = testProject.Create("RelationshipStrengthDifferentLevelHierarchyTest.ecdb", L"RelationshipStrengthTest.01.00.ecschema.xml", false);

	//grandParent1
	ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("Person");
	IECInstancePtr grandParent1 = CreatePerson(*personClass, "First", "GrandParent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent1));
	//grandParent2
	IECInstancePtr grandParent2 = CreatePerson(*personClass, "Second", "GrandParent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent2));
	//Parent1
	IECInstancePtr Parent1 = CreatePerson(*personClass, "First", "Parent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent1));
	//Parent2
	IECInstancePtr Parent2 = CreatePerson(*personClass, "Second", "Parent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent2));
	//child1    
	IECInstancePtr child1 = CreatePerson(*personClass, "First", "Child");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child1));
	//child2
	IECInstancePtr child2 = CreatePerson(*personClass, "Second", "Child");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child2));
	//pet1
	IECInstancePtr pet1 = CreatePerson(*personClass, "First", "pet");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet1));
	//pet2
	IECInstancePtr pet2 = CreatePerson(*personClass, "Second", "pet");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet2));


	// Holding relationship (GrandParent2 -> Parent2 -> child2 -> pet2 )
	ECRelationshipClassP parentsHaveAnyChildren = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ParentsHaveAnyChildren")->GetRelationshipClassP();
	IECRelationshipInstancePtr holdingRelationInstance;

	IECRelationshipInstancePtr holdingRelationInstanceG2_P2 = CreateRelationship(*parentsHaveAnyChildren, *grandParent2, *Parent2);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceG2_P2);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr holdingRelationInstanceP2_C2 = CreateRelationship(*parentsHaveAnyChildren, *Parent2, *child2);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceP2_C2);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr holdingRelationInstanceC2_Pt2 = CreateRelationship(*parentsHaveAnyChildren, *child2, *pet2);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceC2_Pt2);
	ASSERT_TRUE(InsertionStatus);

	// Embedding relationship (GrandParent1 -> Parent1 -> child1 -> pet1 )
	ECRelationshipClassP parentHasAnyChild = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ParentHasAnyChild")->GetRelationshipClassP();
	IECRelationshipInstancePtr embeddedRelationInstance;

	IECRelationshipInstancePtr embeddedRelationInstanceG1_P1 = CreateRelationship(*parentHasAnyChild, *grandParent1, *Parent1);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceG1_P1);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr embeddedRelationInstanceP1_C1 = CreateRelationship(*parentHasAnyChild, *Parent1, *child1);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceP1_C1);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr embeddedRelationInstanceC1_Pt1 = CreateRelationship(*parentHasAnyChild, *child1, *pet1);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceC1_Pt1);
	ASSERT_TRUE(InsertionStatus);

	//Parent1 embed Parent 2
	IECRelationshipInstancePtr embeddedRelationInstanceP1_P2 = CreateRelationship(*parentHasAnyChild, *Parent1, *Parent2);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceP1_P2);
	ASSERT_TRUE(InsertionStatus);

	//Child2 holds Child1
	IECRelationshipInstancePtr holdingRelationInstanceC2_C1 = CreateRelationship(*parentsHaveAnyChildren, *child2, *child1);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceC2_C1);
	ASSERT_TRUE(InsertionStatus);

	ecDb.SaveChanges();
	/*
	Test 1: Delete GrandParent2
	Validate: Relation,Pet1 gets deleted
	*/
	/*numDeleted =*/ DeleteInstance(*grandParent2, ecDb);
	//ASSERT_EQ(13, numDeleted);

	ASSERT_TRUE(HasInstance(*grandParent1, ecDb));
	ASSERT_TRUE(HasInstance(*Parent1, ecDb));

	ASSERT_FALSE(HasInstance(*holdingRelationInstanceG2_P2, ecDb));
	ASSERT_FALSE(HasInstance(*holdingRelationInstanceP2_C2, ecDb));
	ASSERT_FALSE(HasInstance(*holdingRelationInstanceC2_Pt2, ecDb));

	ASSERT_FALSE(HasInstance(*embeddedRelationInstanceP1_P2, ecDb));
	ASSERT_FALSE(HasInstance(*holdingRelationInstanceC2_C1, ecDb));

	ASSERT_FALSE(HasInstance(*child1, ecDb));
	ASSERT_FALSE(HasInstance(*pet1, ecDb));

	ASSERT_FALSE(HasInstance(*Parent2, ecDb));
	ASSERT_FALSE(HasInstance(*child2, ecDb));
	ASSERT_FALSE(HasInstance(*pet2, ecDb));

}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Rafay.Muneeb	                     03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstancesRelationshipStrength, HoldingDeleteRuleTest)
{
	//int numDeleted = 0;
	bool InsertionStatus;
	ECDbTestProject testProject;
	ECDbR ecDb = testProject.Create("RelationshipStrengthDifferentLevelHierarchyTest.ecdb", L"RelationshipStrengthTest.01.00.ecschema.xml", false);

	//grandParent1
	ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("Person");
	IECInstancePtr grandParent1 = CreatePerson(*personClass, "First", "GrandParent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent1));
	//grandParent2
	IECInstancePtr grandParent2 = CreatePerson(*personClass, "Second", "GrandParent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent2));
	//Parent1
	IECInstancePtr Parent1 = CreatePerson(*personClass, "First", "Parent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent1));
	//Parent2
	IECInstancePtr Parent2 = CreatePerson(*personClass, "Second", "Parent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent2));
	//child1    
	IECInstancePtr child1 = CreatePerson(*personClass, "First", "Child");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child1));
	//child2
	IECInstancePtr child2 = CreatePerson(*personClass, "Second", "Child");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child2));
	//pet1
	IECInstancePtr pet1 = CreatePerson(*personClass, "First", "pet");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet1));
	//pet2
	IECInstancePtr pet2 = CreatePerson(*personClass, "Second", "pet");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet2));


	// Holding relationship (GrandParent2 -> Parent2 -> child2 -> pet2 )
	ECRelationshipClassP parentsHaveAnyChildren = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ParentsHaveAnyChildren")->GetRelationshipClassP();
	IECRelationshipInstancePtr holdingRelationInstance;

	IECRelationshipInstancePtr holdingRelationInstanceG2_P2 = CreateRelationship(*parentsHaveAnyChildren, *grandParent2, *Parent2);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceG2_P2);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr holdingRelationInstanceP2_C2 = CreateRelationship(*parentsHaveAnyChildren, *Parent2, *child2);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceP2_C2);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr holdingRelationInstanceC2_Pt2 = CreateRelationship(*parentsHaveAnyChildren, *child2, *pet2);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceC2_Pt2);
	ASSERT_TRUE(InsertionStatus);

	// Embedding relationship (GrandParent1 -> Parent1 -> child1 -> pet1 )
	ECRelationshipClassP parentHasAnyChild = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ParentHasAnyChild")->GetRelationshipClassP();
	IECRelationshipInstancePtr embeddedRelationInstance;

	IECRelationshipInstancePtr embeddedRelationInstanceG1_P1 = CreateRelationship(*parentHasAnyChild, *grandParent1, *Parent1);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceG1_P1);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr embeddedRelationInstanceP1_C1 = CreateRelationship(*parentHasAnyChild, *Parent1, *child1);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceP1_C1);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr embeddedRelationInstanceC1_Pt1 = CreateRelationship(*parentHasAnyChild, *child1, *pet1);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceC1_Pt1);
	ASSERT_TRUE(InsertionStatus);

	//Parent1 embed Parent 2
	IECRelationshipInstancePtr embeddedRelationInstanceP1_P2 = CreateRelationship(*parentHasAnyChild, *Parent1, *Parent2);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceP1_P2);
	ASSERT_TRUE(InsertionStatus);

	//Child2 holds Child1
	IECRelationshipInstancePtr holdingRelationInstanceC2_C1 = CreateRelationship(*parentsHaveAnyChildren, *child2, *child1);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceC2_C1);
	ASSERT_TRUE(InsertionStatus);

	ecDb.SaveChanges();
	/*
	Test 1: Delete Parent2
	Validate: (orphaned relationships) child1->pet1
	Validate: Relation,Pet1 gets deleted
	*/
	//this test is valid, as hold relation will follow its delete rule despite child1 still have embedding relation with parent 1
	/*numDeleted =*/ DeleteInstance(*Parent2, ecDb);
	//ASSERT_EQ(12, numDeleted);

	ASSERT_TRUE(HasInstance(*grandParent2, ecDb));
	ASSERT_TRUE(HasInstance(*Parent1, ecDb));


	ASSERT_FALSE(HasInstance(*holdingRelationInstanceG2_P2, ecDb));
	ASSERT_FALSE(HasInstance(*holdingRelationInstanceP2_C2, ecDb));
	ASSERT_FALSE(HasInstance(*holdingRelationInstanceC2_Pt2, ecDb));

	ASSERT_FALSE(HasInstance(*embeddedRelationInstanceP1_P2, ecDb));
	ASSERT_FALSE(HasInstance(*holdingRelationInstanceC2_C1, ecDb));

	ASSERT_FALSE(HasInstance(*child1, ecDb));
	ASSERT_FALSE(HasInstance(*Parent2, ecDb));
	ASSERT_FALSE(HasInstance(*child2, ecDb));
	ASSERT_FALSE(HasInstance(*pet2, ecDb));





}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Rafay.Muneeb	                     03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstancesRelationshipStrength, ForwardEmbedDeleteThirdLevelParentWithTwoLinks)
{
	//int numDeleted = 0;
	bool InsertionStatus;
	ECDbTestProject testProject;
	ECDbR ecDb = testProject.Create("RelationshipStrengthDifferentLevelHierarchyTest.ecdb", L"RelationshipStrengthTest.01.00.ecschema.xml", false);

	//grandParent1
	ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("Person");
	IECInstancePtr grandParent1 = CreatePerson(*personClass, "First", "GrandParent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent1));
	//grandParent2
	IECInstancePtr grandParent2 = CreatePerson(*personClass, "Second", "GrandParent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent2));
	//Parent1
	IECInstancePtr Parent1 = CreatePerson(*personClass, "First", "Parent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent1));
	//Parent2
	IECInstancePtr Parent2 = CreatePerson(*personClass, "Second", "Parent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent2));
	//child1    
	IECInstancePtr child1 = CreatePerson(*personClass, "First", "Child");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child1));
	//child2
	IECInstancePtr child2 = CreatePerson(*personClass, "Second", "Child");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child2));
	//pet1
	IECInstancePtr pet1 = CreatePerson(*personClass, "First", "pet");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet1));
	//pet2
	IECInstancePtr pet2 = CreatePerson(*personClass, "Second", "pet");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet2));


	// Holding relationship (GrandParent2 -> Parent2 -> child2 -> pet2 )
	ECRelationshipClassP parentsHaveAnyChildren = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ParentsHaveAnyChildren")->GetRelationshipClassP();
	IECRelationshipInstancePtr holdingRelationInstance;

	IECRelationshipInstancePtr holdingRelationInstanceG2_P2 = CreateRelationship(*parentsHaveAnyChildren, *grandParent2, *Parent2);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceG2_P2);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr holdingRelationInstanceP2_C2 = CreateRelationship(*parentsHaveAnyChildren, *Parent2, *child2);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceP2_C2);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr holdingRelationInstanceC2_Pt2 = CreateRelationship(*parentsHaveAnyChildren, *child2, *pet2);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceC2_Pt2);
	ASSERT_TRUE(InsertionStatus);

	// Embedding relationship (GrandParent1 -> Parent1 -> child1 -> pet1 )
	ECRelationshipClassP parentHasAnyChild = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ParentHasAnyChild")->GetRelationshipClassP();
	IECRelationshipInstancePtr embeddedRelationInstance;

	IECRelationshipInstancePtr embeddedRelationInstanceG1_P1 = CreateRelationship(*parentHasAnyChild, *grandParent1, *Parent1);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceG1_P1);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr embeddedRelationInstanceP1_C1 = CreateRelationship(*parentHasAnyChild, *Parent1, *child1);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceP1_C1);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr embeddedRelationInstanceC1_Pt1 = CreateRelationship(*parentHasAnyChild, *child1, *pet1);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceC1_Pt1);
	ASSERT_TRUE(InsertionStatus);

	//Parent1 embed Parent 2
	IECRelationshipInstancePtr embeddedRelationInstanceP1_P2 = CreateRelationship(*parentHasAnyChild, *Parent1, *Parent2);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceP1_P2);
	ASSERT_TRUE(InsertionStatus);

	//Child2 holds Child1
	IECRelationshipInstancePtr holdingRelationInstanceC2_C1 = CreateRelationship(*parentsHaveAnyChildren, *child2, *child1);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceC2_C1);
	ASSERT_TRUE(InsertionStatus);

	ecDb.SaveChanges();
	/*
	Test 1: Delete Pet1
	Validate: (orphaned relationships) child1->pet1
	Validate: Relation,Pet1 gets deleted
	*/
	/*numDeleted = */DeleteInstance(*pet1, ecDb);
	//ASSERT_EQ(2, numDeleted);
	ASSERT_FALSE(HasInstance(*embeddedRelationInstanceC1_Pt1, ecDb));
	ASSERT_FALSE(HasInstance(*pet1, ecDb));

	ASSERT_TRUE(HasInstance(*child1, ecDb));
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Rafay.Muneeb	                     03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstancesRelationshipStrength, DeleteInstanceOfMultipleRelationsPath)
{
	/*int numDeleted = 0;*/
	bool InsertionStatus;
	ECDbTestProject testProject;
	ECDbR ecDb = testProject.Create("RelationshipStrengthDifferentLevelHierarchyTest.ecdb", L"RelationshipStrengthTest.01.00.ecschema.xml", false);

	//grandParent1
	ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("Person");
	IECInstancePtr grandParent1 = CreatePerson(*personClass, "First", "GrandParent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent1));
	//grandParent2
	IECInstancePtr grandParent2 = CreatePerson(*personClass, "Second", "GrandParent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent2));
	//Parent1
	IECInstancePtr Parent1 = CreatePerson(*personClass, "First", "Parent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent1));
	//Parent2
	IECInstancePtr Parent2 = CreatePerson(*personClass, "Second", "Parent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent2));
	//child1    
	IECInstancePtr child1 = CreatePerson(*personClass, "First", "Child");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child1));
	//child2
	IECInstancePtr child2 = CreatePerson(*personClass, "Second", "Child");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child2));
	//pet1
	IECInstancePtr pet1 = CreatePerson(*personClass, "First", "pet");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet1));
	//pet2
	IECInstancePtr pet2 = CreatePerson(*personClass, "Second", "pet");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet2));


	// Holding relationship (GrandParent2 -> Parent2 -> child2 -> pet2 )
	ECRelationshipClassP parentsHaveAnyChildren = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ParentsHaveAnyChildren")->GetRelationshipClassP();
	IECRelationshipInstancePtr holdingRelationInstance;

	IECRelationshipInstancePtr holdingRelationInstanceG2_P2 = CreateRelationship(*parentsHaveAnyChildren, *grandParent2, *Parent2);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceG2_P2);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr holdingRelationInstanceP2_C2 = CreateRelationship(*parentsHaveAnyChildren, *Parent2, *child2);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceP2_C2);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr holdingRelationInstanceC2_Pt2 = CreateRelationship(*parentsHaveAnyChildren, *child2, *pet2);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceC2_Pt2);
	ASSERT_TRUE(InsertionStatus);

	// Embedding relationship (GrandParent1 -> Parent1 -> child1 -> pet1 )
	ECRelationshipClassP parentHasAnyChild = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ParentHasAnyChild")->GetRelationshipClassP();
	IECRelationshipInstancePtr embeddedRelationInstance;

	IECRelationshipInstancePtr embeddedRelationInstanceG1_P1 = CreateRelationship(*parentHasAnyChild, *grandParent1, *Parent1);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceG1_P1);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr embeddedRelationInstanceP1_C1 = CreateRelationship(*parentHasAnyChild, *Parent1, *child1);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceP1_C1);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr embeddedRelationInstanceC1_Pt1 = CreateRelationship(*parentHasAnyChild, *child1, *pet1);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceC1_Pt1);
	ASSERT_TRUE(InsertionStatus);

	//Parent1 embed Parent 2
	IECRelationshipInstancePtr embeddedRelationInstanceP1_P2 = CreateRelationship(*parentHasAnyChild, *Parent1, *Parent2);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceP1_P2);
	ASSERT_TRUE(InsertionStatus);

	//Child2 holds Child1
	IECRelationshipInstancePtr holdingRelationInstanceC2_C1 = CreateRelationship(*parentsHaveAnyChildren, *child2, *child1);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceC2_C1);
	ASSERT_TRUE(InsertionStatus);

	ecDb.SaveChanges();
	/*
	Test 1: Delete Child1
	Validate: (orphaned relationships) child1->pet1
	Validate: Child1,Pet1 gets deleted
	*/
	/*numDeleted =*/ DeleteInstance(*child1, ecDb);
	//ASSERT_EQ(5, numDeleted);

	ASSERT_FALSE(HasInstance(*holdingRelationInstanceC2_C1, ecDb));
	ASSERT_FALSE(HasInstance(*embeddedRelationInstanceP1_C1, ecDb));
	ASSERT_FALSE(HasInstance(*embeddedRelationInstanceC1_Pt1, ecDb));
	ASSERT_FALSE(HasInstance(*child1, ecDb));
	ASSERT_FALSE(HasInstance(*pet1, ecDb));

	ASSERT_TRUE(HasInstance(*Parent1, ecDb));
	ASSERT_TRUE(HasInstance(*Parent2, ecDb));
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Rafay.Muneeb	                     03/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstancesRelationshipStrength, ForwardEmbeddingWithMultipleLevelChildrens)
{
	//int numDeleted = 0;
	bool InsertionStatus;
	ECDbTestProject testProject;
	ECDbR ecDb = testProject.Create("RelationshipStrengthDifferentLevelHierarchyTest.ecdb", L"RelationshipStrengthTest.01.00.ecschema.xml", false);

	//grandParent1
	ECClassCP personClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("Person");
	IECInstancePtr grandParent1 = CreatePerson(*personClass, "First", "GrandParent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent1));
	//grandParent2
	IECInstancePtr grandParent2 = CreatePerson(*personClass, "Second", "GrandParent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *grandParent2));
	//Parent1
	IECInstancePtr Parent1 = CreatePerson(*personClass, "First", "Parent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent1));
	//Parent2
	IECInstancePtr Parent2 = CreatePerson(*personClass, "Second", "Parent");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *Parent2));
	//child1    
	IECInstancePtr child1 = CreatePerson(*personClass, "First", "Child");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child1));
	//child2
	IECInstancePtr child2 = CreatePerson(*personClass, "Second", "Child");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *child2));
	//pet1
	IECInstancePtr pet1 = CreatePerson(*personClass, "First", "pet");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet1));
	//pet2
	IECInstancePtr pet2 = CreatePerson(*personClass, "Second", "pet");
	ASSERT_TRUE(InsertInstance(ecDb, *personClass, *pet2));


	// Holding relationship (GrandParent2 -> Parent2 -> child2 -> pet2 )
	ECRelationshipClassP parentsHaveAnyChildren = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ParentsHaveAnyChildren")->GetRelationshipClassP();
	IECRelationshipInstancePtr holdingRelationInstance;

	IECRelationshipInstancePtr holdingRelationInstanceG2_P2 = CreateRelationship(*parentsHaveAnyChildren, *grandParent2, *Parent2);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceG2_P2);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr holdingRelationInstanceP2_C2 = CreateRelationship(*parentsHaveAnyChildren, *Parent2, *child2);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceP2_C2);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr holdingRelationInstanceC2_Pt2 = CreateRelationship(*parentsHaveAnyChildren, *child2, *pet2);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceC2_Pt2);
	ASSERT_TRUE(InsertionStatus);

	// Embedding relationship (GrandParent1 -> Parent1 -> child1 -> pet1 )
	ECRelationshipClassP parentHasAnyChild = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP("ParentHasAnyChild")->GetRelationshipClassP();
	IECRelationshipInstancePtr embeddedRelationInstance;

	IECRelationshipInstancePtr embeddedRelationInstanceG1_P1 = CreateRelationship(*parentHasAnyChild, *grandParent1, *Parent1);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceG1_P1);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr embeddedRelationInstanceP1_C1 = CreateRelationship(*parentHasAnyChild, *Parent1, *child1);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceP1_C1);
	ASSERT_TRUE(InsertionStatus);

	IECRelationshipInstancePtr embeddedRelationInstanceC1_Pt1 = CreateRelationship(*parentHasAnyChild, *child1, *pet1);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceC1_Pt1);
	ASSERT_TRUE(InsertionStatus);

	//Parent1 embed Parent 2
	IECRelationshipInstancePtr embeddedRelationInstanceP1_P2 = CreateRelationship(*parentHasAnyChild, *Parent1, *Parent2);
	InsertionStatus = InsertInstance(ecDb, *parentHasAnyChild, *embeddedRelationInstanceP1_P2);
	ASSERT_TRUE(InsertionStatus);

	//Child2 holds Child1
	IECRelationshipInstancePtr holdingRelationInstanceC2_C1 = CreateRelationship(*parentsHaveAnyChildren, *child2, *child1);
	InsertionStatus = InsertInstance(ecDb, *parentsHaveAnyChildren, *holdingRelationInstanceC2_C1);
	ASSERT_TRUE(InsertionStatus);

	ecDb.SaveChanges();
	/*
	Test 1: Delete Parent1
	Validate: (orphaned relationships) Parent1->child1->pet1
	Validate: Parent1,Child1,Pet1 gets deleted
	*/
	/*numDeleted =*/ DeleteInstance(*Parent1, ecDb);
	//ASSERT_EQ(14, numDeleted);
	ASSERT_FALSE(HasInstance(*Parent1, ecDb));
	ASSERT_FALSE(HasInstance(*child1, ecDb));
	ASSERT_FALSE(HasInstance(*pet1, ecDb));

	ASSERT_FALSE(HasInstance(*Parent1, ecDb));
	ASSERT_FALSE(HasInstance(*child1, ecDb));
	ASSERT_FALSE(HasInstance(*pet1, ecDb));
}
END_ECDBUNITTESTS_NAMESPACE

