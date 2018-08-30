/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Tile.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/Tile.h>
#include <numeric>
#include <DgnPlatform/RangeIndex.h>
#include <GeomJsonWireFormat/JsonUtils.h>
#include <numeric>
#include <inttypes.h>
#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif

USING_NAMESPACE_TILE

// Obsolete versions of table storing tile data
#define TABLE_NAME_TileTree3 "TileTree3"

// 4th version of this table: modified for iModelJs (imodel02 branch):
// Element tiles are now the only types of tiles produced and cached by the backend.
//  - Remove ContentType and Expires columns
//  - Add Metadata column, containing data formerly stored in the binary stream (geometry flags like 'is curved', zoom factor, etc)
#define TABLE_NAME_TileTree "TileTree4"

// Second version: Same primary key as tile data table.
#define TABLE_NAME_TileTreeCreateTime "TileTreeCreateTime2"

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
constexpr double s_maxLeafTolerance = 1.0; // the maximum tolerance at which we will stop subdividing tiles, regardless of # of elements contained or whether curved geometry exists.

#if defined (BENTLEYCONFIG_PARASOLID) 
static RefCountedPtr<PSolidThreadUtil::MainThreadMark> s_psolidMainThreadMark;
#endif

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

struct ClassificationTree;

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     08/18
//=======================================================================================
struct ClassificationLoader : Loader
{
    DEFINE_T_SUPER(Loader);
private:
    double m_offset; // NB: will be used in future?
    DRange3d m_range;

    bool SeparatePrimitivesById() const final { return true; }
    CurveVectorPtr Preprocess(CurveVectorR cv) const final { return &cv; }
    PolyfaceHeaderPtr Preprocess(PolyfaceHeaderR pf) const final;
public:
    ClassificationLoader(ClassificationTree& tree, ContentIdCR contentId);
};

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     08/18
//=======================================================================================
struct ClassificationTree : Tree
{
    DEFINE_T_SUPER(Tree);

    double m_classifierOffset; // NB: will be initialized+used in future?
    DRange3d m_classifierRange;

    ClassificationTree(GeometricModelCR model, TransformCR location, DRange3dCR range, Render::SystemR system)
        : T_Super(model, location, range, system)
        {
        Transform transformFromDgn;
        transformFromDgn.InverseOf(location);
        transformFromDgn.Multiply(m_classifierRange, model.GetDgnDb().GeoLocation().GetProjectExtents());
        }

