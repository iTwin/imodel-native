/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/ChangeSet.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Provides a stable, lifetime-safe view of a single column value from either a
//! SQLite Statement or a changeset iterator row. Callers must not hold a pointer to
//! this object across a Step/Reset call.
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct IDbValueView
    {
    virtual ~IDbValueView() = default;
    virtual bool        IsNull()        const = 0;
    virtual int         GetInt()        const = 0;
    virtual int64_t     GetInt64()      const = 0;
    virtual double      GetDouble()     const = 0;
    virtual Utf8CP      GetText()       const = 0;
    virtual void const* GetBlob()       const = 0;
    virtual int         GetBytes()      const = 0;
    };

//=======================================================================================
//! IDbValueView backed directly by SQLite column APIs on a Statement.
//! Valid for the lifetime of the current row (until the next Step/Reset).
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct StatementDbValueView final : IDbValueView
    {
    private:
        Statement& m_stmt;
        int        m_col;
    public:
        StatementDbValueView(Statement& stmt, int col) : m_stmt(stmt), m_col(col) {}
        bool        IsNull()        const override { return m_stmt.GetColumnType(m_col) == DbValueType::NullVal; }
        int         GetInt()        const override { return m_stmt.GetValueInt(m_col); }
        int64_t     GetInt64()      const override { return m_stmt.GetValueInt64(m_col); }
        double      GetDouble()     const override { return m_stmt.GetValueDouble(m_col); }
        Utf8CP      GetText()       const override { return m_stmt.GetValueText(m_col); }
        void const* GetBlob()       const override { return m_stmt.GetValueBlob(m_col); }
        int         GetBytes()      const override { return m_stmt.GetColumnBytes(m_col); }
    };

//=======================================================================================
//! IDbValueView backed by a (non-owning) DbValue from a changeset iterator.
//! Valid for the lifetime of the current changeset row.
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangeSetDbValueView final : IDbValueView
    {
    private:
        DbValue m_val;
    public:
        explicit ChangeSetDbValueView(DbValue const& val) : m_val(val) {}
        bool        IsNull()        const override { return m_val.IsNull(); }
        int         GetInt()        const override { return m_val.GetValueInt(); }
        int64_t     GetInt64()      const override { return m_val.GetValueInt64(); }
        double      GetDouble()     const override { return m_val.GetValueDouble(); }
        Utf8CP      GetText()       const override { return m_val.GetValueText(); }
        void const* GetBlob()       const override { return m_val.GetValueBlob(); }
        int         GetBytes()      const override { return m_val.GetValueBytes(); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
