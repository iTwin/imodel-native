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
// RelationsCursor — Filter
// =====================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationsModule::RelationsTable::RelationsCursor::Filter(int idxNum, const char* idxStr, int argc, DbValue* argv)
    {
    m_results.clear();
    m_index = 0;

    // BestIndex rejects any plan without both required arguments, so this is defensive only.
    if ((idxNum & 3) != 3 || argc < 2)
        {
        GetTable().SetError("Relations() requires both an ECInstanceId and an ECClassId argument.");
        return BE_SQLITE_ERROR;
        }

    // BestIndex assigns argv indices in a fixed column order: ECInstanceId, ECClassId, TraversalDirection.
    int argIdx = 0;
    ECInstanceId instanceId((uint64_t) argv[argIdx++].GetValueInt64());
    ECClassId classId((uint64_t) argv[argIdx++].GetValueInt64());
    TraversalDirection dir = TraversalDirection::Both;

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
                dir = TraversalDirection::Forward;
            else if (BeStringUtilities::StricmpAscii(dirStr, "backward") == 0)
                dir = TraversalDirection::Backward;
            else if (BeStringUtilities::StricmpAscii(dirStr, "both") == 0)
                dir = TraversalDirection::Both;
            else
                {
                GetTable().SetError(Utf8PrintfString("Relations(): invalid TraversalDirection '%s'. Expected 'forward', 'backward' or 'both'.", dirStr).c_str());
                return BE_SQLITE_ERROR;
                }
            }
        }

    // An invalid (zero/NULL) seed simply has no relationships. This is not an error, so that
    // Relations() can be joined against columns that are legitimately NULL.
    if (!instanceId.IsValid() || !classId.IsValid())
        return BE_SQLITE_OK;

    // Build a single-hop InstanceGraph and collect results
    ECDbR ecdb = static_cast<RelationsModule&>(GetTable().GetModule()).GetECDb();
    InstanceGraph graph(ecdb);
    ECInstanceKey seedKey(classId, instanceId);
    graph.AddSeed(seedKey);

    if (graph.ExpandNode(seedKey, dir) != SUCCESS)
        {
        GetTable().SetError("Relations(): failed to traverse relationships for the given seed instance.");
        return BE_SQLITE_ERROR;
        }

    auto const* related = graph.GetRelated(seedKey);
    if (related != nullptr)
        m_results = *related;

    return BE_SQLITE_OK;
    }

// =====================================================================================
// RelationsCursor — Navigation
// =====================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationsModule::RelationsTable::RelationsCursor::Next()
    {
    ++m_index;
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationsModule::RelationsTable::RelationsCursor::GetColumn(int i, Context& ctx)
    {
    if (m_index >= m_results.size())
        return BE_SQLITE_ERROR;

    auto const& rel = m_results[m_index];

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
    rowId = (int64_t) m_index;
    return BE_SQLITE_OK;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
