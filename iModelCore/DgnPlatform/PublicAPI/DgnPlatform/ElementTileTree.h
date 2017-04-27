/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ElementTileTree.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/TileTree.h>
#include <DgnPlatform/DgnModel.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/RenderPrimitives.h>

#define BEGIN_ELEMENT_TILETREE_NAMESPACE BEGIN_BENTLEY_DGN_NAMESPACE namespace ElementTileTree {
#define END_ELEMENT_TILETREE_NAMESPACE } END_BENTLEY_DGN_NAMESPACE
#define USING_NAMESPACE_ELEMENT_TILETREE using namespace BentleyApi::Dgn::ElementTileTree;

BEGIN_ELEMENT_TILETREE_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Root);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Loader);
DEFINE_POINTER_SUFFIX_TYPEDEFS(LoadContext);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Context);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Result);

DEFINE_REF_COUNTED_PTR(Tile);
DEFINE_REF_COUNTED_PTR(Root);
DEFINE_REF_COUNTED_PTR(Loader);

typedef bvector<TilePtr>    TileList;
typedef bvector<TileP>      TilePList;

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Loader : TileTree::TileLoader
{
    DEFINE_T_SUPER(TileTree::TileLoader);

private:
    Loader(TileR tile, TileTree::TileLoadStatePtr loads);

    folly::Future<BentleyStatus> _GetFromSource() override;
    BentleyStatus _LoadTile() override;
    folly::Future<BentleyStatus> _ReadFromDb() override { return ERROR; }
    folly::Future<BentleyStatus> _SaveToDb() override { return SUCCESS; }

    BentleyStatus DoGetFromSource();
public:
    static LoaderPtr Create(TileR tile, TileTree::TileLoadStatePtr loads) { return new Loader(tile, loads); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct LoadContext
{
private:
    LoaderCP    m_loader;
public:
    explicit LoadContext(LoaderCP loader) : m_loader(loader) { }

    bool WasAborted() const { return nullptr != m_loader && m_loader->IsCanceledOrAbandoned(); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Root : TileTree::OctTree::Root
{
    DEFINE_T_SUPER(TileTree::OctTree::Root);

private:
    struct SolidPrimitivePartMap
    {
        struct Key
        {
            ISolidPrimitivePtr                      m_solidPrimitive;
            DRange3d                                m_range;
            Render::Primitives::DisplayParamsCPtr   m_displayParams;

            Key() { }
            Key(ISolidPrimitiveR solidPrimitive, DRange3dCR range, Render::Primitives::DisplayParamsCR displayParams) : m_solidPrimitive(&solidPrimitive), m_range(range), m_displayParams(&displayParams) { }

            bool operator<(Key const& rhs) const;
            bool IsEqual(Key const& rhs) const;
        };

        typedef bmultimap<Key, Render::Primitives::GeomPartPtr> Map;

        Map m_map;

        Render::Primitives::GeomPartPtr FindOrInsert(ISolidPrimitiveR prim, DRange3dCR range, Render::Primitives::DisplayParamsCR displayParams, DgnElementId elemId, DgnDbR db);
    };

    typedef bmap<DgnGeometryPartId, Render::Primitives::GeomPartPtr> GeomPartMap;
    typedef bmap<DgnElementId, Render::Primitives::GeometryList> GeomListMap;

    DgnModelId                      m_modelId;
    Utf8String                      m_name;
    double                          m_leafTolerance;
    mutable BeMutex                 m_mutex;
    mutable BeSQLite::BeDbMutex     m_dbMutex;
    mutable GeomPartMap             m_geomParts;
    mutable SolidPrimitivePartMap   m_solidPrimitiveParts;
    mutable GeomListMap             m_geomLists;
    bool                            m_is3d;
    bool                            m_debugRanges;
    bool                            m_cacheGeometry;

    Root(GeometricModelR model, TransformCR transform, Render::SystemR system);

    Utf8CP _GetName() const override { return m_name.c_str(); }
    Render::ViewFlagsOverrides _GetViewFlagsOverrides() const override { return Render::ViewFlagsOverrides(); }

    bool LoadRootTile(DRange3dCR range, GeometricModelR model);
public:
    static RootPtr Create(GeometricModelR model, Render::SystemR system);
    virtual ~Root() { ClearAllTiles(); }

    DgnModelId GetModelId() const { return m_modelId; }
    GeometricModelPtr GetModel() const { return GetDgnDb().Models().Get<GeometricModel>(GetModelId()); }
    bool Is3d() const { return m_is3d; }
    bool Is2d() const { return !Is3d(); }
    bool WantDebugRanges() const { return m_debugRanges; }
    double GetLeafTolerance() const { return m_leafTolerance; }

    BeSQLite::BeDbMutex& GetDbMutex() const { return m_dbMutex; }

    Render::Primitives::GeomPartPtr GetGeomPart(DgnGeometryPartId partId) const;
    void AddGeomPart(DgnGeometryPartId partId, Render::Primitives::GeomPartR geomPart) const;
    Render::Primitives::GeomPartPtr FindOrInsertGeomPart(ISolidPrimitiveR prim, DRange3dCR range, Render::Primitives::DisplayParamsCR displayParams, DgnElementId elemId) const;

    bool GetCachedGeometry(Render::Primitives::GeometryList& geometry, DgnElementId elementId, double rangeDiagonalSquared) const;
    void AddCachedGeometry(Render::Primitives::GeometryList&& geometry, DgnElementId elementId, double rangeDiagonalSquared) const;
    bool WantCacheGeometry(double rangeDiagonalSquared) const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Tile : TileTree::OctTree::Tile
{
    DEFINE_T_SUPER(TileTree::OctTree::Tile);
private:
    double          m_tolerance;

    Tile(Root& root, TileTree::OctTree::TileId id, Tile const* parent, DRange3dCP range);

    TileTree::TileLoaderPtr _CreateTileLoader(TileTree::TileLoadStatePtr) override;
    TileTree::TilePtr _CreateChild(TileTree::OctTree::TileId) const override;
    double _GetMaximumSize() const override;

    Render::Primitives::MeshList GenerateMeshes(Render::Primitives::GeometryOptionsCR options, Render::Primitives::GeometryList const& geometries, bool doRangeTest, LoadContextCR context) const;
    Render::Primitives::GeometryList CollectGeometry(double tolerance, bool surfacesOnly, LoadContextCR context);
    Render::Primitives::GeometryCollection CreateGeometryCollection(Render::Primitives::GeometryList const&, Render::Primitives::GeometryOptionsCR, LoadContextCR context) const;

public:
    static TilePtr Create(Root& root, TileTree::OctTree::TileId id, Tile const& parent) { return new Tile(root, id, &parent, nullptr); }
    static TilePtr Create(Root& root, DRange3dCR range) { return new Tile(root, TileTree::OctTree::TileId::RootId(), nullptr, &range); }

    double GetTolerance() const { return m_tolerance; }
    DRange3d GetDgnRange() const;
    DRange3d GetTileRange() const { return GetRange(); }

    RootCR GetElementRoot() const { return static_cast<RootCR>(GetRoot()); }
    RootR GetElementRoot() { return static_cast<RootR>(GetRootR()); }

    Render::Primitives::GeometryCollection GenerateGeometry(Render::Primitives::GeometryOptionsCR options, LoadContextCR context);
};

END_ELEMENT_TILETREE_NAMESPACE

