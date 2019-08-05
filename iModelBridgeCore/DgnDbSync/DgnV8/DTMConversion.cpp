/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

#undef __bcDTMClassH__
#include <VersionedDgnV8Api/TerrainModel/terrainmodel.h>
#define DGNV8_DTMStatusInt ::DTMStatusInt
#include <VersionedDgnV8Api/TerrainModel/Core/idtm.h>
#include <VersionedDgnV8Api/TerrainModel/Core/bcdtmClass.h>
#include <VersionedDgnV8Api/TerrainModel/ElementHandler/DTMElementHandlerManager.h>
#include <VersionedDgnV8Api/TerrainModel/ElementHandler/DTMDataRef.h>
#include <VersionedDgnV8Api/TerrainModel/ElementHandler/TMElementDisplayHandler.h>
#include <VersionedDgnV8Api/TerrainModel/Core/DTMIterators.h>

#include <ScalableMesh/ScalableMeshDefs.h>
#include <VersionedDgnV8Api/ScalableMesh/ScalableMeshDefs.h>
#include <VersionedDgnV8Api/DgnPlatform/ScalableMeshBaseElementHandler.h>
#include <ScalableMesh\IScalableMeshSourceCreator.h>


// Environment variables
//   DGNDB_DTMTOTILES = Use new code.
//   DGNDB_DTMFROMELEMREF = Use the element ref to create the 3SM.
//   DGNDB_STATICDTMSIZE = Size of a static DTM any less than this and it is a dynamic one.
USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA

