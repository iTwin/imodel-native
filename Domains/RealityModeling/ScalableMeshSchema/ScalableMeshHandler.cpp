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

#include <Bentley\BeDirectoryIterator.h>
#include <ScalableMesh/ScalableMeshLib.h>
#include <ScalableMesh\ScalableMeshUtilityFunctions.h>

#include <DgnPlatform\TextString.h>


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
// @bsimethod                                                 Mathieu.St-Pierre     3/2017
//----------------------------------------------------------------------------------------
IScalableMeshLocationProviderPtr ScalableMeshModel::m_locationProviderPtr = nullptr;

BentleyStatus IScalableMeshLocationProvider::GetExtraFileDirectory(BeFileNameR extraFileDir, DgnDbCR dgnDb) const
    {
    return _GetExtraFileDirectory(extraFileDir, dgnDb);
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
AxisAlignedBox3d ScalableMeshModel::_GetRange() const
    {
    AxisAlignedBox3d range;
    if (m_smPtr.IsValid()) 
        {
        m_smPtr->GetRange(range);
        m_smPtr->GetReprojectionTransform().Multiply(range, range);
        }
    return range;
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
     if (!IsTerrain())
        return SUCCESS;

    bvector<uint64_t> clipIds;
    clipIds.push_back(clipMaskElementId.GetValue());
    if (GetProgressiveQueryEngine().IsValid())
        GetProgressiveQueryEngine()->ClearCaching(clipIds, m_smPtr);
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
 //    if (!IsTerrain())
 //       return SUCCESS;

	if (!m_terrainParts.empty())
		for (auto& part : m_terrainParts)
			part->StartClipMaskBulkInsert();
    if (nullptr == m_smPtr.get()) return ERROR;
    m_isInsertingClips = true;
    m_startClipCount++;
    m_smPtr->SetIsInsertingClips(true);
    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_StopClipMaskBulkInsert()
    {
//     if (!IsTerrain())
//        return SUCCESS;

	if (!m_terrainParts.empty())
		for (auto& part : m_terrainParts)
			part->StopClipMaskBulkInsert();

    if (nullptr == m_smPtr.get()) return ERROR;
    m_startClipCount--;
    if (0 != m_startClipCount) return SUCCESS;
    m_isInsertingClips = false;
    m_smPtr->SetIsInsertingClips(false);

    SetActiveClipSets(m_activeClips, m_activeClips);
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

    DMatrix4d storageToUor; 
    storageToUor.InitFrom(m_smToModelUorTransform);    

    if (m_smPtr->GetGroup().IsValid() && !m_terrainParts.empty())
        return m_smPtr->GetGroup()->GetDTMInterface(storageToUor, type);
    return m_smPtr->GetDTMInterface(storageToUor, type);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Mathieu.St-Pierre     3/2017
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::SetLocationProvider(IScalableMeshLocationProvider& locationProvider)
    {
    m_locationProviderPtr = &locationProvider;
    return SUCCESS;
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

//For now keep the old value (i.e. : 1) for already support sources but set a higher level for streaming texture source because 1 seems a bit too low anyway.
static double s_maxPixelError = 1;
static double s_maxPixelErrorStreamingTexture = 2.5;

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
static Byte s_transparency = 0;
static bool s_applyClip = false;
static bool s_dontShowMesh = false;
static bool s_showTiles = false;
static double s_tileSizePerIdStringSize = 10.0;



void ProgressiveDrawMeshNode(bvector<IScalableMeshCachedDisplayNodePtr>& meshNodes,
                             bvector<IScalableMeshCachedDisplayNodePtr>& overviewMeshNodes,
                             ViewContextR                                context, 
                             const Transform&                            smToDgnUorTransform,
                             ScalableMeshDisplayCacheManager*            mgr, 
                             bset<uint64_t>&                             activeClips, 
                             bool                                        displayTexture)
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
      
    context.PushTransform(smToDgnUorTransform);

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
                matSymbP->SetLineColor(overviewMeshNodes[nodeInd]->IsTextured() && displayTexture ? white : green);
                matSymbP->SetFillColor(overviewMeshNodes[nodeInd]->IsTextured() && displayTexture ? white : green);
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
#ifndef NDEBUG
            if (s_showTiles)
                { 
                DRange3d contentExtent(meshNodes[nodeInd]->GetContentExtent());
                __int64  nodeId(meshNodes[nodeInd]->GetNodeId());

                TextString nodeIdString;

                DPoint3d stringOrigin = { (contentExtent.high.x + contentExtent.low.x) / 2, (contentExtent.high.y + contentExtent.low.y) / 2, contentExtent.high.z};

                char buffer[1000];
                BeStringUtilities::FormatUInt64(buffer, nodeId);
                
                nodeIdString.SetOrigin(stringOrigin);
                nodeIdString.SetText(buffer);

                TextStringStyle stringStyle;
                double maxExtentDim = std::max(contentExtent.XLength(), contentExtent.YLength());
                int textSize = std::max((int)(maxExtentDim / s_tileSizePerIdStringSize), 1);
                stringStyle.SetSize(textSize);
                nodeIdString.SetStyle(stringStyle);                

                ElemMatSymbP matSymbP = context.GetElemMatSymb();
                matSymbP->Init();                                
                matSymbP->SetLineColor(ColorDef::Red());
                matSymbP->SetFillColor(ColorDef::Red());                
                context.GetIDrawGeom().ActivateMatSymb(matSymbP);                                
                context.GetIDrawGeom().DrawTextString(nodeIdString);

                DPoint3d box[8];
                contentExtent.Get8Corners(box);                
                std::swap(box[6], box[7]);
                box[3] = box[7];
                
                context.GetIDrawGeom().DrawLineString3d(5, &box[3], nullptr);                
                }
#endif

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
                                                    matSymbP->SetLineColor(meshNodes[nodeInd]->IsTextured() != 0 && displayTexture ? white : green);
                                                    matSymbP->SetFillColor(meshNodes[nodeInd]->IsTextured() != 0 && displayTexture ? white : green);
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
                meshNodes[nodeInd]->GetSkirtMeshes(skirtMeshParts, activeClips);
                for (auto& part : skirtMeshParts)
                    context.GetIDrawGeom().DrawPolyface(*part);
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
    const Transform&                        m_smToDgnUorTransform;
    bool                                    m_hasFetchedFinalNode;
    bool                                    m_hasFetchedFinalTerrainNode;
    IScalableMeshDisplayCacheManager*       m_displayNodesCache;
    bset<uint64_t>                          m_activeClips;
    bool                                    m_displayTexture;


    ScalableMeshProgressiveDisplay (IScalableMeshProgressiveQueryEnginePtr& progressiveQueryEngine,
                                    ScalableMeshDrawingInfoPtr&             currentDrawingInfoPtr, 
                                    Transform&                              smToDgnUorTransform,
                                    IScalableMeshDisplayCacheManagerPtr&    cacheManager, 
                                    bset<uint64_t>                          activeClips, 
                                    bool                                    displayTexture)
    : m_smToDgnUorTransform(smToDgnUorTransform), 
      m_activeClips(activeClips)
        { 
        DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT

        m_progressiveQueryEngine = progressiveQueryEngine;
        m_currentDrawingInfoPtr = currentDrawingInfoPtr;        
        m_hasFetchedFinalNode = false;     
        m_displayNodesCache = cacheManager.get();                
        m_displayTexture = displayTexture;
        }

public:

    virtual bool _WantTimeoutSet(uint32_t& limit)   {return false; }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Mathieu.St-Pierre     02/2016
//----------------------------------------------------------------------------------------
virtual Completion _Process(ViewContextR viewContext) override
    {

    if ( m_hasFetchedFinalNode && !((ScalableMeshDisplayCacheManager*)m_displayNodesCache)->IsDirty())
        return Completion::Finished;

    Completion completionStatus = Completion::Aborted; 


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

    if (m_hasFetchedFinalNode)
        {
        completionStatus = Completion::HealRequired;
        }
    else    
    if (s_drawInProcess)
        {
        ProgressiveDrawMeshNode(m_currentDrawingInfoPtr->m_meshNodes, m_currentDrawingInfoPtr->m_overviewNodes, viewContext, m_smToDgnUorTransform, (ScalableMeshDisplayCacheManager*)m_displayNodesCache,  m_activeClips, m_displayTexture);
        }
            
    return completionStatus;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                               Mathieu.St-Pierre      02/2016
//----------------------------------------------------------------------------------------
static void Schedule (IScalableMeshProgressiveQueryEnginePtr& progressiveQueryEngine,
                      ScalableMeshDrawingInfoPtr&             currentDrawingInfoPtr, 
                      Transform&                              smToDgnUorTransform,
                      ViewContextR                            context,
                      IScalableMeshDisplayCacheManagerPtr&    cacheManager, 
                      bset<uint64_t>                          activeClips, 
                      bool                                    displayTexture)
    {
    RefCountedPtr<ScalableMeshProgressiveDisplay> progressiveDisplay(new ScalableMeshProgressiveDisplay(progressiveQueryEngine,
                                                                                                        currentDrawingInfoPtr,
                                                                                                        smToDgnUorTransform,
                                                                                                        cacheManager, 
                                                                                                        activeClips, 
                                                                                                        displayTexture));
    
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

static bool s_waitQueryComplete = false;



void GetBingLogoInfo(Transform& correctedViewToView, ViewContextR context)
    {
    DPoint3d lowRect;
    DPoint3d highRect;
    context.GetViewport()->GetViewCorners(lowRect, highRect);    

    DPoint2d nonPrintableMargin = { 0,0 };
    
    // CorrectedViewToView transform: adjust for swapped y and non-printable margin.
    if (lowRect.y > highRect.y)
        {
        correctedViewToView.InitFrom(nonPrintableMargin.x, lowRect.y - nonPrintableMargin.y, 0);
        correctedViewToView.form3d[1][1] = -1;
        }
    else
        {
        correctedViewToView.InitFrom(nonPrintableMargin.x, nonPrintableMargin.y, 0);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  08/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::DrawBingLogo(ViewContextR context, Byte const* pBitmapRGBA, DPoint2d const& bitmapSize)
    {
    if (NULL == pBitmapRGBA)
        return;

    // SetToViewCoords is only valid during overlay mode aka DecorateScreen
    //m_viewContext.GetViewport ()->GetIViewOutput ()->SetToViewCoords (true);

    DPoint2d bitmapDrawSize = { bitmapSize.x, bitmapSize.y };

    DPoint3d bitmapInView[4];
    bitmapInView[0].x = 0;
    bitmapInView[0].y = bitmapDrawSize.y;
    bitmapInView[1].x = bitmapDrawSize.x;
    bitmapInView[1].y = bitmapDrawSize.y;
    bitmapInView[2].x = 0;
    bitmapInView[2].y = 0;
    bitmapInView[3].x = bitmapDrawSize.x;
    bitmapInView[3].y = 0;
    bitmapInView[0].z = bitmapInView[1].z = bitmapInView[2].z = bitmapInView[3].z = 0;

    Transform correctedViewToView;
    
    GetBingLogoInfo(correctedViewToView, context);

    correctedViewToView.Multiply(bitmapInView, 4);

    DPoint3d bitmapInLocal[4];
    context.ViewToLocal(bitmapInLocal, bitmapInView, 4);

    bitmapInView[0].z = bitmapInView[1].z = bitmapInView[2].z = bitmapInView[3].z = 0/*context.GetViewport()->GetActiveZRoot()*/;

    // When raster is drawn by D3D, the texture is modulated by the active color (the materal fill color). 
    // Override the material fill color for raster to white so that the appearance won't mysteriously change in the future.
    ElemMatSymbP matSymb = context.GetElemMatSymb();

    ColorDef color(0xff, 0xff, 0xff, 0x01);    
    matSymb->SetLineColor(color);
    matSymb->SetFillColor(color);
    context.GetIDrawGeom().ActivateMatSymb(matSymb);

    //ok to call this here? m_viewContext.GetViewport ()->GetIViewOutput ()->ShowTransparent();
    //m_viewContext.GetIViewDraw()->SetSymbology (0x00FFFFFF, 0x00FFFFFF, 0, 0);
    context.GetIViewDraw().DrawRaster(bitmapInLocal, (int)(bitmapSize.x * 4), (int)bitmapSize.x, (int)bitmapSize.y, true, QV_RGBA_FORMAT, pBitmapRGBA, NULL);

    // SetToViewCoords is only valid during overlay mode aka DecorateScreen
    //m_viewContext.GetViewport ()->GetIViewOutput ()->SetToViewCoords (false);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  12/2016
//----------------------------------------------------------------------------------------
IScalableMeshProgressiveQueryEnginePtr ScalableMeshModel::GetProgressiveQueryEngine()
    {
    if (m_progressiveQueryEngine == nullptr)
        {
        m_displayNodesCache = new ScalableMeshDisplayCacheManager(GetDgnDb());
        if (!((ScalableMeshDisplayCacheManager*)m_displayNodesCache.get())->CanDisplay())
            {
            return nullptr;
            }
        m_progressiveQueryEngine = IScalableMeshProgressiveQueryEngine::Create(m_smPtr, m_displayNodesCache, m_displayTexture);
        }

    return m_progressiveQueryEngine;
    }

void DoPick(bvector<IScalableMeshCachedDisplayNodePtr>& meshNodes,
            bvector<IScalableMeshCachedDisplayNodePtr>& overviewMeshNodes, 
            ViewContextR                                viewContext, 
            const Transform&                            smToDgnUorTransform)
    {
    assert(DrawPurpose::Pick == viewContext.GetDrawPurpose());
    
        {
        viewContext.PushTransform(smToDgnUorTransform);
        IPickGeomP  pickGeom = viewContext.GetIPickGeom();
        DPoint3d pt;
        pickGeom->_GetPickPointView().GetProjectedXYZ(pt);
        viewContext.ViewToNpc(&pt, &pt, 1);
        DPoint3d endPt = pt;
        endPt.z = 1;
        pt.z = 0;

        viewContext.NpcToView(&pt, &pt, 1);
        viewContext.ViewToLocal(&pt, &pt, 1);

        viewContext.NpcToView(&endPt, &endPt, 1);
        viewContext.ViewToLocal(&endPt, &endPt, 1);

        DRay3d ray = DRay3d::FromOriginAndVector(pt, DVec3d::FromStartEndNormalize(pt, endPt));

        for (auto& node : meshNodes)
            {
            DRange3d nodeBox = node->GetContentExtent();
            double paramA, paramB;
            DPoint3d pointA, pointB;
            if (!nodeBox.IntersectRay(paramA, paramB, pointA, pointB, pt, ray.direction)) continue;
            bvector<SmCachedDisplayMesh*> cachedMeshes;
            bvector<bpair<bool, uint64_t>>  texIDs;

            if (SUCCESS == node->GetCachedMeshes(cachedMeshes, texIDs))
                {
                for (auto& cachedMesh : cachedMeshes)
                    {
                    QvElem* qvElem = 0;
                    if (cachedMesh != 0)
                        {
                        qvElem = cachedMesh->m_qvElem;
                        }
                    if (qvElem != 0)
                        {
                        viewContext.GetIViewDraw().DrawQvElem(qvElem);
                        }
                    }
                }
            }

        for (auto& node : overviewMeshNodes)
            {
            DRange3d nodeBox = node->GetContentExtent();
            double paramA, paramB;
            DPoint3d pointA, pointB;
            if (!nodeBox.IntersectRay(paramA, paramB, pointA, pointB, pt, ray.direction)) continue;
            bvector<SmCachedDisplayMesh*> cachedMeshes;
            bvector<bpair<bool, uint64_t>> texIDs;

            if (SUCCESS == node->GetCachedMeshes(cachedMeshes, texIDs))
                {
                for (auto& cachedMesh : cachedMeshes)
                    {
                    QvElem* qvElem = 0;
                    if (cachedMesh != 0)
                        {
                        qvElem = cachedMesh->m_qvElem;
                        }
                    if (qvElem != 0)
                        {
                        viewContext.GetIViewDraw().DrawQvElem(qvElem);
                        }
                    }
                }
            }
        viewContext.PopTransformClip();        
        return;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ScalableMeshModel::ClearAllDisplayMem()
    {
    if (!m_smPtr.IsValid())
        return;

    IScalableMeshProgressiveQueryEngine::CancelAllQueries();
    ClearProgressiveQueriesInfo();
    m_currentDrawingInfoPtr = nullptr;
    m_progressiveQueryEngine = nullptr;
    m_smPtr->RemoveAllDisplayData();    
    RefreshClips();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ScalableMeshModel::_AddGraphicsToScene(ViewContextR context)
    {       
    if (m_smPtr == 0 && !m_tryOpen)
        {
        //BeFileName smFileName(((this)->m_properties).m_fileId);        
        T_HOST.GetPointCloudAdmin()._ResolveFileName(m_path, (((this)->m_properties).m_fileId), GetDgnDb());

        if (BeFileName::DoesPathExist(m_path.c_str()) || BeFileName::IsUrl(m_path.c_str()))
            {
            OpenFile(m_path, GetDgnDb());
            }

        m_tryOpen = true;
        }

    if (m_smPtr == 0) return; //if open failed, we can't draw anything

    // BingMaps Brand logo for legal purpose.
    if (m_textureInfo->IsUsingBingMap() && m_textureInfo->IsTextureAvailable())
        {
        DPoint2d bitmapSize;
    
        const Byte* pBitmap = IScalableMeshTextureInfo::GetBingMapLogo(bitmapSize);

        if (NULL != pBitmap)
            DrawBingLogo(context, pBitmap, bitmapSize);
        }

    if (DrawPurpose::Pick == context.GetDrawPurpose() && m_currentDrawingInfoPtr.IsValid())
        {        
        DoPick(m_currentDrawingInfoPtr->m_meshNodes, m_currentDrawingInfoPtr->m_overviewNodes, context, m_smToModelUorTransform);
        }

    //On first draw, we make sure all models know which of the rendered models "belong" to each other and set the groups accordingly.
    if (!m_loadedAllModels)
        {
        InitializeTerrainRegions(context);
        }
    

    if (!ShouldDrawInContext(context) || NULL == context.GetViewport() || !m_smPtr.IsValid() || !GetProgressiveQueryEngine().IsValid())
        return;
         
    ScalableMeshDrawingInfoPtr nextDrawingInfoPtr(new ScalableMeshDrawingInfo(&context));
    nextDrawingInfoPtr->m_smPtr = m_smPtr.get();
    nextDrawingInfoPtr->m_currentQuery = (int)((GetModelId().GetValue() - GetModelId().GetBriefcaseId().GetValue()) & 0xFFFF);

    if ((m_currentDrawingInfoPtr != nullptr) &&
        (m_currentDrawingInfoPtr->GetDrawPurpose() != DrawPurpose::UpdateDynamic))
        {
        //If the m_dtmPtr equals 0 it could mean that the last data request to the STM was cancelled, so start a new request even
        //if the view has not changed.
        if (m_currentDrawingInfoPtr->HasAppearanceChanged(nextDrawingInfoPtr) == false && !m_forceRedraw)                
            {
            //assert((m_currentDrawingInfoPtr->m_overviewNodes.size() == 0) && (m_currentDrawingInfoPtr->m_meshNodes.size() > 0));

            ProgressiveDrawMeshNode(m_currentDrawingInfoPtr->m_meshNodes, m_currentDrawingInfoPtr->m_overviewNodes, context, m_smToModelUorTransform, (ScalableMeshDisplayCacheManager*)m_displayNodesCache.get(), m_smPtr->ShouldInvertClips() ? m_notActiveClips : m_activeClips, m_displayTexture);
            
            return;                        
            }   
        }        

    bool restartQuery = true;

    if ((m_currentDrawingInfoPtr != nullptr) && m_currentDrawingInfoPtr->HasAppearanceChanged(nextDrawingInfoPtr) == false && !m_forceRedraw)
        { 
        restartQuery = false;
        }
        
    BentleyStatus status;

    if (restartQuery)
        {
        status = GetProgressiveQueryEngine()->StopQuery(/*nextDrawingInfoPtr->GetViewNumber()*/nextDrawingInfoPtr->m_currentQuery);
        assert(status == SUCCESS);
        }

    m_forceRedraw = false;                                   
    m_currentDrawingInfoPtr = nextDrawingInfoPtr;
    int queryId = nextDrawingInfoPtr->m_currentQuery;

    // Need to get the fence info.
    /*
    DTMDrawingInfo drawingInfo;
    DTMElementDisplayHandler::GetDTMDrawingInfo(drawingInfo, m_DTMDataRef->GetElement(), m_DTMDataRef, context);

    if (!drawingInfo.IsVisible ())
        {
        return;
        }
        */    
    
    if (restartQuery)
        {
        DMatrix4d localToView(context.GetLocalToView());
                                   
        DMatrix4d smToUOR = DMatrix4d::From(m_smToModelUorTransform);

        bsiDMatrix4d_multiply(&localToView, &localToView, &smToUOR);

        //DPoint3d viewBox[8];

        //NEEDS_WORK_SM : Remove from query
        //GetViewBoxFromContext(viewBox, _countof(viewBox), context, drawingInfo);        
        DMatrix4d rootToStorage;

        //Convert the view box in storage.
        bool inverted = bsiDMatrix4d_invertQR(&rootToStorage, &m_storageToUorsTransfo);

        BeAssert(inverted != 0);
    
        status = SUCCESS;
            
        IScalableMeshViewDependentMeshQueryParamsPtr viewDependentQueryParams(IScalableMeshViewDependentMeshQueryParams::CreateParams());

        viewDependentQueryParams->SetMinScreenPixelsPerPoint(s_minScreenPixelsPerPoint);
                
        if (m_textureInfo->IsUsingBingMap())
            {
            viewDependentQueryParams->SetMaxPixelError(s_maxPixelErrorStreamingTexture);
            }
        else
            {
            viewDependentQueryParams->SetMaxPixelError(s_maxPixelError);
            }
            
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
        clipVectorCopy->TransformInPlace(m_modelUorToSmTransform);
    

        
        viewDependentQueryParams->SetViewClipVector(clipVectorCopy);
                          
        m_currentDrawingInfoPtr->m_overviewNodes.clear();
        queryId = (int)((GetModelId().GetValue() - GetModelId().GetBriefcaseId().GetValue()) & 0xFFFF);//nextDrawingInfoPtr->GetViewNumber();                 
        m_currentDrawingInfoPtr->m_currentQuery = queryId;
        bvector<bool> clips;
        /*NEEDS_WORK_SM : Get clips
        m_DTMDataRef->GetVisibleClips(clips);
        */

        status = GetProgressiveQueryEngine()->StartQuery(queryId,
                                                          viewDependentQueryParams, 
                                                          m_currentDrawingInfoPtr->m_meshNodes, 
                                                          m_displayTexture, //No wireframe mode, so always load the texture.
                                                          clips,
                                                          m_smPtr); 


        assert(status == SUCCESS);
        }

    if (s_waitQueryComplete || !m_isProgressiveDisplayOn)
        {
        while (!GetProgressiveQueryEngine()->IsQueryComplete(queryId))
            {
            BeThreadUtilities::BeSleep (200);
            }
        }

   // int terrainQueryId = -1;    
    //auto terrainSM = m_smPtr->GetTerrainSM();
    
 /*   if (!clipFromCoverageSet.empty() && terrainSM.IsValid())
        {
        m_currentDrawingInfoPtr->m_terrainOverviewNodes.clear();
        terrainQueryId = (int)((GetModelId().GetValue() - GetModelId().GetBriefcaseId().GetValue()) & 0xFFFFFFFF | 0xAFFF);//nextDrawingInfoPtr->GetViewNumber();                 
        m_currentDrawingInfoPtr->m_terrainQuery = terrainQueryId;
        bvector<bool> clips;
             
        status = GetProgressiveQueryEngine()->StartQuery(terrainQueryId,
                                                      viewDependentQueryParams,
                                                      m_currentDrawingInfoPtr->m_terrainMeshNodes,
                                                      true, //No wireframe mode, so always load the texture.
                                                      clips,
                                                     terrainSM);

        if (!m_isProgressiveDisplayOn)
            {
            while (!GetProgressiveQueryEngine()->IsQueryComplete(terrainQueryId))
                {
                BeThreadUtilities::BeSleep (200);
                }
            }
        }*/

    bool needProgressive;


    if (GetProgressiveQueryEngine()->IsQueryComplete(queryId))
        {        
        m_currentDrawingInfoPtr->m_meshNodes.clear();
        status = GetProgressiveQueryEngine()->GetRequiredNodes(m_currentDrawingInfoPtr->m_meshNodes, queryId);
        assert(status == SUCCESS);
        m_currentDrawingInfoPtr->m_overviewNodes.clear();
       
        bvector<IScalableMeshNodePtr> nodes;
        for (auto& nodeP : m_currentDrawingInfoPtr->m_meshNodes) nodes.push_back(nodeP.get());
        m_smPtr->SetCurrentlyViewedNodes(nodes);

                        
        needProgressive = false;        
        }
    else
        {  
        status = GetProgressiveQueryEngine()->GetOverviewNodes(m_currentDrawingInfoPtr->m_overviewNodes, queryId);

        m_currentDrawingInfoPtr->m_meshNodes.clear();

        status = GetProgressiveQueryEngine()->GetRequiredNodes(m_currentDrawingInfoPtr->m_meshNodes, queryId);
		bvector<IScalableMeshNodePtr> nodes;
		for (auto& nodeP : m_currentDrawingInfoPtr->m_meshNodes) nodes.push_back(nodeP.get());
		for (auto& nodeP : m_currentDrawingInfoPtr->m_overviewNodes) nodes.push_back(nodeP.get());
		m_smPtr->SetCurrentlyViewedNodes(nodes);
        assert(status == SUCCESS);
        
        //NEEDS_WORK_MST : Will be fixed when the lowest resolution is created and pin at creation time.
        //assert(m_currentDrawingInfoPtr->m_overviewNodes.size() > 0);
        assert(status == SUCCESS);

        needProgressive = true;
        }                         


    ProgressiveDrawMeshNode(m_currentDrawingInfoPtr->m_meshNodes, m_currentDrawingInfoPtr->m_overviewNodes, context, m_smToModelUorTransform, (ScalableMeshDisplayCacheManager*)m_displayNodesCache.get(), m_smPtr->ShouldInvertClips() ? m_notActiveClips : m_activeClips, m_displayTexture);


    if (needProgressive)
        {
        IScalableMeshProgressiveQueryEnginePtr queryEnginePtr(GetProgressiveQueryEngine());        
        ScalableMeshProgressiveDisplay::Schedule(queryEnginePtr, m_currentDrawingInfoPtr, m_smToModelUorTransform, context, m_displayNodesCache, m_smPtr->ShouldInvertClips() ? m_notActiveClips :m_activeClips, m_displayTexture);
        }    
    }                 

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ScalableMeshModel::GetAllScalableMeshes(BentleyApi::Dgn::DgnDbCR dgnDb, bvector<IMeshSpatialModelP>& models)
    {
    DgnClassId classId(dgnDb.Schemas().GetECClassId("ScalableMesh", "ScalableMeshModel"));
    BeAssert(classId.IsValid());

    for (auto& model : dgnDb.Models().GetLoadedModels())
        {
        if (model.second->GetClassId() == classId) models.push_back(dynamic_cast<IMeshSpatialModelP>(model.second.get()));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ScalableMeshModel::GetScalableMeshTypes(BentleyApi::Dgn::DgnDbCR dgnDb, bool& has3D, bool& hasTerrain, bool& hasExtractedTerrain, bool& hasCesium3DTiles)
    {    
    has3D = false; 
    hasTerrain = false;
    hasExtractedTerrain = false;
    hasCesium3DTiles = false;

    bvector<IMeshSpatialModelP> smModels;

    GetAllScalableMeshes(dgnDb, smModels);

    for (auto& smModel : smModels)
        {
        ScalableMeshModel* scalableMeshModel = (ScalableMeshModel*)smModel;

        if (!scalableMeshModel->IsTerrain())
            {
            IScalableMesh* sm = scalableMeshModel->GetScalableMesh();

            if (sm != nullptr)
                {
                bvector<uint64_t> ids;
                sm->GetCoverageIds(ids);
                if (ids.size() > 0) hasExtractedTerrain = true;

                if (sm->IsCesium3DTiles()) hasCesium3DTiles = true;
                }

            has3D = true;
            }
        else
            {
            hasTerrain = true;
            }
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
    m_isInsertingClips = false;
    m_subModel = false;
    m_loadedAllModels = false;
    m_startClipCount = 0;
    
    m_displayTexture = true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
ScalableMeshModel::~ScalableMeshModel()
    {
    Cleanup(false);
    }

void ScalableMeshModel::Cleanup(bool isModelDelete)
    {
    if (nullptr != m_progressiveQueryEngine.get() && nullptr != m_currentDrawingInfoPtr.get()) m_progressiveQueryEngine->StopQuery(m_currentDrawingInfoPtr->m_currentQuery);
    if (nullptr != m_currentDrawingInfoPtr.get())
    {
        m_currentDrawingInfoPtr->m_meshNodes.clear();
        m_currentDrawingInfoPtr->m_overviewNodes.clear();
    }

    ScalableMeshTerrainModelAppData* appData(ScalableMeshTerrainModelAppData::Get(GetDgnDb()));
    if (appData != nullptr && appData->m_smTerrainPhysicalModelP == this)
        ScalableMeshTerrainModelAppData::Delete(GetDgnDb());
    ClearProgressiveQueriesInfo();

    if (m_smPtr.IsValid())
        {                
        bvector<BeFileName> extraFileNames;

        if (isModelDelete)
            m_smPtr->GetExtraFileNames(extraFileNames);

        //Close the 3SM file, to close extra clip files.
		m_currentDrawingInfoPtr = nullptr;
		m_progressiveQueryEngine = nullptr;        
        m_smPtr = nullptr;        

        for (auto& extraFileName : extraFileNames)
            {
            _wremove(extraFileName.c_str());
            }        
        }    
    }


BeFileName ScalableMeshModel::GenerateClipFileName(BeFileNameCR smFilename, DgnDbR dgnProject)
    {
    BeFileName clipFileBase;
    BeFileName extraFileDir;

    if (m_locationProviderPtr.IsValid() && SUCCESS == m_locationProviderPtr->GetExtraFileDirectory(extraFileDir, dgnProject))
        {
        clipFileBase = extraFileDir;        
        }
    else
        {
        clipFileBase = BeFileName(ScalableMeshModel::GetTerrainModelPath(dgnProject)).GetDirectoryName();
        }
        
    WChar modelIdStr[1000];
    BeStringUtilities::FormatUInt64(modelIdStr, GetModelId().GetValue());    
    clipFileBase.AppendToPath(modelIdStr);
    return clipFileBase;
    }


void ScalableMeshModel::ClearExtraFiles()
{
	BeFileName clipFileBase = GenerateClipFileName(m_path, GetDgnDb());

	bvector<BeFileName> names;
	BeFileName clipFileName = clipFileBase;
	clipFileName.append(L"_clips");

	BeFileName clipDefFileName = clipFileBase;
	clipDefFileName.append(L"_clipDefinitions");

	names.push_back(clipFileName);
	names.push_back(clipDefFileName);
	for (auto& fileName : names)
	{
		if (BeFileName::DoesPathExist(fileName.c_str()))
		{
			BeFileName::BeDeleteFile(fileName.c_str());
		}
	}
}

void ScalableMeshModel::CompactExtraFiles()
{
	if (!m_smPtr.IsValid())
		return;

	m_smPtr->CompactExtraFiles();
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
        {
        if (sm != this) nOfOtherModels++;
        }


    allScalableMeshes.clear();    

    BeFileName clipFileBase = GenerateClipFileName(smFilename, dgnProject);

    m_basePath = clipFileBase;
    m_smPtr = IScalableMesh::GetFor(smFilename, Utf8String(clipFileBase.c_str()), false, true);

    if (!m_smPtr.IsValid())
        return;    

#if SM_ACTIVATE_UPLOADER == 1 || SM_ACTIVATE_LOAD_TEST == 1
    WString projectName = dgnProject.GetFileName().GetFileNameWithoutExtension();
#endif

#if SM_ACTIVATE_UPLOADER == 1
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
        else if (projectName.Equals(WString(L"upload_to_cloud_local_curl")))
            {
            WString container(L"scalablemeshtest"); // local disk container
            m_smPtr->ConvertToCloud(container, smFilename.GetFileNameWithoutExtension(), SMCloudServerType::LocalDiskCURL);
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
    if (projectName.Contains(WString(L"load_test")))
        {
        size_t nbLoadedNodes = 0;
        m_smPtr->LoadAllNodeData(nbLoadedNodes, 6);
        }
#endif

    //if (m_smPtr->IsTerrain())
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

    DgnGCSPtr projGCS = dgnProject.Units().GetDgnGCS();
    
    if (gcs.HasGeoRef())
        {
        DgnGCSPtr dgnGcsPtr(DgnGCS::CreateGCS(gcs.GetGeoRef().GetBasePtr().get(), dgnProject));        
        dgnGcsPtr->UorsFromCartesian(scale, scale);
        
        if (projGCS.IsValid() && !projGCS->IsEquivalent(*dgnGcsPtr))
            {
            dgnGcsPtr->SetReprojectElevation(true);            

            DRange3d smExtent, smExtentUors;
            m_smPtr->GetRange(smExtent);
            Transform trans;
            trans.InitFromScaleFactors(scale.x, scale.y, scale.z);
            trans.Multiply(smExtentUors, smExtent);

            DPoint3d extent;
            extent.DifferenceOf(smExtentUors.high, smExtentUors.low);
            Transform       approxTransform;

			auto coordInterp = m_smPtr->IsCesium3DTiles() ? GeoCoordInterpretation::XYZ : GeoCoordInterpretation::Cartesian;

            StatusInt status = dgnGcsPtr->GetLocalTransform(&approxTransform, smExtentUors.low, &extent, true/*doRotate*/, true/*doScale*/, coordInterp, *projGCS);
            if (0 == status || 1 == status || 25 == status)
                {                
                DRange3d smExtentInDestGCS1;
                approxTransform.Multiply(smExtentInDestGCS1, smExtentUors);                
                m_smToModelUorTransform = Transform::FromProduct(approxTransform, trans);

                DRange3d smExtentInDestGCS;
                m_smToModelUorTransform.Multiply(smExtentInDestGCS, smExtent);                
                }
            else
                { 
                m_smToModelUorTransform = Transform::FromScaleFactors(scale.x, scale.y, scale.z);
                }
            }
        else
            {            
            m_smToModelUorTransform = Transform::FromScaleFactors(scale.x, scale.y, scale.z);
            }
        }
    else
        {                        
        dgnProject.Units().GetDgnGCS()->UorsFromCartesian(scale, scale);
        assert(scale.x == 1 && scale.y == 1 && scale.z == 1);
        m_smToModelUorTransform = Transform::FromScaleFactors(scale.x, scale.y, scale.z);
        }
           
    m_smPtr->SetReprojection(*projGCS, m_smToModelUorTransform);

    DPoint3d translation = {0,0,0};
    
    m_storageToUorsTransfo = DMatrix4d::FromScaleAndTranslation(scale, translation);                    

    bool invertResult = m_modelUorToSmTransform.InverseOf(m_smToModelUorTransform);
    assert(invertResult);
    
    if (m_smPtr->IsCesium3DTiles() && !(smFilename.ContainsI(L"realitydataservices") && smFilename.ContainsI(L"S3MXECPlugin")))
        {
        // The mesh likely comes from ProjectWiseContextShare, if it does then save that instead
        auto pwcsLink = BeFileName(m_smPtr->GetProjectWiseContextShareLink().c_str());
        if (!pwcsLink.empty()) m_path = pwcsLink;
        }

    // NEEDS_WORK_SM
    BeFileName dbFileName(dgnProject.GetDbFileName());
    BeFileName basePath = dbFileName.GetDirectoryName();
    T_HOST.GetPointCloudAdmin()._CreateLocalFileId(m_properties.m_fileId, m_path, basePath);
  
    StatusInt result = m_smPtr->GetTextureInfo(m_textureInfo);

    assert(result == SUCCESS);

    if (!m_textureInfo->IsTextureAvailable())
        {
        SetDisplayTexture(false);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::CloseFile()
    {
	if (m_subModel)
	{
		m_loadedAllModels = false;
	}
    if (nullptr != m_progressiveQueryEngine.get() && nullptr != m_currentDrawingInfoPtr.get()) m_progressiveQueryEngine->StopQuery(m_currentDrawingInfoPtr->m_currentQuery);
    if (nullptr != m_currentDrawingInfoPtr.get())
        {
        m_currentDrawingInfoPtr->m_meshNodes.clear();
        m_currentDrawingInfoPtr->m_overviewNodes.clear();
        m_currentDrawingInfoPtr->m_smPtr = nullptr;
        }

    m_progressiveQueryEngine = nullptr;    
    m_smPtr = nullptr;
    m_displayNodesCache = nullptr;
    m_tryOpen = false;

    //Ensure the file has really been closed.
    assert(ScalableMeshLib::GetHost().GetRegisteredScalableMesh(m_path) == nullptr);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ScalableMeshModel::UpdateFilename (BeFileNameCR newFilename)
    {    
    if (!BeFileName::DoesPathExist(newFilename))
        return ERROR;
    
    BeFileName dbFileName(m_dgndb.GetDbFileName());
    BeFileName basePath = dbFileName.GetDirectoryName();
    T_HOST.GetPointCloudAdmin()._CreateLocalFileId(m_properties.m_fileId, newFilename, basePath);
    OpenFile(newFilename, GetDgnDb());
    m_tryOpen = true;

    Update();

    // file will be open when required
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ScalableMeshModel::UpdateExtractedTerrainLocation(BeFileNameCR oldLocation, BeFileNameCR newLocation)
    {
    assert(m_tryOpen == true);

    if (m_smPtr == nullptr)
        return ERROR;

    bvector<IMeshSpatialModelP> allScalableMeshes;
    ScalableMeshModel::GetAllScalableMeshes(GetDgnDb(), allScalableMeshes);
    
    bvector<uint64_t> coverageIds;
    m_smPtr->GetCoverageIds(coverageIds);

    for (auto& pMeshModel : allScalableMeshes)
        {                                 
        ScalableMeshModelP pScalableMesh = ((ScalableMeshModelP)pMeshModel);
        if (this == pScalableMesh)
            continue;        
                            
        for (uint64_t coverageId : coverageIds)
            {
            BeFileName terrainPath;
            GetPathForTerrainRegion(terrainPath, coverageId, oldLocation);

            if (pScalableMesh->GetPath().CompareToI(terrainPath) == 0)
                {                    
                BeFileName newFileName(newLocation); 
                newFileName.AppendString(pScalableMesh->GetPath().GetFileNameAndExtension().c_str());
                pScalableMesh->CloseFile();
                pScalableMesh->UpdateFilename(newFileName);                                                            
                }                
            }
        }

    return SUCCESS;
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
// @bsimethod                                                 Elenie.Godzaridis     4/2017
//----------------------------------------------------------------------------------------
ScalableMeshModelP ScalableMeshModel::CreateModel(BentleyApi::Dgn::DgnDbR dgnDb, WString terrainName, BeFileName terrainPath)
{
    DgnClassId classId(dgnDb.Schemas().GetECClassId("ScalableMesh", "ScalableMeshModel"));
    BeAssert(classId.IsValid());

    ScalableMeshModelP model = new ScalableMeshModel(DgnModel::CreateParams(dgnDb, classId, DgnModel::CreateModelCode(Utf8String(terrainName.c_str()).c_str())));

    model->Insert();
    model->OpenFile(terrainPath, dgnDb);
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
IScalableMesh* ScalableMeshModel::GetScalableMesh(bool wantGroup)
    {
    if (m_smPtr.IsNull())
        return NULL;

    if (m_smPtr->GetGroup().IsValid() && !m_terrainParts.empty() && wantGroup)
        return m_smPtr->GetGroup().get();
    return m_smPtr.get();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/201
//----------------------------------------------------------------------------------------
IScalableMesh* ScalableMeshModel::GetScalableMeshHandle()
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
    
    bset<uint64_t> clips = activeClips;

    m_activeClips = clips;

    if (m_isInsertingClips) return;

    bvector<uint64_t> clipIds;
    for (auto& clip: previouslyActiveClips)
       clipIds.push_back(clip);

    auto tryProgressiveQueryEngine = GetProgressiveQueryEngine();
    if (tryProgressiveQueryEngine.get() == nullptr) return;
    GetProgressiveQueryEngine()->SetActiveClips(clips, m_smPtr);
    GetProgressiveQueryEngine()->ClearCaching(clipIds, m_smPtr);
    
    m_forceRedraw = true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::ActivateClip(uint64_t clipId, ClipMode mode)
    {
    m_currentClips[clipId] = make_bpair(mode, true);
    RefreshClips();

    if (!m_terrainParts.empty())
        {
        for (auto& model : m_terrainParts)
            model->ActivateClip(clipId, mode);
        }

    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::DeactivateClip(uint64_t clipId)
    {
    m_currentClips[clipId].second = false;
    RefreshClips();
    if (!m_terrainParts.empty())
        {
        for (auto& model : m_terrainParts)
            model->DeactivateClip(clipId);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::RefreshClips()
    {
    if (!m_smPtr.IsValid())
        return;

    bset<uint64_t> toActivate;
    bset<uint64_t> notActivated;
    for (auto& it : m_currentClips)
        {
        if (it.second.first == ClipMode::Mask && it.second.second != m_smPtr->ShouldInvertClips())
            {
              toActivate.insert(it.first);
            }
        else notActivated.insert(it.first);

        if (it.second.first == ClipMode::Clip && it.second.second && m_smPtr->ShouldInvertClips())
            toActivate.insert(it.first);
        else notActivated.insert(it.first);

        }

    m_notActiveClips = notActivated;
    SetActiveClipSets(toActivate, toActivate);
    }

bool ScalableMeshModel::HasClipBoundary(const bvector<DPoint3d>& clipBoundary, uint64_t clipID)
{
	bvector<DPoint3d> data;
	SMNonDestructiveClipType type;
	m_smPtr->GetClipType(clipID, type);
	if (type != SMNonDestructiveClipType::Boundary)
		return false;

	if (!m_smPtr->GetClip(clipID, data))
		return false;

	CurveVectorPtr curveP = CurveVector::CreateLinear(data);
	CurveVectorPtr testP = CurveVector::CreateLinear(clipBoundary);
   
	return curveP->IsSameStructureAndGeometry(*testP, 1e-5);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ScalableMeshModel::InitializeTerrainRegions(ViewContextR context)
    {
	bvector<uint64_t> allClips;


	GetClipSetIds(allClips);
	for (auto elem : allClips)
	{
		SMNonDestructiveClipType type;
		m_smPtr->GetClipType(elem, type);

		if(type == SMNonDestructiveClipType::Boundary)
			m_smPtr->SetInvertClip(true);
	}

    bvector<IMeshSpatialModelP> allScalableMeshes;
    ScalableMeshModel::GetAllScalableMeshes(GetDgnDb(), allScalableMeshes);
    if (!m_subModel)
        SetDefaultClipsActive();


    bvector<uint64_t> coverageIds;
    m_smPtr->GetCoverageIds(coverageIds);

    for (auto& pMeshModel : allScalableMeshes)
        {                
        ScalableMeshModelP pScalableMesh = ((ScalableMeshModelP)pMeshModel);
        if (this == pScalableMesh)
            continue;        

        for (uint64_t coverageId : coverageIds)
            {
           // BeFileName terrainPath;

           // GetPathForTerrainRegion(terrainPath, coverageId, m_basePath);
			bvector<DPoint3d> regionData;
			if (!m_smPtr->GetClip(coverageId, regionData)) continue;

            if (nullptr != pScalableMesh->GetScalableMesh(false) && pScalableMesh->HasClipBoundary(regionData, coverageId)/*&& pScalableMesh->GetPath().CompareToI(terrainPath) == 0*/)
                {                                            
                 AddTerrainRegion(coverageId, pScalableMesh, regionData);
                break;
                }
            }                      

        if (nullptr != context.GetViewport())
            {
            bool isDisplayed = context.GetViewport()->GetViewController().IsModelViewed(pScalableMesh->GetModelId());
            SetRegionVisibility(pScalableMesh->GetAssociatedRegionId(), isDisplayed);
            }
        }

    m_loadedAllModels = true;

    ScalableMeshTerrainModelAppData* appData = ScalableMeshTerrainModelAppData::Get(m_dgndb);
    if (((ScalableMeshModelP)appData->m_smTerrainPhysicalModelP == nullptr || (((ScalableMeshModelP)appData->m_smTerrainPhysicalModelP)->m_subModel == true && !m_subModel)) && (m_smPtr->IsTerrain() || !m_terrainParts.empty()))
        {
        appData->m_smTerrainPhysicalModelP = this;
        appData->m_modelSearched = true;
        }


    for (auto& smP : m_terrainParts)
        {
        for (auto& id : coverageIds)
            smP->ActivateClip(id, ClipMode::Clip);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::AddTerrainRegion(uint64_t id, ScalableMeshModel* terrainModel, const bvector<DPoint3d> region)
    {
    m_terrainParts.push_back(terrainModel);
    terrainModel->m_subModel = true;
    terrainModel->m_parentModel = this;
    terrainModel->m_associatedRegion = id;

    IScalableMeshPtr smPtr = terrainModel->GetScalableMesh();
    if (!m_smPtr->GetGroup().IsValid()) m_smPtr->AddToGroup(m_smPtr, false);
    m_smPtr->AddToGroup(smPtr, true, region.data(), region.size());
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::FindTerrainRegion(uint64_t id, ScalableMeshModel*& terrainModel)
    {
    for (auto& part: m_terrainParts)
        if (part->m_associatedRegion == id)
            {
            terrainModel = part;
            return;
            }

    terrainModel = nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     3/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::SetRegionVisibility(uint64_t id, bool isVisible)
{
    //clip or un-clip the 3d 3sm
    m_currentClips[id].second = isVisible;
    RefreshClips();
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::RemoveRegion(uint64_t id)
    {
    IScalableMeshPtr smPtr;
    bvector<ScalableMeshModel*>::iterator toDelete = m_terrainParts.end();
    for (auto it = m_terrainParts.begin(); it != m_terrainParts.end(); ++it)
        if ((*it)->m_associatedRegion == id)
            {
            smPtr = (*it)->GetScalableMeshHandle();
            toDelete = it;
            }

    if (smPtr.IsValid())
        m_smPtr->RemoveFromGroup(smPtr);

    if (toDelete != m_terrainParts.end())
        m_terrainParts.erase(toDelete);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::GetPathForTerrainRegion(BeFileNameR terrainName, uint64_t id, const WString& basePath)
    {
    assert(m_smPtr.IsValid());

    Utf8String coverageName;
    GetScalableMesh(false)->GetCoverageName(coverageName, id);
    GetCoverageTerrainAbsFileName(terrainName, basePath, coverageName);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2017
//----------------------------------------------------------------------------------------
bool ScalableMeshModel::HasQueuedTerrainRegions()
    {
    return !m_queuedRegions.empty();
    }

void LoadAllScalableMeshModels(DgnDbCR database, Utf8CP label)
    {
    DgnClassId classId(database.Schemas().GetECClassId("ScalableMesh", "ScalableMeshModel"));
    auto modelList = database.Models().MakeIterator();

    bvector<DgnModelId> modelsToLoad;
    for (auto& model : modelList)
        {
        if (model.GetClassId() == classId && model.GetLabel() != label)
            {
            modelsToLoad.push_back(model.GetModelId());
            }
        }
    for (auto& id : modelsToLoad)
        if (!database.Models().FindModel(id).IsValid()) database.Models().GetModel(id);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::SyncTerrainRegions(bvector<uint64_t>& newModelIds)
    {
    LoadAllScalableMeshModels(GetDgnDb(), GetLabel());
    for (auto& reg : m_queuedRegions)
        {
        if (reg.regionData.empty())
            {
                RemoveRegion(reg.id);      
            }
        else
            {
            BeFileName terrainPath;            
            bvector<IMeshSpatialModelP> allScalableMeshes;
            ScalableMeshModel::GetAllScalableMeshes(GetDgnDb(), allScalableMeshes);
            ScalableMeshModelP terrainRegion = nullptr;
            for (auto& sm : allScalableMeshes)
                {                        
                if (sm == this || dynamic_cast<ScalableMeshModel*>(sm)->GetScalableMesh(false) == nullptr) continue;
                
                GetPathForTerrainRegion(terrainPath, reg.id, m_basePath);

                if (dynamic_cast<ScalableMeshModel*>(sm)->GetPath().CompareToI(terrainPath) == 0)
                    {                    
                    terrainRegion = dynamic_cast<ScalableMeshModel*>(sm);
                    }
                }

            if (terrainRegion == nullptr) continue;

            IScalableMeshPtr sm = terrainRegion->GetScalableMesh();
            terrainRegion->LoadOverviews(sm);
            ActivateClip(reg.id);
            DgnElementId id = DgnElementId(reg.id);
            ReloadClipMask(id, true);

            terrainRegion->GetScalableMesh()->AddClip(reg.regionData.data(), reg.regionData.size(), reg.id, SMClipGeometryType::Polygon, SMNonDestructiveClipType::Boundary, true);
            sm->SetInvertClip(true);
            terrainRegion->ActivateClip(reg.id, ClipMode::Clip);

            AddTerrainRegion(reg.id, terrainRegion, reg.regionData);
            newModelIds.push_back(terrainRegion->GetModelId().GetValue());
            }
        }
    m_queuedRegions.clear();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::QueueDeleteTerrainRegions(uint64_t id)
    {
    QueuedRegionOp reg;
    reg.id = id;
    m_queuedRegions.push_back(reg);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::QueueAddTerrainRegions(uint64_t id, const bvector<DPoint3d>& boundary)
    {
    QueuedRegionOp reg;
    reg.id = id;
    reg.regionData = boundary;
    m_queuedRegions.push_back(reg);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::CreateBreaklines(const BeFileName& extraLinearFeatureAbsFileName, bvector<DSegment3d> const& breaklines)
    {
    TerrainModel::DTMPtr dtm(GetDTM(DTMAnalysisType::RawDataOnly));

    TerrainModel::BcDTMPtr bcDtmPtr(TerrainModel::BcDTM::Create());
    TerrainModel::DTMDrapedLinePtr drapedLine;
    TerrainModel::IDTMDraping* draping = dtm->GetDTMDraping();
    bool hasAddedBreaklines = false;

    for (size_t segmentInd = 0; segmentInd < breaklines.size() - 1; segmentInd++)
        {

        DTMStatusInt status = draping->DrapeLinear(drapedLine, breaklines[segmentInd].point, 2);
        assert(status == DTMStatusInt::DTM_SUCCESS);

        bvector<DPoint3d> breaklinePts;

        for (size_t ptInd = 0; ptInd < drapedLine->GetPointCount(); ptInd++)
            {
            DPoint3d pt;
            double distance;
            DTMDrapedLineCode code;

            DTMStatusInt status = drapedLine->GetPointByIndex(pt, &distance, &code, (int)ptInd);
            assert(status == SUCCESS);
            breaklinePts.push_back(pt);
            }

        if (breaklinePts.size() == 0)
            continue;

        DTMFeatureId featureId;

        status = bcDtmPtr->AddLinearFeature(DTMFeatureType::Breakline, &breaklinePts[0], (int)breaklinePts.size(), &featureId);
        assert(status == DTMStatusInt::DTM_SUCCESS);
        hasAddedBreaklines = true;
        }

    if (hasAddedBreaklines)
        {
        DTMStatusInt status = bcDtmPtr->SaveAsGeopakDat(extraLinearFeatureAbsFileName.c_str());
        assert(status == DTMStatusInt::DTM_SUCCESS);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::SetDefaultClipsActive()
    {

        bvector<uint64_t> allClips;


        _StartClipMaskBulkInsert();
        GetClipSetIds(allClips);
        for (auto elem : allClips)
            {
            ActivateClip(elem);
            if (m_smPtr->ShouldInvertClips())
                DeactivateClip(elem);
            }
        _StopClipMaskBulkInsert();
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
// @bsimethod                                                 Elenie.Godzaridis     4/2017
//----------------------------------------------------------------------------------------
bool ScalableMeshModel::HasTerrain()
    {
    return m_terrainParts.size() > 0;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Mathieu.St-Pierre    09/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::SetDisplayTexture(bool displayTexture)
    {
    if (!m_textureInfo->IsTextureAvailable() && displayTexture == true)
        { 
        assert(!"Texture source is unavailable. Cannot turn on texture.");
        return;
        }

    if (m_displayTexture != displayTexture)
        { 
        m_displayTexture = displayTexture;
        ClearAllDisplayMem();
        }   
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

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     1/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::GetActiveClipSetIds(bset<uint64_t>& allShownIds)
    {

    bset<uint64_t> clips;

/*    if (m_smPtr.get() != nullptr && m_smPtr->ShouldInvertClips())
        {
        bvector<uint64_t> clipVec;
        bset<uint64_t> allClips;
        GetClipSetIds(clipVec);
        for (auto elem : clipVec)
            allClips.insert(elem);
        clips.clear();
        if (m_activeClips.empty()) clips = allClips;
        else
            {
            std::set<uint64_t> totalSet;
            std::set<uint64_t> subSet;
            std::set<uint64_t> outSet;

            for (auto& elem : allClips)
                totalSet.insert(elem);

            for (auto& elem : m_activeClips)
                subSet.insert(elem);

            std::set_difference(totalSet.begin(), totalSet.end(), subSet.begin(), subSet.end(), std::inserter(outSet, outSet.end()));

            for (auto& elem : outSet)
                clips.insert(elem);
            }
        }
    else*/ clips = m_activeClips;
    allShownIds = clips;
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

  //  if (model->IsTerrain())
        {
        ScalableMeshTerrainModelAppData* appData(ScalableMeshTerrainModelAppData::Get(db));

        if (appData->m_smTerrainPhysicalModelP == nullptr)
            {
            appData->m_smTerrainPhysicalModelP = model.get();
            appData->m_modelSearched = true;
            }
        }
    //else
        {
        nameToSet = Utf8String(smFilename.GetFileNameWithoutExtension().c_str());
        DgnCode newModelCode(model->GetCode().GetAuthority(), nameToSet, NULL);
        model->SetCode(newModelCode);
        model->Update();
        }

    db.SaveChanges();
    

    return model.get();    
    }

void ScalableMeshModel::ActivateTerrainRegion(const BentleyApi::Dgn::DgnElementId& id, ScalableMeshModel* terrainModel)
{
	if(terrainModel->GetScalableMesh() == nullptr)
		terrainModel->OpenFile(terrainModel->GetPath(), GetDgnDb());
	ActivateClip(id.GetValue());
	ReloadClipMask(id, true);
	terrainModel->GetScalableMesh()->SetInvertClip(true);
	terrainModel->ActivateClip(id.GetValue(), ClipMode::Clip);
}

void ScalableMeshModel::UnlinkTerrainRegion(const BentleyApi::Dgn::DgnElementId& blanketId, const BentleyApi::Dgn::DgnModelId& modelId)
    {
	RemoveRegion(blanketId.GetValue());

	if (nullptr != GetScalableMesh())
		GetScalableMesh()->DeleteCoverage(blanketId.GetValue());

    DeactivateClip(blanketId.GetValue());
	ReloadClipMask(blanketId, true);
    }

void ScalableMeshModel::LinkTerrainRegion(const BentleyApi::Dgn::DgnElementId& blanketId, const BentleyApi::Dgn::DgnModelId& modelId, const bvector<DPoint3d> region, const Utf8String& blanketName)
    {
	if (nullptr != GetScalableMesh())
	    {
		GetScalableMesh()->CreateCoverage(region, blanketId.GetValue(), blanketName.c_str());
	    }

	ActivateClip(blanketId.GetValue());
	ReloadClipMask(blanketId, true);

	ScalableMeshSchema::ScalableMeshModelP terrainModelP = dynamic_cast<ScalableMeshSchema::ScalableMeshModelP>(GetDgnDb().Models().FindModel(modelId).get());
	if (terrainModelP == nullptr)
	{
		terrainModelP = dynamic_cast<ScalableMeshSchema::ScalableMeshModelP>(GetDgnDb().Models().GetModel(modelId).get());
	}
	ScalableMeshModelP regionModelP = nullptr;

	FindTerrainRegion(blanketId.GetValue(), regionModelP);
	if (regionModelP == nullptr)
		{
		AddTerrainRegion(blanketId.GetValue(), terrainModelP, region);
		}
	    
	ActivateTerrainRegion(blanketId, terrainModelP);
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
        T_HOST.GetPointCloudAdmin()._ResolveFileName(m_path, (((this)->m_properties).m_fileId), GetDgnDb());

        if (BeFileName::DoesPathExist(m_path.c_str()) || BeFileName::IsUrl(m_path.c_str()))
        {
            OpenFile(m_path, GetDgnDb());
        }

        m_tryOpen = true;
    }
    }

void ScalableMeshModel::ReloadMesh() // force to reload the entire mesh data
    {
    m_forceRedraw = true;
    }


DgnDbStatus ScalableMeshModel::_OnDelete()
    {

    if (m_subModel)
    {
        m_parentModel->RemoveRegion(m_associatedRegion);
    }


    Cleanup(true);

	DgnDbStatus stat = T_Super::_OnDelete();

    return stat;
    }


HANDLER_DEFINE_MEMBERS(ScalableMeshModelHandler)