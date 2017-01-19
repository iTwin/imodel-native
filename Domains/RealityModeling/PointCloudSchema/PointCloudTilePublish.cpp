/*-------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudTilePublish.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudInternal.h>
#include <PointCloud/PointCloudHandler.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_POINTCLOUD
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

BEGIN_UNNAMED_NAMESPACE

static size_t                           s_maxTilePointCount = 200000;

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     01/2017
//=======================================================================================
struct  PublishTileNode : ModelTileNode
{
    PointCloudModel const&      m_pointCloudModel;

    PublishTileNode(PointCloudModel const& model, DRange3dCR range, TransformCR transformDbToTile, size_t depth, size_t siblingIndex, TileNodeP parent)
        : ModelTileNode(model, range, transformDbToTile, depth, siblingIndex, parent, 0.0), m_pointCloudModel(model) { }

    PointCloudModel const& GetPointCloudModel() { return m_pointCloudModel; }
    virtual WString _GetFileExtension() const override { return L"pnts"; }

   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudQueryHandlePtr CreateQuery (int densityType, float densityValue) const
    {
    Transform                   worldToScene;
    DRange3d                    sceneRange;
                                
    worldToScene.InverseOf (m_pointCloudModel.GetSceneToWorld());
    worldToScene.Multiply (sceneRange, m_dgnRange);
    
    return  m_pointCloudModel.GetPointCloudSceneP()->CreateBoundingBoxQuery(sceneRange, densityType, densityValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  QueryPointCount(size_t maxPoints) const
    {
    // It doesn't seem from the API documentation that there is a way to count total points in a range without getting the points back. 
    PointCloudQueryHandlePtr                queryHandle = CreateQuery(QUERY_DENSITY_FULL, 1.0);
    RefCountedPtr<PointCloudQueryBuffers>   queryBuffers = PointCloudQueryBuffers::Create(maxPoints, (uint32_t) PointCloudChannelId::Xyz);

    return (size_t)  queryBuffers->GetPoints(queryHandle->GetHandle());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual PublishableTileGeometry _GeneratePublishableGeometry(DgnDbR dgnDb, TileGeometry::NormalMode normalMode, bool twoSidedTriangles, bool doPolylines, ITileGenerationFilterCP filter = nullptr) const override
    {
    PublishableTileGeometry                 publishableGeometry;
    PointCloudQueryHandlePtr                queryHandle = CreateQuery(QUERY_DENSITY_LIMIT, (float) s_maxTilePointCount);
    RefCountedPtr<PointCloudQueryBuffers>   queryBuffers = PointCloudQueryBuffers::Create(s_maxTilePointCount, (uint32_t) PointCloudChannelId::Xyz);
    uint32_t                                nPoints;
    DPoint3dCP                              pPoints;

    if (0 == (nPoints = queryBuffers->GetPoints(queryHandle->GetHandle())) ||
        nullptr == (pPoints = queryBuffers->GetXyzChannel()->GetChannelBuffer()))
        {
        BeAssert(false && "No point cloud points");
        } 
    else
        {
        TileDisplayParamsPtr    displayParams = TileDisplayParams::Create();

        publishableGeometry.PointClouds().push_back(TileMeshPointCloud::Create(displayParams, pPoints, nPoints, Transform::FromProduct(GetTransformFromDgn(), m_pointCloudModel.GetSceneToWorld()))); 
        }

    return publishableGeometry;
    }

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
    
    PublishPointCloudContext (PointCloudModel const& model, TransformCR transformDbToTile, TileGenerator::ITileCollector& collector, ITileGenerationProgressMonitorR progressMeter) :
                         m_model(model), m_transformDbToTile(transformDbToTile), m_collector(collector), m_progressMeter(progressMeter), m_totalTiles(1), m_progressTimer(true) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ProcessTile(PublishTileNode& tile, size_t depth, size_t siblingIndex)
    {
    if (tile.QueryPointCount(s_maxTilePointCount) >= s_maxTilePointCount)
        {
        bvector<DRange3d>   childRanges;
        size_t      childIndex = 0;

        TileNode::ComputeChildTileRanges (childRanges, tile.GetDgnRange());
        depth++;

        for (auto& childRange : childRanges)    
            {
            PublishTileNode     childTile(tile.GetPointCloudModel(), childRange, m_transformDbToTile, depth + 1, childIndex++, &tile);

            if (childTile.QueryPointCount(1) > 0)   
                {
                m_totalTiles++;
                tile.GetChildren().push_back(&childTile);
                ProcessTile(childTile, depth+1, childIndex++);
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
void    IndicateProgress()
    {
    // No.
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

END_UNNAMED_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus PointCloudModel::_GenerateMeshTiles(TileNodePtr& rootTile, TransformCR transformDbToTile, double leafTolerance, TileGenerator::ITileCollector& collector, ITileGenerationProgressMonitorR progressMeter) 
    {
    DRange3d                    dgnRange;
    PublishPointCloudContext    publishContext (*this, transformDbToTile, collector, progressMeter);

    GetRange(dgnRange, PointCloudModel::Unit::World);
    T_PublishTilePtr    rootPublishTile =  new PublishTileNode(*this, dgnRange, transformDbToTile, 0, 0, nullptr);

    rootTile = rootPublishTile;

    publishContext.ProcessTile(*rootPublishTile, 0, 0);

    static const uint32_t s_sleepMillis = 1000.0;
    while (publishContext.ProcessingRemains())
        BeThreadUtilities::BeSleep(s_sleepMillis);

    return progressMeter._WasAborted() ? TileGeneratorStatus::Aborted : TileGeneratorStatus::Success;
    }