Bentley::BentleyStatus ConvertDTMElementRefTo3SM(DgnV8EhCR v8el, WCharCP smFile);

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
void AddFixedBoundary(Bentley::TerrainModel::BcDTMR dtm)
    {
    auto dtmP = dtm.GetTinHandle();
    bcdtmList_copyHptrListToTptrListDtmObject (dtmP, dtmP->hullPoint);
    long newDtmFeatureNum;
    bcdtmInsert_addToFeatureTableDtmObject(dtmP, nullptr, 0, DTMFeatureType::Hull, 0, DTM_NULL_FEATURE_ID, dtmP->hullPoint, &newDtmFeatureNum);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Daryl.Holmwood                      9/19
//+---------------+---------------+---------------+---------------+---------------+-------
bool IsStaticTerrain(DgnV8EhCR v8el, Converter& converter, Bentley::TerrainModel::BcDTMR dtm)
    {
    Bentley::WString cfgValue;
    long  staticTerrainPointLimit = 500000;
    if (BSISUCCESS == DgnV8Api::ConfigurationManager::GetVariable(cfgValue, L"DGNDB_STATICDTMSIZE"))
        {
        int intValue;
        if (1 == ::swscanf(cfgValue.c_str(), L"%d", &intValue))
            staticTerrainPointLimit = intValue;
        }
    return (dtm.GetPointCount() > staticTerrainPointLimit);
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
    for(int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
            trsf.form3d[i][j] = rootTransform.form3d[i][j];

    dataRef->GetDTMReference(dtm, trsf);

    if (dtm.IsNull())
        return Result::SkipElement;

    Bentley::TerrainModel::BcDTMPtr bcDTM = dtm->GetBcDTM();

    if (!bcDTM.IsValid())
        return Result::SkipElement;

    if (!IsStaticTerrain(v8el, converter, *bcDTM))
        return Result::Proceed;

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
        converter.GetProgressMeter().SetCurrentStepName("Converting TM to STM");
#ifdef FROMELEMREF
        if (!DgnV8Api::ConfigurationManager::IsVariableDefinedAndTrue(L"DGNDB_DTMFROMELEMREF"))
            ConvertDTMElementRefTo3SM(v8el, smFile.c_str());
        else
#endif
            {

            StatusInt status;
            auto scalableMeshCreatorPtr = BentleyM0200::ScalableMesh::IScalableMeshSourceCreator::GetFor(smFile.c_str(), status);

            BeFileName dtmFile = tempPath;
            dtmFile.AppendExtension(L"bcdtm");
            Bentley::TerrainModel::DTMFeatureStatisticsInfo info;
            bcDTM->CalculateFeatureStatistics(info);

            // If there is no fixed boundary create one.
            if (!info.hasHull)
                {
                bcDTM = bcDTM->Clone();
                AddFixedBoundary(*bcDTM);
                }

            bcDTM->Save(dtmFile.c_str());
            BentleyM0200::ScalableMesh::IDTMSourcePtr sourceP = ScalableMesh::IDTMLocalFileSource::Create(ScalableMesh::DTMSourceDataType::DTM_SOURCE_DATA_DTM, dtmFile.c_str());

            scalableMeshCreatorPtr->EditSources().Add(sourceP);
            scalableMeshCreatorPtr->Create();
            scalableMeshCreatorPtr->SaveToFile();
            scalableMeshCreatorPtr = nullptr;
            BeFileName::BeDeleteFile(dtmFile.c_str());
            }
        }
    auto ret = DoConvert(v8el, smFile.c_str(), converter, v8mm);

    //BeFileName::BeDeleteFile(smFile.c_str());
    return  Result::Proceed; // ret;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Daryl.Holmwood                      9/19
//+---------------+---------------+---------------+---------------+---------------+-------
bool ConvertDTMElement::_UseProxyGraphics(DgnV8EhCR, Converter&, ResolvedModelMapping const&)
    {
    return false;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Daryl.Holmwood                      9/19
//+---------------+---------------+---------------+---------------+---------------+-------
void SetSymbology(GeometryBuilderR builder, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, Converter& converter, Bentley::DgnPlatform::DTMElementSubHandler::SymbologyParams& displayParams, DgnCategoryId categoryId, DgnSubCategoryId subCategoryId)
    {
    Bentley::DgnPlatform::ElemDisplayParams elParams;
    
    auto dHandler = v8eh.GetDisplayHandler ();

    if (nullptr == dHandler)
        elParams.Init ();
    else
        dHandler->GetElemDisplayParams (v8eh, elParams);

    MSElementCR elm = *v8eh.GetElementCP ();/*drawingInfo.GetOriginalElement()*/

    elParams.SetLevel ((displayParams.GetLevelId() != DgnV8Api::LEVEL_BYCELL) ? displayParams.GetLevelId() : elm.ehdr.level);
    elParams.SetLineColor ((displayParams.GetSymbology().color != DgnV8Api::COLOR_BYCELL) ? displayParams.GetSymbology().color : elm.hdr.dhdr.symb.color);
    elParams.SetWeight ((displayParams.GetSymbology().weight != DgnV8Api::WEIGHT_BYCELL) ? displayParams.GetSymbology().weight : elm.hdr.dhdr.symb.weight);
    elParams.SetLineStyle ((displayParams.GetSymbology().style != DgnV8Api::STYLE_BYCELL) ? displayParams.GetSymbology().style : elm.hdr.dhdr.symb.style);
    elParams.SetTransparency (1 - ((1 - displayParams.GetTransparency ()) * (1 - displayParams.GetTransparency())));
    elParams.SetMaterial (nullptr);

    auto model = v8eh/*drawingInfo.GetSymbologyElement()*/.GetModelRef();

    GeometryParams params = builder.GetGeometryParams();
    Bentley::DgnPlatform::NullOutput          output;
    Bentley::DgnPlatform::NullContext         context (&output);
    auto vi = Bentley::DgnPlatform::ViewInfo::Create(false);
    vi->SetRootModel(model->AsDgnModelP());
    Bentley::DgnPlatform::NonVisibleViewport viewport(*vi);
    context.Attach (&viewport, Bentley::DgnPlatform::DrawPurpose::NotSpecified);
    context.PushModelRef (model, true);
    elParams.Resolve(context);

    //context.SetPathRoot (hitPath.GetRoot());
//    converter.InitGeometryParams(params, elParams, context, true, *model->AsDgnModelCP());
//    params.ResetAppearance();
    params.SetCategoryId(categoryId);
    params.SetSubCategoryId(subCategoryId);
    params.SetLineColor(BentleyApi::Dgn::ColorDef(elParams.GetLineColorTBGR()));
    builder.Append(params);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Daryl.Holmwood                      9/19
//+---------------+---------------+---------------+---------------+---------------+-------
void CreateMesh(Bentley::TerrainModel::BcDTMR dtm, ElementConversionResults& results, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, Converter& converter, BentleyM0200::Dgn::DgnModelR model, DgnCategoryId categoryId, DgnSubCategoryId subCategoryId)
    {
    Bentley::DRange3d fullRange;
    const int s_tilePointSize = 10000;
    const bool useOneBuilder = true;
    const int iterMaxTriangles = 100000;
    int m_numTilesX;
    int m_numTilesY;
    Int64 numberOfPoints = dtm.GetPointCount();
    Bentley::DgnPlatform::DTMElementTrianglesHandler::DisplayParams displayParams(v8eh);
    DgnClassId childClassId = converter.ComputeElementClassIgnoringEcContent(v8eh, v8mm);
    DgnCode elementCode;

    BentleyM0200::Transform trsf;
    dtm.GetTransformation((Bentley::TransformR)trsf);
    dtm.GetRange(fullRange);
    if (numberOfPoints > s_tilePointSize)
        {
        BeAssert(numberOfPoints <= INT_MAX);
        double numOfTiles = (int)numberOfPoints / s_tilePointSize;
        int iNumOfTiles = (int)(sqrt(numOfTiles) + 0.5);

        m_numTilesX = iNumOfTiles;
        m_numTilesY = iNumOfTiles;
        }
    else
        {
        m_numTilesX = 1;
        m_numTilesY = 1;
        }

    double gapX = fullRange.high.x - fullRange.low.x;
    double gapY = fullRange.high.y - fullRange.low.y;
    gapX /= m_numTilesX;
    gapY /= m_numTilesY;
    double x = fullRange.low.x;
    DRange3d tileRange;
    tileRange.low.z = 0;
    tileRange.high.z = 0;
    GeometryBuilderPtr builder;

    if (useOneBuilder)
        {
        builder = GeometryBuilder::Create(model, categoryId, trsf);
        builder->SetAppendAsSubGraphics();
        SetSymbology(*builder, v8eh, v8mm, converter, displayParams, categoryId, subCategoryId);
        }
    for (int dx = 0; dx < m_numTilesX; dx++)
        {
        tileRange.low.x = x;
        tileRange.high.x = x + gapX;
        double y = fullRange.low.y;
        for (int dy = 0; dy < m_numTilesY; dy++)
            {
            tileRange.low.y = y;
            tileRange.high.y = y + gapY;
            DRange3d expandedTileRange = tileRange;

            DPoint3d fencePts[5];
            fencePts[0].x = tileRange.low.x; fencePts[0].y = tileRange.low.y;
            fencePts[1].x = tileRange.high.x; fencePts[1].y = tileRange.low.y;
            fencePts[2].x = tileRange.high.x; fencePts[2].y = tileRange.high.y;
            fencePts[3].x = tileRange.low.x; fencePts[3].y = tileRange.high.y;
            fencePts[4] = fencePts[0];

            Bentley::TerrainModel::DTMFenceParams fence(DGNV8_DTMFenceType::Block, DGNV8_DTMFenceOption::Overlap, (DPoint3d*)fencePts, 5);
            Bentley::TerrainModel::DTMMeshEnumeratorPtr en = Bentley::TerrainModel::DTMMeshEnumerator::Create(dtm);
            en->SetFence(fence);
            en->SetMaxTriangles(iterMaxTriangles);
            en->SetTilingMode(true);

            for (const auto& polyface : *en)
                {
                BentleyApi::PolyfaceHeaderPtr clone;
                
                Converter::ConvertPolyface(clone, *polyface);
                auto geometry = GeometricPrimitive::Create(clone);
                if (!useOneBuilder && builder.IsNull())
                    {
                    builder = GeometryBuilder::Create(model, categoryId, trsf);
                    builder->SetAppendAsSubGraphics();
                    SetSymbology(*builder, v8eh, v8mm, converter, displayParams, categoryId, subCategoryId);
                    }
                builder->Append(*geometry);
                }

            // Use the polyface to create the 3d tiles.
            if (!useOneBuilder && builder.IsValid())
                {
                auto gel = Converter::CreateNewElement(model, childClassId, categoryId, elementCode, "Mesh");

                if (gel.IsValid() && SUCCESS == builder->Finish(*gel->ToGeometrySourceP()))
                    {
                    ElementConversionResults resultsForChild;
                    resultsForChild.m_element = gel.get();
                    results.m_childElements.push_back(resultsForChild);
                    }
                builder = nullptr;
                }
            y += gapY;
            }
        x += gapX;
        }
    if (useOneBuilder && builder.IsValid())
        {
        auto gel = Converter::CreateNewElement(model, childClassId, categoryId, elementCode, "Mesh");

        if (gel.IsValid() && SUCCESS == builder->Finish(*gel->ToGeometrySourceP()))
            {
            ElementConversionResults resultsForChild;
            resultsForChild.m_element = gel.get();
            results.m_childElements.push_back(resultsForChild);
            }
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Daryl.Holmwood                      9/19
//+---------------+---------------+---------------+---------------+---------------+-------
void CreateFeatures(Bentley::DgnPlatform::DTMElementSubHandler::SymbologyParams& displayParams, DGNV8_DTMFeatureType featureType, Utf8CP subCategoryName, Bentley::TerrainModel::BcDTMR dtm, ElementConversionResults& results, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, Converter& converter, BentleyM0200::Dgn::DgnModelR model, DgnCategoryId categoryId)
    {
    if (featureType == DGNV8_DTMFeatureType::Hull)
        return; // This is covered seperately.

    Bentley::TerrainModel::DTMFeatureEnumeratorPtr features = Bentley::TerrainModel::DTMFeatureEnumerator::Create(dtm);

    features->ExcludeAllFeatures();
    features->IncludeFeature(featureType);
    GeometryBuilderPtr builder;

    for (const auto& feature : *features)
        {
        if (builder == nullptr)
            {
            auto subCategoryId = converter.GetOrCreateSubCategoryId(categoryId, subCategoryName);
            auto newCategoryId = DgnSubCategory::QueryCategoryId(converter.GetDgnDb(), subCategoryId);
            builder = GeometryBuilder::Create(model, newCategoryId, BentleyM0200::Transform::FromIdentity());
            builder->SetAppendAsSubGraphics();
            SetSymbology(*builder, v8eh, v8mm, converter, displayParams, categoryId, subCategoryId);
            }
        Bentley::TerrainModel::DTMPointArray points;
        feature.GetFeaturePoints(points);
        if (featureType == DGNV8_DTMFeatureType::GroupSpots)
            {
            auto primitive = BentleyApi::ICurvePrimitive::CreatePointString((BentleyApi::DPoint3dCP)points.data(), points.size());
            builder->Append(*primitive);
            }
        else
            {
            auto curves = BentleyApi::CurveVector::CreateLinear((BentleyApi::DPoint3dCP)points.data(), points.size(), BentleyApi::CurveVector::BOUNDARY_TYPE_Open);
            builder->Append(*curves);
            }
        }

    if (builder != nullptr)
        {
        DgnClassId childClassId = converter.ComputeElementClassIgnoringEcContent(v8eh, v8mm);
        DgnCode elementCode;
        auto gel = Converter::CreateNewElement(model, childClassId, categoryId, elementCode, subCategoryName);

        if (gel.IsValid() && SUCCESS == builder->Finish(*gel->ToGeometrySourceP()))
            {
            ElementConversionResults resultsForChild;
            resultsForChild.m_element = gel.get();
            results.m_childElements.push_back(resultsForChild);
            }
        builder = nullptr;
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Daryl.Holmwood                      9/19
//+---------------+---------------+---------------+---------------+---------------+-------
void ConvertDTMElement::_ProcessResults(ElementConversionResults& results, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, Converter& converter)
    {
    bool hasPrimaryInstance = false; // ecContent.m_primaryV8Instance != nullptr;
    DgnCategoryId categoryId;
    DgnClassId elementClassId;
    DgnCode elementCode;

    Bentley::RefCountedPtr<Bentley::TerrainModel::Element::DTMDataRef> dataRef;
    if (SUCCESS != Bentley::TerrainModel::Element::DTMElementHandlerManager::GetDTMDataRef(dataRef, v8eh))
        return;

    Bentley::TerrainModel::DTMPtr dtm;
    auto rootTransform = converter.GetRootTrans();
    Bentley::Transform trsf;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
            trsf.form3d[i][j] = rootTransform.form3d[i][j];

    dataRef->GetDTMReference(dtm, trsf);

    Bentley::WString name;
    Bentley::TerrainModel::Element::DTMElementHandlerManager::GetName (v8eh, name);

    results.m_element->SetUserLabel(Utf8String(name.c_str()).c_str());
    auto source = results.m_element->ToGeometrySourceP();
    if (nullptr == source)
        return;

    if (nullptr != source)
        categoryId = source->GetCategoryId();

    auto model = results.m_element->GetModel();

    if (model.IsNull())
        return;

    DgnElementPtr   gel;

    if (false) // CreateTerrain Category
        {
        SpatialCategory category(*converter.GetJobDefinitionModel(), "Terrain");
        category.Insert(DgnSubCategory::Appearance());
        categoryId = category.GetCategoryId();
        source->SetCategoryId(categoryId);
        }

    DgnSubCategoryId subCategoryId = converter.GetOrCreateSubCategoryId(categoryId, "Boundary");
    auto newCategoryId = DgnSubCategory::QueryCategoryId(converter.GetDgnDb(), subCategoryId);

    GeometryBuilderPtr builder;

    builder = GeometryBuilder::Create(*model, categoryId, Transform::FromIdentity());
    builder->SetAppendAsSubGraphics();

    Bentley::TerrainModel::DTMPointArray boundaryPts;
    dtm->GetBoundary(boundaryPts);

    Bentley::DgnPlatform::DTMElementFeaturesHandler::DisplayParams boundaryDisplayParams(v8eh);

    if (Bentley::DgnPlatform::DTMElementFeaturesHandler::GetSubElement(v8eh, Bentley::DgnPlatform::DTMElementFeaturesHandler::Boundary, boundaryDisplayParams))
        SetSymbology(*builder, v8eh, v8mm, converter, boundaryDisplayParams, newCategoryId, subCategoryId);

    builder->Append(*CurveVector::CreateLinear((DPoint3dCP)boundaryPts.data(), boundaryPts.size(), CurveVector::BOUNDARY_TYPE_Open));

    DgnClassId      childClassId = converter.ComputeElementClassIgnoringEcContent(v8eh, v8mm);
    gel = Converter::CreateNewElement(*model, childClassId, categoryId, elementCode, "Boundary");

    if (gel.IsValid() && SUCCESS == builder->Finish(*gel->ToGeometrySourceP()))
        {
        ElementConversionResults resultsForChild;
        resultsForChild.m_element = gel.get();
        results.m_childElements.push_back(resultsForChild);
        }
    builder = nullptr;

    auto bcDTM = dtm->GetBcDTM();
    if (IsStaticTerrain(v8eh, converter, *bcDTM))
        return;

    // ToDo output the Mesh.
    subCategoryId = converter.GetOrCreateSubCategoryId(categoryId, "Mesh");
    newCategoryId = DgnSubCategory::QueryCategoryId(converter.GetDgnDb(), subCategoryId);

    CreateMesh(*bcDTM, results, v8eh, v8mm, converter, *model, newCategoryId, subCategoryId);

    Bentley::DgnPlatform::DTMSubElementIter iter(v8eh);

    for (; iter.IsValid(); iter.ToNext())
        {
        auto hand = Bentley::DgnPlatform::DTMElementSubHandler::FindHandler (iter);

        if (Bentley::DgnPlatform::DTMElementFeaturesHandler::GetInstance()->GetSubHandlerId() == iter.GetCurrentId().GetHandlerId())
            {
            Bentley::DgnPlatform::DTMElementFeaturesHandler::DisplayParams dp(iter);
            DGNV8_DTMFeatureType featureTypeMap[] = {
                DGNV8_DTMFeatureType::Breakline,
                DGNV8_DTMFeatureType::Hole,
                DGNV8_DTMFeatureType::Island,
                DGNV8_DTMFeatureType::Void,
                DGNV8_DTMFeatureType::Hull,
                DGNV8_DTMFeatureType::Contour,
                DGNV8_DTMFeatureType::FeatureSpot,
                };

            Utf8CP subCategoryNames[] =
                {
                "Breakline",
                "Hole",
                "Island",
                "Void",
                "Hull",
                "Contour",
                "FeatureSpot",
                };

            CreateFeatures(dp, featureTypeMap[dp.GetTag()], subCategoryNames[dp.GetTag()], *bcDTM, results, v8eh, v8mm, converter, *model, categoryId);
            }
        else if (Bentley::DgnPlatform::DTMElementFeatureSpotsHandler::GetInstance()->GetSubHandlerId() == iter.GetCurrentId().GetHandlerId())
            {
            Bentley::DgnPlatform::DTMElementFeatureSpotsHandler::DisplayParams dp(iter);
            CreateFeatures(dp, DGNV8_DTMFeatureType::GroupSpots, "FeatureSpots", *bcDTM, results, v8eh, v8mm, converter, *model, categoryId);
            }
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Daryl.Holmwood                      9/19
//+---------------+---------------+---------------+---------------+---------------+-------
void ConvertDTMElement::Register()
    {
    if (!DgnV8Api::ConfigurationManager::IsVariableDefinedAndTrue(L"DGNDB_DTMTOTILES"))
        return;

    ConvertDTMElement* instance = new ConvertDTMElement();
    const int TMElementMajorId = 22764;
    const int ELEMENTHANDLER_DTMELEMENT = 11;

    DgnV8Api::ElementHandlerId handlerId(TMElementMajorId, ELEMENTHANDLER_DTMELEMENT);
    DgnV8Api::Handler* elHandler = DgnV8Api::ElementHandlerManager::FindHandler(handlerId);

    assert(elHandler != nullptr);

    if (elHandler != nullptr)
        RegisterExtension(*elHandler, *instance);

    DgnV8Api::ElementHandlerId cifHandlerId(CifTerrainElementHandler::XATTRIBUTEID_CifTerrainModel, CifTerrainElementHandler::ELEMENTHANDLER_SUBTYPE_DTMENTITY );
    DgnV8Api::Handler* cifElHandler = DgnV8Api::ElementHandlerManager::FindHandler(cifHandlerId);

    assert(cifElHandler != nullptr);

    if (cifElHandler != nullptr)
        RegisterExtension(*cifElHandler, *instance);
    }
