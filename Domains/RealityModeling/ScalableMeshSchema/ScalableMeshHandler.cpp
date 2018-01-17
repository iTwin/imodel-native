/*-------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshSchema/ScalableMeshHandler.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ScalableMeshSchemaPCH.h"
#include <ScalableMesh\ScalableMeshLib.h>
#include <BeSQLite\BeSQLite.h>
#include <ScalableMeshSchema\ScalableMeshHandler.h>
#include "ScalableMeshDisplayCacheManager.h"
#include <ScalableMesh\GeoCoords\GCS.h>
#include <DgnPlatform\LinkElement.h>
#include <Bentley\BeDirectoryIterator.h>
#include <ScalableMesh/ScalableMeshLib.h>
#include <ScalableMesh\ScalableMeshUtilityFunctions.h>
#include <DgnPlatform\TextString.h>
#include <DgnPlatform\DgnGeoCoord.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_TILETREE

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
    if (m_smPtr.IsValid())
		{
        m_smPtr->GetRange(const_cast<AxisAlignedBox3d&>(m_range));
		m_smPtr->GetReprojectionTransform().Multiply(m_range, m_range);
		}

    return m_range;
    }

#if defined(TODO_TILE_PUBLISH)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
PublishedTilesetInfo ScalableMeshModel::_GetPublishedTilesetInfo()
    {
    if (m_smPtr.IsNull())
        return PublishedTilesetInfo();

    return PublishedTilesetInfo(Utf8String(GetPath()), _GetRange());
    }
#endif

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Mathieu.St-Pierre     8/2017
//----------------------------------------------------------------------------------------
AxisAlignedBox3d ScalableMeshModel::_QueryModelRange() const
    {        
    AxisAlignedBox3d range(DRange3d::NullRange());

    if (m_smPtr.IsValid()) 
        { 
        m_smPtr->GetRange(const_cast<AxisAlignedBox3d&>(range));
        Transform transform(m_smPtr->GetReprojectionTransform());

        transform.Multiply(range, range);
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
    switch (viewContext.GetViewFlags().GetRenderMode())
        {        
        case RenderMode::SmoothShade:        
            return false;
                       
        case RenderMode::Wireframe:        
        case RenderMode::HiddenLine:
        case RenderMode::SolidFill:
            return true;
        }
        BeAssert(!"Unknown render mode");
        return true;
    }

static bool s_waitCheckStop = false;
static Byte s_transparency = 0;
static bool s_applyClip = false;
static bool s_dontShowMesh = false;
static bool s_showTiles = false;
static double s_tileSizePerIdStringSize = 10.0;

void ProgressiveDrawMeshNode2(bvector<IScalableMeshCachedDisplayNodePtr>& meshNodes,
        bvector<IScalableMeshCachedDisplayNodePtr>& overviewMeshNodes,
        Dgn::RenderContextR                         context,
        const DMatrix4d&                            storageToUors,

                             bset<uint64_t>&                             activeClips,
                             bool                                        displayTexture, 
                             bool                                        isCesium)
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
				   matSymbP->SetLineColor(overviewMeshNodes[nodeInd]->IsTextured() && displayTexture ? white : green);
	               matSymbP->SetFillColor(overviewMeshNodes[nodeInd]->IsTextured() && displayTexture ? white : green);

                   matSymbP->SetFillTransparency(s_transparency);
                   matSymbP->SetLineTransparency(s_transparency);

                   matSymbP->SetIsFilled(false);
                   context.ResetContextOverrides(); // If not reset, last drawn override is applyed to dtm (Selected/Hide preview)
                   context.GetIDrawGeom().ActivateMatSymb(matSymbP);
                   */
                }

            bvector<SmCachedDisplayMesh*> cachedMeshes;
            bvector<bpair<bool, uint64_t>> textureIds;
                        
            if (SUCCESS == overviewMeshNodes[nodeInd]->GetCachedMeshes(cachedMeshes, textureIds))
                {
                for (auto& cachedMesh : cachedMeshes)
                    {
                    graphics.Add(*cachedMesh->m_graphic);
                    }        
                }
            else
                {
                /*NEEDS_WORK_SM : Not support yet.
                  __int64 meshId = GetMeshId(overviewMeshNodes[nodeInd]->GetNodeId());

                  qvElem = QvCachedNodeManager::GetManager().FindQvElem(meshId, dtmDataRef.get());
                  */
                //assert(!"Should not get here");
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

            bvector<SmCachedDisplayMesh*> cachedMeshes;
            bvector<bpair<bool, uint64_t>> textureIds;
                        
            if (SUCCESS == meshNodes[nodeInd]->GetCachedMeshes(cachedMeshes, textureIds))
                {
                for (auto& cachedMesh : cachedMeshes)
                    {
                    graphics.Add(*cachedMesh->m_graphic);
                    }        
                }
            else
                {
                //assert(!"Should not occur");
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

    //auto group = context.CreateBranch(Render::Graphic::CreateParams(nullptr, storageToUorsTransform), graphics);
    auto group = context.CreateBranch(graphics, &storageToUorsTransform);
    context.OutputGraphic(*group, nullptr);

#endif
    }

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
	


void GetBingLogoInfo(Transform& correctedViewToView, ViewContextR context)
    {
    DRange3d viewCorner(context.GetViewport()->GetViewCorners());

    DPoint2d nonPrintableMargin = { 0,0 };
    
    // CorrectedViewToView transform: adjust for swapped y and non-printable margin.
    if (viewCorner.low.y > viewCorner.high.y)
        {
        correctedViewToView.InitFrom(nonPrintableMargin.x, viewCorner.low.y - nonPrintableMargin.y, 0);
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


    assert(!"ViewToLocal not available on Bim02");

#if 0

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

#endif
    }

static bool s_loadTexture = true;
static bool s_waitQueryComplete = true;

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                    Mathieu.St-Pierre  08/17
 +---------------+---------------+---------------+---------------+---------------+------*/
SMGeometry::SMGeometry(CreateParams const& params, SMSceneR scene, Dgn::Render::SystemP sys) : Dgn::TileTree::TriMeshTree::TriMesh(params, scene, sys) { }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  08/17
//----------------------------------------------------------------------------------------
SMNode::SMLoader::SMLoader(Dgn::TileTree::TileR tile, Dgn::TileTree::TileLoadStatePtr loads, Dgn::Render::SystemP renderSys)
    : TileLoader("", tile, loads, tile._GetTileCacheKey(), renderSys)
    {
    //assert(renderSys != nullptr);

    if (renderSys == nullptr)
        m_renderSys = m_tile->GetRoot().GetRenderSystemP();
    }

//SMNode
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  08/17
//----------------------------------------------------------------------------------------
TileLoaderPtr SMNode::_CreateTileLoader(TileLoadStatePtr loads, Dgn::Render::SystemP renderSys)
    {
    return new SMLoader(*this, loads, renderSys);
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                    Mathieu.St-Pierre  08/17
 +---------------+---------------+---------------+---------------+---------------+------*/
bool SMNode::_WantDebugRangeGraphics() const
    {
    static bool s_debugRange = false;
    return s_debugRange;
    }

void SMNode::_GetCustomMetadata(Utf8StringR name, Json::Value& data) const
    {
    name = "SMHeader";
    IScalableMeshPublisher::Create(SMPublishType::CESIUM)->ExtractPublishNodeHeader(m_scalableMeshNodePtr, data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre  08/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_tryProgQuery = false;
static bool s_tryCustomSelect = true;

Tile::ChildTiles const* SMNode::_GetChildren(bool load) const
    { 
    if (!s_tryProgQuery && !s_tryCustomSelect)
        return __super::_GetChildren(load);
/*
    if (!IsReady())
        return nullptr;

    if (m_children.size() == 0)
        return nullptr;

    if (!m_children[0]->IsReady())
        return nullptr;

    return &m_children;
*/
    
    bvector<IScalableMeshNodePtr> childrenNodes(m_scalableMeshNodePtr->GetChildrenNodes());

    if (m_children.size() == 0 && childrenNodes.size() > 0)
        {
        for (auto& childNode : childrenNodes)
            {
            //BentleyB0200::Dgn::TileTree::TriMeshTree::Tile* thisTile(const_cast<BentleyB0200::Dgn::TileTree::TriMeshTree::Tile*>(this))
            SMNode* thisTile(const_cast<SMNode*>(this));

            SMNodePtr nodeptr = new SMNode(thisTile->GetTriMeshRootR(), thisTile, childNode);
            nodeptr->m_3smModel = m_3smModel;

            /*
            if (!nodeptr->ReadHeader(centroid))
            return ERROR;
            */
            /*
            if (loadChildren)
            {
            nodeptr->Read3SMTile(in, scene, renderSys, false);
            }
            */

            m_children.push_back(nodeptr);
            }
        }

    if (m_children.size() == 0)
        return nullptr;

    return &m_children;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre  08/17
+---------------+---------------+---------------+---------------+---------------+------*/
/*
bool SMNode::IsNotLoaded() const
    { 
    if (m_loadStatus.load() == LoadStatus::NotLoaded)
        return true;
    
    if (m_children.size() > 0 && m_children[0].GetLoadStatus() == == LoadStatus::NotLoaded)
        return true;

    return false;
    }
*/

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                   Mathieu.St-Pierre  08/17
 +---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SMNode::_GetTileCacheKey() const
    {
    std::stringstream stream;
    stream << m_scalableMeshNodePtr->GetNodeId();
    return Utf8String(stream.str().c_str());
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                   Mathieu.St-Pierre  08/17
 +---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SMNode::Read3SMTile(StreamBuffer& in, SMSceneR scene, Dgn::Render::SystemP renderSys, bool loadChildren)
    {
    BeAssert(!IsReady());

#if 0
    //Just need to load the children
    if (m_children.size() > 0)
        {
        for (auto& child : m_children)
            {                     
            ((SMNode*)&child)->Read3SMTile(in, scene, renderSys, false);
            }

        SetIsReady();
        return SUCCESS;
        }
#endif

    if (SUCCESS != DoRead(in, scene, renderSys, loadChildren))
        {
        SetNotFound();
        BeAssert(false);
        return ERROR;
        }

    // only after we've successfully read the entire node, mark it as ready so other threads can look at its child nodes.
    //if (loadChildren)
        SetIsReady();

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.St-Pierre  12/17
+---------------+---------------+---------------+---------------+---------------+------*/

#ifndef NDEBUG
    static double s_firstNodeSearchingDelay = (double)1 / 15 * CLOCKS_PER_SEC;
#else
    //static double s_firstNodeSearchingDelay = (double)1 / 10 * CLOCKS_PER_SEC;
    static double s_firstNodeSearchingDelay = (double)1 / 60 * CLOCKS_PER_SEC;
#endif

Dgn::TileTree::Tile::SelectParent SMNode::_SelectTiles(bvector<Dgn::TileTree::TileCPtr>& selected, Dgn::TileTree::DrawArgsR args) const
    {

    if (s_tryCustomSelect)
        {        
        static clock_t startTime = 0;
        static IScalableMeshViewDependentMeshQueryParamsPtr viewDependentQueryParams;

        if (m_parent == nullptr)
            {
            startTime = clock();
            
            viewDependentQueryParams = IScalableMeshViewDependentMeshQueryParams::CreateParams();

            DMatrix4d localToView(args.m_context.GetWorldToView().M0);

            ClipVectorPtr clipVector;
            //clip = args.m_context.GetTransformClipStack().GetClip();
            Render::FrustumPlanes frustumPlanes(args.m_context.GetFrustumPlanes());

            ConvexClipPlaneSet convexClipPlaneSet(&frustumPlanes.m_planes[0], 6);

            ClipPlaneSet clipPlaneSet(convexClipPlaneSet);

            ClipPrimitivePtr clipPrimitive(ClipPrimitive::CreateFromClipPlanes(clipPlaneSet));

            clipVector = ClipVector::CreateFromPrimitive(clipPrimitive.get());


            DMatrix4d smToUOR = DMatrix4d::From(m_3smModel->m_smToModelUorTransform);

            bsiDMatrix4d_multiply(&localToView, &localToView, &smToUOR);
            
            viewDependentQueryParams->SetMinScreenPixelsPerPoint(s_minScreenPixelsPerPoint);

            if (m_3smModel->m_textureInfo->IsUsingBingMap())
            {
                viewDependentQueryParams->SetMaxPixelError(s_maxPixelErrorStreamingTexture);
            }
            else
            {
                viewDependentQueryParams->SetMaxPixelError(s_maxPixelError);
            }

            viewDependentQueryParams->SetRootToViewMatrix(localToView.coff);

            clipVector->TransformInPlace(m_3smModel->m_modelUorToSmTransform);

            viewDependentQueryParams->SetViewClipVector(clipVector);
            }

        if ((clock() - startTime) > s_firstNodeSearchingDelay)
            {
            return SelectParent::Yes;            
            }
        

        DgnDb::VerifyClientThread();        

        SMNodeViewStatus viewStatus = m_scalableMeshNodePtr->IsCorrectForView(viewDependentQueryParams);

        if (viewStatus == SMNodeViewStatus::NotVisible)
            {
            return SelectParent::No;
            }

        if (viewStatus == SMNodeViewStatus::Fine)
            {
            if (IsReady())
                {
                selected.push_back(this);
                return SelectParent::No;
                }
            else
                {
                /*
                SMNodePtr thisTile(const_cast<SMNode*>(this));
                m_3smModel->m_currentDrawingInfoPtr->m_nodesToLoad.push_back(thisTile);
                */
                args.InsertMissing(*this);
                return SelectParent::Yes;
                }
            }

        assert(viewStatus == SMNodeViewStatus::TooCoarse);
        //if (viewStatus == SMNodeViewStatus::TooCoarse)        
        bool drawParent = false;
        
        auto children = _GetChildren(true);

        if (nullptr != children)
            {
            for (auto const& child : *children)
                {
                if (SelectParent::Yes == child->_SelectTiles(selected, args))
                    {
                    drawParent = true;
                    // NB: We must continue iterating children so that they can be requested if missing...
                    }
                }
            }

        if (!drawParent)
            {
            return SelectParent::No;
            }

        if (IsReady())
            {
            if (_HasGraphics())
                selected.push_back(this);

            return SelectParent::No;
            }
        else if (_HasBackupGraphics())
            {
            // Caching previous graphics while regenerating tile to reduce flishy-flash when model changes.
            selected.push_back(this);
            return SelectParent::No;
            }

        return SelectParent::Yes;
        }


    if (m_parent == nullptr && s_tryProgQuery)
        { 
        //m_displayNodesCache->SetRenderSys(Dgn::Render::SystemP renderSys);

        ScalableMeshDrawingInfoPtr nextDrawingInfoPtr(new ScalableMeshDrawingInfo(&args.m_context));
        
        //nextDrawingInfoPtr->m_smPtr = m_smPtr.get();
        //nextDrawingInfoPtr->m_currentQuery = (int)((GetModelId().GetValue() - GetModelId().GetBriefcaseId().GetValue()) & 0xFFFF);
        nextDrawingInfoPtr->m_currentQuery = 0;

        bool newQuery = true;

        if ((m_3smModel->m_currentDrawingInfoPtr != nullptr)/* &&
            (m_3smModel->m_currentDrawingInfoPtr->GetDrawPurpose() != DrawPurpose::UpdateDynamic)*/)
        {
            //If the m_dtmPtr equals 0 it could mean that the last data request to the STM was cancelled, so start a new request even
            //if the view has not changed.
            if (m_3smModel->m_currentDrawingInfoPtr->HasAppearanceChanged(nextDrawingInfoPtr) == false /*&& !m_forceRedraw*/)
            {
                //assert((m_currentDrawingInfoPtr->m_overviewNodes.size() == 0) && (m_currentDrawingInfoPtr->m_meshNodes.size() > 0));            
                //ProgressiveDrawMeshNode(m_currentDrawingInfoPtr->m_meshNodes, m_currentDrawingInfoPtr->m_overviewNodes, context, m_smToModelUorTransform, (ScalableMeshDisplayCacheManager*)m_displayNodesCache.get(), m_smPtr->ShouldInvertClips() ? m_notActiveClips : m_activeClips, m_displayTexture, m_smPtr->IsCesium3DTiles());


                int queryId = m_3smModel->m_currentDrawingInfoPtr->m_currentQuery;

                if (m_3smModel->GetProgressiveQueryEngine()->IsQueryComplete(queryId))
                {
                    m_3smModel->m_currentDrawingInfoPtr->m_meshNodes.clear();

                    StatusInt status = m_3smModel->GetProgressiveQueryEngine()->GetRequiredNodes(m_3smModel->m_currentDrawingInfoPtr->m_meshNodes, queryId);

                    assert(status == SUCCESS);

                    assert(m_3smModel->m_currentDrawingInfoPtr->m_overviewNodes.size() == 0 || m_3smModel->m_currentDrawingInfoPtr->m_meshNodes.size() > 0);
                    bvector<IScalableMeshNodePtr> nodes;
                    for (auto& nodeP : m_3smModel->m_currentDrawingInfoPtr->m_meshNodes) nodes.push_back(nodeP.get());
                    m_3smModel->m_smPtr->SetCurrentlyViewedNodes(nodes);

                    m_3smModel->m_currentDrawingInfoPtr->m_overviewNodes.clear();

                    status = m_3smModel->GetProgressiveQueryEngine()->StopQuery(queryId);

                    assert(status == SUCCESS);

//                    m_hasFetchedFinalNode = true;
                }
                else
                {
                    m_3smModel->m_currentDrawingInfoPtr->m_meshNodes.clear();
                    StatusInt status = m_3smModel->GetProgressiveQueryEngine()->GetRequiredNodes(m_3smModel->m_currentDrawingInfoPtr->m_meshNodes, queryId);
                    assert(status == SUCCESS);

                    m_3smModel->m_currentDrawingInfoPtr->m_overviewNodes.clear();
                    status = m_3smModel->GetProgressiveQueryEngine()->GetOverviewNodes(m_3smModel->m_currentDrawingInfoPtr->m_overviewNodes, queryId);
                    assert(status == SUCCESS);
                }

                
                newQuery = false;
            }
        }


        if (newQuery)
            {

            m_3smModel->m_currentDrawingInfoPtr = nextDrawingInfoPtr;

            BentleyStatus status;

            /*
            if (restartQuery)
            {*/
                status = m_3smModel->GetProgressiveQueryEngine()->StopQuery(/*nextDrawingInfoPtr->GetViewNumber()*/nextDrawingInfoPtr->m_currentQuery);
                assert(status == SUCCESS);
            //}

            
            DMatrix4d localToView(args.m_context.GetWorldToView().M0);

            ClipVectorPtr clipVector;
            //clip = args.m_context.GetTransformClipStack().GetClip();
            Render::FrustumPlanes frustumPlanes(args.m_context.GetFrustumPlanes());
                
            ConvexClipPlaneSet convexClipPlaneSet(&frustumPlanes.m_planes[0], 6);

            ClipPlaneSet clipPlaneSet(convexClipPlaneSet);

            ClipPrimitivePtr clipPrimitive(ClipPrimitive::CreateFromClipPlanes(clipPlaneSet));

            clipVector = ClipVector::CreateFromPrimitive(clipPrimitive.get());


            DMatrix4d smToUOR = DMatrix4d::From(m_3smModel->m_smToModelUorTransform);

            bsiDMatrix4d_multiply(&localToView, &localToView, &smToUOR);
   

            IScalableMeshViewDependentMeshQueryParamsPtr viewDependentQueryParams(IScalableMeshViewDependentMeshQueryParams::CreateParams());

            viewDependentQueryParams->SetMinScreenPixelsPerPoint(s_minScreenPixelsPerPoint);

            if (m_3smModel->m_textureInfo->IsUsingBingMap())
                {
                viewDependentQueryParams->SetMaxPixelError(s_maxPixelErrorStreamingTexture);
                }
            else
                {
                viewDependentQueryParams->SetMaxPixelError(s_maxPixelError);
                }

            viewDependentQueryParams->SetRootToViewMatrix(localToView.coff);
        
            clipVector->TransformInPlace(m_3smModel->m_modelUorToSmTransform);

            viewDependentQueryParams->SetViewClipVector(clipVector);


            int queryId = 0;

            bvector<bool> clips;
            /*NEEDS_WORK_SM : Get clips
            m_DTMDataRef->GetVisibleClips(clips);
            */   

            //ScalableMeshDrawingInfoPtr nextDrawingInfoPtr(new ScalableMeshDrawingInfo(&context));
            m_3smModel->m_currentDrawingInfoPtr = new ScalableMeshDrawingInfo(&args.m_context);

            StatusInt statusQuery = m_3smModel->GetProgressiveQueryEngine()->StartQuery(queryId,
                viewDependentQueryParams,
                m_3smModel->m_currentDrawingInfoPtr->m_meshNodes,
                m_3smModel->m_displayTexture, //No wireframe mode, so always load the texture.
                clips,
                m_3smModel->m_smPtr);

            assert(statusQuery == SUCCESS);
        
            if (s_waitQueryComplete || !m_3smModel->m_isProgressiveDisplayOn)
                {
                while (!m_3smModel->GetProgressiveQueryEngine()->IsQueryComplete(queryId))
                    {
                    BeThreadUtilities::BeSleep(200);
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
 

            if (m_3smModel->GetProgressiveQueryEngine()->IsQueryComplete(queryId))
                {
                m_3smModel->m_currentDrawingInfoPtr->m_meshNodes.clear();
                status = m_3smModel->GetProgressiveQueryEngine()->GetRequiredNodes(m_3smModel->m_currentDrawingInfoPtr->m_meshNodes, queryId);
                assert(status == SUCCESS);
                m_3smModel->m_currentDrawingInfoPtr->m_overviewNodes.clear();

                bvector<IScalableMeshNodePtr> nodes;
                for (auto& nodeP : m_3smModel->m_currentDrawingInfoPtr->m_meshNodes) nodes.push_back(nodeP.get());
                m_3smModel->m_smPtr->SetCurrentlyViewedNodes(nodes);    
                //needProgressive = false;
                }
            else
                {
                status = m_3smModel->GetProgressiveQueryEngine()->GetOverviewNodes(m_3smModel->m_currentDrawingInfoPtr->m_overviewNodes, queryId);

                m_3smModel->m_currentDrawingInfoPtr->m_meshNodes.clear();

                status = m_3smModel->GetProgressiveQueryEngine()->GetRequiredNodes(m_3smModel->m_currentDrawingInfoPtr->m_meshNodes, queryId);
                bvector<IScalableMeshNodePtr> nodes;
                for (auto& nodeP : m_3smModel->m_currentDrawingInfoPtr->m_meshNodes) nodes.push_back(nodeP.get());
                for (auto& nodeP : m_3smModel->m_currentDrawingInfoPtr->m_overviewNodes) nodes.push_back(nodeP.get());
                m_3smModel->m_smPtr->SetCurrentlyViewedNodes(nodes);
                assert(status == SUCCESS);

                //NEEDS_WORK_MST : Will be fixed when the lowest resolution is created and pin at creation time.
                //assert(m_currentDrawingInfoPtr->m_overviewNodes.size() > 0);
                assert(status == SUCCESS);

                //needProgressive = true;
                }
            }
        }
        
    if (s_tryProgQuery)
        {
        DgnDb::VerifyClientThread();

        bool foundNode = false; 

        for (auto& node : m_3smModel->m_currentDrawingInfoPtr->m_meshNodes)
            {        
            if (m_scalableMeshNodePtr->GetNodeId() == node->GetNodeId())
                {
                foundNode = true;
                break;
                }
            }

        if (foundNode)
            {
            if (IsReady())
                {   
                selected.push_back(this);
                return SelectParent::No;
                }
            else
                {                                
                SMNodePtr thisTile(const_cast<SMNode*>(this));
                //m_3smModel->m_currentDrawingInfoPtr->m_nodesToLoad.push_back(thisTile);
                args.InsertMissing(*this);
                return SelectParent::Yes;
                }        
            }

        bset<__int64>& ancestors(m_3smModel->m_currentDrawingInfoPtr->GetAncestors());

        bool drawParent = false;
    
        if (ancestors.find(m_scalableMeshNodePtr->GetNodeId()) != ancestors.end())
            {
            auto children = _GetChildren(true);

            if (nullptr != children)
                {
                for (auto const& child : *children)
                    {                    
                    if (SelectParent::Yes == child->_SelectTiles(selected, args))
                        {
                        drawParent = true;
                        // NB: We must continue iterating children so that they can be requested if missing...
                        }
                    }            
                }
            }
    
        if (!drawParent)
            {
            return SelectParent::No;
            }

        if (IsReady())
            {
            if (_HasGraphics())
                selected.push_back(this);

            return SelectParent::No;
            }
        else if (_HasBackupGraphics())
            {
            // Caching previous graphics while regenerating tile to reduce flishy-flash when model changes.
            selected.push_back(this);
            return SelectParent::No;
            }

        return SelectParent::Yes;
        }
    
    return __super::_SelectTiles(selected, args);
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                   Mathieu.St-Pierre  08/17
 +---------------+---------------+---------------+---------------+---------------+------*/
static double s_maxDiamFactor = 10;
static double s_constantFactor = 100;
bool SMNode::ReadHeader(Transform& locationTransform)
    {
    m_range.low = m_scalableMeshNodePtr->GetContentExtent().low;
    m_range.high = m_scalableMeshNodePtr->GetContentExtent().high;

    locationTransform.Multiply(m_range.low);
    locationTransform.Multiply(m_range.high);
    /*
       JsonValueCR val = pt["maxScreenDiameter"];
       if (val.empty())
       {
       LOG_ERROR("Cannot find \"maxScreenDiameter\" entry");
       return false;
       }

       m_maxDiameter = val.asDouble();
       */

    float geometricResolution;
    float textureResolution;

    m_scalableMeshNodePtr->GetResolutions(geometricResolution, textureResolution);
        
    //m_maxDiameter = m_range.low.Distance(m_range.high) / std::min(geometricResolution, textureResolution) / s_maxDiamFactor;
    m_maxDiameter = s_constantFactor / std::min(geometricResolution, textureResolution);
	
	//m_maxDiameter = 1000 / std::min(geometricResolution, textureResolution);

    /*
       if (!readVectorEntry(pt, "resources", nodeResources))
       {
       LOG_ERROR("Cannot find \"resources\" entry");
       return false;
       }

       bvector<Utf8String> children;
       if (!readVectorEntry(pt, "children", children))
       return false;

       BeAssert(children.size() <= 1);

       if (1 == children.size())
       m_childPath = children[0];

       if (m_parent)
       m_parent->ExtendRange(m_range);
       */
    return true;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                    Mathieu.St-Pierre  12/17
//----------------------------------------------------------------------------------------
SMNode::~SMNode()
    {
    int i = 0;
    i = i;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Mathieu.St-Pierre  08/17
//----------------------------------------------------------------------------------------
static bool s_applyTexture = true;

BentleyStatus SMNode::DoRead(StreamBuffer& in, SMSceneR scene, Dgn::Render::SystemP renderSys, bool loadChildren)
    {
    //BeAssert(IsQueued() || ((m_parent != nullptr) && (m_parent->GetLoadStatus() == LoadStatus::Loading)));

    m_loadStatus.store(LoadStatus::Loading);

    BeAssert(m_children.empty());

    DRange3d range3D(scene.m_smPtr->GetRootNode()->GetContentExtent());
    //DRange3d range3D(m_scalableMeshNodePtr->GetContentExtent());


    Transform toFloatTransform(scene.GetToFloatTransform());

    if (!ReadHeader(toFloatTransform))
        return ERROR;

#if 0
    bmap<Utf8String, int> textureIds, nodeIds;
    bmap<Utf8String, Utf8String> geometryNodeCorrespondence;
    int nodeCount = 0;

    uint32_t magicSize = (uint32_t)GetMagicString().size();
    ByteCP currPos = in.GetCurrent();
    if (!in.Advance(magicSize))
        {
        LOG_ERROR("Can't read magic number");
        return ERROR;
        }

    Utf8String magicNumber((Utf8CP)currPos, (Utf8CP)in.GetCurrent());
    if (magicNumber != GetMagicString())
        {
        LOG_ERROR("wrong magic number");
        return ERROR;
        }

    uint32_t infoSize;
    if (!CtmContext::ReadBytes(in, &infoSize, 4))
        {
        LOG_ERROR("Can't read size");
        return ERROR;
        }

    Utf8P infoStr = (Utf8P)in.GetCurrent();
    Json::Value pt;
    Json::Reader reader;
    if (!reader.parse(infoStr, infoStr + infoSize, pt))
        {
        LOG_ERROR("Cannot parse info: ");
        return ERROR;
        }

    int version = pt.get("version", 0).asInt();
    if (version != 1)
        {
        LOG_ERROR("Unsupported version");
        return ERROR;
        }

    JsonValueCR nodes = pt["nodes"];
    if (!nodes.empty())
        {
        for (JsonValueCR node : nodes)
            {
            Utf8String nodeName;
            bvector<Utf8String> nodeResources;
            nodeName = node.get("id", "").asCString();

            NodePtr nodeptr = new Node(GetRootR(), this);

            if (!nodeptr->ReadHeader(node, nodeName, nodeResources))
                return ERROR;

            nodeIds[nodeName] = nodeCount++;
            for (size_t i = 0; i < nodeResources.size(); ++i)
                geometryNodeCorrespondence[nodeResources[i]] = nodeName;

            m_children.push_back(nodeptr);
            }
        }

    Utf8String resourceType, resourceFormat, resourceName;
    uint32_t resourceSize;
    uint32_t offset = (uint32_t)GetMagicString().size() + 4 + infoSize;

    JsonValueCR resources = pt["resources"];
    if (resources.empty())
        return SUCCESS;

    bmap<Utf8String, Dgn::Render::TexturePtr> renderTextures;
    for (JsonValueCR resource : resources)
        {
        resourceType = resource.get("type", "").asCString();
        resourceFormat = resource.get("format", "").asCString();
        resourceName = resource.get("id", "").asCString();
        resourceSize = resource.get("size", 0).asUInt();

        uint32_t thisOffset = offset;
        offset += resourceSize;

        if (resourceType == "textureBuffer" && resourceFormat == "jpg" && !resourceName.empty() && resourceSize > 0)
            {
            in.SetPos(thisOffset);
            ByteCP buffer = in.GetCurrent();
            if (!in.Advance(resourceSize))
                {
                LOG_ERROR("Cannot read texture data");
                return ERROR;
                }

            ImageSource jpeg(ImageSource::Format::Jpeg, ByteStream(buffer, resourceSize));
            renderTextures[resourceName] = scene._CreateTexture(jpeg, Image::Format::Rgb, Image::BottomUp::Yes, renderSys);
            }
        }

    offset = (uint32_t)GetMagicString().size() + 4 + infoSize;
    for (JsonValueCR resource : resources)
        {
        resourceType = resource.get("type", "").asCString();
        resourceFormat = resource.get("format", "").asCString();
        resourceName = resource.get("id", "").asCString();
        resourceSize = resource.get("size", 0).asUInt();

        uint32_t thisOffset = offset;
        offset += resourceSize;

        if (resourceType == "geometryBuffer" && resourceFormat == "ctm" && !resourceName.empty() && resourceSize > 0)
            {
            if (geometryNodeCorrespondence.find(resourceName) == geometryNodeCorrespondence.end())
                {
                LOG_ERROR("Geometry is not referenced by any node");
                return ERROR;
                }

            Utf8String nodeName = geometryNodeCorrespondence[resourceName];
            auto nodeId = nodeIds.find(nodeName);
            if (nodeId == nodeIds.end())
                {
                LOG_ERROR("Node name is unknown");
                return ERROR;
                }

            CtmContext ctm(in, thisOffset);
            if (CTM_NONE != ctm.GetError())
                {
                LOG_ERROR("CTM read error: %s", ctmErrorString(ctm.GetError()));
                return ERROR;
                }

            uint32_t textureCoordsArrays = ctm.GetInteger(CTM_UV_MAP_COUNT);
            if (textureCoordsArrays != 1)
                continue;

            Utf8String texName = resource.get("texture", Json::Value("")).asCString();
            if (texName.empty())
                continue;

            Render::IGraphicBuilder::TriMeshArgs trimesh;
            trimesh.m_numPoints = ctm.GetInteger(CTM_VERTEX_COUNT);
            trimesh.m_points = ctm.GetFloatArray(CTM_VERTICES);
            trimesh.m_normals = (ctm.GetInteger(CTM_HAS_NORMALS) == CTM_TRUE) ? ctm.GetFloatArray(CTM_NORMALS) : nullptr;
            trimesh.m_numIndices = 3 * ctm.GetInteger(CTM_TRIANGLE_COUNT);
            trimesh.m_vertIndex = ctm.GetIntegerArray(CTM_INDICES);
            trimesh.m_textureUV = (FPoint2d const*)ctm.GetFloatArray(CTM_UV_MAP_1);

            auto texture = renderTextures.find(texName);
            trimesh.m_texture = (texture == renderTextures.end()) ? nullptr : texture->second;

            ((Node*)m_children[nodeId->second].get())->m_meshes.push_front(scene.CreateGeometry(trimesh, renderSys));
            }
        }
#endif


    if (!s_tryProgQuery && !s_tryCustomSelect)
        { 
        bvector<IScalableMeshNodePtr> childrenNodes(m_scalableMeshNodePtr->GetChildrenNodes());
    
        for (auto& childNode : childrenNodes)
            {
            SMNodePtr nodeptr = new SMNode(GetTriMeshRootR(), this, childNode);
            nodeptr->m_3smModel = m_3smModel;

            /*
               if (!nodeptr->ReadHeader(centroid))
               return ERROR;
               */
    /*
            if (loadChildren)
                {
                nodeptr->Read3SMTile(in, scene, renderSys, false);
                }
    */

            m_children.push_back(nodeptr);
            }
        }


    IScalableMeshMeshFlagsPtr loadFlagsPtr(IScalableMeshMeshFlags::Create(true, false));
    IScalableMeshMeshPtr smMeshPtr(m_scalableMeshNodePtr->GetMesh(loadFlagsPtr));

    if (!smMeshPtr.IsValid())
        return SUCCESS;

    const PolyfaceQuery* polyfaceQuery(smMeshPtr->GetPolyfaceQuery());

    TileTree::TriMeshTree::TriMesh::CreateParams trimesh;

    trimesh.m_numPoints = polyfaceQuery->GetPointIndexCount(); //smMeshPtr->GetNbPoints();
    FPoint3d* points = new FPoint3d[trimesh.m_numPoints];

    for (size_t pointInd = 0; pointInd < trimesh.m_numPoints; pointInd++)
        {
        DPoint3d resultPts;
        toFloatTransform.Multiply(resultPts, polyfaceQuery->GetPointCP()[polyfaceQuery->GetPointIndexCP()[pointInd] - 1]);
        
        points[pointInd].x = (float)resultPts.x;
        points[pointInd].y = (float)resultPts.y;
        points[pointInd].z = (float)resultPts.z;

        /*
           points[pointInd].x = (float)polyfaceQuery->GetPointCP()[pointInd].x;
           points[pointInd].y = (float)polyfaceQuery->GetPointCP()[pointInd].y;
           points[pointInd].z = (float)polyfaceQuery->GetPointCP()[pointInd].z;
           */
        }

    trimesh.m_points = points;

    trimesh.m_numIndices = polyfaceQuery->GetPointIndexCount();
    int* vertIndex = new int[trimesh.m_numIndices];

    _fPoint2d* textureUv;

    textureUv = new _fPoint2d[trimesh.m_numIndices];

    for (size_t faceVerticeInd = 0; faceVerticeInd < polyfaceQuery->GetPointIndexCount(); faceVerticeInd++)
        {
        vertIndex[faceVerticeInd] = faceVerticeInd; //polyfaceQuery->GetPointIndexCP()[faceVerticeInd] - 1;
        }  

    for (size_t paramInd = 0; paramInd < polyfaceQuery->GetPointIndexCount(); paramInd++)
        {
        const DPoint2d* uv = &polyfaceQuery->GetParamCP()[polyfaceQuery->GetParamIndexCP()[paramInd] - 1];
        textureUv[paramInd].x = uv->x;
        textureUv[paramInd].y = uv->y;
        }

    trimesh.m_vertIndex = vertIndex;

    if (s_applyTexture && renderSys != nullptr)
        {        
        //IScalableMeshTexturePtr smTexturePtr(m_scalableMeshNodePtr->GetTexture());

#if 1 //NEEDS_WORK_SM GetTextureCompressed doesn't currently work for Cesium. 
        IScalableMeshTexturePtr smTexturePtr(m_scalableMeshNodePtr->GetTexture());

        if (smTexturePtr.IsValid())
            {
            trimesh.m_textureUV = textureUv;
            ByteStream imageBytes(smTexturePtr->GetSize());

            size_t lineSize = smTexturePtr->GetDimension().x * 3;

            for (size_t indY = 0; indY < smTexturePtr->GetDimension().y; indY++)
                memcpy(imageBytes.GetDataP() + indY * lineSize, smTexturePtr->GetData() + (smTexturePtr->GetDimension().y - indY - 1) * lineSize, lineSize);
                                                        
            Image binaryImage(smTexturePtr->GetDimension().x, smTexturePtr->GetDimension().y, std::move(imageBytes), Image::Format::Rgb);

            Render::Texture::CreateParams params;
            params.SetIsTileSection();  // tile section have clamp instead of warp mode for out of bound pixels. That help reduce seams between tiles when magnified.            
            trimesh.m_texture = renderSys->_CreateTexture(binaryImage, scene.GetDgnDb(), params);
            }
#else

        IScalableMeshTexturePtr compressedTexturePtr(m_scalableMeshNodePtr->GetTextureCompressed());
        Image jpegImage(Image::FromJpeg(compressedTexturePtr->GetData(), compressedTexturePtr->GetSize(), Image::Format::Rgb));        
        ImageSource imageSource(jpegImage, ImageSource::Format::Jpeg);        
        trimesh.m_texture = renderSys->_CreateTexture(imageSource, Image::Format::Rgb, Image::BottomUp::Yes);
        trimesh.m_textureUV = textureUv;
#endif
        //ImageSource jpeg(ImageSource::Format::Jpeg, ByteStream(buffer, resourceSize));
        
                
        }

    Dgn::TileTree::TriMeshTree::TriMeshList triMeshList;
    scene.CreateGeometry(triMeshList, trimesh, renderSys);
    for (auto& meshEntry : triMeshList)
        {
        m_meshes.push_front(meshEntry);
        }

    delete [] trimesh.m_points;
    delete [] trimesh.m_vertIndex;
    delete [] trimesh.m_textureUV;

#if 0
    Render::IGraphicBuilder::TriMeshArgs trimesh;
    trimesh.m_numPoints = scalableMesh->GetNbPoints();
    trimesh.m_points = ctm.GetFloatArray(CTM_VERTICES);
    trimesh.m_normals = (ctm.GetInteger(CTM_HAS_NORMALS) == CTM_TRUE) ? ctm.GetFloatArray(CTM_NORMALS) : nullptr;
    trimesh.m_numIndices = 3 * ctm.GetInteger(CTM_TRIANGLE_COUNT);
    trimesh.m_vertIndex = ctm.GetIntegerArray(CTM_INDICES);
    trimesh.m_textureUV = (FPoint2d const*)ctm.GetFloatArray(CTM_UV_MAP_1);
#endif

    //BENTLEY_SM_EXPORT IScalableMeshTexturePtr GetTexture() const;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                   Mathieu.St-Pierre  08/17
 +---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SMScene::LoadNodeSynchronous(SMNodeR node)
    {
    auto result = _RequestTile(node, nullptr, nullptr, BeDuration());
    result.wait();
    return result.isReady() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                   Mathieu.St-Pierre  08/17
 +---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SMScene::LoadScene()
    {
    /*
       if (SUCCESS != ReadSceneFile())
       return ERROR;
       */

    if (!m_smPtr.IsValid())
        return ERROR;

    //CreateCache(m_sceneInfo.m_sceneName.c_str(), 1024 * 1024 * 1024); // 1 GB

    IScalableMeshNodePtr smNode(m_smPtr->GetRootNode());
    SMNode* root = new SMNode(*this, nullptr, smNode);
    //root->m_childPath = m_sceneInfo.m_rootNodePath;
    m_rootTile = root;
    root->m_3smModel = m_3smModel;

    auto result = _RequestTile(*root, nullptr, GetRenderSystemP(), BeDuration());
    result.wait(BeDuration::Seconds(2)); // only wait for 2 seconds
    return result.isReady() ? SUCCESS : ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Mathieu.St-Pierre  08/17
//----------------------------------------------------------------------------------------
BentleyStatus SMScene::LocateFromSRS()
    {
#if 0
    DgnGCSPtr bimGCS = m_db.GeoLocation().GetDgnGCS();
    if (!bimGCS.IsValid())
        return ERROR; // BIM is not geolocated, can't use geolocation in 3mx scene

    if (m_sceneInfo.m_reprojectionSystem.empty())
        return SUCCESS;  // scene has no spatial reference system, give up.

    WString    warningMsg;
    StatusInt  status, warning;

    DgnGCSPtr threeMxGCS = DgnGCS::CreateGCS(m_db);

    int epsgCode;
    double latitude, longitude;
    if (1 == sscanf(m_sceneInfo.m_reprojectionSystem.c_str(), "EPSG:%d", &epsgCode))
        status = threeMxGCS->InitFromEPSGCode(&warning, &warningMsg, epsgCode);
    else if (2 == sscanf(m_sceneInfo.m_reprojectionSystem.c_str(), "ENU:%lf,%lf", &latitude, &longitude))
        {
        // ENU specification does not impose any projection method so we use the first azimuthal available using values that will
        // mimic the intent (North is Y positive, no offset)
        // Note that we could have injected the origin here but keeping it in the transform as for other GCS specs
        if (latitude < 90.0 && latitude > -90.0 && longitude < 180.0 && longitude > -180.0)
            status = threeMxGCS->InitAzimuthalEqualArea(&warningMsg, L"WGS84", L"METER", longitude, latitude, 0.0, 1.0, 0.0, 0.0, 1);
        else
            status = ERROR;
        }
    else
        status = threeMxGCS->InitFromWellKnownText(&warning, &warningMsg, DgnGCS::wktFlavorEPSG, WString(m_sceneInfo.m_reprojectionSystem.c_str(), false).c_str());

    if (SUCCESS != status)
        {
        BeAssert(false && warningMsg.c_str());
        return ERROR;
        }

    // Compute a linear transform that approximates the reprojection transformation at the origin.
    Transform localTransform;
    status = threeMxGCS->GetLocalTransform(&localTransform, m_sceneInfo.m_origin, nullptr, true/*doRotate*/, true/*doScale*/, *bimGCS);

    // 0 == SUCCESS, 1 == Warning, 2 == Severe Warning,  Negative values are severe errors.
    if (status == 0 || status == 1)
        {
        m_location = localTransform;
        return SUCCESS;
        }
#endif

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                   Mathieu.St-Pierre  08/17
 +---------------+---------------+---------------+---------------+---------------+------*/
#if 0
BentleyStatus Scene::ReadSceneFile()
    {
    StreamBuffer rootStream;

    if (IsHttp())
        {
        TileTree::HttpDataQuery query(m_sceneFile, nullptr);
        query.Perform().wait();

        rootStream = std::move(query.GetData());
        }
    else
        {
        TileTree::FileDataQuery query(m_sceneFile, nullptr);
        rootStream = std::move(query.Perform().get());
        }

    return rootStream.HasData() ? m_sceneInfo.Read(rootStream) : ERROR;
    }
#endif


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  12/2016
//----------------------------------------------------------------------------------------
IScalableMeshProgressiveQueryEnginePtr ScalableMeshModel::GetProgressiveQueryEngine()
    {
    if (m_progressiveQueryEngine == nullptr)
        {
        m_displayNodesCache = new ScalableMeshDisplayCacheManager();
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
    
    assert(!"PushTransform not available on Bim02");

#if 0 
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
#endif
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshModel::SetClip(Dgn::ClipVectorCP clip)
    {
    m_clip = clip;
    if (m_root.IsValid())
        static_cast<SMSceneP>(m_root.get())->SetClip(clip);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::RootPtr ScalableMeshModel::_CreateTileTree(Render::SystemP system)
    {
    Utf8String sceneFile;

    Transform toLocationTransform;
    Transform toFloatTransform;

    if (m_smPtr.IsValid())
        {
        DRange3d range3D(m_smPtr->GetRootNode()->GetContentExtent());    
        DPoint3d centroid;
        centroid = DPoint3d::From((range3D.high.x + range3D.low.x) / 2.0, (range3D.high.y + range3D.low.y) / 2.0, (range3D.high.z + range3D.low.z) / 2.0);

#if 0 
        DPoint3d go = m_dgndb.GeoLocation().GetGlobalOrigin();

        GeoCoords::GCS gcs(m_smPtr->GetGCS());
        DgnGCSPtr  smGCS = DgnGCS::CreateGCS(gcs.GetGeoRef().GetBasePtr().get(), m_dgndb);

        DPoint3d scale = DPoint3d::FromXYZ(1, 1, 1);
        smGCS->UorsFromCartesian(scale, scale);        
                    
        scale.DifferenceOf(scale, go);

        smGCS->UorsFromCartesian(centroid, centroid);       

        toFloatTransform = Transform::FromRowValues(scale.x, 0, 0, -(centroid.x - go.x),
                                                    0, scale.y, 0, -(centroid.y - go.y),
                                                    0, 0, scale.z, -(centroid.z - go.z));

        
/*
        computedTransform = Transform::FromRowValues(scale.x, 0, 0, globalOrigin.x,
                                                     0, scale.y, 0, globalOrigin.y,
                                                     0, 0, scale.z, globalOrigin.z);
*/

        scale = DPoint3d::FromXYZ(1, 1, 1);

        location = Transform::FromRowValues(scale.x, 0, 0, centroid.x,
                                                     0, scale.y, 0, centroid.y,
                                                     0, 0, scale.z, centroid.z);

        //location = Transform::From(centroid.x + go.x, centroid.y + go.y, centroid.z + go.z);                                    
#endif

        m_smToModelUorTransform.Multiply(centroid, centroid);
        
        toFloatTransform = Transform::FromRowValues(1.0, 0, 0, -(centroid.x),
                                                    0, 1.0, 0, -(centroid.y),
                                                    0, 0, 1.0, -(centroid.z));

        toLocationTransform = Transform::FromRowValues(1.0, 0, 0, (centroid.x),
                                                       0, 1.0, 0, (centroid.y),
                                                       0, 0, 1.0, (centroid.z));

        toFloatTransform = Transform::FromProduct(toFloatTransform, m_smToModelUorTransform);
        //toLocationTransform = Transform::FromProduct(toLocationTransform, m_smToModelUorTransform);
        }
    else
        { 
        toLocationTransform = Transform::FromIdentity();
        toFloatTransform = Transform::FromIdentity();
        }

    SMScenePtr scene = new SMScene(m_dgndb, m_smPtr, toLocationTransform, toFloatTransform, sceneFile.c_str(), system);
    scene->m_3smModel = this;
    scene->SetPickable(true);
    if (SUCCESS != scene->LoadScene())
        return nullptr;

    scene->SetClip(m_clip.get());
    
    return scene.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
SMSceneP ScalableMeshModel::Load(Dgn::Render::SystemP renderSys) const
    {
    auto root = const_cast<ScalableMeshModel&>(*this).GetTileTree(renderSys);
    return static_cast<SMSceneP>(root);
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                   Mathieu.St-Pierre  08/17
 +---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshModel::_PickTerrainGraphics(Dgn::PickContextR context) const
    {
/*
    auto scene = Load(nullptr);
    if (nullptr == scene)
        return;

    //MST_TODO
    Dgn::ClipVectorCPtr clip;

    PickContext::ActiveDescription descr(context, GetName());
    scene->Pick(context, scene->GetLocation(), clip.get());
*/
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                   Mathieu.St-Pierre  08/17
 +---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshModel::_OnFitView(FitContextR context)
    {
    auto scene = Load(nullptr);
    if (nullptr == scene)
        return;

    ElementAlignedBox3d rangeWorld = scene->ComputeRange();
    context.ExtendFitRange(rangeWorld, scene->GetLocation());
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

    m_displayNodesCache = new ScalableMeshDisplayCacheManager();
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
        clipFileBase = BeFileName(ScalableMeshModel::GetTerrainModelPath(dgnProject, false)).GetDirectoryName();
        }
        
    Utf8Char modelIdStr[1000];
    BeStringUtilities::FormatUInt64(modelIdStr, GetModelId().GetValue());    
    clipFileBase.AppendToPath(WString(modelIdStr, true).c_str());
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
    
    DgnGCS* projGCS = dgnProject.GeoLocation().GetDgnGCS();
    DPoint3d globalOrigin = dgnProject.GeoLocation().GetGlobalOrigin();

    if (gcs.HasGeoRef())
        {
        DgnGCSPtr dgnGcsPtr(DgnGCS::CreateGCS(gcs.GetGeoRef().GetBasePtr().get(), dgnProject));
        dgnGcsPtr->UorsFromCartesian(scale, scale);
        scale.DifferenceOf(scale, globalOrigin);

        if (projGCS != nullptr && !projGCS->IsEquivalent(*dgnGcsPtr))
            {
            dgnGcsPtr->SetReprojectElevation(true);

            Transform trans = Transform::FromRowValues(scale.x, 0, 0, globalOrigin.x,
                                                         0, scale.y, 0, globalOrigin.y,
                                                         0, 0, scale.z, globalOrigin.z);

            DRange3d smExtent, smExtentUors;
            m_smPtr->GetRange(smExtent);
            trans.Multiply(smExtentUors, smExtent);

            DPoint3d extent;
            extent.DifferenceOf(smExtentUors.high, smExtentUors.low);
            Transform       approxTransform;

            auto coordInterp = m_smPtr->IsCesium3DTiles() ? Dgn::GeoCoordInterpretation::XYZ : Dgn::GeoCoordInterpretation::Cartesian;

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
                m_smToModelUorTransform = Transform::FromRowValues(scale.x, 0, 0, -globalOrigin.x,
                                                                   0, scale.y, 0, -globalOrigin.y,
                                                                   0, 0, scale.y, -globalOrigin.z);
                }
            }
        else
            {
            m_smToModelUorTransform = Transform::FromRowValues(scale.x, 0, 0, -globalOrigin.x,
                                                               0, scale.y, 0, -globalOrigin.y,
                                                               0, 0, scale.y, -globalOrigin.z);
            }
        }
    else
        {
        dgnProject.GeoLocation().GetDgnGCS()->UorsFromCartesian(scale, scale);
        assert(scale.x == 1 && scale.y == 1 && scale.z == 1);
        m_smToModelUorTransform = Transform::FromScaleFactors(scale.x, scale.y, scale.z);
        }

    m_smPtr->SetReprojection(*projGCS, m_smToModelUorTransform);


    
    m_storageToUorsTransfo = DMatrix4d::From(m_smToModelUorTransform);

    bool invertResult = m_modelUorToSmTransform.InverseOf(m_smToModelUorTransform);
    assert(invertResult);
    
    m_path = smFilename;
    if (m_smPtr->IsCesium3DTiles() && !(smFilename.ContainsI(L"realitydataservices") && smFilename.ContainsI(L"S3MXECPlugin")))
        {
        // The mesh likely comes from ProjectWiseContextShare, if it does then save that instead
        auto pwcsLink = BeFileName(m_smPtr->GetProjectWiseContextShareLink().c_str());
        if (!pwcsLink.empty()) m_path = pwcsLink;
        }

    // NEEDS_WORK_SM
    BeFileName dbFileName(dgnProject.GetDbFileName());
    BeFileName basePath = dbFileName.GetDirectoryName();
    //T_HOST.GetPointCloudAdmin()._CreateLocalFileId(m_properties.m_fileId, m_path, basePath);
    m_properties.m_fileId = Utf8String(m_path);
  
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
    //T_HOST.GetPointCloudAdmin()._CreateLocalFileId(m_properties.m_fileId, newFilename, basePath);
    m_properties.m_fileId = Utf8String(newFilename);
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
// @bsimethod                                                 Elenie.Godzaridis     4/2017
//----------------------------------------------------------------------------------------
ScalableMeshModelP ScalableMeshModel::CreateModel(BentleyApi::Dgn::DgnDbR dgnDb, WString terrainName, BeFileName terrainPath)
    {
    DgnClassId classId(dgnDb.Schemas().GetClassId("ScalableMesh", "ScalableMeshModel"));
    BeAssert(classId.IsValid());

    Utf8String linkName = Utf8String(terrainName); /*/BeFileName(rootUrl).GetFileNameWithoutExtension());*/    
    Utf8String terrainPathUtf8 = Utf8String(terrainPath);

    RepositoryLinkPtr repositoryLink = RepositoryLink::Create(*dgnDb.GetRealityDataSourcesModel(), terrainPathUtf8.c_str(), linkName.c_str());
    if (!repositoryLink.IsValid() || !repositoryLink->Insert().IsValid())
        return nullptr;

    ScalableMeshModelP model = new ScalableMeshModel(DgnModel::CreateParams(dgnDb, classId, repositoryLink->GetElementId()));

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
WString ScalableMeshModel::GetTerrainModelPath(BentleyApi::Dgn::DgnDbCR dgnDb, bool createDir)
    {
    BeFileName tmFileName;
    tmFileName = dgnDb.GetFileName().GetDirectoryName();
    tmFileName.AppendToPath(dgnDb.GetFileName().GetFileNameWithoutExtension().c_str());

    if (!tmFileName.DoesPathExist() && createDir)
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

void LoadAllScalableMeshModels(DgnDbCR database)
    {
    DgnClassId classId(database.Schemas().GetClassId("ScalableMesh", "ScalableMeshModel"));
    auto modelList = database.Models().MakeIterator("ScalableMeshModel");

    bvector<DgnModelId> modelsToLoad;
    for (auto& model : modelList)
        {
        if (model.GetClassId() == classId)
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
    LoadAllScalableMeshModels(GetDgnDb());
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

IMeshSpatialModelP ScalableMeshModelHandler::AttachTerrainModel(DgnDb& db, Utf8StringCR modelName, BeFileNameCR smFilename, RepositoryLinkCR modeledElement, bool openFile, ClipVectorCP clip, ModelSpatialClassifiersCP classifiers)
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

    if (nullptr != clip)
        model->SetClip(ClipVector::CreateCopy(*clip).get());

    if (nullptr != classifiers)
        model->SetClassifiers(*classifiers);
    
    //After Insert model pointer is handled by DgnModels.
    model->Insert();

    if (openFile)
        {
        model->OpenFile(smFilename, db);
        }
    else
        {
        model->SetFileNameProperty(smFilename);
        }

    //model->OpenFile(smFilename, db);
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
/*      SetCode doesn't exist on Bim02
        nameToSet = Utf8String(smFilename.GetFileNameWithoutExtension().c_str());
        DgnCode newModelCode(model->GetCode().GetAuthority(), nameToSet, NULL);
        model->SetCode(newModelCode);
        model->Update();
*/
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
// @bsimethod                                                   Mathieu.St-Pierre  07/2017
//----------------------------------------------------------------------------------------
void ScalableMeshModel::SetFileNameProperty(BeFileNameCR smFilename)
    {
    m_properties.m_fileId = smFilename.GetNameUtf8();
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

    if (m_clip.IsValid())
        val[json_clip()] = m_clip->ToJson();

    if (!m_classifiers.empty())
        val[json_classifiers()] = m_classifiers.ToJson();

    SetJsonProperties(json_scalablemesh(), val);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  12/2017
//----------------------------------------------------------------------------------------
bool IsUrl(WCharCP filename)
    {
    return NULL != filename && (0 == wcsncmp(L"http:", filename, 5) || 0 == wcsncmp(L"https:", filename, 6));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  03/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::_OnLoadedJsonProperties()
    {
    T_Super::_OnLoadedJsonProperties();

    Json::Value val(GetJsonProperties(json_scalablemesh()));

    m_properties.FromJson(val);

    if (val.isMember(json_clip()))
        m_clip = ClipVector::FromJson(val[json_clip()]);

    if (val.isMember(json_classifiers()))
        m_classifiers.FromJson(val[json_classifiers()]);

    if (m_smPtr == 0 && !m_tryOpen)
        {
        WString fileNameW(((this)->m_properties).m_fileId.c_str(), true);
        m_path = BeFileName(fileNameW);
                
        //NEEDS_WORK_SM : Doesn't work with URL
        if (BeFileName::DoesPathExist(m_path.c_str()) || IsUrl(m_path.c_str()))
            {
            OpenFile(m_path, GetDgnDb());
            }

        m_tryOpen = true;
        }
    
    if (m_smPtr.IsValid() && !m_smPtr->IsCesium3DTiles())
        {
        Json::Value publishingMetadata;
        publishingMetadata["name"] = "SMMasterHeader";

        IScalableMeshPublisher::Create(SMPublishType::CESIUM)->ExtractPublishMasterHeader(m_smPtr, publishingMetadata["properties"]);
        SetJsonProperties(json_publishing(), publishingMetadata);
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

