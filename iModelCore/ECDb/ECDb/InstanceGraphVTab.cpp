/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "InstanceGraphVTab.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

// =====================================================================================
// RelationsModule — Connect
// =====================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationsModule::Connect(DbVirtualTable*& out, Config& conf, int argc, const char* const* argv)
    {
    out = new RelationsTable(*this);
    conf.SetTag(Config::Tags::Innocuous);
    return BE_SQLITE_OK;
    }

// =====================================================================================
// RelationsTable — BestIndex
// =====================================================================================

//! BestIndex bitmask:
//! bit 0 (1) = ECInstanceId EQ constraint
//! bit 1 (2) = ECClassId EQ constraint
//! bit 2 (4) = TraversalDirection EQ constraint (optional)
//! bit 3 (8) = internal options EQ constraint (optional)
//! bit 4 (16) = stream adjacent RelationshipECClassId groups
//! bit 5 (32) = RelationshipECClassId EQ/IN filter
//! bit 6 (64) = RelatedECInstanceId EQ/IN filter
//! bit 7 (128) = Direction EQ filter
//! bit 8 (256) = the RelationshipECClassId filter is an all-at-once IN list
//! bit 9 (512) = the RelatedECInstanceId filter is an all-at-once IN list
//! The output-column filters only narrow the traversal. They are never omitted, so SQLite
//! still evaluates them with its own type and collation rules.
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationsModule::RelationsTable::BestIndex(IndexInfo& indexInfo)
    {
    // SQLite lists the constraints of the explicit WHERE clause *before* the terms it
    // synthesizes for table-valued-function arguments, so the constraint order does not
    // match the column order. Record the constraint index per column here and assign the
    // argv indices in a fixed column order afterwards, so that Filter() can decode argv
    // deterministically.
    int instIdIdx = -1;
    int classIdIdx = -1;
    int dirIdx = -1;
    int optionsIdx = -1;
    int relClassIdIdx = -1;
    int relatedIdIdx = -1;
    int directionIdx = -1;

    for (int i = 0; i < indexInfo.GetConstraintCount(); i++)
        {
        auto pConstraint = indexInfo.GetConstraint(i);
        int col = pConstraint->GetColumn();
        if (pConstraint->IsUsable() && pConstraint->GetOp() == IndexInfo::Operator::EQ)
            {
            if (col == (int) RelationsCursor::Columns::RelationshipECClassId && relClassIdIdx < 0)
                relClassIdIdx = i;
            else if (col == (int) RelationsCursor::Columns::RelatedECInstanceId && relatedIdIdx < 0)
                relatedIdIdx = i;
            else if (col == (int) RelationsCursor::Columns::Direction && directionIdx < 0)
                directionIdx = i;
            }

        if (col != (int) RelationsCursor::Columns::ECInstanceId &&
            col != (int) RelationsCursor::Columns::ECClassId &&
            col != (int) RelationsCursor::Columns::TraversalDir &&
            col != (int) RelationsCursor::Columns::Options)
            continue;

        if (!pConstraint->IsUsable() || pConstraint->GetOp() != IndexInfo::Operator::EQ)
            continue;

        // Only the first usable EQ constraint per column is handled. Any further constraint
        // on the same column is left for SQLite to verify (SetOmit is not called for it).
        if (col == (int) RelationsCursor::Columns::ECInstanceId)
            {
            if (instIdIdx < 0)
                instIdIdx = i;
            }
        else if (col == (int) RelationsCursor::Columns::ECClassId)
            {
            if (classIdIdx < 0)
                classIdIdx = i;
            }
        else if (col == (int) RelationsCursor::Columns::TraversalDir)
            {
            if (dirIdx < 0)
                dirIdx = i;
            }
        else if (optionsIdx < 0)
            optionsIdx = i;
        }

    // ECInstanceId and ECClassId are mandatory. Reject the plan so that SQLite either
    // reorders the loops or reports an error, rather than silently returning no rows.
    if (instIdIdx < 0 || classIdIdx < 0)
        return BE_SQLITE_CONSTRAINT;

    int idxNum = 1 | 2;
    int nArg = 0;

    indexInfo.GetConstraintUsage(instIdIdx)->SetArgvIndex(++nArg);
    indexInfo.GetConstraintUsage(instIdIdx)->SetOmit(true);

    indexInfo.GetConstraintUsage(classIdIdx)->SetArgvIndex(++nArg);
    indexInfo.GetConstraintUsage(classIdIdx)->SetOmit(true);

    if (dirIdx >= 0)
        {
        idxNum |= 4;
        indexInfo.GetConstraintUsage(dirIdx)->SetArgvIndex(++nArg);
        indexInfo.GetConstraintUsage(dirIdx)->SetOmit(true);
        }

    if (optionsIdx >= 0)
        {
        idxNum |= 8;
        indexInfo.GetConstraintUsage(optionsIdx)->SetArgvIndex(++nArg);
        indexInfo.GetConstraintUsage(optionsIdx)->SetOmit(true);
        }

    double estimatedRows = 100;
    if (relClassIdIdx >= 0)
        {
        idxNum |= 32;
        indexInfo.GetConstraintUsage(relClassIdIdx)->SetArgvIndex(++nArg);
        if (indexInfo.SetIn(relClassIdIdx, -1))
            {
            indexInfo.SetIn(relClassIdIdx, 1);
            idxNum |= 256;
            }
        estimatedRows = (idxNum & 256) != 0 ? 25 : 10;
        }

    if (relatedIdIdx >= 0)
        {
        idxNum |= 64;
        indexInfo.GetConstraintUsage(relatedIdIdx)->SetArgvIndex(++nArg);
        if (indexInfo.SetIn(relatedIdIdx, -1))
            {
            indexInfo.SetIn(relatedIdIdx, 1);
            idxNum |= 512;
            }
        estimatedRows = std::min(estimatedRows, (idxNum & 512) != 0 ? 10.0 : 1.0);
        }

    if (directionIdx >= 0)
        {
        idxNum |= 128;
        indexInfo.GetConstraintUsage(directionIdx)->SetArgvIndex(++nArg);
        estimatedRows = std::max(1.0, estimatedRows / 2);
        }

    // Resolving and preparing the traversal plans is a fixed cost per seed. Filters reduce the
    // rows read and returned, but never make a traversal free, so repeated per-row traversals
    // are not preferred over a single traversal followed by a join.
    indexInfo.SetEstimatedCost(5 + estimatedRows / 20);
    indexInfo.SetEstimatedRows((int64_t) estimatedRows);
    if (indexInfo.GetDistinct() == 1 && indexInfo.GetIndexOrderByCount() == 1 &&
        indexInfo.GetOrderBy(0)->GetColumn() == (int) RelationsCursor::Columns::RelationshipECClassId)
        {
        idxNum |= 16;
        indexInfo.SetOrderByConsumed(true);
        }
    indexInfo.SetIdxNum(idxNum);
    return BE_SQLITE_OK;
    }

