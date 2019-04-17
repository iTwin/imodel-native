/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

#include <Raster/RasterApi.h>
#include <Raster/RasterFileHandler.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

DWG_PROTOCOLEXT_DEFINE_MEMBERS(DwgRasterImageExt)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgRasterImageExt::GetExistingModel (ResolvedModelMapping& modelMap)
    {
    DwgDbObjectId   rasterId = m_dwgRaster->GetObjectId ();
    Transform       toDgn = m_toBimContext->GetTransform();

    if (m_importer->IsUpdating())
        {
        DwgDbDatabaseP  dwg = rasterId.GetDatabase ();
        if (nullptr == dwg)
            dwg = &m_importer->GetDwgDb ();

        DwgImporter::RootTransformInfo const&   rootTransInfo = m_importer->GetRootTransformInfo ();
        bool    searchByOldTrans = rootTransInfo.HasChanged() && m_dwgRaster->GetOwnerId() == m_importer->GetModelSpaceId();
        Transform   searchTrans;
        if (searchByOldTrans)
            {
            Transform   fromNewToOld = rootTransInfo.GetChangeTransformFromNewToOld ();
            searchTrans.InitProduct (fromNewToOld, toDgn);
            }
        else
            {
            // no root model transform change:
            searchTrans = toDgn;
            }

        // search the syncInfo
        modelMap = m_importer->GetModelFromSyncInfo (rasterId, *dwg, searchTrans);
        if (modelMap.IsValid() && modelMap.GetModel() != nullptr && modelMap.GetMapping().GetSourceType() == DwgSyncInfo::ModelSourceType::RasterAttachment)
            {
            if (searchByOldTrans)
                {
                // update model map with the new transform
                modelMap.SetTransform (toDgn);
                modelMap.GetMapping().Update (m_importer->GetDgnDb());
                }
            return  true;
            }
        // not found in syncinfo => treat as insert
        }

    // see if this model has already been created
    modelMap = m_importer->FindModel (rasterId, toDgn, DwgSyncInfo::ModelSourceType::RasterAttachment);

    return  modelMap.IsValid() && modelMap.GetModel() != nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgRasterImageExt::GetRasterMatrix (DMatrix4dR matrixOut)
    {
    // transform the raster from pixels to world:
    Transform   pixelToModel;
    m_dwgRaster->GetOrientation (pixelToModel, true);

    // transform raster to model
    Transform   toDgn = m_toBimContext->GetTransform();
    Transform   pixelToDgn = Transform::FromProduct (pixelToModel, toDgn);

    // scale its origin to DgnDb
    DPoint3d    origin;
    pixelToModel.GetTranslation (origin);
    origin.Scale (toDgn.ColumnXMagnitude());
    pixelToDgn.SetTranslation (origin);

    matrixOut = DMatrix4d::From (pixelToDgn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgRasterImageExt::ClipRasterModel (Raster::RasterFileModel& model)
    {
    if (!m_dwgRaster->IsClipped() || !m_dwgRaster->IsShownClipped())
        {
        if (m_importer->IsUpdating())
            {
            Raster::RasterClip  clipper = model.GetClip ();
            if (!clipper.IsEmpty())
                {
                clipper.Clear ();
                model.SetClip (clipper);
                return  true;
                }
            }
        return  false;
        }

    DPoint3dArray   clipPoints;
    if (m_dwgRaster->GetClippingBoundary(clipPoints) < 3)
        return  false;

    Transform   toDgn = m_toBimContext->GetTransform();
    toDgn.Multiply (clipPoints.data(), static_cast<int>(clipPoints.size()));

    CurveVector::BoundaryType   boundaryType = m_dwgRaster->IsClipInverted() ? CurveVector::BOUNDARY_TYPE_Inner : CurveVector::BOUNDARY_TYPE_Outer;
    CurveVectorPtr              curveVector = CurveVector::CreateLinear (clipPoints.data(), clipPoints.size(), boundaryType, false);
    if (!curveVector.IsValid())
        return  false;
    
    Raster::RasterClip    clipper;
    if (CurveVector::BOUNDARY_TYPE_Outer == boundaryType)
        clipper.SetBoundary (curveVector.get());
    else
        clipper.AddMask (*curveVector.get());

    model.SetClip (clipper);

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgRasterImageExt::CopyRasterToDgnDbFolder (BeFileNameCR rasterFile, BeFileNameCR dbFile, BeFileNameCR altPath)
    {
    // build full source file path from input raster path and/or active path:
    BeFileName  sourceFilename = rasterFile.DoesPathExist() ? rasterFile : altPath;
    if (!sourceFilename.DoesPathExist())
        return  false;
        
    // copy to DgnDb root folder:
    WString     rootName = rasterFile.GetFileNameAndExtension ();
    BeFileName  destFilename = dbFile.GetDirectoryName ();
    destFilename.AppendToPath (rootName.c_str());

    BeFileNameStatus    status = BeFileName::BeCopyFile (sourceFilename, destFilename);
    return status == BeFileNameStatus::Success || status == BeFileNameStatus::AlreadyExists;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DwgRasterImageExt::GetUrlCacheFile (DwgStringR checkPath)
    {
#ifdef BENTLEY_WIN32
    if (::PathIsURLW(checkPath.c_str()))
        {
        WString localPath;
        if (DwgImportHost::GetHost().GetCachedLocalFile(localPath, WString(checkPath.c_str())))
            {
            checkPath.Assign (localPath.c_str());
            return  true;
            }
        else
            {
            m_importer->ReportError (IssueCategory::DiskIO(), Issue::Message(), "Failed downloading a raster file from URL or could not find its cached local path!");
            }
        }
#endif 
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgRasterImageExt::_ConvertToBim (ProtocolExtensionContext& context, DwgImporter& importer)
    {
    m_toBimContext = &context;
    m_importer = &importer;
    m_dwgRaster = DwgDbRasterImage::Cast(&context.GetEntity());
    if (nullptr == m_dwgRaster)
        return  BSIERROR;

    // if user does not want raster images, do as wished.
    if (!importer.GetOptions().GetImportRasterAttachments())
        return  BSISUCCESS;

    // get DWG file name
    DwgString   rasterPath, activePath;
    if (DwgDbStatus::Success != m_dwgRaster->GetFileName(rasterPath, &activePath))
        return  BSIERROR;
 
    this->GetUrlCacheFile (rasterPath);

    // see if there exists the model:
    ResolvedModelMapping modelMap;
    this->GetExistingModel (modelMap);

    BentleyStatus   status = BSIERROR;
    BeFileName      rasterFilename (rasterPath.c_str());
    DgnModelP       model = modelMap.GetModel ();

    if (nullptr == model)
        status = this->CreateRasterModel (rasterFilename, BeFileName(activePath));
    else
        status = this->UpdateRasterModel (modelMap, rasterFilename, BeFileName(activePath));

    // return the raster model back to the caller:
    context.SetResultantModel (model);
    
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgRasterImageExt::CreateRasterModel (BeFileNameCR rasterFilename, BeFileNameCR activePath)
    {
    // create a new model with raster object id suffix
    Utf8String  fileId;
    if (BSISUCCESS != T_HOST.GetRasterAttachmentAdmin()._CreateFileUri(fileId, Utf8String(rasterFilename.c_str())))
        return  BSIERROR;

    DwgDbDatabasePtr    dwg = m_dwgRaster->GetDatabase ();
    if (dwg.IsNull())
        return  BSIERROR;

    DwgDbObjectId   rasterId = m_dwgRaster->GetObjectId ();

    // the default raster admin expects raster file to be located on the DgnDb folder:
    if (!this->CopyRasterToDgnDbFolder(rasterFilename, m_importer->GetDgnDb().GetFileName(), BeFileName(activePath.c_str())))
        {
        auto from = rasterFilename.DoesPathExist() ? rasterFilename.c_str() : activePath.c_str();
        auto to = m_importer->GetDgnDb().GetFileName().GetDirectoryName().c_str ();
        m_importer->ReportError (IssueCategory::DiskIO(), Issue::CantCreateRaster(), Utf8PrintfString("failed copying file %ls to %ls", from, to).c_str());
        return BSIERROR;
        }

    // build the transformation for the raster
    DMatrix4d   matrix;
    this->GetRasterMatrix (matrix);

    Utf8String  proposedName (rasterFilename.GetFileNameWithoutExtension().c_str());

    Utf8String      idSuffix;
    idSuffix.Sprintf ("(%llx)", rasterId.ToUInt64());

    // propose a model name from DWG file name, raster file name and out file suffix:
    BeFileName  baseFilename = BeFileName (BeFileName::Basename, dwg->GetFileName().c_str());
    Utf8String  modelName = m_importer->ComputeModelName (proposedName, baseFilename, rasterFilename, idSuffix.c_str(), DgnClassId());
    DgnCode     linkCode = RepositoryLink::CreateUniqueCode (*m_importer->GetDgnDb().GetRealityDataSourcesModel(), modelName.c_str());

    RepositoryLinkPtr repositoryLink = RepositoryLink::Create(*m_importer->GetDgnDb().GetRealityDataSourcesModel(), fileId.c_str(), linkCode.GetValueUtf8CP());
    if (!repositoryLink.IsValid() || !repositoryLink->Insert().IsValid())
        return BSIERROR;

    // create a new model
    Raster::RasterFileModelPtr rasterModel = Raster::RasterFileModelHandler::CreateRasterFileModel(Raster::RasterFileModel::CreateParams(m_importer->GetDgnDb(), *repositoryLink, &matrix));
    if (!rasterModel.IsValid())
        return BSIERROR;

    // clip the model, if one exists
    this->ClipRasterModel (*rasterModel.get());

    // add the model into DgnDb
    if (DgnDbStatus::Success != rasterModel->Insert())
        {
        m_importer->ReportError (IssueCategory::Unknown(), Issue::CantCreateModel(), Utf8PrintfString("<%s (%I64d)>", fileId.c_str(), rasterId.ToUInt64()).c_str());
        return  BSIERROR;
        }

    DgnModelId  modelId = rasterModel->GetModelId ();
    DgnModelP   model = m_importer->GetDgnDb().Models().GetModel(modelId).get ();
    if (nullptr == model)
        {
        m_importer->ReportError (IssueCategory::Unknown(), Issue::CantCreateRaster(), Utf8PrintfString("%s (%I64d)", fileId.c_str(), rasterId.ToUInt64()).c_str());
        return BSIERROR;
        }

    Transform   toDgn = m_toBimContext->GetTransform();

    // add the model to the sync info:
    DwgSyncInfo::DwgModelMapping   mapping;
    auto    rc = m_importer->GetSyncInfo().InsertModel (mapping, model->GetModelId(), *m_dwgRaster, toDgn);
    if (rc != BSISUCCESS)
        {
        BeAssert (false && "Raster model cannot be inserted into DwgSync DB!");
        return BSIERROR;
        }

    if (LOG_MODEL_IS_SEVERITY_ENABLED(NativeLogging::LOG_TRACE))
        LOG_MODEL.tracev("+ %s %d -> %s %d", mapping.GetDwgName().c_str(), mapping.GetDwgModelId().GetValue(), model->GetName().c_str(), model->GetModelId().GetValue());

    ResolvedModelMapping   modelMap(rasterId, model, mapping);

    // save the model info in our list of known model mappings.
    m_importer->AddToDwgModelMap (modelMap);
    // tell the updater about the newly discovered model
    m_importer->_GetChangeDetector()._OnModelInserted (*m_importer, modelMap, nullptr);
    m_importer->_GetChangeDetector()._OnModelSeen (*m_importer, modelMap);

    // add the new model ID to views
    if (m_dwgRaster->IsDisplayed())
        this->AddModelToViews (modelId);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgRasterImageExt::AddModelToViews (DgnModelId modelId)
    {
    DgnDbR          db = m_importer->GetDgnDb ();
    DwgDbObjectIdCR currentspace = m_importer->GetCurrentSpaceId ();
    DwgDbObjectIdCR modelspace = m_importer->GetModelSpaceId ();

    // if we are in modelspace, add the model to the modelspace view; otherwise add it to paperspace view:
    for (auto const& entry : ViewDefinition::MakeIterator(db))
        {
        auto view = ViewDefinition::Get (db, entry.GetId());

        if (view.IsValid())
            {
            ViewControllerPtr   viewController = view->LoadViewController ();
            if (viewController.IsValid())
                {
                auto spatialView = viewController->ToSpatialViewP ();
                if (currentspace == modelspace && nullptr != spatialView)
                    {
                    // add the raster model into the modelspace view:
                    auto&   modelSelector = spatialView->GetSpatialViewDefinition().GetModelSelector ();
                    modelSelector.AddModel (modelId);
                    modelSelector.Update ();
                    m_importer->SaveViewDefinition (*viewController);
                    continue;
                    }
                else
                    {
                    // WIP - add model to paperspace view
                    m_importer->ReportError (IssueCategory::Unsupported(), Issue::Message(), Utf8PrintfString("adding a raster model in sheet view <%s (%I64d)>", view->GetName().c_str(), modelId.GetValue()).c_str());
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgRasterImageExt::UpdateViews (DgnModelId modelId, bool isOn)
    {
    DgnDbR          db = m_importer->GetDgnDb ();
    DwgDbObjectIdCR currentspace = m_importer->GetCurrentSpaceId ();
    DwgDbObjectIdCR modelspace = m_importer->GetModelSpaceId ();

    for (auto const& entry : ViewDefinition::MakeIterator(db))
        {
        auto view = ViewDefinition::Get (db, entry.GetId());

        if (view.IsValid())
            {
            ViewControllerPtr   viewController = view->LoadViewController ();
            if (viewController.IsValid())
                {
                auto spatialView = viewController->ToSpatialViewP ();
                if (currentspace == modelspace && nullptr != spatialView)
                    {
                    // add or drop the raster model from modelspace view
                    auto&   modelSelector = spatialView->GetSpatialViewDefinition().GetModelSelector ();
                    bool    wasOn = modelSelector.ContainsModel (modelId);

                    if (isOn && !wasOn)
                        modelSelector.AddModel (modelId);
                    else if (!isOn && wasOn)
                        modelSelector.DropModel (modelId);
                    else
                        continue;

                    modelSelector.Update ();
                    m_importer->SaveViewDefinition (*viewController);
                    continue;
                    }
                else
                    {
                    // WIP - add or drop the rater model from paperspace view
                    m_importer->ReportError (IssueCategory::Unsupported(), Issue::Message(), Utf8PrintfString("adding a raster mode in sheet view <%s (%I64d)>", view->GetName().c_str(), modelId.GetValue()).c_str());
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgRasterImageExt::UpdateRasterModel (ResolvedModelMapping& modelMap, BeFileNameCR rasterFilename, BeFileNameCR activePath)
    {
    Raster::RasterFileModelP    rasterModel = dynamic_cast<Raster::RasterFileModelP> (modelMap.GetModel());
    if (nullptr == rasterModel) 
        return  BSIERROR;

    // check if the raster has been moved or rotated
    DMatrix4d   newMatrix, oldMatrix;
    this->GetRasterMatrix (newMatrix);
    oldMatrix = rasterModel->GetSourceToWorld ();

    bool    diffFound = false;
    for (int i = 0; i < 4; i++)
        {
        for (int j = 0; j < 4; j++)
            {
            if (fabs(newMatrix.coff[i][j] - oldMatrix.coff[i][j]) > 0.01)
                {
                diffFound = true;
                break;
                }
            }
        if (diffFound)
            break;
        }
    if (diffFound)
        {
        // WIP - set the new matrix
        BeDataAssert (false && "need to support editing RasterFileModel!");
        m_importer->ReportError (IssueCategory::Unsupported(), Issue::Message(), Utf8PrintfString("changing raster model origin/size <%s (%I64d)>", rasterModel->GetName().c_str(), rasterModel->GetModelId().GetValue()).c_str());
        }

    // update clipper
    if (this->ClipRasterModel(*rasterModel))
        rasterModel->Update ();

    // turn "on/off" the model accordingly:
    this->UpdateViews (rasterModel->GetModelId(), m_dwgRaster->IsDisplayed());

    // save the model info in our list of known model mappings.
    m_importer->AddToDwgModelMap (modelMap);
    // tell the updater about the newly discovered model
    m_importer->_GetChangeDetector()._OnModelSeen (*m_importer, modelMap);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DwgImporter::AddToDwgModelMap (ResolvedModelMapping const& modelMap)
    {
    // if we are using m_dwgModelMap, don't add to it!
    if (m_isProcessingDwgModelMap)
        return  false;

    m_dwgModelMap.insert (modelMap);
    return  true;
    }
