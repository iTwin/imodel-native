/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbColumnFactory.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
DbColumnFactory::DbColumnFactory(ECDbCR ecdb, ClassMapCR classMap) : m_ecdb(ecdb), m_classMap(classMap), m_usesSharedColumnStrategy(false)
    {
    TablePerHierarchyInfo const& tphInfo = m_classMap.GetMapStrategy().GetTphInfo();
    m_usesSharedColumnStrategy = tphInfo.IsValid() && tphInfo.GetShareColumnsMode() == TablePerHierarchyInfo::ShareColumnsMode::Yes;
    BeAssert(!m_usesSharedColumnStrategy || m_classMap.GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy);
    Update(false);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
DbColumn* DbColumnFactory::CreateColumn(ECN::ECPropertyCR ecProp, Utf8CP accessString, Utf8CP requestedColumnName, DbColumn::Type colType, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraints::Collation collation) const
    {
    if (!CanEnforceColumnConstraints() &&
        (addNotNullConstraint || addUniqueConstraint || collation != DbColumn::Constraints::Collation::Default))
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Warning,
                                                        "For the ECProperty '%s' on ECClass '%s' either a NOT NULL, UNIQUE or COLLATE constraint is defined. The constraint cannot be enforced though because "
                                                        "the ECProperty is mapped to a column shared with other ECProperties or the ECProperty has base ECClasses mapped to the same table.",
                                                        ecProp.GetName().c_str(), ecProp.GetClass().GetFullName());

        addNotNullConstraint = false;
        addUniqueConstraint = false;
        collation = DbColumn::Constraints::Collation::Default;
        }

    DbColumn* outColumn = nullptr;
    if (m_usesSharedColumnStrategy)
        outColumn = ApplySharedColumnStrategy(colType, addNotNullConstraint, addUniqueConstraint, collation);
    else
        outColumn = ApplyDefaultStrategy(requestedColumnName, ecProp, accessString, colType, addNotNullConstraint, addUniqueConstraint, collation);

    if (outColumn == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    CacheUsedColumn(*outColumn);
    return outColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//-----------------------------------------------------------------------------------------
DbColumn* DbColumnFactory::ApplyDefaultStrategy(Utf8CP requestedColumnName, ECN::ECPropertyCR ecProp, Utf8CP accessString, DbColumn::Type colType, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraints::Collation collation) const
    {
    BeAssert(!Utf8String::IsNullOrEmpty(requestedColumnName) && "Column name must not be null for default strategy");

    DbColumn* existingColumn = GetTable().FindColumnP(requestedColumnName);
    if (existingColumn != nullptr && !IsColumnInUseByClassMap(*existingColumn) &&
        DbColumn::IsCompatible(existingColumn->GetType(), colType))
        {
        if (!GetTable().IsOwnedByECDb() || (existingColumn->GetConstraints().HasNotNullConstraint() == addNotNullConstraint &&
                                            existingColumn->GetConstraints().HasUniqueConstraint() == addUniqueConstraint &&
                                            existingColumn->GetConstraints().GetCollation() == collation))
            {
            return existingColumn;
            }

        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Column %s in table %s is used by multiple property maps where property name and data type matches,"
                                                        " but where one of the constraints NOT NULL, UNIQUE, or COLLATE differs.",
                                                        existingColumn->GetName().c_str(), GetTable().GetName().c_str());
        return nullptr;
        }

    const ECClassId classId = GetPersistenceClassId(ecProp, accessString);
    if (!classId.IsValid())
        return nullptr;

    Utf8String resolvedColumnName, tmp;
    int retryCount = 0;
    if (SUCCESS != ResolveColumnName(tmp, requestedColumnName, classId, retryCount))
        return nullptr;

    resolvedColumnName = tmp;
    while (GetTable().FindColumnP(resolvedColumnName.c_str()) != nullptr)
        {
        retryCount++;
        resolvedColumnName = tmp;
        if (SUCCESS != ResolveColumnName(resolvedColumnName, requestedColumnName, classId, retryCount))
            return nullptr;
        }

    DbColumn* newColumn = GetTable().CreateColumn(resolvedColumnName, colType, DbColumn::Kind::DataColumn, PersistenceType::Persisted);
    if (newColumn == nullptr)
        {
        BeAssert(false && "Failed to create column");
        return nullptr;
        }

    if (addNotNullConstraint)
        newColumn->GetConstraintsR().SetNotNullConstraint();

    if (addUniqueConstraint)
        newColumn->GetConstraintsR().SetUniqueConstraint();

    newColumn->GetConstraintsR().SetCollation(collation);
    return newColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
DbColumn* DbColumnFactory::ApplySharedColumnStrategy(DbColumn::Type colType, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraints::Collation collation) const
    {
    DbColumn const* reusableColumn = nullptr;
    if (TryFindReusableSharedDataColumn(reusableColumn))
        return const_cast<DbColumn*>(reusableColumn);

    return GetTable().CreateOverflowSlaveColumn(colType, addNotNullConstraint, addUniqueConstraint, collation);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
BentleyStatus DbColumnFactory::ResolveColumnName(Utf8StringR resolvedColumName, Utf8CP requestedColumnName, ECN::ECClassId classId, int retryCount) const
    {
    if (retryCount > 0)
        {
        BeAssert(!resolvedColumName.empty());
        resolvedColumName += SqlPrintfString("%d", retryCount);
        return SUCCESS;
        }

    if (Utf8String::IsNullOrEmpty(requestedColumnName))
        {
        //use name generator
        resolvedColumName.clear();
        return SUCCESS;
        }

    DbColumn const* existingColumn = GetTable().FindColumnP(requestedColumnName);
    if (existingColumn != nullptr && IsColumnInUseByClassMap(*existingColumn))
        {
        Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
        classId.ToString(classIdStr);
        resolvedColumName.Sprintf("c%s_%s", classIdStr, requestedColumnName);
        }
    else
        resolvedColumName.assign(requestedColumnName);

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECClassId DbColumnFactory::GetPersistenceClassId(ECN::ECPropertyCR ecProp, Utf8CP accessString) const
    {
    Utf8String propAccessString(accessString);
    const size_t dotPosition = propAccessString.find(".");
    ECPropertyCP property = nullptr;
    if (dotPosition != Utf8String::npos)
        {
        //! Get root property in given accessString.
        property = m_classMap.GetClass().GetPropertyP(propAccessString.substr(0, dotPosition).c_str());
        }
    else
        property = m_classMap.GetClass().GetPropertyP(propAccessString.c_str());


    if (property == nullptr)
        {
        BeAssert(false && "Failed to find root property");
        return ECClassId();
        }

    return property->GetClass().GetId();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
bool DbColumnFactory::TryFindReusableSharedDataColumn(DbColumn const*& reusableColumn) const
    {
    reusableColumn = nullptr;
    std::vector<DbColumn const*> reusableColumns;
    for (DbColumn const* column : GetTable().GetColumns())
        {
        if (column->IsShared() && !IsColumnInUseByClassMap(*column))
            reusableColumns.push_back(column);
        }

    if (reusableColumns.empty())
        return false;

    reusableColumn = reusableColumns.front();
    return true;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void DbColumnFactory::CacheUsedColumn(DbColumn const& column) const
    {
    m_idsOfColumnsInUseByClassMap.insert(column.GetId());
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
bool DbColumnFactory::IsColumnInUseByClassMap(DbColumn const& column) const
    {
    bool isUsed = m_idsOfColumnsInUseByClassMap.find(column.GetId()) != m_idsOfColumnsInUseByClassMap.end();
    return isUsed;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void DbColumnFactory::Update(bool includeDerivedClasses) const
    {
    m_idsOfColumnsInUseByClassMap.clear();
    GetColumnsPropertyMapVisitor sharedColumnVisitor(PropertyMap::Type::Data);
    m_classMap.GetPropertyMaps().AcceptVisitor(sharedColumnVisitor);
    for (DbColumn const* columnInUse : sharedColumnVisitor.GetColumns())
        {
        if (columnInUse->IsShared() && &columnInUse->GetTable() == &GetTable())
            CacheUsedColumn(*columnInUse);
        }

    if (includeDerivedClasses)
        {
        std::vector<DbColumn const*> columns;
        GetDerivedColumnList(columns);
        for (DbColumn const* columnInUse : columns)
            {
            //WIP Why can the column ever be nullptr at all??
            if (columnInUse != nullptr)
                CacheUsedColumn(*columnInUse);
            }
        }
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       10 / 2016
//------------------------------------------------------------------------------------------
BentleyStatus DbColumnFactory::GetDerivedColumnList(std::vector<DbColumn const*>& columns) const
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(
        "SELECT c.Name FROM ec_Column c "
        "              JOIN ec_PropertyMap pm ON c.Id = pm.ColumnId "
        "              JOIN ec_ClassMap cm ON cm.ClassId = pm.ClassId "
        "              JOIN " TABLE_ClassHierarchyCache " ch ON ch.ClassId = cm.ClassId "
        "              JOIN ec_Table t on t.Id = c.TableId "
        "WHERE ch.BaseClassId=? AND t.Name=? and c.ColumnKind & " SQLVAL_DbColumn_Kind_SharedDataColumn " <> 0 "
        "GROUP BY c.Name");

    if (stmt == nullptr)
        return ERROR;
    stmt->BindId(1, m_classMap.GetClass().GetId());
    stmt->BindText(2, GetTable().GetName().c_str(), Statement::MakeCopy::No);
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        columns.push_back(GetTable().FindColumn(stmt->GetValueText(0)));
        }

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    12/2016
//------------------------------------------------------------------------------------------
bool DbColumnFactory::CanEnforceColumnConstraints() const
    {
    return !m_usesSharedColumnStrategy && (!GetTable().HasExclusiveRootECClass() || GetTable().GetExclusiveRootECClassId() == m_classMap.GetClass().GetId());
    }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
DbTable& DbColumnFactory::GetTable() const
    {
    return m_classMap.GetJoinedTable();
    }
//**************************ClassMapUsedSharedColumnQuery***********************************
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
BentleyStatus ClassMapUsedSharedColumnQuery::_Query(bset<DbColumn const*>& columns, DbTable const& table) const 
    {
    GetColumnsPropertyMapVisitor sharedColumnVisitor(PropertyMap::Type::Data);
    m_classMap.GetPropertyMaps().AcceptVisitor(sharedColumnVisitor);
    for (DbColumn const* columnInUse : sharedColumnVisitor.GetColumns())
        {
        if (columnInUse->IsShared() && &columnInUse->GetTable() == &table)
            columns.insert(columnInUse);
        }
    return SUCCESS;
    }
//*************************DerivedClassUsedSharedColumnQuery********************************
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
BentleyStatus DerivedClassUsedSharedColumnQuery::_Query(bset<DbColumn const*>& columns, DbTable const& table) const
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(
        "SELECT c.Id FROM ec_Column c "
        "              JOIN ec_PropertyMap pm ON c.Id = pm.ColumnId "
        "              JOIN ec_ClassMap cm ON cm.ClassId = pm.ClassId "
        "              JOIN " TABLE_ClassHierarchyCache " ch ON ch.ClassId = cm.ClassId "
        "              JOIN ec_Table t on t.Id = c.TableId "
        "WHERE ch.BaseClassId=? AND t.Name=? and c.ColumnKind & " SQLVAL_DbColumn_Kind_SharedDataColumn " <> 0 "
        "GROUP BY c.Name");

    if (stmt == nullptr)
        return ERROR;
    stmt->BindId(1, m_classId);
    stmt->BindText(2, table.GetName().c_str(), Statement::MakeCopy::No);
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        columns.insert(table.FindColumn(stmt->GetValueText(0)));
        }

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE