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
        CompatibilityStatus insertTestElementsAndModels();

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

    GeometryBuilderPtr builder = GeometryBuilder::CreateWorld(*geomElement);
    ICurvePrimitivePtr line = ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(1, 0, 0)));
    builder->Append(*line);
    if (SUCCESS != builder->SetGeometryStreamAndPlacement(*geomElement))
        return ERROR;

    out = element;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
CompatibilityStatus BackwardsCompatibilityTests::insertTestElementsAndModels()
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
    if (!m_db->IsReadonly())
        {
        status = insertTestElementsAndModels();
        if (status != CompatibilityStatus::Success)
            return status;
        }

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
    Utf8String activeStream = "DgnDb61-16Q2";

    BeFileName compatibilityRoot;
    BeTest::GetHost().GetDocumentsRoot(compatibilityRoot);
    compatibilityRoot.AppendToPath(L"DgnDb");
    compatibilityRoot.AppendToPath(L"CompatibilityRoot");
    compatibilityRoot.AppendToPath(L"DgnDb*");

    BeFileName outputRoot;
    BeTest::GetHost().GetOutputRoot(outputRoot);

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
        //printf("DirectoryName: %ls\n", dirName.GetFileNameWithoutExtension().c_str());
        if (BeFileName(dirName.GetFileNameWithoutExtension()) == BeFileName(activeStream))
            {
            continue;
            }

        BeFileName sourceFilesPath = dirName;
        sourceFilesPath.AppendToPath(L"*.ibim");

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
            if (BeFileNameStatus::Success == copyFileStatus)
                {
                DbResult status;
                m_db = DgnDb::OpenDgnDb(&status, outputFilePath, DgnDb::OpenParams(Db::OpenMode::Readonly));
                if (DbResult::BE_SQLITE_OK == status && m_db.IsValid())
                    {
                    stat = VerifyElementsAndModels();
                    }
                else
                    {
                    stat = CompatibilityStatus::ERROR_FileOpeningFailed;
                    }
                fprintf(f, "%ls, %ls, %s, %s\n", outputFilePath.GetFileNameAndExtension().c_str(), dirName.GetFileNameWithoutExtension().c_str(), activeStream.c_str(), getCompatibilityStatusString(stat));
                }
            else
                fprintf(f, "%ls, %ls, %s %s\n", dbName.GetFileNameAndExtension().c_str(), dirName.GetFileNameWithoutExtension().c_str(), activeStream.c_str(), "Error Copying File");
            }
        }
    }
