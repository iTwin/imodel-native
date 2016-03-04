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

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
AxisAlignedBox3dCR ScalableMeshModel::_GetRange() const
    {
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
    bvector<uint64_t> clipIds;
    clipIds.push_back(clipMaskElementId.GetValue());
    m_progressiveQueryEngine->ClearCaching(clipIds, m_smPtr);
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
    m_smPtr->SetIsInsertingClips(true);
    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
BentleyStatus ScalableMeshModel::_StopClipMaskBulkInsert()
    {
    if (nullptr == m_smPtr.get()) return ERROR;
    m_smPtr->SetIsInsertingClips(false);
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
TerrainModel::IDTM* ScalableMeshModel::_GetDTM()
    {
    if (nullptr == m_smPtr.get()) return nullptr;
    return m_smPtr->GetDTMInterface(m_storageToUorsTransfo);
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

static double s_minScreenPixelsPerPoint = 50;

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



void ProgressiveDrawMeshNode2(bvector<IScalableMeshCachedDisplayNodePtr>& meshNodes,
                              bvector<IScalableMeshCachedDisplayNodePtr>& overviewMeshNodes,                                                                                          
                              ViewContextR                                context, 
                              const DMatrix4d&                            storageToUors)
    {    
    static size_t s_callCount = 0;
    
    bool isOutputQuickVision = context.GetIViewDraw ().IsOutputQuickVision();

    assert(isOutputQuickVision == true);

    /*ElemMatSymbP matSymbP = context.GetElemMatSymb ();

    matSymbP->Init ();
    matSymbP->SetLineColor (ColorDef(0,0x77,0));
    matSymbP->SetFillColor (ColorDef(0,0x77,0));    
    
    context.ResetContextOverrides(); // If not reset, last drawn override is applyed to dtm (Selected/Hide preview)
    context.GetIDrawGeom ().ActivateMatSymb (matSymbP);*/
      
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
            //if (overviewMeshNodes[nodeInd]->GetNbTexture() != 0)
                {
                ElemMatSymbP matSymbP = context.GetElemMatSymb();

                matSymbP->Init();
                ColorDef white(0xff, 0xff, 0xff);
                ColorDef green(0, 0x77, 0);
                matSymbP->SetLineColor(overviewMeshNodes[nodeInd]->GetNbTexture() != 0 ? white : green);
                matSymbP->SetFillColor(overviewMeshNodes[nodeInd]->GetNbTexture() != 0 ? white : green);
                matSymbP->SetIsFilled(false);
                context.ResetContextOverrides(); // If not reset, last drawn override is applyed to dtm (Selected/Hide preview)
                context.GetIDrawGeom().ActivateMatSymb(matSymbP);
                }
            for (size_t meshInd = 0; meshInd < overviewMeshNodes[nodeInd]->GetNbMeshes(); meshInd++)
                {
                SmCachedDisplayMesh* cachedMesh = 0;
                QvElem* qvElem = 0;                
                bool isEmptyMesh = false;

                if (isOutputQuickVision && (SUCCESS == overviewMeshNodes[nodeInd]->GetCachedMesh(cachedMesh, meshInd)))
                    {
                    if (cachedMesh != 0)
                        {                        
                        qvElem = cachedMesh->m_qvElem;                    
                        assert(qvElem != 0);
                        }
                    else
                        {
                        qvElem = 0;
                        isEmptyMesh = true;
                        }                                        
                    }
                else
                    {                    
                    /*NEEDS_WORK_SM : Not support yet.
                    __int64 meshId = GetMeshId(overviewMeshNodes[nodeInd]->GetNodeId(), meshInd);

                    qvElem = QvCachedNodeManager::GetManager().FindQvElem(meshId, dtmDataRef.get());                            
                    */
                    }                
        
                if (qvElem != 0)
                    {   
                    if (cachedMesh == 0)
                        {
                        //NEEDS_WORK_SM : Not support yet.
                        //ActivateMaterial(overviewMeshNodes[nodeInd], meshInd, context);
                        }
                    
                    SmCachedGraphics smCached(context.GetDgnDb(), qvElem);
                    context.DrawCached(smCached);
                    smCached.UnlinkQvElem();
                    
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
                    IScalableMeshMeshPtr mrdtmMeshPtr(overviewMeshNodes[nodeInd]->GetMeshByParts(clips, meshInd));

                    if (mrdtmMeshPtr != 0)
                        {           
                        ActivateMaterial(overviewMeshNodes[nodeInd], meshInd, context);

                        __int64 meshId = GetMeshId(overviewMeshNodes[nodeInd]->GetNodeId(), meshInd);
                        CreateQvElemForMesh(mrdtmMeshPtr, dtmDataRef, element, context, meshId, drawingInfo);                                                       
                        }
                        */
                    }
                }        
            }   
        }


     if (meshNodes.size() > 0)
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
            for (size_t meshInd = 0; meshInd < meshNodes[nodeInd]->GetNbMeshes(); meshInd++)                
                {
                SmCachedDisplayMesh* cachedMesh = 0;
                QvElem* qvElem = 0;                
                bool isEmptyNode = false;

                if (isOutputQuickVision && (SUCCESS == meshNodes[nodeInd]->GetCachedMesh(cachedMesh, meshInd)))
                    {
                    if (cachedMesh != 0)
                        {
                        qvElem = cachedMesh->m_qvElem;                    
                        assert(qvElem != 0);
                        }
                    else
                        {
                        qvElem = 0;
                        isEmptyNode = true;
                        }                    
                    }
                else
                    {
                    /*NEEDS_WORK_SM : Not support yet.
                    __int64 meshId = GetMeshId(meshNodes[nodeInd]->GetNodeId(), meshInd);
                    qvElem = QvCachedNodeManager::GetManager().FindQvElem(meshId, dtmDataRef.get());                            
                    */
                    }
                        
                if (qvElem != 0)
                    {
                    wasDrawn = true;
                    if (cachedMesh == 0)
                        {
                        //NEEDS_WORK_SM : Not support yet.
                        //ActivateMaterial(meshNodes[nodeInd], meshInd, context);
                        }
                                        {
                                        ElemMatSymbP matSymbP = context.GetElemMatSymb();

                                        matSymbP->Init();
                                        ColorDef white(0xff, 0xff, 0xff);
                                        ColorDef green(0, 0x77, 0);
                                        matSymbP->SetLineColor(meshNodes[nodeInd]->GetNbTexture() != 0 ? white : green);
                                        matSymbP->SetFillColor(meshNodes[nodeInd]->GetNbTexture() != 0 ? white : green);
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

            bvector<bool> clips;
            //NEEDS_WORK_SM : Not supported yet
            //dtmDataRef->GetVisibleClips(clips);



            for (size_t meshInd = 0; meshInd < node->GetNbMeshes(); meshInd++)
                {
                IScalableMeshMeshPtr mrdtmMeshPtr(node->GetMeshByParts(clips,meshInd));

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
        }


    context.PopTransformClip();
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

    ScalableMeshProgressiveDisplay (IScalableMeshProgressiveQueryEnginePtr& progressiveQueryEngine,
                                    ScalableMeshDrawingInfoPtr&             currentDrawingInfoPtr, 
                                    DMatrix4d&                              storageToUorsTransfo) 
    : m_storageToUorsTransfo(storageToUorsTransfo)
        { 
        m_progressiveQueryEngine = progressiveQueryEngine;
        m_currentDrawingInfoPtr = currentDrawingInfoPtr;        
        }

public:

    virtual bool _WantTimeoutSet(uint32_t& limit)   {return false; }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Mathieu.St-Pierre     02/2016
//----------------------------------------------------------------------------------------
virtual Completion _Process(ViewContextR viewContext) override
    {
    Completion completionStatus = Completion::Aborted; 

    if (m_currentDrawingInfoPtr->m_overviewNodes.size() > 0)
        {                    
        int queryId = m_currentDrawingInfoPtr->GetViewNumber();

        if (m_progressiveQueryEngine->IsQueryComplete(queryId))
            {
            m_currentDrawingInfoPtr->m_meshNodes.clear();
            StatusInt status = m_progressiveQueryEngine->GetQueriedNodes(m_currentDrawingInfoPtr->m_meshNodes, queryId);
            
            assert(m_currentDrawingInfoPtr->m_overviewNodes.size() == 0 || m_currentDrawingInfoPtr->m_meshNodes.size() > 0);

            m_currentDrawingInfoPtr->m_overviewNodes.clear();

            assert(status == SUCCESS);

            completionStatus = Completion::HealRequired;            
            }
        else
            {                                                
            m_currentDrawingInfoPtr->m_meshNodes.clear();
            StatusInt status = m_progressiveQueryEngine->GetQueriedNodes(m_currentDrawingInfoPtr->m_meshNodes, queryId);
            assert(status == SUCCESS);                                  

            completionStatus = Completion::Aborted;
            }
        }
    else
        {
        completionStatus = Completion::Finished;
        }

    if (s_drawInProcess)
        ProgressiveDrawMeshNode2(m_currentDrawingInfoPtr->m_meshNodes, m_currentDrawingInfoPtr->m_overviewNodes, viewContext, m_storageToUorsTransfo);                              
            
    return completionStatus;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                               Mathieu.St-Pierre      02/2016
//----------------------------------------------------------------------------------------
static void Schedule (IScalableMeshProgressiveQueryEnginePtr& progressiveQueryEngine,
                      ScalableMeshDrawingInfoPtr&             currentDrawingInfoPtr, 
                      DMatrix4d&                              storageToUorsTransfo, 
                      ViewContextR                            context) 
    {
    ScalableMeshProgressiveDisplay* progressiveDisplay = new ScalableMeshProgressiveDisplay(progressiveQueryEngine,
                                                                                            currentDrawingInfoPtr,
                                                                                            storageToUorsTransfo);
    
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


void ScalableMeshModel::_AddGraphicsToScene(ViewContextR context)
    {    
    if (!ShouldDrawInContext(context) || NULL == context.GetViewport())
        return;
         
    ScalableMeshDrawingInfoPtr nextDrawingInfoPtr(new ScalableMeshDrawingInfo(&context));

    if ((m_currentDrawingInfoPtr != nullptr) &&
        (m_currentDrawingInfoPtr->GetDrawPurpose() != DrawPurpose::UpdateDynamic))
        {
        //If the m_dtmPtr equals 0 it could mean that the last data request to the STM was cancelled, so start a new request even
        //if the view has not changed.
        if (m_currentDrawingInfoPtr->HasAppearanceChanged(nextDrawingInfoPtr) == false)                
            {
            //assert((m_currentDrawingInfoPtr->m_overviewNodes.size() == 0) && (m_currentDrawingInfoPtr->m_meshNodes.size() > 0));

            ProgressiveDrawMeshNode2(m_currentDrawingInfoPtr->m_meshNodes, m_currentDrawingInfoPtr->m_overviewNodes, context, m_storageToUorsTransfo);                              
            return;                        
            }                        
        }        
    
    BentleyStatus status;

    status = m_progressiveQueryEngine->StopQuery(nextDrawingInfoPtr->GetViewNumber()); 
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
    int queryId = nextDrawingInfoPtr->GetViewNumber();                 
    
    bvector<bool> clips;
    /*NEEDS_WORK_SM : Get clips
    m_DTMDataRef->GetVisibleClips(clips);
    */

    status = m_progressiveQueryEngine->StartQuery(queryId, 
                                                  viewDependentQueryParams, 
                                                  m_currentDrawingInfoPtr->m_meshNodes, 
                                                  !IsWireframeRendering(context), 
                                                  clips,
                                                  &m_currentDrawingInfoPtr->GetLocalToViewTransform(), 
                                                  &nextDrawingInfoPtr->GetLocalToViewTransform()); 

    assert(status == SUCCESS);

    bool needProgressive;
    
    if (m_progressiveQueryEngine->IsQueryComplete(queryId))
        {
        m_currentDrawingInfoPtr->m_meshNodes.clear();
        status = m_progressiveQueryEngine->GetQueriedNodes(m_currentDrawingInfoPtr->m_meshNodes, queryId);
        m_currentDrawingInfoPtr->m_overviewNodes.clear();
        assert(status == SUCCESS);

        needProgressive = false;
        }
    else
        {        
        status = m_progressiveQueryEngine->GetOverviewNodes(m_currentDrawingInfoPtr->m_overviewNodes, queryId);
        m_currentDrawingInfoPtr->m_overviewNodes.insert(m_currentDrawingInfoPtr->m_overviewNodes.end(), m_currentDrawingInfoPtr->m_meshNodes.begin(), m_currentDrawingInfoPtr->m_meshNodes.end());
        m_currentDrawingInfoPtr->m_meshNodes.clear();
        assert(m_currentDrawingInfoPtr->m_overviewNodes.size() > 0);
        assert(status == SUCCESS);

        needProgressive = true;
        }                         

    ProgressiveDrawMeshNode2(m_currentDrawingInfoPtr->m_meshNodes, m_currentDrawingInfoPtr->m_overviewNodes, context, m_storageToUorsTransfo);                              

    if (needProgressive)
        {
        ScalableMeshProgressiveDisplay::Schedule(m_progressiveQueryEngine, m_currentDrawingInfoPtr, m_storageToUorsTransfo, context);
        }
    }                 

//NEEDS_WORK_SM : Should be at application level
void GetScalableMeshTerrainFileName(BeFileName& smtFileName, const BeFileName& dgnDbFileName)
    {    
    //smtFileName = params.m_dgndb.GetFileName().GetDirectoryName();

    smtFileName = dgnDbFileName.GetDirectoryName();
    smtFileName.AppendToPath(dgnDbFileName.GetFileNameWithoutExtension().c_str());
    smtFileName.AppendString(L"\\terrain.smt");
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
ScalableMeshModel::ScalableMeshModel(BentleyApi::Dgn::DgnModel::CreateParams const& params)
    : T_Super(params)
    {
    BeFileName tmFileName;
    tmFileName = params.m_dgndb.GetFileName().GetDirectoryName();
    tmFileName.AppendToPath(params.m_dgndb.GetFileName().GetFileNameWithoutExtension().c_str());
    tmFileName.AppendString(L"\\terrain.stm");

    if (BeFileName::DoesPathExist(tmFileName.c_str()))
        {
        OpenFile(tmFileName, GetDgnDb());    
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
ScalableMeshModel::~ScalableMeshModel()
    {

    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::OpenFile(BeFileNameCR smFilename, DgnDbR dgnProject)
    {    
    assert(m_smPtr == nullptr);

    m_smPtr = IScalableMesh::GetFor(smFilename.GetWCharCP(), false, true);

    assert(m_smPtr != 0);

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
        }
    else
        {                
        dgnProject.Units().GetDgnGCS()->UorsFromCartesian(scale, scale);
        }
           
    DPoint3d translation = {0,0,0};
    
    m_storageToUorsTransfo = DMatrix4d::FromScaleAndTranslation(scale, translation);                
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
ScalableMeshModelP ScalableMeshModel::CreateModel(BentleyApi::Dgn::DgnDbR dgnDb)
    {
    DgnClassId classId(dgnDb.Schemas().GetECClassId("ScalableMesh","ScalableMeshModel"));
    BeAssert(classId.IsValid());

    ScalableMeshModelP model = new ScalableMeshModel(DgnModel::CreateParams(dgnDb, classId, DgnModel::CreateModelCode("scalableTerrain")));

    model->Insert();
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


IMeshSpatialModelP ScalableMeshModelHandler::AttachTerrainModel(DgnDbR db, Utf8StringCR modelName, BeFileNameCR smFilename)
    {    
    /*    
    BeFileName smtFileName;
    GetScalableMeshTerrainFileName(smtFileName, db.GetFileName());
    
    if (!smtFileName.GetDirectoryName().DoesPathExist())
        BeFileName::CreateNewDirectory(smtFileName.GetDirectoryName().c_str());
        */

    
    DgnClassId classId(db.Schemas().GetECClassId("ScalableMesh", "ScalableMeshModel"));
    BeAssert(classId.IsValid());        
         
    RefCountedPtr<ScalableMeshModel> model(new ScalableMeshModel(DgnModel::CreateParams(db, classId, DgnModel::CreateModelCode(modelName))));

    model->OpenFile(smFilename, db);

    //After Insert model pointer is handled by DgnModels.
    model->Insert();
                    
    ScalableMeshTerrainModelAppData* appData(ScalableMeshTerrainModelAppData::Get(db));        

    appData->m_smTerrainPhysicalModelP = model.get();
    appData->m_modelSearched = true;

    db.SaveChanges();
        
    return model.get();    
    }



HANDLER_DEFINE_MEMBERS(ScalableMeshModelHandler)