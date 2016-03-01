/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/BackwardCompatibility_Tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

//---------------------------------------------------------------------------------------
// @bsiclass                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
struct BackwardsCompatibilityTests : public DgnDbTestFixture
    {
    bool VerifyElementsAndModels()
        {
        bool status = true;
        DgnModels::Iterator modelsIterator = m_db->Models().MakeIterator();
        for (DgnModels::Iterator::Entry modelEntry : modelsIterator)
            {
            DgnModelPtr model = m_db->Models().GetModel(modelEntry.GetModelId());
            //printf ("modelName: %s \n", model->GetCode ().GetValue ().c_str ());
            if (!model.IsValid())
                {
                status = false;
                break;
                }
            model->FillModel();
            if (!model->IsFilled())
                {
                status = false;
                break;
                }
            DgnElementMap elements = model->GetElements();
            for (auto element : elements)
                {
                //printf ("elementDisplayLabel: %s \n", element.second->GetDisplayLabel().c_str());
                DgnElementId elementId = element.first;
                if (!elementId.IsValid())
                    {
                    status = false;
                    break;
                    }
                DgnElementCPtr elementCPtr = m_db->Elements().GetElement(elementId);
                if (!elementCPtr.IsValid())
                    {
                    status = false;
                    break;
                    }
                }
            }
        return status;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BackwardsCompatibilityTests, OpenDgndbInCurrent)
    {
    BeFileName compatibilityRoot;
    BeTest::GetHost().GetDocumentsRoot(compatibilityRoot);
    compatibilityRoot.AppendToPath(L"DgnDb");
    compatibilityRoot.AppendToPath(L"CompatibilityRoot");

    BeFileName srcFilesPath = compatibilityRoot;
    srcFilesPath.AppendToPath(L"DgnDb0601");
    srcFilesPath.AppendToPath(L"*.idgndb");

    FILE *f;
    BeFileName resultsFilePath;
    BeTest::GetHost().GetOutputRoot(resultsFilePath);
    resultsFilePath.AppendToPath(L"CompatibilityResults_06.txt");
    f = fopen(resultsFilePath.GetNameUtf8().c_str(), "a");
    fprintf(f, "Test Files Stream: DgnDb0601 \n");

    BeFileListIterator filesIterator(srcFilesPath, false);
    BeFileName fileName;
    while (filesIterator.GetNextFileName(fileName) != ERROR)
        {
        //printf ("dgndb Name: %s", fileName.GetNameUtf8 ().c_str ());
        DbResult status;
        m_db = DgnDb::OpenDgnDb(&status, fileName, DgnDb::OpenParams(Db::OpenMode::Readonly));
        if (DbResult::BE_SQLITE_OK == status && m_db.IsValid() && VerifyElementsAndModels())
            {
            fprintf(f, "SUCCESS: %ls \n", fileName.GetFileNameAndExtension().c_str());
            }
        else
            fprintf(f, "ERROR: %ls \n", fileName.GetFileNameAndExtension().c_str());
        }
    }