// =====================================================================================
// RelationsCursor — Construction
// =====================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelationsModule::RelationsTable::RelationsCursor::RelationsCursor(RelationsTable& vt)
    : ECDbCursor(vt), m_iter(static_cast<RelationsModule&>(vt.GetModule()).GetECDb())
    {}

// =====================================================================================
// RelationsCursor — Filter
// =====================================================================================

//! Collects the integer values of an EQ or all-at-once IN argument. Returns false when a value
//! is neither INTEGER nor NULL, or the IN list cannot be read. The filter is then not pushed
//! down and SQLite alone evaluates it. NULL never compares equal, so it is skipped.
static bool CollectIdArgs(bvector<uint64_t>& ids, DbValue& arg, bool isInList)
    {
    auto add = [&ids](DbValue& value)
        {
        if (value.IsNull())
            return true;
        if (value.GetValueType() != DbValueType::IntegerVal)
            return false;
        ids.push_back((uint64_t) value.GetValueInt64());
        return true;
        };

    if (!isInList)
        {
        if (!add(arg))
            return false;
        }
    else
        {
        DbValue value(nullptr);
        DbResult rc = DbModule::InFirst(arg, value);
        while (rc == BE_SQLITE_OK && value.IsValid())
            {
            if (!add(value))
                return false;
            rc = DbModule::InNext(arg, value);
            }
        if (rc != BE_SQLITE_OK && rc != BE_SQLITE_DONE)
            return false;
        }

    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationsModule::RelationsTable::RelationsCursor::Filter(int idxNum, const char* idxStr, int argc, DbValue* argv)
    {
    m_iter.Clear();
    m_rowId = 0;
    m_eof = true;
    m_seedInstanceId = ECInstanceId();
    m_seedClassId = ECClassId();
    m_dir = TraversalDirection::Both;
    m_navRelClassIdFallback = false;

    // BestIndex rejects any plan without both required arguments, so this is defensive only.
    if ((idxNum & 3) != 3 || argc < 2)
        {
        GetTable().SetError("Relations() requires both an ECInstanceId and an ECClassId argument.");
        return BE_SQLITE_ERROR;
        }

    // BestIndex assigns argv indices in a fixed column order:
    // ECInstanceId, ECClassId, TraversalDirection, Options.
    int argIdx = 0;
    m_seedInstanceId = ECInstanceId((uint64_t) argv[argIdx++].GetValueInt64());
    m_seedClassId = ECClassId((uint64_t) argv[argIdx++].GetValueInt64());

    if ((idxNum & 4) != 0)
        {
        if (argIdx >= argc)
            {
            GetTable().SetError("Relations(): missing TraversalDirection argument.");
            return BE_SQLITE_ERROR;
            }

        DbValue& dirValue = argv[argIdx++];
        if (!dirValue.IsNull())
            {
            Utf8CP dirStr = dirValue.GetValueText();
            if (dirStr == nullptr)
                {
                GetTable().SetError("Relations(): TraversalDirection must be one of 'forward', 'backward' or 'both'.");
                return BE_SQLITE_ERROR;
                }

            if (BeStringUtilities::StricmpAscii(dirStr, "forward") == 0)
                m_dir = TraversalDirection::Forward;
            else if (BeStringUtilities::StricmpAscii(dirStr, "backward") == 0)
                m_dir = TraversalDirection::Backward;
            else if (BeStringUtilities::StricmpAscii(dirStr, "both") == 0)
                m_dir = TraversalDirection::Both;
            else
                {
                GetTable().SetError(Utf8PrintfString("Relations(): invalid TraversalDirection '%s'. Expected 'forward', 'backward' or 'both'.", dirStr).c_str());
                return BE_SQLITE_ERROR;
                }
            }
        }

    if ((idxNum & 8) != 0)
        {
        if (argIdx >= argc)
            {
            GetTable().SetError("Relations(): missing internal options argument.");
            return BE_SQLITE_ERROR;
            }
        DbValue& optionsValue = argv[argIdx++];
        m_navRelClassIdFallback = !optionsValue.IsNull() && optionsValue.GetValueInt64() != 0;
        }

    // Output-column filters, in the argv order assigned by BestIndex. They must only ever
    // narrow the traversal to a superset of the rows SQLite accepts afterwards.
    GraphTraversalFilter filter;
    TraversalDirection effectiveDir = m_dir;
    bool noMatch = false;
    if ((idxNum & 32) != 0)
        {
        if (argIdx >= argc)
            {
            GetTable().SetError("Relations(): missing RelationshipECClassId filter argument.");
            return BE_SQLITE_ERROR;
            }
        bvector<uint64_t> ids;
        if (CollectIdArgs(ids, argv[argIdx++], (idxNum & 256) != 0))
            {
            filter.m_filterRelClassIds = true;
            for (uint64_t id : ids)
                filter.m_relClassIds.push_back(ECClassId(id));
            noMatch |= ids.empty();
            }
        }

    if ((idxNum & 64) != 0)
        {
        if (argIdx >= argc)
            {
            GetTable().SetError("Relations(): missing RelatedECInstanceId filter argument.");
            return BE_SQLITE_ERROR;
            }
        bvector<uint64_t> ids;
        if (CollectIdArgs(ids, argv[argIdx++], (idxNum & 512) != 0))
            {
            filter.m_filterRelatedIds = true;
            for (uint64_t id : ids)
                filter.m_relatedIds.push_back(ECInstanceId(id));
            noMatch |= ids.empty();
            }
        }

    if ((idxNum & 128) != 0)
        {
        if (argIdx >= argc)
            {
            GetTable().SetError("Relations(): missing Direction filter argument.");
            return BE_SQLITE_ERROR;
            }
        // Matched case-insensitively, which is a superset of any collation SQLite applies.
        DbValue& dirValue = argv[argIdx++];
        Utf8CP dirStr = dirValue.GetValueType() == DbValueType::TextVal ? dirValue.GetValueText() : nullptr;
        TraversalDirection requested = TraversalDirection::Both;
        if (dirValue.IsNull())
            noMatch = true;
        else if (dirStr != nullptr && BeStringUtilities::StricmpAscii(dirStr, "forward") == 0)
            requested = TraversalDirection::Forward;
        else if (dirStr != nullptr && BeStringUtilities::StricmpAscii(dirStr, "backward") == 0)
            requested = TraversalDirection::Backward;

        if (requested != TraversalDirection::Both)
            {
            if (effectiveDir == TraversalDirection::Both)
                effectiveDir = requested;
            else if (effectiveDir != requested)
                noMatch = true;
            }
        }

    // An invalid (zero/NULL) seed simply has no relationships. This is not an error, so that
    // Relations() can be joined against columns that are legitimately NULL.
    if (!m_seedInstanceId.IsValid() || !m_seedClassId.IsValid() || noMatch)
        return BE_SQLITE_OK;

    // Ordinary scans stream immediately. Grouped scans buffer one row per SQL stream,
    // although ordering physical relationship classes may require inner SQLite sorts.
    if (SUCCESS != m_iter.Reset(ECInstanceKey(m_seedClassId, m_seedInstanceId), effectiveDir, m_navRelClassIdFallback, (idxNum & 16) != 0,
                                filter.IsEmpty() ? nullptr : &filter))
        {
        GetTable().SetError("Relations(): failed to traverse relationships for the given seed instance.");
        return BE_SQLITE_ERROR;
        }

    return Next();
    }

// =====================================================================================
// RelationsCursor — Navigation
// =====================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationsModule::RelationsTable::RelationsCursor::Next()
    {
    if (SUCCESS != m_iter.MoveNext())
        {
        m_eof = true;
        GetTable().SetError("Relations(): failed to traverse relationships for the given seed instance.");
        return BE_SQLITE_ERROR;
        }

    m_eof = m_iter.IsEof();
    if (!m_eof)
        ++m_rowId;

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationsModule::RelationsTable::RelationsCursor::GetColumn(int i, Context& ctx)
    {
    switch ((Columns) i)
        {
        // The hidden argument columns are answered even at EOF: SQLite is free to ignore the
        // SetOmit hint and re-check the constraint itself.
        case Columns::ECInstanceId:
            ctx.SetResultInt64(m_seedInstanceId.GetValueUnchecked());
            return BE_SQLITE_OK;
        case Columns::ECClassId:
            ctx.SetResultInt64(m_seedClassId.GetValueUnchecked());
            return BE_SQLITE_OK;
        case Columns::TraversalDir:
            ctx.SetResultText(m_dir == TraversalDirection::Forward ? "forward"
                              : m_dir == TraversalDirection::Backward ? "backward" : "both", -1, Context::CopyData::Yes);
            return BE_SQLITE_OK;
        case Columns::Options:
            ctx.SetResultInt(m_navRelClassIdFallback ? 1 : 0);
            return BE_SQLITE_OK;
        default:
            break;
        }

    if (m_eof)
        return BE_SQLITE_ERROR;

    auto const& rel = m_iter.GetCurrent();

    switch ((Columns) i)
        {
        case Columns::RelatedECInstanceId:
            ctx.SetResultInt64(rel.GetKey().GetInstanceId().GetValueUnchecked());
            break;
        case Columns::RelatedECClassId:
            ctx.SetResultInt64(rel.GetKey().GetClassId().GetValueUnchecked());
            break;
        case Columns::Direction:
            ctx.SetResultText(rel.GetDirection() == TraversalDirection::Forward ? "forward" : "backward", -1, Context::CopyData::Yes);
            break;
        case Columns::RelationshipECClassId:
            ctx.SetResultInt64(rel.GetRelClassId().GetValueUnchecked());
            break;
        case Columns::RelationshipECInstanceId:
            if (rel.GetRelInstanceId().IsValid())
                ctx.SetResultInt64(rel.GetRelInstanceId().GetValueUnchecked());
            else
                ctx.SetResultNull();
            break;
        case Columns::NavPropertyName:
            if (!rel.GetNavPropertyName().empty())
                ctx.SetResultText(rel.GetNavPropertyName().c_str(), -1, Context::CopyData::Yes);
            else
                ctx.SetResultNull();
            break;
        default:
            ctx.SetResultNull();
            break;
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationsModule::RelationsTable::RelationsCursor::GetRowId(int64_t& rowId)
    {
    rowId = m_rowId;
    return BE_SQLITE_OK;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
