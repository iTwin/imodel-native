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
DbResult ClassPropsModule::ClassPropsVirtualTable::ClassPropsCursor::Next() final {
    ++m_it;
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ClassPropsModule::ClassPropsVirtualTable::ClassPropsCursor::GetRowId(int64_t& rowId) final {
    rowId =(int64_t)((*m_it).GetValueUnchecked());
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ClassPropsModule::ClassPropsVirtualTable::ClassPropsCursor::GetColumn(int i, Context& ctx) final {
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
DbResult ClassPropsModule::ClassPropsVirtualTable::ClassPropsCursor::Filter(int idxNum, const char *idxStr, int argc, DbValue* argv) final {
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
        if (json.isArray()) {
            Utf8String propList;
            json.ForEachArrayMemberValue([&](int idx, BeJsValue val) {
                if (!val.isString()) {
                    propList.clear();
                    return false;
                }
                if (idx > 0)
                    propList.append(",");
                propList.append("'").append(val.asCString()).append("'");
                return true;
            });
            if (propList.empty())  {
                GetTable().SetError("ContainsProps vtab: expect json array of strings contain property names");
                return BE_SQLITE_ERROR;
            }
            Utf8String sql = SqlPrintfString(R"x(
                SELECT DISTINCT [pm].[ClassId]
                FROM   [ec_propertyMap] [pm]
                    JOIN [ec_classmap] [cm] ON [cm].[ClassId] = [pm].[ClassId]
                            AND [cm].[MapStrategy] NOT IN (0, 10, 11)
                    JOIN [ec_PropertyPath] [pp] ON [pp].[Id] = [pm].[PropertyPathId]
                    JOIN [ec_Property] [pt] ON [pt].[Id] = [pp].[RootPropertyId]
                WHERE  [pt].[Name] IN (%s))x", propList.c_str()).GetUtf8CP();
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
DbResult ClassPropsModule::ClassPropsVirtualTable::BestIndex(IndexInfo& indexInfo) final {
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
DbResult ClassPropsModule::Connect(VirtualTable*& out, Config& conf, int argc, const char* const* argv) final {
    out = new ClassPropsVirtualTable(*this);
    conf.SetTag(Config::Tags::Innocuous);
    return BE_SQLITE_OK;
}

DbResult RegisterBuildInVTabs(ECDbR ecdb) {
    auto rc = (new ClassPropsModule(ecdb))->Register();
    return rc;
}
END_BENTLEY_SQLITE_EC_NAMESPACE