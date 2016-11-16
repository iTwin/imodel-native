/*-------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshSchema/ScalableMeshHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ScalableMeshSchemaPCH.h"

#include <BeSQLite\BeSQLite.h>
#include <ScalableMeshSchema\ScalableMeshHandler.h>
#include "ScalableMeshDisplayCacheManager.h"

#include <ScalableMesh\GeoCoords\GCS.h>



USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA


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

#define SM_ACTIVATE_UPLOADER 0
#define SM_ACTIVATE_LOAD_TEST 0
//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
AxisAlignedBox3dCR ScalableMeshModel::_GetRange() const
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
BentleyStatus ScalableMeshModel::_ReloadClipMask(BentleyApi::Dgn::DgnElementId& clipMaskElementId, bool isNew)
    {
     if (!IsTerrain())
        return SUCCESS;

    bvector<uint64_t> clipIds;
    clipIds.push_back(clipMaskElementId.GetValue());
    m_progressiveQueryEngine->ClearCaching(clipIds, m_smPtr);
    m_forceRedraw = true;
    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_ReloadAllClipMasks()
    {
    if (!IsTerrain())
        return SUCCESS;

    return ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_StartClipMaskBulkInsert()
    {
     if (!IsTerrain())
        return SUCCESS;

    if (nullptr == m_smPtr.get()) return ERROR;
    m_smPtr->SetIsInsertingClips(true);
    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_StopClipMaskBulkInsert()
    {
     if (!IsTerrain())
        return SUCCESS;

    if (nullptr == m_smPtr.get()) return ERROR;
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

static double s_minScreenPixelsPerPoint = 800;

bool IsWireframeRendering(ViewContextCR viewContext)
    {    
    // Check context render mode
    switch (viewContext.GetViewFlags()->GetRenderMode())
        {
        case DgnRenderMode::ConstantShade:
        case DgnRenderMode::SmoothShade:
        case DgnRenderMode::Phong:
        case DgnRenderMode::RayTrace:
        case DgnRenderMode::Radiosity:
        case DgnRenderMode::ParticleTrace:
        case DgnRenderMode::RenderLuxology:
            return false;
                       
        case DgnRenderMode::Wireframe:
        case DgnRenderMode::CrossSection:
        case DgnRenderMode::Wiremesh:
        case DgnRenderMode::HiddenLine:
        case DgnRenderMode::SolidFill:
        case DgnRenderMode::RenderWireframe:
            return true;
        }
        BeAssert(!"Unknown render mode");
        return true;
    }


struct SmCachedGraphics : TransientCachedGraphics
    {
    explicit SmCachedGraphics (DgnDbR dgnDb, QvElem* qvElem) : TransientCachedGraphics (dgnDb) 
        {
        m_qvElem = qvElem;
        }


    virtual void _StrokeForCache(BentleyG06::Dgn::ViewContextR, double) override
        {
        }

    void UnlinkQvElem()
        {
        m_qvElem = 0;
        }

    /*
    virtual void _Draw (ViewContextR context, TransformCP transform)
        {
        if (nullptr != transform)
            context.PushTransform (*transform);
        
            context.DrawCached (*this);
        
            if (nullptr != transform)
                context.PopTransformClip ();
        }
        */
    //bool IsCacheCreated() const {return nullptr != m_qvElem;}
    };


static bool s_waitCheckStop = false;
static Byte s_transparency = 100;
static bool s_applyClip = false;
static bool s_dontShowMesh = false;




