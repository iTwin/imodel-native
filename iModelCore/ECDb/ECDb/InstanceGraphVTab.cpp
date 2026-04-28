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
    int idxNum = 0;
    int nArg = 0;

    for (int i = 0; i < indexInfo.GetConstraintCount(); i++)
        {
        auto pConstraint = indexInfo.GetConstraint(i);
        if (!pConstraint->IsUsable() || pConstraint->GetOp() != IndexInfo::Operator::EQ)
            continue;

        int col = pConstraint->GetColumn();
        if (col == (int) RelationsCursor::Columns::ECInstanceId)
            {
            idxNum |= 1;
            indexInfo.GetConstraintUsage(i)->SetArgvIndex(++nArg);
            indexInfo.GetConstraintUsage(i)->SetOmit(true);
            }
        else if (col == (int) RelationsCursor::Columns::ECClassId)
            {
            idxNum |= 2;
            indexInfo.GetConstraintUsage(i)->SetArgvIndex(++nArg);
            indexInfo.GetConstraintUsage(i)->SetOmit(true);
            }
        else if (col == (int) RelationsCursor::Columns::TraversalDir)
            {
            idxNum |= 4;
            indexInfo.GetConstraintUsage(i)->SetArgvIndex(++nArg);
            indexInfo.GetConstraintUsage(i)->SetOmit(true);
            }
        }

    // Both ECInstanceId and ECClassId are required
    if ((idxNum & 3) == 3)
        {
        indexInfo.SetEstimatedCost(10);
        indexInfo.SetEstimatedRows(100);
        }
    else
        {
        // Missing required constraints — discourage planner
        indexInfo.SetEstimatedCost(1000000);
        indexInfo.SetEstimatedRows(1000000);
        }

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

    if ((idxNum & 3) != 3)
        return BE_SQLITE_OK; // missing required constraints, return empty

    int argIdx = 0;
    ECInstanceId instanceId;
    ECClassId classId;
    TraversalDirection dir = TraversalDirection::Both;

    // Extract ECInstanceId (always first due to BestIndex ordering)
    if (idxNum & 1)
        instanceId = ECInstanceId((uint64_t) argv[argIdx++].GetValueInt64());

    // Extract ECClassId
    if (idxNum & 2)
        classId = ECClassId((uint64_t) argv[argIdx++].GetValueInt64());

    // Extract optional TraversalDirection
    if (idxNum & 4)
        {
        Utf8CP dirStr = argv[argIdx++].GetValueText();
        if (dirStr != nullptr)
            {
            if (BeStringUtilities::StricmpAscii(dirStr, "forward") == 0)
                dir = TraversalDirection::Forward;
            else if (BeStringUtilities::StricmpAscii(dirStr, "backward") == 0)
                dir = TraversalDirection::Backward;
            // else default Both
            }
        }

    if (!instanceId.IsValid() || !classId.IsValid())
        return BE_SQLITE_OK;

    // Build a single-hop InstanceGraph and collect results
    ECDbR ecdb = static_cast<RelationsModule&>(GetTable().GetModule()).GetECDb();
    InstanceGraph graph(ecdb);
    ECInstanceKey seedKey(classId, instanceId);
    graph.AddSeed(seedKey);

    if (graph.ExpandNode(seedKey, dir) != SUCCESS)
        return BE_SQLITE_ERROR;

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
