/*-------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudTilePublish.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudInternal.h>
#include <PointCloud/PointCloudHandler.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_POINTCLOUD
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

BEGIN_UNNAMED_NAMESPACE

static size_t                           s_maxTilePointCount = 2000000;


//=======================================================================================
// @bsiclass                                                    Ray.Bentley     01/2017
//=======================================================================================
struct  PublishTileNode : ModelTileNode
{
    struct PublishPointCloudContext&      m_publishContext;

    PublishTileNode(struct PublishPointCloudContext& publishContext, DgnModelCR model, DRange3dCR range, TransformCR transformDbToTile, size_t depth, size_t siblingIndex, TileNodeP parent)
        : ModelTileNode(model, range, transformDbToTile, depth, siblingIndex, parent, 0.0), m_publishContext(publishContext) { }

    virtual WString _GetFileExtension() const override { return L"pnts"; }
    virtual PublishableTileGeometry _GeneratePublishableGeometry(DgnDbR dgnDb, TileGeometry::NormalMode normalMode, bool doPolylines, bool doInstancing, ITileGenerationFilterCP filter = nullptr) const override;

    size_t  QueryPointCount(size_t maxPoints) const;


};  //  PublishTileNode

typedef RefCountedPtr<PublishTileNode>  T_PublishTilePtr;

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     10/2016
//=======================================================================================
struct PublishPointCloudContext
{
    PointCloudModel const&          m_model;
    TransformCR                     m_transformDbToTile;
    TileGenerator::ITileCollector&  m_collector;
    ITileGenerationProgressMonitorR m_progressMeter;
    uint32_t                        m_totalTiles;
    BeAtomic<uint32_t>              m_completedTiles;
    StopWatch                       m_progressTimer;
    BeMutex                         m_queryMutex;
    
    PublishPointCloudContext (PointCloudModel const& model, TransformCR transformDbToTile, TileGenerator::ITileCollector& collector, ITileGenerationProgressMonitorR progressMeter) :
                         m_model(model), m_transformDbToTile(transformDbToTile), m_collector(collector), m_progressMeter(progressMeter), m_totalTiles(1), m_progressTimer(true) { }

    void    IndicateProgress()      {   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  MakeQuery (PointCloudQueryBuffersPtr& queryBuffers, DRange3dCR dgnRange, int densityType, float densityValue) 
    {
    BeMutexHolder lock(m_queryMutex);

    Transform                   worldToScene;
    DRange3d                    sceneRange;
                                
    worldToScene.InverseOf (m_model.GetSceneToWorld());
    worldToScene.Multiply (sceneRange, dgnRange);
    
    auto pointCloudScene = m_model.GetPointCloudSceneP();
    if (nullptr == pointCloudScene)
        {
        BeAssert(!"Point cloud scene is missing");
        return 0;
        }
    PointCloudQueryHandlePtr    queryHandle = pointCloudScene->CreateBoundingBoxQuery(sceneRange, densityType, densityValue);

    return queryBuffers->GetPoints (queryHandle->GetHandle());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ProcessTile(PublishTileNode& tile, double leafTolerance, size_t depth)
    {
    static size_t       s_depthLimit = 20;
    static size_t       s_minTilePointCount = 100;

    static const double s_minToleranceRatio = 1000.0;       // This is key parameter - increasing it will make fewer, larger tiles.   Too low will produce small tiles and excessive draw calls degrading performance.
                                                            // 1000 will generate around a half million points per tile which seems to work well.
    double              tileTolerance = tile.GetDgnRange().DiagonalDistance() / s_minToleranceRatio;
    bool                isLeaf = tileTolerance < leafTolerance;

    tile.SetTolerance (tileTolerance);
    if (depth < s_depthLimit &&!isLeaf && tile.QueryPointCount(s_maxTilePointCount) >= s_maxTilePointCount)
        {
        bvector<DRange3d>   childRanges;
        size_t      childIndex = 0;

        TileNode::ComputeChildTileRanges (childRanges, tile.GetDgnRange());

        for (auto& childRange : childRanges)    
            {
            T_PublishTilePtr    childTile = new PublishTileNode(*this, m_model, childRange, m_transformDbToTile, depth + 1, childIndex++, &tile);
            size_t              childPointCount = childTile->QueryPointCount(s_maxTilePointCount);

            if (childPointCount > s_minTilePointCount)   
                {
                m_totalTiles++;
                tile.GetChildren().push_back(childTile);
                ProcessTile(*childTile, leafTolerance, depth+1);
                }
            }
        }

    // Process this tile.
    folly::via(&BeFolly::ThreadPool::GetIoPool(), [&]()  
        {  
        m_collector._AcceptTile(tile);  
        m_completedTiles++;
        });

    IndicateProgress();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ProcessingRemains()
    {
    if (m_completedTiles >= m_totalTiles)
        return false;

    IndicateProgress();
    return true;
    }


};  // PublishPointCloudContext


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  PublishTileNode::QueryPointCount(size_t maxPoints) const
    {
    // It doesn't seem from the API documentation that there is a way to count total points in a range without getting the points back. 
    PointCloudQueryBuffersPtr       queryBuffers = PointCloudQueryBuffers::Create(maxPoints, (uint32_t) PointCloudChannelId::Xyz);

    return m_publishContext.MakeQuery (queryBuffers, m_dgnRange, QUERY_DENSITY_FULL, 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PublishableTileGeometry PublishTileNode::_GeneratePublishableGeometry(DgnDbR dgnDb, TileGeometry::NormalMode normalMode, bool doPolylines, bool doInstancing, ITileGenerationFilterCP filter) const 
    {
    PublishableTileGeometry publishableGeometry;

    auto pointCloudScene = m_publishContext.m_model.GetPointCloudSceneP();
    if (nullptr == pointCloudScene)
        {
        BeAssert(!"Point cloud scene is missing");
        return publishableGeometry;
        }

    bool                                    useRGB = pointCloudScene->_HasRGBChannel();
    PointCloudQueryBuffersPtr               queryBuffers = PointCloudQueryBuffers::Create(s_maxTilePointCount, (uint32_t) PointCloudChannelId::Xyz | (useRGB ? (uint32_t) PointCloudChannelId::Rgb : 0));
    size_t                                  nPoints = m_publishContext.MakeQuery (queryBuffers, m_dgnRange, QUERY_DENSITY_LIMIT, (float) s_maxTilePointCount);
    DPoint3dCP                              pPoints;

    if (0 == nPoints || nullptr == (pPoints = queryBuffers->GetXyzChannel()->GetChannelBuffer()))
        {
        BeAssert(false && "No point cloud points");
        } 
    else
        {
        TileDisplayParamsCPtr               displayParams = TileDisplayParams::Create();
        static double                       s_clusterFraction = 1.0;
        double                              clusterTolerance = GetTolerance() * s_clusterFraction;
        Transform                           sceneToTile = Transform::FromProduct(GetTransformFromDgn(), m_publishContext.m_model.GetSceneToWorld());
        bvector<uint16_t>                   packedColors(nPoints);
        TileMeshPointCloud::Rgb const*      colorCP = nullptr == queryBuffers->GetRgbChannel() ? nullptr : reinterpret_cast<TileMeshPointCloud::Rgb const*> (queryBuffers->GetRgbChannel()->GetChannelBuffer());

        publishableGeometry.PointClouds().push_back(TileMeshPointCloud::Create(*displayParams, pPoints, colorCP, nullptr, nPoints, sceneToTile, clusterTolerance)); 
        }

    return publishableGeometry;
    }

END_UNNAMED_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus PointCloudModel::_GenerateMeshTiles(TileNodePtr& rootTile, TransformCR transformDbToTile, double leafTolerance, TileGenerator::ITileCollector& collector, ITileGenerationProgressMonitorR progressMeter) 
    {
    DRange3d                    dgnRange;
    PublishPointCloudContext    publishContext (*this, transformDbToTile, collector, progressMeter);

    GetRange(dgnRange, PointCloudModel::Unit::World);
    T_PublishTilePtr    rootPublishTile =  new PublishTileNode(publishContext, *this, dgnRange, transformDbToTile, 0, 0, nullptr);

    rootTile = rootPublishTile;

    publishContext.ProcessTile(*rootPublishTile, leafTolerance, 0);

    static const uint32_t s_sleepMillis = 1000.0;
    while (publishContext.ProcessingRemains())
        BeThreadUtilities::BeSleep(s_sleepMillis);

    return progressMeter._WasAborted() ? TileGeneratorStatus::Aborted : TileGeneratorStatus::Success;
    }

