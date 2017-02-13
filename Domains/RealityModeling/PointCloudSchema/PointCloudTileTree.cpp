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
    static size_t               s_maxTilePointCount = 1000000;
    static size_t               s_maxLeafPointCount = 20000;
    bvector<FPoint3d>           points(s_maxTilePointCount);
    bvector<PointCloudColorDef> colors(s_maxTilePointCount);
    bool                        colorsPresent;
    static   BeMutex            s_queryMutex;
    BeMutexHolder               lock(s_queryMutex);        // Arrgh.... Queries are not thread safe??
    PointCloudQueryHandlePtr    queryHandle = root.InitQuery(colorsPresent, tile.GetRange(), s_maxTilePointCount);
    size_t                      nPoints, totalPoints = 0;
    
    while (0 != (nPoints = (size_t) ptGetDetailedQueryPointsf (queryHandle->GetHandle(), s_maxTilePointCount, (float*)points.data(), colorsPresent? (Byte*) colors.data() : nullptr, nullptr, nullptr, nullptr, nullptr, 0, nullptr, nullptr)))
        {
        Transform                   dgnToTile, cloudToTile;
        Render::GraphicBuilderPtr   graphic = system._CreateGraphic(Graphic::CreateParams());

        dgnToTile.InverseOf(root.GetLocation());
        cloudToTile = Transform::FromProduct(dgnToTile, root.GetPointCloudModel().GetSceneToWorld());

        for(size_t i=0; i<nPoints; i++)
            {
            DPoint3d        tmpPoint = {points[i].x, points[i].y, points[i].z};

            cloudToTile.Multiply(tmpPoint);

            points[i] = {(float) tmpPoint.x, (float) tmpPoint.y, (float) tmpPoint.z}; 
            }


        // TODO - Add technique for monochrome point clouds...
        if (!colorsPresent)
            colors =  bvector<PointCloudColorDef> (nPoints, PointCloudColorDef(255, 255, 255));

        graphic->AddPointCloud((int) nPoints, DPoint3d::FromZero(), &points.front(), colors.empty() ? nullptr : (ByteCP) colors.data());

        tile.AddGraphic(*graphic);
        totalPoints += nPoints;
        graphic->Close();
        }

    if(totalPoints < s_maxLeafPointCount)
        tile.SetIsLeaf();

#ifdef DRAW_RANGE
    {
    GraphicParams               params;
    Render::GraphicBuilderPtr   graphic = system._CreateGraphic(Graphic::CreateParams());

    params.SetLineColor(ColorDef::Red());

    graphic->ActivateGraphicParams(params);
    graphic->AddRangeBox (tile.GetRange());
    graphic->Close();
    tile.AddGraphic();
    }
#endif

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
#ifdef ASYNCH_ROOT
    // Push to always request root tile --- this provides a low resolution proxy while the actual nodes load (but delays final diaplay).
    auto result = _RequestTile(*m_rootTile, nullptr);
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
    
    colorsPresent = m_model.GetPointCloudSceneP()->_HasRGBChannel();
    worldToScene.InverseOf (m_model.GetSceneToWorld());
    Transform::FromProduct (worldToScene, GetLocation()).Multiply (sceneRange, tileRange);

    DVec3d              diagonalVector = sceneRange.DiagonalVector();
    Transform           eyeTransform = Transform::FromProduct (Transform::From(-1.0, -1.0, -1.0), 
                                                               Transform::FromScaleFactors(2.0 / diagonalVector.x, 2.0/diagonalVector.y, 2.0/diagonalVector.z), 
                                                               Transform::From(-sceneRange.low.x, -sceneRange.low.y, -sceneRange.low.z));
    DMatrix4d           identityMatrix, eyeMatrix = DMatrix4d::From(eyeTransform);


    identityMatrix.InitIdentity();

    ptSetViewProjectionMatrix(&identityMatrix.coff[0][0], true);
    ptSetViewEyeMatrix(&eyeMatrix.coff[0][0], true);
    ptSetViewportSize(0, 0, 512, 512);

    PointCloudQueryHandlePtr    queryHandle = PointCloudQueryHandle::Create(ptCreateFrustumPointsQuery());

    ptResetQuery(queryHandle->GetHandle());
    ptSetQueryScope(queryHandle->GetHandle(), m_model.GetPointCloudSceneP()->GetSceneHandle());
    ptSetQueryDensity (queryHandle->GetHandle(), QUERY_DENSITY_VIEW_COMPLETE, 1.0);

    while (0 != ptPtsToLoadInViewport(m_model.GetPointCloudSceneP()->GetSceneHandle(), true))
        BeDuration::FromMilliseconds(10).Sleep();

    return queryHandle;
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


