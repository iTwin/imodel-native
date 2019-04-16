/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

//---------------------------------------------------------------------------------------
// @bsiclass                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
enum class CompatibilityStatus
    {
    Success,
    ERROR_FileOpeningFailed,
    ERROR_BadElement,
    ERROR_BadModel,
    ERROR_ModelInsertFailed,
    ERROR_ElementInsertFailed,
    ERROR_ModelFillFailed
    };

struct BackwardsCompatibilityTests : public DgnDbTestFixture
    {
    private:
        StatusInt CreateArbitraryElement(DgnElementPtr& out, DgnModelR model);
        CompatibilityStatus insertTestElement();

    protected:
        CompatibilityStatus VerifyElementsAndModels();
        Utf8CP getCompatibilityStatusString(CompatibilityStatus num);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
StatusInt BackwardsCompatibilityTests::CreateArbitraryElement(DgnElementPtr& out, DgnModelR model)
    {
    DgnCategoryId categoryId = DgnDbTestUtils::GetFirstSpatialCategoryId(model.GetDgnDb());
    if (!categoryId.IsValid())
        return ERROR;

    DgnElementPtr element = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(model.GetDgnDb(), model.GetModelId(), DgnClassId(model.GetDgnDb().Schemas().GetClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject)), categoryId, Placement3d()));
    if (!element.IsValid())
        return ERROR;

    GeometrySourceP geomElement = element->ToGeometrySourceP();
    if (nullptr == geomElement)
        return ERROR;

    geomElement->SetCategoryId(categoryId);

    GeometryBuilderPtr builder = GeometryBuilder::Create(*geomElement);
    ICurvePrimitivePtr line = ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(1, 0, 0)));
    builder->Append(*line);
    if (SUCCESS != builder->Finish(*geomElement))
        return ERROR;

    out = element;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
CompatibilityStatus BackwardsCompatibilityTests::insertTestElement()
    {
    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "newModel");

    DgnElementPtr element;
    DgnDbStatus insertStatus;
    for (int i = 0; i < 3; i++)
        {
        CreateArbitraryElement(element, *model);

        model->GetDgnDb().Elements().Insert(*element, &insertStatus);
        if (DgnDbStatus::Success != insertStatus)
            return CompatibilityStatus::ERROR_ElementInsertFailed;
        }

    m_db->SaveChanges();
    return CompatibilityStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
