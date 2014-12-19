/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/NonPublished/ECDb/ECDbSchemas_Test_Old.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/NonPublished/ECDb/ECDbTestProject.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE


extern void WriteECSchemaDiffToLog (ECDiffR diff, NativeLogging::SEVERITY severity = NativeLogging::LOG_INFO);
extern void PopulatePrimitiveValueWithCustomDataSet2 (ECValueR value, PrimitiveType primitiveType, ECPropertyCP ecProperty);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ECDbSchemas_Old, UpdatingExistingECSchema)
    {
    // Save a test project
    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create ("RSComponents.ecdb", L"RSComponents.01.00.ecschema.xml", true);

    ECSchemaPtr modifiedECSchema;
    ECSchemaReadContextPtr schemaContext = nullptr;

    ECDbTestUtility::ReadECSchemaFromDisk (modifiedECSchema, schemaContext, L"RSComponents.02.00.ecschema.xml", nullptr);
    ASSERT_TRUE(modifiedECSchema.IsValid());
    modifiedECSchema->SetVersionMajor(1); //Major version should match
    modifiedECSchema->SetVersionMinor(1); //Minor version should be greater then existing schema minor version

    auto importSchemaStatus = db.GetEC ().GetSchemaManager ().ImportECSchemas (schemaContext->GetCache (), ECDbSchemaManager::ImportOptions (true, true));
    ASSERT_EQ (SUCCESS, importSchemaStatus);

    ECSchemaP  updatedECSchema;
    db.GetEC().GetSchemaManager().GetECSchema(updatedECSchema, "RSComponents");

    ECDiffPtr diff = ECDiff::Diff(*updatedECSchema, *modifiedECSchema);
    ASSERT_EQ (diff->GetStatus() , DIFFSTATUS_Success);
    if (!diff->IsEmpty())
        {
        bmap<WString, DiffNodeState> searchResults;
        diff->GetNodesState(searchResults, L"*.ArrayInfo");
        if (!searchResults.empty())
            LOG.error(L"*** Feature missing : Array type property Maxoccurs and Minoccurs are not stored currently by ECDbSchemaManager");
        WriteECSchemaDiffToLog(*diff, NativeLogging::LOG_ERROR);
        ASSERT_TRUE(false && "There should be no difference between in memory and stored ECSchema after update");
        }
    //Read back schema and generate some new instance with additional properties
    //ECSchemaP storedSchema;
    //db.GetEC().GetSchemaManager().GetECSchema(storedSchema, "RSComponents", true);
    for(auto ecClass : modifiedECSchema->GetClasses())
        {
        if (ecClass->GetRelationshipClassCP() || ecClass->GetIsStruct() || ecClass->GetIsCustomAttributeClass())
            continue; 

        auto persistence = db.GetEC().GetECPersistence (nullptr, *ecClass);
        if (persistence.IsNull())
            {
            LOG.errorv(L"Failed to get persistence for %ls", ecClass->GetName().c_str());

            }
        ASSERT_TRUE (persistence.IsValid());
        for( int i=0; i<3; i++)
            {
            auto newInst = ECDbTestProject::CreateArbitraryECInstance (*ecClass, PopulatePrimitiveValueWithCustomDataSet2);
            auto insertStatus = persistence->Insert (nullptr, *newInst);
            ASSERT_EQ (INSERT_Success, insertStatus);    
            }
        }

    }


END_ECDBUNITTESTS_NAMESPACE
