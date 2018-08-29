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

END_UNNAMED_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContentId::ToString() const
    {
    Utf8String str;
    Utf8Char buf[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    uint64_t parts[] = { static_cast<uint64_t>(m_depth), m_i, m_j, m_k, 0 == static_cast<uint64_t>(m_mult ? 1 : m_mult) };
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

