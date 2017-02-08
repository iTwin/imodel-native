/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloud/PointCloudTileTree.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <PointCloud/PointCloudHandler.h>
#include <DgnPlatform/TileTree.h>


#define BEGIN_POINTCLOUD_TILETREE_NAMESPACE BEGIN_BENTLEY_DGN_NAMESPACE namespace PointCloudTileTree {
#define END_POINTCLOUD_TILETREE_NAMESPACE } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_POINTCLOUD_TILETREE using namespace BentleyApi::Dgn::PointCloudTileTree;

BEGIN_POINTCLOUD_TILETREE_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(Root)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile)
DEFINE_REF_COUNTED_PTR(Root)
DEFINE_REF_COUNTED_PTR(Tile)

USING_NAMESPACE_BENTLEY_POINTCLOUD


//=======================================================================================
// @bsiclass                                                    Ray.Bentley      02/2017
//=======================================================================================
struct Root : Dgn::TileTree::OctTree::Root
{
    DEFINE_T_SUPER(TileTree::OctTree::Root);

private:
    PointCloudModelR    m_model;
    Utf8String          m_name;

    virtual Utf8CP _GetName() const override { return m_name.c_str(); }
    Root(PointCloudModelR model, TransformCR transform, Render::SystemR system, ViewControllerCR view);
    void LoadRootTile(DRange3dCR tileRange);

public:
    static RootPtr Create(PointCloudModelR model, Render::SystemR system, ViewControllerCR view);
    virtual ~Root() { ClearAllTiles(); }

    size_t              MakeQuery (PointCloudQueryBuffersPtr& queryBuffers, DRange3dCR tileRange, int densityType, float densityValue) const;
    PointCloudModelCR   GetPointCloudModel() const { return m_model; }


};//     TileRoot


//=======================================================================================
// @bsiclass                                                    Ray.Bentley      02/2017
//=======================================================================================
struct Tile : Dgn::TileTree::OctTree::Tile
{
    DEFINE_T_SUPER(TileTree::OctTree::Tile);

private:
    double                  m_tolerance;

    Tile(Root& root, TileTree::OctTree::TileId id, Tile const* parent, DRange3dCP range);

    virtual TileTree::TileLoaderPtr _CreateTileLoader(TileTree::TileLoadStatePtr) override;
    virtual TileTree::TilePtr _CreateChild(TileTree::OctTree::TileId) const override;
    virtual double _GetMaximumSize() const override;

public:
    RootCR GetPointCloudRoot() const { return static_cast<RootCR>(GetRoot()); }

    static TilePtr Create(Root& root, TileTree::OctTree::TileId id, Tile const& parent) { return new Tile(root, id, &parent, nullptr); }
    static TilePtr Create(Root& root, DRange3dCR range) { return new Tile(root, TileTree::OctTree::TileId::RootId(), nullptr, &range); }

    double GetTolerance() const         { return m_tolerance; }

};//     TileNode


END_POINTCLOUD_TILETREE_NAMESPACE
