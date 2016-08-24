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
    DgnCategoryId categoryId = DgnCategory::QueryFirstCategoryId(model.GetDgnDb());
    if (!categoryId.IsValid())
        return ERROR;

    DgnElementPtr element = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(model.GetDgnDb(), model.GetModelId(), DgnClassId(model.GetDgnDb().Schemas().GetECClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASSNAME_PhysicalObject)), categoryId, Placement3d()));
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
    DgnClassId mclassId = DgnClassId(m_db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SpatialModel));
    SpatialModelPtr model = new SpatialModel(SpatialModel::CreateParams(*m_db, mclassId, DgnModel::CreateModelCode("newModel")));
    if (DgnDbStatus::Success != model->Insert()) /* Insert the new model into the DgnDb */
        return CompatibilityStatus::ERROR_ModelInsertFailed;

    DgnElementPtr element;
    DgnDbStatus insertStatus;
    for (int i = 0; i < 3; i++)
        {
        CreateArbitraryElement(element, *model);

        model->GetDgnDb().Elements().Insert(*element, &insertStatus);
        if (DgnDbStatus::Success != insertStatus)
            return CompatibilityStatus::ERROR_ElementInsertFailed;
        }

    return CompatibilityStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
CompatibilityStatus BackwardsCompatibilityTests::VerifyElementsAndModels()
    {
    CompatibilityStatus status = CompatibilityStatus::Success;
    insertTestElement();
    DgnModels::Iterator modelsIterator = m_db->Models().MakeIterator();
    for (DgnModels::Iterator::Entry modelEntry : modelsIterator)
        {
        DgnModelPtr model = m_db->Models().GetModel(modelEntry.GetModelId());
        //printf("modelName: %s \n", model->GetCode().GetValue().c_str());
        if (!model.IsValid())
            {
            status = CompatibilityStatus::ERROR_BadModel;
            break;
            }
        model->FillModel();
        if (!model->IsFilled())
            {
            status = CompatibilityStatus::ERROR_ModelFillFailed;
            break;
            }

        DgnElementMap elements = model->GetElements();
        for (auto element : elements)
            {
            //printf("elementDisplayLabel: %s \n", element.second->GetDisplayLabel().c_str());
            DgnElementId elementId = element.first;
            DgnElementCPtr elementCPtr = m_db->Elements().GetElement(elementId);
            if (!elementCPtr.IsValid())
                {
                status = CompatibilityStatus::ERROR_BadElement;
                break;
                }
            }
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
    BeFileName srcFilesPath;
    BeTest::GetHost().GetDocumentsRoot(srcFilesPath);
    srcFilesPath.AppendToPath(L"DgnDb");
    srcFilesPath.AppendToPath(L"CompatibilityRoot");
    srcFilesPath.AppendToPath(L"DgnDb61-16Q2");
    srcFilesPath.AppendToPath(L"*.idgndb");

    BeFileName outputRoot;
    BeTest::GetHost().GetOutputRoot(outputRoot);
    WString testCaseName(TEST_FIXTURE_NAME, true);
    outputRoot.AppendToPath(testCaseName.c_str());
    if (!outputRoot.DoesPathExist())
        ASSERT_TRUE(BeFileNameStatus::Success == BeFileName::CreateNewDirectory(outputRoot.c_str()));

    BeFileName resultsFilePath = outputRoot;
    resultsFilePath.AppendToPath(L"CompatibilityResults_DgnDb0601.csv");

    FILE *f;
    f = fopen(resultsFilePath.GetNameUtf8().c_str(), "a");
    if (f != NULL)
        {
        fprintf(f, "FileName, ConvertedThrough, TestedIn, FileOpeningStatus\n");
        }
    else
        ASSERT_TRUE(false) << "Error opening csv file";

    BeFileListIterator filesIterator(srcFilesPath, false);
    BeFileName dbName;

    while (filesIterator.GetNextFileName(dbName) != ERROR)
        {
        //printf ("dgndb Name: %s", fileName.GetNameUtf8 ().c_str ());
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
                fprintf(f, "%ls, DgnDb61-16Q2, DgnDb0601, %s\n", outputFilePath.GetFileNameAndExtension().c_str(), getCompatibilityStatusString(stat));
            else
                {
                fprintf(f, "%ls, DgnDb61-16Q2, DgnDb0601, %s\n", outputFilePath.GetFileNameAndExtension().c_str(), getCompatibilityStatusString(stat));
                EXPECT_TRUE(false) << getCompatibilityStatusString(stat);
                }
            }
        else
            {
            fprintf(f, "%ls, DgnDb61-16Q2, DgnDb0601, %s\n", dbName.GetFileNameAndExtension().c_str(), "Error Copying File");
            EXPECT_TRUE(false) << "copying file failed for:" << dbName.GetFileNameAndExtension().c_str();
            }
        }
    fclose(f);
    }