void ProgressiveDrawMeshNode(bvector<IScalableMeshCachedDisplayNodePtr>&  meshNodes,
                              bvector<IScalableMeshCachedDisplayNodePtr>& overviewMeshNodes,
                              ViewContextR                                context, 
                              const DMatrix4d&                            storageToUors,
                              ScalableMeshDisplayCacheManager*            mgr)
    {    

#ifdef PRINT_SMDISPLAY_MSG
    PRINT_MSG("ProgressiveDrawMeshNode meshNode : %I64u overviewMeshNode : %I64u \n", meshNodes.size(), overviewMeshNodes.size());
#endif

    static size_t s_callCount = 0;
    
    bool isOutputQuickVision = context.GetIViewDraw ().IsOutputQuickVision();

    //Since QVElem are created in the background in a parallel fashion we always want to use the Cached Graphics.
    bool& usedCached = context.GetUseCachedGraphics();
    bool usedCachedOld = usedCached;
    usedCached = true;

    //NEEDS_WORK_MST : Will be fixed when the lowest resolution is created and pin at creation time.
    //assert(overviewMeshNodes.size() > 0 || meshNodes.size() > 0);
    assert(isOutputQuickVision == true);

    /*ElemMatSymbP matSymbP = context.GetElemMatSymb ();

    matSymbP->Init ();
    matSymbP->SetLineColor (ColorDef(0,0x77,0));
    matSymbP->SetFillColor (ColorDef(0,0x77,0));    
    
    context.ResetContextOverrides(); // If not reset, last drawn override is applyed to dtm (Selected/Hide preview)
    context.GetIDrawGeom ().ActivateMatSymb (matSymbP);*/

    if (s_waitCheckStop)
        {
        while (!context.CheckStop())
            {
            }
        }


      
    Transform storageToUorsTransform;
    storageToUorsTransform.InitFrom(storageToUors);
    context.PushTransform(storageToUorsTransform);

    bvector<IScalableMeshCachedDisplayNodePtr> requestedNodes;
    bvector<IScalableMeshCachedDisplayNodePtr> nodesWithoutQvElem;
    
    if (overviewMeshNodes.size() > 0)
        {
        //NEEDS_WORK_SM : If kept needs clean up
        for (size_t nodeInd = 0; nodeInd < overviewMeshNodes.size(); nodeInd++)
            {             
            if (context.CheckStop())
                break;                           
                                        
            //NEEDS_WORK_SM_PROGRESSIVE : IsMeshLoaded trigger load header.
            //assert(overviewMeshNodes[nodeInd]->IsHeaderLoaded() && overviewMeshNodes[nodeInd]->IsMeshLoaded());
            /*
            if (!meshNodes[nodeInd]->IsHeaderLoaded() || !meshNodes[nodeInd]->IsMeshLoaded())
                requestedNodes.push_back(meshNodes[nodeInd]);
            else
            */            
            
                {
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
                }

            
                bvector<SmCachedDisplayMesh*> cachedMeshes;
                bvector<bpair<bool, uint64_t>> textureIDs;
            QvElem* qvElem = 0;                
            bool isEmptyMesh = false;

            if (isOutputQuickVision && (SUCCESS == overviewMeshNodes[nodeInd]->GetCachedMeshes(cachedMeshes, textureIDs)))
                {
                for (auto&cachedMesh : cachedMeshes)
                    {
                    if (cachedMesh != 0)
                        {
                        qvElem = cachedMesh->m_qvElem;
                        //assert(qvElem != 0);
                        }
                    else
                        {
                        qvElem = 0;
                        isEmptyMesh = true;
                        }
                    }
                if (cachedMeshes.empty()) isEmptyMesh = true;
                }
            else
                {
                /*NEEDS_WORK_SM : Not support yet.
                __int64 meshId = GetMeshId(overviewMeshNodes[nodeInd]->GetNodeId());

                qvElem = QvCachedNodeManager::GetManager().FindQvElem(meshId, dtmDataRef.get());
                */
                }

            if (qvElem != 0)
                {
                if (cachedMeshes.empty())
                    {
                    //NEEDS_WORK_SM : Not support yet.
                    //ActivateMaterial(overviewMeshNodes[nodeInd], context);
                    }

                bvector<ClipVectorPtr> clipVectors;

                overviewMeshNodes[nodeInd]->GetDisplayClipVectors(clipVectors);

                if (clipVectors.size() == 0 || !s_applyClip)
                    {
                    SmCachedGraphics smCached(context.GetDgnDb(), qvElem);
                    context.DrawCached(smCached);
                    smCached.UnlinkQvElem();
                    }
                else
                    {
                    for (auto& clip : clipVectors)
                        {
                        context.PushClip(*clip);
                        SmCachedGraphics smCached(context.GetDgnDb(), qvElem);
                        context.DrawCached(smCached);
                        smCached.UnlinkQvElem();
                        context.PopTransformClip();
                        }
                    }

                //context.DrawQvElem (qvElem, &storageToUorsTransform, 0, false, false, true);                                       
                }
            else
                if (!isEmptyMesh)
                    {
                    //nodesWithoutQvElem.push_back(overviewMeshNodes[nodeInd]);                
                    //NEEDS_WORK_SM_PROGRESSIVE : Getclip passed to progressive display engine
                    /*
                    bvector<bool> clips;
                    dtmDataRef->GetVisibleClips(clips);
                    IScalableMeshMeshPtr mrdtmMeshPtr(overviewMeshNodes[nodeInd]->GetMeshByParts(clips));

                    if (mrdtmMeshPtr != 0)
                    {
                    ActivateMaterial(overviewMeshNodes[nodeInd], context);

                    __int64 meshId = GetMeshId(overviewMeshNodes[nodeInd]->GetNodeId());
                    CreateQvElemForMesh(mrdtmMeshPtr, dtmDataRef, element, context, meshId, drawingInfo);
                    }
                    */
                    }
                
            } 
        }


     if (meshNodes.size() > 0 && !s_dontShowMesh)
        {
        //NEEDS_WORK_SM : If kept needs clean up
        for (size_t nodeInd = 0; nodeInd < meshNodes.size(); nodeInd++)
            {                      
            if (context.CheckStop())
                break;            
            
            //NEEDS_WORK_SM_PROGRESSIVE : IsMeshLoaded trigger load header.
            //assert(meshNodes[nodeInd]->IsHeaderLoaded() && meshNodes[nodeInd]->IsMeshLoaded());
            /*
            if (!meshNodes[nodeInd]->IsHeaderLoaded() || !meshNodes[nodeInd]->IsMeshLoaded())
                requestedNodes.push_back(meshNodes[nodeInd]);
            else
            */
            bool wasDrawn = false;
          
            bvector<SmCachedDisplayMesh*> cachedMeshes;
            bvector<bpair<bool,uint64_t>> textureIDs;
            QvElem* qvElem = 0;                
            bool isEmptyNode = false;


            if (isOutputQuickVision && (SUCCESS == meshNodes[nodeInd]->GetCachedMeshes(cachedMeshes, textureIDs)))
                {
                for (auto& cachedMesh : cachedMeshes)
                    {
                    if (cachedMesh != 0)
                        {
                        qvElem = cachedMesh->m_qvElem;
                        char elemId[500];
                        std::sprintf(elemId, "%p", qvElem);
                        if (!mgr->IsValid(qvElem) || !mgr->IsValidForId(qvElem, meshNodes[nodeInd]->GetNodeId()))
                        {
                            volatile int a = 1;
                            a = a;
                        }
                        //assert(qvElem != 0);
                        }
                    else
                        {
                        qvElem = 0;
                        isEmptyNode = true;
                        }


                    //else
                    //     {
                    /*NEEDS_WORK_SM : Not support yet.
                    __int64 meshId = GetMeshId(meshNodes[nodeInd]->GetNodeId());
                    qvElem = QvCachedNodeManager::GetManager().FindQvElem(meshId, dtmDataRef.get());
                    */
                    //  }

                    if (qvElem != 0)
                        {
                        wasDrawn = true;
                        if (cachedMeshes.empty())
                            {
                            //NEEDS_WORK_SM : Not support yet.
                            //ActivateMaterial(meshNodes[nodeInd], context);
                            }
                                                    {
                                                    ElemMatSymbP matSymbP = context.GetElemMatSymb();

                                                    matSymbP->Init();
                                                    ColorDef white(0xff, 0xff, 0xff);
                                                    ColorDef green(0, 0x77, 0);
                                                    matSymbP->SetLineColor(meshNodes[nodeInd]->IsTextured() != 0 ? white : green);
                                                    matSymbP->SetFillColor(meshNodes[nodeInd]->IsTextured() != 0 ? white : green);
                                                    context.OnPreDrawTransient(); // If not reset, last drawn override is applyed to dtm (Selected/Hide preview)
                                                    context.GetIDrawGeom().ActivateMatSymb(matSymbP);
                                                        }
                                                    SmCachedGraphics smCached(context.GetDgnDb(), qvElem);
                                                    context.DrawCached(smCached);
                                                    smCached.UnlinkQvElem();
                        }
                    else
                        if (!isEmptyNode)
                            {
                            nodesWithoutQvElem.push_back(meshNodes[nodeInd]);
                            }
                    }

                }
            if (cachedMeshes.empty())
                {
                nodesWithoutQvElem.push_back(meshNodes[nodeInd]);
                }
            
            if (wasDrawn)
                {
                ElemMatSymbP matSymbP = context.GetElemMatSymb();

                matSymbP->Init();
                ColorDef green(0, 0x77, 0);
                matSymbP->SetLineColor(green);
                matSymbP->SetFillColor(green);
                context.OnPreDrawTransient(); 
                context.GetIDrawGeom().ActivateMatSymb(matSymbP);
                bvector<PolyfaceHeaderPtr> skirtMeshParts;
                meshNodes[nodeInd]->GetSkirtMeshes(skirtMeshParts);
                for (auto& part : skirtMeshParts)
                    context.GetIDrawGeom().DrawPolyface(*part);
                }
            }
    
        for (auto& node : nodesWithoutQvElem)
            {
            if (context.CheckStop())
                break;     

            bset<uint64_t> clips;
            //NEEDS_WORK_SM : Not supported yet
            //dtmDataRef->GetVisibleClips(clips);
            
            IScalableMeshMeshPtr mrdtmMeshPtr(node->GetMeshByParts(clips));

            if (mrdtmMeshPtr != 0)
                {         
                /*NEEDS_WORK_SM : Not supported yet
                ActivateMaterial(node, meshInd, context);

                __int64 meshId = GetMeshId(node->GetNodeId(), meshInd);
                CreateQvElemForMesh(mrdtmMeshPtr, dtmDataRef, element, context, meshId, drawingInfo);                                                   
                */                    
                }
            }
        }


    context.PopTransformClip();

    //Restore GetUseCachedGraphics setting
    usedCached = usedCachedOld;    
    }


