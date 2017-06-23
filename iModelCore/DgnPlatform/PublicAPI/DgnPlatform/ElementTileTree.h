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
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeomPartBuilder);

DEFINE_REF_COUNTED_PTR(Tile);
DEFINE_REF_COUNTED_PTR(Root);
DEFINE_REF_COUNTED_PTR(Loader);
DEFINE_REF_COUNTED_PTR(GeomPartBuilder);

typedef bvector<TilePtr>    TileList;
typedef bvector<TileP>      TilePList;

#define TILECACHE_DEBUG

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Loader : TileTree::TileLoader
{
    DEFINE_T_SUPER(TileTree::TileLoader);

#ifdef TILECACHE_DEBUG
    double  m_loadTime = 0.0;
#endif

private:
    Loader(TileR tile, TileTree::TileLoadStatePtr loads, Dgn::Render::SystemP renderSys);

    folly::Future<BentleyStatus> _GetFromSource() override;
    BentleyStatus _LoadTile() override;

    BentleyStatus LoadGeometryFromModel(Render::Primitives::GeometryCollection& geometry);
    BentleyStatus DoGetFromSource();
public:
    static LoaderPtr Create(TileR tile, TileTree::TileLoadStatePtr loads, Dgn::Render::SystemP renderSys) { return new Loader(tile, loads, renderSys); }
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
    Dgn::Render::SystemP GetRenderSystem() const {return m_loader->GetRenderSystem();}
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
struct GeomPartBuilder : RefCountedBase
{
private:
    BeConditionVariable             m_cv;
    Render::Primitives::GeomPartPtr m_part;

    GeomPartBuilder() { }
public:
    static GeomPartBuilderPtr Create() { return new GeomPartBuilder(); }

    Render::Primitives::GeomPartPtr WaitForPart();
    Render::Primitives::GeomPartPtr GeneratePart(DgnGeometryPartId partId, DgnDbR db, Render::GeometryParamsR geomParams, ViewContextR viewContext);
    BeMutex& GetMutex() { return m_cv.GetMutex(); }
    void NotifyAll() { BeAssert(m_part.IsValid()); m_cv.notify_all(); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
struct GeomPartCache
{
private:
    typedef bmap<DgnGeometryPartId, Render::Primitives::GeomPartPtr> PartMap;
    typedef bmap<DgnGeometryPartId, GeomPartBuilderPtr> BuilderMap;

    std::mutex  m_mutex;
    PartMap     m_parts;
    BuilderMap  m_builders;
public:
    Render::Primitives::GeomPartPtr FindOrInsert(DgnGeometryPartId, DgnDbR, Render::GeometryParamsR, ViewContextR);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Root : TileTree::OctTree::Root
{
    DEFINE_T_SUPER(TileTree::OctTree::Root);

    enum class DebugOptions
    {
        None = 0,
        ShowBoundingVolume = 1 << 0,
        ShowContentVolume = 1 << 1,
    };
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

    typedef bmap<DgnElementId, Render::Primitives::GeometryList> GeomListMap;

    DgnModelId                      m_modelId;
    Utf8String                      m_name;
    mutable BeMutex                 m_mutex;
    mutable std::mutex              m_dbMutex;
    mutable GeomPartCache           m_geomParts;
    mutable SolidPrimitivePartMap   m_solidPrimitiveParts;
    mutable GeomListMap             m_geomLists;
    bool                            m_is3d;
    DebugOptions                    m_debugOptions = DebugOptions::None;
    bool                            m_cacheGeometry;

    Root(GeometricModelR model, TransformCR transform, Render::SystemR system);

    Utf8CP _GetName() const override { return m_name.c_str(); }
    Render::ViewFlagsOverrides _GetViewFlagsOverrides() const override { return Render::ViewFlagsOverrides(); }

    bool LoadRootTile(DRange3dCR range, GeometricModelR model, bool populate);
public:
    static RootPtr Create(GeometricModelR model, Render::SystemR system);
    virtual ~Root() { ClearAllTiles(); }

    DgnModelId GetModelId() const { return m_modelId; }
    GeometricModelPtr GetModel() const { return GetDgnDb().Models().Get<GeometricModel>(GetModelId()); }
    bool Is3d() const { return m_is3d; }
    bool Is2d() const { return !Is3d(); }
    DebugOptions GetDebugOptions() const;
    void SetDebugOptions(DebugOptions opts) { m_debugOptions = opts; }

    std::mutex& GetDbMutex() const { return m_dbMutex; }

    Render::Primitives::GeomPartPtr FindOrInsertGeomPart(DgnGeometryPartId partId, Render::GeometryParamsR geomParams, ViewContextR viewContext);
    Render::Primitives::GeomPartPtr FindOrInsertGeomPart(ISolidPrimitiveR prim, DRange3dCR range, Render::Primitives::DisplayParamsCR displayParams, DgnElementId elemId) const;

    bool GetCachedGeometry(Render::Primitives::GeometryList& geometry, DgnElementId elementId, double rangeDiagonalSquared) const;
    void AddCachedGeometry(Render::Primitives::GeometryList&& geometry, DgnElementId elementId, double rangeDiagonalSquared) const;
    bool WantCacheGeometry(double rangeDiagonalSquared) const;

    DGNPLATFORM_EXPORT static void ToggleDebugBoundingVolumes();
};

ENUM_IS_FLAGS(Root::DebugOptions);

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Tile : TileTree::OctTree::Tile
{
    DEFINE_T_SUPER(TileTree::OctTree::Tile);

private:
    struct DebugGraphics
    {
        Render::GraphicPtr  m_graphic;
        Root::DebugOptions  m_options = Root::DebugOptions::None;
    };

    double                      m_tolerance;
    mutable ElementAlignedBox3d m_contentRange;
    mutable DebugGraphics       m_debugGraphics;
    double                      m_zoomFactor = 1.0;
    bool                        m_hasZoomFactor = false;
    bool                        m_displayable = true;

    Tile(Root& root, TileTree::OctTree::TileId id, Tile const* parent, DRange3dCP range, bool displayable);
    explicit Tile(Tile const& parent);

    void InitTolerance();

    TileTree::TileLoaderPtr _CreateTileLoader(TileTree::TileLoadStatePtr, Dgn::Render::SystemP renderSys = nullptr) override;
    TileTree::TilePtr _CreateChild(TileTree::OctTree::TileId) const override;
    double _GetMaximumSize() const override;
    void _Invalidate() override;
    bool _IsInvalidated(TileTree::DirtyRangesCR dirty) const override;
    void _DrawGraphics(TileTree::DrawArgsR) const override;
    Utf8String _GetTileCacheKey() const override;
    ChildTiles const* _GetChildren(bool load) const override;

    Render::Primitives::MeshList GenerateMeshes(Render::Primitives::GeometryList const& geometries, bool doRangeTest, LoadContextCR context) const;
    Render::Primitives::GeometryList CollectGeometry(LoadContextCR context);
    Render::Primitives::GeometryCollection CreateGeometryCollection(Render::Primitives::GeometryList const&, LoadContextCR context) const;

    Render::GraphicPtr GetDebugGraphics(Root::DebugOptions options) const;
public:
    static TilePtr Create(Root& root, TileTree::OctTree::TileId id, Tile const& parent) { return new Tile(root, id, &parent, nullptr, true); }
    static TilePtr CreateRoot(Root& root, DRange3dCR range, bool populate) { return new Tile(root, TileTree::OctTree::TileId::RootId(), nullptr, &range, populate); }
    static TilePtr CreateWithZoomFactor(Tile const& parent) { return new Tile(parent); }

    double GetTolerance() const { return m_tolerance; }
    DRange3d GetDgnRange() const;
    DRange3d GetTileRange() const { return GetRange(); }
    ElementAlignedBox3d const& _GetContentRange() const override { return m_contentRange.IsNull() ? GetRange() : m_contentRange; }

    RootCR GetElementRoot() const { return static_cast<RootCR>(GetRoot()); }
    RootR GetElementRoot() { return static_cast<RootR>(GetRootR()); }

    Render::Primitives::GeometryCollection GenerateGeometry(LoadContextCR context);

    void SetZoomFactor(double zoom) { BeAssert(!IsLeaf()); m_zoomFactor = zoom; m_hasZoomFactor = true; }
    bool HasZoomFactor() const { return m_hasZoomFactor; }
    double GetZoomFactor() const { BeAssert(HasZoomFactor()); return HasZoomFactor() ? m_zoomFactor : 1.0; }
    void SetContentRange (ElementAlignedBox3dCR contentRange) { m_contentRange = contentRange; }
    Utf8String GetDebugId() const { return _GetTileCacheKey(); }
};

END_ELEMENT_TILETREE_NAMESPACE

