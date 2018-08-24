/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ElementTileTree.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/TileTree.h>
#include <DgnPlatform/DgnModel.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/Render.h>

#define BEGIN_ELEMENT_TILETREE_NAMESPACE BEGIN_BENTLEY_DGN_NAMESPACE namespace ElementTileTree {
#define END_ELEMENT_TILETREE_NAMESPACE } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_ELEMENT_TILETREE using namespace BentleyApi::Dgn::ElementTileTree;

BEGIN_ELEMENT_TILETREE_NAMESPACE

using Root = TileTree::Root;

DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Loader);

DEFINE_REF_COUNTED_PTR(Tile);
DEFINE_REF_COUNTED_PTR(Loader);

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Loader : TileTree::TileLoader
{
    DEFINE_T_SUPER(TileTree::TileLoader);

private:
    uint64_t        m_createTime;       // The time of the most recent change to any element in the associated model when the tile loader was created.
    uint64_t        m_cacheCreateTime;  // The time of the most recent change to any element in the associated model when the tile cache data was created.
    Render::Primitives::GeometryCollection  m_geometry;

    Loader(TileR tile, TileTree::TileLoadStateSPtr loads);

    BentleyStatus _GetFromSource() override;
    BentleyStatus _LoadTile() override;
    bool _IsExpired(uint64_t) override;
    bool _IsValidData() override;
    bool _IsCompleteData() override;
    BentleyStatus _ReadFromDb() override;

    BentleyStatus LoadGeometryFromModel();
    BentleyStatus DoGetFromSource();
    bool IsExpired() const { return m_cacheCreateTime < m_createTime; }
    TileCR GetElementTile() const;
    TileR GetElementTile();

    uint64_t _GetCreateTime() const override { return m_createTime; }
public:
    static LoaderPtr Create(TileR tile, TileTree::TileLoadStateSPtr loads) { return new Loader(tile, loads); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Tile : TileTree::Tile
{
    DEFINE_T_SUPER(TileTree::Tile);

protected:
    double                      m_tolerance;

    Tile(Root& root, TileTree::TileId id, Tile const* parent, DRange3dCP range, bool displayable);
    explicit Tile(Tile const& parent);
    Tile(Root& root, TileTree::TileId id, DRange3dCR range, double minToleranceRatio);
    ~Tile();

    void InitTolerance(double minToleranceRatio, bool isLeaf=false);

    TileTree::TileLoaderPtr _CreateTileLoader(TileTree::TileLoadStateSPtr) override;
    TileTree::TilePtr _CreateChild(TileTree::TileId) const override;
    double _GetMaximumSize() const override;

    Utf8String _GetTileCacheKey() const override;

    ChildTiles const* _GetChildren(bool load) const override;

    Render::Primitives::MeshList GenerateMeshes(Render::Primitives::GeometryList const& geometries, bool doRangeTest, TileTree::LoadContextCR context) const;
public:
    static TilePtr Create(Root& root, TileTree::TileId id, Tile const& parent) { return new Tile(root, id, &parent, nullptr, true); }
    static TilePtr CreateRoot(Root& root, DRange3dCR range, bool populate) { return new Tile(root, TileTree::TileId::RootId(), nullptr, &range, populate); }
    static TilePtr CreateWithZoomFactor(Tile const& parent) { return new Tile(parent); }

    double GetTolerance() const { return m_tolerance; }
    DRange3d GetDgnRange() const;
    DRange3d GetTileRange() const { return GetRange(); }

    Render::Primitives::GeometryCollection GenerateGeometry(TileTree::LoadContextCR context);

    bool _ToJson(Json::Value&) const override;
    Utf8String GetIdString() const;
    TilePtr FindTile(TileTree::TileId id, double zoomFactor);
    TilePtr FindTile(double zoomFactor);
    TileP GetElementParent() const { return const_cast<TileP>(static_cast<TileCP>(GetParent())); }
};

END_ELEMENT_TILETREE_NAMESPACE

