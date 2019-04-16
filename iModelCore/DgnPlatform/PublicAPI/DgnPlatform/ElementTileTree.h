/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Root);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Loader);
DEFINE_POINTER_SUFFIX_TYPEDEFS(LoadContext);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Context);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Result);
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeomPartBuilder);
DEFINE_POINTER_SUFFIX_TYPEDEFS(TileGenerator);
DEFINE_POINTER_SUFFIX_TYPEDEFS(ThematicMeshBuilder);

DEFINE_REF_COUNTED_PTR(Tile);
DEFINE_REF_COUNTED_PTR(Root);
DEFINE_REF_COUNTED_PTR(Loader);
DEFINE_REF_COUNTED_PTR(GeomPartBuilder);
DEFINE_REF_COUNTED_PTR(ThematicMeshBuilder);

typedef bvector<TilePtr>    TileList;
typedef bvector<TileP>      TilePList;

typedef std::unique_ptr<TileGenerator> TileGeneratorUPtr;

//#define TILECACHE_DEBUG

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Loader : TileTree::TileLoader
{
    DEFINE_T_SUPER(TileTree::TileLoader);

private:
    uint64_t        m_createTime;       // The time of the most recent change to any element in the associated model when the tile loader was created.
    uint64_t        m_cacheCreateTime;  // The time of the most recent change to any element in the associated model when the tile cache data was created.
    DgnElementIdSet m_omitElemIds;      // IDs of any elements present in cache FeatureTable but since deleted from associated model or modified.
    DgnElementIdSet m_tileElemIds;      // IDs of elements present in cache, modulo any present in m_omitElemIds.
    Render::Primitives::GeometryCollection  m_geometry;
    bool m_doTileRepair;

    Loader(TileR tile, TileTree::TileLoadStatePtr loads, Dgn::Render::SystemP renderSys);

    folly::Future<BentleyStatus> _GetFromSource() override;
    BentleyStatus _LoadTile() override;
    bool _IsExpired(uint64_t) override;
    bool _IsValidData() override;
    bool _IsCompleteData() override;
    folly::Future<BentleyStatus> _ReadFromDb() override;

    BentleyStatus LoadGeometryFromModel();
    void SetupForTileRepair();
    BentleyStatus DoGetFromSource();
    bool IsCacheable() const;
    bool IsExpired() const { return m_cacheCreateTime < m_createTime; }
    TileCR GetElementTile() const;
    TileR GetElementTile();

    uint64_t _GetCreateTime() const override { return m_createTime; }
    bool _WantWaitOnSave() const override;
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
    BeTimePoint m_deadline;
public:
    explicit LoadContext(LoaderCP loader) : m_loader(loader)
        {
        if (nullptr != loader && loader->WantPartialTiles() && loader->HasPartialTimeout())
            m_deadline = BeTimePoint::Now() + loader->GetPartialTimeout();
        }

    bool WasAborted() const { return nullptr != m_loader && m_loader->IsCanceledOrAbandoned(); }
    Dgn::Render::SystemP GetRenderSystem() const {return m_loader->GetRenderSystem();}
    bool IsPastCollectionDeadline() const { return m_deadline.IsValid() && m_deadline.IsInPast(); }
    bool WantPartialTiles() const { return m_deadline.IsValid(); }
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
protected:
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

    Utf8String                      m_name;
    mutable BeMutex                 m_mutex;
    mutable std::mutex              m_dbMutex;
    mutable GeomPartCache           m_geomParts;
    mutable SolidPrimitivePartMap   m_solidPrimitiveParts;
    mutable GeomListMap             m_geomLists;
    DebugOptions                    m_debugOptions = DebugOptions::None;
    bool                            m_cacheGeometry;

    Root(GeometricModelR model, TransformCR transform, Render::SystemR system);

    Utf8CP _GetName() const override { return m_name.c_str(); }
    Render::ViewFlagsOverrides _GetViewFlagsOverrides() const override { return Render::ViewFlagsOverrides(); }

    void RemoveCachedGeometry(DRange3dCR range, DgnElementId id);
    void _OnRemoveFromRangeIndex(DRange3dCR range, DgnElementId id) override { RemoveCachedGeometry(range, id); }
    void _OnUpdateRangeIndex(DRange3dCR oldRange, DRange3dCR newRange, DgnElementId id) override { RemoveCachedGeometry(oldRange, id); }
    void _OnProjectExtentsChanged(AxisAlignedBox3dCR) override;

    bool LoadRootTile(DRange3dCR range, GeometricModelR model, bool populate);
public:
    static RootPtr Create(GeometricModelR model, Render::SystemR system);
    static RootPtr Create(GeometricModelR model, RenderContextR context);
    virtual ~Root() { ClearAllTiles(); }

    GeometricModelPtr GetModel() const { return GetDgnDb().Models().Get<GeometricModel>(GetModelId()); }
    DebugOptions GetDebugOptions() const;
    void SetDebugOptions(DebugOptions opts) { m_debugOptions = opts; }

    std::mutex& GetDbMutex() const { return m_dbMutex; }

    Render::Primitives::GeomPartPtr FindOrInsertGeomPart(DgnGeometryPartId partId, Render::GeometryParamsR geomParams, ViewContextR viewContext);
    Render::Primitives::GeomPartPtr FindOrInsertGeomPart(ISolidPrimitiveR prim, DRange3dCR range, Render::Primitives::DisplayParamsCR displayParams, DgnElementId elemId) const;

    bool GetCachedGeometry(Render::Primitives::GeometryList& geometry, DgnElementId elementId, double rangeDiagonalSquared) const;
    void AddCachedGeometry(Render::Primitives::GeometryList const& geometry, size_t startIndex, DgnElementId elementId, double rangeDiagonalSquared) const;
    bool WantCacheGeometry(double rangeDiagonalSquared) const;

    DGNPLATFORM_EXPORT static void ToggleDebugBoundingVolumes();

    Transform GetLocationForTileGeneration() const; //!< @private

    template<typename T> auto UnderMutex(T func) const -> decltype(func()) { BeMutexHolder lock(m_mutex); return func(); }

    bool _ToJson(Json::Value&) const override;
    TileTree::TilePtr _FindTileById(Utf8CP id) override;
};

