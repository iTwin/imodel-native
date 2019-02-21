/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/ScalableMeshConversion.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <ScalableMesh/ScalableMeshDefs.h>
#include <VersionedDgnV8Api/ScalableMesh/ScalableMeshDefs.h>
#include <VersionedDgnV8Api/DgnPlatform/ScalableMeshBaseElementHandler.h>
//#include <VersionedDgnV8Api/ScalableMeshElement/ScalableMeshAttachment.h>


USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA


struct DgnV8ThreeMxClipFlags
{
    unsigned        m_isMask:1;
    unsigned        m_clipZLow:1;
    unsigned        m_clipZHigh:1;
    unsigned        m_reserved:29;
};


struct ScalableMeshModelInfo
    {
    ScalableMeshModel*                                   m_model;    
    DGNV8_BENTLEY_NAMESPACE_NAME::DgnPlatform::ElementId m_elemId;
    BentleyB0200::bvector<BentleyB0200::bpair<uint64_t, uint64_t>>     m_linksToGroundModels;
    Bentley::bvector<bool>                               m_linkedModelIdResolved;
    };

static std::vector<ScalableMeshModelInfo> s_modelInfos;


void ResolveLinkedModelId(ScalableMeshModel* model, DGNV8_BENTLEY_NAMESPACE_NAME::DgnPlatform::ElementId elemId, Bentley::bvector<Bentley::bpair<uint64_t, uint64_t>>& linksToModelsV8)
    {
    ScalableMeshModelInfo modelInfo; 
    
    modelInfo.m_model = model;
    modelInfo.m_elemId = elemId;

    bool needToResolved = linksToModelsV8.size() > 0;
    bool allResolved = true;

    for (auto& modelIter : linksToModelsV8)
        {        
        uint64_t linkedElemId = modelIter.first;        
        bool     isModelFound = false;
        BentleyB0200::bpair<uint64_t, uint64_t> linksToGround;

        linksToGround.first = modelIter.first;
        linksToGround.second = modelIter.second;

        for (auto& modelInfoIter : s_modelInfos)
            { 
            if (modelInfoIter.m_elemId == linkedElemId)
                {
                isModelFound = true;
                linksToGround.second = (uint64_t)modelInfoIter.m_model->GetModelId().GetValue();                
                break;
                }            
            }

        if (!isModelFound)
            allResolved = false;

        modelInfo.m_linksToGroundModels.push_back(linksToGround);
        modelInfo.m_linkedModelIdResolved.push_back(isModelFound);
        }   

    if (needToResolved && allResolved)
        {
        modelInfo.m_model->SetGroundModelLinks(modelInfo.m_linksToGroundModels);
        modelInfo.m_model->Update();
        }

    for (auto& modelInfoIter : s_modelInfos)
        {        
        needToResolved = false;
        allResolved = true;

        for (size_t modelId = 0; modelId < modelInfoIter.m_linkedModelIdResolved.size(); modelId++)
            {
            if (modelInfoIter.m_linkedModelIdResolved[modelId] == false)
                { 
                needToResolved = true;

                if (modelInfoIter.m_linksToGroundModels[modelId].first == modelInfo.m_elemId)
                    {
                    modelInfoIter.m_linksToGroundModels[modelId].first = modelInfo.m_model->GetModelId().GetValue();                    
                    }
                else
                    {
                    allResolved = false;
                    }
                }
            }

        if (needToResolved && allResolved)
            {
            modelInfoIter.m_model->SetGroundModelLinks(modelInfoIter.m_linksToGroundModels);
            modelInfoIter.m_model->Update();
            }
        }

    s_modelInfos.push_back(modelInfo);
    }


Bentley::BentleyStatus ExtractClipDefinitionsInfo(Bentley::bmap<uint64_t, Bentley::bpair<DGNV8_BENTLEY_NAMESPACE_NAME::ScalableMesh::SMNonDestructiveClipType, Bentley::bvector<Bentley::DPoint3d>>> * clipDefs, DgnV8EhCR eh);
Bentley::BentleyStatus ExtractTerrainLinkInfo(Bentley::bvector<Bentley::bpair<uint64_t, uint64_t>>* linksToModels, DgnV8EhCR eh);





