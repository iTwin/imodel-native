/*-------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudTileTree.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudInternal.h>
#include <PointCloud/PointCloudHandler.h>
#include <PointCloud/PointCloudTileTree.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_POINTCLOUD
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD
USING_NAMESPACE_POINTCLOUD_TILETREE



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::Tile(Root& octRoot, TileTree::OctTree::TileId id, Tile const* parent, DRange3dCP range)
    : T_Super(octRoot, id, parent, false)
    {
    static const double s_minToleranceRatio = 1000.0;

    if (nullptr != parent)
        m_range = ElementAlignedBox3d(parent->ComputeChildRange(*this));
    else
        m_range.Extend (*range);

    m_tolerance = m_range.DiagonalDistance() / s_minToleranceRatio;
    }

DEFINE_POINTER_SUFFIX_TYPEDEFS(Loader)
DEFINE_REF_COUNTED_PTR(Loader)

//=======================================================================================
// @bsistruct                                                    Ray.Bentley    02/2017
//=======================================================================================
struct Loader : TileTree::TileLoader
{
    DEFINE_T_SUPER(TileTree::TileLoader);

private:
    Loader(TileR tile, TileTree::TileLoadStatePtr loads) : T_Super("", tile, loads, "") { }

    folly::Future<BentleyStatus> _ReadFromDb() override { return ERROR; }
    folly::Future<BentleyStatus> _SaveToDb() override   { return SUCCESS; }
    BentleyStatus DoGetFromSource() { return IsCanceledOrAbandoned() ? ERROR : SUCCESS; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus _LoadTile()
    {
    auto&                       tile = static_cast<TileR>(*m_tile);
    auto&                       root = static_cast<RootCR>(m_tile->GetRoot());
    auto&                       system = *root.GetRenderSystem();
    PointCloudModelCR           pointCloudModel = root.GetPointCloudModel();
    static size_t               s_maxTilePointCount = 500000;
    static size_t               s_maxLeafPointCount = 20000;
    bool                        useRGB = pointCloudModel.GetPointCloudSceneP()->_HasRGBChannel();
    PointCloudQueryBuffersPtr   queryBuffers = PointCloudQueryBuffers::Create(s_maxTilePointCount, (uint32_t) PointCloudChannelId::Xyz | (useRGB ? (uint32_t) PointCloudChannelId::Rgb : 0));
    size_t                      nPoints = root.MakeQuery (queryBuffers, m_tile->GetRange(), QUERY_DENSITY_LIMIT, (float) s_maxTilePointCount);
    DPoint3dCP                  pPoints;
    Transform                   dgnToTile, cloudToTile;


    dgnToTile.InverseOf(root.GetLocation());
    cloudToTile = Transform::FromProduct(dgnToTile, pointCloudModel.GetSceneToWorld());
    if (nullptr == (pPoints = queryBuffers->GetXyzChannel()->GetChannelBuffer()))
        {
        BeAssert(false && "No point cloud points");
        return ERROR;
        } 
    if(nPoints < s_maxLeafPointCount)
        tile.SetIsLeaf();

    if (0 != nPoints)
        {
        bvector<FPoint3d>           cloudPoints(nPoints);

        for(size_t i=0; i<nPoints; i++)
            {
            DPoint3d        tilePoint;

            cloudToTile.Multiply(tilePoint, pPoints[i]);
            cloudPoints[i] = {(float) tilePoint.x, (float) tilePoint.y, (float) tilePoint.z}; 
            }
        Render::GraphicBuilderPtr graphic = system._CreateGraphic(Graphic::CreateParams());

        graphic->AddPointCloud((int) nPoints, DPoint3d::FromZero(), &cloudPoints.front(), useRGB ? (ByteCP) queryBuffers->GetRgbChannel()->GetChannelBuffer() : nullptr);

#define DRAW_RANGE
#ifdef DRAW_RANGE
        GraphicParams params;
        params.SetLineColor(ColorDef::Red());

        graphic->ActivateGraphicParams(params);
        graphic->AddRangeBox (tile.GetRange());
#endif
        graphic->Close();
        tile.AddGraphic(*graphic);
        }

    return SUCCESS;  
    }


public:
    static LoaderPtr Create(TileR tile, TileTree::TileLoadStatePtr loads) { return new Loader(tile, loads); }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> _GetFromSource()
    {
    LoaderPtr me(this);
    return folly::via(&BeFolly::ThreadPool::GetCpuPool(), [me]() { return me->DoGetFromSource(); });
    }

};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::TileLoaderPtr Tile::Tile::_CreateTileLoader(TileTree::TileLoadStatePtr loadState) 
    {
    return Loader::Create(*this, loadState);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::TilePtr Tile::_CreateChild(TileTree::OctTree::TileId childId) const 
    {
    return Tile::Create(const_cast<RootR>(GetPointCloudRoot()), childId, *this);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
double Tile::_GetMaximumSize() const
    {
    return 512; // ###TODO: come up with a decent value, and account for device ppi
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Root::LoadRootTile(DRange3dCR tileRange)
    {
    m_rootTile = Tile::Create(*this, tileRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  Root::MakeQuery (PointCloudQueryBuffersPtr& queryBuffers, DRange3dCR tileRange, int densityType, float densityValue) const
    {
    Transform                   worldToScene;
    DRange3d                    sceneRange;
                                
    worldToScene.InverseOf (m_model.GetSceneToWorld());
    Transform::FromProduct (worldToScene, GetLocation()).Multiply (sceneRange, tileRange);
    
    PointCloudQueryHandlePtr    queryHandle = m_model.GetPointCloudSceneP()->CreateBoundingBoxQuery(sceneRange, densityType, densityValue);

    return queryBuffers->GetPoints (queryHandle->GetHandle());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Root::Root(PointCloudModelR model, TransformCR transform, Render::SystemR system, ViewControllerCR view)
    : T_Super(model.GetDgnDb(), transform, "", &system), m_model(model), m_name(model.GetName())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RootPtr Root::Create(PointCloudModelR model, Render::SystemR system, ViewControllerCR view)
    {
    DgnDb::VerifyClientThread();

    DRange3d                    dgnRange;
    model.GetRange(dgnRange, PointCloudModel::Unit::World);

    // Translate world coordinates to center of range in order to reduce precision errors
    DPoint3d    centroid = DPoint3d::FromInterpolate(dgnRange.low, 0.5, dgnRange.high);
    Transform   transformFromTile = Transform::From(centroid), transformToTile;

    transformToTile.InverseOf(transformFromTile);

    RootPtr     root = new Root(model, transformFromTile, system, view);
    DRange3d    tileRange;

    transformToTile.Multiply(tileRange, dgnRange);
    root->LoadRootTile(tileRange);

    return root;
    }


