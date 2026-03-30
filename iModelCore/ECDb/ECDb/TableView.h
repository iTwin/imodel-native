/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ClassMap.h"
#include "RelationshipClassMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSqlSelectPreparedStatement;

//=======================================================================================
//! A lightweight, seekable view over a single SQLite table row, keyed by ECInstanceId
//! (ROWID). Used by InstanceReader and ChangesetFieldFactory to read live DB values
//! without going through the full ECSql engine.
//
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct TableView final {
    using Ptr = std::unique_ptr<TableView>;

private:
    std::unique_ptr<ECSqlSelectPreparedStatement> m_stmt;
    std::map<DbColumnId, int> m_colIndexMap;
    DbTableId m_id;
    int m_ecClassIdCol;
    int m_ecSourceClassIdCol;
    int m_ecTargetClassIdCol;

    static Ptr CreateNullTableView(ECDbCR, DbTable const&);
    static Ptr CreateTableView(ECDbCR, DbTable const&);
    static Ptr CreateLinkTableView(ECDbCR, DbTable const&, RelationshipClassLinkTableMap const&);
    static Ptr CreateEntityTableView(ECDbCR, DbTable const&, ClassMapCR);

public:
    explicit TableView(ECDbCR conn);
    ~TableView();
    TableView(TableView const&) = delete;
    TableView& operator=(TableView const&) = delete;

    Statement& GetSqliteStmt() const;
    ECSqlSelectPreparedStatement& GetECSqlStmt() const;

    int GetColumnIndexOf(DbColumnId) const;
    int GetColumnIndexOf(DbColumn const& col) const { return GetColumnIndexOf(col.GetId()); }

    int GetClassIdCol() const { return m_ecClassIdCol; }
    int GetSourceClassIdCol() const { return m_ecSourceClassIdCol; }
    int GetTargetClassIdCol() const { return m_ecTargetClassIdCol; }

    size_t GetColumnCount() const { return m_colIndexMap.size(); }

    static Ptr Create(ECDbCR, DbTable const&);
    DbTableId GetId() const { return m_id; }
    bool Seek(ECInstanceId rowId, ECN::ECClassId* classId = nullptr) const;
};

END_BENTLEY_SQLITE_EC_NAMESPACE
