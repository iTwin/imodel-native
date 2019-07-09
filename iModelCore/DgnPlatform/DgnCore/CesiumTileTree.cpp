/*-------------------------------------------------------------------------------------+                                                                                           
|

|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/CesiumTileTree.h>
#include <BeHttp/HttpClient.h>
#include <folly/BeFolly.h>

BEGIN_UNNAMED_NAMESPACE

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void floatToDouble(double* pDouble, float const* pFloat, size_t n)
    {
    for (double* pEnd = pDouble + n; pDouble < pEnd; )
        *pDouble++ = *pFloat++;
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  12/15
+===============+===============+===============+===============+===============+======*/
struct Clipper : PolyfaceQuery::IClipToPlaneSetOutput
{
    bool m_unclipped;
    bvector<PolyfaceHeaderPtr> m_output;

    Clipper() : m_unclipped(false) {}
    StatusInt _ProcessUnclippedPolyface(PolyfaceQueryCR) override {m_unclipped = true; return SUCCESS;}
    StatusInt _ProcessClippedPolyface(PolyfaceHeaderR mesh) override {PolyfaceHeaderPtr meshPtr = &mesh; m_output.push_back(meshPtr); return SUCCESS;}
    bvector<PolyfaceHeaderPtr>& ClipPolyface(PolyfaceQueryCR mesh, ClipVectorCR clip, bool triangulate) {clip.ClipPolyface(mesh, *this, triangulate); return m_output;}
    bool IsUnclipped() {return m_unclipped;}
}; // Clipper

END_UNNAMED_NAMESPACE