    Utf8String _GetId() const final { Utf8String id("Classifier_"); id.append(T_Super::_GetId()); return id; }
    LoaderPtr CreateLoader(ContentIdCR contentId) final { return new ClassificationLoader(*this, contentId); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationLoader::ClassificationLoader(ClassificationTree& tree, ContentIdCR contentId) : T_Super(tree, contentId), m_offset(tree.m_classifierOffset), m_range(tree.m_classifierRange)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/18
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr ClassificationLoader::Preprocess(PolyfaceHeaderR pf) const
    {
    DPlane3d plane;
    if (pf.IsClosedPlanarRegion(plane))
        {
        DRay3d              ray = DRay3d::FromOriginAndVector(plane.origin, plane.normal);
        DRange1d            classifiedProjection = m_range.GetCornerRange(ray);
        PolyfaceHeaderPtr   extrudedPolyface = pf.ComputeOffset(PolyfaceHeader::OffsetOptions(), classifiedProjection.high, classifiedProjection.low);

        if (extrudedPolyface.IsValid())
            {
            extrudedPolyface->Triangulate();
            extrudedPolyface->BuildPerFaceNormals();
            return extrudedPolyface;
            }
        }

    return &pf;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static DRange3d bisectRange2d(DRange3dCR range, bool takeLow)
    {
    DVec3d diag = range.DiagonalVector();
    DRange3d subRange = range;

    double bisect;
    double* replace;
    if (diag.x > diag.y)
        {
        bisect = (range.low.x + range.high.x) / 2.0;
        replace = takeLow ? &subRange.high.x : &subRange.low.x;
        }
    else
        {
        bisect = (range.low.y + range.high.y) / 2.0;
        replace = takeLow ? &subRange.high.y : &subRange.low.y;
        }

    *replace = bisect;
    return subRange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
static DRange3d bisectRange(DRange3dCR range, bool takeLow)
    {
    DVec3d diag = range.DiagonalVector();
    DRange3d subRange = range;

    double bisect;
    double* replace;
    if (diag.x > diag.y && diag.x > diag.z)
        {
        bisect = (range.low.x + range.high.x) / 2.0;
        replace = takeLow ? &subRange.high.x : &subRange.low.x;
        }
    else if (diag.y > diag.z)
        {
        bisect = (range.low.y + range.high.y) / 2.0;
        replace = takeLow ? &subRange.high.y : &subRange.low.y;
        }
    else
        {
        bisect = (range.low.z + range.high.z) / 2.0;
        replace = takeLow ? &subRange.high.z : &subRange.low.z;
        }

    *replace = bisect;
    return subRange;
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
bool isElementCountLessThan(uint32_t threshold, RangeIndex::Tree& tree)
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
    return counter.m_count < threshold;
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContentId::ToString() const
    {
    Utf8String str;
    Utf8Char buf[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    uint64_t parts[] = { static_cast<uint64_t>(m_depth), m_i, m_j, m_k, static_cast<uint64_t>(m_mult) };
    uint32_t nSeparators = 0;
    for (auto part : parts)
        {
        BeStringUtilities::FormatUInt64(buf, BeInt64Id::ID_STRINGBUFFER_LENGTH, part, HexFormatOptions::None);
        str.append(nSeparators, '/');
        nSeparators = 1;
        str.append(buf);
        }

    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentId::FromString(Utf8CP str)
    {
    if (Utf8String::IsNullOrEmpty(str))
        return false;

    uint64_t i, j, k;
    uint8_t depth, mult;
    if (5 != BE_STRING_UTILITIES_UTF8_SSCANF(str, "%" SCNx8 "/%" SCNx64 "/%" SCNx64 "/%" SCNx64 "/%" SCNx8, &depth, &i, &j, &k, &mult))
        return false;

    *this = ContentId(depth, i, j, k, mult);
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

    return NodeId(depth - 1, m_i >> 2, m_j >> 2, m_k >> 2);
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

    // We should never subdivide along z for 2d tiles...###TODO Use a quad-tree for 2d, not an oct-tree.
    auto bisect = is2d ? bisectRange2d : bisectRange;
    DRange3d range = bisect(parentRange, lowI);
    range = bisect(range, lowJ);
    range = bisect(range, lowK);

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* Create the table to hold entries in this Cache
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Cache::_Prepare() const 
    {
    // When current tables were TileTreeCreateTime2 and TileTree3, we changed the file extension from .TileCache to .Tiles
    // So we will never encounter an existing .Tiles file containing previous versions of those tables.
    if (m_db.TableExists(TABLE_NAME_TileTree))
        {
        if (!ValidateData())
            {
            // The db schema is current, but the binary data format is not. Discard it.
            CachedStatementPtr stmt;
            m_db.GetCachedStatement(stmt, "DELETE FROM " TABLE_NAME_TileTreeCreateTime);
            if (BE_SQLITE_DONE != stmt->Step())
                {
                BeAssert(false && "Failed to delete contents of TileTreeCreateTime table");
                }

            m_db.GetCachedStatement(stmt, "DELETE FROM " TABLE_NAME_TileTree);
            if (BE_SQLITE_DONE != stmt->Step())
                {
                BeAssert(false && "Failed to delete contents of TileTree table");
                }
            }

        return SUCCESS;
        }
        
    // Drop leftover tables from previous versions
    m_db.DropTableIfExists(TABLE_NAME_TileTree3);

    // Create the tables
    if (!m_db.TableExists(TABLE_NAME_TileTreeCreateTime) && BE_SQLITE_OK != m_db.CreateTable(TABLE_NAME_TileTreeCreateTime, "TileId CHAR PRIMARY KEY,Created BIGINT"))
        return ERROR;

    return BE_SQLITE_OK == m_db.CreateTable(TABLE_NAME_TileTree,
        "TileId CHAR PRIMARY KEY,Data BLOB,DataSize BIGINT,Metadata TEXT") ? SUCCESS : ERROR;
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
Utf8CP Cache::GetCurrentVersion()
    {
    // Increment this when the binary tile format changes...
    // We changed the cache db schema shortly after creating imodel02 branch - so version history restarts at same time.
    // 0: Initial version following db schema change
    // 1: Do not set 'is leaf' if have size multiplier ('zoom factor'); add flag and value for size multiplier
    return "0";
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
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
TreePtr Tree::Create(GeometricModelR model, Render::SystemR system, TreeType type)
    {
    if (DgnDbStatus::Success != model.FillRangeIndex())
        return nullptr;

    // ###TODO: determine RefinementMode for root tile, store in Tree.
    // bool populateRootTile;
    DRange3d range;
    if (model.Is3dModel())
        {
        range = model.GetDgnDb().GeoLocation().GetProjectExtents();
        range = scaleSpatialRange(range);
        // populateRootTile = isElementCountLessThan(s_minElementsPerTile, *model.GetRangeIndex());
        }
    else
        {
        RangeAccumulator accum(range, model.Is2dModel());
        if (!accum.Accumulate(*model.GetRangeIndex()))
            range = DRange3d::From(DPoint3d::FromZero());

        auto sheet = model.ToSheetModel();
        if (nullptr != sheet)
            range.Extend(sheet->GetSheetExtents());

        // populateRootTile = accum.GetElementCount() < s_minElementsPerTile;
        }
    
    // Translate world coordinates to center of range in order to reduce precision errors
    DPoint3d centroid = DPoint3d::FromInterpolate(range.low, 0.5, range.high);
    Transform transform = Transform::From(centroid);

#if defined (BENTLEYCONFIG_PARASOLID)
    PSolidKernelManager::StartSession();

    if (s_psolidMainThreadMark.IsNull())
        s_psolidMainThreadMark = new PSolidThreadUtil::MainThreadMark();
#endif

    Transform rangeTransform;
    rangeTransform.InverseOf(transform);
    DRange3d tileRange;
    rangeTransform.Multiply(tileRange, range);

    return TreeType::Classifier == type ? new ClassificationTree(model, transform, tileRange, system) : new Tree(model, transform, tileRange, system);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Tree::Tree(GeometricModelCR model, TransformCR location, DRange3dCR range, Render::SystemR system)
    : m_db(model.GetDgnDb()), m_location(location), m_renderSystem(system), m_modelId(model.GetModelId()), m_is3d(model.Is3d()), m_cache(model.GetDgnDb().ElementTileCache())
    {
    // ###TODO: Cache changes to Tile::Cache...
    m_range.Extend(range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Tree::~Tree()
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

    json["id"] = _GetId();
    json["maxTilesToSkip"] = 1;
    json["tileScreenSize"] = s_tileScreenSize;
    JsonUtils::TransformToJson(json["location"], GetLocation());

    // ###TODO: root tile refinement mode...

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
ContentCPtr Tree::RequestContent(ContentIdCR contentId)
    {
    LoaderPtr loader;
    BeMutexHolder lock(m_cv.GetMutex());
    auto iter = m_activeLoads.find(contentId);
    if (iter == m_activeLoads.end())
        {
        // Load the tile content synchronously on this thread
        loader = CreateLoader(contentId);
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
Loader::Loader(TreeR tree, ContentIdCR contentId) : m_contentId(contentId), m_tree(tree), m_cacheKey(tree.ConstructCacheKey(contentId)),
    m_createTime(tree.FetchModel()->GetLastElementModifiedTime())
    {
    BeAssert(State::Loading == GetState()); // zero-initialized...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Loader::~Loader()
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Loader::IsExpired(uint64_t createTimeMillis) const
    {
    // ###TODO: For now assuming model has not been modified since loader was created...
    return createTimeMillis < m_createTime;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Loader::IsValidData(ByteStreamCR cacheData) const
    {
    // ###TODO: We used to verify the feature table in order to detect element updates which had been *undone* - the only sort of change
    // not detectable by checking GeometricModel::GetLastElementModifiedTime().
    // For now not concerned about that case - any other modification is handled by IsExpired().
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
        }

    SetState(state);
    BeAssert(!IsLoading());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Loader::State Loader::ReadFromCache()
    {
    BeAssert(IsLoading());
    auto cache = GetTree().GetCache();
    if (nullptr == cache)
        return State::Invalid;

    if (true)
        {
        RealityData::Cache::AccessLock lock(*cache); // block writes to cache while we're reading

        enum Column : int { Data, DataSize, Created, Rowid };
        CachedStatementPtr stmt;
        constexpr Utf8CP selectSql = "SELECT Data,DataSize,Created," TABLE_NAME_TileTree ".ROWID as TileRowId"
            " FROM " JOIN_TileTreeTables " WHERE " COLUMN_TileTree_TileId "=?";

        if (BE_SQLITE_OK != cache->GetDb().GetCachedStatement(stmt, selectSql))
            return State::Invalid;
        
        stmt->ClearBindings();
        stmt->BindText(1, m_cacheKey, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW != stmt->Step())
            return State::NotFound;

        uint64_t rowId = stmt->GetValueInt64(Column::Rowid);
        uint64_t createTime = stmt->GetValueInt64(Column::Created);
        if (IsExpired(createTime))
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

            m_content = new Content(std::move(bytes));
            }

        // We've loaded data from cache. Update timestamp.
        if (BE_SQLITE_OK == cache->GetDb().GetCachedStatement(stmt, "UPDATE " TABLE_NAME_TileTreeCreateTime " SET Created=? WHERE TileId=?"))
            {
            stmt->BindInt64(1, GetCreateTime());
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
    return State::NotFound; // ###TODO...
    }