ENUM_IS_FLAGS(Root::DebugOptions);

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct Tile : TileTree::OctTree::Tile
{
    DEFINE_T_SUPER(TileTree::OctTree::Tile);

protected:
    struct DebugGraphics
    {
        Render::GraphicPtr  m_graphic;
        Root::DebugOptions  m_options = Root::DebugOptions::None;

        bool IsUsable(Root::DebugOptions opts) const { return m_options == opts && m_graphic.IsNull() == (Root::DebugOptions::None == opts); }
        void Reset() { m_graphic = nullptr; m_options = Root::DebugOptions::None; }
    };

    double                      m_tolerance;
    mutable ElementAlignedBox3d m_contentRange;
    mutable DebugGraphics       m_debugGraphics;
    double                      m_zoomFactor = 1.0;
    uint64_t                    m_debugId;
    Render::GraphicPtr          m_backupGraphic;
    TileGeneratorUPtr           m_generator;
    bool                        m_hasZoomFactor = false;
    bool                        m_displayable = true;

    Tile(Root& root, TileTree::OctTree::TileId id, Tile const* parent, DRange3dCP range, bool displayable);
    explicit Tile(Tile const& parent);
    Tile(Root& root, TileTree::OctTree::TileId id, DRange3dCR range, double minToleranceRatio);
    ~Tile();

    void InitTolerance(double minToleranceRatio, bool isLeaf=false);

    TileTree::TileLoaderPtr _CreateTileLoader(TileTree::TileLoadStatePtr, Dgn::Render::SystemP renderSys = nullptr) override;
    TileTree::TilePtr _CreateChild(TileTree::OctTree::TileId) const override;
    double _GetMaximumSize() const override;
    void _Invalidate() override;
    bool _IsInvalidated(TileTree::DirtyRangesCR dirty) const override;
    void _UpdateRange(DRange3dCR parentOld, DRange3dCR parentNew) override;

    SelectParent SelectTiles(bvector<TileTree::TileCPtr>& selected, TileTree::DrawArgsR args, uint32_t skipDepth) const;
    SelectParent _SelectTiles(bvector<TileTree::TileCPtr>& selected, TileTree::DrawArgsR args) const override { return SelectTiles(selected, args, 0); }
    void _DrawGraphics(TileTree::DrawArgsR) const override;
    Utf8String _GetTileCacheKey() const override;

    ChildTiles const* _GetChildren(bool load) const override;
    void _ValidateChildren() const override;

    Render::Primitives::MeshList GenerateMeshes(Render::Primitives::GeometryList const& geometries, bool doRangeTest, LoadContextCR context) const;

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

    bool _HasBackupGraphics() const override { return m_backupGraphic.IsValid(); }
    bool _HasGraphics() const override { return m_graphic.IsValid() || _HasBackupGraphics(); }
    void ClearBackupGraphic() { m_backupGraphic = nullptr; }

    bool _IsPartial() const override { return nullptr != m_generator.get(); }
    void UpdateRange(DRange3dCR parentOld, DRange3dCR parentNew, bool allowShrink);

    virtual bool IsCacheable() const;
    void SetGenerator(TileGeneratorUPtr&&);
    void SetDisplayable(bool displayable) { m_displayable = displayable; }

    bool _ToJson(Json::Value&) const override;
    Utf8String GetIdString() const;
    TilePtr FindTile(TileTree::OctTree::TileId id, double zoomFactor);
    TilePtr FindTile(double zoomFactor);
    TileP GetElementParent() const { return const_cast<TileP>(static_cast<TileCP>(GetParent())); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/18
//=======================================================================================
struct TileCache : TileTree::TileCache
{
    DEFINE_T_SUPER(TileTree::TileCache);
private:
    explicit TileCache(uint64_t maxSize) : T_Super(maxSize) { }

    BentleyStatus _Initialize() const final;
    bool _ValidateData() const final;

    static BeSQLite::PropertySpec GetVersionSpec() { return BeSQLite::PropertySpec("binaryFormatVersion", "elementTileCache"); }
    static Utf8CP GetCurrentVersion();

    bool WriteCurrentVersion() const;
public:
    static RealityData::CachePtr Create(DgnDbCR db);
};

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     03/2018
//=======================================================================================
struct ThematicMeshBuilder : RefCountedBase
{
private:
    Utf8String          m_displacementChannel;
    Utf8String          m_paramChannel;

public:
                ThematicMeshBuilder(Utf8CP displacementChannel, Utf8CP paramChannel) : m_displacementChannel(displacementChannel), m_paramChannel(paramChannel) { }
    void        BuildMeshAuxData(Render::MeshAuxDataR auxData, PolyfaceQueryCR mesh, Render::Primitives::DisplayParamsCR displayParams);
    void        InitThematicDisplay(PolyfaceHeaderR mesh, Render::Primitives::DisplayParamsCR displayParams);
    

};

END_ELEMENT_TILETREE_NAMESPACE