BEGIN_DGN_CESIUM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(DgnDbR db, TransformCR location, Utf8CP rootResource) : m_db(db), m_location(location)
    {
    // unless a root directory is specified, we assume it's http.
    m_isHttp = true;

    if (nullptr == rootResource)
        return;

    m_isHttp = (0 == strncmp("http:", rootResource, 5) || 0 == strncmp("https:", rootResource, 6));

    m_rootResource.assign (rootResource);

    if (m_isHttp)
        {
        m_rootResource = m_rootResource.substr(0, m_rootResource.find_last_of("/"));
        }
    else if (!m_rootResource.empty())
        {
        BeFileName rootDirectory(BeFileName::DevAndDir, BeFileName(m_rootResource));
        BeFileName::FixPathName(rootDirectory, rootDirectory, false);
        m_rootResource = rootDirectory.GetNameUtf8();
        m_isHttp = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::StartTileLoad(LoadStateR state) const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    BeAssert(m_activeLoads.end() == m_activeLoads.find(&state));
    m_activeLoads.insert(&state);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::DoneTileLoad(LoadStateR state) const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    BeAssert(m_activeLoads.end() != m_activeLoads.find(&state));
    m_activeLoads.erase(&state);
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* This method gets called on the (valid) children of nodes as they are unloaded. Its purpose is to notify the loading
* threads that these nodes are no longer referenced and we shouldn't waste time loading them.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::SetAbandoned()
    {
    auto children = _GetChildren(false);
    if (nullptr != children)
        for (auto const& child : *children)
            child->SetAbandoned();

    SetLoadStatus(LoadStatus::Abandoned);
    }

/*---------------------------------------------------------------------------------**//**
* ensure that this Tile's range includes its child's range.
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Tile::ExtendRange(DRange3dCR childRange) const
    {
    if (childRange.IsContained(m_range))
        return;

    const_cast<ElementAlignedBox3dR>(m_range).Extend(childRange);

    if (nullptr != m_parent)
        m_parent->ExtendRange(childRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::ClearAllTiles()
    {
    if (!m_rootTile.IsValid())
        return;

    m_rootTile->SetAbandoned();
    WaitForAllLoads();

    m_rootTile = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::CancelTileLoad(TileCR tile)
    {
    // Bentley containers don't support 'transparent' comparators, meaning we can't compare a TileLoadStatePtr to a Tile even
    // though the comparator can. We should fix that - but for now, instead, we're using std::set.
    BeMutexHolder lock(m_cv.GetMutex());
    auto iter = m_activeLoads.find(&tile);
    if (iter != m_activeLoads.end())
        {
        (*iter)->SetCanceled();
        m_activeLoads.erase(iter);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::CancelAllTileLoads()
    {
    BeMutexHolder lock(m_cv.GetMutex());
    for (auto& load : m_activeLoads)
        load->SetCanceled();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> Root::_RequestTile(TileR tile, LoadStateR loads, OutputR output)
    {
    if (!tile.IsNotLoaded()) // this should only be called when the tile is in the "not loaded" state.
        {
        BeAssert(false);
        return ERROR;
        }

    auto loader = tile._CreateLoader(loads, output);
    if (loader.IsNull())
        return ERROR;   
    
    return loader->Perform();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> Loader::_GetFromSource()
    {
    if (m_tile->GetRoot().IsHttp())
        {
        auto query = std::make_shared<HttpDataQuery>(m_resourceName, *m_loads);

        LoaderPtr me(this);
        return query->Perform().then([me, query] (Http::Response const& response)
            {
            if (Http::ConnectionStatus::OK != response.GetConnectionStatus() || Http::HttpStatus::OK != response.GetHttpStatus())
                return ERROR;

            if (!query->m_responseBody->GetByteStream().HasData())
                return ERROR;

            me->m_tileBytes = StreamBuffer(std::move(query->m_responseBody->GetByteStream())); // NEEDSWORK this is a copy not a move...
            me->m_contentType = response.GetHeaders().GetContentType();

            return SUCCESS;
            });
        }                                           
    else
        {
        auto query = std::make_shared<FileDataQuery>(m_resourceName, *m_loads);
     
        LoaderPtr me(this);
        return query->Perform().then([me, query](ByteStream const& data)
            {
            if (!data.HasData())
                return ERROR;

            me->m_tileBytes = StreamBuffer(std::move(data)); // NEEDSWORK this is a copy not a move...
            me->m_contentType = "";     // unknown 

            return SUCCESS;
            });         
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016                                        
//----------------------------------------------------------------------------------------
BentleyStatus Loader::LoadTile()
    {
    // During the read we may have abandoned the tile. Do not waste time loading it.
    if (IsCanceledOrAbandoned())
        return ERROR;

    BeAssert(m_tile->IsQueued());
    return _LoadTile();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> Loader::Perform()
    {
    m_tile->SetIsQueued(); // mark as queued so we don't request it again.

    LoaderPtr me(this);
    auto loadFlag = std::make_shared<LoadFlag>(*m_loads);
    return me->_GetFromSource().then(&BeFolly::ThreadPool::GetCpuPool(), [me, loadFlag](BentleyStatus status)
        {
        DgnDb::SetThreadId(DgnDb::ThreadId::CpuPool);
        auto& tile = *me->m_tile;
        if (SUCCESS != status || SUCCESS != me->LoadTile())
            {
            if (me->m_loads->IsCanceled())
                tile.SetNotLoaded();    // Mark it as not loaded so we can retry again.
            else
                tile.SetNotFound();

            return ERROR;
            }

        tile.SetIsReady();  // OK, we're all done loading and the other thread may now use this data. Set the "ready" flag.
        return SUCCESS;
        });
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
HttpDataQuery::HttpDataQuery(Utf8StringCR url, LoadStateP loads) : m_request(url), m_loads(loads), m_responseBody(Http::HttpByteStreamBody::Create())
    {
    m_request.SetResponseBody(m_responseBody);
    m_request.SetRetryOptions(Http::Request::RetryOption::ResetTransfer, 1);
    if (nullptr != loads)
        m_request.SetCancellationToken(loads->GetCancellationToken());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
folly::Future<Http::Response> HttpDataQuery::Perform()
    {
    LoadStatePtr loads(m_loads);
    return GetRequest().Perform().then([loads] (Http::Response response)
        {
        return response;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ByteStream> FileDataQuery::Perform()
    {
    auto filename = m_fileName;
    LoadStatePtr loads(m_loads);
    return folly::via(&BeFolly::ThreadPool::GetIoPool(), [filename, loads] ()
        {
        ByteStream data;
        BeFile dataFile;
        if (BeFileStatus::Success != dataFile.Open(filename.c_str(), BeFileAccess::Read))
            return data;

        if (BeFileStatus::Success != dataFile.ReadEntireFile(data))
            return data;

        return data;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::QPoint3dList TriMeshTree::TriMesh::CreateParams::QuantizePoints() const
    {
    // ###TODO: Is the tile's range known yet, and do we expect the range of points within it to be significantly smaller?
    DRange3d range = DRange3d::NullRange();
    for (int32_t i = 0; i < m_numPoints; i++)
        range.Extend(DPoint3d::From(m_points[i]));

    Render::QPoint3dList qpts(range);
    qpts.reserve(m_numPoints);
    for (int32_t i = 0; i < m_numPoints; i++)
        qpts.Add(DPoint3d::From(m_points[i]));

    return qpts;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::OctEncodedNormalList TriMeshTree::TriMesh::CreateParams::QuantizeNormals() const
    {
    OctEncodedNormalList oens;
    if (nullptr != m_normals)
        {
        oens.resize(m_numPoints);
        for (size_t i = 0; i < m_numPoints; i++)
            {
            FPoint3d normal = m_normals[i];
            FVec3d vec = FVec3d::From(normal.x, normal.y, normal.z);
            oens[i] = OctEncodedNormal::From(vec);
            }
        }

    return oens;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/17
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr TriMeshTree::TriMesh::CreateParams::ToPolyface() const
    {
    PolyfaceHeaderPtr polyFace = PolyfaceHeader::CreateFixedBlockIndexed(3);

    BlockedVectorIntR pointIndex = polyFace->PointIndex();
    pointIndex.resize(m_numIndices);
    uint32_t const* pIndex = (uint32_t const*)m_vertIndex;
    uint32_t const* pEnd = pIndex + m_numIndices;
    int* pOut = &pointIndex.front();

    for (; pIndex < pEnd; )
        *pOut++ = 1 + *pIndex++;

    if (nullptr != m_points)
        {
        polyFace->Point().resize(m_numPoints);
        for (int i = 0; i < m_numPoints; i++)
            polyFace->Point()[i] = DVec3d::From(m_points[i].x, m_points[i].y, m_points[i].z);
        }

    if (nullptr != m_normals)
        {
        polyFace->Normal().resize(m_numPoints);
        for (int i = 0; i < m_numPoints; i++)
            polyFace->Normal()[i] = DVec3d::From(m_normals[i].x, m_normals[i].y, m_normals[i].z);
        }

    if (nullptr != m_textureUV)
        {
        polyFace->Param().resize(m_numPoints);
        floatToDouble(&polyFace->Param().front().x, &m_textureUV->x, 2 * m_numPoints);
        polyFace->ParamIndex() = pointIndex;
        }

    return polyFace;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TriMeshTree::TriMesh::CreateParams::FromTile(Render::TextureCR tile, GraphicBuilder::TileCorners const& corners, FPoint3d* fpts)
    {
    for (int i = 0; i < 4; i++)
        {
        fpts[i] = FPoint3d::From(corners.m_pts[i].x, corners.m_pts[i].y, corners.m_pts[i].z);
        }
    m_numPoints  = 4;
    m_points     = fpts;
    m_normals    = nullptr;

    static int32_t indices[] = {0,1,2,2,1,3};
    m_numIndices = 6;
    m_vertIndex  = indices;

    static FPoint2d textUV[] = 
        {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {0.0f, 1.0f},
            {1.0f, 1.0f},
        };
    m_textureUV = textUV;
    m_texture = const_cast<Render::Texture*>(&tile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TriMeshArgs TriMeshTree::TriMesh::CreateTriMeshArgs(Render::TextureP texture, FPoint2d const* textureUV) const
    {
    TriMeshArgs trimesh;
    trimesh.m_numIndices = m_indices.size();
    trimesh.m_vertIndex = (uint32_t const*) (m_indices.empty() ? nullptr : &m_indices.front());
    trimesh.m_numPoints = (uint32_t) m_points.size();
    trimesh.m_points  = m_points.empty() ? nullptr : &m_points.front();
    trimesh.m_normals = m_normals.empty() ? nullptr : &m_normals.front();
    trimesh.m_textureUV = textureUV;
    trimesh.m_pointParams = m_points.GetParams();
    trimesh.m_texture = texture;

    return trimesh;
    }

/*-----------------------------------------------------------------------------------**//**
* Construct a Geometry from a CreateParams and a Scene. The scene is necessary to get the Render::System, and this
* Geometry is only valid for that Render::System
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TriMeshTree::TriMesh::TriMesh(CreateParams const& args, OutputR output)
    {
    m_indices.resize(args.m_numIndices);
    memcpy(&m_indices.front(), args.m_vertIndex, args.m_numIndices * sizeof(int32_t));
    m_points = args.QuantizePoints();
    if (nullptr != args.m_normals)
        m_normals = args.QuantizeNormals();

    auto trimesh = CreateTriMeshArgs(args.m_texture.get(), args.m_textureUV);
    output._AddTriMesh(trimesh);

    // ###TODO: Don't make these members - just use local variables...
    m_indices.clear();
    m_points.clear();
    m_normals.clear();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Mark.Schlosser  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void TriMeshTree::Root::ClipTriMesh(TriMeshList& triMeshList, TriMesh::CreateParams const& geomParams, OutputR output)
    {
    Clipper clipper;
    PolyfaceHeaderPtr polyface = geomParams.ToPolyface();
    bvector<PolyfaceHeaderPtr>& clippedPolyfaces = clipper.ClipPolyface(*polyface, *_GetClipVector(), true);
    if (clipper.IsUnclipped())
        {
        triMeshList.push_front(new TriMesh(geomParams, output));
        }
    else
        {
        for (PolyfaceHeaderPtr clippedPolyface : clippedPolyfaces)
            {
            if (!clippedPolyface->IsTriangulated())
                clippedPolyface->Triangulate();

            if ((0 != clippedPolyface->GetParamCount() && clippedPolyface->GetParamCount() != clippedPolyface->GetPointCount()) || 
                (0 != clippedPolyface->GetNormalCount() && clippedPolyface->GetNormalCount() != clippedPolyface->GetPointCount()))
                clippedPolyface = PolyfaceHeader::CreateUnifiedIndexMesh(*clippedPolyface);

            size_t              numPoints = clippedPolyface->GetPointCount();
            bvector<int32_t>    indices;
            bvector<FPoint3d>   points(numPoints), normals(nullptr == clippedPolyface->GetNormalCP() ? 0 : numPoints);
            bvector<FPoint2d>   params(nullptr == clippedPolyface->GetParamCP() ? 0 : numPoints);

            for (size_t i=0; i<numPoints; i++)
                {
                points[i] = FPoint3d::From (clippedPolyface->GetPointCP()[i]);
                //bsiFPoint3d_initFromDPoint3d(&points[i], &clippedPolyface->GetPointCP()[i]);
                if (nullptr != clippedPolyface->GetNormalCP())
                    normals[i] = FPoint3d::From (clippedPolyface->GetNormalCP()[i]);
                    //bsiFPoint3d_initFromDPoint3d(&normals[i], &clippedPolyface->GetNormalCP()[i]);

                if (nullptr != clippedPolyface->GetParamCP())
                    {
                    params[i].x = clippedPolyface->GetParamCP()[i].x;
                    params[i].y = clippedPolyface->GetParamCP()[i].y;
                    }
                    //bsiFPoint2d_initFromDPoint2d(&params[i], &clippedPolyface->GetParamCP()[i]);
                }
            PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*clippedPolyface, true);
            for (visitor->Reset(); visitor->AdvanceToNextFace();)
                {   
                indices.push_back(visitor->GetClientPointIndexCP()[0]);
                indices.push_back(visitor->GetClientPointIndexCP()[1]);
                indices.push_back(visitor->GetClientPointIndexCP()[2]);
                }

            TriMesh::CreateParams clippedGeomParams;
            clippedGeomParams.m_numIndices = indices.size();
            clippedGeomParams.m_vertIndex  = &indices.front();
            clippedGeomParams.m_numPoints  = numPoints;
            clippedGeomParams.m_points     = &points.front();
            clippedGeomParams.m_normals    = normals.empty() ? nullptr : &normals.front();
            clippedGeomParams.m_textureUV  = params.empty() ? nullptr : &params.front();
            clippedGeomParams.m_texture    = geomParams.m_texture;

            triMeshList.push_front(new TriMesh(clippedGeomParams, output));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mark.Schlosser   12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void TriMeshTree::Root::CreateGeometry(TriMeshList& triMeshList, TriMesh::CreateParams const& geomParams, OutputR output)
    {
    if (nullptr != _GetClipVector())
        ClipTriMesh(triMeshList, geomParams, output);
    else
        triMeshList.push_front(new TriMesh(geomParams, output));
    }

END_DGN_CESIUM_NAMESPACE