//========================================================================================
// @bsiclass                                                        Mathieu.St-Pierre     02/2016
//========================================================================================
static bool s_drawInProcess = true;

struct ScalableMeshProgressiveDisplay : Dgn::IProgressiveDisplay, NonCopyableClass
{
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS

protected:
        
    IScalableMeshProgressiveQueryEnginePtr  m_progressiveQueryEngine;        
    ScalableMeshDrawingInfoPtr              m_currentDrawingInfoPtr;
    const DMatrix4d&                        m_storageToUorsTransfo;
    bool                                    m_hasFetchedFinalNode;
    bool                                    m_hasFetchedFinalTerrainNode;
    IScalableMeshDisplayCacheManager*       m_displayNodesCache;


    ScalableMeshProgressiveDisplay (IScalableMeshProgressiveQueryEnginePtr& progressiveQueryEngine,
                                    ScalableMeshDrawingInfoPtr&             currentDrawingInfoPtr, 
                                    DMatrix4d&                              storageToUorsTransfo,
                                     IScalableMeshDisplayCacheManagerPtr& cacheManager)
    : m_storageToUorsTransfo(storageToUorsTransfo)
        { 
        DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT

        m_progressiveQueryEngine = progressiveQueryEngine;
        m_currentDrawingInfoPtr = currentDrawingInfoPtr;        
        m_hasFetchedFinalNode = false;
        m_hasFetchedFinalTerrainNode = m_currentDrawingInfoPtr->m_coverageClips.empty();      
        m_displayNodesCache = cacheManager.get();
        }

public:

