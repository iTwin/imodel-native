/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ClassPropsModule::ClassPropsVirtualTable::ClassPropsCursor::Next() {
    ++m_it;
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ClassPropsModule::ClassPropsVirtualTable::ClassPropsCursor::GetRowId(int64_t& rowId) {
    rowId =(int64_t)((*m_it).GetValueUnchecked());
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ClassPropsModule::ClassPropsVirtualTable::ClassPropsCursor::GetColumn(int i, Context& ctx) {
    if ((Columns)i == Columns::Text) {
        ctx.SetResultText(m_props.c_str(), (int)m_props.size(), Context::CopyData::No);
    } else if ((Columns)i == Columns::ClassId && m_it != m_classIds.end()) {
        ctx.SetResultInt64((int64_t)(*m_it).GetValueUnchecked());
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ClassPropsModule::ClassPropsVirtualTable::ClassPropsCursor::Filter(int idxNum, const char *idxStr, int argc, DbValue* argv) {
    int i = 0;
    bool recompute = true;
    if (idxNum & 1) {
        auto props = argv[i++].GetValueText();
        if (m_props.EqualsIAscii(props)) {
            recompute = false;
        } else {
            m_props = props;
        }
    } else {
        m_props = "[]";
    }
    if (recompute) {
        m_classIds.clear();
        m_it = m_classIds.begin();
        BeJsDocument json;
        json.Parse(m_props);
        int propCount = 0;
        if (json.isArray()) {
            Utf8String propList;
            json.ForEachArrayMemberValue([&](int idx, BeJsValue val) {
                if (!val.isString()) {
                    propList.clear();
                    return true;
                }
                if (idx > 0)
                    propList.append(",");
                propList.append("'").append(val.asCString()).append("'");
                ++propCount;
                return false;
            });
            if (propList.empty())  {
                GetTable().SetError("ContainsProps vtab: expect json array of strings contain property names");
                return BE_SQLITE_ERROR;
            }
            Utf8String sql = SqlPrintfString(R"x(
                SELECT [pm].[ClassId]
                FROM   [ec_propertyMap] [pm]
                    JOIN [ec_classmap] [cm] ON [cm].[ClassId] = [pm].[ClassId]
                            AND [cm].[MapStrategy] NOT IN (0, 10, 11)
                    JOIN [ec_PropertyPath] [pp] ON [pp].[Id] = [pm].[PropertyPathId]
                    JOIN [ec_Property] [pt] ON [pt].[Id] = [pp].[RootPropertyId]
                WHERE  [pt].[Name] IN (%s) GROUP BY [pm].[ClassId] HAVING COUNT(DISTINCT [pt].[Name]) = %d)x", propList.c_str(), propCount).GetUtf8CP();
            BeSQLite::Statement stmt;
            if(BE_SQLITE_OK != stmt.Prepare(GetTable().GetModule().GetDb(), sql.c_str())) {
                GetTable().SetError("ContainsProps vtab: unable to prepare sql stmt");
                return BE_SQLITE_ERROR;
            }
            while(stmt.Step() == BE_SQLITE_ROW) {
                m_classIds.insert(stmt.GetValueId<ECClassId>(0));
            }
        }
    }
    m_it = m_classIds.begin();
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ClassPropsModule::ClassPropsVirtualTable::BestIndex(IndexInfo& indexInfo) {
    int i, j;              /* Loop over constraints */
    int idxNum = 0;        /* The query plan bitmask */
    int unusableMask = 0;  /* Mask of unusable constraints */
    int nArg = 0;          /* Number of arguments that seriesFilter() expects */
    int aIdx[2];           /* Constraints on start, stop, and step */
    const int SQLITE_SERIES_CONSTRAINT_VERIFY = 0;
    aIdx[0] = aIdx[1] = -1;
    int nConstraint = indexInfo.GetConstraintCount();

    for(i=0; i<nConstraint; i++){
        auto pConstraint = indexInfo.GetConstraint(i);
        int iCol;    /* 0 for start, 1 for stop, 2 for step */
        int iMask;   /* bitmask for those column */
        if( pConstraint->GetColumn()< (int)ClassPropsCursor::Columns::Text) continue;
        iCol = pConstraint->GetColumn() - (int)ClassPropsCursor::Columns::Text;
        iMask = 1 << iCol;
        if (!pConstraint->IsUsable()){
            unusableMask |=  iMask;
            continue;
        } else if (pConstraint->GetOp() == IndexInfo::Operator::EQ ){
            idxNum |= iMask;
            aIdx[iCol] = i;
        }
    }
    for( i = 0; i < 1; i++) {
        if( (j = aIdx[i]) >= 0 ) {
            indexInfo.GetConstraintUsage(j)->SetArgvIndex(++nArg);
            indexInfo.GetConstraintUsage(j)->SetOmit(!SQLITE_SERIES_CONSTRAINT_VERIFY);
        }
    }

    if ((unusableMask & ~idxNum)!=0 ){
        return BE_SQLITE_CONSTRAINT;
    }

    indexInfo.SetEstimatedCost(0);
    indexInfo.SetEstimatedRows(100);
    indexInfo.SetIdxNum(idxNum);
    // indexInfo.SetIdxFlags(IndexInfo::ScanFlags::UNIQUE);
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ClassPropsModule::Connect(DbVirtualTable*& out, Config& conf, int argc, const char* const* argv) {
    out = new ClassPropsVirtualTable(*this);
    conf.SetTag(Config::Tags::Innocuous);
    return BE_SQLITE_OK;
}

/*IdSet Virtual Table*/

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult IdSetModule::IdSetTable::IdSetCursor::Next() {
    ++m_index;
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult IdSetModule::IdSetTable::IdSetCursor::GetRowId(int64_t& rowId) {
    rowId = (int64_t)(*m_index);
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult IdSetModule::IdSetTable::IdSetCursor::GetColumn(int i, Context& ctx) {
    if ((Columns)i == Columns::Json_array_ids) {
        ctx.SetResultText(m_text.c_str(), (int)m_text.size(), Context::CopyData::No);
    } else if ((Columns)i == Columns::Id && m_index != m_idSet.end()) {
        ctx.SetResultInt64((int64_t)(*m_index));
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult IdSetModule::IdSetTable::IdSetCursor::FilterJSONStringIntoArray(BeJsDocument& doc) {
    if(!doc.isArray()) {
        GetTable().SetError("IdSet vtab: The argument should be a valid JSON array of ids");
        return BE_SQLITE_ERROR;
    }
    bool flag = doc.ForEachArrayMember([&](BeJsValue::ArrayIndex a, BeJsConst k1) {
                                            if(BE_SQLITE_OK != FilterJSONBasedOnType(k1)) {
                                                GetTable().SetError(SqlPrintfString("IdSet vtab: The element with index %u is invalid", a));
                                                return true;
                                            }
                                            return false; 
                                        });
    if(flag)
        return BE_SQLITE_ERROR;
    return BE_SQLITE_OK;
}


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult IdSetModule::IdSetTable::IdSetCursor::FilterJSONBasedOnType(BeJsConst& val) {
    if(val.isNull()) {
        return BE_SQLITE_ERROR;
    }
    else if(val.isNumeric()) {
        uint64_t id = val.GetUInt64();
        if(id == 0)
            return BE_SQLITE_ERROR;
        m_idSet.insert(id);
    }
    else if(val.isString()) {
        uint64_t id;
        BentleyStatus status = BeStringUtilities::ParseUInt64(id, val.ToUtf8CP());
        if(status != BentleyStatus::SUCCESS)
            return BE_SQLITE_ERROR;
        if(id == 0)
            return BE_SQLITE_ERROR;
        m_idSet.insert(id);
    }
    else
        return BE_SQLITE_ERROR;
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult IdSetModule::IdSetTable::IdSetCursor::Filter(int idxNum, const char *idxStr, int argc, DbValue* argv) {
    int recompute = false;
    if( idxNum & 1 ){
        if(argv[0].GetValueType() == DbValueType::TextVal) {
            Utf8String valueGiven = argv[0].GetValueText();
            if(!valueGiven.EqualsIAscii(m_text)) {
                m_text = valueGiven;
                recompute = true;
            }
        } else {
            Reset();
        }
    } else {
        Reset();
    }
    if(recompute) {
        m_idSet.clear();
        BeJsDocument doc;
        doc.Parse(m_text.c_str());
        
        if(FilterJSONStringIntoArray(doc) != BE_SQLITE_OK) {
            Reset();
            m_index = m_idSet.begin();
            return BE_SQLITE_ERROR;
        }
    }
    m_index = m_idSet.begin();
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void IdSetModule::IdSetTable::IdSetCursor::Reset() {
    m_text = "[]";
    m_idSet.clear();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult IdSetModule::IdSetTable::BestIndex(IndexInfo& indexInfo) {
    int i, j;              /* Loop over constraints */
    int idxNum = 0;        /* The query plan bitmask */
    int unusableMask = 0;  /* Mask of unusable constraints */
    int nArg = 0;          /* Number of arguments that seriesFilter() expects */
    int aIdx[2];           /* Constraints on start, stop, and step */
    const int SQLITE_SERIES_CONSTRAINT_VERIFY = 0;
    aIdx[0] = aIdx[1] = -1;
    int nConstraint = indexInfo.GetConstraintCount();

    for(i=0; i<nConstraint; i++) {
        auto pConstraint = indexInfo.GetConstraint(i);
        int iCol;    /* 0 for start, 1 for stop, 2 for step */
        int iMask;   /* bitmask for those column */
        if( pConstraint->GetColumn()< (int)IdSetCursor::Columns::Json_array_ids) continue;
        iCol = pConstraint->GetColumn() - (int)IdSetCursor::Columns::Json_array_ids;
        iMask = 1 << iCol;
        if (!pConstraint->IsUsable()){
            unusableMask |=  iMask;
            continue;
        } else if (pConstraint->GetOp() == IndexInfo::Operator::EQ ){
            idxNum |= iMask;
            aIdx[iCol] = i;
        }
    }
    for( i = 0; i < 1; i++) {
        if( (j = aIdx[i]) >= 0 ) {
            indexInfo.GetConstraintUsage(j)->SetArgvIndex(++nArg);
            indexInfo.GetConstraintUsage(j)->SetOmit(!SQLITE_SERIES_CONSTRAINT_VERIFY);
        }
    }

    if ((unusableMask & ~idxNum)!=0 ) {
        return BE_SQLITE_CONSTRAINT;
    }

    indexInfo.SetEstimatedCost(0);
    indexInfo.SetEstimatedRows(100);
    indexInfo.SetIdxNum(idxNum);
    return BE_SQLITE_OK;

}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult IdSetModule::Connect(DbVirtualTable*& out, Config& conf, int argc, const char* const* argv) {
    out = new IdSetTable(*this);
    conf.SetTag(Config::Tags::Innocuous);
    return BE_SQLITE_OK;
}

DbResult RegisterBuildInVTabs(ECDbR ecdb) {
    DbResult rc = (new ClassPropsModule(ecdb))->Register();
    DbResult rcIdSet = (new IdSetModule(ecdb))->Register();
    if(rc != BE_SQLITE_OK || rcIdSet != BE_SQLITE_OK)
        return rc;
    return BE_SQLITE_OK;
}
END_BENTLEY_SQLITE_EC_NAMESPACE