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

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
AxisAlignedBox3d ScalableMeshModel::_GetRange() const
    {
    if (m_smPtr.IsValid())
        m_smPtr->GetRange(const_cast<AxisAlignedBox3d&>(m_range));

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
        const DMatrix4d&                            storageToUors
        /*ScalableMeshDisplayCacheManager*            mgr*/)
    {
//#if 0 //NEEDS_WORK_SM_TEMP_OUT

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

//#endif
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

static bool s_loadTexture = true;
static bool s_waitQueryComplete = false;

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
    static bool s_debugRange = true;
    return s_debugRange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre  08/17
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::ChildTiles const* SMNode::_GetChildren(bool load) const
    { 
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
 * Draw this node.
 * @bsimethod                                                    Mathieu.St-Pierre  08/17
 +---------------+---------------+---------------+---------------+---------------+------*/
void SMNode::_DrawGraphics(Dgn::TileTree::DrawArgsR args) const
    {
    static bool s_debugTexture = false;
    if (!s_debugTexture)
        {
        T_Super::_DrawGraphics(args);
        return;
        }

    if (_WantDebugRangeGraphics())
        AddDebugRangeGraphics(args);

    for (auto& mesh : m_meshes)
        {
        Render::GraphicBuilderPtr graphic = args.m_context.CreateWorldGraphic();

        GraphicBuilder::TileCorners corners;
        DPoint3d rangeCorners[8];
        m_range.Get8Corners(rangeCorners);

        memcpy(&corners.m_pts[0], &rangeCorners[4], sizeof(DPoint3d));
        memcpy(&corners.m_pts[1], &rangeCorners[5], sizeof(DPoint3d));
        memcpy(&corners.m_pts[2], &rangeCorners[6], sizeof(DPoint3d));
        memcpy(&corners.m_pts[3], &rangeCorners[7], sizeof(DPoint3d));

        auto& geom = static_cast<SMGeometry&>(*mesh);

        GraphicParams params;
        params.SetLineColor(ColorDef::Blue());
        graphic->ActivateGraphicParams(params);
        graphic->SetSymbology(ColorDef::White(), ColorDef::White(), 0);
        graphic->AddTile(*geom.m_texture, corners);

        args.m_graphics.Add(*graphic->Finish());
        }
    }

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

            ((Node*)m_children[nodeId->second].get())->m_meshes.push_front(scene._CreateGeometry(trimesh, renderSys));
            }
        }
#endif

    bvector<IScalableMeshNodePtr> childrenNodes(m_scalableMeshNodePtr->GetChildrenNodes());
    
    for (auto& childNode : childrenNodes)
        {
        SMNodePtr nodeptr = new SMNode(GetTriMeshRootR(), this, childNode);

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
            trimesh.m_texture = renderSys->_CreateTexture(binaryImage, params);
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

    m_meshes.push_front(scene._CreateGeometry(trimesh, renderSys));

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
    auto result = _RequestTile(node, nullptr, nullptr);
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

    auto result = _RequestTile(*root, nullptr, GetRenderSystemP());
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
    scene->SetPickable(true);
    if (SUCCESS != scene->LoadScene())
        return nullptr;
static bool s_useProgressiveQuery = true; 

if (!s_useProgressiveQuery)
{ 
}
else
{
    ScalableMeshModel* unconstModel = const_cast<ScalableMeshModel*>(this);
/*
*/

#if 0 
#endif
        while (!unconstModel->GetProgressiveQueryEngine()->IsQueryComplete(terrainQueryId))
#endif

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
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                   Mathieu.St-Pierre  08/17
 +---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshModel::_PickTerrainGraphics(Dgn::PickContextR context) const
    {
    auto scene = Load(nullptr);
    if (nullptr == scene)
        return;

    //MST_TODO
    Dgn::ClipVectorCPtr clip;

    PickContext::ActiveDescription descr(context, GetName());
    scene->Pick(context, scene->GetLocation(), clip.get());
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

    m_displayNodesCache = new ScalableMeshDisplayCacheManager();

    // ScalableMeshTerrainModelAppData* appData = ScalableMeshTerrainModelAppData::Get(params.m_dgndb);
    // appData->m_smTerrainPhysicalModelP = this;
    // appData->m_modelSearched = true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                 Elenie.Godzaridis     2/2016
//----------------------------------------------------------------------------------------
ScalableMeshModel::~ScalableMeshModel()
    {
    if (nullptr != m_progressiveQueryEngine.get() && nullptr != m_currentDrawingInfoPtr.get())
        m_progressiveQueryEngine->StopQuery(m_currentDrawingInfoPtr->m_currentQuery);

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
    
    DgnGCS* projGCS = dgnProject.GeoLocation().GetDgnGCS();

    if (gcs.HasGeoRef())
        {
        DgnGCSPtr dgnGcsPtr(DgnGCS::CreateGCS(gcs.GetGeoRef().GetBasePtr().get(), dgnProject));
        dgnGcsPtr->UorsFromCartesian(scale, scale);

        if (projGCS != nullptr && !projGCS->IsEquivalent(*dgnGcsPtr))
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
        dgnProject.GeoLocation().GetDgnGCS()->UorsFromCartesian(scale, scale);
        assert(scale.x == 1 && scale.y == 1 && scale.z == 1);
        m_smToModelUorTransform = Transform::FromScaleFactors(scale.x, scale.y, scale.z);
        }

    m_smPtr->SetReprojection(*projGCS, m_smToModelUorTransform);

    DPoint3d translation = { 0,0,0 };

    m_storageToUorsTransfo = DMatrix4d::FromScaleAndTranslation(scale, translation);

    bool invertResult = m_modelUorToSmTransform.InverseOf(m_smToModelUorTransform);
    assert(invertResult);


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
    if (m_smPtr.get() != nullptr)
        m_smPtr->GetAllClipIds(allShownIds);
    }

IMeshSpatialModelP ScalableMeshModelHandler::AttachTerrainModel(DgnDb& db, Utf8StringCR modelName, BeFileNameCR smFilename, RepositoryLinkCR modeledElement, bool openFile)
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

    if (openFile)
        {
        model->OpenFile(smFilename, db);
        }
    else
        {
        model->SetFileNameProperty(smFilename);
        }

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
        WString fileNameW(((this)->m_properties).m_fileId.c_str(), true);
        BeFileName smFileName;
        smFileName.AppendString(fileNameW.c_str());
        //BeFileName smFileName;

        //NEEDS_WORK_SM : Doesn't work with URL
        if (true /*BeFileName::DoesPathExist(smFileName.c_str())*/)
            {
            OpenFile(smFileName, GetDgnDb());
            }

        m_tryOpen = true;
        }
    }

HANDLER_DEFINE_MEMBERS(ScalableMeshModelHandler)

