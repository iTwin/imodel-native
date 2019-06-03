/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/GeneratedInserts_test.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ClassificationSystems/GeneratedInsertsApi.h>
#include "ClassificationSystemsTestsBase.h"

USING_NAMESPACE_CLASSIFICATIONSYSTEMS

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
struct GeneratedTestFixture : public ClassificationSystemsTestsBase 
    {
    GeneratedTestFixture() {};
    ~GeneratedTestFixture() {};
    };

int countClassifications(ClassificationTableCR table)
    {
    int counter = 0;
    for(auto classification : table.MakeClassificationIterator())
        counter++;
    return counter;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Elonas.Seviakovas               04/2019
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(GeneratedTestFixture, InsertMasterFormatDefinitions_InsertsHierarchySuccessfully)
    {
    Dgn::DgnDbR db = *DgnClientFx::DgnClientApp::App().Project();
    db.BriefcaseManager().StartBulkOperation();

    GeneratedInserts generatedInserts;

    generatedInserts.InsertMasterFormatDefinitions(db, db.GetDictionaryModel ());

    ClassificationSystemCPtr system = ClassificationSystem::TryGet(db, db.GetDictionaryModel (), "MasterFormat", "2010");
    ASSERT_TRUE(system.IsValid()) << "Failed to create MasterFormat system";

    ClassificationTableCPtr table;
    for (auto childId : system->QueryChildren())
    {
        table = ClassificationTable::Get(db, childId);
        if(table.IsValid())
            break;
    }
        
    ASSERT_TRUE(table.IsValid()) << "Failed to create MasterFormat table";

    ASSERT_TRUE(countClassifications(*table) > 1) << "Not enough classifications created";

    db.SaveChanges();
    }