    virtual bool _WantTimeoutSet(uint32_t& limit)   {return false; }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Mathieu.St-Pierre     02/2016
//----------------------------------------------------------------------------------------
virtual Completion _Process(ViewContextR viewContext) override
    {

    if (m_hasFetchedFinalTerrainNode && m_hasFetchedFinalNode && !((ScalableMeshDisplayCacheManager*)m_displayNodesCache)->IsDirty())
        return Completion::Finished;

    Completion completionStatus = Completion::Aborted; 

    if (!m_currentDrawingInfoPtr->m_coverageClips.empty() && !m_hasFetchedFinalTerrainNode)
        {
        if (!m_progressiveQueryEngine->IsQueryComplete(m_currentDrawingInfoPtr->m_terrainQuery))
            {
            m_currentDrawingInfoPtr->m_terrainMeshNodes.clear();
            StatusInt status = m_progressiveQueryEngine->GetRequiredNodes(m_currentDrawingInfoPtr->m_terrainMeshNodes, m_currentDrawingInfoPtr->m_terrainQuery);
            assert(status == SUCCESS);

            m_currentDrawingInfoPtr->m_terrainOverviewNodes.clear();
            status = m_progressiveQueryEngine->GetOverviewNodes(m_currentDrawingInfoPtr->m_terrainOverviewNodes, m_currentDrawingInfoPtr->m_terrainQuery);
            assert(status == SUCCESS);
            }
        else
            {
            m_currentDrawingInfoPtr->m_terrainMeshNodes.clear();

            StatusInt status = m_progressiveQueryEngine->GetRequiredNodes(m_currentDrawingInfoPtr->m_terrainMeshNodes, m_currentDrawingInfoPtr->m_terrainQuery);

            assert(status == SUCCESS);
            m_currentDrawingInfoPtr->m_terrainOverviewNodes.clear();

            status = m_progressiveQueryEngine->StopQuery(m_currentDrawingInfoPtr->m_terrainQuery);

            assert(status == SUCCESS);            
            m_hasFetchedFinalTerrainNode = true;
            }
        }

    if (!m_hasFetchedFinalNode)
        {                    
        int queryId = m_currentDrawingInfoPtr->m_currentQuery;

        if (m_progressiveQueryEngine->IsQueryComplete(queryId))
            {
            m_currentDrawingInfoPtr->m_meshNodes.clear();

            StatusInt status = m_progressiveQueryEngine->GetRequiredNodes(m_currentDrawingInfoPtr->m_meshNodes, queryId);

            assert(status == SUCCESS);

            assert(m_currentDrawingInfoPtr->m_overviewNodes.size() == 0 || m_currentDrawingInfoPtr->m_meshNodes.size() > 0);
            bvector<IScalableMeshNodePtr> nodes;
            for (auto& nodeP : m_currentDrawingInfoPtr->m_meshNodes) nodes.push_back(nodeP.get());
            m_currentDrawingInfoPtr->m_smPtr->SetCurrentlyViewedNodes(nodes);

            m_currentDrawingInfoPtr->m_overviewNodes.clear();                
                            
            status = m_progressiveQueryEngine->StopQuery(queryId);

            assert(status == SUCCESS);

#ifdef PRINT_SMDISPLAY_MSG        
            PRINT_MSG("Heal required  meshNode : %I64u overviewMeshNode : %I64u \n", m_currentDrawingInfoPtr->m_meshNodes.size(), m_currentDrawingInfoPtr->m_overviewNodes.size());       
#endif

            m_hasFetchedFinalNode = true;
            }
        else
            {                                            
            m_currentDrawingInfoPtr->m_meshNodes.clear();
            StatusInt status = m_progressiveQueryEngine->GetRequiredNodes(m_currentDrawingInfoPtr->m_meshNodes, queryId);
            assert(status == SUCCESS);                                  
            
            m_currentDrawingInfoPtr->m_overviewNodes.clear();
            status = m_progressiveQueryEngine->GetOverviewNodes(m_currentDrawingInfoPtr->m_overviewNodes, queryId);
            assert(status == SUCCESS);                                                                      
            }
        }

    if (m_hasFetchedFinalTerrainNode && m_hasFetchedFinalNode)
        {
        completionStatus = Completion::HealRequired;
        }
    else    
    if (s_drawInProcess)
        {
        ProgressiveDrawMeshNode(m_currentDrawingInfoPtr->m_meshNodes, m_currentDrawingInfoPtr->m_overviewNodes, viewContext, m_storageToUorsTransfo, (ScalableMeshDisplayCacheManager*)m_displayNodesCache);
        if (!m_currentDrawingInfoPtr->m_coverageClips.empty())
            {
            for (auto& clip : m_currentDrawingInfoPtr->m_coverageClips)
                {
                viewContext.PushClip(*clip);
                ProgressiveDrawMeshNode(m_currentDrawingInfoPtr->m_terrainMeshNodes, m_currentDrawingInfoPtr->m_terrainOverviewNodes, viewContext, m_storageToUorsTransfo, (ScalableMeshDisplayCacheManager*)m_displayNodesCache);
                viewContext.PopTransformClip();
                }
            }
        }
            
    return completionStatus;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                               Mathieu.St-Pierre      02/2016
//----------------------------------------------------------------------------------------
static void Schedule (IScalableMeshProgressiveQueryEnginePtr& progressiveQueryEngine,
                      ScalableMeshDrawingInfoPtr&             currentDrawingInfoPtr, 
                      DMatrix4d&                              storageToUorsTransfo, 
                      ViewContextR                            context,
                      IScalableMeshDisplayCacheManagerPtr& cacheManager)
    {
    RefCountedPtr<ScalableMeshProgressiveDisplay> progressiveDisplay(new ScalableMeshProgressiveDisplay(progressiveQueryEngine,
                                                                                                        currentDrawingInfoPtr,
                                                                                                        storageToUorsTransfo,
                                                                                                        cacheManager));
    
    context.GetViewport()->ScheduleProgressiveDisplay (*progressiveDisplay);
    }

};  

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
bool ShouldDrawInContext (ViewContextR context) 
    {
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

    return true;
    }

static bool s_loadTexture = true;
static bool s_waitQueryComplete = false;

void ScalableMeshModel::_AddGraphicsToScene(ViewContextR context)
    {       
    if (m_smPtr == 0 && !m_tryOpen)
        {
        //BeFileName smFileName(((this)->m_properties).m_fileId);
        BeFileName smFileName;
        T_HOST.GetPointCloudAdmin()._ResolveFileName(smFileName, (((this)->m_properties).m_fileId), GetDgnDb());

        if (BeFileName::DoesPathExist(smFileName.c_str()))
            {
            OpenFile(smFileName, GetDgnDb()); 
            }

        m_tryOpen = true;
        }

    if (!ShouldDrawInContext(context) || NULL == context.GetViewport() || !m_smPtr.IsValid())
        return;
         
    ScalableMeshDrawingInfoPtr nextDrawingInfoPtr(new ScalableMeshDrawingInfo(&context));
    nextDrawingInfoPtr->m_smPtr = m_smPtr.get();
    nextDrawingInfoPtr->m_currentQuery = (int)((GetModelId().GetValue() - GetModelId().GetBriefcaseId().GetValue()) & 0xFFFF);
    nextDrawingInfoPtr->m_terrainQuery = (int)((GetModelId().GetValue() - GetModelId().GetBriefcaseId().GetValue()) & 0xFFFFFFFF | 0xAFFF);//nextDrawingInfoPtr->GetViewNumber();                 

    

    bvector<bvector<DPoint3d>> coverages;
    m_smPtr->GetAllCoverages(coverages);

    bvector<ClipVectorPtr> clipFromCoverageSet;
    if (!coverages.empty())
        {
        //clip vector from coverages
        if (m_currentDrawingInfoPtr.IsValid() && m_currentDrawingInfoPtr->m_hasCoverage && coverages.size() == m_currentDrawingInfoPtr->m_coverageClips.size()) clipFromCoverageSet = m_currentDrawingInfoPtr->m_coverageClips;
        else
            {
            if (!m_currentDrawingInfoPtr.IsValid() || !m_currentDrawingInfoPtr->m_hasCoverage)
                {
                auto smPtr = m_smPtr->GetTerrainSM();

                if (smPtr.IsValid())
                    m_progressiveQueryEngine->InitScalableMesh(smPtr);
                }            

            for (auto& coverageVal : coverages)
                {
                DPoint3d origin;
                DVec3d normal;
                double area;
                PolygonOps::CentroidNormalAndArea(&coverageVal[0], (int)coverageVal.size(), origin, normal, area);
                Transform toCoverageTrans = Transform::FromFixedPointAndScaleFactors(origin, 1.1, 1.1, 1);
                toCoverageTrans.Multiply(&coverageVal[0], (int)coverageVal.size());
                CurveVectorPtr curvePtr = CurveVector::CreateLinear(coverageVal, CurveVector::BOUNDARY_TYPE_Outer, true);
                ClipPrimitivePtr clipPrimitive = ClipPrimitive::CreateFromBoundaryCurveVector(*curvePtr, DBL_MAX, 0, 0, 0, 0, true);
                clipPrimitive->SetIsMask(false);
                ClipVectorPtr clip = ClipVector::CreateFromPrimitive(clipPrimitive);
                clipFromCoverageSet.push_back(clip);
                }
            nextDrawingInfoPtr->m_coverageClips = clipFromCoverageSet;
            }

        nextDrawingInfoPtr->m_hasCoverage = true;
        }

    if ((m_currentDrawingInfoPtr != nullptr) &&
        (m_currentDrawingInfoPtr->GetDrawPurpose() != DrawPurpose::UpdateDynamic))
        {
        //If the m_dtmPtr equals 0 it could mean that the last data request to the STM was cancelled, so start a new request even
        //if the view has not changed.
        if (m_currentDrawingInfoPtr->HasAppearanceChanged(nextDrawingInfoPtr) == false && !m_forceRedraw)                
            {
            //assert((m_currentDrawingInfoPtr->m_overviewNodes.size() == 0) && (m_currentDrawingInfoPtr->m_meshNodes.size() > 0));

            ProgressiveDrawMeshNode(m_currentDrawingInfoPtr->m_meshNodes, m_currentDrawingInfoPtr->m_overviewNodes, context, m_storageToUorsTransfo, (ScalableMeshDisplayCacheManager*)m_displayNodesCache.get());
            if (!clipFromCoverageSet.empty())
                {
                for (auto& clip : clipFromCoverageSet)
                    {
                    context.PushClip(*clip);
                    ProgressiveDrawMeshNode(m_currentDrawingInfoPtr->m_terrainMeshNodes, m_currentDrawingInfoPtr->m_terrainOverviewNodes, context, m_storageToUorsTransfo, (ScalableMeshDisplayCacheManager*)m_displayNodesCache.get());
                    context.PopTransformClip();
                    }
                }
            return;                        
            }   
        }        
    BentleyStatus status;

    status = m_progressiveQueryEngine->StopQuery(/*nextDrawingInfoPtr->GetViewNumber()*/nextDrawingInfoPtr->m_currentQuery);
    if (!clipFromCoverageSet.empty())
        {
        status = m_progressiveQueryEngine->StopQuery(nextDrawingInfoPtr->m_terrainQuery);
        }
    assert(status == SUCCESS);
                                   
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

    DMatrix4d localToView(context.GetLocalToView());
                                   
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
            
    ClipVectorCP clip;
    clip = context.GetTransformClipStack().GetClip();
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

    ClipVectorPtr clipVectorCopy(ClipVector::CreateCopy(*clip));
    
    Transform rootToStorageTransform; 
    bool result = rootToStorageTransform.InitFrom (rootToStorage); 
    assert(result == true);

    clipVectorCopy->TransformInPlace(rootToStorageTransform);
    
    viewDependentQueryParams->SetViewClipVector(clipVectorCopy);
                          
    m_currentDrawingInfoPtr->m_overviewNodes.clear();
    int queryId = (int)((GetModelId().GetValue() - GetModelId().GetBriefcaseId().GetValue()) & 0xFFFF);//nextDrawingInfoPtr->GetViewNumber();                 
    m_currentDrawingInfoPtr->m_currentQuery = queryId;
    bvector<bool> clips;
    /*NEEDS_WORK_SM : Get clips
    m_DTMDataRef->GetVisibleClips(clips);
    */

    status = m_progressiveQueryEngine->StartQuery(queryId, 
                                                  viewDependentQueryParams, 
                                                  m_currentDrawingInfoPtr->m_meshNodes, 
                                                  true, //No wireframe mode, so always load the texture.
                                                  clips,
                                                  m_smPtr); 


    assert(status == SUCCESS);


    if (s_waitQueryComplete || !m_isProgressiveDisplayOn)
        {
        while (!m_progressiveQueryEngine->IsQueryComplete(queryId))
            {
            BeThreadUtilities::BeSleep (200);
            }
        }

    int terrainQueryId = -1;    
    auto terrainSM = m_smPtr->GetTerrainSM();
    
    if (!clipFromCoverageSet.empty() && terrainSM.IsValid())
        {
        m_currentDrawingInfoPtr->m_terrainOverviewNodes.clear();
        terrainQueryId = (int)((GetModelId().GetValue() - GetModelId().GetBriefcaseId().GetValue()) & 0xFFFFFFFF | 0xAFFF);//nextDrawingInfoPtr->GetViewNumber();                 
        m_currentDrawingInfoPtr->m_terrainQuery = terrainQueryId;
        bvector<bool> clips;
        /*NEEDS_WORK_SM : Get clips
        m_DTMDataRef->GetVisibleClips(clips);
        */               
        status = m_progressiveQueryEngine->StartQuery(terrainQueryId,
                                                      viewDependentQueryParams,
                                                      m_currentDrawingInfoPtr->m_terrainMeshNodes,
                                                      true, //No wireframe mode, so always load the texture.
                                                      clips,
                                                     terrainSM);

        if (!m_isProgressiveDisplayOn)
            {
            while (!m_progressiveQueryEngine->IsQueryComplete(terrainQueryId))
                {
                BeThreadUtilities::BeSleep (200);
                }
            }
        }

    bool needProgressive;
    bool isTerrainComplete = false;

    if (m_progressiveQueryEngine->IsQueryComplete(queryId))
        {        
        m_currentDrawingInfoPtr->m_meshNodes.clear();
        status = m_progressiveQueryEngine->GetRequiredNodes(m_currentDrawingInfoPtr->m_meshNodes, queryId);
        assert(status == SUCCESS);
        m_currentDrawingInfoPtr->m_overviewNodes.clear();
       
        bvector<IScalableMeshNodePtr> nodes;
        for (auto& nodeP : m_currentDrawingInfoPtr->m_meshNodes) nodes.push_back(nodeP.get());
        m_smPtr->SetCurrentlyViewedNodes(nodes);

        /*
        BentleyStatus status;
        status = m_progressiveQueryEngine->StopQuery(queryId);
        assert(status == SUCCESS);
        */
                        
        needProgressive = false;        
        }
    else
        {  
        bvector<IScalableMeshNodePtr> nodes;
        for (auto& nodeP : m_currentDrawingInfoPtr->m_meshNodes) nodes.push_back(nodeP.get());
        m_smPtr->SetCurrentlyViewedNodes(nodes);
        status = m_progressiveQueryEngine->GetOverviewNodes(m_currentDrawingInfoPtr->m_overviewNodes, queryId);

        m_currentDrawingInfoPtr->m_meshNodes.clear();

        status = m_progressiveQueryEngine->GetRequiredNodes(m_currentDrawingInfoPtr->m_meshNodes, queryId);
        assert(status == SUCCESS);
        
        //NEEDS_WORK_MST : Will be fixed when the lowest resolution is created and pin at creation time.
        //assert(m_currentDrawingInfoPtr->m_overviewNodes.size() > 0);
        assert(status == SUCCESS);

        needProgressive = true;
        }                         

    if (!clipFromCoverageSet.empty() && terrainSM.IsValid())
        {
        if (m_progressiveQueryEngine->IsQueryComplete(terrainQueryId))
            {
            m_currentDrawingInfoPtr->m_terrainMeshNodes.clear();
            status = m_progressiveQueryEngine->GetRequiredNodes(m_currentDrawingInfoPtr->m_terrainMeshNodes, terrainQueryId);
            assert(status == SUCCESS);
            m_currentDrawingInfoPtr->m_terrainOverviewNodes.clear();
            /*
            BentleyStatus status;
            status = m_progressiveQueryEngine->StopQuery(terrainQueryId);
            assert(status == SUCCESS);
            */

            isTerrainComplete = true;
            }
        else
            {
            status = m_progressiveQueryEngine->GetOverviewNodes(m_currentDrawingInfoPtr->m_terrainOverviewNodes, terrainQueryId);

            m_currentDrawingInfoPtr->m_terrainMeshNodes.clear();

            status = m_progressiveQueryEngine->GetRequiredNodes(m_currentDrawingInfoPtr->m_terrainMeshNodes, terrainQueryId);
            assert(status == SUCCESS);
            needProgressive = true;
            isTerrainComplete = false;
            }
        }
    else isTerrainComplete = true;

    if (isTerrainComplete && !needProgressive && !!((ScalableMeshDisplayCacheManager*)m_displayNodesCache.get())->IsDirty()) m_forceRedraw = false;

    ProgressiveDrawMeshNode(m_currentDrawingInfoPtr->m_meshNodes, m_currentDrawingInfoPtr->m_overviewNodes, context, m_storageToUorsTransfo, (ScalableMeshDisplayCacheManager*)m_displayNodesCache.get());
    if (!clipFromCoverageSet.empty() && terrainSM.IsValid())
        {
        for (auto&clip : clipFromCoverageSet)
            {            
            context.PushClip(*clip);
            ProgressiveDrawMeshNode(m_currentDrawingInfoPtr->m_terrainMeshNodes, m_currentDrawingInfoPtr->m_terrainOverviewNodes, context, m_storageToUorsTransfo, (ScalableMeshDisplayCacheManager*)m_displayNodesCache.get());
            context.PopTransformClip();
            }
        }

    if (needProgressive)
        {
        ScalableMeshProgressiveDisplay::Schedule(m_progressiveQueryEngine, m_currentDrawingInfoPtr, m_storageToUorsTransfo, context, m_displayNodesCache);
        }

    }                 


void ScalableMeshModel::GetAllScalableMeshes(BentleyApi::Dgn::DgnDbCR dgnDb, bvector<IMeshSpatialModelP>& models)
    {
    DgnClassId classId(dgnDb.Schemas().GetECClassId("ScalableMesh", "ScalableMeshModel"));
    BeAssert(classId.IsValid());

    for (auto& model : dgnDb.Models().GetLoadedModels())
        {
        if (model.second->GetClassId() == classId) models.push_back(dynamic_cast<IMeshSpatialModelP>(model.second.get()));
        }
    }

//NEEDS_WORK_SM : Should be at application level
void GetScalableMeshTerrainFileName(BeFileName& smtFileName, const BeFileName& dgnDbFileName)
    {    
    //smtFileName = params.m_dgndb.GetFileName().GetDirectoryName();

    smtFileName = dgnDbFileName.GetDirectoryName();
    smtFileName.AppendToPath(dgnDbFileName.GetFileNameWithoutExtension().c_str());
    smtFileName.AppendString(L"\\terrain.3sm");
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

   // ScalableMeshTerrainModelAppData* appData = ScalableMeshTerrainModelAppData::Get(params.m_dgndb);
   // appData->m_smTerrainPhysicalModelP = this;
   // appData->m_modelSearched = true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
ScalableMeshModel::~ScalableMeshModel()
    {
    if (nullptr != m_progressiveQueryEngine.get()) m_progressiveQueryEngine->StopQuery(m_currentDrawingInfoPtr->m_currentQuery);
    if (nullptr != m_currentDrawingInfoPtr.get())
        {
        m_currentDrawingInfoPtr->m_meshNodes.clear();
        m_currentDrawingInfoPtr->m_overviewNodes.clear();
        }
    ScalableMeshTerrainModelAppData::Delete (GetDgnDb());
    ClearProgressiveQueriesInfo();
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
    size_t nOfModels = allScalableMeshes.size();
    allScalableMeshes.clear();

    BeFileName clipFileBase = BeFileName(ScalableMeshModel::GetTerrainModelPath(dgnProject)).GetDirectoryName();
    clipFileBase.AppendString(smFilename.GetFileNameWithoutExtension().c_str());
    clipFileBase.AppendUtf8("_");
    clipFileBase.AppendUtf8(std::to_string(nOfModels-1).c_str());
    m_smPtr = IScalableMesh::GetFor(smFilename.GetWCharCP(), Utf8String(clipFileBase.c_str()), false, true);
    assert(m_smPtr != 0);

#if SM_ACTIVATE_UPLOADER == 1
    WString projectName = dgnProject.GetFileName().GetFileNameWithoutExtension();
    if (projectName.Contains(WString(L"upload_to_cloud")))
        {
        if (projectName.Equals(WString(L"upload_to_cloud_wsg")))
            {
            WString container(L"scalablemesh"); // WSG container
            m_smPtr->ConvertToCloud(container, smFilename.GetFileNameWithoutExtension(), SMCloudServerType::WSG);
            }
        else if (projectName.Equals(WString(L"upload_to_cloud_azure")))
            {
            WString container(L"scalablemeshtest"); // Azure container
            m_smPtr->ConvertToCloud(container, smFilename.GetFileNameWithoutExtension(), SMCloudServerType::Azure);
            }
        else if (projectName.Equals(WString(L"upload_to_cloud_local")))
            {
            WString container(L"scalablemeshtest"); // local disk container
            m_smPtr->ConvertToCloud(container, smFilename.GetFileNameWithoutExtension(), SMCloudServerType::LocalDisk);
            }
        else
            {
            assert(false); // unknown service
            }
        }
#endif

#if SM_ACTIVATE_LOAD_TEST == 1
    WString projectName = dgnProject.GetFileName().GetFileNameWithoutExtension();
    if (projectName.Contains(WString(L"load_test")))
        {
        size_t nbLoadedNodes = 0;
        m_smPtr->LoadAllNodeHeaders(nbLoadedNodes, 6);
        }
#endif

    if (m_smPtr->IsTerrain())
        {
         ScalableMeshTerrainModelAppData* appData = ScalableMeshTerrainModelAppData::Get(m_dgndb);
         if (appData->m_smTerrainPhysicalModelP == nullptr)
             {
             appData->m_smTerrainPhysicalModelP = this;
             appData->m_modelSearched = true;
             }
        }

    if (m_progressiveQueryEngine == nullptr)
        {
        m_displayNodesCache = new ScalableMeshDisplayCacheManager(dgnProject);
        m_progressiveQueryEngine = IScalableMeshProgressiveQueryEngine::Create(m_smPtr, m_displayNodesCache);

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

        DgnGCSPtr projGCS = dgnProject.Units().GetDgnGCS();
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
        dgnProject.Units().GetDgnGCS()->UorsFromCartesian(scale, scale);
        }
           
    DPoint3d translation = {0,0,0};
    
    m_storageToUorsTransfo = DMatrix4d::FromScaleAndTranslation(scale, translation);                
    
    // NEEDS_WORK_SM
    BeFileName dbFileName(dgnProject.GetDbFileName());
    BeFileName basePath = dbFileName.GetDirectoryName();
    T_HOST.GetPointCloudAdmin()._CreateLocalFileId(m_properties.m_fileId, smFilename, basePath);

    bvector<uint64_t> allClips;
    bset<uint64_t> clipsToShow;
    bset<uint64_t> clipsShown;
    GetClipSetIds(allClips);
    for (auto elem : allClips)
        clipsToShow.insert(elem);
    SetActiveClipSets(clipsToShow, clipsShown);
    //m_properties.m_fileId = smFilename.GetNameUtf8();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::CloseFile()
    {
    if (nullptr != m_progressiveQueryEngine.get() && nullptr != m_currentDrawingInfoPtr.get()) m_progressiveQueryEngine->StopQuery(m_currentDrawingInfoPtr->m_currentQuery);
    if (nullptr != m_currentDrawingInfoPtr.get())
        {
        m_currentDrawingInfoPtr->m_meshNodes.clear();
        m_currentDrawingInfoPtr->m_overviewNodes.clear();
        }
    m_progressiveQueryEngine = nullptr;
    m_smPtr = nullptr;
    m_displayNodesCache = nullptr;
    m_tryOpen = false;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
ScalableMeshModelP ScalableMeshModel::CreateModel(BentleyApi::Dgn::DgnDbR dgnDb)
    {
    DgnClassId classId(dgnDb.Schemas().GetECClassId("ScalableMesh","ScalableMeshModel"));
    BeAssert(classId.IsValid());

    ScalableMeshModelP model = new ScalableMeshModel(DgnModel::CreateParams(dgnDb, classId, DgnModel::CreateModelCode("terrain")));
    
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
    tmFileName.AppendString(L"\\terrain.3sm");
    return tmFileName;
    }

void ScalableMeshModel::ClearOverviews(IScalableMeshPtr& targetSM)
    {
    m_progressiveQueryEngine->ClearOverviews(targetSM.get());
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
    }

void ScalableMeshModel::LoadOverviews(IScalableMeshPtr& targetSM)
    {
    m_progressiveQueryEngine->InitScalableMesh(targetSM);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     3/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::SetActiveClipSets(bset<uint64_t>& activeClips, bset<uint64_t>& previouslyActiveClips)
    {
    if (!IsTerrain())
        return;

    m_activeClips = activeClips;
    bvector<uint64_t> clipIds;
    for (auto& clip: previouslyActiveClips)
       clipIds.push_back(clip);
    m_progressiveQueryEngine->SetActiveClips(activeClips, m_smPtr);
    m_progressiveQueryEngine->ClearCaching(clipIds, m_smPtr);

    if (m_smPtr->GetTerrainSM().IsValid())
        {
        m_progressiveQueryEngine->SetActiveClips(activeClips, m_smPtr->GetTerrainSM());
        m_progressiveQueryEngine->ClearCaching(clipIds, m_smPtr->GetTerrainSM());
        }
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

IMeshSpatialModelP ScalableMeshModelHandler::AttachTerrainModel(DgnDbR db, Utf8StringCR modelName, BeFileNameCR smFilename)
    {    
    /*    
    BeFileName smtFileName;
    GetScalableMeshTerrainFileName(smtFileName, db.GetFileName());
    
    if (!smtFileName.GetDirectoryName().DoesPathExist())
        BeFileName::CreateNewDirectory(smtFileName.GetDirectoryName().c_str());
        */
    Utf8String nameToSet = modelName;
    DgnClassId classId(db.Schemas().GetECClassId("ScalableMesh", "ScalableMeshModel"));
    BeAssert(classId.IsValid());        
         
    ScalableMeshTerrainModelAppData* appData(ScalableMeshTerrainModelAppData::Get(db));

    if (appData->m_smTerrainPhysicalModelP != nullptr)
        nameToSet = Utf8String(smFilename.GetFileNameWithoutExtension().c_str());

    RefCountedPtr<ScalableMeshModel> model(new ScalableMeshModel(DgnModel::CreateParams(db, classId, DgnModel::CreateModelCode(nameToSet))));

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
        DgnCode newModelCode(model->GetCode().GetAuthority(), nameToSet, NULL);
        model->SetCode(newModelCode);
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
void ScalableMeshModel::_WriteJsonProperties(Json::Value& v) const
    {
    T_Super::_WriteJsonProperties(v);
    m_properties.ToJson(v);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  03/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::_ReadJsonProperties(Json::Value const& v)
    {
    T_Super::_ReadJsonProperties(v);
    m_properties.FromJson(v);

    if (m_smPtr == 0 && !m_tryOpen)
    {
        //BeFileName smFileName(((this)->m_properties).m_fileId);
        BeFileName smFileName;
        T_HOST.GetPointCloudAdmin()._ResolveFileName(smFileName, (((this)->m_properties).m_fileId), GetDgnDb());

        if (BeFileName::DoesPathExist(smFileName.c_str()))
        {
            OpenFile(smFileName, GetDgnDb());
        }

        m_tryOpen = true;
    }
    }



HANDLER_DEFINE_MEMBERS(ScalableMeshModelHandler)