/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Published/ECDbRelationshipTests_Old.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

extern IECRelationshipInstancePtr CreateRelationship (ECN::ECRelationshipClassCR relationshipClass, IECInstanceR source, IECInstanceR target);
extern IECRelationshipInstancePtr CreateRelationship
(
ECDbTestProject& test,
WCharCP sourceClassName,
WCharCP targetClassName,
WCharCP relationshipClassName
);
extern ECInstanceId InstanceToId (IECInstanceCR ecInstance);
extern ECN::ECClassId ClassToId (ECClassCR ecClass);
extern void ValidatePersistingRelationship
(
BeSQLiteDbR db,
Utf8CP tableName,
ECInstanceId whereECInstanceId,
Utf8CP expectedIdColumnName,
Int64 expectedId
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PersistRelationship_Old (IECRelationshipInstanceR relInstance, ECDbStoreCR ecDbStore)
    {
    ECPersistencePtr persistence = ecDbStore.GetECPersistence (nullptr, relInstance.GetClass());
    ASSERT_TRUE (persistence.IsValid());
    InsertStatus insertStatus = persistence->Insert (nullptr, relInstance);
    ASSERT_EQ (INSERT_Success, insertStatus);    
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Adeel.Shoukat                   07/13
 +---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus PersistRelationshipWithStatus_Old (IECRelationshipInstanceR relInstance, ECDbStoreCR ecDbStore)
{
	ECPersistencePtr persistence = ecDbStore.GetECPersistence (nullptr, relInstance.GetClass());
	EXPECT_TRUE (persistence.IsValid());
	InsertStatus insertStatus = persistence->Insert (nullptr, relInstance);
	
	return insertStatus;
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPersistencePtr GetECPersistence (const ECDbTestProject& testProject, WCharCP className, ECDbStoreCR ecDbStore)
    {
    ECClassCP ecClass = testProject.GetTestSchemaManager().GetTestSchema()->GetClassP (className);
    if (ecClass == nullptr)
        return nullptr;
    ECPersistencePtr persistence = ecDbStore.GetECPersistence (nullptr, *ecClass);
    return persistence;
    }

extern bool GetECInstanceIdFromECInstance (ECInstanceId& ecInstanceId, IECInstanceCR instance);
extern void ValidateReadingRelationship (ECDbTestProject& testProject, WCharCP relationshipName, IECRelationshipInstanceCR importedRelInstance);
extern void ValidateReadingRelated (ECDbTestProject& testProject, WCharCP relationshipName, IECInstancePtr relSourceInstance, IECInstancePtr relTargetInstance);

//-------------------------------------------------------------------------------------
// This test is to document and back the current contract about what the ECInstanceId of an inserted ECRelationship is.
// @bsimethod                                   Krischan.Eberle                   06/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbRelationships_Old, RelationshipECInstanceIdContract)
    {
    // Create and populate a sample project
    ECDbTestProject test;
    auto& ecdb = test.Create ("ecrelationshipecinstanceidcontract.ecdb", L"StartupCompany.02.00.ecschema.xml", true);

        {
        //Inserting relationship with non-link-table mapping
        auto relationshipInstance = CreateRelationship (test, L"Employee", L"Chair", L"EmployeeFurniture");
        ASSERT_TRUE(relationshipInstance.IsValid());
        PersistRelationship_Old (*relationshipInstance, ecdb.GetEC ());

        ECInstanceId relECInstanceId;
        ASSERT_TRUE (GetECInstanceIdFromECInstance (relECInstanceId, *relationshipInstance));
        ECInstanceId sourceECInstanceId;
        ASSERT_TRUE (GetECInstanceIdFromECInstance (sourceECInstanceId, *relationshipInstance->GetSource ()));
        ECInstanceId targetECInstanceId;
        ASSERT_TRUE (GetECInstanceIdFromECInstance (targetECInstanceId, *relationshipInstance->GetTarget ()));

        EXPECT_EQ (targetECInstanceId.GetValue (), relECInstanceId.GetValue ()) << "For non-link table mappings, the ECInstanceId of the relationship is equal to the ECInstanceId of the child instance, i.e. the instance on the end of the relationship into whose table the relationship is persisted";
        EXPECT_NE (sourceECInstanceId.GetValue (), relECInstanceId.GetValue ()) << "For non-link table mappings, the ECInstanceId of the relationship must not be equal to the ECInstanceId of the parent instance.";
        LOG.infov (L"ECRelationship ECInstanceId Contract for non-link-table mappings: Relationship ECInstanceId: %lld - Source ECInstanceId: %lld - Target ECInstanceId: %lld (Target is where relationship is persisted to)",
            relECInstanceId.GetValue (), sourceECInstanceId.GetValue (), targetECInstanceId.GetValue ());
        }

        {
        //Inserting relationship with link-table mapping
        auto relationshipInstance = CreateRelationship (test, L"Employee", L"Hardware", L"EmployeeHardware");
        ASSERT_TRUE(relationshipInstance.IsValid());
        PersistRelationship_Old (*relationshipInstance, ecdb.GetEC ());

        ECInstanceId relECInstanceId;
        ASSERT_TRUE (GetECInstanceIdFromECInstance (relECInstanceId, *relationshipInstance));
        ECInstanceId sourceECInstanceId;
        ASSERT_TRUE (GetECInstanceIdFromECInstance (sourceECInstanceId, *relationshipInstance->GetSource ()));
        ECInstanceId targetECInstanceId;
        ASSERT_TRUE (GetECInstanceIdFromECInstance (targetECInstanceId, *relationshipInstance->GetTarget ()));

        EXPECT_NE (sourceECInstanceId.GetValue (), relECInstanceId.GetValue ()) << "For link table mappings, the ECInstanceId of the relationship is different from the ECInstanceIds of source and target instance.";
        EXPECT_NE (targetECInstanceId.GetValue (), relECInstanceId.GetValue ()) << "For link table mappings, the ECInstanceId of the relationship is different from the ECInstanceIds of source and target instance.";
        LOG.infov (L"ECRelationship ECInstanceId Contract for link-table mappings: Relationship ECInstanceId: %lld - Source ECInstanceId: %lld - Target ECInstanceId: %lld",
            relECInstanceId.GetValue (), sourceECInstanceId.GetValue (), targetECInstanceId.GetValue ());
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbRelationships_Old, MappingRelationshipsWithAdditionalECProperties)
    {
    // Create and populate a sample project
    ECDbTestProject test;
    auto& ecdb = test.Create ("ecrelationshipmapping.ecdb", L"StartupCompany.02.00.ecschema.xml", true);

    const DateTime created (2010, 8, 1);

   ASSERT_TRUE (ecdb.ColumnExists ("sc_RelationWithLinkTableMapping", "Created")) << "Link table is expected to have columns for any additional property on the ECRelationshipClass";

        {
        BeTest::SetFailOnAssert (false);
        //Inserting relationship with link-table mapping
        auto relationshipInstance = CreateRelationship (test, L"Employee", L"Hardware", L"RelationWithLinkTableMapping");
        ASSERT_TRUE (relationshipInstance != nullptr) << "Creating the relationship instance in memory is not expected to fail.";
        ASSERT_EQ (ECOBJECTS_STATUS_Success, relationshipInstance->SetValue (L"Created", ECValue (created)));
        auto persistence = GetECPersistence (test, L"RelationWithLinkTableMapping", ecdb.GetEC ());
        ASSERT_EQ (INSERT_Success, 
            persistence->Insert (nullptr, *relationshipInstance)) << "Inserting relationship with additional properties is expected to be supported for link table mapping";
        BeTest::SetFailOnAssert (true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ECDbRelationships_Old, ImportECRelationshipInstances)
    {
    // Create and populate a sample project
    ECDbTestProject test;
    ECDbR db = test.Create ("importecrelationshipinstances.ecdb", L"StartupCompany.02.00.ecschema.xml", true);
    ECDbStoreCR ecDbStore = db.GetEC();

    IECRelationshipInstancePtr relInstance;

    // Import a 1-to-1 relationship
    // EmployeePhone
    //   Inherits: AssetRelationshipsBase, EmployeeRelationshipsBase
    //   Relates:  Employee (1) -> Phone (1)
    //   Should write to RK_EmployeePhone column in sc_Employee table
    relInstance = CreateRelationship (test, L"Employee", L"Phone", L"EmployeePhone");
    ASSERT_TRUE (relInstance.IsValid());
    PersistRelationship_Old (*relInstance, ecDbStore);
    ValidatePersistingRelationship (db, "sc_Employee", InstanceToId (*relInstance->GetSource()), 
        "Phone__trg_01_id", InstanceToId (*relInstance->GetTarget()).GetValue ());
    ValidateReadingRelationship (test, L"EmployeePhone", *relInstance);
    ValidateReadingRelated (test, L"EmployeePhone", relInstance->GetSource(), relInstance->GetTarget());

    // Import another 1-to-1 relationships (with multiple classes on one end)
    // Foo_has_SomethingInOneOfManyTables
    //   Relates: Foo (1) -> Asset, Employee (1)
    //   Should write to RK_Foo_has_SomethingInOneOfManyTables, RC_Foo_has_SomethingInOneOfManyTables in FOO_FIGHTERS table (Note: Foo is mapped to FOO_FIGHTERS)
    relInstance = CreateRelationship (test, L"Foo", L"Asset", L"Foo_has_SomethingInOneOfManyTables");
    ASSERT_TRUE (relInstance.IsValid());
    PersistRelationship_Old (*relInstance, ecDbStore);
    ValidatePersistingRelationship (db, "FOO_FIGHTERS", InstanceToId (*relInstance->GetSource()), 
        "RK_Foo_has_SomethingInOneOfManyTables", InstanceToId (*relInstance->GetTarget()).GetValue ());
    ValidatePersistingRelationship (db, "FOO_FIGHTERS", InstanceToId (*relInstance->GetSource()), 
        "RC_Foo_has_SomethingInOneOfManyTables", ClassToId (relInstance->GetTarget()->GetClass()));
    ValidateReadingRelationship (test, L"Foo_has_SomethingInOneOfManyTables", *relInstance);
    ValidateReadingRelated (test, L"Foo_has_SomethingInOneOfManyTables", relInstance->GetSource(), relInstance->GetTarget());

    //now attempt to insert a second relation to same Foo object which will violate cardinality
    relInstance = CreateRelationship (test, L"Foo", L"Employee", L"Foo_has_SomethingInOneOfManyTables");
    ASSERT_TRUE (relInstance.IsValid());
    ECPersistencePtr persistence = ecDbStore.GetECPersistence (nullptr, relInstance->GetClass ());
    ASSERT_TRUE (persistence.IsValid ());
    InsertStatus insertStatus = persistence->Insert (nullptr, *relInstance);
    ASSERT_EQ (INSERT_InvalidInputInstance, insertStatus) << "Inserting second relationship for same Foo instance is expected to violate cardinality 1:1";

    // Import 1-to-M relationships
    // EmployeeFurniture
    //   Inherits: AssetRelationshipsBase, EmployeeRelationshipsBase
    //   Relates: Employee (1) -> Chair, Desk  (M)
    //   Should write to RK_EmployeeFurniture columns in sc_Furniture table (for Chair, Desk)
    relInstance = CreateRelationship (test, L"Employee", L"Chair", L"EmployeeFurniture");
    ASSERT_TRUE (relInstance.IsValid());
    PersistRelationship_Old (*relInstance, ecDbStore);
    ValidatePersistingRelationship (db, "sc_Furniture", InstanceToId (*relInstance->GetTarget()), 
        "Employee__src_01_id", InstanceToId (*relInstance->GetSource()).GetValue ());
    ValidateReadingRelationship (test, L"EmployeeFurniture", *relInstance);
    ValidateReadingRelated (test, L"EmployeeFurniture", relInstance->GetSource(), relInstance->GetTarget());

    relInstance = CreateRelationship (test, L"Employee", L"Desk", L"EmployeeFurniture");
    ASSERT_TRUE (relInstance.IsValid());
    PersistRelationship_Old (*relInstance, ecDbStore);
    ValidatePersistingRelationship (db, "sc_Furniture", InstanceToId (*relInstance->GetTarget()), 
        "Employee__src_01_id", InstanceToId (*relInstance->GetSource()).GetValue ());
    ValidateReadingRelationship (test, L"EmployeeFurniture", *relInstance);
    ValidateReadingRelated (test, L"EmployeeFurniture", relInstance->GetSource(), relInstance->GetTarget());

    // Import a M-to-1 relationship
    // EmployeeCompany
    //   Inherits: EmployeeRelationshipsBase
    //   Relates: Employee (M) -> Company (1)
    //   Should write RK_EmployeeCompany column in sc_Employee table
    relInstance = CreateRelationship (test, L"Employee", L"Company", L"EmployeeCompany");
    ASSERT_TRUE (relInstance.IsValid());
    PersistRelationship_Old (*relInstance, ecDbStore);
    ValidatePersistingRelationship (db, "sc_Employee", InstanceToId (*relInstance->GetSource()), 
        "Company__trg_11_id", InstanceToId (*relInstance->GetTarget()).GetValue ());
    ValidateReadingRelationship (test, L"EmployeeCompany", *relInstance);
    ValidateReadingRelated (test, L"EmployeeCompany", relInstance->GetSource(), relInstance->GetTarget());

    // Import a M-to-M relationship
    // EmployeeHardware
    //   Inherits: AssetRelationshipsBase, EmployeeRelationshipsBase
    //   Relates: Employee (M) -> Hardware (M)
    //   Should write to a new link table RK_Source, RK_Target columns
    db.SaveChanges();
    relInstance = CreateRelationship (test, L"Employee", L"Hardware", L"EmployeeHardware");
    ASSERT_TRUE (relInstance.IsValid());
    PersistRelationship_Old (*relInstance, ecDbStore);
    db.SaveChanges();
    ValidatePersistingRelationship (db, "sc_EmployeeHardware", InstanceToId (*relInstance), 
        "Employee__src_0N_id", InstanceToId (*relInstance->GetSource()).GetValue ());
    ValidatePersistingRelationship (db, "sc_EmployeeHardware", InstanceToId (*relInstance), 
        "Hardware__trg_0N_id", InstanceToId (*relInstance->GetTarget()).GetValue ());
    ValidatePersistingRelationship (db, "sc_EmployeeHardware", InstanceToId (*relInstance), 
        "RC_Target", ClassToId (relInstance->GetTarget()->GetClass()));

    ValidateReadingRelationship (test, L"EmployeeHardware", *relInstance);
    ValidateReadingRelated (test, L"EmployeeHardware", relInstance->GetSource(), relInstance->GetTarget());

    db.SaveChanges();
    }

END_ECDBUNITTESTS_NAMESPACE

