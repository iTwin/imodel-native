/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <PointCloud/PointCloudHandler.h>
#include <DgnPlatform/CesiumTileTree.h>

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
//! Identifies a tile in an OctTree
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct TileId
{
    uint8_t     m_level;
    uint32_t    m_i;
    uint32_t    m_j;
    uint32_t    m_k;

    TileId(uint8_t level, uint32_t i, uint32_t j, uint32_t k) : m_level(level), m_i(i), m_j(j), m_k(k) { }
    TileId() : TileId(0,0,0,0) { }

    TileId CreateChildId(uint32_t i, uint32_t j, uint32_t k) const { return TileId(m_level+1, m_i*2+i, m_j*2+j, m_k*2+k); }
    TileId GetRelativeId(TileId parentId) const
        {
        BeAssert(parentId.m_level+1 == m_level);
        return TileId(parentId.m_level, m_i % 2, m_j % 2, m_k % 2);
        }

    static TileId RootId() { return TileId(0,0,0,0); }

    bool operator==(TileId const& other) const { return m_level == other.m_level && m_i == other.m_i && m_j == other.m_j && m_k == other.m_k; }
    bool IsRoot() const { return *this == RootId(); }
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley      02/2017
//=======================================================================================
struct Root : Dgn::Cesium::Root
{
    DEFINE_T_SUPER(Dgn::Cesium::Root);
private:
    PointCloudModelR m_model;

    Root(PointCloudModelR model, TransformCR transform) : T_Super(model.GetDgnDb(), transform, ""), m_model(model) { }
    void LoadRootTile(DRange3dCR tileRange, Cesium::OutputR);
public:
    static RootPtr Create(PointCloudModelR model, Cesium::OutputR);
    virtual ~Root() { ClearAllTiles(); }

    PointCloudModelCR GetPointCloudModel() const { return m_model; }
    PointCloudQueryHandlePtr InitQuery(bool& colorsPresent, DRange3dCR tileRange, size_t maxCount) const;
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley      02/2017
//=======================================================================================
struct Tile : Dgn::Cesium::Tile
{
    DEFINE_T_SUPER(Dgn::Cesium::Tile);
private:
    mutable Cesium::ChildTiles m_children;
    TileId m_id;
    double m_tolerance;
    Render::QPoint3dList m_points;
    bvector<PointCloudColorDef> m_colors;
    bool m_isLeaf;

    Tile(Root& root, TileId id, Tile const* parent, DRange3dCP range);

    Cesium::LoaderPtr _CreateLoader(Cesium::LoadStateR, Cesium::OutputR) override;
    double _GetMaximumSize() const override;

    Cesium::TilePtr CreateChild(TileId) const;
public:
    RootCR GetPointCloudRoot() const { return static_cast<RootCR>(GetRoot()); }
    double GetTolerance() const { return m_tolerance; }

    QPoint3dList& Points() { return m_points; }
    bvector<PointCloudColorDef>& Colors() { return m_colors; }

    static TilePtr Create(Root& root, TileId id, Tile const& parent) { return new Tile(root, id, &parent, nullptr); }
    static TilePtr Create(Root& root, DRange3dCR range) { return new Tile(root, TileId::RootId(), nullptr, &range); }

    BentleyStatus Read(StreamBuffer&);
    BentleyStatus AddGraphics(Cesium::OutputR);

    Utf8String _GetName() const override { return Utf8PrintfString("%d/%d/%d/%d", m_id.m_level, m_id.m_i, m_id.m_j, m_id.m_k); }
    Cesium::ChildTiles const* _GetChildren(bool load) const override;

    TileId GetTileId() const { return m_id; }
    TileId GetRelativeTileId() const;
    DRange3d ComputeChildRange(Tile&) const;

    bool IsLeaf() const { return m_isLeaf; }
    void SetIsLeaf(bool isLeaf = true) { m_isLeaf = isLeaf; }
};

END_POINTCLOUD_TILETREE_NAMESPACE
