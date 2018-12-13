/*-------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshSchema/ScalableMeshHandler.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ScalableMeshSchemaPCH.h"
#include <ScalableMesh/ScalableMeshLib.h>
#include <ScalableMesh/IScalableMeshClippingOptions.h>
#include <BeSQLite/BeSQLite.h>
#include <ScalableMeshSchema/ScalableMeshHandler.h>
#include <ScalableMesh/GeoCoords/GCS.h>
#include <DgnPlatform/LinkElement.h>
#include <Bentley/BeDirectoryIterator.h>
#include <ScalableMesh/ScalableMeshLib.h>
#include <ScalableMesh/IScalableMeshSaveAs.h>
#include <ScalableMesh/ScalableMeshUtilityFunctions.h>
#include <DgnView/ViewManager.h>
#include <DgnView/DgnViewLib.h>

#if defined(_WIN32) && !defined(NDEBUG)
	#define _X86_
	#include <debugapi.h>
	#undef min
#endif



#define SCALABLEMESH_MODEL_PROP_Clips           "SmModelClips"
#define SCALABLEMESH_MODEL_PROP_GroundCoverages "SmGroundCoverages"
#define SCALABLEMESH_MODEL_PROP_ClipVectors "SmModelClipVectors"

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

#ifndef NDEBUG
    static int s_activeNodeCounter = 0;
    static int s_activeLoadedNodeCounter = 0;
#endif



//----------------------------------------------------------------------------------------
// @bsimethod                                                 Mathieu.St-Pierre     3/2017
//----------------------------------------------------------------------------------------
IScalableMeshLocationProviderPtr ScalableMeshModel::m_locationProviderPtr = nullptr;

BentleyStatus IScalableMeshLocationProvider::GetExtraFileDirectory(BeFileNameR extraFileDir, DgnDbCR dgnDb) const
    {
    return _GetExtraFileDirectory(extraFileDir, dgnDb);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  12/2017
//----------------------------------------------------------------------------------------
bool IsSMNameUrl(WCharCP filename)
    {
    return NULL != filename && (0 == wcsncmp(L"http:", filename, 5) || 0 == wcsncmp(L"https:", filename, 6));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Mathieu.St-Pierre    06/2018
//----------------------------------------------------------------------------------------
BentleyStatus ResolveFileName(BeFileNameR fileName, Utf8StringCR fileId, DgnDbCR db)
    {
    WString fileNameW(fileId.c_str(), true);
    BeFileName fileIdWStr(fileNameW);

    if (BeFileName::DoesPathExist(fileIdWStr.c_str()) || IsSMNameUrl(fileIdWStr.c_str()))
        {               
        fileName = fileIdWStr;
        return SUCCESS;        
        }
    
    BeFileName dbFileName(db.GetDbFileName());
    BeFileName relativeFileName = dbFileName.GetDirectoryName();            
    relativeFileName.AppendString(fileIdWStr.GetFileNameAndExtension().c_str());

    if (BeFileName::DoesPathExist(relativeFileName.c_str()))
        {
        fileName = relativeFileName;            
        return SUCCESS;                
        }

    return ERROR;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
PublishedTilesetInfo ScalableMeshModel::_GetPublishedTilesetInfo()
    {
    if (m_smPtr.IsNull())
        return PublishedTilesetInfo();

    return PublishedTilesetInfo(Utf8String(GetPath()), m_smPtr->GetRootNode()->GetContentExtent());
    }

//*---------------------------------------------------------------------------------**//**
/* @bsimethod                                                    Richard.Bois   05/18
/+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshModel::_DoPublish(BeFileNameCR outDir, WString rootName, DRange3dR range, const Transform& tileToECEF, const Transform& dbToTile)
    {
    BeFileName rootPath = outDir;
    rootPath.AppendToPath(rootName.c_str());
    WriteCesiumTileset(rootPath, outDir, tileToECEF, dbToTile);
    m_smPtr->GetRange(range);
    }

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

#if defined(ANDROID) || defined(__APPLE__)
    static double s_maxPixelError = 10;
    static double s_maxPixelErrorStreamingTexture = 10;
#else
 static double s_maxPixelError = 3;
    static double s_maxPixelErrorStreamingTexture = 3;
#endif

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

#ifdef _WIN32    
    IScalableMeshPublisher::Create(SMPublishType::CESIUM)->ExtractPublishNodeHeader(m_scalableMeshNodePtr, data);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre  08/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_tryCustomSelect = true;

Tile::ChildTiles const* SMNode::_GetChildren(bool load) const
    { 
    if (!s_tryCustomSelect)
        return T_Super::_GetChildren(load);
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
* @bsimethod                                                    Mathieu.St-Pierre  08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void SMNode::_OnChildrenUnloaded() const
    {   
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre  08/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_unloadChildren = true;

void SMNode::_UnloadChildren(BeTimePoint olderThan) const
    {
    if (s_unloadChildren)
        {
        if (!m_canUnloadChildren)
            {
            for (auto const& child : m_children)
                {
#if defined(ANDROID) || defined(__APPLE__)
                child->_UnloadChildren(BeTimePoint::Now());
#else
                child->_UnloadChildren(olderThan);
#endif
                }
            }
#if defined(ANDROID) || defined(__APPLE__)
        else
            {
            if (true /*IsReady() || IsNotLoaded()*/)
                {                                      
                TileTree::Tile::_UnloadChildren(BeTimePoint::Now());
                }
            
            return;
            }
#endif        



        T_Super::_UnloadChildren(olderThan);
        }
    }

#if defined(ANDROID) || defined(__APPLE__)
void SMNode::CleanupUnusedChildren(bvector<Dgn::TileTree::TileCPtr>& selected) const
    {
    if (s_unloadChildren)
        {
        if (m_canUnloadChildren)
            {
            if (true /*IsReady() || IsNotLoaded()*/)
                {
                  //static BeTimePoint Now() {return std::chrono::steady_clock::now();}
                //TileTree::Tile::_UnloadChildren(olderThan);
                /*
                for (auto const& child : m_children)
                    {
                    int refCount = child->GetRefCount();
                    assert(refCount == 1);
                    refCount = refCount;
                    }
                    */

                for (auto const& child : m_children)
                    {
                    SMNode* node = static_cast<SMNode*>(child.get());
                    
                    if ((node->m_meshes.begin() != node->m_meshes.end()))
                        {                        
                        bool found = false;

                        for (auto const& selectChild : selected)
                            {
                            const SMNode* selectNode = static_cast<const SMNode*>(selectChild.get());

                            if (node->m_scalableMeshNodePtr->GetNodeId() == selectNode->m_scalableMeshNodePtr->GetNodeId())
                                {
                                found = true;
                                break;
                                }                                                
                            }

                        if (!found)
                            {
                            node->ClearGeometry();   
                            node->SetNotLoaded();            
#ifndef NDEBUG
                            s_activeLoadedNodeCounter--;
#if defined(_WIN32)                            
                            if (selected.size() == 0)
                                {
                                wchar_t text_buffer[1000] = { 0 }; //temporary buffer        
                                swprintf(text_buffer, _countof(text_buffer), L"CLEANUP NODE : %I64d \r\n", node->m_scalableMeshNodePtr->GetNodeId()); // convert
                                OutputDebugStringW(text_buffer); // print
                                }
#endif
#endif
                            }
                        }

                    node->CleanupUnusedChildren(selected);
                    }                    
                }      

            return;
            }
        else
            {
            for (auto const& child : m_children)
                {
                SMNode* node = static_cast<SMNode*>(child.get());
                node->CleanupUnusedChildren(selected);
                }
            }
        }
    }
#endif

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

#ifndef NDEBUG
    if (m_meshes.begin() != m_meshes.end())
        s_activeLoadedNodeCounter++;
