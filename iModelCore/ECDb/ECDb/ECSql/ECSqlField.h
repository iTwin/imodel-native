/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/IECSqlValue.h>
#include "IDbValueView.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSqlSelectPreparedStatement;

//=======================================================================================
//! Strategy interface for providing column values to an ECSqlField.
//! Produces an IDbValueView for the given column index.
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct IECSqlDataReaderStrategy
    {
    virtual ~IECSqlDataReaderStrategy() = default;
    virtual std::unique_ptr<IDbValueView> GetValue(int colNum) const = 0;
    };

//=======================================================================================
//! IECSqlDataReaderStrategy backed by a SQLite Statement.
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct SqliteStatementDataReaderStrategy : IECSqlDataReaderStrategy
    {
    private:
        Statement& m_sqliteStatement;
    public:
        SqliteStatementDataReaderStrategy(Statement& sqliteStatement) : m_sqliteStatement(sqliteStatement) {}
        std::unique_ptr<IDbValueView> GetValue(int colNum) const override
            { return std::make_unique<StatementDbValueView>(m_sqliteStatement, colNum); }
    };

//=======================================================================================
//! IECSqlDataReaderStrategy backed by a changeset iterator row.
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangeSetDataReaderStrategy : IECSqlDataReaderStrategy
    {
    private:
        Changes::Change const& m_change;
        Changes::Change::Stage const& m_stage;
    public:
        ChangeSetDataReaderStrategy(Changes::Change const& change, Changes::Change::Stage const& stage) : m_change(change), m_stage(stage) {}
        std::unique_ptr<IDbValueView> GetValue(int colNum) const override
            {
            BeAssert(colNum >= 0 && colNum < m_change.GetColumnCount() && "Column index is out of bounds.");
            return std::make_unique<ChangeSetDbValueView>(m_change.GetValue(colNum, m_stage));
            }
    };

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlField : public IECSqlValue
    {
private:
    ECDbCR m_ecdb;
    std::unique_ptr<IECSqlDataReaderStrategy> m_dataReaderStrategy;
    ECSqlColumnInfo m_ecsqlColumnInfo;
    ECSqlColumnInfo m_ecsqlDynamicColumnInfo;
    bool m_requiresOnAfterStep = false;
    bool m_requiresOnAfterReset = false;
private:

    ECSqlColumnInfoCR _GetColumnInfo() const override { return m_ecsqlDynamicColumnInfo.IsValid() ? m_ecsqlDynamicColumnInfo : m_ecsqlColumnInfo; }

    virtual ECSqlStatus _OnAfterReset() { SetDynamicColumnInfo(ECSqlColumnInfo()); return ECSqlStatus::Success; }
    virtual ECSqlStatus _OnAfterStep() { return ECSqlStatus::Success; }
    virtual void _OnDynamicPropertyUpdated() {}
protected:
    //! Constructor for Statement-based fields (existing ECSql path)
    ECSqlField(ECDbCR ecdb, Statement& sqliteStatement, ECSqlColumnInfo const& ecsqlColumnInfo, bool needsOnAfterStep, bool needsOnAfterReset)
        : m_ecdb(ecdb), m_dataReaderStrategy(std::make_unique<SqliteStatementDataReaderStrategy>(sqliteStatement)),
          m_ecsqlColumnInfo(ecsqlColumnInfo), m_requiresOnAfterStep(needsOnAfterStep), m_requiresOnAfterReset(needsOnAfterReset)
        {}

    //! Constructor for Change-based fields (changeset reader path)
    ECSqlField(ECDbCR ecdb, Changes::Change const& change, Changes::Change::Stage const& stage, ECSqlColumnInfo const& ecsqlColumnInfo, bool needsOnAfterStep, bool needsOnAfterReset)
        : m_ecdb(ecdb), m_dataReaderStrategy(std::make_unique<ChangeSetDataReaderStrategy>(change, stage)),
          m_ecsqlColumnInfo(ecsqlColumnInfo), m_requiresOnAfterStep(needsOnAfterStep), m_requiresOnAfterReset(needsOnAfterReset)
        {}

    std::unique_ptr<IDbValueView> GetSqliteValue(int colNum) const;
    ECDbCR GetECDb() const { return m_ecdb; }
    void SetRequiresOnAfterStep(bool req) { m_requiresOnAfterStep = req; }
    void SetRequiresOnAfterReset(bool req) { m_requiresOnAfterReset = req; }

public:
    virtual ~ECSqlField() {}
    bool RequiresOnAfterStep() const { return m_requiresOnAfterStep; }
    ECSqlStatus OnAfterStep() { return _OnAfterStep(); }
    bool RequiresOnAfterReset() const { return m_requiresOnAfterReset || _GetColumnInfo().IsDynamic(); }
    ECSqlStatus OnAfterReset() { return _OnAfterReset(); }
    void SetDynamicColumnInfo(ECSqlColumnInfoCR info);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE