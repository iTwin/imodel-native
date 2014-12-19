/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Published/ECDbRelationshipStrength_Test_Old.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

extern bool InsertInstance (ECDbR db, ECClassCR ecClass, IECInstanceR ecInstance);
extern IECRelationshipInstancePtr CreateRelationship (ECRelationshipClassCR relationshipClass, IECInstanceR source, IECInstanceR target);
extern ECInstanceId InstanceToId (IECInstanceCR ecInstance);
extern IECInstancePtr CreatePerson (ECClassCR ecClass, WCharCP firstName, WCharCP lastName);
extern bool HasInstance (IECInstanceCR instance, ECDbR ecDb);


//+===============+===============+===============+===============+===============+======
// @bsiclass                                                Ramanujam.Raman      09/2013
//+===============+===============+===============+===============+===============+======
struct TestECDbDeleteHandler : ECDbDeleteHandler
{
    int m_numDeleteCalls;

    void _OnBeforeDelete (ECClassCR ecClass, ECInstanceId ecInstanceId, ECDbR ecDb)
        {
        m_numDeleteCalls++;
        }

    TestECDbDeleteHandler() {m_numDeleteCalls = 0;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
int DeleteInstance_Old (IECInstanceCR instance, ECDbR ecDb)
    {
    ECClassCR ecClass = instance.GetClass();
    ECPersistencePtr persistence = ecDb.GetEC().GetECPersistence (nullptr, ecClass);
    if (persistence.IsNull())
        return -1;

    TestECDbDeleteHandler deleteHandler;
    DeleteStatus status = persistence->Delete (InstanceToId (instance), &deleteHandler);
    if (status != DELETE_Success)
        return -1;
    return deleteHandler.m_numDeleteCalls;
    }
    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances_Old, RelationshipStrength)
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
    ASSERT_TRUE (HasInstance (*grandParent1HasSpouse, ecDb));
    ASSERT_TRUE (HasInstance (*grandParent2HasSpouse, ecDb));

    int numDeleted = DeleteInstance_Old (*grandParent1, ecDb);
    ASSERT_EQ (4, numDeleted);

    ASSERT_FALSE (HasInstance (*grandParent1, ecDb));
    ASSERT_FALSE (HasInstance (*grandParent1HasSpouse, ecDb));
    ASSERT_FALSE (HasInstance (*grandParent2HasSpouse, ecDb));
    ASSERT_FALSE (HasInstance (*grandParent1HasSingleParent, ecDb));
    ASSERT_TRUE (HasInstance (*singleParent, ecDb));
    
    /*
     * Test 2: Delete GrandParent2
     * Validate grandParent2HasSingleParent has been deleted (orphaned relationship)
     * Validate singeParent has been deleted (held instance with no parents remaining)
     */
    numDeleted = DeleteInstance_Old (*grandParent2, ecDb);
    ASSERT_EQ (7, numDeleted);

    ASSERT_FALSE (HasInstance (*grandParent2, ecDb));
    ASSERT_FALSE (HasInstance (*grandParent2HasSingleParent, ecDb));
    ASSERT_FALSE (HasInstance (*singleParent, ecDb));
    ASSERT_FALSE (HasInstance (*singleParentHasChild1, ecDb));
    ASSERT_FALSE (HasInstance (*singleParentHasChild2, ecDb));
    ASSERT_FALSE (HasInstance (*child1, ecDb));
    ASSERT_FALSE (HasInstance (*child2, ecDb));
    }

    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbInstances_Old, RelationshipStrengthAnyClass)
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
    int numDeleted = DeleteInstance_Old (*grandParent1, ecDb);
    ASSERT_EQ (2, numDeleted);

    ASSERT_FALSE (HasInstance (*grandParent1, ecDb));
    ASSERT_FALSE (HasInstance (*grandParent1HasSingleParent, ecDb));
    ASSERT_TRUE (HasInstance (*singleParent, ecDb));
    
    /*
     * Test 2: Delete GrandParent2
     * Validate grandParent2HasSingleParent has been deleted (orphaned relationship)
     * Validate singeParent has been deleted (held instance with no parents remaining)
     */
    numDeleted = DeleteInstance_Old (*grandParent2, ecDb);
    ASSERT_EQ (7, numDeleted);

    ASSERT_FALSE (HasInstance (*grandParent2, ecDb));
    ASSERT_FALSE (HasInstance (*grandParent2HasSingleParent, ecDb));
    ASSERT_FALSE (HasInstance (*singleParent, ecDb));
    ASSERT_FALSE (HasInstance (*singleParentHasChild1, ecDb));
    ASSERT_FALSE (HasInstance (*singleParentHasChild2, ecDb));
    ASSERT_FALSE (HasInstance (*child1, ecDb));
    ASSERT_FALSE (HasInstance (*child2, ecDb));
    }

END_ECDBUNITTESTS_NAMESPACE