CompatibilityStatus BackwardsCompatibilityTests::VerifyElementsAndModels()
    {
    CompatibilityStatus status = CompatibilityStatus::Success;
    if (!m_db->IsReadonly())
        {
        status = insertTestElement();
        if (status != CompatibilityStatus::Success)
            return status;
        }

    for (ModelIteratorEntryCR modelEntry : m_db->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_Model)))
        {
        DgnModelPtr model = m_db->Models().GetModel(modelEntry.GetModelId());
        //printf("modelName: %s \n", model->GetCode().GetValue().c_str());
        if (!model.IsValid())
            return CompatibilityStatus::ERROR_BadModel;
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP BackwardsCompatibilityTests::getCompatibilityStatusString(CompatibilityStatus stat)
    {
    switch (stat)
        {
            case CompatibilityStatus::Success:
                return "SUCCESS";
                break;
            case CompatibilityStatus::ERROR_FileOpeningFailed:
                return "ERROR_FileOpeningFailed";
                break;
            case CompatibilityStatus::ERROR_BadElement:
                return "ERROR_BadElement";
                break;
            case CompatibilityStatus::ERROR_BadModel:
                return "ERROR_BadModel";
                break;
            case CompatibilityStatus::ERROR_ModelInsertFailed:
                return "ERROR_ModelInsertFailed";
                break;
            case CompatibilityStatus::ERROR_ElementInsertFailed:
                return "ERROR_ElementInsertFailed";
                break;
            case CompatibilityStatus::ERROR_ModelFillFailed:
                return "ERROR_ModelFillFailed";
                break;
            default:
                return "Bad Enum Value";
                break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BackwardsCompatibilityTests, OpenDgndbInCurrent)
    {
    Utf8String activeStream = "Bim0200";

    BeFileName compatibilityRoot;
    BeTest::GetHost().GetDocumentsRoot(compatibilityRoot);
    compatibilityRoot.AppendToPath(L"DgnDb");
    compatibilityRoot.AppendToPath(L"CompatibilityRoot");
    compatibilityRoot.AppendToPath(L"DgnDb*");

    BeFileName outputRoot;
    BeTest::GetHost().GetOutputRoot(outputRoot);
    WString testCaseName(TEST_FIXTURE_NAME, true);
    outputRoot.AppendToPath(testCaseName.c_str());
    if (!outputRoot.DoesPathExist())
        ASSERT_TRUE(BeFileNameStatus::Success == BeFileName::CreateNewDirectory(outputRoot.c_str()));

    BeFileName resultsFilePath = outputRoot;
    resultsFilePath.AppendToPath(L"CompatibilityResults_").AppendA(activeStream.c_str()).AppendA(".csv");

    FILE *f;
    f = fopen(resultsFilePath.GetNameUtf8().c_str(), "a");
    if (f != NULL)
        {
        fprintf(f, "FileName, ConvertedThrough, TestedIn, FileOpeningStatus\n");
        }
    else
        ASSERT_TRUE(false) << "Error opening csv file";

    BeFileListIterator dirIterator(compatibilityRoot, false);
    BeFileName dirName;
    while (dirIterator.GetNextFileName(dirName) != ERROR)
        {
        printf("DirectoryName: %ls\n", dirName.GetFileNameAndExtension().c_str());
        if (BeFileName(dirName.GetFileNameWithoutExtension()) == BeFileName(activeStream))
            {
            continue;
            }

        BeFileName sourceFilesPath = dirName;
        sourceFilesPath.AppendToPath(L"*.idgndb");

        BeFileListIterator filesIterator(sourceFilesPath, false);
        BeFileName dbName;

        while (filesIterator.GetNextFileName(dbName) != ERROR)
            {
            //printf ("dgndb Name: %s", dbName.GetNameUtf8 ().c_str ());
            BeFileName outputFilePath = outputRoot;
            outputFilePath.AppendToPath(dbName.GetFileNameAndExtension().c_str());
            //printf("dgndb Name: %s", outputFilePath.GetNameUtf8().c_str());

            BeFileNameStatus copyFileStatus = BeFileName::BeCopyFile(dbName, outputFilePath);
            CompatibilityStatus stat = CompatibilityStatus::Success;
            bool writeStatus;
            if (BeFileNameStatus::Success == copyFileStatus)
                {
                DbResult status;
                m_db = DgnDb::OpenDgnDb(&status, outputFilePath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
                if (DbResult::BE_SQLITE_OK == status && m_db.IsValid())
                    {
                    stat = VerifyElementsAndModels();
                    if (stat == CompatibilityStatus::Success)
                        writeStatus = true;
                    else
                        writeStatus = false;
                    }
                else
                    {
                    stat = CompatibilityStatus::ERROR_FileOpeningFailed;
                    writeStatus = false;
                    }

                if (writeStatus)
                    fprintf(f, "%ls, %ls, %s, %s\n", outputFilePath.GetFileNameAndExtension().c_str(), dirName.GetFileNameWithoutExtension().c_str(), activeStream.c_str(), getCompatibilityStatusString(stat));
                else
                    {
                    fprintf(f, "%ls, %ls, %s, %s\n", outputFilePath.GetFileNameAndExtension().c_str(), dirName.GetFileNameWithoutExtension().c_str(), activeStream.c_str(), getCompatibilityStatusString(stat));
                    EXPECT_TRUE(false) << getCompatibilityStatusString(stat);
                    }
                }
            else
                {
                fprintf(f, "%ls, %ls, %s %s\n", dbName.GetFileNameAndExtension().c_str(), dirName.GetFileNameWithoutExtension().c_str(), activeStream.c_str(), "Error Copying File");
                EXPECT_TRUE(false) << "copying file failed for:" << dbName.GetFileNameAndExtension().c_str();
                }
            }
        }
    fclose(f);
    }
