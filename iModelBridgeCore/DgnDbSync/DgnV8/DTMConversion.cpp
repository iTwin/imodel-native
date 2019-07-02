/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <ScalableMesh/ScalableMeshDefs.h>
#include <VersionedDgnV8Api/ScalableMesh/ScalableMeshDefs.h>
#include <VersionedDgnV8Api/DgnPlatform/ScalableMeshBaseElementHandler.h>

#undef __bcDTMClassH__
#include <VersionedDgnV8Api/TerrainModel/terrainmodel.h>
#define DGNV8_DTMStatusInt ::DTMStatusInt
#include <VersionedDgnV8Api/TerrainModel/Core/idtm.h>
#include <VersionedDgnV8Api/TerrainModel/Core/bcdtmClass.h>
#include <VersionedDgnV8Api/TerrainModel/ElementHandler/DTMElementHandlerManager.h>
#include <VersionedDgnV8Api/TerrainModel/ElementHandler/DTMDataRef.h>
#include <VersionedDgnV8Api/TerrainModel/Core/DTMIterators.h>
#include <ScalableMesh\IScalableMeshSourceCreator.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA

//----------------------------------------------------------------------------------------
// @bsimethod                                    Daryl.Holmwood                      9/19
//+---------------+---------------+---------------+---------------+---------------+-------
ConvertToDgnDbElementExtension::Result ConvertDTMElement::DoConvert(DgnV8EhCR v8el, WCharCP url, Converter& converter, ResolvedModelMapping const& v8mm)
    {
    Transform               location = Transform::FromIdentity();
    ClipVectorPtr           clipVector;
    uint32_t                activeClassifierId = 0xffff;
    ModelSpatialClassifiers classifiers;
    Utf8String              rootUrl(url);

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
        spatialModel = (ScalableMeshModelHandler::AttachTerrainModel(db, modelName, smFileName, *repositoryLink, location, true, clipVector.get(), &classifiers));
    else
        {
        spatialModel = (db.Models().Get<ScalableMeshModel>(DgnModelId(existingId.GetValue()))).get();
        ClipVectorCP clip = clipVector.get();
        ((ScalableMeshModel*) spatialModel)->SetClip(clipVector.get());
        ((ScalableMeshModel*) spatialModel)->SetClassifiers(classifiers);

        spatialModel->Update();
        }

#ifdef NOTSUPPORTED
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
#endif
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
    // Schedule reality model tileset creation.
    converter.AddModelRequiringRealityTiles(modelId, smFileName.GetNameUtf8(), converter.GetRepositoryLinkId(*v8el.GetDgnFileP()));

    return Result::SkipElement;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Daryl.Holmwood                      9/19
//+---------------+---------------+---------------+---------------+---------------+-------
ConvertToDgnDbElementExtension::Result ConvertDTMElement::_PreConvertElement(DgnV8EhCR v8el, Converter& converter, ResolvedModelMapping const& v8mm)
    {
    Bentley::RefCountedPtr<Bentley::TerrainModel::Element::DTMDataRef> dataRef;
    if (SUCCESS != Bentley::TerrainModel::Element::DTMElementHandlerManager::GetDTMDataRef(dataRef, v8el))
        return Result::SkipElement;

    Bentley::TerrainModel::DTMPtr dtm;
    auto rootTransform = converter.GetRootTrans();
    Bentley::Transform trsf;
    for(int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            trsf.form3d[i][j] = rootTransform.form3d[i][j];

    dataRef->GetDTMReference(dtm, trsf);

    if (dtm.IsNull())
        return Result::SkipElement;

    auto bcDTM = dtm->GetBcDTM();

    if (nullptr == bcDTM)
        return Result::SkipElement;

//    Bentley::TerrainModel::DTMMeshEnumeratorPtr meshes = Bentley::TerrainModel::DTMMeshEnumerator::Create(*bcDTM);
//    meshes->SetMaxTriangles(bcDTM->GetTrianglesCount());    // This will return 1 mesh, if you want to have a smaller dataset then select the number of points.

    //meshes->SetFence(TerrainModel::DTMFenceParams(DTMFenceType::Block, DTMFenceOption::Overlap, pts, pts.size())); // Use this if you want to get the triangles in a Block. with the 5 points closed shape.

//    for (const auto& polyface : *meshes)
//        {
        // Use the polyface to create the 3d tiles.
//        }
//    return Result::SkipElement;
    DgnPlatformLib::Host::IKnownLocationsAdmin& locationAdmin(DgnPlatformLib::QueryHost()->GetIKnownLocationsAdmin());
    BeFileName tempPath = locationAdmin.GetLocalTempDirectoryBaseName();
    WString file;
    file.Sprintf(L"%ls%ld", BeFileName(v8el.GetDgnFileP()->GetFileName().c_str()).GetFileNameWithoutExtension().c_str(), v8el.GetElementId());
    tempPath.AppendToPath(file.c_str());

    BeFileName smFile = tempPath;
    smFile.AppendExtension(L"3sm");

    bool fileExists = smFile.DoesPathExist();
    if (!fileExists)
        {
        StatusInt status;
        auto scalableMeshCreatorPtr = BentleyM0200::ScalableMesh::IScalableMeshSourceCreator::GetFor(smFile.c_str(), status);

#ifdef USEDTMELEMENT
        ScalableMesh::IDTMDgnTerrainModelSourcePtr sourceP = ScalableMesh::IDTMDgnTerrainModelSource::Create(ScalableMesh::DTMSourceDataType::DTM_SOURCE_DATA_DTM,
            dgnFileMonikerPtr,
            v8el.GetElementId(),
            L"kkk").get();
#else
        BeFileName dtmFile = tempPath;
        dtmFile.AppendExtension(L"bcdtm");
        bcDTM->Save(dtmFile.c_str());
        BentleyM0200::ScalableMesh::IDTMSourcePtr sourceP = ScalableMesh::IDTMLocalFileSource::Create(ScalableMesh::DTMSourceDataType::DTM_SOURCE_DATA_DTM, dtmFile.c_str());
#endif
        scalableMeshCreatorPtr->EditSources().Add(sourceP);
        scalableMeshCreatorPtr->Create();
        scalableMeshCreatorPtr->SaveToFile();
        scalableMeshCreatorPtr = nullptr;
#ifndef USEDTMELEMENT
    BeFileName::BeDeleteFile(dtmFile.c_str());
#endif
        }
    auto ret = DoConvert(v8el, smFile.c_str(), converter, v8mm);

    //BeFileName::BeDeleteFile(smFile.c_str());
    return ret;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Daryl.Holmwood                      9/19
//+---------------+---------------+---------------+---------------+---------------+-------
void ConvertDTMElement::Register()
    {
    ConvertDTMElement* instance = new ConvertDTMElement();
    const int TMElementMajorId = 22764;
    const int ELEMENTHANDLER_DTMELEMENT = 11;

    DgnV8Api::ElementHandlerId handlerId(TMElementMajorId, ELEMENTHANDLER_DTMELEMENT);
    DgnV8Api::Handler* elHandler = DgnV8Api::ElementHandlerManager::FindHandler(handlerId);

    assert(elHandler != nullptr);

    if (elHandler != nullptr)
        {
        RegisterExtension(*elHandler, *instance);
        }

    DgnV8Api::ElementHandlerId cifHandlerId(CifTerrainElementHandler::XATTRIBUTEID_CifTerrainModel, CifTerrainElementHandler::ELEMENTHANDLER_SUBTYPE_DTMENTITY );
    DgnV8Api::Handler* cifElHandler = DgnV8Api::ElementHandlerManager::FindHandler(handlerId);

    assert(cifElHandler != nullptr);

    if (cifElHandler != nullptr)
        {
        RegisterExtension(*cifElHandler, *instance);
        }
    }
