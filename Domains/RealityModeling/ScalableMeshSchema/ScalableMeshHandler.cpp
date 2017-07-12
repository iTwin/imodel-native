/*-------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshSchema/ScalableMeshHandler.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ScalableMeshSchemaPCH.h"

#include <BeSQLite\BeSQLite.h>
#include <ScalableMeshSchema\ScalableMeshHandler.h>
#include "ScalableMeshDisplayCacheManager.h"
#include <ScalableMesh\GeoCoords\GCS.h>
#include <DgnPlatform\LinkElement.h>



USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA
USING_NAMESPACE_BENTLEY_RENDER

//#define PRINT_SMDISPLAY_MSG

/*
#ifndef NDEBUG
#define PRINT_TERRAIN_MSG 1
#endif
*/

#ifdef PRINT_SMDISPLAY_MSG
#define PRINT_MSG_IF(condition, ...) if(##condition##) printf(__VA_ARGS__);
#define PRINT_MSG(...) printf(__VA_ARGS__);
#else
#define PRINT_MSG_IF(condition, ...)
#define PRINT_MSG(...) 
#endif

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
AxisAlignedBox3d ScalableMeshModel::_GetRange() const
    {
    if (m_smPtr.IsValid()) m_smPtr->GetRange(const_cast<AxisAlignedBox3d&>(m_range));
    return m_range;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_QueryTexturesLod(bvector<ITerrainTexturePtr>& textures, size_t maxSizeBytes) const
    {
    return ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_QueryTexture(ITextureTileId const& tileId, ITerrainTexturePtr& texture) const
    {
    return ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_ReloadClipMask(const BentleyApi::Dgn::DgnElementId& clipMaskElementId, bool isNew)
    {
    bvector<uint64_t> clipIds;
    clipIds.push_back(clipMaskElementId.GetValue());
    GetProgressiveQueryEngine()->ClearCaching(clipIds, m_smPtr);
    m_forceRedraw = true;
    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_ReloadAllClipMasks()
    {
    return ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_StartClipMaskBulkInsert()
    {
    if (nullptr == m_smPtr.get()) return ERROR;
    m_isInsertingClips = true;
    m_smPtr->SetIsInsertingClips(true);
    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_StopClipMaskBulkInsert()
    {
    if (nullptr == m_smPtr.get()) return ERROR;
    m_isInsertingClips = false;
    m_smPtr->SetIsInsertingClips(false);
    bvector<uint64_t> currentlyShown;
    bset<uint64_t> ids;
    GetClipSetIds(currentlyShown);
    for (auto elem : currentlyShown)
        ids.insert(elem);
    SetActiveClipSets(ids, ids);
    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_CreateIterator(ITerrainTileIteratorPtr& iterator)
    {
    return ERROR;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
TerrainModel::IDTM* ScalableMeshModel::_GetDTM(ScalableMesh::DTMAnalysisType type)
    {
    if (nullptr == m_smPtr.get()) return nullptr;
    return m_smPtr->GetDTMInterface(m_storageToUorsTransfo, type);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::_RegisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener)
    {

    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
bool ScalableMeshModel::_UnregisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener)
    {
    return false;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
#define QUERY_ID 0 

static double s_minScreenPixelsPerPoint = 1000;

bool IsWireframeRendering(ViewContextCR viewContext)
    {    

    // Check context render mode
    switch (viewContext.GetViewFlags().GetRenderMode())
        {        
        case Render::RenderMode::SmoothShade:        
            return false;
                       
        case Render::RenderMode::Wireframe:        
        case Render::RenderMode::HiddenLine:
        case Render::RenderMode::SolidFill:        
            return true;
        }
        BeAssert(!"Unknown render mode");
        return true;
    }


static Byte s_transparency = 100;

void ProgressiveDrawMeshNode2(bvector<IScalableMeshCachedDisplayNodePtr>& meshNodes,
                              bvector<IScalableMeshCachedDisplayNodePtr>& overviewMeshNodes,                              
                              Dgn::RenderContextR                         context,
                              const DMatrix4d&                            storageToUors,
                              ScalableMeshDisplayCacheManager*            mgr)
    {    
#if 0 //NEEDS_WORK_SM_TEMP_OUT

#ifdef PRINT_SMDISPLAY_MSG
    PRINT_MSG("ProgressiveDrawMeshNode2 meshNode : %I64u overviewMeshNode : %I64u \n", meshNodes.size(), overviewMeshNodes.size());
#endif

    static size_t s_callCount = 0;
                      
    bvector<IScalableMeshCachedDisplayNodePtr> requestedNodes;
    bvector<IScalableMeshCachedDisplayNodePtr> nodesWithoutQvElem;

    Render::GraphicBranch graphics; 
    
    if (overviewMeshNodes.size() > 0)
        {
        //NEEDS_WORK_SM : If kept needs clean up
        for (size_t nodeInd = 0; nodeInd < overviewMeshNodes.size(); nodeInd++)
            {                       
            /*
            if (context.CheckStop())
                break;                           
                */
            
            
            //NEEDS_WORK_SM_PROGRESSIVE : IsMeshLoaded trigger load header.
            //assert(overviewMeshNodes[nodeInd]->IsHeaderLoaded() && overviewMeshNodes[nodeInd]->IsMeshLoaded());
            /*
            if (!meshNodes[nodeInd]->IsHeaderLoaded() || !meshNodes[nodeInd]->IsMeshLoaded())
                requestedNodes.push_back(meshNodes[nodeInd]);
            else
            */            
            
                {
                /*
                ElemMatSymbP matSymbP = context.GetElemMatSymb();

                matSymbP->Init();
                ColorDef white(0xff, 0xff, 0xff);
                ColorDef green(0, 0x77, 0);
                matSymbP->SetLineColor(overviewMeshNodes[nodeInd]->IsTextured() ? white : green);
                matSymbP->SetFillColor(overviewMeshNodes[nodeInd]->IsTextured() ? white : green);                
                matSymbP->SetFillTransparency(s_transparency);
                matSymbP->SetLineTransparency(s_transparency);

                matSymbP->SetIsFilled(false);
                context.ResetContextOverrides(); // If not reset, last drawn override is applyed to dtm (Selected/Hide preview)
                context.GetIDrawGeom().ActivateMatSymb(matSymbP);
                */
                }

            
            SmCachedDisplayMesh* cachedMesh = 0;                        

            if (SUCCESS == overviewMeshNodes[nodeInd]->GetCachedMesh(cachedMesh))
                {                
                graphics.Add(*cachedMesh->m_graphic);
                }
            else
                {                    
                /*NEEDS_WORK_SM : Not support yet.
                __int64 meshId = GetMeshId(overviewMeshNodes[nodeInd]->GetNodeId());

                qvElem = QvCachedNodeManager::GetManager().FindQvElem(meshId, dtmDataRef.get());                            
                */
                assert(!"Should not get here");
                }
            }
        }


     if (meshNodes.size() > 0)
        {
        //NEEDS_WORK_SM : If kept needs clean up
        for (size_t nodeInd = 0; nodeInd < meshNodes.size(); nodeInd++)
            {                      
            /*
            if (context.CheckStop())
                break;            
                */
            
            //NEEDS_WORK_SM_PROGRESSIVE : IsMeshLoaded trigger load header.
            //assert(meshNodes[nodeInd]->IsHeaderLoaded() && meshNodes[nodeInd]->IsMeshLoaded());
            /*
            if (!meshNodes[nodeInd]->IsHeaderLoaded() || !meshNodes[nodeInd]->IsMeshLoaded())
                requestedNodes.push_back(meshNodes[nodeInd]);
            else
            */            
          
            SmCachedDisplayMesh* cachedMesh = 0;
                        
            if (SUCCESS == meshNodes[nodeInd]->GetCachedMesh(cachedMesh))
                {
                graphics.Add(*cachedMesh->m_graphic);
                }
            else
                {
                assert(!"Should not occur");
                /*NEEDS_WORK_SM : Not support yet.
                __int64 meshId = GetMeshId(meshNodes[nodeInd]->GetNodeId());
                qvElem = QvCachedNodeManager::GetManager().FindQvElem(meshId, dtmDataRef.get());                            
                */
                }                                                     
            }    
        }

    if (graphics.m_entries.empty())
        return;
        
    Transform storageToUorsTransform;
    storageToUorsTransform.InitFrom(storageToUors);
    //context.PushTransform(storageToUorsTransform);

    auto group = context.CreateBranch(Render::Graphic::CreateParams(nullptr, storageToUorsTransform), graphics);    
    context.OutputGraphic(*group, nullptr);    

#endif
    }


//========================================================================================
// @bsiclass                                                        Mathieu.St-Pierre     02/2016
//========================================================================================
static bool s_drawInProcess = true;

struct ScalableMeshProgressiveTask : ProgressiveTask
    {
    
protected:
        
    IScalableMeshProgressiveQueryEnginePtr  m_progressiveQueryEngine;        
    ScalableMeshDrawingInfoPtr              m_currentDrawingInfoPtr;
    const DMatrix4d&                        m_storageToUorsTransfo;    
    IScalableMeshDisplayCacheManager*       m_displayNodesCache;
    uint64_t                                m_nextShow;

    ScalableMeshProgressiveTask (IScalableMeshProgressiveQueryEnginePtr& progressiveQueryEngine,
                                 ScalableMeshDrawingInfoPtr&             currentDrawingInfoPtr, 
                                    DMatrix4d&                              storageToUorsTransfo,
                                     IScalableMeshDisplayCacheManagerPtr& cacheManager)
    : m_storageToUorsTransfo(storageToUorsTransfo)
        {    
        m_progressiveQueryEngine = progressiveQueryEngine;
        m_currentDrawingInfoPtr = currentDrawingInfoPtr;        
        m_nextShow = 0;
        m_displayNodesCache = cacheManager.get();
        }

public:

    virtual bool _WantTimeoutSet(uint32_t& limit)   {return false; }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Mathieu.St-Pierre     02/2016
//----------------------------------------------------------------------------------------
ProgressiveTask::Completion _DoProgressive(RenderListContext& context, WantShow& wantShow)
    {   
#if 0 //NEEDS_WORK_SM_TEMP_OUT
    uint64_t now = BeTimeUtilities::QueryMillisecondsCounter();

    if (m_currentDrawingInfoPtr->m_overviewNodes.size() > 0)
        {                    
        int queryId = m_currentDrawingInfoPtr->m_currentQuery;

        if (m_progressiveQueryEngine->IsQueryComplete(queryId))
            {
            m_currentDrawingInfoPtr->m_meshNodes.clear();
            StatusInt status = m_progressiveQueryEngine->GetRequiredNodes(m_currentDrawingInfoPtr->m_meshNodes, queryId);

            assert(m_currentDrawingInfoPtr->m_overviewNodes.size() == 0 || m_currentDrawingInfoPtr->m_meshNodes.size() > 0);

            m_currentDrawingInfoPtr->m_overviewNodes.clear();

            assert(status == SUCCESS);

            //completionStatus = Completion::HealRequired; 

#ifdef PRINT_SMDISPLAY_MSG        
            PRINT_MSG("Heal required  meshNode : %I64u overviewMeshNode : %I64u \n", m_currentDrawingInfoPtr->m_meshNodes.size(), m_currentDrawingInfoPtr->m_overviewNodes.size());       
#endif
            }
        else
            {                                       
            m_currentDrawingInfoPtr->m_meshNodes.clear();
            StatusInt status = m_progressiveQueryEngine->GetRequiredNodes(m_currentDrawingInfoPtr->m_meshNodes, queryId);
            assert(status == SUCCESS);                                  

            m_currentDrawingInfoPtr->m_overviewNodes.clear();
            status = m_progressiveQueryEngine->GetOverviewNodes(m_currentDrawingInfoPtr->m_overviewNodes, queryId);
            assert(status == SUCCESS);                                  
                                    
            if (s_drawInProcess)
                ProgressiveDrawMeshNode2(m_currentDrawingInfoPtr->m_meshNodes, m_currentDrawingInfoPtr->m_overviewNodes, context, m_storageToUorsTransfo);                                                      
            }
        }
    else
        {
        context.GetViewport()->SetNeedsHeal(); // unfortunately the newly drawn tiles may be obscured by lower resolution ones
        return Completion::Finished;
        }     
    
    if (now > m_nextShow)
        {
        m_nextShow = now + 1000; // once per second
        wantShow = WantShow::Yes;
        }
           
#endif
    return Completion::Aborted;    
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                               Mathieu.St-Pierre      02/2016
//----------------------------------------------------------------------------------------
static void Schedule (IScalableMeshProgressiveQueryEnginePtr& progressiveQueryEngine,
                      ScalableMeshDrawingInfoPtr&             currentDrawingInfoPtr, 
                      DMatrix4d&                              storageToUorsTransfo, 
                      TerrainContextR                            context,
                      IScalableMeshDisplayCacheManagerPtr& cacheManager)
    {
/*
    context.GetViewport()->ScheduleTerrainProgressiveTask (*new ScalableMeshProgressiveTask(progressiveQueryEngine,
                                                                                            currentDrawingInfoPtr,
                                                                                                        storageToUorsTransfo,
                                                                                                        cacheManager));
*/
    }

};  

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
bool ShouldDrawInContext (ViewContextR context) 
    {
    /*
    switch (context.GetDrawPurpose())
        {
        case DrawPurpose::Hilite:
        case DrawPurpose::Unhilite:
        case DrawPurpose::ChangedPre:       // Erase, rely on Healing.
        case DrawPurpose::RestoredPre:      // Erase, rely on Healing.
        case DrawPurpose::Pick:
        case DrawPurpose::Flash:
        case DrawPurpose::CaptureGeometry:
        case DrawPurpose::FenceAccept:
        case DrawPurpose::RegionFlood:
        case DrawPurpose::FitView:
        case DrawPurpose::ExportVisibleEdges:
        case DrawPurpose::ModelFacet:
            return false;
        }
        */

    return true;
    }

static bool s_loadTexture = true;
static bool s_waitQueryComplete = false;


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  12/2016
//----------------------------------------------------------------------------------------
IScalableMeshProgressiveQueryEnginePtr ScalableMeshModel::GetProgressiveQueryEngine()
    {
    if (m_progressiveQueryEngine == nullptr)
        {
        //m_displayNodesCache = new ScalableMeshDisplayCacheManager(GetDgnDb());
        m_progressiveQueryEngine = IScalableMeshProgressiveQueryEngine::Create(m_smPtr, m_displayNodesCache);

        bvector<uint64_t> allClips;
        bset<uint64_t> clipsToShow;
        bset<uint64_t> clipsShown;
        GetClipSetIds(allClips);
        for (auto elem : allClips)
            clipsToShow.insert(elem);
        SetActiveClipSets(clipsToShow, clipsShown);
        }

    return m_progressiveQueryEngine;
    }

void ScalableMeshModel::_AddTerrainGraphics(TerrainContextR context) const
    {        
#if 0 //NEEDS_WORK_SM_TEMP_OUT
    if (m_smPtr == 0 && !m_tryOpen)
        {
        //BeFileName smFileName(((this)->m_properties).m_fileId);
        BeFileName smFileName;
        T_HOST.GetPointCloudAdmin()._ResolveFileName(smFileName, (((this)->m_properties).m_fileId), GetDgnDb());

        if (BeFileName::DoesPathExist(smFileName.c_str()))
            {
            const_cast<ScalableMeshModel*>(this)->OpenFile(smFileName, GetDgnDb()); 
            }
        
        (static_cast<ScalableMeshDisplayCacheManager*>(m_displayNodesCache.get()))->SetRenderSys(&context.GetTargetR().GetSystem());

        m_tryOpen = true;
        }

    

    if (!ShouldDrawInContext(context) || NULL == context.GetViewport() || !m_smPtr.IsValid())
        return;
         
    ScalableMeshDrawingInfoPtr nextDrawingInfoPtr(new ScalableMeshDrawingInfo(&context));
    nextDrawingInfoPtr->m_currentQuery = (int)((GetModelId().GetValue() - GetModelId().GetBriefcaseId().GetValue()) & 0xFFFF);
    if ((m_currentDrawingInfoPtr != nullptr) &&
        (m_currentDrawingInfoPtr->GetDrawPurpose() != DrawPurpose::Dynamics))
        {
        //If the m_dtmPtr equals 0 it could mean that the last data request to the STM was cancelled, so start a new request even
        //if the view has not changed.
        if (m_currentDrawingInfoPtr->HasAppearanceChanged(nextDrawingInfoPtr) == false && !m_forceRedraw)                
            {
            //assert((m_currentDrawingInfoPtr->m_overviewNodes.size() == 0) && (m_currentDrawingInfoPtr->m_meshNodes.size() > 0));

            ProgressiveDrawMeshNode2(m_currentDrawingInfoPtr->m_meshNodes, m_currentDrawingInfoPtr->m_overviewNodes, context, m_storageToUorsTransfo);                              
            return;                        
            }   
        }        
    BentleyStatus status;

    status = GetProgressiveQueryEngine()->StopQuery(/*nextDrawingInfoPtr->GetViewNumber()*/nextDrawingInfoPtr->m_currentQuery);
    if (!clipFromCoverageSet.empty())
        {
        status = m_progressiveQueryEngine->StopQuery(nextDrawingInfoPtr->m_terrainQuery);
        }
    assert(status == SUCCESS);

    m_forceRedraw = false;                                   
    m_currentDrawingInfoPtr = nextDrawingInfoPtr;

    // Need to get the fence info.
    /*
    DTMDrawingInfo drawingInfo;
    DTMElementDisplayHandler::GetDTMDrawingInfo(drawingInfo, m_DTMDataRef->GetElement(), m_DTMDataRef, context);

    if (!drawingInfo.IsVisible ())
        {
        return;
        }
        */

    DMatrix4d localToView(context.GetViewport()->GetWorldToViewMap()->M0);
                                   
    bsiDMatrix4d_multiply(&localToView, &localToView, &m_storageToUorsTransfo);              

    //DPoint3d viewBox[8];

    //NEEDS_WORK_SM : Remove from query
    //GetViewBoxFromContext(viewBox, _countof(viewBox), context, drawingInfo);        
    DMatrix4d rootToStorage;

    //Convert the view box in storage.
    bool inverted = bsiDMatrix4d_invertQR(&rootToStorage, &m_storageToUorsTransfo);

    BeAssert(inverted != 0);

    /*
    bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray(&rootToStorage, viewBox, viewBox, 8);
    */
    
    status = SUCCESS;
            
    IScalableMeshViewDependentMeshQueryParamsPtr viewDependentQueryParams(IScalableMeshViewDependentMeshQueryParams::CreateParams());

    viewDependentQueryParams->SetMinScreenPixelsPerPoint(s_minScreenPixelsPerPoint);
                
    ClipVectorPtr frustumClipVector;    
    Render::FrustumPlanes frustumPlanes(context.GetFrustumPlanes());

    assert(frustumPlanes.IsValid());

    ClipPlaneSet clipPlaneSet(frustumPlanes.m_planes, 6);    
    ClipVector::AppendPlanes(frustumClipVector, clipPlaneSet);
                
    //NEEDS_WORK_SM : Need to keep only SetViewBox or SetViewClipVector for visibility
    //viewDependentQueryParams->SetViewBox(viewBox);
    viewDependentQueryParams->SetRootToViewMatrix(localToView.coff);    

    //NEEDS_WORK_SM : Needed?
    /*
    if (s_progressiveDraw)
        {
        viewDependentQueryParams->SetProgressiveDisplay(true);
        viewDependentQueryParams->SetStopQueryCallback(CheckStopQueryCallback);
        }            
        */
        
    Transform rootToStorageTransform; 
    bool result = rootToStorageTransform.InitFrom (rootToStorage); 
    assert(result == true);

    frustumClipVector->TransformInPlace(rootToStorageTransform);
    
    viewDependentQueryParams->SetViewClipVector(frustumClipVector);
                          
    m_currentDrawingInfoPtr->m_overviewNodes.clear();
    int queryId = (int)((GetModelId().GetValue() - GetModelId().GetBriefcaseId().GetValue()) & 0xFFFF);//nextDrawingInfoPtr->GetViewNumber();                 
    m_currentDrawingInfoPtr->m_currentQuery = queryId;
    bvector<bool> clips;
    /*NEEDS_WORK_SM : Get clips
    m_DTMDataRef->GetVisibleClips(clips);
    */

    status = GetProgressiveQueryEngine()->StartQuery(queryId,
                                                      viewDependentQueryParams, 
                                                      m_currentDrawingInfoPtr->m_meshNodes, 
                                                  !IsWireframeRendering(context) && s_loadTexture, 
                                                      clips,
                                                  &m_currentDrawingInfoPtr->GetLocalToViewTransform(), 
                                                  &nextDrawingInfoPtr->GetLocalToViewTransform()); 

    assert(status == SUCCESS);


    if (s_waitQueryComplete || !m_isProgressiveDisplayOn)
        {
        while (!GetProgressiveQueryEngine()->IsQueryComplete(queryId))
            {
            BeThreadUtilities::BeSleep (200);
            }
        }


        if (!m_isProgressiveDisplayOn)
            {
            while (!GetProgressiveQueryEngine()->IsQueryComplete(terrainQueryId))
                {
                BeThreadUtilities::BeSleep (200);
                }
            }

    bool needProgressive;
    
    if (GetProgressiveQueryEngine()->IsQueryComplete(queryId))
        {
        m_currentDrawingInfoPtr->m_meshNodes.clear();
        status = GetProgressiveQueryEngine()->GetRequiredNodes(m_currentDrawingInfoPtr->m_meshNodes, queryId);

        bvector<IScalableMeshNodePtr> nodes;
        for (auto& nodeP : m_currentDrawingInfoPtr->m_meshNodes) nodes.push_back(nodeP.get());
        m_smPtr->SetCurrentlyViewedNodes(nodes);
        assert(m_currentDrawingInfoPtr->m_meshNodes.size() > 0);

        m_currentDrawingInfoPtr->m_overviewNodes.clear();
        status = GetProgressiveQueryEngine()->StopQuery(queryId);
        assert(status == SUCCESS);
        needProgressive = false;
        m_forceRedraw = false;
        }
    else
        {  
        bvector<IScalableMeshNodePtr> nodes;
        for (auto& nodeP : m_currentDrawingInfoPtr->m_meshNodes) nodes.push_back(nodeP.get());
        m_smPtr->SetCurrentlyViewedNodes(nodes);
        status = GetProgressiveQueryEngine()->GetOverviewNodes(m_currentDrawingInfoPtr->m_overviewNodes, queryId);
        m_currentDrawingInfoPtr->m_overviewNodes.insert(m_currentDrawingInfoPtr->m_overviewNodes.end(), m_currentDrawingInfoPtr->m_meshNodes.begin(), m_currentDrawingInfoPtr->m_meshNodes.end());
        m_currentDrawingInfoPtr->m_meshNodes.clear();

        //NEEDS_WORK_MST : Will be fixed when the lowest resolution is created and pin at creation time.
        //assert(m_currentDrawingInfoPtr->m_overviewNodes.size() > 0);
        assert(status == SUCCESS);

        needProgressive = true;
        }                         

    ProgressiveDrawMeshNode2(m_currentDrawingInfoPtr->m_meshNodes, m_currentDrawingInfoPtr->m_overviewNodes, context, m_storageToUorsTransfo);                              

    if (needProgressive)
        {
        ScalableMeshProgressiveTask::Schedule(m_progressiveQueryEngine, m_currentDrawingInfoPtr, m_storageToUorsTransfo, context, m_displayNodesCache);
        }
#endif
    }                 


void ScalableMeshModel::GetAllScalableMeshes(BentleyApi::Dgn::DgnDbCR dgnDb, bvector<IMeshSpatialModelP>& models)
    {
    DgnClassId classId(dgnDb.Schemas().GetClassId("ScalableMesh", "ScalableMeshModel"));
    BeAssert(classId.IsValid());

    for (auto& model : dgnDb.Models().GetLoadedModels())
        {
        if (model.second->GetClassId() == classId) models.push_back(dynamic_cast<IMeshSpatialModelP>(model.second.get()));
        }
    }

#if 0 

struct  ScalableMeshTileNode : ModelTileNode
    {
    IScalableMeshNodePtr    m_node;
    Transform               m_transform;

    ScalableMeshTileNode(DgnModelCR model, IScalableMeshNodePtr& node, DRange3d transformedRange, TransformCR transform, size_t siblingIndex, TileNodeP parent) :
        m_node(node), m_transform(transform), ModelTileNode(model, transformedRange, node->GetLevel(), siblingIndex, transformedRange.XLength()* transformedRange.YLength() / node->GetPointCount(), parent)
        {}


    virtual TileMeshList _GenerateMeshes(TileGeometryCacheR geometryCache, double tolerance, TileGeometry::NormalMode normalMode = TileGeometry::NormalMode::CurvedSurfacesOnly, bool twoSidedTriangles = false) const override
        {
        TileMeshList        tileMeshes;

        if (m_node->GetChildrenNodes().empty())
            {
            BeAssert(false);
            return tileMeshes;
            }


        for (auto& child : m_node->GetChildrenNodes())
            {

            IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create(true, false);
            bvector<bool> clips;
            auto meshP = child->GetMesh(flags, clips);
            if (!meshP.IsValid() || meshP->GetNbFaces() == 0) continue;
            TileMeshBuilderPtr      builder;
            TileDisplayParamsPtr    displayParams;

            if (child->IsTextured())
                {
                auto textureP = child->GetTexture();
                ByteStream myImage(textureP->GetDimension().x* textureP->GetDimension().y * 3);
                memcpy(myImage.GetDataP(), textureP->GetData(), textureP->GetDimension().x* textureP->GetDimension().y * 3);
                Image image(textureP->GetDimension().x, textureP->GetDimension().y, std::move(myImage), Image::Format::Rgb);
                ImageSource jpgTex(image, ImageSource::Format::Jpeg, 70);
                TileTextureImagePtr     tileTexture = new TileTextureImage(jpgTex);
                displayParams = new TileDisplayParams(0xffffff, tileTexture, true);
                }
            else
                {
                TileTextureImagePtr     tileTexture = nullptr;
                displayParams = new TileDisplayParams(0x007700, tileTexture, false);
                }
            builder = TileMeshBuilder::Create(displayParams, NULL, 0.0);
            for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*meshP->GetPolyfaceQuery()); visitor->AdvanceToNextFace();)
                builder->AddTriangle(*visitor, GetModel().GetModelId(), false, twoSidedTriangles);

            tileMeshes.push_back(builder->GetMesh());
            }

        return tileMeshes;
        }

    };  //  ScalableMeshTileNode

#endif

void ScalableMeshModel::MakeTileSubTree(TileNodePtr& rootTile, IScalableMeshNodePtr& node, TransformCR transformDbToTile, size_t childIndex, TileNode* parent)
    {
    DRange3d transformedRange = node->GetContentExtent();
    if (transformedRange.IsNull() || transformedRange.IsEmpty()) transformedRange = node->GetNodeExtent();
    transformDbToTile.Multiply(transformedRange, transformedRange);
    //rootTile = new ScalableMeshTileNode(*this, node, transformedRange, transformDbToTile, childIndex, parent);

    for (auto& child : node->GetChildrenNodes())
        {
        size_t idx = &child - &node->GetChildrenNodes().front();
        TileNodePtr childTile;
        MakeTileSubTree(childTile, child, transformDbToTile, idx, rootTile.get());
        rootTile->GetChildren().push_back(childTile);
        }
    }

#if 0 
TileGeneratorStatus ScalableMeshModel::_GenerateMeshTiles(TileNodePtr& rootTile, TransformCR transformDbToTile, double leafTolerance, TileGenerator::ITileCollector& collector, ITileGenerationProgressMonitorR progressMeter) 
    {
    assert(!"Not implemented yet");
    
    return TileGeneratorStatus::Aborted;
    }
#endif

//NEEDS_WORK_SM : Should be at application level
void GetScalableMeshTerrainFileName(BeFileName& smtFileName, const BeFileName& dgnDbFileName)
    {    
    //smtFileName = params.m_dgndb.GetFileName().GetDirectoryName();

    smtFileName = dgnDbFileName.GetDirectoryName();
    smtFileName.AppendToPath(dgnDbFileName.GetFileNameWithoutExtension().c_str());
    smtFileName.AppendString(L"\\terrain.stm");
    }


//=======================================================================================
//! Helper class used to kept pointers with a DgnDb in-memory
//=======================================================================================
struct ScalableMeshTerrainModelAppData : Db::AppData
    {
    static Key DataKey;

    ScalableMeshModel*  m_smTerrainPhysicalModelP;
    bool                m_modelSearched;

    ScalableMeshTerrainModelAppData () 
        {
        m_smTerrainPhysicalModelP = 0;
        m_modelSearched = false;
        }
    virtual ~ScalableMeshTerrainModelAppData () {}

     ScalableMeshModel* GetModel(DgnDbCR db)
        {
        if (m_modelSearched == false)
            {
            //NEEDS_WORK_SM : Not yet done. 
            /*
            BeSQLite::EC::ECSqlStatement stmt;
            if (BeSQLite::EC::ECSqlStatus::Success != stmt.Prepare(dgnDb, "SELECT ECInstanceId FROM ScalableMeshModel.ScalableMesh;"))
                return nullptr;

            if (BeSQLite::BE_SQLITE_ROW != stmt.Step()) return nullptr;
            DgnModelId smModelID = DgnModelId(stmt.GetValueUInt64(0));
            DgnModelPtr dgnModel = dgnDb.Models().FindModel(smModelID);

            if (dgnModel.get() == 0)
                {
                dgnModel = dgnDb.Models().GetModel(smModelID);
                }

            if (dgnModel.get() != 0)
                {
                assert(dynamic_cast<ScalableMeshModel*>(dgnModel.get()) != 0);

                return static_cast<ScalableMeshModel*>(dgnModel.get());
                }
                */        

            m_modelSearched = true;
            }

        return m_smTerrainPhysicalModelP;
        }
    
    static ScalableMeshTerrainModelAppData* Get (DgnDbCR dgnDb)
        {
        ScalableMeshTerrainModelAppData* appData = dynamic_cast<ScalableMeshTerrainModelAppData*>(dgnDb.FindAppData (ScalableMeshTerrainModelAppData::DataKey));
        if (!appData)
            {
            appData = new ScalableMeshTerrainModelAppData ();
            dgnDb.AddAppData (ScalableMeshTerrainModelAppData::DataKey, appData);
            }

        return appData;
        }

    static void Delete (DgnDbCR dgnDb)
        {        
        dgnDb.DropAppData (ScalableMeshTerrainModelAppData::DataKey);        
        }    
    };

Db::AppData::Key ScalableMeshTerrainModelAppData::DataKey;

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
ScalableMeshModel::ScalableMeshModel(BentleyApi::Dgn::DgnModel::CreateParams const& params)
    : T_Super(params)
    {
    m_tryOpen = false;               
    m_forceRedraw = false;
    m_isProgressiveDisplayOn = true;
    m_isInsertingClips = false;

   // ScalableMeshTerrainModelAppData* appData = ScalableMeshTerrainModelAppData::Get(params.m_dgndb);
   // appData->m_smTerrainPhysicalModelP = this;
   // appData->m_modelSearched = true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
ScalableMeshModel::~ScalableMeshModel()
    {
    if (nullptr != m_progressiveQueryEngine.get() && nullptr != m_currentDrawingInfoPtr.get()) m_progressiveQueryEngine->StopQuery(m_currentDrawingInfoPtr->m_currentQuery);
    if (nullptr != m_currentDrawingInfoPtr.get())
        {
        m_currentDrawingInfoPtr->m_meshNodes.clear();
        m_currentDrawingInfoPtr->m_overviewNodes.clear();
        }
    ScalableMeshTerrainModelAppData::Delete (GetDgnDb());
    //ClearProgressiveQueriesInfo();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::OpenFile(BeFileNameCR smFilename, DgnDbR dgnProject) 
    {    
    assert(m_smPtr == nullptr);
    m_path = smFilename;

    bvector<IMeshSpatialModelP> allScalableMeshes;
    ScalableMeshModel::GetAllScalableMeshes(dgnProject, allScalableMeshes);
    size_t nOfOtherModels = 0;
    for (auto& sm : allScalableMeshes)
        if (sm != this) nOfOtherModels++;
    allScalableMeshes.clear();

    BeFileName clipFileBase = BeFileName(ScalableMeshModel::GetTerrainModelPath(dgnProject)).GetDirectoryName();
    clipFileBase.AppendString(smFilename.GetFileNameWithoutExtension().c_str());
    clipFileBase.AppendUtf8("_");
    clipFileBase.AppendUtf8(std::to_string(nOfOtherModels).c_str());
    m_smPtr = IScalableMesh::GetFor(smFilename.GetWCharCP(), Utf8String(clipFileBase.c_str()), false, true);
    assert(m_smPtr != 0);
    if (m_smPtr->IsTerrain())
        {
         ScalableMeshTerrainModelAppData* appData = ScalableMeshTerrainModelAppData::Get(m_dgndb);
         if (appData->m_smTerrainPhysicalModelP == nullptr)
             {
             appData->m_smTerrainPhysicalModelP = this;
             appData->m_modelSearched = true;
             }
        }
    
    const GeoCoords::GCS& gcs(m_smPtr->GetGCS());

    DPoint3d scale;
    scale.x = 1;
    scale.y = 1;
    scale.z = 1;
    
    if (gcs.HasGeoRef())
        {
        DgnGCSPtr dgnGcsPtr(DgnGCS::CreateGCS(gcs.GetGeoRef().GetBasePtr().get(), dgnProject));        
        dgnGcsPtr->UorsFromCartesian(scale, scale);

        DgnGCSPtr projGCS = dgnProject.GeoLocation().GetDgnGCS();
        if (projGCS.IsValid() && !projGCS->IsEquivalent(*dgnGcsPtr))
            {
            DRange3d smExtent, smExtentUors;
            m_smPtr->GetRange(smExtent);
            Transform trans;
            trans.InitFromScaleFactors(scale.x, scale.y, scale.z);
            trans.Multiply(smExtentUors, smExtent);

            DPoint3d extent;
            extent.DifferenceOf(smExtentUors.high, smExtentUors.low);
            Transform       approxTransform;

            StatusInt status = dgnGcsPtr->GetLocalTransform(&approxTransform, smExtentUors.low, &extent, true/*doRotate*/, true/*doScale*/, *projGCS);
            if (0 == status || 1 == status)
                m_smPtr->SetReprojection(*projGCS, approxTransform);
            }
        }
    else
        {
        if (dgnProject.GeoLocation().GetDgnGCS()!=nullptr)
            dgnProject.GeoLocation().GetDgnGCS()->UorsFromCartesian(scale, scale);
        }
           
    DPoint3d translation = {0,0,0};
    
    m_storageToUorsTransfo = DMatrix4d::FromScaleAndTranslation(scale, translation);                
    
    // NEEDS_WORK_SM
/*
    BeFileName dbFileName(dgnProject.GetDbFileName());
    BeFileName basePath = dbFileName.GetDirectoryName();        
    T_HOST.GetPointCloudAdmin()._CreateLocalFileId(m_properties.m_fileId, smFilename, basePath);    
*/

    m_properties.m_fileId = smFilename.GetNameUtf8();
    
    //m_properties.m_fileId = smFilename.GetNameUtf8();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
ScalableMeshModelP ScalableMeshModel::CreateModel(BentleyApi::Dgn::DgnDbR dgnDb)
    {
    DgnClassId classId(dgnDb.Schemas().GetClassId("ScalableMesh","ScalableMeshModel"));
    BeAssert(classId.IsValid());

    ScalableMeshModelP model = new ScalableMeshModel(DgnModel::CreateParams(dgnDb, classId, dgnDb.Elements().GetRootSubjectId()/*, DgnModel::CreateModelCode("terrain")*/));
    
    BeFileName terrainDefaultFileName(ScalableMeshModel::GetTerrainModelPath(dgnDb));
    model->Insert();
    model->OpenFile(terrainDefaultFileName, dgnDb);
    model->Update();

    ScalableMeshTerrainModelAppData* appData(ScalableMeshTerrainModelAppData::Get(dgnDb));

    appData->m_smTerrainPhysicalModelP = model;
    appData->m_modelSearched = true;
    dgnDb.SaveChanges();
    return model;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
Transform ScalableMeshModel::GetUorsToStorage()
    {
    Transform t;
    t.InitFrom(m_storageToUorsTransfo);
    t = t.ValidatedInverse();
    return t;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
IMeshSpatialModelP ScalableMeshModel::GetTerrainModelP(BentleyApi::Dgn::DgnDbCR dgnDb)
    {   
    return ScalableMeshTerrainModelAppData::Get(dgnDb)->GetModel(dgnDb);
    }


BeFileName ScalableMeshModel::GetPath()
    {
    return m_path;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
IScalableMesh* ScalableMeshModel::GetScalableMesh()
    {
    return m_smPtr.get();
    }
    
//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
WString ScalableMeshModel::GetTerrainModelPath(BentleyApi::Dgn::DgnDbCR dgnDb)
    {
    BeFileName tmFileName;
    tmFileName = dgnDb.GetFileName().GetDirectoryName();
    tmFileName.AppendToPath(dgnDb.GetFileName().GetFileNameWithoutExtension().c_str());
    if (!tmFileName.DoesPathExist())
        BeFileName::CreateNewDirectory(tmFileName.c_str());
    tmFileName.AppendString(L"\\terrain.stm");
    return tmFileName;
    }

void ScalableMeshModel::ClearOverviews(IScalableMeshPtr& targetSM)
    {
#if 0
    GetProgressiveQueryEngine()->ClearOverviews(targetSM.get());
    if (targetSM.get() == m_smPtr.get())
        {
        if (nullptr != m_progressiveQueryEngine.get() && m_currentDrawingInfoPtr.IsValid()) m_progressiveQueryEngine->StopQuery(m_currentDrawingInfoPtr->m_currentQuery);
        }
    if (targetSM.get() == m_smPtr->GetTerrainSM().get())
        {
        if (nullptr != m_progressiveQueryEngine.get() && m_currentDrawingInfoPtr.IsValid()) m_progressiveQueryEngine->StopQuery(m_currentDrawingInfoPtr->m_terrainQuery);
        if (m_currentDrawingInfoPtr.IsValid())
            {
            m_currentDrawingInfoPtr->m_terrainMeshNodes.clear();
            m_currentDrawingInfoPtr->m_terrainOverviewNodes.clear();
            }
        }

#endif
    }

void ScalableMeshModel::LoadOverviews(IScalableMeshPtr& targetSM)
    {
    GetProgressiveQueryEngine()->InitScalableMesh(targetSM);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     3/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::SetActiveClipSets(bset<uint64_t>& activeClips, bset<uint64_t>& previouslyActiveClips)
    {
    if (m_isInsertingClips) return;

    m_activeClips = activeClips;
    bvector<uint64_t> clipIds;
    for (auto& clip: previouslyActiveClips)
       clipIds.push_back(clip);
    GetProgressiveQueryEngine()->SetActiveClips(activeClips, m_smPtr);
    GetProgressiveQueryEngine()->ClearCaching(clipIds, m_smPtr);
    m_forceRedraw = true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     4/2016
//----------------------------------------------------------------------------------------
bool ScalableMeshModel::IsTerrain()
    {
    if (m_smPtr.get() == nullptr) return false;
    return m_smPtr->IsTerrain();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Mathieu.St-Pierre    10/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::SetProgressiveDisplay(bool isProgressiveDisplayOn)
    {
    m_isProgressiveDisplayOn = isProgressiveDisplayOn;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     3/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::GetClipSetIds(bvector<uint64_t>& allShownIds)
    {
    if (m_smPtr.get() != nullptr) m_smPtr->GetAllClipIds(allShownIds);
    }

IMeshSpatialModelP ScalableMeshModelHandler::AttachTerrainModel(DgnDb& db, Utf8StringCR modelName, BeFileNameCR smFilename, RepositoryLinkCR modeledElement)
    {    
    /*    
    BeFileName smtFileName;
    GetScalableMeshTerrainFileName(smtFileName, db.GetFileName());
    
    if (!smtFileName.GetDirectoryName().DoesPathExist())
        BeFileName::CreateNewDirectory(smtFileName.GetDirectoryName().c_str());
        */
    Utf8String nameToSet = modelName;
    DgnClassId classId(db.Schemas().GetClassId("ScalableMesh", "ScalableMeshModel"));
    BeAssert(classId.IsValid());        
         
    ScalableMeshTerrainModelAppData* appData(ScalableMeshTerrainModelAppData::Get(db));

    if (appData->m_smTerrainPhysicalModelP != nullptr)
        nameToSet = Utf8String(smFilename.GetFileNameWithoutExtension().c_str());

    RefCountedPtr<ScalableMeshModel> model(new ScalableMeshModel(DgnModel::CreateParams(db, classId, modeledElement.GetElementId())));

    //After Insert model pointer is handled by DgnModels.
    model->Insert();
    model->OpenFile(smFilename, db);
    model->Update();

    if (model->IsTerrain())
        {
        ScalableMeshTerrainModelAppData* appData(ScalableMeshTerrainModelAppData::Get(db));

        if (appData->m_smTerrainPhysicalModelP == nullptr)
            {
            appData->m_smTerrainPhysicalModelP = model.get();
            appData->m_modelSearched = true;
            }
        }
    else
        {
        nameToSet = Utf8String(smFilename.GetFileNameWithoutExtension().c_str());
        /*
        DgnCode newModelCode(model->GetCode().GetAuthority(), nameToSet, NULL);
        model->SetCode(newModelCode);
*/
        model->Update();
        }

    db.SaveChanges();
    

    return model.get();    
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  03/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::Properties::ToJson(Json::Value& v) const
    {
    v["FileId"] = m_fileId.c_str();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  03/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::Properties::FromJson(Json::Value const& v)
    {
    m_fileId = v["FileId"].asString();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  03/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::_OnSaveJsonProperties()
    {
    T_Super::_OnSaveJsonProperties();

    Json::Value val;

    m_properties.ToJson(val);
            
    SetJsonProperties(json_scalablemesh(), val);
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  03/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::_OnLoadedJsonProperties()
    {
    T_Super::_OnLoadedJsonProperties();
        
    Json::Value val(GetJsonProperties(json_scalablemesh()));
        
    m_properties.FromJson(val);

    if (m_smPtr == 0 && !m_tryOpen)
        {
        //BeFileName smFileName(((this)->m_properties).m_fileId);
        BeFileName smFileName;
        T_HOST.GetPointCloudAdmin()._ResolveFileUri(smFileName, (((this)->m_properties).m_fileId), GetDgnDb());

        if (BeFileName::DoesPathExist(smFileName.c_str()))
            {
            OpenFile(smFileName, GetDgnDb());
            }

        m_tryOpen = true;
        }
    }


HANDLER_DEFINE_MEMBERS(ScalableMeshModelHandler)
