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

    for (int i = 0; i < indexInfo.GetConstraintCount(); i++)
        {
        auto pConstraint = indexInfo.GetConstraint(i);
        int col = pConstraint->GetColumn();
        if (col != (int) RelationsCursor::Columns::ECInstanceId &&
            col != (int) RelationsCursor::Columns::ECClassId &&
            col != (int) RelationsCursor::Columns::TraversalDir)
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
        else
            {
            if (dirIdx < 0)
                dirIdx = i;
            }
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

    indexInfo.SetEstimatedCost(10);
    indexInfo.SetEstimatedRows(100);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationsModule::RelationsTable::RelationsCursor::Filter(int idxNum, const char* idxStr, int argc, DbValue* argv)
    {
    m_rowId = 0;
    m_eof = true;
    m_seedInstanceId = ECInstanceId();
    m_seedClassId = ECClassId();
    m_dir = TraversalDirection::Both;

    // BestIndex rejects any plan without both required arguments, so this is defensive only.
    if ((idxNum & 3) != 3 || argc < 2)
        {
        GetTable().SetError("Relations() requires both an ECInstanceId and an ECClassId argument.");
        return BE_SQLITE_ERROR;
        }

    // BestIndex assigns argv indices in a fixed column order: ECInstanceId, ECClassId, TraversalDirection.
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

    // An invalid (zero/NULL) seed simply has no relationships. This is not an error, so that
    // Relations() can be joined against columns that are legitimately NULL.
    if (!m_seedInstanceId.IsValid() || !m_seedClassId.IsValid())
        return BE_SQLITE_OK;

    // Rows are streamed: the related instances are never materialized as a whole, so a seed with
    // a very high fan-out does not have to be fully read before the first row is produced.
    if (SUCCESS != m_iter.Reset(ECInstanceKey(m_seedClassId, m_seedInstanceId), m_dir))
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
