/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/ChangeSet.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Provides a uniform, row-level interface for reading column values from either a
//! SQLite Statement or a changeset iterator row. All methods take a 0-based column
//! index matching the BeSQLite::Statement API convention.
//! Callers must not use a value across a Step/Reset call.
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct IDbRow
    {
    virtual ~IDbRow() = default;
    virtual bool        IsColumnNull(int col)   const = 0;
    virtual int         GetValueInt(int col)    const = 0;
    virtual int64_t     GetValueInt64(int col)  const = 0;
    virtual double      GetValueDouble(int col) const = 0;
    virtual Utf8CP      GetValueText(int col)   const = 0;
    virtual void const* GetValueBlob(int col)   const = 0;
    virtual int         GetColumnBytes(int col) const = 0;
    };

//=======================================================================================
//! IDbRow backed directly by SQLite column APIs on a BeSQLite::Statement.
//! Valid for the lifetime of the current row (until the next Step/Reset).
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct StatementDbRow final : IDbRow
    {
    private:
        Statement& m_stmt;
    public:
        explicit StatementDbRow(Statement& stmt) : m_stmt(stmt) {}
        bool        IsColumnNull(int col)   const override { return m_stmt.GetColumnType(col) == DbValueType::NullVal; }
        int         GetValueInt(int col)    const override { return m_stmt.GetValueInt(col); }
        int64_t     GetValueInt64(int col)  const override { return m_stmt.GetValueInt64(col); }
        double      GetValueDouble(int col) const override { return m_stmt.GetValueDouble(col); }
        Utf8CP      GetValueText(int col)   const override { return m_stmt.GetValueText(col); }
        void const* GetValueBlob(int col)   const override { return m_stmt.GetValueBlob(col); }
        int         GetColumnBytes(int col) const override { return m_stmt.GetColumnBytes(col); }
    };

//=======================================================================================
//! IDbRow backed by a changeset iterator row.
//! Valid for the lifetime of the current changeset row.
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangeSetDbRow final : IDbRow
    {
    private:
        Changes::Change const& m_change;
        Changes::Change::Stage m_stage;
    public:
        ChangeSetDbRow(Changes::Change const& change, Changes::Change::Stage stage) : m_change(change), m_stage(stage) {}
        bool        IsColumnNull(int col)   const override { return m_change.GetValue(col, m_stage).IsNull(); }
        int         GetValueInt(int col)    const override { return m_change.GetValue(col, m_stage).GetValueInt(); }
        int64_t     GetValueInt64(int col)  const override { return m_change.GetValue(col, m_stage).GetValueInt64(); }
        double      GetValueDouble(int col) const override { return m_change.GetValue(col, m_stage).GetValueDouble(); }
        Utf8CP      GetValueText(int col)   const override { return m_change.GetValue(col, m_stage).GetValueText(); }
        void const* GetValueBlob(int col)   const override { return m_change.GetValue(col, m_stage).GetValueBlob(); }
        int         GetColumnBytes(int col) const override { return m_change.GetValue(col, m_stage).GetValueBytes(); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
