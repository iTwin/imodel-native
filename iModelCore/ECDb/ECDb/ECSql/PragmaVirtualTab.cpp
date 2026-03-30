/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// PragmaVirtualTabCursor
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void PragmaVirtualTabCursor::EnsureColumnCapacity(int col) {
    if (col >= (int)m_currentRow.size())
        m_currentRow.resize((size_t)(col + 1));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void PragmaVirtualTabCursor::SetColumnNull(int col) {
    EnsureColumnCapacity(col);
    m_currentRow[col] = PragmaColumnValue();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void PragmaVirtualTabCursor::SetColumnInt64(int col, int64_t v) {
    EnsureColumnCapacity(col);
    m_currentRow[col] = PragmaColumnValue(v);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void PragmaVirtualTabCursor::SetColumnDouble(int col, double v) {
    EnsureColumnCapacity(col);
    m_currentRow[col] = PragmaColumnValue(v);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void PragmaVirtualTabCursor::SetColumnText(int col, Utf8CP v) {
    EnsureColumnCapacity(col);
    m_currentRow[col] = PragmaColumnValue(v);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void PragmaVirtualTabCursor::SetColumnText(int col, Utf8StringCR v) {
    EnsureColumnCapacity(col);
    m_currentRow[col] = PragmaColumnValue(v);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void PragmaVirtualTabCursor::SetColumnBool(int col, bool v) {
    EnsureColumnCapacity(col);
    m_currentRow[col] = PragmaColumnValue(v);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
PragmaColumnValue const& PragmaVirtualTabCursor::GetCurrentRowValue(int col) const {
    static PragmaColumnValue s_null;
    if (col < 0 || col >= (int)m_currentRow.size())
        return s_null;
    return m_currentRow[col];
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaVirtualTabCursor::GetColumn(int i, Context& ctx) {
    PragmaColumnValue const& val = GetCurrentRowValue(i);
    if (val.IsNull()) {
        ctx.SetResultNull();
        return BE_SQLITE_OK;
    }
    switch (val.GetType()) {
        case PragmaColumnValue::Type::Int64:
            ctx.SetResultInt64(val.AsInt64());
            break;
        case PragmaColumnValue::Type::Bool:
            ctx.SetResultInt64(val.AsBool() ? 1 : 0);
            break;
        case PragmaColumnValue::Type::Double:
            ctx.SetResultDouble(val.AsDouble());
            break;
        case PragmaColumnValue::Type::String: {
            Utf8CP text = val.AsCString();
            ctx.SetResultText(text, text != nullptr ? -1 : 0, Context::CopyData::Yes);
            break;
        }
        default:
            ctx.SetResultNull();
            break;
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaVirtualTabCursor::GetRowId(int64_t& rowId) {
    rowId = m_rowId;
    return BE_SQLITE_OK;
}

//=======================================================================================
// PragmaVirtualTabResult
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaVirtualTabResult::PrepareQuery(Utf8CP sql) {
    return m_stmt.Prepare(GetECDb(), sql);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaVirtualTabResult::BindText(int idx, Utf8CP text) {
    return m_stmt.BindText(idx, text, BeSQLite::Statement::MakeCopy::Yes);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaVirtualTabResult::BindInt64(int idx, int64_t val) {
    return m_stmt.BindInt64(idx, val);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaVirtualTabResult::BindNull(int idx) {
    return m_stmt.BindNull(idx);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaVirtualTabResult::BindBool(int idx, bool val) {
    return BindInt64(idx, val ? 1 : 0);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaVirtualTabResult::_Step() {
    return m_stmt.Step();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaVirtualTabResult::_Reset() {
    return m_stmt.Reset();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult PragmaVirtualTabResult::_Init() {
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
PragmaColumnValue PragmaVirtualTabResult::_GetCurrentValue(int col) const {
    auto type = const_cast<BeSQLite::Statement&>(m_stmt).GetColumnType(col);
    switch (type) {
        case BeSQLite::DbValueType::IntegerVal:
            return PragmaColumnValue(const_cast<BeSQLite::Statement&>(m_stmt).GetValueInt64(col));
        case BeSQLite::DbValueType::FloatVal:
            return PragmaColumnValue(const_cast<BeSQLite::Statement&>(m_stmt).GetValueDouble(col));
        case BeSQLite::DbValueType::TextVal:
            return PragmaColumnValue(const_cast<BeSQLite::Statement&>(m_stmt).GetValueText(col));
        default:
            return PragmaColumnValue();
    }
}

END_BENTLEY_SQLITE_EC_NAMESPACE
