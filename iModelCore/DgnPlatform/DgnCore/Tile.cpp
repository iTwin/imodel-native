/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/Tile.h>
#include <DgnPlatform/TileIO.h>
#include <numeric>
#include <DgnPlatform/RangeIndex.h>
#include <GeomJsonWireFormat/JsonUtils.h>
#include <numeric>
#include <algorithm>
#include <set>
#include <map>
#include <inttypes.h>
#if defined (BENTLEYCONFIG_PARASOLID)
#include <BRepCore/PSolidUtil.h>
#endif

USING_NAMESPACE_TILE
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

// Obsolete versions of table storing tile data
#define TABLE_NAME_TileTree3 "TileTree3"
#define TABLE_NAME_TileTree4 "TileTree4"

// 5th version of this table - modified for iModelJs (imodel02 mercurial branch):
// Element tiles are now the only types of tiles produced and cached by the backend.
//  - Remove Metadata column added in TileTree4 iteration.
//  - Change binary format to store geometry in a form ready for submission to WebGL - tesselated polylines, vertex data in lookup tables, etc.
//  - Change magic number from 'dgnT' to 'iMdl' to reflect this change
#define TABLE_NAME_TileTree "TileTree5"

// Obsolete versions of table storing tile data creation time
#define TABLE_NAME_TileTreeCreateTime2 "TileTreeCreateTime2"
// Second version: Previous version retained "FileName" column which should have been renamed to "TileId"
#define TABLE_NAME_TileTreeCreateTime3 "TileTreeCreateTime3"

// Third version: The CreateTime field is now the time that the TILE was created - used solely for purging least-recently-used tiles. Updated when a tile is retrieved from the cache.
// Adds a Guid column corresponding to the GeometryGuid property of the model at the time of tile creation. When a tile is retrieved, if this value doesn't match the model's current
// Guid, it means some geometry has changed within the model - so we discard+regenerate the tile.
#define TABLE_NAME_TileTreeCreateTime "TileTreeCreateTime4"

#define COLUMN_TileTree_TileId TABLE_NAME_TileTree ".TileId"
#define COLUMN_TileTreeCreateTime_TileId TABLE_NAME_TileTreeCreateTime ".TileId"
#define JOIN_TileTreeTables TABLE_NAME_TileTree " JOIN " TABLE_NAME_TileTreeCreateTime " ON " COLUMN_TileTree_TileId "=" COLUMN_TileTreeCreateTime_TileId

BEGIN_UNNAMED_NAMESPACE

using FBox3d = RangeIndex::FBox;

constexpr double s_minRangeBoxSize = 2.5;     // Threshold below which we consider geometry/element too small to contribute to tile mesh ###TODO: Revisit...
constexpr double s_tileScreenSize = 512.0;
constexpr double s_minToleranceRatioMultiplier = 2.0;
constexpr double s_minToleranceRatio = s_tileScreenSize * s_minToleranceRatioMultiplier;
constexpr uint32_t s_minElementsPerTile = 100; // ###TODO: The complexity of a single element's geometry can vary wildly...
constexpr uint32_t s_hardMaxFeaturesPerTile = 2048*1024;
// unused - constexpr double s_maxLeafTolerance = 1.0; // the maximum tolerance at which we will stop subdividing tiles, regardless of # of elements contained or whether curved geometry exists.
static const Utf8String s_classifierIdPrefix("C:");
static const Utf8String s_animationIdPrefix("A:");
static const Utf8String s_omitEdgesPrefix("E:0");
static const Utf8String s_planarClassifierIdPrefix("CP:");


//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
struct CacheBlobHeader
{
    enum {DB_Signature06 = 0x0600};
    uint32_t m_signature;    // write this so we can detect errors on read
    uint32_t m_size;

    CacheBlobHeader(uint32_t size) {m_signature = DB_Signature06; m_size=size;}
    CacheBlobHeader(SnappyReader& in) {uint32_t actuallyRead; in._Read((Byte*) this, sizeof(*this), actuallyRead);}
};

DEFINE_POINTER_SUFFIX_TYPEDEFS(GeometryLoader);
DEFINE_POINTER_SUFFIX_TYPEDEFS(TileContext);
DEFINE_POINTER_SUFFIX_TYPEDEFS(MeshGenerator);
DEFINE_POINTER_SUFFIX_TYPEDEFS(ElementMeshGenerator);
DEFINE_POINTER_SUFFIX_TYPEDEFS(SharedMeshGenerator);
DEFINE_POINTER_SUFFIX_TYPEDEFS(InstanceableGeom);
DEFINE_POINTER_SUFFIX_TYPEDEFS(InstanceableGeomBucket);
DEFINE_REF_COUNTED_PTR(InstanceableGeom);
DEFINE_POINTER_SUFFIX_TYPEDEFS(SubRanges);

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     08/18
//=======================================================================================
struct VolumeClassificationLoader : Loader
{
    DEFINE_T_SUPER(Loader);
private:
    double m_expansion;
    DRange3d m_range;

    bool CompressMeshQuantization() const final { return true; }
    void  Preprocess(PolyfaceList& polyfaces, StrokesList& strokes) const final;
    virtual uint64_t GetNodeId(DgnElementId elementId) const final { return elementId.GetValue(); }

public:
    VolumeClassificationLoader(Tree& tree, ContentIdCR contentId, bool useCache);
};

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     08/18
//=======================================================================================
struct PlanarClassificationLoader : Loader
{
    DEFINE_T_SUPER(Loader);
private:
    double m_expansion;
    void  Preprocess(PolyfaceList& polyfaces, StrokesList& strokes) const final;

public:
    PlanarClassificationLoader(Tree& tree, ContentIdCR contentId, bool useCache);
};


//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
struct GeometryLoader
{
private:
    LoaderCR m_loader;
    DRange3d m_range;
    double m_tolerance;
    Render::Primitives::GeometryCollection m_geometry;
    Content::Metadata m_metadata;
public:
    explicit GeometryLoader(LoaderCR loader);

    TreeR GetTree() const { return m_loader.GetTree(); }
    DgnDbR GetDgnDb() const { return GetTree().GetDgnDb(); }
    bool Is3d() const { return GetTree().Is3d(); }
    bool Is2d() const { return GetTree().Is2d(); }
    LoaderCR GetLoader() const { return m_loader; }
    double GetTolerance() const { return m_tolerance; }
    Render::Primitives::GeometryCollectionCR GetGeometry() const { return m_geometry; }
    ContentIdCR GetContentId() const { return m_loader.GetContentId(); }
    double GetSizeMultiplier() const { return GetContentId().GetSizeMultiplier(); }
    Render::SystemR GetRenderSystem() const { return GetTree().GetRenderSystem(); }
    Content::MetadataCR GetMetadata() const { return m_metadata; }
    bool IsCanceled() const { return m_loader.IsCanceled(); }
    bool AllowInstancing() const { return m_loader.AllowInstancing(); }

    DRange3dCR GetTileRange() const { return m_range; }
    DRange3d GetDgnRange() const
        {
        DRange3d range;
        GetTree().GetLocation().Multiply(range, GetTileRange());
        return range;
        }

