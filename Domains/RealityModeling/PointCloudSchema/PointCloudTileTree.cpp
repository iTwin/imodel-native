/*-------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudInternal.h>
#include <PointCloud/PointCloudHandler.h>
#include "PointCloudTileTree.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_POINTCLOUD
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD
USING_NAMESPACE_POINTCLOUD_TILETREE


static int  s_nominalTileSize = 512;
static int s_pointCountLimit = 125000;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
static DRange3d bisectRange(DRange3dCR range, bool takeLow)
    {
    DVec3d diag = range.DiagonalVector();
    DRange3d subRange = range;

    double bisect;
    double* replace;
    if (diag.x > diag.y && diag.x > diag.z)
        {
        bisect = (range.low.x + range.high.x) / 2.0;
        replace = takeLow ? &subRange.high.x : &subRange.low.x;
        }
    else if (diag.y > diag.z)
        {
        bisect = (range.low.y + range.high.y) / 2.0;
        replace = takeLow ? &subRange.high.y : &subRange.low.y;
        }
    else
        {
        bisect = (range.low.z + range.high.z) / 2.0;
        replace = takeLow ? &subRange.high.z : &subRange.low.z;
        }

    *replace = bisect;
    return subRange;
    }

DEFINE_POINTER_SUFFIX_TYPEDEFS(Loader)
DEFINE_REF_COUNTED_PTR(Loader)

//=======================================================================================
// Since we can only create one tile at a time, we serialize all requests through a single thread.
// This avoids all of the requests blocking in the IoPool.
// @bsiclass                                                    Mathieu.Marchand 12/16
//=======================================================================================
struct PointCloudFileThread : BeFolly::ThreadPool
    {
    PointCloudFileThread() : ThreadPool(1, "PointCloud") {}
    static PointCloudFileThread& Get() { static folly::Singleton<PointCloudFileThread> s_pool; return *s_pool.try_get_fast(); }
    };

BEGIN_POINTCLOUD_TILETREE_NAMESPACE

//=======================================================================================
// @bsistruct                                                    Ray.Bentley    02/2017
//=======================================================================================
struct Loader : Cesium::Loader
{
    DEFINE_T_SUPER(Cesium::Loader);

private:
    Loader(TileR tile, Cesium::LoadStateR loads, Cesium::OutputR output) : T_Super("", tile, output, loads) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus _LoadTile() override 
    { 
    TileR   tile = static_cast<TileR> (*m_tile);

    tile.Read(m_tileBytes);
    return tile.AddGraphics(GetOutput()); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> _GetFromSource() override
    {
    RefCountedPtr<Loader> me(this);
    return folly::via(&PointCloudFileThread::Get(), [me] ()
        {
        if (me->IsCanceledOrAbandoned())
            return ERROR;

        return me->DoGetFromSource();
        });
    }


struct QPointComparator
    {
    bool operator()(QPoint3d const& lhs, QPoint3d const& rhs) const
        {
        if (lhs.x < rhs.x)
            return true;
        else if (lhs.x > rhs.x)
            return false;

        if (lhs.y < rhs.y)
            return true;
        else if (lhs.y > rhs.y)
            return false;

        return lhs.z < rhs.z;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DoGetFromSource()
    {
    auto&                       tile = static_cast<TileR>(*m_tile);
    auto&                       root = static_cast<RootCR>(m_tile->GetRoot());
    static const size_t         s_maxTileBatchCount = 500000;
#ifdef DO_VIEWPORT_QUERY
    static size_t               s_maxLeafPointCount = 20000;
#else   
    static size_t               s_maxLeafPointCount = s_pointCountLimit * .9;
#endif    
    bool                        colorsPresent;

    static   BeMutex            s_queryMutex;
    BeMutexHolder               lock(s_queryMutex);        // Arrgh.... Queries are not thread safe??

    PointCloudQueryHandlePtr    queryHandle = root.InitQuery(colorsPresent, tile.GetRange(), s_maxTileBatchCount);

    if (queryHandle == nullptr)
        return ERROR;
    size_t                      nBatchPoints = 0;
    bvector<FPoint3d>           batchPoints(s_maxTileBatchCount);
    bvector<PointCloudColorDef> batchColors(s_maxTileBatchCount), colors;
    Transform                   dgnToTile, cloudToTile;
    DRange3d                    tileRange = tile.GetRange();
    QPoint3dList                points(tileRange);
    bset<QPoint3d, QPointComparator> qPointSet;


    dgnToTile.InverseOf(root.GetLocation());
    cloudToTile = Transform::FromProduct(dgnToTile, root.GetPointCloudModel().GetSceneToWorld());

    nBatchPoints = (size_t) ptGetQueryPointsf(queryHandle->GetHandle(), s_maxTileBatchCount, &batchPoints.front().x,
                                              colorsPresent ? (PTubyte*) batchColors.data() : nullptr, nullptr, nullptr, nullptr);
    for (size_t i = 0; i < nBatchPoints; i++)
        {
        DPoint3d        tmpPoint = {batchPoints[i].x, batchPoints[i].y, batchPoints[i].z};

        cloudToTile.Multiply(tmpPoint);

        if (tileRange.IsContained(tmpPoint))
            {
            QPoint3d    qPoint(tmpPoint, points.GetParams());

            auto        inserted = qPointSet.insert(qPoint);
            if (inserted.second)
                {
                points.push_back(qPoint);
                if (colorsPresent)
                    colors.push_back(batchColors[i]);
                }
            }
        }

    if (points.size() < s_maxLeafPointCount && tile.GetParent() != nullptr)         // TFS 173793 - Children are required at root.
        tile.SetIsLeaf();

    // TODO - Add technique for monochrome point clouds...
    if (!colorsPresent)
        colors = bvector<PointCloudColorDef>(points.size(), PointCloudColorDef(0, 0xff, 0));

    Json::Value     featureTable;
    bool            rgbPresent = colors.size() == points.size();
    DVec3d          tileRangeDiagonal = tileRange.DiagonalVector();

    featureTable["POINTS_LENGTH"] = static_cast<uint32_t>(points.size());
    featureTable["POSITION"]["byteOffset"] = 0;

    featureTable["QUANTIZED_VOLUME_OFFSET"].append(tileRange.low.x);
    featureTable["QUANTIZED_VOLUME_OFFSET"].append(tileRange.low.y);
    featureTable["QUANTIZED_VOLUME_OFFSET"].append(tileRange.low.z);
    featureTable["QUANTIZED_VOLUME_SCALE"].append(tileRangeDiagonal.x);
    featureTable["QUANTIZED_VOLUME_SCALE"].append(tileRangeDiagonal.y);
    featureTable["QUANTIZED_VOLUME_SCALE"].append(tileRangeDiagonal.z);

    if (rgbPresent)
        featureTable["RGB"]["byteOffset"] = static_cast<uint32_t>(points.size() * sizeof(QPoint3d));

    Utf8String      featureTableStr = Json::FastWriter().write(featureTable);
    uint32_t        featureTableStrLen = featureTableStr.size();

    m_tileBytes.Append((uint8_t const*) &featureTableStrLen, sizeof(featureTableStrLen));
    m_tileBytes.Append((uint8_t const*) featureTableStr.c_str(), featureTableStrLen);
    if (!points.empty())
        {
        m_tileBytes.Append((uint8_t const*) points.data(), points.size() * sizeof(QPoint3d));
        if (rgbPresent)
            m_tileBytes.Append((uint8_t const*) colors.data(), colors.size() * sizeof(PointCloudColorDef));
        }

    return SUCCESS;
    }


public:
    static Cesium::LoaderPtr Create(TileR tile, Cesium::LoadStateR loads, Cesium::OutputR output) { return new Loader(tile, loads, output); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::Tile(Root& octRoot, TileId id, Tile const* parent, DRange3dCP range)
    : T_Super(octRoot, parent), m_id(id), m_isLeaf(false), m_points(QPoint3d::Params())
    {
    static const double s_minToleranceRatio = 1000.0;

    if (nullptr != parent)
        m_range = ElementAlignedBox3d(parent->ComputeChildRange(*this));
    else
        m_range.Extend (*range);

    m_points.Reset(QPoint3d::Params(m_range));
    m_tolerance = m_range.DiagonalDistance() / s_minToleranceRatio;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Cesium::LoaderPtr Tile::_CreateLoader(Cesium::LoadStateR loadState, Cesium::OutputR output)
    {                                                                        
    return Loader::Create(*this, loadState, output);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Cesium::TilePtr Tile::CreateChild(TileId childId) const
    {
    return Tile::Create(const_cast<RootR>(GetPointCloudRoot()), childId, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
double Tile::_GetMaximumSize() const
    {
    return s_nominalTileSize; // ###TODO: come up with a decent value, and account for device ppi
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Tile::Read (StreamBuffer& streamBuffer)
    {
    if (streamBuffer.empty())
        return ERROR;

    uint32_t        featureTableStrLen;

    streamBuffer.ReadBytes(&featureTableStrLen, sizeof(featureTableStrLen));
    bvector<char>       featureTableData(featureTableStrLen);
    Json::Value         featureTable;
    Json::Reader        reader;                                                                                                                        
    
    if(!streamBuffer.ReadBytes(featureTableData.data(), featureTableStrLen) ||
       !reader.parse(featureTableData.data(), featureTableData.data() + featureTableStrLen, featureTable))
        {
        BeAssert(false);
        return ERROR;
        }
    uint32_t      nPoints = featureTable["POINTS_LENGTH"].asUInt();
    uint32_t      binaryPos = streamBuffer.GetPos(), positionOffset, rgbOffset;
    
    if (featureTable.isMember("POSITION"))
        {
        m_points.resize(nPoints);                                      
        
        streamBuffer.SetPos(binaryPos + (positionOffset = featureTable["POSITION"]["byteOffset"].asUInt()));;
        if (0 != nPoints && !streamBuffer.ReadBytes(m_points.data(), nPoints * sizeof(QPoint3d)))
            {
            BeAssert(false);
            return ERROR;
            }
        }

    if (featureTable.isMember("RGB"))
        {
        m_colors.resize(nPoints);

        streamBuffer.SetPos(binaryPos + (rgbOffset = featureTable["RGB"]["byteOffset"].asUInt()));
        if (0 != nPoints && !streamBuffer.ReadBytes(m_colors.data(), nPoints * sizeof(PointCloudColorDef)))
            {
            BeAssert(false);
            return ERROR;
            }

        }

    return SUCCESS; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Tile::AddGraphics(Cesium::OutputR output) 
    {
    if (!m_points.empty())
        {
        auto& root   = static_cast<RootCR>(GetRoot());
        Render::PointCloudArgs args(QPoint3d::Params(m_range), static_cast<int32_t>(m_points.size()), &m_points.front(), reinterpret_cast<ByteCP>(m_colors.data()));
        output._AddPointCloud(args);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Tile::ComputeChildRange(Tile& child) const
    {
    // Each dimension of the relative ID is 0 or 1, indicating which bisection of the range to take
    TileId relativeId = child.GetRelativeTileId();
    BeAssert(2 > relativeId.m_i && 2 > relativeId.m_j && 2 > relativeId.m_k);

    DRange3d range = bisectRange(GetRange(), 0 == relativeId.m_i);
    range = bisectRange(range, 0 == relativeId.m_j);
    range = bisectRange(range, 0 == relativeId.m_k);

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Cesium::ChildTiles const* Tile::_GetChildren(bool load) const
    {
    if (m_isLeaf)
        return nullptr;

    if (load && m_children.empty())
        {
        for (int i = 0; i < 2; i++)
            {
            for (int j = 0; j < 2; j++)
                {
                for (int k = 0; k < 2; k++)
                    {
                    auto child = CreateChild(m_id.CreateChildId(i, j, k));
                    if (child.IsValid())
                        m_children.push_back(child.get());
                    }
                }
            }
        }

    return m_children.empty() ? nullptr : &m_children;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileId Tile::GetRelativeTileId() const
    {
    auto tileId = GetTileId();
    auto parent = static_cast<TileCP>(GetParent());
    if (nullptr != parent)
        tileId = tileId.GetRelativeId(parent->GetTileId());

    return tileId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::LoadRootTile(DRange3dCR tileRange, Cesium::OutputR output)
    {
    m_rootTile = Tile::Create(*this, tileRange);

#define LOAD_ASYNCH_ROOT
#ifdef LOAD_ASYNCH_ROOT
    // Push to always request root tile --- this provides a low resolution proxy while the actual nodes load (but delays final display).
    auto loadState = Cesium::LoadState::Create(*m_rootTile);
    auto result = _RequestTile(*m_rootTile, *loadState, output);
    result.wait();
#endif
    }

PointCloudQueryHandlePtr createBoundingBoxQuery (PointCloudSceneP scene, DRange3dCR sceneRange, size_t maxCount)
    {
    return scene->CreateBoundingBoxQuery(sceneRange, QUERY_DENSITY_LIMIT, (float) maxCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudQueryHandlePtr  Root::InitQuery (bool& colorsPresent, DRange3dCR tileRange, size_t maxCount) const
    {
    Transform           worldToScene;
    DRange3d            sceneRange;
    
    auto pointCloudScene = m_model.GetPointCloudSceneP();
    if (nullptr == pointCloudScene)
        {
        BeAssert(!"Point cloud scene is missing");
        return 0;
        }

    colorsPresent = pointCloudScene->_HasRGBChannel();
    worldToScene.InverseOf (m_model.GetSceneToWorld());
    Transform::FromProduct (worldToScene, GetLocation()).Multiply (sceneRange, tileRange);
#ifdef DO_VIEWPORT_QUERY
    DVec3d              diagonalVector = sceneRange.DiagonalVector();
    Transform           eyeTransform = Transform::FromProduct (Transform::From(-1.0, -1.0, -1.0), 
                                                               Transform::FromScaleFactors(2.0 / diagonalVector.x, 2.0/diagonalVector.y, 2.0/diagonalVector.z), 
                                                               Transform::From(-sceneRange.low.x, -sceneRange.low.y, -sceneRange.low.z));
    DMatrix4d           identityMatrix, eyeMatrix = DMatrix4d::From(eyeTransform);


    identityMatrix.InitIdentity();

    ptSetViewProjectionMatrix(&identityMatrix.coff[0][0], true);
    ptSetViewEyeMatrix(&eyeMatrix.coff[0][0], true);
    ptSetViewportSize(0, 0, s_nominalTileSize, s_nominalTileSize);

    PointCloudQueryHandlePtr    queryHandle = PointCloudQueryHandle::Create(ptCreateFrustumPointsQuery());

    ptResetQuery(queryHandle->GetHandle());
    ptSetQueryScope(queryHandle->GetHandle(), pointCloudScene->GetSceneHandle());
    ptSetQueryDensity (queryHandle->GetHandle(), QUERY_DENSITY_VIEW_COMPLETE, 1.0);

    // Ick... the points are being loaded from the .pod file in another thread...
    while (0 != ptPtsToLoadInViewport(pointCloudScene->GetSceneHandle(), true))
        BeDuration::FromMilliseconds(10).Sleep();

    return queryHandle;
#else
    return createBoundingBoxQuery (pointCloudScene, sceneRange, s_pointCountLimit);
#endif    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RootPtr Root::Create(PointCloudModelR model, Cesium::OutputR output)
    {
    DgnDb::VerifyClientThread();

    DRange3d                    dgnRange;
    model.GetRange(dgnRange, PointCloudModel::Unit::World);

    // Translate world coordinates to center of range in order to reduce precision errors
    DPoint3d    centroid = DPoint3d::FromInterpolate(dgnRange.low, 0.5, dgnRange.high);
    Transform   transformFromTile = Transform::From(centroid), transformToTile;

    transformToTile.InverseOf(transformFromTile);

    RootPtr     root = new Root(model, transformFromTile);
    DRange3d    tileRange;

    transformToTile.Multiply(tileRange, dgnRange);
    root->LoadRootTile(tileRange, output);

    return root;
    }

END_POINTCLOUD_TILETREE_NAMESPACE
