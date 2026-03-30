/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <BeSQLite/VirtualTab.h>
#include "PragmaECSqlPreparedStatement.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Base cursor for pragma virtual tables. Stores the current row as a vector of
//! PragmaColumnValue entries populated by subclasses via SetColumnXxx helpers.
//! Provides GetColumn()/GetRowId() and a default Eof() driven by m_eof.
//! Subclasses must implement Filter() and Next().
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaVirtualTabCursor : BeSQLite::DbModule::DbVirtualTable::DbCursor {
protected:
    std::vector<PragmaColumnValue> m_currentRow;
    bool m_eof = true;
    int64_t m_rowId = 0;

    void EnsureColumnCapacity(int col);
    void SetColumnNull(int col);
    void SetColumnInt64(int col, int64_t v);
    void SetColumnDouble(int col, double v);
    void SetColumnText(int col, Utf8CP v);
    void SetColumnText(int col, Utf8StringCR v);
    void SetColumnBool(int col, bool v);

public:
    explicit PragmaVirtualTabCursor(BeSQLite::DbModule::DbVirtualTable& vt)
        : DbCursor(vt) {}

    PragmaColumnValue const& GetCurrentRowValue(int col) const;

    //! Access the owning ECDb. Works because all pragma modules store a DbR which is ECDbR.
    ECDbR GetECDb() { return reinterpret_cast<ECDbR>(GetTable().GetModule().GetDb()); }

    bool Eof() override { return m_eof; }
    DbResult GetColumn(int i, Context& ctx) final;
    DbResult GetRowId(int64_t& rowId) final;

    DbResult Next() override = 0;
    DbResult Filter(int idxNum, const char* idxStr, int argc, BeSQLite::DbValue* args) override = 0;
};

//=======================================================================================
//! PragmaResult subclass that queries a registered BeSQLite::DbModule virtual table via
//! "SELECT * FROM pragma_<name>(...)" for lazy row iteration.
//! Replaces the eager BeJsDocument approach of StaticPragmaResult.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaVirtualTabResult : PragmaResult {
private:
    BeSQLite::Statement m_stmt;

    DbResult _Step() override;
    DbResult _Reset() override;
    DbResult _Init() override;
    PragmaColumnValue _GetCurrentValue(int col) const override;

public:
    explicit PragmaVirtualTabResult(ECDbCR ecdb) : PragmaResult(ecdb) {}
    virtual ~PragmaVirtualTabResult() {}

    DbResult PrepareQuery(Utf8CP sql);
    DbResult BindText(int idx, Utf8CP text);
    DbResult BindInt64(int idx, int64_t val);
    DbResult BindNull(int idx);
    DbResult BindBool(int idx, bool val);
};

END_BENTLEY_SQLITE_EC_NAMESPACE