    bool GenerateGeometry();
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static DRange3d bisectRange2d(DRange3dCR range, bool takeUpper)
    {
    DVec3d diag = range.DiagonalVector();
    DRange3d subRange = range;

    double bisect;
    double* replace;
    if (diag.x > diag.y)
        {
        bisect = (range.low.x + range.high.x) / 2.0;
        replace = takeUpper ? &subRange.high.x : &subRange.low.x;
        }
    else
        {
        bisect = (range.low.y + range.high.y) / 2.0;
        replace = takeUpper ? &subRange.high.y : &subRange.low.y;
        }

    *replace = bisect;
    return subRange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
static DRange3d bisectRange(DRange3dCR range, bool takeUpper)
    {
    DVec3d diag = range.DiagonalVector();
    DRange3d subRange = range;

    double bisect;
    double* replace;
    if (diag.x > diag.y && diag.x > diag.z)
        {
        bisect = (range.low.x + range.high.x) / 2.0;
        replace = takeUpper ? &subRange.high.x : &subRange.low.x;
        }
    else if (diag.y > diag.z)
        {
        bisect = (range.low.y + range.high.y) / 2.0;
        replace = takeUpper ? &subRange.high.y : &subRange.low.y;
        }
    else
        {
        bisect = (range.low.z + range.high.z) / 2.0;
        replace = takeUpper ? &subRange.high.z : &subRange.low.z;
        }

    *replace = bisect;
    return subRange;
    }

//=======================================================================================
// Sub-divides a tile's volume in to 4 (2d) or 8 (3d) sub-volumes and tracks which sub-volumes
// are empty. This information is used by the front-end to trivially detect empty child
// tiles and avoid needlessly requesting their content.
// @bsistruct                                                   Paul.Connelly   03/19
//=======================================================================================
struct SubRanges
{
private:
    DRange3d    m_ranges[8];
    bool        m_empty[8];
    uint8_t     m_size;
public:
    SubRanges(DRange3dCR tileRange, bool is2d) : m_size(is2d ? 4 : 8)
        {
        // ###TODO this can be made more efficient
        auto bisect = is2d ? bisectRange2d : bisectRange;
        for (auto i = 0; i < 2; i++)
            {
            for (auto j = 0; j < 2; j++)
                {
                for (auto k = 0; k < (is2d ? 1 : 2); k++)
                    {
                    DRange3d range = tileRange;
                    range = bisect(range, 0 == i);
                    range = bisect(range, 0 == j);
                    if (!is2d)
                        range = bisect(range, 0 == k);

                    auto index = i + j*2 + k*4;
                    m_ranges[index] = range;
                    m_empty[index] = true;
                    }
                }
            }
        }

    void Add(DRange3dCR range)
        {
        for (uint8_t i = 0; i < m_size; i++)
            {
            if (m_empty[i] && m_ranges[i].IntersectsWith(range))
                m_empty[i] = false;
            }
        }

    // Compute bitfield with a 1 indicating an empty sub-range where each bit = i + j*2 + k*4
    // NB: This will fit in a single byte but for alignment and other reasons make it occupy 4 bytes in binary tile header.
    Content::SubRange ToBitfield() const
        {
        uint32_t flags = 0;
        for (uint8_t i = 0; i < m_size; i++)
            if (m_empty[i])
                flags |= (1 << i);

        return static_cast<Content::SubRange>(flags);
        }
};

//=======================================================================================
//! Populates a set of the largest N elements within a range, excluding any elements below
//! a specified size. Keeps track of whether any elements were skipped.
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
struct ElementCollector : RangeIndex::Traverser
{
    typedef bmultimap<double, DgnElementId, std::greater<double>> Entries;
private:
    Entries             m_entries;
    FBox3d              m_range;
    double              m_minRangeDiagonalSquared;
    uint32_t            m_maxElements;
    LoaderCR            m_loader;
    size_t              m_numSkipped = 0;
    SubRangesR          m_subRanges;
    Transform           m_transformFromDgn;
    bool                m_aborted = false;

    bool CheckStop() { return m_aborted || (m_aborted = m_loader.IsCanceled()); }

    void Insert(double diagonalSq, DgnElementId elemId)
        {
        m_entries.Insert(diagonalSq, elemId);
        if (m_entries.size() > m_maxElements)
            {
            m_entries.erase(--m_entries.end()); // remove the smallest element.
            ++m_numSkipped;
            }
        }

    Accept _CheckRangeTreeNode(FBox3d const& box, bool is3d) const override
        {
        if (!m_aborted && box.IntersectsWith(m_range))
            return Accept::Yes;
        else
            return Accept::No;
        }

    Stop _VisitRangeTreeEntry(RangeIndex::EntryCR entry) override
        {
        if (CheckStop())
            return Stop::Yes;
        else if (!entry.m_range.IntersectsWith(m_range))
            return Stop::No; // why do we need to check the range again here? _CheckRangeTreeNode() should have handled it, but doesn't...

        double sizeSq = entry.m_range.IsMinimal() ? 0.0 : entry.m_range.DistanceSquared();
        if (entry.m_range.IsMinimal())
            sizeSq = 0.0;

        if (0.0 == sizeSq || sizeSq >= m_minRangeDiagonalSquared)
            {
            Insert(sizeSq, entry.m_id);
            }
        else
            {
            // This element is too small to contribute geometry - but record which sub-range(s) it intersects.
            DRange3d tileRange = entry.m_range.ToRange3d();
            m_transformFromDgn.Multiply(tileRange, tileRange);
            m_subRanges.Add(tileRange);

            ++m_numSkipped;
            }

        return Stop::No;
        }
public:
    ElementCollector(DRange3dCR range, RangeIndex::Tree& rangeIndex, double minRangeDiagonalSquared, LoaderCR loader, uint32_t maxElements, TransformCR transformFromDgn, SubRangesR subRanges)
        : m_range(range, !rangeIndex.Is3d()), m_minRangeDiagonalSquared(minRangeDiagonalSquared), m_maxElements(maxElements), m_loader(loader), m_transformFromDgn(transformFromDgn), m_subRanges(subRanges)
        {
        // ###TODO: Do not traverse if tile is not expired (only deletions may have occurred)...
        rangeIndex.Traverse(*this);
        }

    bool AnySkipped() const { return 0 < m_numSkipped; }
    size_t GetNumSkipped() const { return m_numSkipped; }
    Entries const& GetEntries() const { return m_entries; }
    uint32_t GetMaxElements() const { return m_maxElements; }
};

/*---------------------------------------------------------------------------------**//**
* This exists because DRange3d::IntersectionOf() treats a zero-thickness intersection as
* null - so if the intersection in any dimension is zero, it nulls out the entire range.
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void clipContentRangeToTileRange(DRange3dR content, DRange3dCR tile)
    {
    content.low.x = std::max(content.low.x, tile.low.x);
    content.low.y = std::max(content.low.y, tile.low.y);
    content.low.z = std::max(content.low.z, tile.low.z);
    content.high.x = std::min(content.high.x, tile.high.x);
    content.high.y = std::min(content.high.y, tile.high.y);
    content.high.z = std::min(content.high.z, tile.high.z);

    if (content.low.x > content.high.x || content.low.y > content.high.y || content.low.z > content.high.z)
        content.Init();
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct RangeAccumulator : RangeIndex::Traverser
{
    DRange3dR       m_range;
    uint32_t        m_numElements = 0;
    bool            m_is2d;

    RangeAccumulator(DRange3dR range, bool is2d) : m_range(range), m_is2d(is2d) { m_range = DRange3d::NullRange(); }

    bool _AbortOnWriteRequest() const final { return true; }
    Accept _CheckRangeTreeNode(FBox3d const&, bool) const final { return Accept::Yes; }
    Stop _VisitRangeTreeEntry(RangeIndex::EntryCR entry) final
        {
        ++m_numElements;
        m_range.Extend(entry.m_range.ToRange3d());
        return Stop::No;
        }

    bool Accumulate(RangeIndex::Tree& tree)
        {
        if (Stop::Yes == tree.Traverse(*this))
            return false;
        else
            return !m_range.IsNull();
        }

    uint32_t GetElementCount() const { return m_numElements; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
static DRange3d scaleSpatialRange(DRange3dCR range)
    {
    // Geometry often lies in a plane precisely coincident with the planes of the project extents.
    // We must expand the extents slightly to prevent floating point fuzz in range intersection tests
    // against such geometry - otherwise portions may be inappropriately culled.
    // Similarly (TFS#863543) some 3d data sets consist of essentially 2d data sitting smack in the center of the
    // project extents. If we subdivide tiles in half, we face similar issues where the geometry ends up precisely
    // aligned to tile boundaries. Bias the scale to prevent this.
    // NOTE: There's no simple way to detect and deal with arbitrarily located geometry just-so-happening to
    // align with some tile's boundary...
    constexpr double loScale = 1.0001,
                     hiScale = 1.0002,
                     fLo = 0.5 * (1.0 + loScale),
                     fHi = 0.5 * (1.0 + hiScale);

    DRange3d result = range;
    if (!result.IsNull())
        {
        result.high.Interpolate(range.low, fHi, range.high);
        result.low.Interpolate(range.high, fLo, range.low);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool isElementCountLessThan(uint32_t threshold, RangeIndex::Tree& tree, uint32_t* pCount)
    {
    struct Counter : RangeIndex::Traverser
    {
        uint32_t    m_count = 0;
        uint32_t    m_threshold;

        explicit Counter(uint32_t threshold) : m_threshold(threshold) { }

        Accept _CheckRangeTreeNode(FBox3d const&, bool) const final { return m_count < m_threshold ? Accept::Yes : Accept::No; }
        Stop _VisitRangeTreeEntry(RangeIndex::EntryCR entry) final
            {
            ++m_count;
            return m_count < m_threshold ? Stop::No : Stop::Yes;
            }
    };

    Counter counter(threshold);
    tree.Traverse(counter);
    if (nullptr != pCount)
        *pCount = counter.m_count;

    return counter.m_count < threshold;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileGeometrySource
{
    struct GeomBlob
    {
        void const* m_blob;
        int         m_size;

        GeomBlob(void const* blob, int size) : m_blob(blob), m_size(size) { }
        template<typename T> GeomBlob(T& stmt, int columnIndex)
            {
            m_blob = stmt.GetValueBlob(columnIndex);
            m_size = stmt.GetColumnBytes(columnIndex);
            }
    };
protected:
    DgnCategoryId           m_categoryId;
    GeometryStream          m_geom;
    DgnDbR                  m_db;
    bool                    m_isGeometryValid;

    TileGeometrySource(DgnCategoryId categoryId, DgnDbR db, GeomBlob const& geomBlob) : m_categoryId(categoryId), m_db(db)
        {
        m_isGeometryValid = DgnDbStatus::Success == db.Elements().LoadGeometryStream(m_geom, geomBlob.m_blob, geomBlob.m_size);
        }
public:
    bool IsGeometryValid() const { return m_isGeometryValid; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileGeometrySource3d : TileGeometrySource, GeometrySource3d
{
private:
    Placement3d     m_placement;

    TileGeometrySource3d(DgnCategoryId categoryId, DgnDbR db, GeomBlob const& geomBlob, Placement3dCR placement)
        : TileGeometrySource(categoryId, db, geomBlob), m_placement(placement) { }

    DgnDbR _GetSourceDgnDb() const override { return m_db; }
    DgnElementCP _ToElement() const override { return nullptr; }
    GeometrySource3dCP _GetAsGeometrySource3d() const override { return this; }
    DgnCategoryId _GetCategoryId() const override { return m_categoryId; }
    GeometryStreamCR _GetGeometryStream() const override { return m_geom; }
    Placement3dCR _GetPlacement() const override { return m_placement; }

    DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
    DgnDbStatus _SetPlacement(Placement3dCR) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
public:
    static std::unique_ptr<GeometrySource> Create(DgnCategoryId categoryId, DgnDbR db, GeomBlob const& geomBlob, Placement3dCR placement)
        {
        std::unique_ptr<GeometrySource> pSrc(new TileGeometrySource3d(categoryId, db, geomBlob, placement));
        if (!static_cast<TileGeometrySource3d const&>(*pSrc).IsGeometryValid())
            return nullptr;

        return pSrc;
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct TileGeometrySource2d : TileGeometrySource, GeometrySource2d
{
private:
    Placement2d     m_placement;

    TileGeometrySource2d(DgnCategoryId categoryId, DgnDbR db, GeomBlob const& geomBlob, Placement2dCR placement)
        : TileGeometrySource(categoryId, db, geomBlob), m_placement(placement) { }

    DgnDbR _GetSourceDgnDb() const override { return m_db; }
    DgnElementCP _ToElement() const override { return nullptr; }
    GeometrySource2dCP _GetAsGeometrySource2d() const override { return this; }
    DgnCategoryId _GetCategoryId() const override { return m_categoryId; }
    GeometryStreamCR _GetGeometryStream() const override { return m_geom; }
    Placement2dCR _GetPlacement() const override { return m_placement; }

    DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
    DgnDbStatus _SetPlacement(Placement2dCR) override { BeAssert(false && "No reason to access this"); return DgnDbStatus::BadRequest; }
public:
    static std::unique_ptr<GeometrySource> Create(DgnCategoryId categoryId, DgnDbR db, GeomBlob const& geomBlob, Placement2dCR placement)
        {
        std::unique_ptr<GeometrySource> pSrc(new TileGeometrySource2d(categoryId, db, geomBlob, placement));
        if (!static_cast<TileGeometrySource2d const&>(*pSrc).IsGeometryValid())
            return nullptr;

        return pSrc;
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct GeometrySelector3d
{
    static bool Is3d() { return true; }

    static Utf8CP GetSql()
        {
        return "SELECT CategoryId,GeometryStream,Yaw,Pitch,Roll,Origin_X,Origin_Y,Origin_Z,BBoxLow_X,BBoxLow_Y,BBoxLow_Z,BBoxHigh_X,BBoxHigh_Y,BBoxHigh_Z FROM "
                BIS_TABLE(BIS_CLASS_GeometricElement3d) " WHERE ElementId=?";
        }

    static std::unique_ptr<GeometrySource> ExtractGeometrySource(BeSQLite::CachedStatement& stmt, DgnDbR db)
        {
        auto categoryId = stmt.GetValueId<DgnCategoryId>(0);
        TileGeometrySource::GeomBlob geomBlob(stmt, 1);

        DPoint3d origin = DPoint3d::From(stmt.GetValueDouble(5), stmt.GetValueDouble(6), stmt.GetValueDouble(7)),
                 boxLo  = DPoint3d::From(stmt.GetValueDouble(8), stmt.GetValueDouble(9), stmt.GetValueDouble(10)),
                 boxHi  = DPoint3d::From(stmt.GetValueDouble(11), stmt.GetValueDouble(12), stmt.GetValueDouble(13));

        Placement3d placement(origin,
                YawPitchRollAngles(Angle::FromDegrees(stmt.GetValueDouble(2)), Angle::FromDegrees(stmt.GetValueDouble(3)), Angle::FromDegrees(stmt.GetValueDouble(4))),
                ElementAlignedBox3d(boxLo.x, boxLo.y, boxLo.z, boxHi.x, boxHi.y, boxHi.z));

        return TileGeometrySource3d::Create(categoryId, db, geomBlob, placement);
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/16
//=======================================================================================
struct GeometrySelector2d
{
    static bool Is3d() { return false; }

    static Utf8CP GetSql()
        {
        return "SELECT CategoryId,GeometryStream,Rotation,Origin_X,Origin_Y,BBoxLow_X,BBoxLow_Y,BBoxHigh_X,BBoxHigh_Y FROM "
                BIS_TABLE(BIS_CLASS_GeometricElement2d) " WHERE ElementId=?";
        }

    static std::unique_ptr<GeometrySource> ExtractGeometrySource(BeSQLite::CachedStatement& stmt, DgnDbR db)
        {
        auto categoryId = stmt.GetValueId<DgnCategoryId>(0);
        TileGeometrySource::GeomBlob geomBlob(stmt, 1);

        auto rotation = AngleInDegrees::FromDegrees(stmt.GetValueDouble(2));
        DPoint2d origin = DPoint2d::From(stmt.GetValueDouble(3), stmt.GetValueDouble(4));
        ElementAlignedBox2d bbox(stmt.GetValueDouble(5), stmt.GetValueDouble(6), stmt.GetValueDouble(7), stmt.GetValueDouble(8));

        Placement2d placement(origin, rotation, bbox);
        return TileGeometrySource2d::Create(categoryId, db, geomBlob, placement);
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
struct TileBuilder : GeometryListBuilder
{
protected:
    TileContext&                m_context;
    double                      m_rangeDiagonalSquared;

    static void AddNormals(PolyfaceHeaderR, IFacetOptionsR);
    static void AddParams(PolyfaceHeaderR, IFacetOptionsR);

    void _AddPolyface(PolyfaceQueryCR, bool) override;
    void _AddPolyfaceR(PolyfaceHeaderR, bool) override;
    void _AddSubGraphic(GraphicR, TransformCR, GraphicParamsCR, ClipVectorCP) override;
    bool _WantStrokeLineStyle(LineStyleSymbCR, IFacetOptionsPtr&) override;
    void _AddSolidPrimitiveR(ISolidPrimitiveR) override;

    GraphicBuilderPtr _CreateSubGraphic(TransformCR, ClipVectorCP) const override;
    GraphicPtr _FinishGraphic(GeometryAccumulatorR) override;
    void _ActivateGraphicParams(GraphicParamsCR gfParams, GeometryParamsCP geomParams) override
        {
        BeAssert(nullptr == geomParams || geomParams->GetSubCategoryId().IsValid());
        GeometryListBuilder::_ActivateGraphicParams(gfParams, geomParams);
        }

    TileBuilder(TileContext& context, DRange3dCR range);

    void ReInitialize(DRange3dCR range, TransformCR geomToOrigin);
public:
    TileBuilder(TileContext& context, DgnElementId elemId, double rangeDiagonalSquared, CreateParams const& params);

    void ReInitialize(DgnElementId elemId, double rangeDiagonalSquared, TransformCR localToWorld);
    double GetRangeDiagonalSquared() const { return m_rangeDiagonalSquared; }
};

DEFINE_POINTER_SUFFIX_TYPEDEFS(TileBuilder);
DEFINE_REF_COUNTED_PTR(TileBuilder);

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   05/17
//=======================================================================================
struct TileSubGraphic : TileBuilder
{
private:
    DgnGeometryPartCPtr m_input;
    PartGeom::Key       m_key;
    PartGeomPtr         m_output;
    bool                m_hasSymbologyChanges;

    GraphicPtr _FinishGraphic(GeometryAccumulatorR) override;
    void _Reset() override { m_input = nullptr; m_output = nullptr; }

    // NB: Cannot instance solid primitives inside geometry parts...
    void _AddSolidPrimitiveR(ISolidPrimitiveR primitive) override
        {
        GeometryListBuilder::_AddSolidPrimitiveR(primitive);
        }
public:
    TileSubGraphic(TileContext& context, DgnGeometryPartCP part = nullptr, bool hasSymbologyChanges = false, PartGeom::KeyCR key = PartGeom::Key());
    TileSubGraphic(TileContext& context, DgnGeometryPartCR part, bool hasSymbologyChanges, PartGeom::KeyCR key) : TileSubGraphic(context, &part, hasSymbologyChanges, key) { }

    void ReInitialize(DgnGeometryPartCR part, bool hasSymbologyChanges, PartGeom::KeyCR key);

    DgnGeometryPartCR GetInput() const { BeAssert(m_input.IsValid()); return *m_input; }
    PartGeomPtr GetOutput() const { return m_output; }
    bool HasSymbologyChanges() const { return m_hasSymbologyChanges; }
    PartGeom::KeyCR GetPartKey() const { return m_key; }
    void SetOutput(PartGeomR output) { BeAssert(m_output.IsNull()); m_output = &output; }
};

DEFINE_POINTER_SUFFIX_TYPEDEFS(TileSubGraphic);
DEFINE_REF_COUNTED_PTR(TileSubGraphic);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wantGlyphBoxes(double sizeInPixels)
    {
    constexpr double s_glyphBoxSizeThreshold = 3.0;
    return sizeInPixels <= s_glyphBoxSizeThreshold;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool setupForStyledCurveVector(GraphicBuilderR builder, CurveVectorCR curve)
    {
    BeAssert(nullptr != dynamic_cast<TileBuilderP>(&builder));
    auto& tileBuilder = static_cast<TileBuilderR>(builder);
    bool wasCurved = tileBuilder.IsAddingCurved();
    tileBuilder.SetAddingCurved(curve.ContainsNonLinearPrimitive());
    return wasCurved;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
struct TileContext : NullContext
{
    using PartGeoms = std::set<PartGeomPtr, PartGeom::PtrComparator>;
    using SolidPrimitives = std::multiset<SolidPrimitiveGeomPtr, SolidPrimitiveGeom::PtrComparator>;
private:

    IFacetOptionsR                  m_facetOptions;
    GeometryLoaderCR                m_loader;
    GeometryList&                   m_geometries;
    DRange3d                        m_range;
    DRange3d                        m_tileRange;
    BeSQLite::CachedStatementPtr    m_statement;
    Transform                       m_transformFromDgn;
    double                          m_minRangeDiagonalSquared;
    double                          m_minTextBoxSize;
    double                          m_tolerance;
    GraphicPtr                      m_finishedGraphic;
    TileBuilderPtr                  m_tileBuilder;
    TileSubGraphicPtr               m_subGraphic;
    DgnElementId                    m_curElemId;
    double                          m_curRangeDiagonalSquared;
    PartGeoms                       m_geomParts;
    SolidPrimitives                 m_solidPrimitives;
    SubRangesR                      m_subRanges;
protected:
    void PushGeometry(GeometryR geom);

    TileContext(GeometryList& geoms, GeometryLoaderCR loader, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, double tileTolerance, double rangeTolerance, SubRangesR subRanges);

    Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override
        {
        Render::GraphicBuilderPtr builder(m_tileBuilder);
        if (builder->GetRefCount() > 2)
            builder = new TileBuilder(*this, m_curElemId, m_curRangeDiagonalSquared, params);
        else
            m_tileBuilder->ReInitialize(m_curElemId, m_curRangeDiagonalSquared, params.GetPlacement());

        BeAssert(builder->GetRefCount() <= 2);
        return builder;
        }

    bool IsValueNull(int index) { return m_statement->IsColumnNull(index); }
    StatusInt _VisitElement(DgnElementId elementId, bool allowLoad) override;
    Render::GraphicPtr _StrokeGeometry(GeometrySourceCR, double) override;
    void _AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams) override;
    bool _CheckStop() override { return WasAborted() || AddAbortTest(m_loader.IsCanceled()); }
    bool _WantUndisplayed() override { return true; }
    AreaPatternTolerance _GetAreaPatternTolerance(CurveVectorCR) override { return AreaPatternTolerance(m_tolerance); }
    Render::SystemP _GetRenderSystem() const override { return &m_loader.GetRenderSystem(); }
    double _GetPixelSizeAtPoint(DPoint3dCP) const override { return m_tolerance; }
    bool _WantGlyphBoxes(double sizeInPixels) const override { return wantGlyphBoxes(sizeInPixels); }
    double _DepthFromDisplayPriority(int32_t priority) const override { return GeometryListBuilder::DepthFromDisplayPriority(priority); }
    void _DrawStyledCurveVector(GraphicBuilderR builder, CurveVectorCR curve, GeometryParamsR params, bool doCook) override
        {
        bool wasCurved = setupForStyledCurveVector(builder, curve);
        NullContext::_DrawStyledCurveVector(builder, curve, params, doCook);
        static_cast<TileBuilderR>(builder).SetAddingCurved(wasCurved);
        }
    bool _AnyPointVisible(DPoint3dCP pts, int nPts, double tolerance) override
        {
        DRange3d range = DRange3d::From(pts, nPts);
        return range.IntersectsWith(m_range);
        }
public:
    TileContext(GeometryList& geometries, GeometryLoaderCR loader, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, double tolerance, SubRangesR subRanges)
        : TileContext(geometries, loader, range, facetOptions, transformFromDgn, tolerance, tolerance, subRanges) { }

    static Render::ViewFlags GetDefaultViewFlags();

    void ProcessElement(DgnElementId elementId, double diagonalRangeSquared);

    // Returns false to indicate solid primitive should be added as normal (uninstanced) geometry.
    bool AddSolidPrimitiveGeom(ISolidPrimitiveR primitive, TransformCR localToWorld, DisplayParamsCR displayParams);
    void AddPartGeom(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams, GraphicParamsR graphicParams);
    PartGeomPtr GeneratePartGeom(PartGeom::KeyCR, DgnGeometryPartCR, GeometryParamsR);

    TreeR GetTree() const { return m_loader.GetTree(); }
    Render::System& GetRenderSystemR() const { return m_loader.GetRenderSystem(); }
    bool Is3d() const { return m_loader.Is3d(); }

    double GetMinRangeDiagonalSquared() const { return m_minRangeDiagonalSquared; }
    double GetTolerance() const { return m_tolerance; }
    bool BelowMinRange(DRange3dCR range) const
        {
        // Avoid processing any bits of geometry with range smaller than roughly half a pixel...
        auto diag = Is3d() ? range.low.DistanceSquared(range.high) : range.low.DistanceSquaredXY(range.high);
        return diag < m_minRangeDiagonalSquared && 0.0 < diag; // ###TODO_ELEMENT_TILE: Dumb single-point primitives...
        }

    size_t GetGeometryCount() const { return m_geometries.size(); }
    void TruncateGeometryList(size_t maxSize) { m_geometries.resize(maxSize); m_geometries.MarkIncomplete(); }

    DgnElementId GetCurrentElementId() const { return m_curElemId; }
    TransformCR GetTransformFromDgn() const { return m_transformFromDgn; }
    IFacetOptionsR GetFacetOptions() { return m_facetOptions; }
    GeometryLoaderCR GetLoader() const { return m_loader; }
    bool AllowInstancing() const { return GetLoader().AllowInstancing(); }

    GraphicPtr FinishGraphic(GeometryAccumulatorR accum, TileBuilder& builder);
    GraphicPtr FinishSubGraphic(GeometryAccumulatorR accum, TileSubGraphic& subGf);

    void MarkIncomplete() { m_geometries.MarkIncomplete(); }

    PartGeoms const& Parts() const { return m_geomParts; }
    SolidPrimitives const& GetSolidPrimitives() const { return m_solidPrimitives; }

    void AddSharedGeometry(GeometryList& geometryList, ElementMeshGeneratorR meshGenerator, SubRanges& subRanges) const;
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     12/2017
+===============+===============+===============+===============+===============+======*/
struct TileRangeClipOutput : PolyfaceQuery::IClipToPlaneSetOutput
{
    bvector<PolyfaceHeaderPtr>  m_clipped;
    bvector<PolyfaceQueryCP>    m_output;

    StatusInt _ProcessUnclippedPolyface(PolyfaceQueryCR mesh) override { m_output.push_back(&mesh); ; return SUCCESS; }
    StatusInt _ProcessClippedPolyface(PolyfaceHeaderR mesh) override { m_output.push_back(&mesh); m_clipped.push_back(&mesh); return SUCCESS; }
};

struct GlyphAtlas;
struct DeferredGlyph
{
    Polyface m_polyface;
    DgnElementId m_elemId;
    double m_rangePixels;
    bool m_isContained;

    DeferredGlyph(Polyface& polyface, DgnElementId elemId, double rangePixels, bool isContained) : m_polyface(polyface), m_elemId(elemId), m_rangePixels(rangePixels), m_isContained(isContained) {}
    void* GetKey();
};

typedef bvector<DeferredGlyph> DeferredGlyphList;

struct GlyphLocation
{
    uint32_t m_atlasNdx; // which atlas within the manager contains this glyph?
    uint32_t m_atlasSlotX; // what slot within that atlas has the glyph?
    uint32_t m_atlasSlotY;
};

struct GlyphAtlas
{
private:
    static const uint32_t m_glyphSizeInPixels = 48;
    static const uint32_t m_glyphPaddingInPixels = 16;
    static const uint32_t m_glyphHalfPaddingInPixels = m_glyphPaddingInPixels / 2;
    static const uint32_t m_glyphTotalSizeInPixels = m_glyphSizeInPixels + m_glyphPaddingInPixels;
    static const uint32_t m_maxAtlasGlyphsDim = 64; // 64 glyphs * 64 pixels = 4096 pixels

    uint32_t m_index;
    uint32_t m_numGlyphs;
    uint32_t m_numGlyphsInX;
    uint32_t m_numGlyphsInY;
    uint32_t m_numPixelsInX;
    uint32_t m_numPixelsInY;
    uint32_t m_availGlyphSlotX = 0;
    uint32_t m_availGlyphSlotY = 0;

    ByteStream m_atlasBytes;
    TexturePtr m_atlasTexture;

    static bool IsPowerOfTwo(uint32_t num) { return 0 == (num & (num - 1)); }
    static uint32_t NextHighestPowerOfTwo(uint32_t num)
        {
        --num;
        for (int i = 1; i < 32; i <<= 1)
            num = num | num >> i;
        return num + 1;
        }

    void CalculateSize();
public:
    GlyphAtlas(uint32_t index, uint32_t numGlyphs);
    GlyphAtlas(GlyphAtlas&&);
    GlyphAtlas& operator=(GlyphAtlas&& other);

    bool AddGlyph(DeferredGlyph& glyph, GlyphLocation& loc);
    void GetUVCoords(Render::Image* image, const GlyphLocation& loc, DPoint2d uvs[2]);
    TexturePtr GetTexture(Render::System& renderSystem, DgnDbP db);

    static uint32_t GetMaxGlyphsInAtlas() { return m_maxAtlasGlyphsDim * m_maxAtlasGlyphsDim; }
};

struct GlyphAtlasManager
{
private:
    bvector<GlyphAtlas> m_atlases;
    uint32_t m_numGlyphs;
    uint32_t m_numAtlases;
    uint32_t m_curAtlasNdx;
    std::map<void*, GlyphLocation> m_glyphLocations;
public:
    GlyphAtlasManager(uint32_t numGlyphs);

    void AddGlyph(DeferredGlyph& glyph);
    GlyphAtlas& GetAtlasAndLocationForGlyph(DeferredGlyph& glyph, GlyphLocation &loc);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/19
+---------------+---------------+---------------+---------------+---------------+------*/
static PolyfaceHeaderPtr tryDecimate(Polyface& tilePolyface)
    {
    static constexpr size_t minPointCount = 100; // Only decimate meshes with at least this many points.
    PolyfaceHeaderPtr pf = tilePolyface.m_polyface;
    BeAssert(pf.IsValid() && 0 < pf->GetPointIndexCount()); // callers check this
    if (!tilePolyface.CanDecimate() || pf->GetPointCount() <= minPointCount)
        return nullptr;

    static constexpr double minRatio = 0.25; // Decimation must reduce point count by at least this percentage.
    BeAssert(0 == pf->GetEdgeChainCount()); // The decimation does not handle edge chains - but this only occurs for polyfaces which should never have them.
    return pf->ClusteredVertexDecimate(tilePolyface.m_decimationTolerance, minRatio);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/19
+---------------+---------------+---------------+---------------+---------------+------*/
static bool tryDecimate(StrokesR out, StrokesCR in)
    {
    if (!in.CanDecimate())
        return false;

    auto inPointCount = in.ComputePointCount();
    static constexpr size_t minPointCount = 10;
    if (inPointCount <= minPointCount)
        return false;

    bvector<DPoint3d> compressedPoints;
    for (auto const& strokeIn : in.m_strokes)
        {
        compressedPoints.clear();
        if (!strokeIn.m_canDecimate)
            compressedPoints = strokeIn.m_points;
        else
            DPoint3dOps::CompressByChordError(compressedPoints, strokeIn.m_points, in.m_decimationTolerance);

        out.m_strokes.emplace_back(Strokes::PointList(std::move(compressedPoints), false));
        }

    auto outPointCount = out.ComputePointCount();
    auto decimationRatio = static_cast<double>(outPointCount) / static_cast<double>(inPointCount);

    // Prevent further attempts to decimate the output
    out.m_decimationTolerance = 0.0;

    // If at least this percentage of points remain after decimation, consider it not worthwhile
    static constexpr double maxRatio = 0.80;
    return decimationRatio <= maxRatio;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct MeshGenerator : ViewContext
{
protected:
    GeometryLoaderCR    m_loader;
    GeometryOptions     m_options;
    MeshBuilderSet      m_builders;
    DRange3d            m_contentRange = DRange3d::NullRange();
    bool                m_didDecimate = false;

    static double DecimateStrokes(StrokesR strokesOut, StrokesR strokesIn);

    MeshBuilderR GetMeshBuilder(MeshBuilderKey const& key, uint32_t numVertices);

    void AddPolyfaces(PolyfaceList& polyfaces, DgnElementId, double rangePixels, bool isContained);
    void AddClippedPolyface(PolyfaceQueryCR, DgnElementId, DisplayParamsCR, MeshEdgeCreationOptions, bool isPlanar);

    void AddStrokes(StrokesList& strokes, DgnElementId, double rangePixels, bool isContained);

    SystemP _GetRenderSystem() const final { return &m_loader.GetRenderSystem(); }
    GraphicBuilderPtr _CreateGraphic(GraphicBuilder::CreateParams const&) final { BeAssert(false); return nullptr; }
    GraphicPtr _CreateBranch(GraphicBranch&, DgnDbR, TransformCR, ClipVectorCP) final { BeAssert(false); return nullptr; }
    double _GetPixelSizeAtPoint(DPoint3dCP) const final { return GetTolerance(); }
    bool _WantGlyphBoxes(double sizeInPixels) const final { return wantGlyphBoxes(sizeInPixels); }
    double _DepthFromDisplayPriority(int32_t priority) const override { return GeometryListBuilder::DepthFromDisplayPriority(priority); }
    void _DrawStyledCurveVector(GraphicBuilderR builder, CurveVectorCR curve, GeometryParamsR params, bool doCook) final
        {
        bool wasCurved = setupForStyledCurveVector(builder, curve);
        ViewContext::_DrawStyledCurveVector(builder, curve, params, doCook);
        static_cast<TileBuilderR>(builder).SetAddingCurved(wasCurved);
        }
    bool _AnyPointVisible(DPoint3dCP worldPoints, int nPts, double tolerance) final
        {
        DRange3d        pointRange = DRange3d::From(worldPoints, nPts);
        return pointRange.IntersectsWith(m_loader.GetTileRange());
        }

    bool IsCanceled() const { return m_loader.IsCanceled(); }

    MeshGenerator(GeometryLoaderCR loader, double tolerance, GeometryOptionsCR options, DRange3dCR range);
public:
    bool DidDecimation() const { return m_didDecimate; }
    void MarkDecimated() { m_didDecimate = true; }
    bool GetOmitEdges() const { return m_loader.GetTree().GetOmitEdges(); }

    // Add meshes to the MeshBuilder map
    void AddMeshes(GeometryList const& geometries, bool doRangeTest);
    void AddMeshes(GeometryR geom, bool doRangeTest);

    // Return a list of all meshes currently in the builder map
    DRange3dCR GetTileRange() const { return m_builders.GetRange(); }
    GeometryLoaderCR GetLoader() const { return m_loader; }
    GeometryOptionsCR GetOptions() const { return m_options; }

    virtual FeatureTableR GetFeatureTable() = 0;
    virtual double GetTolerance() const = 0;
    virtual void AddDeferredPolyface(Polyface& tilePolyface, DgnElementId, double rangePixels, bool isContained) = 0;
    virtual void ClipAndAddPolyface(PolyfaceHeaderR polyface, DgnElementId elemId, DisplayParamsCR displayParams, MeshEdgeCreationOptions const& edgeOptions, bool isPlanar) = 0;
    virtual void ClipStrokes(StrokesR) const = 0;

    void ExtendContentRange(bvector<DPoint3d> const& points) { m_contentRange.Extend(points); }
    void ExtendContentRange(DRange3dR range) { m_contentRange.Extend(range); }

    MeshBuilderListPtr FindMeshBuilderList(MeshBuilderKey const& key) const
        {
        return m_builders.FindList(key);
        }

    void AddPolyface(Polyface& polyface, DgnElementId, double rangePixels, bool isContained, bool doDefer=true);
    void AddStrokes(StrokesR strokes, DgnElementId, double rangePixels, bool isContained);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/19
//=======================================================================================
struct ElementMeshGenerator : MeshGenerator
{
private:
    FeatureTable        m_featureTable;
    DeferredGlyphList   m_deferredGlyphs;
    std::set<void*>     m_uniqueGlyphKeys;
    bvector<MeshPtr>    m_sharedMeshes;

    Strokes ClipSegments(StrokesCR strokes) const;
    void ClipPoints(StrokesR strokes) const;
public:
    ElementMeshGenerator(GeometryLoaderCR loader, GeometryOptionsCR options) : MeshGenerator(loader, loader.GetTolerance(), options, loader.GetTileRange()),
        m_featureTable(loader.GetTree().GetModelId(), loader.GetRenderSystem()._GetMaxFeaturesPerBatch())
        {
        m_builders.SetFeatureTable(m_featureTable);
        }

    FeatureTableR GetFeatureTable() final { return m_featureTable; }
    double GetTolerance() const final { return m_loader.GetTolerance(); }
    DRange3dCR GetContentRange() const { return m_contentRange; }
    void AddDeferredPolyface(Polyface& tilePolyface, DgnElementId, double rangePixels, bool isContained) final;
    void ClipAndAddPolyface(PolyfaceHeaderR polyface, DgnElementId elemId, DisplayParamsCR displayParams, MeshEdgeCreationOptions const& edgeOptions, bool isPlanar) final;
    void ClipStrokes(StrokesR strokes) const final;

    MeshList GetMeshes();
    void AddDeferredGlyphMeshes(Render::System& renderSystem);
    void AddSharedMesh(MeshR mesh) { m_sharedMeshes.push_back(&mesh); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/19
//=======================================================================================
struct SharedMeshGenerator : MeshGenerator
{
private:
    ElementMeshGeneratorR   m_gen;
    SharedGeomR             m_geom;
public:
    SharedMeshGenerator(ElementMeshGeneratorR gen, SharedGeomR geom) : MeshGenerator(gen.GetLoader(), geom.GetTolerance(gen.GetTolerance()), gen.GetOptions(), geom.GetRange()),
        m_gen(gen), m_geom(geom)
        {
        //
        }

    FeatureTableR GetFeatureTable() final { return m_gen.GetFeatureTable(); }
    double GetTolerance() const final { return m_geom.GetTolerance(m_gen.GetTolerance()); }
    void AddDeferredPolyface(Polyface&, DgnElementId, double, bool) final { BeAssert(false); }
    void ClipAndAddPolyface(PolyfaceHeaderR, DgnElementId, DisplayParamsCR, MeshEdgeCreationOptions const&, bool) final { BeAssert(false); }
    void ClipStrokes(StrokesR) const final { }

    void AddMeshes(bvector<MeshPtr>& meshes);
    DRange3d ComputeContentRange(SubRangesR) const;
};

//=======================================================================================
//! A single instanceable Polyface or Strokes.
// @bsistruct                                                   Paul.Connelly   04/19
//=======================================================================================
struct InstanceableGeom : RefCountedBase
{
private:
    SharedGeomCPtr  m_sharedGeom;
    bool            m_decimated;
protected:
    InstanceableGeom(SharedGeomCR shared, bool decimated) : m_sharedGeom(&shared), m_decimated(decimated) { }

    virtual void AddInstanced(MeshGeneratorR, MeshBuilderR, DRange3dR contentRange) const = 0;
    virtual void AddBatched(ElementMeshGeneratorR meshGen, TransformCR transform, DisplayParamsCR displayParams, DgnElementId elemId) = 0;
public:
    size_t GetVertexCount() const { return GetSharedGeom().GetInstanceCount() * GetSharedVertexCount(); }
    SharedGeomCR GetSharedGeom() const { return *m_sharedGeom; }
    bool IsDecimated() const { return m_decimated; }

    virtual size_t GetSharedVertexCount() const = 0;
    virtual DisplayParamsCR GetDisplayParams() const = 0;
    virtual MeshBuilderKey GetMeshBuilderKey() const = 0;
    virtual DRange3d ComputeRange() const = 0;

    bool operator<(InstanceableGeomCR rhs) const { return GetVertexCount() < rhs.GetVertexCount(); }
    static bool ComparePtrs(InstanceableGeomPtr const& lhs, InstanceableGeomPtr const& rhs) { return *lhs < *rhs; }

    void AddInstanced(ElementMeshGeneratorR meshGen, SubRangesR subRanges);
    void AddBatched(ElementMeshGeneratorR meshGen, SubRangesR subRanges);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/19
//=======================================================================================
struct InstanceablePolyface : InstanceableGeom
{
private:
    Polyface    m_polyface;

    InstanceablePolyface(PolyfaceCR polyface, SharedGeomCR shared, bool decimated) : InstanceableGeom(shared, decimated), m_polyface(polyface) { }

    void AddInstanced(MeshGeneratorR, MeshBuilderR, DRange3dR contentRange) const final;
    void AddBatched(ElementMeshGeneratorR meshGen, TransformCR transform, DisplayParamsCR displayParams, DgnElementId elemId) final;
public:
    static InstanceableGeomPtr Create(PolyfaceR polyface, SharedGeomCR shared)
        {
        PolyfaceHeaderPtr decimated = tryDecimate(polyface);

        // Prevent trying to re-decimate
        // (Note even if tryDecimate() returned null we may subsequently try and fail again if decimation tolerance is non-zero.
        polyface.m_decimationTolerance = 0.0;
        if (decimated.IsValid())
            polyface.m_polyface = decimated;

        return new InstanceablePolyface(polyface, shared, decimated.IsValid());
        }

    size_t GetSharedVertexCount() const final { return m_polyface.m_polyface->GetPointCount(); }

    // NB: Face-attached materials produce multiple polyfaces with different materials...
    DisplayParamsCR GetDisplayParams() const final { return *m_polyface.m_displayParams; }
    DRange3d ComputeRange() const final { return m_polyface.m_polyface->PointRange(); }

    MeshBuilderKey GetMeshBuilderKey() const final
        {
        return MeshBuilderKey(GetDisplayParams(), nullptr != m_polyface.m_polyface->GetNormalIndexCP(), Mesh::PrimitiveType::Mesh, m_polyface.m_isPlanar);
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/19
//=======================================================================================
struct InstanceableStrokes : InstanceableGeom
{
private:
    Strokes m_strokes;
    size_t  m_numVertices = 0;

    InstanceableStrokes(Strokes&& strokes, SharedGeomCR shared, bool decimated) : InstanceableGeom(shared, decimated), m_strokes(std::move(strokes))
        {
        for (auto const& pointList : m_strokes.m_strokes)
            m_numVertices += pointList.m_points.size();
        }

    void AddInstanced(MeshGeneratorR, MeshBuilderR, DRange3dR contentRange) const final;
    void AddBatched(ElementMeshGeneratorR meshGen, TransformCR transform, DisplayParamsCR displayParams, DgnElementId elemId) final;
public:
    static InstanceableGeomPtr Create(Strokes&& strokes, SharedGeomCR shared)
        {
        bool didDecimate = false;
        if (strokes.CanDecimate())
            {
            Strokes decimated(*strokes.m_displayParams, strokes.m_disjoint, strokes.m_isPlanar, strokes.m_decimationTolerance);
            if (tryDecimate(decimated, strokes))
                {
                strokes = std::move(decimated);
                strokes.m_decimationTolerance = 0.0;
                didDecimate = true;
                }
            }

        return new InstanceableStrokes(std::move(strokes), shared, didDecimate);
        }

    size_t GetSharedVertexCount() const final { return m_numVertices; }
    DisplayParamsCR GetDisplayParams() const final { return *m_strokes.m_displayParams; }
    DRange3d ComputeRange() const final
        {
        DRange3d range = DRange3d::NullRange();
        for (auto const& pointList : m_strokes.m_strokes)
            range.Extend(pointList.m_points);

        return range;
        }

    MeshBuilderKey GetMeshBuilderKey() const final
        {
        return MeshBuilderKey(GetDisplayParams(), false, m_strokes.m_disjoint ? Mesh::PrimitiveType::Point : Mesh::PrimitiveType::Polyline, m_strokes.m_isPlanar);
        }
};

//=======================================================================================
// A max-heap of InstanceableGeom whose DisplayParams are compatible for batching.
// All the geometry in this collection *could* be batched into a single mesh.
// Geometry is sorted by number of total vertices so that the geometry with the highest
// vertex count is popped first.
// @bsistruct                                                   Paul.Connelly   04/19
//=======================================================================================
struct InstanceableGeomBucket
{
private:
    // Mutable members because we put these into a set - which only exposes const iterators because changing members might change the sort order
    // But our sort order depends solely on m_displayParams so yeah.
    mutable bvector<InstanceableGeomPtr> m_geom;
    mutable size_t m_numInstanceableVertices = 0;
    DisplayParamsCPtr m_displayParams;
    mutable MeshBuilderListPtr m_compatibleMeshBuilderList; // Contains uninstanced Mesh with compatible DisplayParams
public:
    explicit InstanceableGeomBucket(DisplayParamsCR displayParams) : m_displayParams(&displayParams) { }

    InstanceableGeomBucket(InstanceableGeomBucket&& src) : m_geom(std::move(src.m_geom)), m_numInstanceableVertices(src.m_numInstanceableVertices),
        m_displayParams(src.m_displayParams), m_compatibleMeshBuilderList(src.m_compatibleMeshBuilderList) {}

    InstanceableGeomBucket& operator=(InstanceableGeomBucket&& src)
        {
        if (this != & src)
            {
            m_geom = std::move(src.m_geom);
            m_numInstanceableVertices = src.m_numInstanceableVertices;
            m_compatibleMeshBuilderList = src.m_compatibleMeshBuilderList;
            m_displayParams = src.m_displayParams;
            }

        return *this;
        }

    void Push(InstanceableGeomR geom) const
        {
        BeAssert(GetDisplayParams().IsEqualTo(geom.GetDisplayParams(), ComparePurpose::Merge));

        m_geom.push_back(&geom);
        std::push_heap(m_geom.begin(), m_geom.end(), InstanceableGeom::ComparePtrs);
        m_numInstanceableVertices += geom.GetVertexCount();
        }

    InstanceableGeomPtr Pop() const
        {
        if (m_geom.empty())
            return nullptr;

        std::pop_heap(m_geom.begin(), m_geom.end(), InstanceableGeom::ComparePtrs);
        InstanceableGeomPtr back = m_geom.back();
        m_geom.pop_back();

        BeAssert(m_numInstanceableVertices >= back->GetVertexCount());
        m_numInstanceableVertices -= back->GetVertexCount();

        return back;
        }

    bool empty() const { return m_geom.empty(); }
    size_t size() const { return m_geom.size(); }

    size_t GetTotalVertexCount() const { return GetInstanceableVertexCount() + GetUninstanceableVertexCount(); }
    size_t GetInstanceableVertexCount() const { return m_numInstanceableVertices; }
    size_t GetUninstanceableVertexCount() const
        {
        return m_compatibleMeshBuilderList.IsValid() ? m_compatibleMeshBuilderList->GetVertexCount() : 0;
        }

    DisplayParamsCR GetDisplayParams() const { return *m_displayParams; }
    void SetCompatibleMeshBuilderList(MeshBuilderListP list) const { m_compatibleMeshBuilderList = list; }

    struct Comparator
    {
        bool operator()(InstanceableGeomBucket const& lhs, InstanceableGeomBucket const& rhs) const { return operator()(lhs.GetDisplayParams(), rhs.GetDisplayParams()); }
        bool operator()(InstanceableGeomBucket const& lhs, DisplayParamsCR rhs) const { return operator()(lhs.GetDisplayParams(), rhs); }
        bool operator()(DisplayParamsCR lhs, InstanceableGeomBucket const& rhs) const { return operator()(lhs, rhs.GetDisplayParams()); }
        bool operator()(DisplayParamsCR lhs, DisplayParamsCR rhs) const { return lhs.IsLessThan(rhs, ComparePurpose::Merge); }
    };
};

//=======================================================================================
// A collection of buckets of instanceable geometry. The buckets are grouped according
// to the compatibility of their DisplayParams - all geometry within a single bucket
// could be batched into a single Mesh.
// @bsistruct                                                   Paul.Connelly   04/19
//=======================================================================================
struct InstanceableGeomBuckets
{
private:
    using Buckets = std::set<InstanceableGeomBucket, InstanceableGeomBucket::Comparator>;

    Buckets m_buckets;
    ElementMeshGeneratorR m_meshGenerator;
    SubRangesR m_subRanges;

    void AddPolyfaces(PolyfaceList& polyfaces, SharedGeomCR geom)
        {
        for (auto& polyface : polyfaces)
            {
            PolyfaceHeaderPtr pf = polyface.m_polyface;
            if (pf.IsNull() || 0 == pf->GetPointIndexCount())
                continue;

            Add(*InstanceablePolyface::Create(polyface, geom));
            }
        }

    void AddStrokes(StrokesList&& strokes, SharedGeomCR geom)
        {
        for (auto& stroke : strokes)
            if (!stroke.m_strokes.empty())
                Add(*InstanceableStrokes::Create(std::move(stroke), geom));
        }

    // geom has been removed from the bucket. Determine if geom should be instanced.
    static bool IsWorthInstancing(InstanceableGeomCR geom, InstanceableGeomBucketCR bucket)
        {
        // A bucket consists of:
        //  - The number of uninstanceable facets with which this geometry *could* be batched; and
        //  - The number of instanceable facets with which this geometry *could* be batched.
        // Instancing this geometry will reduce the number of facets in the tile (i.e., reduce memory and bandwidth usage on client), but
        // introduce an extra draw call when rendering it.
        // We need to decide if the savings is worth the cost.

        // How many facets would result from batching the rest of the contents of the bucket (sans this geom)?
        double nBatchedFacets = static_cast<double>(bucket.GetTotalVertexCount());
        if (0.0 == nBatchedFacets)
            {
            // Nothing to batch with - might as well instance - either way is an extra draw call, but instancing saves some memory/bandwidth
            return true;
            }

        // If this geometry's facets represent a significant fraction of the total compatible facets, we consider it worth instancing.
        // Note the ratio cannot exceed 1 because we include this geometry's own facet count in the denominator.
        static double s_minFacetRatio = 0.25;
        double nThisFacets = static_cast<double>(geom.GetVertexCount());
        double ratio = nThisFacets / (nBatchedFacets + nThisFacets);
        return ratio >= s_minFacetRatio;
        }
public:
    InstanceableGeomBuckets(ElementMeshGeneratorR gen, SubRangesR subRanges) : m_meshGenerator(gen), m_subRanges(subRanges) { }

    using const_iterator = Buckets::const_iterator;
    size_t size() const { return m_buckets.size(); }
    bool empty() const { return m_buckets.empty(); }
    const_iterator begin() const { return m_buckets.begin(); }
    const_iterator end() const { return m_buckets.end(); }

    void Add(InstanceableGeomR geom)
        {
        if (!geom.GetSharedGeom().IsWorthInstancing())
            {
            geom.AddBatched(m_meshGenerator, m_subRanges);
            return;
            }

        auto inserted = m_buckets.insert(InstanceableGeomBucket(geom.GetDisplayParams()));
        inserted.first->Push(geom);
        if (inserted.second)
            {
            // We just created+inserted this bucket - find a compatible MeshBuilder for uninstanced geometry
            inserted.first->SetCompatibleMeshBuilderList(m_meshGenerator.FindMeshBuilderList(geom.GetMeshBuilderKey()).get());
            }
        }

    void Add(SharedGeomR sharedGeom)
        {
        auto const& loader = m_meshGenerator.GetLoader();
        SharedMeshGenerator sharedGen(m_meshGenerator, sharedGeom);
        for (auto const& geometry : sharedGeom.GetGeometries())
            {
            if (loader.IsCanceled())
                return;

            // ###TODO_INSTANCING: Consolidate code taken from MeshGenerator::AddMeshes()...
            DRange3dCR geomRange = geometry->GetTileRange();
            double rangePixels = (loader.Is3d() ? geomRange.DiagonalDistance() : geomRange.DiagonalDistanceXY()) / sharedGen.GetTolerance();
            if (rangePixels < s_minRangeBoxSize && 0.0 < geomRange.DiagonalDistance())
                return;

            auto polyfaces = geometry->GetPolyfaces(sharedGen.GetTolerance(), sharedGen.GetOptions().m_normalMode, sharedGen);
            auto strokes = geometry->GetStrokes(sharedGen.GetTolerance(), sharedGen);
            loader.GetLoader().Preprocess(polyfaces, strokes);

            AddPolyfaces(polyfaces, sharedGeom);
            AddStrokes(std::move(strokes), sharedGeom);
            }
        }

    void ProduceMeshes()
        {
        for (auto const& bucket : *this)
            {
            // The geometry in the bucket is sorted in descending order by number of vertices, and the "worth instancing" heuristic
            // is based on number of vertices - so once we decide a particular geometry is not worth instancing, we know all subsequent geometries
            // are also not worth instancing.
            bool shouldInstance = true;
            while (!bucket.empty() && !m_meshGenerator.GetLoader().IsCanceled())
                {
                InstanceableGeomPtr geom = bucket.Pop();
                shouldInstance = shouldInstance && IsWorthInstancing(*geom, bucket);
                if (shouldInstance)
                    geom->AddInstanced(m_meshGenerator, m_subRanges);
                else
                    geom->AddBatched(m_meshGenerator, m_subRanges);
                }
            }
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Feature featureFromParams(DgnElementId elemId, DisplayParamsCR params)
    {
    return Feature(elemId, params.GetSubCategoryId(), params.GetClass());
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/18
//=======================================================================================
struct TileCacheAppData : BeSQLite::Db::AppData
{
    RealityData::CachePtr m_cache;

    explicit TileCacheAppData(DgnDbCR db)
        {
        m_cache = Cache::Create(db);
        BeAssert(m_cache.IsValid());
        }

    static RealityData::CachePtr Get(DgnDbR db)
        {
        static BeSQLite::Db::AppData::Key s_key;
        auto data = db.ObtainAppData(s_key, [&]() { return new TileCacheAppData(db); });
        return data->m_cache;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool hasSymbologyChanges(GeometryStreamIO::Collection const& geom)
    {
    for (auto const& entry : geom)
        {
        switch (entry.m_opCode)
            {
            case GeometryStreamIO::OpCode::BasicSymbology:
            case GeometryStreamIO::OpCode::AreaFill:
            case GeometryStreamIO::OpCode::Pattern:
            case GeometryStreamIO::OpCode::Material:
                return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/19
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP parseId(uint64_t& output, Utf8CP str, Utf8Char terminator)
    {
    Utf8String idStr(str);
    auto terminatorPos = idStr.length();
    if ('\0' != terminator)
        {
        terminatorPos = idStr.find(terminator);
        if (Utf8String::npos == terminatorPos)
            return nullptr;

        idStr.erase(terminatorPos);
        }

    if (!BeInt64Id::IsWellFormedString(idStr))
        return nullptr;

    output = BeInt64Id::FromString(idStr.c_str()).GetValueUnchecked();
    return str + terminatorPos;
    }

/*---------------------------------------------------------------------------------**//**
* Parses lower-case hexadecimal unsigned integer up to terminating character.
* Returns pointer to terminating character, or nullptr if no digits, more digits than will
* fit in T, or terminator not found, or leading zeroes found.
* See Tree::Id::FromString() for why we are using hand-rolled parse.
* @bsimethod                                                    Paul.Connelly   05/19
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static Utf8CP parseHex(T& output, Utf8CP str, Utf8Char terminator)
    {
    output = 0;

    // No leading zeroes
    if ('0' == *str)
        return (terminator == *(str + 1)) ? str + 1 : nullptr;

    static constexpr size_t maxDigits = sizeof(T) << 1;
    for (size_t i = 0; i < maxDigits + 1; i++)
        {
        auto ch = str[i];
        if (ch == terminator)
            return i > 0 ? str + i : nullptr;
        else if (i == maxDigits)
            return nullptr;

        T val = 0;
        if (ch >= '0' && ch <= '9')
            val = ch - '0';
        else if (ch >= 'a' && ch <= 'f')
            val = (ch - 'a') + 10;
        else
            break;

        output = (output << 4) | val;
        }

    // no digits found; or (terminator not found / non-digit found) before max digits exceeded.
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* See Tree::Id::FromString() for why we are using hand-rolled parse.
* @bsimethod                                                    Paul.Connelly   05/19
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP parseExpansion(double& out, Utf8CP str)
    {
    if (':' != *str++)
        return nullptr;

    out = 0.0;
    bool first = true;
    do
        {
        auto ch = *str++;
        if ('.' == ch)
            {
            if (first)
                return nullptr;
            else
                break;
            }

        if (ch < '0' || ch > '9')
            return nullptr;

        out *= 10.0;
        out += ch - '0';
        first = false;
        }
    while (true);

    for (int i = 0; i < 6; i++)
        {
        auto ch = *str++;
        if (ch < '0' || ch > '9')
            return nullptr;

        double m = 1.0 / pow(10, i + 1);
        out += (ch - '0') * m;
        }

    return '_' == *str ? str + 1 : nullptr;
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentId::AllowInstancing() const { return Flags::None != (GetFlags() & Flags::AllowInstancing); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContentId::ToString() const
    {
    switch (GetMajorVersion())
        {
        case 1:
            {
            uint64_t parts[] = { static_cast<uint64_t>(m_depth), m_i, m_j, m_k, static_cast<uint64_t>(m_mult) };
            return Format(parts, _countof(parts));
            }
        case 0:
            BeAssert(false);
            return "";
        default:
            uint64_t parts[] = {
                static_cast<uint64_t>(m_majorVersion),
                static_cast<uint64_t>(m_flags),
                static_cast<uint64_t>(m_depth),
                m_i,
                m_j,
                m_k,
                static_cast<uint64_t>(m_mult),
            };

            if (m_majorVersion < 4)
                return Format(parts, _countof(parts));
            else
                return Format(parts + 1, _countof(parts) - 1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContentId::Format(uint64_t const* parts, size_t numParts) const
    {
    Utf8String str;
    Utf8Char buf[BeInt64Id::ID_STRINGBUFFER_LENGTH];

    bool isV1 = GetMajorVersion() == 1;
    uint32_t nSeparators = isV1 ? 0 : 1;
    Utf8Char separator = isV1 ? '/' : (GetMajorVersion() < 4 ? '_' : '-');
    for (auto i = 0; i < numParts; i++)
        {
        str.append(nSeparators, separator);
        nSeparators = 1;
        BeStringUtilities::FormatUInt64(buf, BeInt64Id::ID_STRINGBUFFER_LENGTH, parts[i], HexFormatOptions::None);
        str.append(buf);
        }

    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentId::FromString(Utf8CP str, uint16_t inMajorVersion)
    {
    if (Utf8String::IsNullOrEmpty(str))
        return false;

    if ('_' != *str)
        return '-' == *str ? FromV4String(str, inMajorVersion) : FromV1String(str);

    uint16_t majorVersion;
    Flags flags;
    uint64_t i, j, k;
    uint32_t mult;
    uint8_t depth;

    auto fmt = "_%" SCNx16 "_%" SCNx32 "_%" SCNx8 "_%" SCNx64 "_%" SCNx64 "_%" SCNx64 "_%" SCNx32 ;
    if (7 != BE_STRING_UTILITIES_UTF8_SSCANF(str, fmt, &majorVersion, &flags, &depth, &i, &j, &k, &mult))
        return false;
    else if (!Tile::IO::IModelTile::Version::IsKnownMajorVersion(majorVersion))
        return false;

    *this = ContentId(depth, i, j, k, mult, majorVersion, flags);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentId::FromV1String(Utf8CP str)
    {
    uint64_t i, j, k;
    uint32_t mult;
    uint8_t depth;
    if (5 != BE_STRING_UTILITIES_UTF8_SSCANF(str, "%" SCNx8 "/%" SCNx64 "/%" SCNx64 "/%" SCNx64 "/%" SCNx32, &depth, &i, &j, &k, &mult))
        return false;

    *this = ContentId(depth, i, j, k, mult, 1, Flags::None);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentId::FromV4String(Utf8CP str, uint16_t majorVersion)
    {
    if (majorVersion < 4)
        return false;

    Flags flags;
    uint64_t i, j, k;
    uint32_t mult;
    uint8_t depth;

    auto fmt = "-%" SCNx32 "-%" SCNx8 "-%" SCNx64 "-%" SCNx64 "-%" SCNx64 "-%" SCNx32 ;
    if (6 != BE_STRING_UTILITIES_UTF8_SSCANF(str, fmt, &flags, &depth, &i, &j, &k, &mult))
        return false;
    else if (!Tile::IO::IModelTile::Version::IsKnownMajorVersion(majorVersion))
        return false;

    *this = ContentId(depth, i, j, k, mult, majorVersion, flags);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
NodeId NodeId::ComputeParentId() const
    {
    auto depth = GetDepth();
    if (depth <= 1)
        {
        BeAssert(depth == 1);
        return RootId();
        }

    return NodeId(depth - 1, m_i >> 1, m_j >> 1, m_k >> 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d NodeId::ComputeRange(DRange3dCR rootRange, bool is2d) const
    {
    if (IsRoot() || rootRange.IsNull())
        return rootRange;

    auto parentId = ComputeParentId();
    auto parentRange = parentId.ComputeRange(rootRange, is2d);

    bool lowI = 0 == m_i % 2,
         lowJ = 0 == m_j % 2,
         lowK = 0 == m_k % 2;

    // 2d tiles are treated as quad-trees, not oct-trees, and never subdivide in Z.
    auto bisect = is2d ? bisectRange2d : bisectRange;
    DRange3d range = bisect(parentRange, lowI);
    range = bisect(range, lowJ);

    if (!is2d)
        range = bisect(range, lowK);

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* Create the table to hold entries in this Cache
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Cache::_Prepare() const
    {
    bool haveDataTable = m_db.TableExists(TABLE_NAME_TileTree);
    bool haveTimeTable = m_db.TableExists(TABLE_NAME_TileTreeCreateTime);

    auto deleteFromTable = [&](Utf8CP tableName)
        {
        Utf8String sql("DELETE FROM ");
        sql.append(tableName);
        CachedStatementPtr stmt;
        m_db.GetCachedStatement(stmt, sql.c_str());
        if (stmt.IsNull() || BE_SQLITE_DONE != stmt->Step())
            {
            BeAssert(false && "Failed to delete tile cache table contents");
            }
        };

    if (haveDataTable && haveTimeTable)
        {
        if (!ValidateData())
            {
            // The db schema is current, but the binary data format is not. Discard it.
            deleteFromTable(TABLE_NAME_TileTree);
            deleteFromTable(TABLE_NAME_TileTreeCreateTime);
            }

        return SUCCESS;
        }

    // Drop leftover tables from previous versions
    m_db.DropTableIfExists(TABLE_NAME_TileTree3);
    m_db.DropTableIfExists(TABLE_NAME_TileTree4);
    m_db.DropTableIfExists(TABLE_NAME_TileTreeCreateTime2);
    m_db.DropTableIfExists(TABLE_NAME_TileTreeCreateTime3);

    // Create the tables; or clear them if they already exist.
    if (haveTimeTable)
        deleteFromTable(TABLE_NAME_TileTreeCreateTime);
    else if (BE_SQLITE_OK != m_db.CreateTable(TABLE_NAME_TileTreeCreateTime, "TileId CHAR PRIMARY KEY,Created BIGINT,Guid BLOB"))
        return ERROR;

    if (haveDataTable)
        deleteFromTable(TABLE_NAME_TileTree);
    else if (BE_SQLITE_OK != m_db.CreateTable(TABLE_NAME_TileTree, "TileId CHAR PRIMARY KEY,Data BLOB,DataSize BIGINT TEXT"))
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Cache::_Cleanup() const
    {
    AccessLock lock(const_cast<Cache&>(*this));

    CachedStatementPtr sumStatement;
    m_db.GetCachedStatement(sumStatement, "SELECT SUM(DataSize) FROM " TABLE_NAME_TileTree);

    if (BE_SQLITE_ROW != sumStatement->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    uint64_t sum = sumStatement->GetValueInt64(0);
    if (sum <= m_allowedSize)
        return SUCCESS;

    uint64_t garbageSize = sum - static_cast<uint64_t>(m_allowedSize * .95); // 5% slack to avoid purging often

    CachedStatementPtr selectStatement;
    constexpr Utf8CP selectSql = "SELECT DataSize,Created FROM " JOIN_TileTreeTables " ORDER BY Created ASC";

    m_db.GetCachedStatement(selectStatement, selectSql);
    BeAssert(selectStatement.IsValid());

    uint64_t runningSum=0;
    while (runningSum < garbageSize)
        {
        if (BE_SQLITE_ROW != selectStatement->Step())
            {
            BeAssert(false);
            return ERROR;
            }

        runningSum += selectStatement->GetValueInt64(0);
        }

    BeAssert(runningSum >= garbageSize);
    uint64_t creationDate = selectStatement->GetValueInt64(1);
    BeAssert(creationDate > 0);

    // ###TODO: We should be using foreign key + cascading delete here...
    CachedStatementPtr deleteDataStatement;
    constexpr Utf8CP deleteDataSql = "DELETE FROM " TABLE_NAME_TileTree " WHERE TileId IN"
        " (SELECT TileId FROM " TABLE_NAME_TileTreeCreateTime " WHERE Created <= ?)";
    m_db.GetCachedStatement(deleteDataStatement, deleteDataSql);
    BeAssert(deleteDataStatement.IsValid());
    deleteDataStatement->BindInt64(1, creationDate);
    if (BE_SQLITE_DONE != deleteDataStatement->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    CachedStatementPtr deleteCreatedStatement;
    constexpr Utf8CP deleteCreatedSql = "DELETE FROM " TABLE_NAME_TileTreeCreateTime " WHERE Created <= ?";
    m_db.GetCachedStatement(deleteCreatedStatement, deleteCreatedSql);
    BeAssert(deleteCreatedStatement.IsValid());
    deleteCreatedStatement->BindInt64(1, creationDate);
    if (BE_SQLITE_DONE != deleteCreatedStatement->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RealityData::CachePtr Cache::Create(DgnDbCR db)
    {
    RealityData::CachePtr cache(new Cache(1024*1024*1024));
    BeFileName cacheName = db.GetFileName();
    cacheName.AppendExtension(L"Tiles");
    if (SUCCESS != cache->OpenAndPrepare(cacheName))
        cache = nullptr;

    return cache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Cache::GetCurrentVersion()
    {
    return Tile::IO::IModelTile::Version::Current().ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Cache::WriteCurrentVersion() const
    {
    // NB: SavePropertyString() is non-const because modifying *cacheable* properties mutates the Db's internal state
    // Our property is *not* cacheable
    auto& db = const_cast<BeSQLite::Db&>(m_db);
    if (BeSQLite::BE_SQLITE_OK != db.SavePropertyString(GetVersionSpec(), GetCurrentVersion()))
        {
        BeAssert(false && "Failed to save tile cache version");
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Cache::_Initialize() const
    {
    // We've created a brand-new cache Db. Write the current binary format version to its property table.
    return WriteCurrentVersion() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Cache::ValidateData() const
    {
    auto spec = GetVersionSpec();
    Utf8String storedVersion;
    if (BE_SQLITE_ROW == m_db.QueryProperty(storedVersion, spec) && storedVersion.Equals(GetCurrentVersion()))
        return true;

    // Binary format has changed. Discard existing tile data.
    WriteCurrentVersion();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t AnimationNodeMap::GetNodeIndex(uint64_t elemId) const
    {
    auto found = m_elemIdToNodeIndex.find(elemId);
    return m_elemIdToNodeIndex.end() != found ? found->second : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t AnimationNodeMap::GetDiscreteNodeIndex(uint64_t elemId) const
    {
    auto index = GetNodeIndex(elemId);
    return 0 != index && m_discreteNodeIndices.end() != m_discreteNodeIndices.find(index) ? index : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/18
+---------------+---------------+---------------+---------------+---------------+------*/
void AnimationNodeMap::Populate(DisplayStyleCR style, DgnModelId modelId)
    {
    Clear();

    auto schedule = style.GetStyle("scheduleScript");
    if (schedule.isNull())
        return;

    for (auto& modelTimeline : schedule)
        {
        if (modelTimeline["modelId"].asUInt64() == modelId.GetValue())
            {
            auto& elementTimelines = modelTimeline["elementTimelines"];
            for (auto& elementTimeline : elementTimelines)
                {
                if (elementTimeline.isMember("elementIds"))
                    {
                    auto& elementIds = elementTimeline["elementIds"];
                    auto nodeIndex = elementTimeline["batchId"].asUInt();
                    BeAssert(0 != nodeIndex);
                    m_maxNodeIndex = std::max(nodeIndex, m_maxNodeIndex);
                    for (auto& elementId : elementIds)
                        m_elemIdToNodeIndex.Insert(elementId.asUInt64(), nodeIndex);

                    if (elementTimeline.isMember("transformTimeline") || elementTimeline.isMember("cuttingPlaneTimeline"))
                        m_discreteNodeIndices.insert(nodeIndex);
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::LoadNodeMapFromAnimation()
    {
    auto displayStyle = m_db.Elements().Get<DisplayStyle>(m_id.GetAnimationSourceId());
    if (displayStyle.IsValid())
        m_nodeMap.Populate(*displayStyle, GetModelId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
TreePtr Tree::Create(GeometricModelR model, Render::SystemR system, Id id)
    {
    if (DgnDbStatus::Success != model.FillRangeIndex())
        return nullptr;

    DRange3d range;
    DRange3d contentRange = DRange3d::NullRange();
    bool populateRootTile;
    bool rootTileEmpty;
    if (model.Is3dModel())
        {
        // NB: We used to use project extents. For read-only scenarios, that's problematic when multiple models exist because:
        // - if a model occupies a small fraction of the extents, we must subdivide its tile tree excessively before we reach useful geometry; and
        // - some models contain junk elements in far orbit which blow out the project extents resulting in same problem - constrain that only to those models.
        // Classifiers are applied to all models (currently...) so they use project extents.
        auto projectExtents = model.GetDgnDb().GeoLocation().GetProjectExtents();
        auto useProjectExtents = id.GetUseProjectExtents();
        range.IntersectionOf(projectExtents, model.QueryElementsRange());

        if(id.IsVolumeClassifier() || id.IsPlanarClassifier() && 0.0 != id.GetClassifierExpansion())
            range.Extend(id.GetClassifierExpansion());

        range = scaleSpatialRange(range);

        if (useProjectExtents)
            {
            // Actually use the project extents as the tile tree range, but also include the model range as a bounding content range.
            // This allows us to produce root tiles of appropriate resolution when model range is small relative to project extents, instead
            // of producing extremely high-resolution tiles inappropriate for display when fitting to project extents.
            contentRange = range;
            range = projectExtents;
            scaleSpatialRange(range);
            }

        uint32_t nElements = 0;

        populateRootTile = !range.IsNull() && isElementCountLessThan(s_minElementsPerTile, *model.GetRangeIndex(), &nElements);
        if (id.IsVolumeClassifier())
            populateRootTile = true;    // The volume classifier algorithm currently cannot handle multiple tiles -- force the root to populate...

        rootTileEmpty = 0 == nElements;
        }
    else
        {
        RangeAccumulator accum(range, model.Is2dModel());
        if (!accum.Accumulate(*model.GetRangeIndex()))
            range = DRange3d::From(DPoint3d::FromZero());

        auto sheet = model.ToSheetModel();
        if (nullptr != sheet)
            range.Extend(sheet->GetSheetExtents());

        populateRootTile = accum.GetElementCount() < s_minElementsPerTile;
        rootTileEmpty = 0 == accum.GetElementCount();
        }

// un-comment this to always populate the root tile
// #define POPULATE_ROOT_TILE
#if defined(POPULATE_ROOT_TILE)
    populateRootTile = true;
#endif

    auto rootTile = rootTileEmpty ? RootTile::Empty : (populateRootTile ? RootTile::Displayable : RootTile::Undisplayable);

    // Translate world coordinates to center of range in order to reduce precision errors
    DPoint3d centroid = DPoint3d::FromInterpolate(range.low, 0.5, range.high);
    Transform transform = Transform::From(centroid);

    DRange3d tileRange = range;
    if (!range.IsNull())
        {
        Transform rangeTransform;
        rangeTransform.InverseOf(transform);
        rangeTransform.Multiply(tileRange, range);

        if (!contentRange.IsNull())
            rangeTransform.Multiply(contentRange, contentRange);
        }

    return new Tree(model, transform, tileRange, system, id, rootTile, contentRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Tree::Tree(GeometricModelCR model, TransformCR location, DRange3dCR range, Render::SystemR system, Id id, RootTile rootTile, DRange3dCR contentRange)
    : m_db(model.GetDgnDb()), m_location(location), m_renderSystem(system), m_id(id), m_is3d(model.Is3d()), m_cache(TileCacheAppData::Get(model.GetDgnDb())),
    m_rootTile(rootTile), m_contentRange(contentRange)
    {
    if (!range.IsNull())
        m_range.Extend(range);

    if (m_id.ContainsAnimation())
        LoadNodeMapFromAnimation();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Tree::~Tree()
    {
    Destroy();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/19
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::Destroy()
    {
    CancelAllTileLoads();
    WaitForAllLoads();
    BeAssert(m_activeLoads.empty());
    m_cache = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value Tree::ToJson() const
    {
    Json::Value json(Json::objectValue);

    json["id"] = GetId().ToString();
    json["maxTilesToSkip"] = 1;
    json["formatVersion"] = Tile::IO::IModelTile::Version::FromMajorVersion(GetId().GetMajorVersion()).ToUint32();
    JsonUtils::TransformToJson(json["location"], GetLocation());

    if (!m_contentRange.IsNull())
        JsonUtils::DRange3dToJson(json["contentRange"], m_contentRange);

    json["rootTile"]["isLeaf"] = RootTile::Empty == m_rootTile;
    json["rootTile"]["maximumSize"] = RootTile::Displayable == m_rootTile ? 512 : 0;
    json["rootTile"]["contentId"] = "0/0/0/0/1"; // NB: Strictly for older front-ends - newer ones ignore and compute based on tile format version
    JsonUtils::DRange3dToJson(json["rootTile"]["range"], GetRange());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Tree::CancelAllTileLoads()
    {
    BeMutexHolder lock(m_cv.GetMutex());
    for (auto& load : m_activeLoads)
        load->SetCanceled();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
LoaderPtr Tree::CreateLoader(ContentIdCR contentId, bool useCache)
    {
    switch (GetType())
        {
        case Type::VolumeClassifier:
            return new VolumeClassificationLoader(*this, contentId, useCache);

        case Type::PlanarClassifier:
            return new PlanarClassificationLoader(*this, contentId, useCache);

        default:
            return new Loader(*this, contentId, useCache);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
ContentCPtr Tree::RequestContent(ContentIdCR contentId, bool useCache)
    {
    LoaderPtr loader;
    BeMutexHolder lock(m_cv.GetMutex());
    auto iter = m_activeLoads.find(contentId);
    if (iter == m_activeLoads.end())
        {
        // Load the tile content synchronously on this thread
        loader = CreateLoader(contentId, useCache);
        LoaderScope scope(*loader, lock);
        }
    else
        {
        // Content is already being loaded on another thread. Await the result on this thread.
        loader = *iter;
        auto condition = [&](BeConditionVariable&) { return !loader->IsLoading(); };
        ConditionVariablePredicate<decltype(condition)> predicate(condition);
        m_cv.ProtectedWaitOnCondition(lock, &predicate, BeConditionVariable::Infinite);
        }

    return loader->IsReady() ? loader->GetContent() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/19
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t Tree::Id::GetDefaultMajorVersion()
    {
    return Tile::IO::IModelTile::Version::Current().m_major;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool Tree::Id::GetUseProjectExtents() const
    {
    return Tree::Flags::None != (GetFlags() & Tree::Flags::UseProjectExtents);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Tree::Id::GetPrefixString() const
    {
    Utf8String prefix;

    // In version 4 we introduced versioning - the ID includes the major version of the tile format plus flags.
    if (GetMajorVersion() > 3)
        {
        Utf8Char buf[BeInt64Id::ID_STRINGBUFFER_LENGTH];
        BeStringUtilities::FormatUInt64(buf, BeInt64Id::ID_STRINGBUFFER_LENGTH, static_cast<uint64_t>(GetMajorVersion()), HexFormatOptions::None);
        prefix.append(buf).append(1, '_');
        BeStringUtilities::FormatUInt64(buf, BeInt64Id::ID_STRINGBUFFER_LENGTH, static_cast<uint64_t>(GetFlags()), HexFormatOptions::None);
        prefix.append(buf).append(1, '-');
        }

    if (IsClassifier())
        {
        prefix.append(IsVolumeClassifier() ? s_classifierIdPrefix : s_planarClassifierIdPrefix);
        prefix.append(Utf8PrintfString("%.6f_", m_expansion));
        if (this->ContainsAnimation())
            prefix.append(s_animationIdPrefix + m_animationSourceId.ToHexStr()).append(1, '_');
        }
    else
        {
        if (this->ContainsAnimation())
            prefix.append(s_animationIdPrefix + m_animationSourceId.ToHexStr()).append(1, '_');

        if (GetOmitEdges())
            prefix.append(s_omitEdgesPrefix).append(1, '_');

        }   


    return prefix;
    }

/*---------------------------------------------------------------------------------**//**
* v3.0 and earlier:
*   Classifiers: "prefix:[A:animSourceId_]expansion_modelId"
*       prefix=CP (planar classifier) or P (volume classifier)
*       expansion=.6f double-precision floating point value
*       modelId=hex-formatted BeInt64Id
*   Others: "[A:animSourceId_][E:0_]modelId" where "[X]" indicates optional component
*       E:0_ indicates edge data should be omitted from tile data
*       A:animSourceId_ includes hex-formatted ID of the source of animation data
*       modelId=hex-formatted BeInt64Id
* v4.0 and later: "vMaj_flags-v3Id"
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Tree::Id::ToString() const
    {
    if (!IsValid())
        return "";

    return GetPrefixString()  + m_modelId.ToHexStr();
    }

/*---------------------------------------------------------------------------------**//**
* NB: The tree ID string is used as a cache lookup key for both binary tile data and
* tile tree JSON. The native string must exactly match that produced by the front-end
* for caching to work correctly.
* @bsimethod                                                    Paul.Connelly   05/19
+---------------+---------------+---------------+---------------+---------------+------*/
Tree::Id Tree::Id::FromString(Utf8StringCR str, DgnDbP db)
    {
    Id invalidId;
    if (Utf8String::IsNullOrEmpty(str.c_str()))
        return invalidId;

    // A hyphen delimits the version+flags from the rest of the ID, if they are present.
    // Absence indicates a pre-v4.0 ID.
    Utf8CP pCur = str.c_str();
    uint16_t majorVersion = 3;
    auto flags = Tree::Flags::None;
    auto hyphenPos = str.find('-');
    bool hasFlagsAndMajorVersion = Utf8String::npos != hyphenPos;
    if (hasFlagsAndMajorVersion)
        {
        pCur = parseHex(majorVersion, pCur, '_');
        if (nullptr == pCur || majorVersion < 4)
            return invalidId;

        uint32_t uflags;
        pCur = parseHex(uflags, pCur + 1, '-');
        if (nullptr == pCur)
            return invalidId;

        flags = static_cast<Tree::Flags>(uflags);
        ++pCur;
        }

    if (!Tile::IO::IModelTile::Version::IsKnownMajorVersion(majorVersion))
        return invalidId;

    auto type = Tree::Type::Model;
    double expansion = 0.0;
    uint64_t animationId64 = 0;
    bool omitEdges = false;
    switch (*pCur)
        {
        case 'C':
            {
            omitEdges = true;
            type = Tree::Type::VolumeClassifier;
            if ('P' == *(++pCur))
                {
                type = Tree::Type::PlanarClassifier;
                ++pCur;
                }

            pCur = parseExpansion(expansion, pCur);
            if (nullptr == pCur)
                return invalidId;

            if ('A' == *pCur) 
                {
                if (':' != *(++pCur))
                    return invalidId;

                pCur = parseId(animationId64, ++pCur, '_');
                if (nullptr == pCur)
                    return invalidId;

                ++pCur;
                }

            break;
            }
        case '0':
            break;
        case 'A':
            {
            if (':' != *(++pCur))
                return invalidId;

            pCur = parseId(animationId64, ++pCur, '_');
            if (nullptr == pCur)
                return invalidId;

            ++pCur;
            if ('0' == *pCur)
                break;
            else if ('E' != *pCur)
                return invalidId;
            }
        // fall-through intentional...
        case 'E':
            omitEdges = true;
            if (':' != *(++pCur) || '0' != *(++pCur) || '_' != *(++pCur))
                return invalidId;

            ++pCur;
            break;
        default:
            return invalidId;
        }

    uint64_t modelId64;
    if (nullptr == parseId(modelId64, pCur, '\0'))
        return invalidId;

    DgnModelId modelId(modelId64);
    if (!modelId.IsValid())
        return invalidId;

    GeometricModelPtr model;
    if (nullptr != db)
        {
        model = db->Models().Get<GeometricModel>(modelId);
        if (model.IsNull())
            return invalidId;
        }

    DgnElementId animationId(animationId64);

    switch (type)
        {
        case Tree::Type::VolumeClassifier:
            if (!hasFlagsAndMajorVersion)
                flags |= Tree::Flags::UseProjectExtents;
            else if (Tree::Flags::None == (flags & Tree::Flags::UseProjectExtents))
                return invalidId;
            // fall-through intentional
        case Tree::Type::PlanarClassifier:
            if (model.IsValid() && !model->Is3d())
                return invalidId;
            else
                return Tree::Id(modelId, flags, majorVersion, expansion, Tree::Type::PlanarClassifier == type, animationId);
        case Tree::Type::Model:
            break;
        }

    if (animationId.IsValid() && nullptr != db)
        {
        auto style = db->Elements().Get<DisplayStyle>(animationId);
        if (style.IsNull() || style->GetStyle("scheduleScript").isNull())
            return invalidId;
        }


    return Tree::Id(modelId, flags, majorVersion, omitEdges, animationId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Tree::LoaderScope::LoaderScope(LoaderR loader, BeMutexHolder& lock) : m_loader(&loader), m_lock(lock)
    {
    // NB: Mutex is held by this thread on entry...
    loader.GetTree().m_activeLoads.insert(m_loader);

    // Release mutex while loading...
    lock.unlock();
    loader.Perform();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Tree::LoaderScope::~LoaderScope()
    {
    // Ensure loader removed and waiting threads notified even if exception occurs during Loader::Perform().
    m_lock.lock();
    auto& loads = m_loader->GetTree().m_activeLoads;
    BeAssert(loads.end() != loads.find(m_loader));
    loads.erase(m_loader);
    BeAssert(loads.end() == loads.find(m_loader));
    m_loader->GetTree().m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Loader::Loader(TreeR tree, ContentIdCR contentId, bool useCache) : m_contentId(contentId), m_tree(tree), m_cacheKey(tree.ConstructCacheKey(contentId)),
    m_geometryGuid(tree.FetchModel()->QueryGeometryGuid()), m_useCache(useCache)
    {
    SetState(State::Loading);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Loader::~Loader()
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/18
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t Loader::GetNodeId(DgnElementId id) const { return m_tree.GetDiscreteNodeId(id); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool Loader::AllowInstancing() const
    {
    return m_tree.AllowInstancing() && GetContentId().AllowInstancing();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/18
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t Tree::GetDiscreteNodeId(DgnElementId elementId) const
    {
    return m_nodeMap.GetDiscreteNodeIndex(elementId.GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Loader::IsExpired(BeSQLite::BeGuid guid) const
    {
    return guid != m_geometryGuid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Loader::IsValidData(ByteStreamCR cacheData) const
    {
    // No further validation required currently - changes to geometric elements are detected by IsExpired.
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Loader::DropFromDb(RealityData::CacheR cache)
    {
    CachedStatementPtr stmt;
    cache.GetDb().GetCachedStatement(stmt, "DELETE FROM " TABLE_NAME_TileTree " WHERE TileId=?");
    stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
    if (BE_SQLITE_DONE != stmt->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    cache.GetDb().GetCachedStatement(stmt, "DELETE FROM " TABLE_NAME_TileTreeCreateTime " WHERE TileId=?");
    stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
    if (BE_SQLITE_DONE != stmt->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Loader::SaveToDb()
    {
    if (!m_useCache)
        return SUCCESS;

    if (m_content.IsNull())
        return ERROR;

    auto cache = GetTree().GetCache();
    if (nullptr == cache)
        return ERROR;

    BeAssert(!m_cacheKey.empty());

    RealityData::Cache::AccessLock lock(*cache);

    // "INSERT OR REPLACE" so we can update old data that we failed to load.
    CachedStatementPtr stmt;
    auto rc = cache->GetDb().GetCachedStatement(stmt, "INSERT OR REPLACE INTO " TABLE_NAME_TileTree " (TileId,Data,DataSize) VALUES (?,?,?)");

    BeAssert(rc == BE_SQLITE_OK);
    BeAssert(stmt.IsValid());

    ByteStreamCR bytes = m_content->GetBytes();

    std::unique_ptr<SnappyToBlob> snappy;
    stmt->ClearBindings();
    stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
    if (bytes.empty())
        {
        stmt->BindZeroBlob(2, 0);
        stmt->BindInt64(3, 0);
        }
    else
        {
        snappy = std::make_unique<SnappyToBlob>();
        snappy->Init();
        CacheBlobHeader header(bytes.GetSize());
        snappy->Write((Byte const*) &header, sizeof(header));
        snappy->Write(bytes.GetData(), (int) bytes.GetSize());
        uint32_t zipSize = snappy->GetCompressedSize();
        stmt->BindZeroBlob(2, zipSize);
        stmt->BindInt64(3, (int64_t) zipSize);
        }

    rc = stmt->Step();
    if (BE_SQLITE_DONE != rc)
        return ERROR;

    // Compress and save the data
    rc = cache->GetDb().GetCachedStatement(stmt, "SELECT ROWID FROM " TABLE_NAME_TileTree " WHERE TileId=?");
    BeAssert(BE_SQLITE_OK == rc && stmt.IsValid());

    stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt->Step())
        return ERROR;

    uint64_t rowId = stmt->GetValueInt64(0);

    if (!bytes.empty())
        {
        BeAssert(nullptr != snappy);
        StatusInt status = snappy->SaveToRow( cache->GetDb(), TABLE_NAME_TileTree, "Data", rowId);
        if (SUCCESS != status)
            return ERROR;
        }

    // Write the tile creation time into separate table so that when we update it on next use of this tile, sqlite doesn't have to copy the potentially-huge data column
    rc = cache->GetDb().GetCachedStatement(stmt, "INSERT OR REPLACE INTO " TABLE_NAME_TileTreeCreateTime " (TileId,Created,Guid) VALUES (?,?,?)");
    BeAssert(BE_SQLITE_OK == rc && stmt.IsValid());

    stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
    stmt->BindInt64(2, BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    stmt->BindGuid(3, GetGeometryGuid());

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Loader::Perform()
    {
    BeAssert(IsLoading());
    if (IsCanceled())
        return;

    auto state = ReadFromCache();
    if (IsCanceled())
        return;

    if (State::NotFound == state)
        {
        state = ReadFromModel();
        if (IsCanceled())
            return;
        else if (State::Ready == state && SUCCESS != SaveToDb())
            BeAssert(false);
        }

    SetState(state);
    BeAssert(!IsLoading());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Loader::State Loader::ReadFromCache()
    {
    if (!m_useCache)
        return State::NotFound;

    BeAssert(IsLoading());
    auto cache = GetTree().GetCache();
    if (nullptr == cache)
        return State::Invalid;

    if (true)
        {
        RealityData::Cache::AccessLock lock(*cache); // block writes to cache while we're reading

        enum Column : int { Data, DataSize, Guid, Rowid };
        CachedStatementPtr stmt;
        constexpr Utf8CP selectSql = "SELECT Data,DataSize,Guid," TABLE_NAME_TileTree ".ROWID as TileRowId"
            " FROM " JOIN_TileTreeTables " WHERE " COLUMN_TileTree_TileId "=?";

        if (BE_SQLITE_OK != cache->GetDb().GetCachedStatement(stmt, selectSql))
            return State::Invalid;

        stmt->ClearBindings();
        stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW != stmt->Step())
            return State::NotFound;

        uint64_t rowId = stmt->GetValueInt64(Column::Rowid);
        BeSQLite::BeGuid guid = stmt->GetValueGuid(Column::Guid);
        if (IsExpired(guid))
            {
            DropFromDb(*cache);
            return State::NotFound;
            }

        if (0 == stmt->GetValueInt64(Column::DataSize))
            {
            m_content = new Content(ByteStream());
            }
        else
            {
            // Potentially too large to allocate on stack...
            auto snappy = std::make_unique<BeSQLite::SnappyFromBlob>();
            if (ZIP_SUCCESS != snappy->Init(cache->GetDb(), TABLE_NAME_TileTree, "Data", rowId))
                return State::Invalid;

            CacheBlobHeader header(*snappy);
            uint32_t sizeRead;
            if (CacheBlobHeader::DB_Signature06 != header.m_signature || 0 == header.m_size)
                return State::Invalid;

            ByteStream bytes;
            bytes.Resize(header.m_size);
            snappy->ReadAndFinish(bytes.GetDataP(), header.m_size, sizeRead);
            if (sizeRead != header.m_size)
                return State::Invalid;

            if (!IsValidData(bytes))
                {
                DropFromDb(*cache);
                return State::NotFound;
                }

// #define DEBUG_DUMP_TILE_BYTES
#if defined(DEBUG_DUMP_TILE_BYTES)
            // We have some tests in iModel.js front-end which want to validate tile content.
            // They want the bytes as a Uint8Array.
            auto filename = GetTree().GetDgnDb().GetFileName();
            WString cacheKey(m_cacheKey.c_str(), true);
            cacheKey.ReplaceAll(L"/", L"_");
            filename.append(cacheKey);
            filename.append(L".json");

            Utf8String jsonStr = "[";
            for (size_t i = 0; i < bytes.size(); i++)
                {
                if (0 == i % 20)
                    jsonStr.append("\n ");

                Utf8PrintfString hex(" 0x%02x,", bytes[i]);
                jsonStr.append(hex);
                }

            jsonStr.append("\n]");

            BeFile file;
            file.Create(filename.c_str(), true);
            file.Write(nullptr, jsonStr.c_str(), static_cast<uint32_t>(jsonStr.size()));
#endif

            m_content = new Content(std::move(bytes));
            }

        // We've loaded data from cache. Update timestamp.
        if (BE_SQLITE_OK == cache->GetDb().GetCachedStatement(stmt, "UPDATE " TABLE_NAME_TileTreeCreateTime " SET Created=? WHERE TileId=?"))
            {
            stmt->BindInt64(1, BeTimeUtilities::GetCurrentTimeAsUnixMillis());
            stmt->BindText(2, m_cacheKey, Statement::MakeCopy::No);
            if (BE_SQLITE_DONE != stmt->Step())
                BeAssert(false);
            }
        }

    return State::Ready;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Loader::State Loader::ReadFromModel()
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    BeAssert(PSolidKernelManager::IsSessionStarted());
    PSolidThreadUtil::WorkerThreadOuterMark outerMark;
#endif

    // Generate geometry
    GeometryLoader geomLoader(*this);
    if (!geomLoader.GenerateGeometry())
        return State::Invalid;
    else if (IsCanceled())
        return State::NotFound; // doesn't matter...

    // Encode geometry to binary stream
    StreamBuffer tileBytes;
    if (Dgn::Tile::IO::IModelTile::WriteStatus::Success != Dgn::Tile::IO::WriteIModelTile(tileBytes, geomLoader.GetMetadata(), geomLoader.GetGeometry(), *this))
        return State::Invalid;

    if (IsCanceled())
        return State::NotFound;

    m_content = new Content(std::move(tileBytes));

    return State::Ready;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
VolumeClassificationLoader::VolumeClassificationLoader(Tree& tree, ContentIdCR contentId, bool useCache) : T_Super(tree, contentId, useCache)
    {
    BeAssert(tree.IsVolumeClassifier());
    Transform fromDgn;
    fromDgn.InverseOf(tree.GetLocation());
    fromDgn.Multiply(m_range, tree.GetDgnDb().GeoLocation().GetProjectExtents());
    m_expansion = tree.GetId().GetClassifierExpansion();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
PlanarClassificationLoader::PlanarClassificationLoader(Tree& tree, ContentIdCR contentId, bool useCache) : T_Super(tree, contentId, useCache)
    {
    BeAssert(tree.IsPlanarClassifier());
    m_expansion = tree.GetId().GetClassifierExpansion();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void VolumeClassificationLoader::Preprocess(PolyfaceList& polyfaces, StrokesList& strokes) const
    {
    if (0.0 != m_expansion)
        {
        for (auto& polyface : polyfaces)
            {
            PolyfaceHeaderPtr offsetPolyface = polyface.m_polyface->ComputeOffset(PolyfaceHeader::OffsetOptions(), m_expansion, 0.0, true, false, false);

            if (offsetPolyface.IsValid())
                polyface.m_polyface = offsetPolyface;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void addPolyfaceQuad(PolyfaceHeaderR polyface, PolyfaceCoordinateMapR coordinateMap, int32_t* normalIndices, DPoint3dCR p0, DPoint3dCR p1, DPoint3dCR p2, DPoint3dCR p3)
    {
    int32_t     indices0[3], indices1[3];

    indices0[0]               = 1 + (int) coordinateMap.AddPoint(p0);
    indices0[2] = indices1[0] = 1 + (int) coordinateMap.AddPoint(p1);
    indices0[1] = indices1[1] = 1 + (int) coordinateMap.AddPoint(p2);
    indices1[2]               = 1 + (int) coordinateMap.AddPoint(p3);

    polyface.AddIndexedFacet(3, indices0, normalIndices);
    polyface.AddIndexedFacet(3, indices1, normalIndices);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/18
+---------------+---------------+---------------+---------------+---------------+------*/
static PolyfaceHeaderPtr polyfaceFromExpandedStrokes(StrokesCR strokes, double expansion)
    {
    PolyfaceHeaderPtr           polyface = PolyfaceHeader::CreateVariableSizeIndexed();
    PolyfaceCoordinateMapPtr    coordinateMap = PolyfaceCoordinateMap::Create(*polyface);
    bvector<DPoint3d>           points;
    static constexpr double     s_tolerance = 1.0E-5;
    static constexpr double     s_maxMiterDot = .75;
    DVec3d                      normal = DVec3d::UnitZ();      // Assume linear classification is always on XY plane...
    int32_t                     normalIndices[3];

    normalIndices[0] = normalIndices[1] = normalIndices[2] = 1 + (int) coordinateMap->AddNormal(normal);
    for (auto& stroke : strokes.m_strokes)
        {
        if (stroke.m_points.empty())
            {
            BeAssert(false);
            continue;
            }

        if (strokes.m_disjoint)
            {
            // TBD... stroke point based on tolerance.
            for (auto& point : stroke.m_points)
                addPolyfaceQuad(*polyface, *coordinateMap, normalIndices, DPoint3d::FromXYZ(point.x - expansion, point.y - expansion, point.z),
                                                                          DPoint3d::FromXYZ(point.x - expansion, point.y + expansion, point.z),
                                                                          DPoint3d::FromXYZ(point.x + expansion, point.y - expansion, point.z),
                                                                          DPoint3d::FromXYZ(point.x + expansion, point.y + expansion, point.z));
            continue;
            }

        size_t  segmentCount = stroke.m_points.size() - 1;

        for (size_t i=0; i<segmentCount; i++)
            {
            DPoint3d    start = stroke.m_points[i], end = stroke.m_points[i+1];
            DVec3d      delta = DVec3d::FromStartEnd(start, end);
            DVec3d      perp = DVec3d::FromCrossProduct(normal, delta);

            if (perp.Normalize() < s_tolerance ||
                delta.Normalize() < s_tolerance)
                continue;

            DVec3d      perp0 = perp, perp1 = perp;
            double      distance0 = expansion, distance1 = expansion;

            if (i > 0)
                {
                DVec3d      dir = delta, prevDir = DVec3d::FromStartEndNormalize(start, stroke.m_points[i-1]);
                double      dot = prevDir.DotProduct(dir);

                if (dot > -.9999 && dot < s_maxMiterDot)
                    {
                    perp0.Normalize(DVec3d::FromSumOf(dir, prevDir));
                    distance0 /= perp0.DotProduct(perp);
                    }
                }
            if (i < segmentCount - 1)
                {
                DVec3d      dir = delta, nextDir = DVec3d::FromStartEndNormalize(end, stroke.m_points[i+2]);
                dir.Negate();
                double      dot = nextDir.DotProduct(dir);

                if (dot > -.9999 && dot < s_maxMiterDot)
                    {
                    perp1.Normalize(DVec3d::FromSumOf(dir, nextDir));
                    distance1 /= perp1.DotProduct(perp);
                    }
                }
            addPolyfaceQuad(*polyface, *coordinateMap, normalIndices, DPoint3d::FromSumOf(start, perp0, -distance0),
                                                                      DPoint3d::FromSumOf(start, perp0,  distance0),
                                                                      DPoint3d::FromSumOf(end,   perp1, -distance1),
                                                                      DPoint3d::FromSumOf(end,   perp1,  distance1));
            }
        }

    return (0 == polyface->GetPointIndexCount()) ? nullptr : polyface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void expandClosedPlanarPolyface(PolyfaceHeaderR polyface, DPlane3dCR plane, double expansion)
    {
    struct Vertex
        {
        int32_t  m_previousIndex;
        int32_t  m_nextIndex;
        Vertex() {}
        Vertex(int32_t previousIndex, int32_t nextIndex): m_previousIndex(previousIndex), m_nextIndex(nextIndex) { }
        };
    bmap<int32_t, Vertex>       vertexMap;

    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(polyface); visitor->AdvanceToNextFace(); )
        {
        uint32_t numEdges =  visitor->NumEdgesThisFace();
        for (uint32_t i = 0; i <numEdges; i++)
            {
            int thisIndex, nextIndex;
            bool visible, nextVisible;

            if (visitor->TryGetClientZeroBasedPointIndex(i, thisIndex, visible) && visible &&
                visitor->TryGetClientZeroBasedPointIndex((i + 1) % numEdges, nextIndex, nextVisible))
                {
                auto thisFound = vertexMap.find(thisIndex);
                if (thisFound == vertexMap.end())
                    vertexMap.Insert(thisIndex, Vertex(-1, nextIndex));
                else
                    thisFound->second.m_nextIndex = nextIndex;

                auto nextFound =  vertexMap.find(nextIndex);
                if (nextFound == vertexMap.end())
                    vertexMap.Insert(nextIndex, Vertex(thisIndex, -1));
                else
                    nextFound->second.m_previousIndex = thisIndex;
                }
            }
        }
    bvector<DPoint3d>  pointCopy = polyface.Point();
    DPoint3dCP points = &pointCopy.front();

    for (auto curr:  vertexMap)
        {
        if (curr.second.m_previousIndex < 0 || curr.second.m_nextIndex < 0)
            {
 //         BeAssert(false);
            continue;
            }
        DPoint3dCR thisPoint = points[curr.first];
        DVec3d  prev = DVec3d::FromStartEnd(points[curr.second.m_previousIndex], thisPoint),
                next = DVec3d::FromStartEnd(points[curr.second.m_nextIndex], thisPoint);
        double minMagnitude = 1.0E-5;
        bool prevDegenerate = prev.Normalize() < minMagnitude, nextDegenerate = next.Normalize() < minMagnitude;
        if (prevDegenerate && nextDegenerate)
            continue;
        double expandDistance = expansion;
        DVec3d expandDirection;
        if (prevDegenerate)
            expandDirection = DVec3d::FromCrossProduct(prev, plane.normal);
        else if (nextDegenerate)
            expandDirection = DVec3d::FromCrossProduct(plane.normal, next);
        else
            {
            DVec3d perp =  DVec3d::FromCrossProduct(prev, plane.normal);
            expandDirection = DVec3d::FromSumOf(prev, next);
            if (expandDirection.Normalize() < minMagnitude)
                expandDirection =  perp;
            else
                expandDistance = expansion / perp.DotProduct(expandDirection);
            }
        polyface.Point()[curr.first] = DPoint3d::FromSumOf(thisPoint, expandDirection, expandDistance);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void PlanarClassificationLoader::Preprocess(PolyfaceList& polyfaces, StrokesList& strokes) const
    {
    if (0.0 != m_expansion)
        {
        for (auto& polyface : polyfaces)
            {
            DPlane3d plane;

            if (polyface.m_polyface->IsClosedPlanarRegion(plane))
                expandClosedPlanarPolyface(*polyface.m_polyface, plane, m_expansion);
            }
        if (!strokes.empty())
            {
            auto    meshDisplayParams  = strokes.front().m_displayParams->CloneForMeshedLineString();
            for (auto& strokes : strokes) {
                PolyfaceHeaderPtr   polyface = polyfaceFromExpandedStrokes(strokes, m_expansion);

                if (polyface.IsValid())
                    polyfaces.push_back(Polyface(*meshDisplayParams, *polyface, false, true));
                }
            }
        strokes.clear();
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryLoader::GeometryLoader(LoaderCR loader) : m_loader(loader)
    {
    m_range = loader.GetContentId().ComputeRange(GetTree().GetRange(), Is2d());
    if (m_range.IsPoint() || m_range.IsEmpty())
        {
        m_tolerance = 0.0;
        }
    else
        {
        double diagDist = Is3d() ? m_range.DiagonalDistance() : m_range.DiagonalDistanceXY();
        m_tolerance = diagDist / (s_minToleranceRatio * GetSizeMultiplier());
        BeAssert(0.0 != m_tolerance);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryLoader::GenerateGeometry()
    {
    if (m_range.IsPoint() || m_range.IsEmpty())
        return true;

    auto model = GetTree().FetchModel();
    if (model.IsNull() || DgnDbStatus::Success != model->FillRangeIndex())
        return false;

    uint32_t maxFeatures = s_hardMaxFeaturesPerTile; // Note: Element != Feature - could have multiple features per element due to differing subcategories/classes within GeometryStream
    maxFeatures = std::min(maxFeatures, GetRenderSystem()._GetMaxFeaturesPerBatch());

    auto tolerance = GetTolerance();
    double minRangeDiagonalSq = s_minRangeBoxSize * tolerance;
    minRangeDiagonalSq *= minRangeDiagonalSq;

    IFacetOptionsPtr facetOptions = Geometry::CreateFacetOptions(tolerance, true);
    facetOptions->SetHideSmoothEdgesWhenGeneratingNormals(false); // We'll do this ourselves when generating meshes - This will turn on sheet edges that should be hidden (Pug.dgn).

    Transform transformFromDgn;
    transformFromDgn.InverseOf(GetTree().GetLocation());

    SubRanges subRanges(GetTileRange(), Is2d());

    // ###TODO: Do not populate the element collection up front - just traverse range index one element at a time
    // (We chiefly did the up-front collection previously in order to support partial tiles - process biggest elements first)
    DRange3d dgnRange = GetDgnRange();
    ElementCollector elementCollector(dgnRange, *model->GetRangeIndex(), minRangeDiagonalSq, m_loader, maxFeatures, transformFromDgn, subRanges);

    if (IsCanceled())
        return false;

    m_metadata.m_tolerance = tolerance;
    m_metadata.m_numElementsIncluded = static_cast<uint32_t>(elementCollector.GetEntries().size());
    GeometryList geometryList;
    if (elementCollector.AnySkipped())
        {
        geometryList.MarkIncomplete();
        m_metadata.m_numElementsExcluded = static_cast<uint32_t>(elementCollector.GetNumSkipped());
        }

    TileContext tileContext(geometryList, *this, dgnRange, *facetOptions, transformFromDgn, tolerance, subRanges);
    ElementMeshGenerator meshGenerator(*this, GeometryOptions());

    if (IsCanceled())
        return false;

    for (ElementCollector::Entries::const_iterator elementIter = elementCollector.GetEntries().begin(); elementCollector.GetEntries().end() != elementIter; ++elementIter)
        {
        // Collect geometry from this element
        geometryList.clear();
        auto const& entry = *elementIter;
        try
            {
            tileContext.ProcessElement(entry.second, entry.first);
            if (tileContext.GetGeometryCount() >= elementCollector.GetMaxElements())
                {
                tileContext.TruncateGeometryList(elementCollector.GetMaxElements());
                break;
                }

            // Convert this element's geometry to meshes
            for (auto const& geom : geometryList)
                meshGenerator.AddMeshes(*geom, true);

            if (IsCanceled())
                return false;

            if (meshGenerator.DidDecimation())
                geometryList.MarkIncomplete();
            }
        catch (...)
            {
            // The only observed exceptions stem from severe Parasolid errors. Whatever went wrong, ignore this element and continue with next.
            }
        }

    meshGenerator.AddDeferredGlyphMeshes(GetRenderSystem());
    if (IsCanceled())
        return false;

    tileContext.AddSharedGeometry(geometryList, meshGenerator, subRanges);
    if (IsCanceled())
        return false;

    // Facet all geometry
    m_geometry.Meshes() = meshGenerator.GetMeshes();
    m_metadata.m_contentRange.Extend(meshGenerator.GetContentRange());
    if (!geometryList.IsComplete())
        {
        // ###TODO: Some redundancy here...
        m_geometry.MarkIncomplete();
        m_metadata.m_flags |= Content::Flags::Incomplete;
        }

    if (geometryList.ContainsCurves())
        {
        m_geometry.MarkCurved();
        m_metadata.m_flags |= Content::Flags::ContainsCurves;
        }

    m_metadata.m_emptySubRanges = subRanges.ToBitfield();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshGenerator::MeshGenerator(GeometryLoaderCR loader, double tolerance, GeometryOptionsCR options, DRange3dCR range)
  : m_loader(loader), m_options(options),
    m_builders(tolerance, nullptr, range, loader.Is2d())
    {
    SetDgnDb(loader.GetDgnDb());
    m_is3dView = loader.Is3d();
    SetViewFlags(TileContext::GetDefaultViewFlags());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshBuilderR MeshGenerator::GetMeshBuilder(MeshBuilderKey const& key, uint32_t numVertices)
    {
    return m_builders.GetMeshBuilder(key, numVertices);
    }

// #define DEBUG_DUMP_TEXTURE_ATLAS_DIR L"E:\\texture_atlas\\"
#if defined(DEBUG_DUMP_TEXTURE_ATLAS_DIR)
static void writeAtlasToImageFile(Byte const* data, uint32_t width, uint32_t height, int32_t bytesPerPixel)
    {
    bool            rgba = 4 == bytesPerPixel;
    Image           image(width, height, ByteStream(data, width * height * bytesPerPixel), (rgba) ? Image::Format::Rgba : Image::Format::Rgb);
    ImageSource     imageSource(image, rgba ? ImageSource::Format::Png: ImageSource::Format::Jpeg);

    static BeAtomic<uint32_t> s_atlasIndex;
    WPrintfString filename(L"%lsatlas_%u.png", DEBUG_DUMP_TEXTURE_ATLAS_DIR, s_atlasIndex.IncrementAtomicPre());
    BeFile file;
    file.Create(filename.c_str(), true);
    file.Write(nullptr, imageSource.GetByteStream().GetData(), imageSource.GetByteStream().GetSize());
    file.Close();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void* DeferredGlyph::GetKey()
    {
    return static_cast<void*>(m_polyface.m_glyphImage);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void GlyphAtlas::CalculateSize()
    {
    m_numGlyphsInX = m_numGlyphs >= m_maxAtlasGlyphsDim ? m_maxAtlasGlyphsDim : m_numGlyphs;
    if (!GlyphAtlas::IsPowerOfTwo(m_numGlyphsInX))
        m_numGlyphsInX = GlyphAtlas::NextHighestPowerOfTwo(m_numGlyphsInX);

    m_numGlyphsInY = static_cast<uint32_t>(ceil(static_cast<double>(m_numGlyphs) / m_maxAtlasGlyphsDim));
    if (!GlyphAtlas::IsPowerOfTwo(m_numGlyphsInY))
        m_numGlyphsInY = GlyphAtlas::NextHighestPowerOfTwo(m_numGlyphsInY);

    m_numPixelsInX = m_numGlyphsInX * m_glyphTotalSizeInPixels;
    m_numPixelsInY = m_numGlyphsInY * m_glyphTotalSizeInPixels;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GlyphAtlas::GlyphAtlas(uint32_t index, uint32_t numGlyphs) : m_index(index), m_numGlyphs(numGlyphs)
    {
    CalculateSize();
    m_atlasBytes.Resize(m_numPixelsInX * m_numPixelsInY * 4); // reserve space for all potential glyph
    memset(m_atlasBytes.GetDataP(), 0, m_atlasBytes.GetSize());
    m_availGlyphSlotX = m_availGlyphSlotY = m_glyphHalfPaddingInPixels;
    m_atlasTexture = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GlyphAtlas::GlyphAtlas(GlyphAtlas&& other)
    {
    m_index = other.m_index;
    m_numGlyphs = other.m_numGlyphs;
    m_numGlyphsInX = other.m_numGlyphsInX;
    m_numGlyphsInY = other.m_numGlyphsInY;
    m_numPixelsInX = other.m_numPixelsInX;
    m_numPixelsInY = other.m_numPixelsInY;
    m_availGlyphSlotX = other.m_availGlyphSlotX;
    m_availGlyphSlotY = other.m_availGlyphSlotY;
    m_atlasBytes = std::move(other.m_atlasBytes);
    m_atlasTexture = other.m_atlasTexture;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GlyphAtlas& GlyphAtlas::operator=(GlyphAtlas&& other)
    {
    if (&other == this)
        return *this;

    m_index = other.m_index;
    m_numGlyphs = other.m_numGlyphs;
    m_numGlyphsInX = other.m_numGlyphsInX;
    m_numGlyphsInY = other.m_numGlyphsInY;
    m_numPixelsInX = other.m_numPixelsInX;
    m_numPixelsInY = other.m_numPixelsInY;
    m_availGlyphSlotX = other.m_availGlyphSlotX;
    m_availGlyphSlotY = other.m_availGlyphSlotY;
    m_atlasBytes = std::move(other.m_atlasBytes);
    m_atlasTexture = other.m_atlasTexture;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool GlyphAtlas::AddGlyph(DeferredGlyph& glyph, GlyphLocation& loc)
    {
    if (m_availGlyphSlotY >= m_numPixelsInY)
        return false; // this atlas is already filled to capacity

    Render::Image* glyphImage = glyph.m_polyface.m_glyphImage;
    ByteStream glyphBytes = glyphImage->GetByteStream();

    // copy unique glyph to next available location in the atlas
    for (uint32_t glyphY = 0; glyphY < glyphImage->GetHeight(); glyphY++)
        {
        for (uint32_t glyphX = 0; glyphX < glyphImage->GetWidth(); glyphX++)
            {
            size_t atlasIndex = (m_availGlyphSlotY + glyphY) * m_numPixelsInX * 4 + (m_availGlyphSlotX + glyphX) * 4;
            size_t glyphIndex = glyphY * glyphImage->GetWidth() * 4 + glyphX * 4;
            m_atlasBytes[atlasIndex] = glyphBytes[glyphIndex];
            m_atlasBytes[atlasIndex + 1] = glyphBytes[glyphIndex + 1];
            m_atlasBytes[atlasIndex + 2] = glyphBytes[glyphIndex + 2];
            m_atlasBytes[atlasIndex + 3] = glyphBytes[glyphIndex + 3];
            }
        }

    loc.m_atlasSlotX = m_availGlyphSlotX;
    loc.m_atlasSlotY = m_availGlyphSlotY;

    // calculate next available slot for a glyph
    m_availGlyphSlotX += m_glyphTotalSizeInPixels;
    if (m_availGlyphSlotX >= m_numPixelsInX)
        {
        m_availGlyphSlotX = m_glyphHalfPaddingInPixels;
        m_availGlyphSlotY += m_glyphTotalSizeInPixels;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void GlyphAtlas::GetUVCoords(Render::Image* glyphImage, const GlyphLocation& loc, DPoint2d uvs[2])
    {
    uvs[0].x = loc.m_atlasSlotX / static_cast<double>(m_numPixelsInX);
    uvs[0].y = loc.m_atlasSlotY / static_cast<double>(m_numPixelsInY);
    uvs[1].x = (loc.m_atlasSlotX + glyphImage->GetWidth()) / static_cast<double>(m_numPixelsInX);
    uvs[1].y = (loc.m_atlasSlotY + glyphImage->GetHeight()) / static_cast<double>(m_numPixelsInY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TexturePtr GlyphAtlas::GetTexture(Render::System& renderSystem, DgnDbP db)
    {
    if (!m_atlasTexture.IsNull())
        return m_atlasTexture;

    Render::Image atlasImage(m_numPixelsInX, m_numPixelsInY, std::move(m_atlasBytes), Render::Image::Format::Rgba);

#if defined(DEBUG_DUMP_TEXTURE_ATLAS_DIR)
    writeAtlasToImageFile(atlasImage.GetByteStream().data(), atlasImage.GetWidth(), atlasImage.GetHeight(), atlasImage.GetBytesPerPixel());
#endif

    // create the texture from the atlas image
    Utf8PrintfString name("gta%u", m_index);
    TextureKey key(name);
    Texture::CreateParams params(key);
    params.m_isGlyph = true;
    m_atlasTexture = renderSystem._CreateTexture(atlasImage, *db, params);

    return m_atlasTexture;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GlyphAtlasManager::GlyphAtlasManager(uint32_t numGlyphs) : m_numGlyphs(numGlyphs)
    {
    const uint32_t maxGlyphsInAtlas = GlyphAtlas::GetMaxGlyphsInAtlas();
    m_numAtlases = static_cast<uint32_t>(ceil(m_numGlyphs / static_cast<double>(maxGlyphsInAtlas)));
    uint32_t i = 0;
    for (; i < m_numAtlases - 1; i++)
        m_atlases.emplace_back(i, maxGlyphsInAtlas);

    m_atlases.emplace_back(i, m_numGlyphs - maxGlyphsInAtlas * (m_numAtlases - 1));
    m_curAtlasNdx = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void GlyphAtlasManager::AddGlyph(DeferredGlyph& glyph)
    {
    void* key = glyph.GetKey();
    if (m_glyphLocations.end() != m_glyphLocations.find(key))
        return; // glyph already exists in an atlas

    GlyphLocation loc;
    if (!m_atlases[m_curAtlasNdx].AddGlyph(glyph, loc))
        {
        m_curAtlasNdx++;
        m_atlases[m_curAtlasNdx].AddGlyph(glyph, loc);
        }
    loc.m_atlasNdx = m_curAtlasNdx;
    m_glyphLocations.insert(std::pair<void*,GlyphLocation>(key, loc));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
GlyphAtlas& GlyphAtlasManager::GetAtlasAndLocationForGlyph(DeferredGlyph& glyph, GlyphLocation& loc)
    {
    void* key = glyph.GetKey();
    std::map<void*,GlyphLocation>::iterator itr = m_glyphLocations.find(key);
    loc = itr->second;
    return m_atlases[loc.m_atlasNdx];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementMeshGenerator::AddDeferredGlyphMeshes(Render::System& renderSystem)
    {
    uint32_t numGlyphs = static_cast<uint32_t>(m_deferredGlyphs.size());
    if (0 == numGlyphs)
        return;

    GlyphAtlasManager atlasManager(static_cast<uint32_t>(m_uniqueGlyphKeys.size()));
    for (auto& deferredGlyph : m_deferredGlyphs)
        atlasManager.AddGlyph(deferredGlyph);

    // add the polyfaces with appropriate UV coordinates
    for (auto& deferredGlyph : m_deferredGlyphs)
        {
        GlyphLocation loc;
        GlyphAtlas& atlas = atlasManager.GetAtlasAndLocationForGlyph(deferredGlyph, loc);

        // override texture
        TexturePtr tex = atlas.GetTexture(renderSystem, m_dgndb);
        deferredGlyph.m_polyface.m_displayParams = deferredGlyph.m_polyface.m_displayParams->CloneForRasterText(*tex);

        // override uvs
        DPoint2d uvs[2];
        atlas.GetUVCoords(deferredGlyph.m_polyface.m_glyphImage, loc, uvs);
        auto& params = deferredGlyph.m_polyface.m_polyface->Param();
        for (auto& param : params)
            {
            param.y = param.y < 0.5 ? uvs[0].x : uvs[1].x;
            param.x = param.x < 0.5 ? uvs[0].y : uvs[1].y;
            }

        AddPolyface(deferredGlyph.m_polyface, deferredGlyph.m_elemId, deferredGlyph.m_rangePixels, deferredGlyph.m_isContained, false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddMeshes(GeometryList const& geometries, bool doRangeTest)
    {
    for (auto& geom : geometries)
        {
        if (m_loader.IsCanceled())
            break;

        AddMeshes(*geom, doRangeTest);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddMeshes(GeometryR geom, bool doRangeTest)
    {
    DRange3dCR geomRange = geom.GetTileRange();
    double rangePixels = (m_loader.Is3d() ? geomRange.DiagonalDistance() : geomRange.DiagonalDistanceXY()) / GetTolerance();
    if (rangePixels < s_minRangeBoxSize && 0.0 < geomRange.DiagonalDistance()) // ###TODO_ELEMENT_TILE: single point primitives have an empty range...
        return;   // ###TODO: -- Produce an artifact from optimized bounding box to approximate from range.

    bool isContained = !doRangeTest || geomRange.IsContained(GetTileRange());

    auto polyfaces = geom.GetPolyfaces(GetTolerance(), m_options.m_normalMode, *this);
    auto strokes = geom.GetStrokes(GetTolerance(), *this);
    m_loader.GetLoader().Preprocess(polyfaces, strokes);

    AddStrokes(strokes, geom.GetEntityId(), rangePixels, isContained);
    AddPolyfaces(polyfaces, geom.GetEntityId(), rangePixels, isContained);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddPolyfaces(PolyfaceList& polyfaces, DgnElementId elemId, double rangePixels, bool isContained)
    {
    for (auto& polyface : polyfaces)
        AddPolyface(polyface, elemId, rangePixels, isContained);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementMeshGenerator::AddDeferredPolyface(Polyface& tilePolyface, DgnElementId elemId, double rangePixels, bool isContained)
    {
    m_deferredGlyphs.push_back(DeferredGlyph(tilePolyface, elemId, rangePixels, isContained)); // defer processing these until we are finished so we can make texture atlas of glyphs
    void* key = m_deferredGlyphs.back().GetKey();
    if (m_uniqueGlyphKeys.find(key) == m_uniqueGlyphKeys.end())
        m_uniqueGlyphKeys.insert(key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddPolyface(Polyface& tilePolyface, DgnElementId elemId, double rangePixels, bool isContained, bool doDefer)
    {
    if (nullptr != tilePolyface.m_glyphImage && doDefer)
        {
        AddDeferredPolyface(tilePolyface, elemId, rangePixels, isContained);
        return;
        }

    // TFS#817210
    PolyfaceHeaderPtr polyface = tilePolyface.m_polyface.get();
    if (polyface.IsNull() || 0 == polyface->GetPointIndexCount())
        return;

    PolyfaceHeaderPtr decimated = tryDecimate(tilePolyface);
    if (decimated.IsValid())
        {
        MarkDecimated();
        polyface = decimated;
        }

    auto edgeOptions = (!GetOmitEdges() && tilePolyface.m_displayEdges) ? MeshEdgeCreationOptions::DefaultEdges : MeshEdgeCreationOptions::NoEdges;

    MeshEdgeCreationOptions edges(edgeOptions);
    bool                    isPlanar = tilePolyface.m_isPlanar;
    DisplayParamsCPtr       displayParams = &tilePolyface.GetDisplayParams();

    if (isContained)
        AddClippedPolyface(*polyface, elemId, *displayParams, edges, isPlanar);
    else
        ClipAndAddPolyface(*polyface, elemId, *displayParams, edges, isPlanar);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementMeshGenerator::ClipAndAddPolyface(PolyfaceHeaderR polyface, DgnElementId elemId, DisplayParamsCR displayParams, MeshEdgeCreationOptions const& edgeOptions, bool isPlanar)
    {
    TileRangeClipOutput clipOutput;
    polyface.ClipToRange(GetTileRange(), clipOutput, false);
    for (auto& clipped : clipOutput.m_output)
        AddClippedPolyface(*clipped, elemId, displayParams, edgeOptions, isPlanar);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/19
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void addClippedPolyface(MeshBuilderR builder, PolyfaceQueryCR polyface, MeshEdgeCreationOptions edgeOptions, FeatureCR feature, DisplayParamsCR displayParams, DgnDbR db, TransformCP transformToDgn, T extendContentRange)
    {
    bool anyContributed = false;
    uint32_t fillColor = displayParams.GetFillColor();
    bool hasNormals = nullptr != polyface.GetNormalCP();
    bool hasTexture = displayParams.IsTextured();
    TextureMappingCR texture = displayParams.GetSurfaceMaterial().GetTextureMapping();
    uint8_t materialIndex = builder.GetMaterialIndex(displayParams.GetSurfaceMaterial().GetMaterial());

    builder.BeginPolyface(polyface, edgeOptions);

    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(polyface); visitor->AdvanceToNextFace(); /**/)
        {
        anyContributed = true;
        builder.AddFromPolyfaceVisitor(*visitor, texture, db, feature, hasTexture, fillColor, hasNormals, materialIndex, transformToDgn);
        extendContentRange(visitor->Point());
        }

    builder.EndPolyface();

    if (anyContributed)
        {
        // NB: The mesh's display params contain a fill color, which is used by the tri mesh primitive if the color table is empty (uniform)
        // But each polyface's display params may have a different fill color.
        // If a polyface contributes no vertices, we may end up incorrectly using its fill color for the primitive
        // Make sure the mesh's display params match one (any) mesh which actually contributed vertices, so that if the result is a uniform color,
        // we will use the fill color of the (only) mesh which contributed.
        builder.SetDisplayParams(displayParams);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddClippedPolyface(PolyfaceQueryCR polyface, DgnElementId elemId, DisplayParamsCR displayParams, MeshEdgeCreationOptions edgeOptions, bool isPlanar)
    {
    DgnDbR              db = m_loader.GetDgnDb();
    uint64_t            keyElementId = m_loader.GetLoader().GetNodeId(elemId); // Create separate primitives per element if classifying.
    MeshBuilderKey      key(displayParams, nullptr != polyface.GetNormalIndexCP(), Mesh::PrimitiveType::Mesh, isPlanar, keyElementId);
    MeshBuilderR        builder = GetMeshBuilder(key, static_cast<uint32_t>(polyface.GetPointCount()));
    TransformCR         tileLocation = m_loader.GetTree().GetLocation();

    auto extendRange = [&](bvector<DPoint3d> const& points) { ExtendContentRange(points); };
    addClippedPolyface(builder, polyface, edgeOptions, featureFromParams(elemId, displayParams), displayParams, db, &tileLocation, extendRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementMeshGenerator::ClipStrokes(StrokesR strokes) const
    {
    if (strokes.m_disjoint)
        ClipPoints(strokes);
    else
        strokes = ClipSegments(strokes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
Strokes ElementMeshGenerator::ClipSegments(StrokesCR input) const
    {
    BeAssert(!input.m_disjoint);

    Strokes output(*input.m_displayParams, false, input.m_isPlanar, input.m_decimationTolerance);
    output.m_strokes.reserve(input.m_strokes.size());

    for (auto const& inputStroke : input.m_strokes)
        {
        auto const& points = inputStroke.m_points;
        if (points.size() <= 1)
            continue;

        DPoint3d prevPt = points.front();
        bool prevOutside = !GetTileRange().IsContained(prevPt);
        if (!prevOutside)
            {
            output.m_strokes.push_back(Strokes::PointList(inputStroke.m_startDistance, inputStroke.m_canDecimate));
            output.m_strokes.back().m_points.push_back(prevPt);
            }

        double length = inputStroke.m_startDistance;        // Cumulative length along polyline
        for (size_t i = 1; i < points.size(); i++)
            {
            auto nextPt = points[i];
            bool nextOutside = !GetTileRange().IsContained(nextPt);
            DSegment3d clippedSegment;
            if (prevOutside || nextOutside)
                {
                double param0, param1;
                DSegment3d unclippedSegment = DSegment3d::From(prevPt, nextPt);
                if (!GetTileRange().IntersectBounded(param0, param1, clippedSegment, unclippedSegment))
                    {
                    // entire segment clipped
                    prevPt = nextPt;
                    continue;
                    }
                }

            DPoint3d startPt = prevOutside ? clippedSegment.point[0] : prevPt;
            DPoint3d endPt = nextOutside ? clippedSegment.point[1] : nextPt;

            if (prevOutside)
                {
                output.m_strokes.push_back(Strokes::PointList(length, inputStroke.m_canDecimate));
                output.m_strokes.back().m_points.push_back(startPt);
                }

            output.m_strokes.back().m_points.push_back(endPt);

            prevPt = nextPt;
            prevOutside = nextOutside;
            }

        BeAssert(output.m_strokes.empty() || 1 < output.m_strokes.back().m_points.size());
        }

    return output;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementMeshGenerator::ClipPoints(StrokesR strokes) const
    {
    BeAssert(strokes.m_disjoint);

    for (auto& stroke : strokes.m_strokes)
        {
        auto eraseAt = std::remove_if(stroke.m_points.begin(), stroke.m_points.end(), [&](DPoint3dCR pt) { return !GetTileRange().IsContained(pt); });
        if (stroke.m_points.end() != eraseAt)
            stroke.m_points.erase(eraseAt, stroke.m_points.end());
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddStrokes(StrokesList& strokes, DgnElementId elemId, double rangePixels, bool isContained)
    {
    for (auto& stroke : strokes)
        AddStrokes(stroke, elemId, rangePixels, isContained);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double MeshGenerator::DecimateStrokes(StrokesR strokesOut, StrokesR strokesIn) // returns percentage of total point reduction
    {
    size_t totalPointsIn = 0;
    size_t totalPointsOut = 0;
    for (auto& strokeIn : strokesIn.m_strokes)
        {
        bool doDecimate = strokeIn.m_canDecimate; // only decimate linestrings
        if (doDecimate)
            {
            bvector<DPoint3d> compressedPoints;
            DPoint3dOps::CompressByChordError(compressedPoints, strokeIn.m_points, strokesIn.m_decimationTolerance);

            totalPointsIn += strokeIn.m_points.size();
            totalPointsOut += compressedPoints.size();

            strokesOut.m_strokes.emplace_back(Strokes::PointList(std::move(compressedPoints), false));
            }
        }

    return 1.0 - static_cast<double>(totalPointsOut) / static_cast<double>(totalPointsIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/19
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void addClippedStrokes(MeshBuilderR builder, StrokesCR strokes, FeatureCR feature, uint32_t color, T extendContentRange)
    {
    auto minPoints = strokes.m_disjoint ? 0 : 1;
    for (auto& stroke : strokes.m_strokes)
        {
        if (stroke.m_points.size() > minPoints)
            {
            extendContentRange(stroke.m_points);
            builder.AddPolyline(stroke.m_points, feature, color, stroke.m_startDistance);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshGenerator::AddStrokes(StrokesR strokes, DgnElementId elemId, double rangePixels, bool isContained)
    {
    if (m_loader.IsCanceled())
        return;

    // Clip before attempting decimation (probably cheaper)
    if (!isContained)
        ClipStrokes(strokes);

    if (strokes.m_strokes.empty())
        return; // avoid potentially creating the builder below...

    DisplayParamsCR displayParams = strokes.GetDisplayParams();

    // Decimate if possible
    Strokes* strokesToAdd = &strokes;
    Strokes decimatedStrokes(*strokes.m_displayParams, strokes.m_disjoint, strokes.m_isPlanar, strokes.m_decimationTolerance);
    if (tryDecimate(decimatedStrokes, strokes))
        {
        strokesToAdd = &decimatedStrokes;
        m_didDecimate = true;
        }

    MeshBuilderKey key(displayParams, false, strokesToAdd->m_disjoint ? Mesh::PrimitiveType::Point : Mesh::PrimitiveType::Polyline, strokesToAdd->m_isPlanar);
    MeshBuilderR builder = GetMeshBuilder(key, strokesToAdd->ComputePointCount());

    uint32_t fillColor = displayParams.GetLineColor();
    addClippedStrokes(builder, *strokesToAdd, featureFromParams(elemId, displayParams), fillColor, [&](bvector<DPoint3d> const& pts) { ExtendContentRange(pts); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileBuilder::TileBuilder(TileContext& context, DgnElementId elemId, double rangeDiagonalSquared, CreateParams const& params)
    : GeometryListBuilder(context.GetRenderSystemR(), params, elemId, context.GetTransformFromDgn()), m_context(context), m_rangeDiagonalSquared(rangeDiagonalSquared)
    {
    SetCheckGlyphBoxes(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileBuilder::TileBuilder(TileContext& context, DRange3dCR range)
    : GeometryListBuilder(context.GetRenderSystemR(), CreateParams::Scene(context.GetDgnDb())), m_context(context),
    m_rangeDiagonalSquared(context.Is3d() ? range.low.DistanceSquared(range.high) : range.low.DistanceSquaredXY(range.high))
    {
    // for TileSubGraphic...
    SetCheckGlyphBoxes(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileBuilder::ReInitialize(DgnElementId elemId, double rangeDiagonalSquared, TransformCR localToWorld)
    {
    GeometryListBuilder::ReInitialize(localToWorld, m_context.GetTransformFromDgn(), elemId);
    m_rangeDiagonalSquared = rangeDiagonalSquared;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileBuilder::ReInitialize(DRange3dCR range, TransformCR geomToOrigin)
    {
    GeometryListBuilder::ReInitialize(Transform::FromIdentity(), geomToOrigin);
    m_rangeDiagonalSquared = m_context.Is3d() ? range.low.DistanceSquared(range.high) : range.low.DistanceSquaredXY(range.high);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileBuilder::_AddPolyface(PolyfaceQueryCR geom, bool filled)
    {
    AddPolyfaceR(*geom.Clone(), filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileBuilder::_AddPolyfaceR(PolyfaceHeaderR geom, bool filled)
    {
    Add(geom, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void TileBuilder::_AddSolidPrimitiveR(ISolidPrimitiveR primitive)
    {
    // NB: Cannot clip instances...
    // AddSolidPrimitiveGeom() returns false if the primitive should be added as normal (uninstanced) geometry.
    if (nullptr != GetCurrentClip() || !m_context.AddSolidPrimitiveGeom(primitive, GetLocalToWorldTransform(), GetMeshDisplayParams(false)))
        GeometryListBuilder::_AddSolidPrimitiveR(primitive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileBuilder::_AddSubGraphic(GraphicR mainGraphic, TransformCR subToGraphic, GraphicParamsCR params, ClipVectorCP clip)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicBuilderPtr TileBuilder::_CreateSubGraphic(TransformCR tf, ClipVectorCP clip) const
    {
    CreateParams params = GetCreateParams().SubGraphic(Transform::FromProduct(GetLocalToWorldTransform(), tf));
    TileBuilderPtr subGf = new TileBuilder(m_context, GetElementId(), m_rangeDiagonalSquared, params);
    subGf->ActivateGraphicParams(GetGraphicParams(), GetGeometryParams());

    if (nullptr != clip)
        {
        ClipVectorPtr tClip = clip->Clone(&GetLocalToWorldTransform());

        Transform tileTransform;
        tileTransform.InverseOf(m_context.GetTree().GetLocation());
        tClip->TransformInPlace(tileTransform);

        subGf->SetCurrentClip(tClip.get());
        }

    return subGf.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr TileBuilder::_FinishGraphic(GeometryAccumulatorR accum)
    {
    BeAssert(nullptr == GetGeometryParams() || GetGeometryParams()->GetSubCategoryId().IsValid());
    return m_context.FinishGraphic(accum, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileBuilder::_WantStrokeLineStyle(LineStyleSymbCR lsSymb, IFacetOptionsPtr& options)
    {
    if (!lsSymb.GetUseStroker())
        return false;

    // We need to stroke if either the stroke length or width exceeds tolerance...
    ILineStyleCP        lineStyle;
    double              maxDimension = (nullptr == (lineStyle = lsSymb.GetILineStyle())) ? 0.0 : lsSymb.GetScale() * std::max(lineStyle->GetMaxWidth(), lineStyle->GetLength());
    constexpr double    s_strokeLineStylePixels = 5.0;      // Stroke if max dimension exceeds 5 pixels.

    if (maxDimension > s_strokeLineStylePixels * m_context.GetTolerance())
        {
        options = &m_context.GetFacetOptions();
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileSubGraphic::TileSubGraphic(TileContext& context, DgnGeometryPartCP part, bool hasPartSymb, PartGeom::KeyCR key)
  : TileBuilder(context, nullptr != part ? static_cast<DRange3d>(part->GetBoundingBox()) : DRange3d::NullRange()),
    m_input(part), m_hasSymbologyChanges(hasPartSymb), m_key(key)
    {
    SetCheckGlyphBoxes(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileSubGraphic::ReInitialize(DgnGeometryPartCR part, bool hasPartSymb, PartGeom::KeyCR key)
    {
    DRange3d range = part.GetBoundingBox();
    DPoint3d centroid = DPoint3d::FromInterpolate(range.low, 0.5, range.high);
    centroid.Negate();
    Transform geomToOrigin = Transform::From(centroid);
    geomToOrigin.Multiply(range, range);

    TileBuilder::ReInitialize(range, geomToOrigin);
    m_input = &part;
    m_key = key;
    m_hasSymbologyChanges = hasPartSymb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr TileSubGraphic::_FinishGraphic(GeometryAccumulatorR accum)
    {
    return m_context.FinishSubGraphic(accum, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileContext::TileContext(GeometryList& geometries, GeometryLoaderCR loader, DRange3dCR range, IFacetOptionsR facetOptions, TransformCR transformFromDgn, double tileTolerance, double rangeTolerance, SubRangesR subRanges)
  : m_geometries (geometries), m_facetOptions(facetOptions), m_loader(loader), m_range(range), m_transformFromDgn(transformFromDgn),
    m_tolerance(tileTolerance), m_statement(loader.GetDgnDb().GetCachedStatement(loader.Is3d() ? GeometrySelector3d::GetSql() : GeometrySelector2d::GetSql())),
    m_finishedGraphic(new Graphic(loader.GetDgnDb())), m_subRanges(subRanges)
    {
    static const double s_minTextBoxSize = 1.0;     // Below this ratio to tolerance  text is rendered as box.

    m_minRangeDiagonalSquared = s_minRangeBoxSize * rangeTolerance;
    m_minRangeDiagonalSquared *= m_minRangeDiagonalSquared;
    m_minTextBoxSize  = s_minTextBoxSize * rangeTolerance;
    GetTransformFromDgn().Multiply (m_tileRange, m_range);

    SetDgnDb(loader.GetDgnDb());
    m_is3dView = loader.Is3d();
    SetViewFlags(GetDefaultViewFlags());

    // These are reused...
    m_tileBuilder = new TileBuilder(*this, DgnElementId(), 0.0, GraphicBuilder::CreateParams::Scene(loader.GetDgnDb()));
    m_subGraphic = new TileSubGraphic(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
PartGeomPtr TileContext::GeneratePartGeom(PartGeom::KeyCR key, DgnGeometryPartCR geomPart, GeometryParamsR geomParams)
    {
    GeometryStreamIO::Collection collection(geomPart.GetGeometryStream().GetData(), geomPart.GetGeometryStream().GetSize());
    bool hasPartSymb = hasSymbologyChanges(collection);

    TileSubGraphicPtr partBuilder(m_subGraphic);
    if (partBuilder->GetRefCount() > 2)
        partBuilder = new TileSubGraphic(*this, geomPart, hasPartSymb, key);
    else
        partBuilder->ReInitialize(geomPart, hasPartSymb, key);

    collection.Draw(*partBuilder, *this, geomParams, false, &geomPart);

    partBuilder->Finish();
    BeAssert(partBuilder->GetRefCount() <= 2);
    return partBuilder->GetOutput();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TileContext::AddPartGeom(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams, GraphicParamsR graphicParams)
    {
    DisplayParamsCR displayParams = m_tileBuilder->GetDisplayParamsCache().GetForMesh(graphicParams, &geomParams, false);
    double tolerance = PartGeom::ComputeTolerance(subToGraphic, GetTolerance());
    PartGeom::Key partKey(partId, tolerance, displayParams);

    PartGeomPtr tilePartGeom;
    auto partIter = m_geomParts.find(partKey);
    if (m_geomParts.end() == partIter)
        {
        DgnGeometryPartCPtr geomPart = GetDgnDb().Elements().Get<DgnGeometryPart>(partId);
        if (geomPart.IsValid())
            tilePartGeom = GeneratePartGeom(partKey, *geomPart, geomParams);

        // ###TODO_INSTANCING: Instance symbology gets baked into part...
        // ###TODO_INSTANCING: For now, we do not instance the part if it contains any symbology of its own.
        // According to Brien the V8 converter will (almost) never produce parts containing symbology.
        // Exception for face-attached materials.
        // A bridge might produce parts containing symbology.
        if (tilePartGeom.IsValid() && tilePartGeom->IsInstanceable() && AllowInstancing())
            {
            m_geomParts.insert(tilePartGeom);
            }
        }
    else
        {
        tilePartGeom = *partIter;
        BeAssert(tilePartGeom.IsValid() && tilePartGeom->IsInstanceable());
        }

    if (tilePartGeom.IsNull())
        return;

    DRange3d range;
    Transform originToPart = Transform::From(tilePartGeom->GetTranslation());
    Transform partToSub = Transform::FromProduct(subToGraphic, originToPart);
    Transform partToWorld = Transform::FromProduct(graphic.GetLocalToWorldTransform(), partToSub);
    Transform tf = Transform::FromProduct(GetTransformFromDgn(), partToWorld);
    tf.Multiply(range, tilePartGeom->GetRange());

    // UVs are assigned based on world coordinates - incompatible with instancing.
    bool hasElevationDrape = TextureMapping::Mode::ElevationDrape == displayParams.GetSurfaceMaterial().GetTextureMapping().GetParams().m_mapMode;

    // We can't clip instances - they must be wholly contained within the tile's volume.
    // We also don't support non-uniform scale - although we could - but bim02/imodel02 explicitly do not allow it so the check should always pass.
    double unusedScale;
    bool isInstanceable = AllowInstancing() && nullptr == graphic.GetCurrentClip() && tilePartGeom->IsInstanceable() && range.IsContained(m_tileRange) && graphic.GetLocalToWorldTransform().IsRigidScale(unusedScale) && !hasElevationDrape;
    if (isInstanceable)
        {
        tilePartGeom->AddInstance(tf, displayParams, GetCurrentElementId());
        }
    else
        {
        BeAssert(nullptr != dynamic_cast<TileBuilderP>(&graphic));
        auto& parent = static_cast<TileBuilderR>(graphic);
        parent.Add(*Geometry::Create(*tilePartGeom, tf, range, GetCurrentElementId(), displayParams, GetDgnDb()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* Returns false to indicate we should add as uninstanced geometry (because not contained
* in tile range, or instancing not enabled).
* @bsimethod                                                    Paul.Connelly   03/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileContext::AddSolidPrimitiveGeom(ISolidPrimitiveR primitive, TransformCR localToWorld, DisplayParamsCR displayParams)
    {
    static bool s_instanceSolidPrimitives = true;
    if (!s_instanceSolidPrimitives || !AllowInstancing())
        return false;

    DRange3d partRange;
    if (!primitive.GetRange(partRange))
        return true;

    DRange3d instanceRange = partRange;
    auto transform = Transform::FromProduct(GetTransformFromDgn(), localToWorld);
    transform.Multiply(instanceRange, instanceRange);
    if (!instanceRange.IsContained(m_tileRange))
        {
        return false;
        }
    else if (BelowMinRange(instanceRange))
        {
        MarkIncomplete();
        return true;
        }

    SolidPrimitiveGeomPtr partGeom;
    SolidPrimitiveGeom::Key key(primitive, displayParams, partRange);
    auto bounds = m_solidPrimitives.equal_range(key);
    for (auto iter = bounds.first; iter != bounds.second; ++iter)
        {
        SolidPrimitiveGeomPtr thisPart = *iter;
        if (thisPart->GetKey().IsEquivalent(key))
            {
            partGeom = thisPart;
            break;
            }
        }

    if (partGeom.IsNull())
        {
        partGeom = SolidPrimitiveGeom::Create(key, partRange, GetDgnDb());
        m_solidPrimitives.insert(partGeom);
        }

    partGeom->AddInstance(transform, displayParams, GetCurrentElementId());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr TileContext::FinishGraphic(GeometryAccumulatorR accum, TileBuilder& builder)
    {
    for (auto& geom : accum.GetGeometries())
        PushGeometry(*geom);

    return m_finishedGraphic; // carries no useful info and is just going to be discarded...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr TileContext::FinishSubGraphic(GeometryAccumulatorR accum, TileSubGraphic& subGf)
    {
    DgnGeometryPartCR input = subGf.GetInput();

    TransformCR tf = accum.GetTransform();
    DRange3d range = input.GetBoundingBox();
    tf.Multiply(range, range);

    DPoint3d translation = tf.Origin();
    translation.Negate();

    // Create the PartGeom even if accum.GetGeometries().empty(), so that we can cache it to avoid reprocessing same part repeatedly
    subGf.SetOutput(*PartGeom::Create(subGf.GetPartKey(), range, accum.GetGeometries(), subGf.HasSymbologyChanges(), translation));

    return m_finishedGraphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileContext::ProcessElement(DgnElementId elemId, double rangeDiagonalSquared)
    {
    m_curElemId = elemId;
    m_curRangeDiagonalSquared = rangeDiagonalSquared;
    VisitElement(elemId, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileContext::PushGeometry(GeometryR geom)
    {
    auto const& range = geom.GetTileRange();
    m_subRanges.Add(range);
    if (BelowMinRange(range))
        MarkIncomplete();
    else
        m_geometries.push_back(geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::ViewFlags TileContext::GetDefaultViewFlags()
    {
    // We need to generate all the geometry - visibility of stuff like text, constructions, etc will be handled in shaders
    // Most default to 'on'
    Render::ViewFlags flags;
    flags.SetShowConstructions(true);
    return flags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void TileContext::_AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, GeometryParamsR geomParams)
    {
    GraphicParams graphicParams;
    _CookGeometryParams(geomParams, graphicParams);
    AddPartGeom(graphic, partId, subToGraphic, geomParams, graphicParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt TileContext::_VisitElement(DgnElementId elementId, bool allowLoad)
    {
    // Never load elements - but do use them if they're already loaded
    DgnElementCPtr el = GetDgnDb().Elements().FindLoadedElement(elementId);
    if (el.IsValid())
        {
        GeometrySourceCP geomElem = el->ToGeometrySource();
        return (nullptr == geomElem) ? ERROR : VisitGeometry(*geomElem);
        }

    // Load only the data we actually need for processing geometry
    // NB: The Step() below as well as each column access requires acquiring the sqlite mutex.
    // Prevent micro-contention by locking the db here
    // Note we do not use a mutex holder because we want to release the mutex before processing the geometry.
    // Also note we use a less-expensive non-recursive mutex for performance.
    GetTree().GetDbMutex().lock();
    StatusInt status = ERROR;
    auto& stmt = *m_statement;
    stmt.BindInt64(1, static_cast<int64_t>(elementId.GetValueUnchecked()));

    if (BeSQLite::BE_SQLITE_ROW == stmt.Step() && !IsValueNull(1))
        {
        auto geomSrcPtr = m_is3dView ? GeometrySelector3d::ExtractGeometrySource(stmt, GetDgnDb()) : GeometrySelector2d::ExtractGeometrySource(stmt, GetDgnDb());

        stmt.Reset();
        GetTree().GetDbMutex().unlock();

        if (nullptr != geomSrcPtr)
            status = VisitGeometry(*geomSrcPtr);
        }
    else
        {
        stmt.Reset();
        GetTree().GetDbMutex().unlock();
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr TileContext::_StrokeGeometry(GeometrySourceCR source, double pixelSize)
    {
    Render::GraphicPtr graphic = source.Draw(*this, pixelSize);
    return WasAborted() ? nullptr : graphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void TileContext::AddSharedGeometry(GeometryList& geometryList, ElementMeshGeneratorR meshGenerator, SubRanges& subRanges) const
    {
    InstanceableGeomBuckets buckets(meshGenerator, subRanges);

    auto process = [&](SharedGeomR shared)
        {
        if (!shared.IsComplete())
            geometryList.MarkIncomplete();

        if (shared.IsCurved())
            geometryList.MarkCurved();

        // NB: If !shared.IsWorthInstancing() - i.e., only 1 instance exists - this immediately adds as batched.
        buckets.Add(shared);

        return !meshGenerator.GetLoader().IsCanceled();
        };

    for (auto const& part : Parts())
        if (!process(*part))
            return;

    for (auto const& primitive : GetSolidPrimitives())
        if (!process(*primitive))
            return;

    buckets.ProduceMeshes();
    }

// #define CLONE_FOR_ADD_BATCHED
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceableGeom::AddBatched(ElementMeshGeneratorR meshGen, SubRangesR subRanges)
    {
    // Reverts the previous instance's transform when applying next instance's transform - eliminates the need
    // to clone+transform polyface/strokes for each instance.
    Transform invTransform = Transform::FromIdentity();
    DRange3d geomRange = ComputeRange();
    auto const& sharedGeom = GetSharedGeom();

    // For face-attached materials these may differ from SharedGeom's DisplayParams.
    // Either way, we need to apply any instance-specific overrides below.
    DisplayParamsCPtr baseDisplayParams = &GetDisplayParams();
    for (auto const& instance : sharedGeom.GetInstances())
        {
#if defined(CLONE_FOR_ADD_BATCHED)
        Transform instanceTransform = instance.GetTransform();
#else
        Transform instanceTransform = Transform::FromProduct(instance.GetTransform(), invTransform);
#endif
        invTransform.InverseOf(instance.GetTransform());

        DRange3d instanceRange;
        instance.GetTransform().Multiply(instanceRange, geomRange);
        subRanges.Add(instanceRange);

        DisplayParamsCPtr displayParams = sharedGeom.CloneDisplayParamsForInstance(*baseDisplayParams, instance.GetDisplayParams());
        AddBatched(meshGen, instanceTransform, *displayParams, instance.GetElementId());
        if (IsDecimated())
            meshGen.MarkDecimated();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceablePolyface::AddBatched(ElementMeshGeneratorR meshGen, TransformCR transform, DisplayParamsCR displayParams, DgnElementId elemId)
    {
#if defined(CLONE_FOR_ADD_BATCHED)
    Polyface pf = m_polyface;
    pf.m_polyface = pf.m_polyface->Clone();
    pf.Transform(transform);
    pf.m_displayParams = &displayParams;
    meshGen.AddPolyface(pf, elemId, 0.0, true, false);
#else
    Polyface pf = m_polyface;
    pf.Transform(transform);
    pf.m_displayParams = &displayParams;
    meshGen.AddPolyface(pf, elemId, 0.0, true, false);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceablePolyface::AddInstanced(MeshGeneratorR meshGen, MeshBuilderR builder, DRange3dR contentRange) const
    {
    auto extendRange = [&contentRange](bvector<DPoint3d> const& points) { contentRange.Extend(points); };
    auto edgeOptions = !meshGen.GetOmitEdges() && m_polyface.m_displayEdges ? MeshEdgeCreationOptions::DefaultEdges : MeshEdgeCreationOptions::NoEdges;
    DgnDbR db = meshGen.GetLoader().GetDgnDb();
    addClippedPolyface(builder, *m_polyface.m_polyface, edgeOptions, Feature(), GetDisplayParams(), db, nullptr, extendRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceableStrokes::AddBatched(ElementMeshGeneratorR meshGen, TransformCR transform, DisplayParamsCR displayParams, DgnElementId elemId)
    {
    m_strokes.Transform(transform);
    m_strokes.m_displayParams = &displayParams;
    meshGen.AddStrokes(m_strokes, elemId, 0.0, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceableStrokes::AddInstanced(MeshGeneratorR meshGen, MeshBuilderR builder, DRange3dR contentRange) const
    {
    auto extendRange = [&contentRange](bvector<DPoint3d> const& points) { contentRange.Extend(points); };
    addClippedStrokes(builder, m_strokes, Feature(), GetDisplayParams().GetLineColor(), extendRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/19
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceableGeom::AddInstanced(ElementMeshGeneratorR meshGen, SubRangesR subRanges)
    {
    auto key = GetMeshBuilderKey();
    SharedGeomCR geom = GetSharedGeom();
    auto tolerance = geom.GetTolerance(meshGen.GetTolerance());
    auto vertTol = tolerance * ToleranceRatio::Vertex();
    auto areaTol = tolerance * ToleranceRatio::FacetArea(); // ###TODO AFAICT this is vestigial and ratio is same as vertex anyway...

    MeshBuilderPtr builder = MeshBuilder::Create(key.GetDisplayParams(), vertTol, areaTol, nullptr, key.GetPrimitiveType(), geom.GetRange(), meshGen.GetLoader().Is2d(), key.IsPlanar(), key.GetNodeIndex(), nullptr);

    DRange3d contentRange = DRange3d::NullRange();
    AddInstanced(meshGen, *builder, contentRange);

    MeshP mesh = builder->GetMesh();
    if (nullptr == mesh)
        return;

    // When writing the Mesh to binary, will pull instances from SharedGeom.
    mesh->SetSharedGeom(geom);
    meshGen.AddSharedMesh(*mesh);

    DRange3d instancesContentRange = DRange3d::NullRange();
    for (auto const& instance : geom.GetInstances())
        {
        DRange3d range;
        instance.GetTransform().Multiply(range, contentRange);
        instancesContentRange.Extend(range);

        // Mark sub-ranges containing instanced geometry as non-empty
        subRanges.Add(range);

        // Ensure feature index allocated for instance
        meshGen.GetFeatureTable().GetIndex(instance.GetFeature());
        }

    // Ensure tile content range includes content range of each instance
    meshGen.ExtendContentRange(instancesContentRange);

    if (IsDecimated())
        meshGen.MarkDecimated();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
MeshList ElementMeshGenerator::GetMeshes()
    {
    MeshList meshes;
    m_builders.GetMeshes(meshes);

    // Do not allow vertices outside of this tile's range to expand its content range
    clipContentRangeToTileRange(m_contentRange, GetTileRange());

    if (m_loader.GetLoader().CompressMeshQuantization())
        for (auto& mesh : meshes)
            mesh->CompressVertexQuantization();

    meshes.insert(meshes.end(), m_sharedMeshes.begin(), m_sharedMeshes.end());

    meshes.m_features = std::move(m_featureTable);
    if (meshes.m_features.size() == 0 && meshes.size() != 0)
        BeAssert(false && "GetMeshes: empty feature table");

    return meshes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d SharedMeshGenerator::ComputeContentRange(SubRangesR subRanges) const
    {
    auto contentRange = DRange3d::NullRange();
    for (auto const& instance : m_geom.GetInstances())
        {
        DRange3d range;
        instance.GetTransform().Multiply(range, m_contentRange);
        contentRange.Extend(range);

        // Ensure feature index allocated for instance
        m_gen.GetFeatureTable().GetIndex(instance.GetFeature());

        // Ensure sub-ranges containing instanced geometry are marked non-empty
        subRanges.Add(range);
        }

    return contentRange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedMeshGenerator::AddMeshes(bvector<MeshPtr>& meshes)
    {
    MeshGenerator::AddMeshes(m_geom.GetGeometries(), false);
    m_builders.GetMeshes(meshes, m_geom);
    }