/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 07/17
+---------------+---------------+---------------+---------------+---------------+------*/
ConvertToDgnDbElementExtension::Result ConvertScalableMeshAttachment::_PreConvertElement(DgnV8EhCR v8el, Converter& converter, ResolvedModelMapping const& v8mm)
    {
    Transform               location;
    ClipVectorPtr           clipVector;
    uint32_t                activeClassifierId = 0xffff;
    ModelSpatialClassifiers classifiers;
    Utf8String              rootUrl;


    if (SUCCESS != RealityMeshAttachmentConversion::ExtractAttachment (rootUrl, location, clipVector, classifiers, activeClassifierId, v8el, converter, v8mm, ScalableMeshElementHandler::XATTRIBUTEID_ScalableMeshAttachment))
        return Result::SkipElement;

    DgnElementId existingId;
    IChangeDetector::SearchResults changeInfo;
    if (converter.GetChangeDetector()._IsElementChanged(changeInfo, converter, v8el, v8mm) && IChangeDetector::ChangeType::Update == changeInfo.m_changeType)
        {
        existingId = changeInfo.GetExistingElementId();
        }

    Utf8String linkName(BeFileName(rootUrl).GetFileNameWithoutExtension());
    DgnDbR db = converter.GetDgnDb();
    RepositoryLinkPtr repositoryLink = RepositoryLink::Create(*db.GetRealityDataSourcesModel(), rootUrl.c_str(), linkName.c_str());
    if (!repositoryLink.IsValid() || !repositoryLink->Insert().IsValid())
        return Result::SkipElement;

    // In DgnV8, ThreeMx attachments are elements, and their visibility is determined by their level. In DgnDb they are DgnModels. 
    // For every spatial view in the DgnDb, determine whether the category of the element's original level is on, and if so
    // add the new ThreeMxModel to the list of viewed models.
    Utf8String modelName;
    BeFileName smFileName; 

    WString rootUrlW(rootUrl.c_str(), true);
  
  	//Avoid slash in URL being converted to backslash  
    smFileName.AppendString(rootUrlW.c_str());

    IMeshSpatialModelP spatialModel = nullptr;
    if (!existingId.IsValid())
        spatialModel = (ScalableMeshModelHandler::AttachTerrainModel(db, modelName, smFileName, *repositoryLink, true, clipVector.get(), &classifiers));
    else
        {
        spatialModel = (db.Models().Get<ScalableMeshModel>(DgnModelId(existingId.GetValue()))).get();
        ClipVectorCP clip = clipVector.get();
        ((ScalableMeshModel*) spatialModel)->SetClip(clipVector.get());
        ((ScalableMeshModel*) spatialModel)->SetClassifiers(classifiers);

        spatialModel->Update();
        }
           
    Bentley::bmap<uint64_t, Bentley::bpair<DGNV8_BENTLEY_NAMESPACE_NAME::ScalableMesh::SMNonDestructiveClipType, Bentley::bvector<Bentley::DPoint3d>>> clipDefs;

    Bentley::BentleyStatus clipInfoStatus(ExtractClipDefinitionsInfo(&clipDefs, v8el));
    
    bmap <uint64_t, SMModelClipInfo> clipInfoMap;

    for (auto& clipDefIter : clipDefs)
        {    
        SMModelClipInfo clipInfo;

        clipInfo.m_shape.resize(clipDefIter.second.second.size());
        memcpy(&clipInfo.m_shape[0], &clipDefIter.second.second[0], clipInfo.m_shape.size() * sizeof(DPoint3d));

        switch (clipDefIter.second.first)
            {
            case DGNV8_BENTLEY_NAMESPACE_NAME::ScalableMesh::SMNonDestructiveClipType::Mask : clipInfo.m_type = SMNonDestructiveClipType::Mask; break;
            case DGNV8_BENTLEY_NAMESPACE_NAME::ScalableMesh::SMNonDestructiveClipType::Boundary: clipInfo.m_type = SMNonDestructiveClipType::Boundary; break;
            default: assert(!"Unknown clip type.");            
            }
       
        clipInfoMap.insert(bpair<uint64_t, SMModelClipInfo>(clipDefIter.first, clipInfo));
        }
    
    if (clipInfoMap.size() > 0)
        { 
        ((ScalableMeshModel*)spatialModel)->SetScalableClips(clipInfoMap);
        spatialModel->Update();
        }
    
    Bentley::bvector<Bentley::bpair<uint64_t, uint64_t>> linksToModelsV8;

    Bentley::BentleyStatus terrainLinkStatus = ExtractTerrainLinkInfo(&linksToModelsV8, v8el);    
       
    ResolveLinkedModelId((ScalableMeshModel*)spatialModel, v8el.GetElementId(), linksToModelsV8);
   
    DgnModelId modelId = spatialModel->GetModelId();
    if (!existingId.IsValid())
        {
        DgnCategoryId category = converter.GetSyncInfo().GetCategory(v8el, v8mm);

        for (auto const& entry : ViewDefinition::MakeIterator(db))
            {
            auto viewController = ViewDefinition::LoadViewController(entry.GetId(), db);
            if (!viewController.IsValid() || !viewController->IsSpatialView() || !viewController->GetViewDefinitionR().GetCategorySelector().IsCategoryViewed(category))
                continue;

            auto& modelSelector = viewController->ToSpatialViewP()->GetSpatialViewDefinition().GetModelSelector();
            modelSelector.AddModel(modelId);
            modelSelector.Update();
            }

        DgnElementId modeledElementId(modelId.GetValue());
        converter.WriteV8ElementExternalSourceAspect(modeledElementId, v8el, v8mm.GetDgnModel().GetModelId()); // NB: last arg (scope) must be the same model as is passed in to _IsElementChanged
        
        converter._GetChangeDetector()._OnElementSeen(converter, modeledElementId);
        }
    if (((ScalableMeshModel*)spatialModel)->_AllowPublishing())
        {
        // Schedule reality model tileset creation.
        converter.AddModelRequiringRealityTiles(modelId, smFileName.GetNameUtf8(), converter.GetRepositoryLinkId(*v8el.GetDgnFileP()));
        }
    else if (!converter.GetDgnDb().GeoLocation().GetEcefLocation().m_isValid)
        {
        // For scalable meshes with in projects with no ECEF we need to record the transform or we have no way to get from tileset (ECEF) to DB.
        Transform   tilesetToDb, dbToTileset = ((ScalableMeshModel*)spatialModel)->GetUorsToStorage();

        tilesetToDb.InverseOf (dbToTileset);
        converter.StoreRealityTilesetTransform(*spatialModel, tilesetToDb);
        spatialModel->Update();
        }

    return Result::SkipElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                  07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvertScalableMeshAttachment::Register()
    {
    ConvertScalableMeshAttachment* instance = new ConvertScalableMeshAttachment();

    DgnV8Api::ElementHandlerId handlerId(ScalableMeshElementHandler::XATTRIBUTEID_ScalableMeshAttachment, 0);
    DgnV8Api::Handler* elHandler = DgnV8Api::ElementHandlerManager::FindHandler(handlerId);

    assert(elHandler != nullptr);
    
    RegisterExtension(*elHandler, *instance);

    RegisterExtension(DgnV8Api::ScalableMeshBaseElementHandler::GetInstance(), *instance);
    }