#endif

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.St-Pierre  08/18
+---------------+---------------+---------------+---------------+---------------+------*/
#ifndef NDEBUG
static double s_firstNodeSearchingDelay = (double)1 / 15 * CLOCKS_PER_SEC;
#else
//static double s_firstNodeSearchingDelay = (double)1 / 10 * CLOCKS_PER_SEC;
static double s_firstNodeSearchingDelay = (double)1 / 60 * CLOCKS_PER_SEC;
#endif

#if defined(ANDROID) || defined(__APPLE__)
    static bool s_clearGeometry = true;
#endif

Dgn::TileTree::Tile::SelectParent SMNode::SelectViewTiles(bvector<Dgn::TileTree::TileCPtr>& selected, Dgn::TileTree::DrawArgsR args, bool& parentSelected, clock_t& startTime, IScalableMeshViewDependentMeshQueryParamsPtr& viewDependentQueryParams) const
    {
    DgnDb::VerifyClientThread();

    SMNodeViewStatus viewStatus = m_scalableMeshNodePtr->IsCorrectForView(viewDependentQueryParams);

    if (viewStatus == SMNodeViewStatus::NotVisible)
        {     
#if defined(ANDROID) || defined(__APPLE__)
        if (s_clearGeometry)
            {
            if (this->GetParent() != nullptr && static_cast<const SMNode*>(this->GetParent())->m_canUnloadChildren && (m_meshes.begin() != m_meshes.end()))
                {
                SMNode* node = const_cast<SMNode*>(this);
                node->ClearGeometry();   
                node->SetNotLoaded();    
#ifndef NDEBUG
                s_activeLoadedNodeCounter--;
#endif
                }
            }
#endif
                    
        _UnloadChildren(args.m_purgeOlderThan);
        return SelectParent::No;
        }

    if ((clock() - startTime) > s_firstNodeSearchingDelay && (m_parent != nullptr) && m_parent->IsReady() && m_parent->_HasGraphics())
        {
        args.InsertMissing(*this);
        parentSelected = true;
        return SelectParent::Yes;
        }

    if (viewStatus == SMNodeViewStatus::Fine)
        {
#if 0
        if (s_showTiles)
            {
            DRange3d contentExtent(m_scalableMeshNodePtr->GetContentExtent());
            /**
            if (isCesium)
            {
            context.PopTransformClip();

            DPoint3d box[8];
            contentExtent.Get8Corners(box);
            smToDgnUorTransform.Multiply(box, 8);
            contentExtent = DRange3d::From(box, 8);
            }
            */

            int64_t  nodeId(m_scalableMeshNodePtr->GetNodeId());

            TextString nodeIdString;

            DPoint3d stringOrigin = { (contentExtent.high.x + contentExtent.low.x) / 2, (contentExtent.high.y + contentExtent.low.y) / 2, contentExtent.high.z };

            char buffer[1000];
            BeStringUtilities::FormatUInt64(buffer, nodeId);

            nodeIdString.SetOrigin(stringOrigin);
            nodeIdString.SetText(buffer);

            TextStringStyle stringStyle;
            double maxExtentDim = std::max(contentExtent.XLength(), contentExtent.YLength());
            int textSize = std::max((int)(maxExtentDim / s_tileSizePerIdStringSize), 1);
            stringStyle.SetSize(textSize);
            nodeIdString.SetStyle(stringStyle);
            /*
            ElemMatSymbP matSymbP = args.m_context.GetElemMatSymb();
            matSymbP->Init();
            matSymbP->SetLineColor(ColorDef::Red());
            matSymbP->SetFillColor(ColorDef::Red());
            args.m_context.GetIDrawGeom().ActivateMatSymb(matSymbP);
            */
            args.m_context.GetIDrawGeom().DrawTextString(nodeIdString);

            DPoint3d box[8];
            contentExtent.Get8Corners(box);
            std::swap(box[6], box[7]);
            box[3] = box[7];

            args.m_context.GetIDrawGeom().DrawLineString3d(5, &box[3], nullptr);

            /*
            if (isCesium)
            {
            context.PushTransform(smToDgnUorTransform);
            }
            */
            }
#endif

        _UnloadChildren(args.m_purgeOlderThan);

        if (IsReady())
            {
            selected.push_back(this);
            return SelectParent::No;
            }
        else
            {            
            args.InsertMissing(*this);
            parentSelected = true;
            return SelectParent::Yes;
            }
        }

    assert(viewStatus == SMNodeViewStatus::TooCoarse);
    //if (viewStatus == SMNodeViewStatus::TooCoarse)        
    bool drawParent = false;

    auto children = _GetChildren(true);
    size_t sizeBeforeChildren = selected.size();

    if (nullptr != children)
        {
        for (auto const& child : *children)
            {            
            if (SelectParent::Yes == ((SMNode*)child.get())->SelectViewTiles(selected, args, parentSelected, startTime, viewDependentQueryParams))
                {
                drawParent = true;
                // NB: We must continue iterating children so that they can be requested if missing...
                }
            }
        }

    if (!drawParent)
        {
        if (sizeBeforeChildren < selected.size())
            {
            return SelectParent::No;
            }
        }

    if (IsReady() && _HasGraphics())
        {
        //if (_HasGraphics())
        selected.push_back(this);
        return SelectParent::No;
        }
    else if (_HasBackupGraphics())
        {
        // Caching previous graphics while regenerating tile to reduce flishy-flash when model changes.
        selected.push_back(this);
        return SelectParent::No;
        }

#if defined(ANDROID) || defined(__APPLE__)
    if (s_clearGeometry)
        {        
        if (this->GetParent() != nullptr && static_cast<const SMNode*>(this->GetParent())->m_canUnloadChildren && (m_meshes.begin() != m_meshes.end()))
            {
            SMNode* node = const_cast<SMNode*>(this);
            node->ClearGeometry(); 
            node->SetNotLoaded();           
#ifndef NDEBUG
            s_activeLoadedNodeCounter--;
#endif
            }                
        }
#endif

    parentSelected = true;
    args.InsertMissing(*this);
    return SelectParent::Yes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.St-Pierre  12/17
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::TileTree::Tile::SelectParent SMNode::_SelectTiles(bvector<Dgn::TileTree::TileCPtr>& selected, Dgn::TileTree::DrawArgsR args) const
    {

    if (!m_3smModel->m_loadedAllModels)
        {
        m_3smModel->InitializeTerrainRegions(/*args.m_context*/);
        }
    
    if (s_tryCustomSelect)
        {   
        assert(selected.size() == 0);
        assert(args.m_missing.size() == 0);
                        
        
        clock_t startTime = 0;
        IScalableMeshViewDependentMeshQueryParamsPtr viewDependentQueryParams;
        
        startTime = clock();
        
        viewDependentQueryParams = IScalableMeshViewDependentMeshQueryParams::CreateParams();

        DMatrix4d localToView(args.GetContext().GetWorldToView().M0);

        ClipVectorPtr clipVector;
        //clip = args.m_context.GetTransformClipStack().GetClip();
        Render::FrustumPlanes frustumPlanes(args.GetContext().GetFrustumPlanes());

        ConvexClipPlaneSet convexClipPlaneSet(&frustumPlanes.m_planes[0], 6);

        ClipPlaneSet clipPlaneSet(convexClipPlaneSet);

        ClipPrimitivePtr clipPrimitive(ClipPrimitive::CreateFromClipPlanes(clipPlaneSet));

        clipVector = ClipVector::CreateFromPrimitive(clipPrimitive.get());


        DMatrix4d smToUOR = DMatrix4d::From(m_3smModel->m_smToModelUorTransform);

        localToView.InitProduct (localToView, smToUOR);
        
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

        bool parentSelected = false;
        
        Dgn::TileTree::Tile::SelectParent selectParent = SelectViewTiles(selected, args, parentSelected, startTime, viewDependentQueryParams);

        if (parentSelected && args.m_missing.size() == 0)
            {
            parentSelected = true;
            }        

#if defined(ANDROID) || defined(__APPLE__)
        CleanupUnusedChildren(selected);
#endif
         
#if !defined(NDEBUG) && defined(_WIN32)                            
        wchar_t text_buffer[1000] = { 0 }; //temporary buffer
        swprintf(text_buffer, _countof(text_buffer), L"Nb SelectedTiles : %zd    Nb MissingTiles : %zd\r\n", selected.size(), args.m_missing.size()); // convert
        OutputDebugStringW(text_buffer); // print
        swprintf(text_buffer, _countof(text_buffer), L"ActiveNodeCounter : %d    ActiveLoadedNodeCounter : %d\r\n", s_activeNodeCounter, s_activeLoadedNodeCounter); // convert
        OutputDebugStringW(text_buffer); // print
#endif
                        
        return selectParent;
        }
          
    return T_Super::_SelectTiles(selected, args);
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                   Mathieu.St-Pierre  08/17
 +---------------+---------------+---------------+---------------+---------------+------*/
bool SMNode::ReadHeader(Transform& locationTransform)
    {
    m_range.low = m_scalableMeshNodePtr->GetContentExtent().low;
    m_range.high = m_scalableMeshNodePtr->GetContentExtent().high;

    locationTransform.Multiply(m_range.low);
    locationTransform.Multiply(m_range.high);
  
    float geometricResolution;
    float textureResolution;

    m_scalableMeshNodePtr->GetResolutions(geometricResolution, textureResolution);
        
	//The formula below should give the same value as maxScreenDiameter in the 3mxb.
    m_maxDiameter = m_range.low.Distance(m_range.high) / std::min(geometricResolution, textureResolution);
	   
    return true;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                    Mathieu.St-Pierre  12/18
//----------------------------------------------------------------------------------------
SMNode::SMNode(Dgn::TileTree::TriMeshTree::Root& root, SMNodeP parent, IScalableMeshNodePtr& smNodePtr) 
    : T_Super(root, parent), 
      m_scalableMeshNodePtr(smNodePtr) 
    {
#ifndef NDEBUG
    s_activeNodeCounter++; 
#endif
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Mathieu.St-Pierre  12/17
//----------------------------------------------------------------------------------------
SMNode::~SMNode()
    {
#ifndef NDEBUG
    s_activeNodeCounter--;
    
    if (m_meshes.begin() != m_meshes.end())
        s_activeLoadedNodeCounter--;    
#endif
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Mathieu.St-Pierre  08/17
//----------------------------------------------------------------------------------------
static bool s_applyTexture = true;
static bool s_applyGeometry = true;
static bool s_applyClips = true;

BentleyStatus SMNode::DoRead(StreamBuffer& in, SMSceneR scene, Dgn::Render::SystemP renderSys, bool loadChildren)
    {

    if (!m_3smModel->m_loadedAllModels)
        {
        m_3smModel->InitializeTerrainRegions(/*args.m_context*/);
        }

    m_loadStatus.store(LoadStatus::Loading);

    //BeAssert(m_children.empty()); 


    Transform toFloatTransform(scene.GetToFloatTransform());

    if (!ReadHeader(toFloatTransform))
        return ERROR;

    if (!s_tryCustomSelect)
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
    
    IScalableMeshMeshPtr smMeshPtr;

    if (!s_applyClips)
        {
        IScalableMeshMeshFlagsPtr loadFlagsPtr(IScalableMeshMeshFlags::Create(true, false));
        loadFlagsPtr->SetLoadClips(false);
        loadFlagsPtr->SetSaveToCache(true);
        smMeshPtr = m_scalableMeshNodePtr->GetMesh(loadFlagsPtr);
        }
    else
        {
        IScalableMeshMeshFlagsPtr loadFlagsPtr(IScalableMeshMeshFlags::Create(true, false));
        loadFlagsPtr->SetLoadClips(true);
        loadFlagsPtr->SetSaveToCache(false); 
        loadFlagsPtr->SetClipsToShow(m_3smModel->m_activeClips, m_3smModel->m_smPtr->ShouldInvertClips());
        if (!m_scalableMeshNodePtr->IsDataUpToDate()) m_scalableMeshNodePtr->UpdateData();
        m_scalableMeshNodePtr->RefreshMergedClip(scene.m_smPtr->GetReprojectionTransform());
        smMeshPtr = m_scalableMeshNodePtr->GetMesh(loadFlagsPtr);
        }
        
    if (!smMeshPtr.IsValid())
        return SUCCESS;

    const PolyfaceQuery* polyfaceQuery(smMeshPtr->GetPolyfaceQuery());

    TileTree::TriMeshTree::TriMesh::CreateParams trimesh;

    trimesh.m_numPoints = polyfaceQuery->GetPointIndexCount(); 
    FPoint3d* points = new FPoint3d[trimesh.m_numPoints];

    for (size_t pointInd = 0; pointInd < trimesh.m_numPoints; pointInd++)
        {
        DPoint3d resultPts;
        toFloatTransform.Multiply(resultPts, polyfaceQuery->GetPointCP()[polyfaceQuery->GetPointIndexCP()[pointInd] - 1]);
        
        points[pointInd].x = (float)resultPts.x;
        points[pointInd].y = (float)resultPts.y;
        points[pointInd].z = (float)resultPts.z;
        }

    trimesh.m_points = points;

    trimesh.m_numIndices = polyfaceQuery->GetPointIndexCount();
    int* vertIndex = new int[trimesh.m_numIndices];

    for (int faceVerticeInd = 0; faceVerticeInd < trimesh.m_numIndices; faceVerticeInd++)
        {
        vertIndex[faceVerticeInd] = faceVerticeInd;
        }

    trimesh.m_vertIndex = vertIndex;

   
    if (s_applyTexture && renderSys != nullptr && m_scalableMeshNodePtr->IsTextured() && m_3smModel->m_displayTexture)
        {                  
#if 1 //NEEDS_WORK_SM GetTextureCompressed doesn't currently work for Cesium. 
        IScalableMeshTexturePtr smTexturePtr(m_scalableMeshNodePtr->GetTexture());

        if (smTexturePtr.IsValid())
            {
            _fPoint2d* textureUv;

            textureUv = new _fPoint2d[trimesh.m_numIndices];

            for (size_t paramInd = 0; paramInd < trimesh.m_numIndices; paramInd++)
                {
                const DPoint2d* uv = &polyfaceQuery->GetParamCP()[polyfaceQuery->GetParamIndexCP()[paramInd] - 1];
                textureUv[paramInd].x = uv->x;
                textureUv[paramInd].y = uv->y;
                }

            smMeshPtr = nullptr;

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
        else
            {
            smMeshPtr = nullptr;
            }
#else

        IScalableMeshTexturePtr compressedTexturePtr(m_scalableMeshNodePtr->GetTextureCompressed());
        Image jpegImage(Image::FromJpeg(compressedTexturePtr->GetData(), compressedTexturePtr->GetSize(), Image::Format::Rgb));        
        ImageSource imageSource(jpegImage, ImageSource::Format::Jpeg);        
        trimesh.m_texture = renderSys->_CreateTexture(imageSource, Image::Format::Rgb, Image::BottomUp::Yes);
        trimesh.m_textureUV = textureUv;
#endif                                
        }
    else
        {
        smMeshPtr = nullptr;
        }

    Dgn::TileTree::TriMeshTree::TriMeshList triMeshList;
    scene.CreateGeometry(triMeshList, trimesh, renderSys);
    for (auto& meshEntry : triMeshList)
        {
        m_meshes.push_front(meshEntry);
        }

    delete [] trimesh.m_points;
    delete [] trimesh.m_vertIndex;

    if (trimesh.m_textureUV)
        delete [] trimesh.m_textureUV;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
SMScene::SMScene(ScalableMeshModelCR model, IScalableMeshPtr& smPtr, TransformCR location, TransformCR toFloatTransform, Utf8CP sceneFile, Dgn::Render::SystemP system)
    : T_Super(model, location, sceneFile, system), m_smPtr(smPtr), m_toFloatTransform(toFloatTransform)
    {
#if !defined(NDEBUG) && defined(_WIN32)
    wchar_t text_buffer[1000] = { 0 }; //temporary buffer
    swprintf(text_buffer, _countof(text_buffer), L"SMSCENE CREATION : ActiveNodeCounter : %d    ActiveLoadedNodeCounter : %d\r\n", s_activeNodeCounter, s_activeLoadedNodeCounter); // convert
    OutputDebugStringW(text_buffer); // print        
#endif
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
* @bsimethod                                                   Mathieu.St-Pierre  08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void SMScene::LoadOverview(SMNode* node)
    {
    StreamBuffer in;

    node->Read3SMTile(in, *this, GetRenderSystemP(), false);

    if (!node->_HasGraphics())
        {
        node->m_canUnloadChildren = false;

        Tile::ChildTiles const* children = node->_GetChildren(true);

        if (nullptr != children)
            {
            for (auto& childNode : *children)
                {            
                LoadOverview((SMNode*)childNode.get());
                }
            }
        }
    
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                   Mathieu.St-Pierre  08/17
 +---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SMScene::LoadScene()
    {    
    if (!m_smPtr.IsValid())
        return ERROR;    

    IScalableMeshNodePtr smNode(m_smPtr->GetRootNode());
    SMNode* root = new SMNode(*this, nullptr, smNode);    
    m_rootTile = root;
    root->m_3smModel = m_3smModel;

    LoadOverview(root);

#if !defined(NDEBUG) && defined(_WIN32)
    wchar_t text_buffer[1000] = { 0 }; //temporary buffer
    swprintf(text_buffer, _countof(text_buffer), L"SMSCENE CREATION AFTER LOAD OVERVIEW: ActiveNodeCounter : %d    ActiveLoadedNodeCounter : %d\r\n", s_activeNodeCounter, s_activeLoadedNodeCounter); // convert
    OutputDebugStringW(text_buffer); // print     
#endif
   
    /*
    auto result = _RequestTile(*root, nullptr, GetRenderSystemP(), BeDuration());
    result.wait(BeDuration::Seconds(2)); // only wait for 2 seconds
    return result.isReady() ? SUCCESS : ERROR;
    */
    return SUCCESS;
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
    m_smPtr->RemoveAllDisplayData();    
    RefreshClips();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre 01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshModel::SetScalableClips(bmap <uint64_t, SMModelClipInfo>& clipInfo)
    {
    m_scalableClipDefs = std::move(clipInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre 01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshModel::SetGroundModelLinks(bvector<bpair<uint64_t, uint64_t>>& linksToGroundModels)
    {
    m_linksToGroundModels = std::move(linksToGroundModels);
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
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::RootPtr ScalableMeshModel::_GetTileTree(RenderContextR context)
    {
    return GetTileTree(context.GetRenderSystem());
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

    SMScenePtr scene = new SMScene(*this, m_smPtr, toLocationTransform, toFloatTransform, sceneFile.c_str(), system);
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

std::recursive_mutex s_loadModelLock;

void LoadAllScalableMeshModels(DgnDbCR database)
    {
    s_loadModelLock.lock();
    DgnClassId classId(database.Schemas().GetClassId("ScalableMesh", "ScalableMeshModel"));
    //auto modelList = database.Models().MakeIterator(BIS_SCHEMA("ScalableMesh"));
    auto modelList = database.Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialModel));
   

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

    s_loadModelLock.unlock();
    }

void ScalableMeshModel::GetAllScalableMeshes(BentleyApi::Dgn::DgnDbCR dgnDb, bvector<IMeshSpatialModelP>& models)
    {
    DgnClassId classId(dgnDb.Schemas().GetClassId("ScalableMesh", "ScalableMeshModel"));
    BeAssert(classId.IsValid());

    //LoadAllScalableMeshModels(dgnDb);

    auto modelIter = dgnDb.Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialModel));

    //ModelIterator modelIter(dgnDb.Models().MakeIterator("ScalableMesh"));
    bvector<DgnModelId> modelIds(modelIter.BuildIdList());

    for (auto& modelId : modelIds)
        {
        //DgnModelPtr modelPtr(dgnDb.Models().FindModel(modelId));
        DgnModelPtr modelPtr(dgnDb.Models().GetModel(modelId));        

        if (modelPtr.IsValid() && modelPtr->GetClassId() == classId)
            {
            //assert(modelPtr.IsValid() && modelPtr->GetClassId() == classId);                    
            models.push_back(dynamic_cast<IMeshSpatialModelP>(modelPtr.get()));
            }
        }
/*
    for (auto& model : dgnDb.Models().GetLoadedModels())
        {
        if (model.second->GetClassId() == classId) models.push_back(dynamic_cast<IMeshSpatialModelP>(model.second.get()));
        }
*/
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
    smtFileName.AppendString(L"//terrain.3sm");
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
    ScalableMeshTerrainModelAppData* appData(ScalableMeshTerrainModelAppData::Get(GetDgnDb()));
    if (appData != nullptr && appData->m_smTerrainPhysicalModelP == this)
        ScalableMeshTerrainModelAppData::Delete(GetDgnDb());
    //ClearProgressiveQueriesInfo();

    if (m_smPtr.IsValid())
        {                
        bvector<BeFileName> extraFileNames;

        if (isModelDelete)
            m_smPtr->GetExtraFileNames(extraFileNames);

        //Close the 3SM file, to close extra clip files.
        m_smPtr = nullptr;        

        for (auto& extraFileName : extraFileNames)
            {
            Utf8String fileNameUtf8(extraFileName.c_str());
            remove(fileNameUtf8.c_str());
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


struct ScalableMeshHandlerViewDecoration : public ViewDecoration
    {
    private: 

        //----------------------------------------------------------------------------------------
        // @bsimethod                                                   Mathieu.St-Pierre  08/2017
        //----------------------------------------------------------------------------------------
        void DrawBingLogo(DecorateContextR context, Byte const* pBitmapRGBA, DPoint2d const& bitmapSize)
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
            bitmapInView[2].x = bitmapDrawSize.x;
            bitmapInView[2].y = 0;
            bitmapInView[3].x = 0;
            bitmapInView[3].y = 0;
            bitmapInView[0].z = bitmapInView[1].z = bitmapInView[2].z = bitmapInView[3].z = 0;

            Transform correctedViewToView;

            GetBingLogoInfo(correctedViewToView, context);

            correctedViewToView.Multiply(bitmapInView, 4);
  

            DPoint3d bitmapInLocal[4];
            //context.ViewToLocal(bitmapInLocal, bitmapInView, 4);
            context.ViewToWorld(bitmapInLocal, bitmapInView, 4);

            bitmapInView[0].z = bitmapInView[1].z = bitmapInView[2].z = bitmapInView[3].z = 0/*context.GetViewport()->GetActiveZRoot()*/;

            // When raster is drawn by D3D, the texture is modulated by the active color (the materal fill color). 
            // Override the material fill color for raster to white so that the appearance won't mysteriously change in the future.
    #if 0
            ElemMatSymbP matSymb = context.GetElemMatSymb();

            ColorDef color(0xff, 0xff, 0xff, 0x01);
            matSymb->SetLineColor(color);
            matSymb->SetFillColor(color);
            context.GetIDrawGeom().ActivateMatSymb(matSymb);
    #endif

            size_t imageDataSize = bitmapSize.x * bitmapSize.y * 4;
            ByteStream imageBytes(imageDataSize);

            memcpy(imageBytes.GetDataP(), pBitmapRGBA, imageDataSize);

            Image binaryImage(bitmapSize.x, bitmapSize.y, std::move(imageBytes), Image::Format::Rgba);

            Render::Texture::CreateParams params;
            params.SetIsTileSection();  // tile section have clamp instead of warp mode for out of bound pixels. That help reduce seams between tiles when magnified.            
            TexturePtr texturePtr(context.CreateTexture(binaryImage));

            Material::CreateParams matParams;

            TextureMapping::Params textureMappingParams;
            matParams.MapTexture(*texturePtr, textureMappingParams);

            MaterialPtr matPtr(context.GetRenderSystem()->_CreateMaterial(matParams, context.GetDgnDb()));

            auto graphic = context.CreateWorldOverlay();

            GraphicParams graphicParams;
            graphicParams.SetMaterial(matPtr.get());

            graphic->SetSymbology(ColorDef::Yellow(), ColorDef::Yellow(), 8);
            graphic->ActivateGraphicParams(graphicParams);
            graphic->AddShape(4, bitmapInLocal, true);
            context.AddWorldOverlay(*graphic->Finish());
            }

    public: 

    virtual ~ScalableMeshHandlerViewDecoration() override {}

    virtual void _DrawDecoration(DecorateContextR context) override 
        {
        DPoint2d bitmapSize;

        const Byte* pBitmap = IScalableMeshTextureInfo::GetBingMapLogo(bitmapSize);

        if (NULL != pBitmap)
            DrawBingLogo(context, pBitmap, bitmapSize);                
        }

    virtual void _PickDecoration(PickContextR) override  {}        
    };


ScalableMeshHandlerViewDecoration s_viewDecoration; 


//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::OpenFile(BeFileNameCR smFilename, DgnDbR dgnProject)
    {
    assert(m_smPtr == nullptr);    
    
    BeFileName clipFileBase = GenerateClipFileName(smFilename, dgnProject);

    m_basePath = clipFileBase;
#if !defined(_WIN32)
	StatusInt status;
	m_smPtr = IScalableMesh::GetFor(smFilename, Utf8String(clipFileBase.c_str()),false, false, true, status);
#else
    m_smPtr = IScalableMesh::GetFor(smFilename, Utf8String(clipFileBase.c_str()), false, true);
#endif
    
    if (!m_smPtr.IsValid())
        {        
        if (Utf8String::IsNullOrEmpty(m_properties.m_fileId.c_str()))
            m_properties.m_fileId = Utf8String(smFilename);

        return;
        }

    //if (m_smPtr->IsTerrain())
        {
         ScalableMeshTerrainModelAppData* appData = ScalableMeshTerrainModelAppData::Get(m_dgndb);
         if (appData->m_smTerrainPhysicalModelP == nullptr)
             {
             appData->m_smTerrainPhysicalModelP = this;
             appData->m_modelSearched = true;
             }
        }
    
    DgnGCS* projGCS = dgnProject.GeoLocation().GetDgnGCS();
    m_smPtr->Reproject(projGCS, dgnProject);

    m_smToModelUorTransform = m_smPtr->GetReprojectionTransform();
        
    bool invertResult = m_modelUorToSmTransform.InverseOf(m_smToModelUorTransform);
    assert(invertResult);
    
    m_path = smFilename;
    if (m_smPtr->IsCesium3DTiles() && !(smFilename.ContainsI(L"realitydataservices") && smFilename.ContainsI(L"S3MXECPlugin")))
        {
        // The mesh likely comes from ProjectWiseContextShare, if it does then save that instead
        auto pwcsLink = BeFileName(m_smPtr->GetProjectWiseContextShareLink().c_str());
        if (!pwcsLink.empty()) m_path = pwcsLink;
        }
        
    BeFileName dbFileName(dgnProject.GetDbFileName());
    BeFileName basePath = dbFileName.GetDirectoryName();
    
    if (Utf8String::IsNullOrEmpty(m_properties.m_fileId.c_str()))
        m_properties.m_fileId = Utf8String(m_path);
  
    StatusInt result = m_smPtr->GetTextureInfo(m_textureInfo);

    assert(result == SUCCESS);

    if (!m_textureInfo->IsTextureAvailable())
        {
        SetDisplayTexture(false);
        }
    
    if (m_textureInfo->IsTextureAvailable() && m_textureInfo->IsUsingBingMap())
        {         
        if (&DgnViewLib::GetHost() != nullptr)
            ViewManager::GetManager().AddViewDecoration(&s_viewDecoration);
        }
    else
        { 
        if (&DgnViewLib::GetHost() != nullptr)
            ViewManager::GetManager().DropViewDecoration(&s_viewDecoration);
        }
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                 Richard.Bois     08/2018
//----------------------------------------------------------------------------------------
void ScalableMeshModel::WriteCesiumTileset(BeFileName outFileName, BeFileNameCR outputDir, const Transform& tileToECEF, const Transform& dbToTile) const
    {
    if (_AllowPublishing())
        {
#ifdef _WIN32
        if (SUCCESS == IScalableMeshSaveAs::Generate3DTiles(m_smPtr, outputDir, tileToECEF, dbToTile))
            {
            BeFileName oldRootFile = outputDir;
            oldRootFile.AppendToPath(L"n_0.json");
            BeFileName::BeMoveFile(oldRootFile, outFileName);
            }
#endif
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Mathieu.St-Pierre     3/2018
//----------------------------------------------------------------------------------------
bool ScalableMeshModel::_AllowPublishing() const 
    { 
    return m_smPtr.IsValid() && !m_smPtr->IsCesium3DTiles() && !m_textureInfo->IsUsingBingMap();
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
    
    m_smPtr = nullptr;    
    m_tryOpen = false;

    //Ensure the file has really been closed.
    assert(ScalableMeshLib::GetHost().GetRegisteredScalableMesh(m_path) == nullptr);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ScalableMeshModel::UpdateFilename (BeFileNameCR newFilename)
    {    
    if (!BeFileName::DoesPathExist(newFilename) && !IsSMNameUrl(newFilename.c_str()))
        return ERROR;
            
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
    return m_modelUorToSmTransform;
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

    tmFileName.AppendString(L"//terrain.3sm");
    return tmFileName;
    }

void ScalableMeshModel::ClearOverviews(IScalableMeshPtr& targetSM)
    {
    assert(!"Not done yet");
    }

void ScalableMeshModel::LoadOverviews(IScalableMeshPtr& targetSM)
    {
    assert(!"Not done yet");    
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
        else 
        {
            if (it.second.first == ClipMode::Mask)
                notActivated.insert(it.first);
        }

        if (it.second.first == ClipMode::Clip && it.second.second && m_smPtr->ShouldInvertClips())
        {
            toActivate.insert(it.first);
        }
        else
        {
            if (it.second.first == ClipMode::Clip) notActivated.insert(it.first);
        }

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
void ScalableMeshModel::InitializeTerrainRegions(/*ViewContextR context*/)
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
    else
        RefreshClips();


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
/*NEEDS_WORK_SM : Still needed on BIM02??
        if (nullptr != context.GetViewport())
            {
            bool isDisplayed = context.GetViewport()->GetViewController().IsModelViewed(pScalableMesh->GetModelId());
            SetRegionVisibility(pScalableMesh->GetAssociatedRegionId(), isDisplayed);
            }
*/
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
    assert(!"Not implemented yet in the tiletree display mechanism.");
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

    // Leave it to iModelBridge to do SaveChanges; otherwise it breaks bridge's bulk operation.
    // db.SaveChanges();

    return model.get();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre 01/2018
//----------------------------------------------------------------------------------------
void ScalableMeshModelHandler::_GetClassParams(ECSqlClassParamsR params)
    {
    T_Super::_GetClassParams(params);
    params.Add(SCALABLEMESH_MODEL_PROP_Clips, ECSqlClassParams::StatementType::All);
    params.Add(SCALABLEMESH_MODEL_PROP_GroundCoverages, ECSqlClassParams::StatementType::All);
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
    m_fileId = v.isMember("tilesetUrl") && !v["tilesetUrl"].asString().empty() ? v["tilesetUrl"].asString() : v["FileId"].asString();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  03/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::_OnSaveJsonProperties()
    {
    T_Super::_OnSaveJsonProperties();

    Json::Value val;

    m_properties.ToJson(val);

    if (m_smPtr != 0 && m_smPtr->IsCesium3DTiles())
        {
        Json::Value tilesetVal = m_properties.m_fileId.c_str();
        SetJsonProperties(json_tilesetUrl(), tilesetVal);
        }

#if !defined(ANDROID)
    if (m_clip.IsValid())
        val[json_clip()] = m_clip->ToJson();

    if (!m_classifiers.empty())     // Note - This originally was stored on the "scalableMesh" member...
        SetJsonProperties(json_classifiers(), m_classifiers.ToJson());
#endif

    SetJsonProperties(json_scalablemesh(), val);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  01/2018
//----------------------------------------------------------------------------------------
void SMModelClipInfo::FromBlob(size_t& currentBlobInd, const uint8_t* pClipData)
    {    
    int64_t nbPts;
    memcpy(&nbPts, &pClipData[currentBlobInd], sizeof(int64_t));
    currentBlobInd += sizeof(int64_t);
         
    m_shape.resize(nbPts);
    memcpy(&m_shape[0], &pClipData[currentBlobInd], m_shape.size() * sizeof(DPoint3d));
    currentBlobInd += (m_shape.size() * sizeof(DPoint3d));
 
    memcpy(&m_type, &pClipData[currentBlobInd], sizeof(m_type));
    currentBlobInd += sizeof(m_type);

    memcpy(&m_isActive, &pClipData[currentBlobInd], sizeof(m_isActive));
    currentBlobInd += sizeof(m_isActive);

    memcpy(&m_geomType, &pClipData[currentBlobInd], sizeof(m_geomType));
    currentBlobInd += sizeof(m_geomType);    
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  01/2018
//----------------------------------------------------------------------------------------
void SMModelClipInfo::ToBlob(bvector<uint8_t>& clipData)
    {        
    size_t currentBlobInd = clipData.size();    
    clipData.resize(clipData.size() + sizeof(int64_t));
    int64_t nbPts = (int64_t)m_shape.size();
    memcpy(&clipData[currentBlobInd], &nbPts, sizeof(int64_t));
    
    currentBlobInd = clipData.size();
    clipData.resize(clipData.size() + m_shape.size() * sizeof(DPoint3d));
    memcpy(&clipData[currentBlobInd], &m_shape[0], m_shape.size() * sizeof(DPoint3d));

    currentBlobInd = clipData.size();
    clipData.resize(clipData.size() + sizeof(m_type));
    memcpy(&clipData[currentBlobInd], &m_type, sizeof(m_type));

    currentBlobInd = clipData.size();
    clipData.resize(clipData.size() + sizeof(m_isActive));
    memcpy(&clipData[currentBlobInd], &m_isActive, sizeof(m_isActive));

    currentBlobInd = clipData.size();
    clipData.resize(clipData.size() + sizeof(m_geomType));
    memcpy(&clipData[currentBlobInd], &m_geomType, sizeof(m_geomType));
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  01/2018
//----------------------------------------------------------------------------------------
void SMModelClipVectorInfo::FromBlob(size_t& currentBlobInd, const uint8_t* pClipData)
    {    
	m_bounds = ClipVector::Create();

	memcpy(&m_type, &pClipData[currentBlobInd], sizeof(m_type));
	currentBlobInd += sizeof(m_type);

	memcpy(&m_isActive, &pClipData[currentBlobInd], sizeof(m_isActive));
	currentBlobInd += sizeof(m_isActive);

	memcpy(&m_geomType, &pClipData[currentBlobInd], sizeof(m_geomType));
	currentBlobInd += sizeof(m_geomType);

	uint32_t nOfClipPrims;
	memcpy(&nOfClipPrims, &pClipData[currentBlobInd], sizeof(uint32_t));
	currentBlobInd += sizeof(uint32_t);

	ClipVectorPtr clipVec = ClipVector::Create();
	for (uint32_t j = 0; j < nOfClipPrims; ++j)
	    {
		uint32_t nOfPlanes;
		memcpy(&nOfPlanes, &pClipData[currentBlobInd], sizeof(uint32_t));
		currentBlobInd += sizeof(uint32_t);
		bvector<ClipPlane> planesVec;
		for (uint32_t plane_i = 0; plane_i < nOfPlanes; ++plane_i)
		    {
			DPoint3d origin;
			DVec3d normal;
			memcpy(&origin.x, &pClipData[currentBlobInd], sizeof(double));
			currentBlobInd += sizeof(double);
			memcpy(&origin.y, &pClipData[currentBlobInd], sizeof(double));
			currentBlobInd += sizeof(double);
			memcpy(&origin.z, &pClipData[currentBlobInd], sizeof(double));
			currentBlobInd += sizeof(double);
			memcpy(&normal.x, &pClipData[currentBlobInd], sizeof(double));
			currentBlobInd += sizeof(double);
			memcpy(&normal.y, &pClipData[currentBlobInd], sizeof(double));
			currentBlobInd += sizeof(double);
			memcpy(&normal.z, &pClipData[currentBlobInd], sizeof(double));
			currentBlobInd += sizeof(double);
			planesVec.push_back(ClipPlane(normal, origin));
		    }
		ClipPlaneSet planeSet(planesVec.data(), planesVec.size());
		auto prim = ClipPrimitive::CreateFromClipPlanes(planeSet);
		bool isMask = false;
		memcpy((uint8_t*)&isMask, &pClipData[currentBlobInd], sizeof(uint8_t));
		currentBlobInd += sizeof(uint8_t);
		prim->SetIsMask(isMask);
		bool hasZClip = false;
		memcpy((uint8_t*)&hasZClip, &pClipData[currentBlobInd], sizeof(uint8_t));
		currentBlobInd += sizeof(uint8_t);
		if (hasZClip)
		    {
			double zClipLow, zClipHigh;
			memcpy(&zClipLow, &pClipData[currentBlobInd], sizeof(double));
			currentBlobInd += sizeof(double);
			memcpy(&zClipHigh, &pClipData[currentBlobInd], sizeof(double));
			currentBlobInd += sizeof(double);
			prim->SetZLow(zClipLow);
			prim->SetZHigh(zClipHigh);
		    }
		m_bounds->push_back(prim);
	    }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  01/2018
//----------------------------------------------------------------------------------------
void SMModelClipVectorInfo::ToBlob(bvector<uint8_t>& clipData)
    {        

	size_t currentBlobInd = clipData.size();
    clipData.resize(clipData.size() + sizeof(m_type));
    memcpy(&clipData[currentBlobInd], &m_type, sizeof(m_type));

    currentBlobInd = clipData.size();
    clipData.resize(clipData.size() + sizeof(m_isActive));
    memcpy(&clipData[currentBlobInd], &m_isActive, sizeof(m_isActive));

    currentBlobInd = clipData.size();
    clipData.resize(clipData.size() + sizeof(m_geomType));
    memcpy(&clipData[currentBlobInd], &m_geomType, sizeof(m_geomType));

	uint32_t nOfClipPrims = m_bounds.IsValid() ? m_bounds->size() : 0;
	currentBlobInd = clipData.size();
	clipData.resize(clipData.size() + sizeof(uint32_t));
	memcpy(&clipData[currentBlobInd], &nOfClipPrims, sizeof(nOfClipPrims));
	for (auto& prim : *m_bounds)
	    {
		prim->ParseClipPlanes();
		auto cPlaneSet = prim->GetMaskOrClipPlanes();
		if (nullptr == cPlaneSet)
			continue;

		uint32_t nOfPlanes = cPlaneSet!= nullptr ? cPlaneSet->size() : 0;
		currentBlobInd = clipData.size();
		clipData.resize(clipData.size() + sizeof(uint32_t));
		memcpy(&clipData[currentBlobInd], &nOfPlanes, sizeof(nOfPlanes));

		for (auto& convexPlane : *cPlaneSet)
			for (auto& plane : convexPlane)
			    {
				auto p = plane.GetDPlane3d();
				currentBlobInd = clipData.size();
				clipData.resize(clipData.size() + sizeof(double));
				memcpy(&clipData[currentBlobInd], &p.origin.x, sizeof(double));
				currentBlobInd = clipData.size();
				clipData.resize(clipData.size() + sizeof(double));
				memcpy(&clipData[currentBlobInd], &p.origin.y, sizeof(double));
				currentBlobInd = clipData.size();
				clipData.resize(clipData.size() + sizeof(double));
				memcpy(&clipData[currentBlobInd], &p.origin.z, sizeof(double));
				currentBlobInd = clipData.size();
				clipData.resize(clipData.size() + sizeof(double));
				memcpy(&clipData[currentBlobInd], &p.normal.x, sizeof(double));
				currentBlobInd = clipData.size();
				clipData.resize(clipData.size() + sizeof(double));
				memcpy(&clipData[currentBlobInd], &p.normal.y, sizeof(double));
				currentBlobInd = clipData.size();
				clipData.resize(clipData.size() + sizeof(double));
				memcpy(&clipData[currentBlobInd], &p.normal.z, sizeof(double));
			}

		uint8_t isMask = (uint8_t)prim->IsMask();
		currentBlobInd = clipData.size();
		clipData.resize(clipData.size() + sizeof(uint8_t));
		memcpy(&clipData[currentBlobInd], &isMask, sizeof(uint8_t));
	
		uint8_t hasZClip = (uint8_t)prim->ClipZHigh() || (uint8_t)prim->ClipZLow();
		currentBlobInd = clipData.size();
		clipData.resize(clipData.size() + sizeof(uint8_t));
		memcpy(&clipData[currentBlobInd], &hasZClip, sizeof(uint8_t));
		
		if (hasZClip)
		    {
			double zLow = prim->GetZLow();
			double zHigh = prim->GetZHigh();

			currentBlobInd = clipData.size();
			clipData.resize(clipData.size() + sizeof(double));
			memcpy(&clipData[currentBlobInd], &zLow, sizeof(double));
			currentBlobInd = clipData.size();
			clipData.resize(clipData.size() + sizeof(double));
			memcpy(&clipData[currentBlobInd], &zHigh, sizeof(double));
		    }
	    }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  01/2018
//----------------------------------------------------------------------------------------
void ScalableMeshModel::_BindWriteParams(BeSQLite::EC::ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    // Shoud have been added by RasterModelHandler::_GetClassParams() if not make sure the handler is registered.
    BeAssert(stmt.GetParameterIndex(SCALABLEMESH_MODEL_PROP_Clips) != -1);
    BeAssert(stmt.GetParameterIndex(SCALABLEMESH_MODEL_PROP_GroundCoverages) != -1);

    bvector<uint8_t> clipData;

    for (auto& clipDef : m_scalableClipDefs)
        {
        size_t currentInd = clipData.size();
        clipData.resize(clipData.size() + sizeof(clipDef.first));
        memcpy(&clipData[currentInd], &clipDef.first, sizeof(clipDef.first));
            
        clipDef.second.ToBlob(clipData);
        }        

    if (clipData.empty())
        stmt.BindNull(stmt.GetParameterIndex(SCALABLEMESH_MODEL_PROP_Clips));
    else
        stmt.BindBlob(stmt.GetParameterIndex(SCALABLEMESH_MODEL_PROP_Clips), clipData.data(), (int)clipData.size(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);


    //SCALABLEMESH_MODEL_PROP_GroundCoverages
    bvector<uint8_t> groundCoverageData;

    if (m_linksToGroundModels.size() > 0)
        {         
        uint64_t nbModels = m_linksToGroundModels.size();
        groundCoverageData.resize(groundCoverageData.size() + sizeof(nbModels));
        memcpy(&groundCoverageData[0], &nbModels, sizeof(nbModels));        
        
        size_t size = sizeof(m_linksToGroundModels[0]);
        groundCoverageData.resize(sizeof(nbModels) + sizeof(uint64_t) * 2 * nbModels);
        uint64_t* linkDataPtr = (uint64_t*)&groundCoverageData[sizeof(nbModels)];

        for (size_t modelId = 0; modelId < m_linksToGroundModels.size(); modelId++)
            {            
            linkDataPtr[modelId * 2] = m_linksToGroundModels[modelId].first;
            linkDataPtr[modelId * 2 + 1] = m_linksToGroundModels[modelId].second;
            }
        }

    if (groundCoverageData.empty())
        stmt.BindNull(stmt.GetParameterIndex(SCALABLEMESH_MODEL_PROP_GroundCoverages));
    else
        stmt.BindBlob(stmt.GetParameterIndex(SCALABLEMESH_MODEL_PROP_GroundCoverages), groundCoverageData.data(), (int)groundCoverageData.size(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  01/2018
//----------------------------------------------------------------------------------------
DgnDbStatus ScalableMeshModel::_ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParamsCR params)
    {
    DgnDbStatus status = T_Super::_ReadSelectParams(statement, params);
    if (DgnDbStatus::Success != status)
        return status;

    // Shoud have been added by ScalableMeshModel::_GetClassParams() if not make sure the handler is registred.
    BeAssert(params.GetSelectIndex(SCALABLEMESH_MODEL_PROP_Clips) != -1);
    BeAssert(params.GetSelectIndex(SCALABLEMESH_MODEL_PROP_GroundCoverages) != -1);
  
    int blobSize = 0;
    void const* pBlob = statement.GetValueBlob(params.GetSelectIndex(SCALABLEMESH_MODEL_PROP_Clips), &blobSize);
    uint8_t* pBlobData = (uint8_t*)pBlob;

    assert(m_scalableClipDefs.size() == 0);

    if (blobSize > 0)
        { 
        size_t currentBlobInd = 0;
        
        while (currentBlobInd < blobSize)
            {
            uint64_t clipId; ;
            memcpy(&clipId, &pBlobData[currentBlobInd], sizeof(clipId));
            currentBlobInd += sizeof(clipId);

            SMModelClipInfo smClipInfo;
            smClipInfo.FromBlob(currentBlobInd, pBlobData);                        
            
            m_scalableClipDefs.insert(bpair<uint64_t, SMModelClipInfo>(clipId, smClipInfo));
            }        
        }

    //---------------SCALABLEMESH_MODEL_PROP_GroundCoverages-----------------------
    blobSize = 0;
    pBlob = statement.GetValueBlob(params.GetSelectIndex(SCALABLEMESH_MODEL_PROP_GroundCoverages), &blobSize);
    pBlobData = (uint8_t*)pBlob;

    if (blobSize > 0)
        {
        uint64_t nbModels;
        memcpy(&nbModels, pBlobData, sizeof(uint64_t));

        uint64_t* linkDataPtr = (uint64_t*)&pBlobData[sizeof(uint64_t)];

        m_linksToGroundModels.resize(nbModels);

        for (size_t modelId = 0; modelId < nbModels; modelId++)
            {
            m_linksToGroundModels[modelId].first = linkDataPtr[modelId * 2];
            m_linksToGroundModels[modelId].second = linkDataPtr[modelId * 2 + 1];            
            }            
        }
    
    if (m_smPtr.IsValid())
        {     
        m_clipProvider = new SMClipProvider(this);

        ScalableMesh::IScalableMeshClippingOptions& options = m_smPtr->EditClippingOptions();
        options.SetClipDefinitionsProvider(m_clipProvider);
        options.SetShouldRegenerateStaleClipFiles(true);
        }

    return DgnDbStatus::Success;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  03/2016
//----------------------------------------------------------------------------------------
void ScalableMeshModel::_OnLoadedJsonProperties()
    {
    T_Super::_OnLoadedJsonProperties();

    Json::Value val(GetJsonProperties(json_scalablemesh()));
    val[json_tilesetUrl()] = GetJsonProperties(json_tilesetUrl()).asString();

    m_properties.FromJson(val);

    if (val.isMember(json_clip()))
        m_clip = ClipVector::FromJson(val[json_clip()]);

            #if !defined(ANDROID)
    Json::Value     classifiers = GetJsonProperties(json_classifiers());
    if (classifiers.isNull())
        classifiers = m_classifiers.FromJson(val[json_classifiers()]);       // Old location.                                                                                                                                                             

    if (!classifiers.isNull())
        m_classifiers.FromJson(classifiers);
        #endif

    if (m_smPtr == 0 && !m_tryOpen)
        {                
        if (SUCCESS == ResolveFileName(m_path, ((this)->m_properties).m_fileId, GetDgnDb()))
            {
            OpenFile(m_path, GetDgnDb());
            }

        m_tryOpen = true;
        }
    
    if (m_smPtr.IsValid() && !m_smPtr->IsCesium3DTiles())
        {
        Json::Value publishingMetadata;
        publishingMetadata["name"] = "SMMasterHeader";

#ifdef _WIN32
        IScalableMeshPublisher::Create(SMPublishType::CESIUM)->ExtractPublishMasterHeader(m_smPtr, publishingMetadata["properties"]);
#endif
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



//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  01/2018
//----------------------------------------------------------------------------------------
SMClipProvider::SMClipProvider(ScalableMeshModel* smModel)
    {
    assert(smModel != nullptr);
    m_smModel = smModel;    
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  01/2018
//----------------------------------------------------------------------------------------
void SMClipProvider::GetClipPolygon(bvector<DPoint3d>& poly, uint64_t id)
    {   
    ScalableMesh::SMNonDestructiveClipType type;
    return GetClipPolygon(poly, id, type);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  01/2018
//----------------------------------------------------------------------------------------
void SMClipProvider::GetClipPolygon(bvector<DPoint3d>& poly, uint64_t id, ScalableMesh::SMNonDestructiveClipType& type)
    {
    auto iterClip = m_smModel->m_scalableClipDefs.find(id);

    if (iterClip != m_smModel->m_scalableClipDefs.end())
        {
        type = iterClip->second.m_type;
        poly.insert(poly.begin(), iterClip->second.m_shape.begin(), iterClip->second.m_shape.end());
        }
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  01/2018
//----------------------------------------------------------------------------------------
void SMClipProvider::SetClipPolygon(const bvector<DPoint3d>& poly, uint64_t id, ScalableMesh::SMNonDestructiveClipType type)
    {
    if (m_smModel->m_scalableClipDefs.count(id) == 0)
        {        
        m_smModel->m_scalableClipDefs.insert(make_bpair(id, SMModelClipInfo(poly, type)));
        }
    else
        { 
        m_smModel->m_scalableClipDefs[id].m_type = type;
        m_smModel->m_scalableClipDefs[id].m_shape = poly;
        }

    m_smModel->Update();
/*
    SetClipPolygon(poly, id);
    m_appData->SetClipType(id, type);
*/
    }

void SMClipProvider::GetClipVector(ClipVectorPtr& clip, uint64_t id, SMNonDestructiveClipType& type)
{
	auto iterClip = m_smModel->m_scalableClipVectorDefs.find(id);

	if (iterClip != m_smModel->m_scalableClipVectorDefs.end())
	{
		type = iterClip->second.m_type;
		clip = iterClip->second.m_bounds;
	}
}

void SMClipProvider::SetClipVector(const ClipVectorPtr& clip, uint64_t id, SMNonDestructiveClipType type)
{
	if (m_smModel->m_scalableClipVectorDefs.count(id) == 0)
	{
		m_smModel->m_scalableClipVectorDefs.insert(make_bpair(id, SMModelClipVectorInfo(clip, type)));
	}
	else
	{
		m_smModel->m_scalableClipVectorDefs[id].m_type = type;
		m_smModel->m_scalableClipVectorDefs[id].m_bounds = clip;
	}

	m_smModel->Update();
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  01/2018
//----------------------------------------------------------------------------------------
void SMClipProvider::SetClipPolygon(const bvector<DPoint3d>& poly, uint64_t id)
    {
    ScalableMesh::SMNonDestructiveClipType type = ScalableMesh::SMNonDestructiveClipType::Mask;

    SetClipPolygon(poly, id, type);   
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  01/2018
//----------------------------------------------------------------------------------------
void SMClipProvider::RemoveClipPolygon(uint64_t id)
    {
    m_smModel->m_scalableClipDefs.erase(id);        
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  01/2018
//----------------------------------------------------------------------------------------
void SMClipProvider::RemoveTerrainRegion(uint64_t id)
    {
    assert(!"RemoveTerrainRegion not implemented yet.");
    //m_appData->RemoveTerrainLinkInfo(id);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  01/2018
//----------------------------------------------------------------------------------------
void SMClipProvider::GetTerrainRegion(bvector<DPoint3d>& poly, uint64_t id)
    {
    assert(!"GetTerrainRegion not implemented yet.");
   // m_appData->GetClipInfo(id, poly);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.St-Pierre  01/2018
//----------------------------------------------------------------------------------------
void SMClipProvider::SetTerrainRegion(const bvector<DPoint3d>& poly, uint64_t id)
    {
    assert(!"SetTerrainRegion not implemented yet.");
//    m_appData->SetClipInfo(id, poly);
    }


void SMClipProvider::SetTerrainRegionName(const Utf8String& name, uint64_t id)
    {
    assert(!"SetTerrainRegionName not implemented yet.");
    }

void SMClipProvider::GetTerrainRegionName(Utf8String& name, uint64_t id)
    {
    assert(!"GetTerrainRegionName not implemented yet.");
/*
    bvector<ElementHandle> attachments;
    ScalableMeshAttachment::FindAttachments(attachments, *m_appData->m_hostEh.GetModelRef(), true);
    uint64_t elementId = 0;
    for (auto& part : m_appData->m_linksToModels)
        {
        if (part.second == id)
            {
            elementId = part.first;
            break;
            }
        }
    for (auto& attachment : attachments)
        {
        if (attachment.GetElementId() == elementId)
            {
            ScalableMeshElementAppData* data = ScalableMeshElementAppData::GetOrAddAppData(attachment);
            if (data != nullptr)
                {
                name = data->GetFileName().GetNameUtf8();
                }
            }
        }
*/
    }

void SMClipProvider::ListClipIDs(bvector<uint64_t>& ids)
    {    
    ids.clear();

    for (auto& def : m_smModel->m_scalableClipDefs)
        {        
        ids.push_back(def.first);
        }
    }

void SMClipProvider::ListTerrainRegionIDs(bvector<uint64_t>& ids)
    {    
    ids.clear();
    for (auto& part : m_smModel->m_linksToGroundModels)
        {
        ids.push_back(part.second);
        }
    }

// This moved from ScalableMeshHandler.h because it produces compilation errors if that header is included in a C++/CLI build.
folly::Future<BentleyStatus> SMNode::SMLoader::_GetFromSource()
    {
    //ScalableMesh has his own loader
    return SUCCESS;
    }

