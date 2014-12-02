/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnFile.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnResourceURI.h>

#if defined (_MSC_VER)
    #pragma warning (disable:4355)
#endif // defined (_MSC_VER)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   05/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnModels::FreeQvCache ()
    {
    if (NULL == m_qvCache)
        return  false;

    // if there is a QvCache associated with this DgnFile, delete it too.
    T_HOST.GetGraphicsAdmin()._DeleteQvCache(m_qvCache);
    m_qvCache = NULL;
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Reclaims disk space by vacuuming the database
* @bsimethod                                                    KeithBentley    12/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFileStatus DgnProject::CompactFile()
    {
    if (1 < GetCurrentSavepointDepth())
        return  DGNFILE_ERROR_TransactionActive;

    Savepoint* savepoint = GetSavepoint(0);
    if (savepoint)
        savepoint->Commit();

    if (BE_SQLITE_OK != TryExecuteSql ("VACUUM"))
        return  DGNFILE_ERROR_SQLiteError;
    
    if (savepoint)
        savepoint->Begin();

    return DGNFILE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnModelStatus insertNewModel (DgnModels& models, DgnModels::Model& row)
    {
    row.GetNameR().Trim();
    row.GetDescriptionR().Trim();

    if (!models.IsValidName(row.GetName()))
        {
        BeAssert (false);
        return  DGNMODEL_STATUS_InvalidModelName;
        }

    if (models.QueryModelId(row.GetNameCP()).IsValid()) // can't allow two models with the same name
        return DGNMODEL_STATUS_DuplicateModelName;

    if (row.GetId().IsValid() && (SUCCESS == models.QueryModelById (NULL, row.GetId()))) // can't allow two models with the same ModelId
        return DGNMODEL_STATUS_DuplicateModelID;

    if (BE_SQLITE_OK != models.InsertModel (row))
        return DGNMODEL_STATUS_ModelTableWriteError;

    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP DgnModels::CreateNewModelFromSeed (DgnModelStatus* result, Utf8CP name, DgnModelId seedModelId)
    {
    DgnModelStatus t, &status(result ? *result : t);

    if (!seedModelId.IsValid())
        {
        status = DGNMODEL_STATUS_BadSeedModel;
        return NULL;
        }

    DgnModelP seedModel = GetModelById(seedModelId);
    if (NULL == seedModel)
        {
        status = DGNMODEL_STATUS_BadSeedModel;
        return NULL;
        }

    DgnModels::Model seedRow;
    if (SUCCESS != QueryModelById(&seedRow, seedModelId))
        {
        status = DGNMODEL_STATUS_BadSeedModel;
        return NULL;
        }

    DgnModels::Model newRow (seedRow);
    newRow.SetName (name);
    newRow.SetId(DgnModelId());

    status = CreateNewModel(newRow);
    if (DGNMODEL_STATUS_Success != status)
        return NULL;

    DgnModelP newModel = GetModelById(newRow.GetId());
    newModel->CopyPropertiesFrom(*seedModel);
    return newModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnModels::CreateNewModel (DgnModels::Model& model)
    {
    //  We create a DgnModel by:
    //  1. Write to the ModelTable
    DgnModelStatus status = insertNewModel (*this, model);
    if (status != DGNMODEL_STATUS_Success)
        return status;

    //  2. Save model properties to the properties table.
    //      Note: this just saves the default settings and properties for the model. 
    //              we must do at least this or LoadModelById will fail.
    DgnModelP tmpModel = nullptr;
    switch (model.GetModelType())
        {
        case DgnModelType::PhysicalRedline:
        case DgnModelType::Physical:
            tmpModel = new PhysicalModel(GetDgnProject(), model.GetId(), model.GetNameCP());
            break;

        case DgnModelType::Component:
            tmpModel = new ComponentModel(GetDgnProject(), model.GetId(), model.GetNameCP());
            break;

        case DgnModelType::Drawing:
            tmpModel = new DrawingModel(GetDgnProject(), model.GetId(), model.GetNameCP());
            break;

        case DgnModelType::Redline:
        case DgnModelType::Sheet:
            tmpModel = new SheetModel(GetDgnProject(), model.GetId(), model.GetNameCP());
            break;

        default:
            BeAssert (false);
            return DGNMODEL_STATUS_BadRequest;
        }

    tmpModel->SaveSettings();
    DgnModelStatus stat = (BE_SQLITE_OK == tmpModel->SaveProperties()) ? DGNMODEL_STATUS_Success : DGNMODEL_STATUS_BadRequest;
    delete tmpModel;

    if (!m_firstModelId.IsValid())
        m_firstModelId = model.GetId();

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             2/91
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnModels::GetUniqueModelName (Utf8CP baseName)
    {
    Utf8String tmpStr(baseName);

    if (!tmpStr.empty() && !m_project.Models().QueryModelId(tmpStr.c_str()).IsValid())
        return tmpStr;

    bool addDash = !tmpStr.empty();
    int index = 0;
    size_t lastDash = tmpStr.find_last_of ('-');
    if (lastDash != Utf8String::npos)
        {
        if (BE_STRING_UTILITIES_UTF8_SSCANF (&tmpStr[lastDash], "-%d", &index) == 1)
            addDash = false;
        else
            index = 0;
        }

    Utf8String uniqueModelName;
    do  {
        uniqueModelName.assign (tmpStr);
        if (addDash)
            uniqueModelName.append ("-");
        uniqueModelName.append (Utf8PrintfString("%d", ++index));
        } while (m_project.Models().QueryModelId (uniqueModelName.c_str()).IsValid());

    return uniqueModelName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DgnModels::GetFirstModelId() const
    {
    return m_firstModelId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModels::OnDbOpened()
    {
    m_firstModelId = MakeIterator(ModelIterate::All).begin().GetModelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP DgnModels::GetAndFillModelById (StatusInt* outStatus, DgnModelId modelID, bool fillCache)
    {
    StatusInt  tStatus, &status = (outStatus ? *outStatus : tStatus);

    DgnModelP dgnModel = GetModelById (modelID);
    if (dgnModel == NULL)
        {
        status = DGNMODEL_STATUS_NotFound;
        return NULL;
        }

    status = fillCache ? dgnModel->FillModel() : SUCCESS;
    return  dgnModel;
    }

