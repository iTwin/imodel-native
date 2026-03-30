/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TableView::TableView(ECDbCR conn)
    : m_stmt(std::make_unique<ECSqlSelectPreparedStatement>(conn)),
      m_ecClassIdCol(-1), m_ecSourceClassIdCol(-1), m_ecTargetClassIdCol(-1) {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TableView::~TableView() = default;

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Statement& TableView::GetSqliteStmt() const { return m_stmt->GetSqliteStatement(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectPreparedStatement& TableView::GetECSqlStmt() const { return *m_stmt; }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool TableView::Seek(ECInstanceId rowId, ECN::ECClassId* classId) const {
    auto& stmt = GetSqliteStmt();
    stmt.Reset();
    stmt.ClearBindings();
    stmt.BindId(1, rowId);
    const auto hasRow = stmt.Step() == BE_SQLITE_ROW;
    if (hasRow && classId && m_ecClassIdCol >= 0) {
        *classId = stmt.GetValueId<ECN::ECClassId>(m_ecClassIdCol);
    }
    return hasRow;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int TableView::GetColumnIndexOf(DbColumnId id) const {
    const auto it = m_colIndexMap.find(id);
    return it == m_colIndexMap.end() ? -1 : it->second;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TableView::Ptr TableView::CreateNullTableView(ECDbCR conn, DbTable const& tbl) {
    if (tbl.GetType() != DbTable::Type::Virtual) {
        BeAssert(false && "Expect a virtual table");
        return nullptr;
    }

    auto tableView = std::make_unique<TableView>(conn);
    NativeSqlBuilder builder;
    builder.Append("SELECT ");
    int appendIndex = 0;
    for (auto col : tbl.GetColumns()) {
        if (col != tbl.GetColumns().front()) {
            builder.AppendComma();
        }

        builder.Append("NULL")
            .AppendSpace()
            .AppendEscaped(col->GetName());

        tableView->m_colIndexMap.insert(std::make_pair(col->GetId(), appendIndex));
        appendIndex++;
    }
    builder.AppendSpace().Append("LIMIT 0");

    tableView->m_id = tbl.GetId();
    const auto rc = tableView->GetSqliteStmt().Prepare(conn, builder.GetSql().c_str());
    if (rc != BE_SQLITE_OK) {
        BeAssert(false && "Failed to prepare statement");
        return nullptr;
    }
    return tableView;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TableView::Ptr TableView::CreateTableView(ECDbCR conn, DbTable const& tbl) {
    auto tableView = std::make_unique<TableView>(conn);
    NativeSqlBuilder builder;
    auto const& columns = tbl.GetColumns();
    int appendCount = 0;
    builder.Append("SELECT ");
    for (auto idx = 0; idx < columns.size(); ++idx) {
        auto& col = columns[idx];
        if (col->IsVirtual()) {
            continue;
        }
        if (appendCount > 0) {
            builder.AppendComma();
        }
        builder.AppendEscaped(col->GetName());
        tableView->m_colIndexMap.insert(std::make_pair(col->GetId(), appendCount));
        if (col == &tbl.GetECClassIdColumn()) {
            tableView->m_ecClassIdCol = appendCount;
        }
        appendCount++;
    }

    builder.Append(" FROM ");
    builder.AppendEscaped(tbl.GetName());
    builder.Append(" WHERE [ROWID]=?");

    tableView->m_id = tbl.GetId();
    const auto rc = tableView->GetSqliteStmt().Prepare(conn, builder.GetSql().c_str());
    if (rc != BE_SQLITE_OK) {
        BeAssert(false && "Failed to prepare statement");
        return nullptr;
    }
    return tableView;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TableView::Ptr TableView::CreateLinkTableView(ECDbCR conn, DbTable const& tbl, RelationshipClassLinkTableMap const& rootMap) {
    auto tableView = std::make_unique<TableView>(conn);
    NativeSqlBuilder builder;
    auto const& columns = tbl.GetColumns();
    int appendCount = 0;

    const auto& sourceClassIdSysProp = rootMap.GetSourceECClassIdPropMap()->GetAs<SystemPropertyMap>();
    const auto& targetClassIdSysProp = rootMap.GetTargetECClassIdPropMap()->GetAs<SystemPropertyMap>();

    const auto& sourceIdSysProp = rootMap.GetSourceECInstanceIdPropMap()->GetAs<SystemPropertyMap>().GetDataPropertyMaps().front();
    const auto& targetIdSysProp = rootMap.GetTargetECInstanceIdPropMap()->GetAs<SystemPropertyMap>().GetDataPropertyMaps().front();

    const auto sourceClassIdProp = sourceClassIdSysProp.GetDataPropertyMaps().front();
    const auto targetClassIdProp = targetClassIdSysProp.GetDataPropertyMaps().front();

    builder.Append("SELECT ");
    for (auto idx = 0; idx < columns.size(); ++idx) {
        auto& col = columns[idx];
        if (appendCount > 0) {
            builder.AppendComma();
        }
        if (col->IsVirtual()) {
            if (col == &tbl.GetECClassIdColumn()) {
                builder.Append(rootMap.GetClass().GetId().ToHexStr())
                    .AppendSpace()
                    .AppendEscaped(col->GetName());
                tableView->m_ecClassIdCol = appendCount;
            } else if (col == &sourceClassIdProp->GetColumn()) {
                builder.Append(rootMap.GetRelationshipClass().GetSource().GetConstraintClasses().front()->GetId().ToHexStr())
                    .AppendSpace()
                    .AppendEscaped(col->GetName());
                tableView->m_ecSourceClassIdCol = appendCount;
            } else if (col == &targetClassIdProp->GetColumn()) {
                builder.Append(rootMap.GetRelationshipClass().GetTarget().GetConstraintClasses().front()->GetId().ToHexStr())
                    .AppendSpace()
                    .AppendEscaped(col->GetName());
                tableView->m_ecTargetClassIdCol = appendCount;
            } else {
                continue;
            }
            tableView->m_colIndexMap.insert(std::make_pair(col->GetId(), appendCount));
            ++appendCount;
            continue;
        }

        builder.AppendFullyQualified(tbl.GetName(), col->GetName());
        tableView->m_colIndexMap.insert(std::make_pair(col->GetId(), appendCount));
        if (col == &tbl.GetECClassIdColumn()) {
            tableView->m_ecClassIdCol = appendCount;
        } else if (col == &sourceClassIdProp->GetColumn()) {
            tableView->m_ecSourceClassIdCol = appendCount;
        } else if (col == &targetClassIdProp->GetColumn()) {
            tableView->m_ecTargetClassIdCol = appendCount;
        }
        ++appendCount;
    }

    NativeSqlBuilder sourceJoinBuilder;
    NativeSqlBuilder targetJoinBuilder;

    --appendCount;
    if (!sourceClassIdSysProp.IsMappedToClassMapTables()) {
        const auto kSourceTableAlias = "SourceClassIdTable";
        builder.AppendComma();
        builder.AppendEscaped(kSourceTableAlias)
            .AppendDot()
            .AppendEscaped(sourceClassIdProp->GetColumn().GetName());
        tableView->m_ecSourceClassIdCol = ++appendCount;

        auto& classIdTable = sourceClassIdProp->GetTable();
        sourceJoinBuilder.AppendFormatted(
            " JOIN [%s] [%s] ON [%s].[%s] = [%s].[%s]",
            classIdTable.GetName().c_str(),
            kSourceTableAlias,
            kSourceTableAlias,
            classIdTable.FindFirst(DbColumn::Kind::ECInstanceId)->GetName().c_str(),
            tbl.GetName().c_str(),
            sourceIdSysProp->GetColumn().GetName().c_str()
        );
    }
    if (!rootMap.GetTargetECClassIdPropMap()->IsMappedToClassMapTables()) {
        const auto kTargetTableAlias = "TargetClassIdTable";
        builder.AppendComma();
        builder.AppendEscaped(kTargetTableAlias)
            .AppendDot()
            .AppendEscaped(targetClassIdProp->GetColumn().GetName());
        tableView->m_ecTargetClassIdCol = ++appendCount;

        auto& classIdTable = targetClassIdProp->GetTable();
        targetJoinBuilder.AppendFormatted(
            " JOIN [%s] [%s] ON [%s].[%s] = [%s].[%s]",
            classIdTable.GetName().c_str(),
            kTargetTableAlias,
            kTargetTableAlias,
            classIdTable.FindFirst(DbColumn::Kind::ECInstanceId)->GetName().c_str(),
            tbl.GetName().c_str(),
            targetIdSysProp->GetColumn().GetName().c_str()
        );
    }

    builder.Append(" FROM ");
    builder.AppendEscaped(tbl.GetName());

    if (!sourceJoinBuilder.IsEmpty()) {
        builder.Append(sourceJoinBuilder);
    }
    if (!targetJoinBuilder.IsEmpty()) {
        builder.Append(targetJoinBuilder);
    }

    builder.AppendFormatted(" WHERE [%s].[ROWID]=?", tbl.GetName().c_str());
    tableView->m_id = tbl.GetId();
    const auto rc = tableView->GetSqliteStmt().Prepare(conn, builder.GetSql().c_str());
    if (rc != BE_SQLITE_OK) {
        ECDbLogger::Get().errorv("InstanceReader: Failed to prepare link table SQL for class '%s': %s",
            rootMap.GetClass().GetFullName(), builder.GetSql().c_str());
        BeAssert(false && "Failed to prepare statement");
        return nullptr;
    }
    return tableView;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TableView::Ptr TableView::CreateEntityTableView(ECDbCR conn, DbTable const& tbl, ClassMapCR rootMap) {
    auto tableView = std::make_unique<TableView>(conn);
    NativeSqlBuilder builder;
    auto const& columns = tbl.GetColumns();
    int appendCount = 0;
    builder.Append("SELECT ");
    for (auto idx = 0; idx < columns.size(); ++idx) {
        auto& col = columns[idx];
        if (col == &tbl.GetECClassIdColumn()) {
            if (appendCount > 0) {
                builder.AppendComma();
            }
            if (col->IsVirtual()) {
                builder.Append(rootMap.GetClass().GetId().ToHexStr())
                    .AppendSpace()
                    .AppendEscaped(col->GetName());
            } else {
                builder.AppendEscaped(col->GetName().c_str());
            }
            tableView->m_ecClassIdCol = appendCount;
        } else {
            if (col->IsVirtual()) {
                //! RelECClassId could be virtual as well.
                continue;
            }
            if (appendCount > 0) {
                builder.AppendComma();
            }
            builder.AppendEscaped(col->GetName().c_str());
            tableView->m_colIndexMap.insert(std::make_pair(col->GetId(), appendCount));
        }
        ++appendCount;
    }

    builder.Append(" FROM ");
    builder.AppendEscaped(tbl.GetName());
    builder.Append(" WHERE [ROWID]=?");

    tableView->m_id = tbl.GetId();
    const auto rc = tableView->GetSqliteStmt().Prepare(conn, builder.GetSql().c_str());
    if (rc != BE_SQLITE_OK) {
        ECDbLogger::Get().errorv("InstanceReader: Failed to prepare entity table SQL for class '%s': %s",
            rootMap.GetClass().GetFullName(), builder.GetSql().c_str());
        BeAssert(false && "Failed to prepare statement");
        return nullptr;
    }
    return tableView;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TableView::Ptr TableView::Create(ECDbCR conn, DbTable const& tbl) {
    auto getRootClassMap = [&]() -> ClassMap const* {
        ECClassId rootClassId;
        if (tbl.GetType() == DbTable::Type::Overflow) {
            rootClassId = tbl.GetLinkNode().GetParent()->GetTable().GetExclusiveRootECClassId();
        } else {
            rootClassId = tbl.GetExclusiveRootECClassId();
        }

        const auto rootClass = conn.Schemas().Main().GetClass(rootClassId);
        if (rootClass != nullptr) {
            return conn.Schemas().Main().GetClassMap(*rootClass);
        }
        return nullptr;
    };

    // virtual table
    if (tbl.GetType() == DbTable::Type::Virtual) {
        return CreateNullTableView(conn, tbl);
    }

    const auto rootClassMap = getRootClassMap();
    if (rootClassMap->GetClass().IsMixin() || rootClassMap->GetType() == ClassMap::Type::RelationshipEndTable) {
        //! NOT SUPPORTED
        ECDbLogger::Get().debugv("InstanceReader: Class '%s' is not supported for instance queries (%s).",
            rootClassMap->GetClass().GetFullName(),
            rootClassMap->GetClass().IsMixin() ? "mixin classes are not queryable" : "RelationshipEndTable map strategy is not queryable");
        return nullptr;
    }

    if (rootClassMap == nullptr) {
        return CreateTableView(conn, tbl);
    }

    if (rootClassMap->GetType() == ClassMap::Type::NotMapped) {
        ECDbLogger::Get().debugv("InstanceReader: Class '%s' is not supported for instance queries (class is not mapped).",
            rootClassMap->GetClass().GetFullName());
        return nullptr;
    }

    if (rootClassMap->GetType() == ClassMap::Type::Class) {
        return CreateEntityTableView(conn, tbl, *rootClassMap);
    }

    if (rootClassMap->GetType() == ClassMap::Type::RelationshipLinkTable) {
        return CreateLinkTableView(conn, tbl, rootClassMap->GetAs<RelationshipClassLinkTableMap>());
    }
    ECDbLogger::Get().debugv("InstanceReader: Class '%s' has unsupported ClassMap type %d for instance queries.",
        rootClassMap->GetClass().GetFullName(), Enum::ToInt(rootClassMap->GetType()));
    return nullptr;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
