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
DbColumn* DbColumnFactory::CreateColumn(ECN::ECPropertyCR ecProp, Utf8StringCR accessString, DbColumn::Type colType, DbColumn::CreateParams const& params) const
    {
    DbColumn* outColumn = nullptr;
    if (m_usesSharedColumnStrategy)
        outColumn = ApplySharedColumnStrategy(ecProp, colType, params);
    else
        outColumn = ApplyDefaultStrategy(ecProp, accessString, colType, params);

    if (outColumn == nullptr)
        return nullptr;

    CacheUsedColumn(*outColumn);
    return outColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//-----------------------------------------------------------------------------------------
DbColumn* DbColumnFactory::ApplyDefaultStrategy(ECN::ECPropertyCR ecProp, Utf8StringCR accessString, DbColumn::Type colType, DbColumn::CreateParams const& params) const
    {
    BeAssert(!params.GetColumnName().empty() && "Column name must not be null for default strategy");

    DbColumn* existingColumn = GetTable().FindColumnP(params.GetColumnName().c_str());
    if (existingColumn != nullptr && !IsColumnInUseByClassMap(*existingColumn) &&
        DbColumn::IsCompatible(existingColumn->GetType(), colType))
        {
        if (!GetTable().IsOwnedByECDb() || (existingColumn->GetConstraints().HasNotNullConstraint() == params.AddNotNullConstraint() &&
                                            existingColumn->GetConstraints().HasUniqueConstraint() == params.AddUniqueConstraint() &&
                                            existingColumn->GetConstraints().GetCollation() == params.GetCollation()))
            {
            return existingColumn;
            }

        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,"Column %s in table %s is used by multiple property maps where property name and data type matches,"
                     " but where one of the constraints NOT NULL, UNIQUE, or COLLATE differs.",
                     existingColumn->GetName().c_str(), GetTable().GetName().c_str());
        return nullptr;
        }

    bool effectiveNotNullConstraint = params.AddNotNullConstraint();
    if (params.AddNotNullConstraint() && (GetTable().HasExclusiveRootECClass() && GetTable().GetExclusiveRootECClassId() != m_classMap.GetClass().GetId()))
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Warning,
                                                        "For the ECProperty '%s' on ECClass '%s' a NOT NULL constraint is defined. The constraint cannot be enforced though because "
                                                        "the ECProperty has base ECClasses mapped to the same table.",
                                                        ecProp.GetName().c_str(), ecProp.GetClass().GetFullName());

        effectiveNotNullConstraint = false;
        }

    const ECClassId classId = GetPersistenceClassId(ecProp, accessString);
    if (!classId.IsValid())
        return nullptr;

    Utf8String resolvedColumnName, tmp;
    int retryCount = 0;
    if (SUCCESS != ResolveColumnName(tmp, params.GetColumnName(), classId, retryCount))
        return nullptr;

    resolvedColumnName = tmp;
    while (GetTable().FindColumnP(resolvedColumnName.c_str()) != nullptr)
        {
        retryCount++;
        resolvedColumnName = tmp;
        if (SUCCESS != ResolveColumnName(resolvedColumnName, params.GetColumnName(), classId, retryCount))
            return nullptr;
        }

    DbColumn* newColumn = GetTable().CreateColumn(resolvedColumnName, colType, DbColumn::Kind::DataColumn, PersistenceType::Persisted);
    if (newColumn == nullptr)
        {
        BeAssert(false && "Failed to create column");
        return nullptr;
        }

    if (effectiveNotNullConstraint)
        newColumn->GetConstraintsR().SetNotNullConstraint();

    if (params.AddUniqueConstraint())
        newColumn->GetConstraintsR().SetUniqueConstraint();

    if (params.GetCollation() != DbColumn::Constraints::Collation::Unset)
        newColumn->GetConstraintsR().SetCollation(params.GetCollation());
    
    return newColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
DbColumn* DbColumnFactory::ApplySharedColumnStrategy(ECN::ECPropertyCR prop, DbColumn::Type colType, DbColumn::CreateParams const& params) const
    {
    //Defining a col name for a shared column is a DB thing and DB CAs are taken strictly.
    if (params.IsColumnNameFromPropertyMapCA())
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                        "Failed to map ECProperty '%s:%s'. It has a 'PropertyMap' custom attribute which specifies a value for 'ColumnName'."
                                                        "'ColumnName' cannot be specified for this ECProperty because it is mapped to a column shared with other ECProperties.",
                                                        prop.GetClass().GetFullName(), prop.GetName().c_str());
        return nullptr;
        }

    //Defining a collation which is not doable is an error because this is a DB thing and DB CAs are taken strictly.
    if (params.GetCollation() != DbColumn::Constraints::Collation::Unset)
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                        "Failed to map ECProperty '%s:%s'. It has a 'PropertyMap' custom attribute which specifies a Collation constraint "
                                                        "which cannot be created because the ECProperty is mapped to a column shared with other ECProperties.",
                                                        prop.GetClass().GetFullName(), prop.GetName().c_str());
        return nullptr;
        }

    //NOT NULL and UNIQUE will soon become ECSchema level things. They are not an error, and can only be taken as hints because
    //the ECSchema level doesn't say which layer (DB or API) has to enforce it
    bool addNotNullConstraint = params.AddNotNullConstraint();
    bool addUniqueConstraint = params.AddUniqueConstraint();
    if (params.AddNotNullConstraint() || params.AddUniqueConstraint())
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Warning,
                                                        "For the ECProperty '%s' on ECClass '%s' either a NOT NULL or a UNIQUE constraint is defined. The constraint cannot be enforced though because "
                                                        "the ECProperty is mapped to a column shared with other ECProperties.",
                                                        prop.GetName().c_str(), prop.GetClass().GetFullName());

        addNotNullConstraint = false;
        addUniqueConstraint = false;
        }

    DbColumn const* reusableColumn = nullptr;
    if (TryFindReusableSharedDataColumn(reusableColumn))
        return const_cast<DbColumn*>(reusableColumn);
    
    DbColumn* col = GetTable().CreateOverflowSlaveColumn(colType);
    if (col == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    if (addNotNullConstraint)
        col->GetConstraintsR().SetNotNullConstraint();

    if (addUniqueConstraint)
        col->GetConstraintsR().SetUniqueConstraint();

    if (params.GetCollation() == DbColumn::Constraints::Collation::Unset)
        col->GetConstraintsR().SetCollation(params.GetCollation());

    return col;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
BentleyStatus DbColumnFactory::ResolveColumnName(Utf8StringR resolvedColumName, Utf8StringCR requestedColumnName, ECN::ECClassId classId, int retryCount) const
    {
    if (retryCount > 0)
        {
        BeAssert(!resolvedColumName.empty());
        resolvedColumName += SqlPrintfString("%d", retryCount);
        return SUCCESS;
        }

    if (requestedColumnName.empty())
        {
        //use name generator
        resolvedColumName.clear();
        return SUCCESS;
        }

    DbColumn const* existingColumn = GetTable().FindColumnP(requestedColumnName.c_str());
    if (existingColumn != nullptr && IsColumnInUseByClassMap(*existingColumn))
        {
        Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
        classId.ToString(classIdStr);
        resolvedColumName.Sprintf("c%s_%s", classIdStr, requestedColumnName.c_str());
        }
    else
        resolvedColumName.assign(requestedColumnName);

    return SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECClassId DbColumnFactory::GetPersistenceClassId(ECN::ECPropertyCR ecProp, Utf8StringCR propAccessString) const
    {
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