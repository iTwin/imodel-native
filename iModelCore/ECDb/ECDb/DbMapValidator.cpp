/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbMapValidator.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <vector>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                07/2017                
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::Initialize() const
    {
    if (SUCCESS != m_dbMap.GetDbSchema().LoadIndexDefs())
        return ERROR;

    //cache indexes by their columns for later validation
    for (DbTable const* table : m_dbMap.GetDbSchema().Tables())
        {
        for (std::unique_ptr<DbIndex> const& index : table->GetIndexes())
            {
            for (DbColumn const* col : index->GetColumns())
                {
                m_indexesByColumnCache[col->GetId()].insert(index.get());
                }
            }
        }

    if (m_mode == Mode::All)
        {
        CachedStatementPtr stmt = GetECDb().GetCachedStatement("SELECT Id FROM ec_Table");
        if (stmt == nullptr)
            return ERROR;

        while (stmt->Step() == BE_SQLITE_ROW)
            {
            DbTable const* table = GetDbSchema().FindTable(stmt->GetValueId<DbTableId>(0));
            if (table == nullptr)
                {
                BeAssert(false);
                return ERROR;
                }
            }

        stmt = GetECDb().GetCachedStatement("SELECT Id FROM ec_Class");
        if (stmt == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        while (stmt->Step() == BE_SQLITE_ROW)
            {
            const ECClassId classId = stmt->GetValueId<ECClassId>(0);
            ECClassCP ecClass = GetECDb().Schemas().GetClass(classId);
            if (ecClass == nullptr)
                {
                Issues().Report("Could not load ECClass for ECClassId %s from the file.", classId.ToString().c_str());
                return ERROR;
                }

            ClassMap const* classMap = m_dbMap.GetClassMap(*ecClass);
            if (classMap == nullptr)
                {
                Issues().Report("Could not load class map for ECClass %s from the file.", ecClass->GetFullName());
                return ERROR;
                }
            }

        return SUCCESS;
        }


    Statement relStmt;
    if (BE_SQLITE_OK != relStmt.Prepare(GetECDb(), "SELECT DISTINCT RelationshipClassId FROM ec_RelationshipConstraint"))
        {
        BeAssert(false);
        return ERROR;
        }

    while (relStmt.Step() == BE_SQLITE_ROW)
        {
        const ECClassId relClassId = relStmt.GetValueId<ECClassId>(0);
        ECClassCP ecClass = GetECDb().Schemas().GetClass(relClassId);
        if (ecClass == nullptr)
            {
            Issues().Report("Could not load RelationshipECClass for ECClassId %s from the file.", relClassId.ToString().c_str());
            return ERROR;
            }

        ClassMap const* classMap = m_dbMap.GetClassMap(*ecClass);
        if (classMap == nullptr)
            {
            Issues().Report("Could not load class map for RelationshipECClass %s from the file.", ecClass->GetFullName());
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::Validate() const
    {
    if (SUCCESS != Initialize())
        return ERROR;

    if (SUCCESS != ValidateDbSchema())
        return ERROR;

    return ValidateDbMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::ValidateDbSchema() const
    {
    for (DbTable const* table : GetDbSchema().Tables())
        {
        if (SUCCESS != ValidateDbTable(*table))
            return ERROR;


        for (std::unique_ptr<DbIndex> const& index : table->GetIndexes())
            {
            if (SUCCESS != ValidateDbIndex(*index))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::ValidateDbTable(DbTable const& table) const
    {
    if (table.GetName().EqualsIAscii(DBSCHEMA_NULLTABLENAME))
        {
        if (!table.GetColumns().empty() || table.GetType() != DbTable::Type::Virtual)
            {
            BeAssert(false && "Programmer error: Null table " DBSCHEMA_NULLTABLENAME " should never have columns and must be virtual");
            Issues().Report("DbTable ' " DBSCHEMA_NULLTABLENAME "' should have no column and must be virtual.");
            return ERROR;
            }

        return SUCCESS;
        }

    if (table.GetColumns().size() == 0)
        {
        Issues().Report("DbTable '%s' has no columns.", table.GetName().c_str());
        return ERROR;
        }

    bvector<Utf8String> physicalColumns;
    GetECDb().GetColumns(physicalColumns, table.GetName().c_str());
    bset<Utf8String, CompareIUtf8Ascii> physicalColumnsSet(physicalColumns.begin(), physicalColumns.end());
    int nonVirtualColumnCount = 0;
    for (DbColumn const* column : table.GetColumns())
        {
        if (SUCCESS != ValidateDbColumn(*column, physicalColumnsSet))
            return ERROR;

        if (!column->IsVirtual())
            nonVirtualColumnCount++;
        }

    if (table.FindFirst(DbColumn::Kind::ECInstanceId) == nullptr)
        {
        Issues().Report("DbTable '%s' does not have a column of kind 'ECInstanceId'", table.GetName().c_str());
        return ERROR;
        }

    if (table.FindFirst(DbColumn::Kind::ECClassId) == nullptr)
        {
        Issues().Report("DbTable '%s' does not have a column of kind 'ECClassId'", table.GetName().c_str());
        return ERROR;
        }

    switch (table.GetType())
        {
            case DbTable::Type::Existing:
            {
            if (!GetECDb().TableExists(table.GetName().c_str()))
                {
                Issues().Report("DbTable '%s' is if type 'Existing' and therefore must exist in the file.", table.GetName().c_str());
                return ERROR;
                }

            if (table.GetLinkNode().GetParent() != nullptr)
                {
                Issues().Report("DbTable '%s' is of type 'Existing' and therefore it must not have parent table.", table.GetName().c_str());
                return ERROR;
                }

            break;
            }

            case DbTable::Type::Joined:
            {
            if (!GetECDb().TableExists(table.GetName().c_str()))
                {
                Issues().Report("DbTable '%s' is if type 'Joined' and therefore must exist in the file.", table.GetName().c_str());
                return ERROR;
                }

            if (table.GetLinkNode().GetParent() == nullptr)
                {
                Issues().Report("DbTable '%s' is of type 'Joined' and therefore it must have parent table.", table.GetName().c_str());
                return ERROR;
                }

            if (table.GetLinkNode().GetChildren().size() > 1)
                {
                Issues().Report("DbTable '%s' is of type 'Joined' and therefore it must not have more than one child table.", table.GetName().c_str());
                return ERROR;
                }

            if (nonVirtualColumnCount != (int) physicalColumns.size())
                {
                Issues().Report("DbTable '%s' has %d non-virtual columns, but the physical table has %d columns.", table.GetName().c_str(),
                                nonVirtualColumnCount, (int) physicalColumns.size());
                return ERROR;
                }

            break;
            }

            case DbTable::Type::Overflow:
            {
            if (!GetECDb().TableExists(table.GetName().c_str()))
                {
                Issues().Report("DbTable '%s' is if type 'Overflow' and therefore must exist in the file.", table.GetName().c_str());
                return ERROR;
                }


            if (table.GetLinkNode().GetParent() == nullptr)
                {
                Issues().Report("DbTable '%s' is of type 'Overflow' and therefore must have a parent table.", table.GetName().c_str());
                return ERROR;
                }

            if (!table.GetLinkNode().GetChildren().empty())
                {
                Issues().Report("DbTable '%s' is of type 'Overflow' and therefore must not have any child tables.", table.GetName().c_str());
                return ERROR;
                }

            if (nonVirtualColumnCount != (int) physicalColumns.size())
                {
                Issues().Report("DbTable '%s' has %d non-virtual columns, but the physical table has %d columns.", table.GetName().c_str(),
                                nonVirtualColumnCount, (int) physicalColumns.size());
                return ERROR;
                }

            break;
            }

            case DbTable::Type::Primary:
            {
            if (!GetECDb().TableExists(table.GetName().c_str()))
                {
                Issues().Report("DbTable '%s' is if type 'Primary' and therefore must exist in the file.", table.GetName().c_str());
                return ERROR;
                }

            if (table.GetLinkNode().GetParent() != nullptr)
                {
                Issues().Report("DbTable '%s' is of type 'Primary' and therefore must not have parent table.", table.GetName().c_str());
                return ERROR;
                }

            if (nonVirtualColumnCount != (int) physicalColumns.size())
                {
                Issues().Report("DbTable '%s' has %d non-virtual columns, but the physical table has %d columns.", table.GetName().c_str(),
                                nonVirtualColumnCount, (int) physicalColumns.size());
                return ERROR;
                }

            break;
            }

            case DbTable::Type::Virtual:
            {
            if (nonVirtualColumnCount != 0)
                {
                Issues().Report("DbTable '%s' is of type 'Virtual' and therefore all its DbColumns must be virtual as well.", table.GetName().c_str());
                return ERROR;
                }

            if (table.GetLinkNode().GetParent() != nullptr && !table.GetLinkNode().GetChildren().empty())
                {
                Issues().Report("DbTable '%s' is of type 'Virtual' and therefore must neither have a parent table nor a child table.", table.GetName().c_str());
                return ERROR;
                }

            break;
            }

            default:
            {
            Issues().Report("DbTable '%s' has unsupported DbTable::Type: %d.", table.GetName().c_str(), Enum::ToInt(table.GetType()));
            return ERROR;
            }
        }


    for (DbConstraint const* constraint : table.GetConstraints())
        {
        if (SUCCESS != ValidateDbConstraint(*constraint))
            return ERROR;
        }

    for (DbTrigger const* trigger : table.GetTriggers())
        {
        if (SUCCESS != ValidateDbTrigger(*trigger))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::ValidateDbColumn(DbColumn const& column, bset<Utf8String, CompareIUtf8Ascii> const& physicalColumns) const
    {
    DbTable::Type const tableType = column.GetTable().GetType();
    if (!column.IsVirtual() && tableType != DbTable::Type::Virtual)
        {
        if (physicalColumns.find(column.GetName()) == physicalColumns.end())
            {
            Issues().Report("Non-virtual DbTable '%s' has non-virtual column '%s' which does not exist in the file.", column.GetTable().GetName().c_str(), column.GetName().c_str());
            return ERROR;
            }
        }

    if (column.IsShared())
        {
        if (tableType == DbTable::Type::Existing || tableType == DbTable::Type::Virtual)
            {
            Issues().Report("The table '%s' is of type 'Existing' or 'Virtual', but its column '%s' is a shared column. This is invalid.", column.GetTable().GetName().c_str(), column.GetName().c_str());
            return ERROR;
            }

        if (column.GetConstraints().HasNotNullConstraint())
            {
            Issues().Report("Column '%s.%s' is a shared column and has the 'NOT NULL' constraint. This is not valid for shared columns.", column.GetTable().GetName().c_str(), column.GetName().c_str());
            return ERROR;
            }

        if (column.GetConstraints().HasUniqueConstraint())
            {
            Issues().Report("Column '%s.%s' is a shared column and has the 'UNIQUE' constraint. This is not valid for shared columns.", column.GetTable().GetName().c_str(), column.GetName().c_str());
            return ERROR;
            }
        }

    if (column.GetType() != DbColumn::Type::Any &&
        column.GetType() != DbColumn::Type::Blob &&
        column.GetType() != DbColumn::Type::Boolean &&
        column.GetType() != DbColumn::Type::Integer &&
        column.GetType() != DbColumn::Type::Real &&
        column.GetType() != DbColumn::Type::Text &&
        column.GetType() != DbColumn::Type::TimeStamp)
        {
        Issues().Report("DbColumn '%s.%s' has unsupported DbColumn::Type %d.", column.GetTable().GetName().c_str(), column.GetName().c_str(),
                        Enum::ToInt(column.GetType()));
        return ERROR;
        }

    if (column.GetConstraints().GetCollation() != DbColumn::Constraints::Collation::Binary &&
        column.GetConstraints().GetCollation() != DbColumn::Constraints::Collation::NoCase &&
        column.GetConstraints().GetCollation() != DbColumn::Constraints::Collation::RTrim &&
        column.GetConstraints().GetCollation() != DbColumn::Constraints::Collation::Unset)
        {
        Issues().Report("DbColumn '%s.%s' has unsupported DbColumn::Constraints::Collation %d.", column.GetTable().GetName().c_str(), column.GetName().c_str(),
                        Enum::ToInt(column.GetConstraints().GetCollation()));
        return ERROR;
        }

    //Kind used to be an flags enum, but no longer is. so verify that the kind is on only the discrete enum values
    DbColumn::Kind actualKind = column.GetKind();
    if (actualKind != DbColumn::Kind::Default && actualKind != DbColumn::Kind::ECClassId &&
        actualKind != DbColumn::Kind::ECInstanceId && actualKind != DbColumn::Kind::SharedData)
        {
        Issues().Report("DbColumn '%s.%s' has an invalid DbColumn::Kind: %d", column.GetTable().GetName().c_str(), column.GetName().c_str(), Enum::ToInt(actualKind));
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::ValidateDbConstraint(DbConstraint const& constraint) const
    {
    switch (constraint.GetType())
        {
            case DbConstraint::Type::ForeignKey:
                return ValidateForeignKeyDbConstraint(static_cast<ForeignKeyDbConstraint const&> (constraint));

            case DbConstraint::Type::PrimaryKey:
                return ValidatePrimaryKeyDbConstraint(static_cast<PrimaryKeyDbConstraint const&> (constraint));

            default:
                Issues().Report("DbTable '%s' has unsupported DbColumn::Constraint %d.", constraint.GetTable().GetName().c_str(), Enum::ToInt(constraint.GetType()));
                return ERROR;
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::ValidateForeignKeyDbConstraint(ForeignKeyDbConstraint const& constraint) const
    {
    if (constraint.GetFkColumns().empty())
        {
        Issues().Report("DbTable '%s' has a foreign key constraint without any columns.", constraint.GetTable().GetName().c_str());
        return ERROR;
        }

    if (constraint.GetFkColumns().size() != constraint.GetReferencedTableColumns().size())
        {
        Issues().Report("DbTable '%s' has a foreign key constraint with %d foreign key columns, but %d referenced columns in the referenced table.", constraint.GetTable().GetName().c_str(),
            (int) constraint.GetFkColumns().size(), (int) constraint.GetReferencedTableColumns().size());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::ValidatePrimaryKeyDbConstraint(PrimaryKeyDbConstraint const& constraint) const
    {
    if (constraint.GetColumns().empty())
        {
        Issues().Report("DbTable '%s' has a primary key constraint without any columns.", constraint.GetTable().GetName().c_str());
        return ERROR;
        }

    if (constraint.GetColumns().size() != 1)
        {
        Issues().Report("DbTable '%s' has a primary key constraint with more than one column. This is not yet supported by ECDb.", constraint.GetTable().GetName().c_str());
        return ERROR;
        }

    if (constraint.GetColumns().front()->GetType() != DbColumn::Type::Integer)
        {
        Issues().Report("DbTable '%s' has a primary key which is not of type Integer. This is not yet supported by ECDb.", constraint.GetTable().GetName().c_str());
        return ERROR;
        }

    if (constraint.GetColumns().front() != constraint.GetTable().FindFirst(DbColumn::Kind::ECInstanceId))
        {
        Issues().Report("DbTable '%s' has a primary key which is not of kind 'ECInstanceId'.", constraint.GetTable().GetName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::ValidateDbIndex(DbIndex const& index) const
    {
    if (index.GetColumns().empty())
        {
        Issues().Report("Index '%s' must at least specify one column.", index.GetName().c_str());
        return ERROR;
        }

    if (index.GetTable().GetType() == DbTable::Type::Virtual)
        return SUCCESS;

    bset<DbTable const*> tables;
    for (DbColumn const* col : index.GetColumns())
        {
        if (col->GetPersistenceType() == PersistenceType::Virtual)
            {
            Issues().Report("Index '%s' is defined on a virtual column (which does not exist): %s.%s.", index.GetName().c_str(), col->GetTable().GetName().c_str(), col->GetName().c_str());
            return ERROR;
            }

        tables.insert(&col->GetTable());
        if (tables.size() > 1)
            {
            Issues().Report("Index '%s' is defined on columns from different tables.", index.GetName().c_str());
            return ERROR;
            }
        }

    if (!index.IsAutoGenerated() && index.HasClassId())
        {
        ECClassCP ecClass = GetECDb().Schemas().GetClass(index.GetClassId());
        if (ecClass == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        ClassMap const* classMap = m_dbMap.GetClassMap(*ecClass);
        if (classMap == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        if (ecClass->IsEntityClass() && ecClass->GetEntityClassCP()->IsMixin())
            {
            Issues().Report("Used-defined index '%s' is defined on a mixin class ('%s'). This is not supported.", index.GetName().c_str(), ecClass->GetFullName());
            return ERROR;
            }

        if (classMap->GetType() == ClassMap::Type::RelationshipEndTable)
            {
            Issues().Report("Used-defined index '%s' is defined on a foreign key type relationship class ('%s'). This is not supported.", index.GetName().c_str(), ecClass->GetFullName());
            return ERROR;
            }

        if (!classMap->GetMapStrategy().IsTablePerHierarchy() && ecClass->GetClassModifier() != ECClassModifier::Sealed)
            {
            Issues().Report("Used-defined index '%s' is defined on class '%s' which is not sealed or which is not mapped with strategy 'TablePerHierarchy'. Indexes can only be defined on sealed classes or on classes mapped as 'TablePerHierarchy'",
                            index.GetName().c_str(), ecClass->GetFullName());
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::ValidateDbMap() const
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), "SELECT count(*) FROM " TABLE_Class))
        {
        BeAssert(false);
        return ERROR;
        }
    if (BE_SQLITE_ROW != stmt.Step())
        {
        BeAssert(false);
        return ERROR;
        }
    const int classCount = stmt.GetValueInt(0);
    stmt.Finalize();
    if (BE_SQLITE_OK != stmt.Prepare(GetECDb(), "SELECT count(*) FROM " TABLE_ClassMap))
        {
        BeAssert(false);
        return ERROR;
        }
    if (BE_SQLITE_ROW != stmt.Step())
        {
        BeAssert(false);
        return ERROR;
        }
    const int classMapCount = stmt.GetValueInt(0);
    stmt.Finalize();

    if (classCount != classMapCount)
        {
        Issues().Report("The system tables " TABLE_Class " and " TABLE_ClassMap " must have the same number of rows, but they don't: "
                        TABLE_Class ": %d rows, " TABLE_ClassMap ": %d rows.", classCount, classMapCount);
        return ERROR;
        }

    std::vector<ClassMap const*> classMaps;
    for (bpair<ECClassId, ClassMapPtr> const& entry : m_dbMap.GetClassMapCache())
        {
        classMaps.push_back(entry.second.get());
        }

    for (ClassMap const* classMap : classMaps)

    if (SUCCESS != ValidateClassMap(*classMap))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::ValidateClassMap(ClassMap const& classMap) const
    {
    if (classMap.GetType() == ClassMap::Type::NotMapped)
        {
        if (classMap.GetPropertyMaps().Size() != 0)
            {
            Issues().Report("Class '%s' is not mapped and therefore must not have property maps.", classMap.GetClass().GetFullName());
            return ERROR;
            }

        return SUCCESS;
        }

    int dataPropertyMapCount = 0;
    int systemPropertyMapCount = 0;
    for (PropertyMap const* propertyMap : classMap.GetPropertyMaps())
        {
        if (propertyMap->IsData())
            dataPropertyMapCount++;
        else if (propertyMap->IsSystem())
            systemPropertyMapCount++;
        else
            BeAssert(false);
        }


    switch (classMap.GetType())
        {
            case ClassMap::Type::Class:
            {
            const int expectedSystemPropertyMapCount = 2;
            if (expectedSystemPropertyMapCount != systemPropertyMapCount)
                {
                Issues().Report("The class map for '%s' must have %d system properties, but has %d.", classMap.GetClass().GetFullName(),
                                expectedSystemPropertyMapCount, systemPropertyMapCount);
                return ERROR;
                }

            const int propCount = (int) classMap.GetClass().GetPropertyCount(true);
            if (dataPropertyMapCount != propCount)
                {
                Issues().Report("The number of property maps for ECClass '%s' does not match the number of properties. Property maps: %d, properties: %d.", classMap.GetClass().GetFullName(),
                                dataPropertyMapCount, propCount);
                return ERROR;
                }

            if (classMap.GetECInstanceIdPropertyMap() == nullptr)
                {
                Issues().Report("The class map of '%s' does not have an " ECDBSYS_PROP_ECInstanceId " property map.", classMap.GetClass().GetFullName());
                return ERROR;
                }

            if (classMap.GetECClassIdPropertyMap() == nullptr)
                {
                Issues().Report("The class map of '%s' does not have an " ECDBSYS_PROP_ECClassId " property map.", classMap.GetClass().GetFullName());
                return ERROR;
                }
            break;
            }
            case ClassMap::Type::RelationshipEndTable:
            {
            const int expectedSystemPropertyMapCount = 6;

            if (expectedSystemPropertyMapCount != systemPropertyMapCount)
                {
                Issues().Report("The class map for the ECRelationshipClass '%s' must have %d system property maps, but has %d.", classMap.GetClass().GetFullName(),
                                expectedSystemPropertyMapCount, systemPropertyMapCount);
                return ERROR;
                }

            if (dataPropertyMapCount != 0)
                {
                Issues().Report("The class map for the foreign key type ECRelationshipClass '%s' must not have data property maps, but has %d.", classMap.GetClass().GetFullName(),
                                dataPropertyMapCount);
                return ERROR;
                }

            if (SUCCESS != ValidateRelationshipClassEndTableMap(classMap.GetAs<RelationshipClassEndTableMap>()))
                return ERROR;

            break;
            }
            case ClassMap::Type::RelationshipLinkTable:
            {
            const int expectedSystemPropertyMapCount = 6;

            if (expectedSystemPropertyMapCount != systemPropertyMapCount)
                {
                Issues().Report("The class map for the ECRelationshipClass '%s' must have %d system property maps, but has %d.", classMap.GetClass().GetFullName(),
                                expectedSystemPropertyMapCount, systemPropertyMapCount);
                return ERROR;
                }

            if (dataPropertyMapCount != classMap.GetClass().GetPropertyCount(true))
                {
                Issues().Report("ECClass '%s' has at least one property for which no property map exists.", classMap.GetClass().GetFullName());
                return ERROR;
                }

            if (SUCCESS != ValidateRelationshipClassLinkTableMap(classMap.GetAs<RelationshipClassLinkTableMap>()))
                return ERROR;

            break;
            }

            default:
                Issues().Report("The class map of '%s' has the unknown ClassMap::Type::%d.", classMap.GetClass().GetFullName(), Enum::ToInt(classMap.GetType()));
                return ERROR;
        }

    bmap<DbColumn const*, SingleColumnDataPropertyMap const*> duplicateColumnMappings;
    ECClassCR ecClass = classMap.GetClass();
    for (PropertyMap const* propMap : classMap.GetPropertyMaps())
        {
        //ignore inherited properties as they were validated on their base classes
        if (&propMap->GetProperty().GetClass() != &ecClass)
            continue;

        if (SUCCESS != ValidatePropertyMap(*propMap, duplicateColumnMappings))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::ValidateRelationshipClassEndTableMap(RelationshipClassEndTableMap const& relMap) const
    {
    if (relMap.GetECInstanceIdPropertyMap() == nullptr)
        {
        Issues().Report("The class map of '%s' does not have an " ECDBSYS_PROP_ECInstanceId " property map.", relMap.GetClass().GetFullName());
        return ERROR;
        }

    if (relMap.GetECClassIdPropertyMap() == nullptr)
        {
        Issues().Report("The class map of '%s' does not have an " ECDBSYS_PROP_ECClassId " property map.", relMap.GetClass().GetFullName());
        return ERROR;
        }


    if (relMap.GetPropertyMaps().Find(ECDBSYS_PROP_SourceECInstanceId) == nullptr)
        {
        Issues().Report("The class map of the ECRelationshipClass '%s' does not have a " ECDBSYS_PROP_SourceECInstanceId " property map.", relMap.GetClass().GetFullName());
        return ERROR;
        }

    if (relMap.GetPropertyMaps().Find(ECDBSYS_PROP_SourceECClassId) == nullptr)
        {
        Issues().Report("The class map of ECRelationshipClass '%s' does not have a " ECDBSYS_PROP_SourceECClassId " property map.", relMap.GetClass().GetFullName());
        return ERROR;
        }

    if (relMap.GetPropertyMaps().Find(ECDBSYS_PROP_TargetECInstanceId) == nullptr)
        {
        Issues().Report("The class map of ECRelationshipClass '%s' does not have a " ECDBSYS_PROP_TargetECInstanceId " property map.", relMap.GetClass().GetFullName());
        return ERROR;
        }

    if (relMap.GetPropertyMaps().Find(ECDBSYS_PROP_TargetECClassId) == nullptr)
        {
        Issues().Report("The class map of ECRelationshipClass '%s' does not have a " ECDBSYS_PROP_TargetECClassId " property map.", relMap.GetClass().GetFullName());
        return ERROR;
        }




    const std::vector<DbTable const*> otherEndTables = RelationshipClassEndTableMap::PartitionView::GetOtherEndTables(relMap);
    if (otherEndTables.size() > 1)
        {
        Utf8String tableStr;
        bool isFirstTable = true;
        for (DbTable const* table : otherEndTables)
            {
            if (!isFirstTable)
                tableStr.append(",");

            tableStr.append(table->GetName().c_str());
            isFirstTable = false;
            }

        Issues().Report("The class map for the foreign key type ECRelationshipClass '%s' maps to more than one table on the %s constraint: %s.",
                        relMap.GetClass().GetFullName(), relMap.GetReferencedEnd() == ECRelationshipEnd_Source ? "source" : "target",
                        tableStr.c_str());
        return ERROR;
        }

    if (relMap.GetTables().size() != 1 || relMap.GetTables().front()->GetType() != DbTable::Type::Virtual)
        {
        Issues().Report("The foreign key type ECRelationshipClass '%s' does not map to a single virtual table: %s.", relMap.GetClass().GetFullName());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::ValidateRelationshipClassLinkTableMap(RelationshipClassLinkTableMap const& relMap) const
    {
    if (relMap.GetECInstanceIdPropertyMap() == nullptr)
        {
        Issues().Report("The class map of '%s' does not have an " ECDBSYS_PROP_ECInstanceId " property map.", relMap.GetClass().GetFullName());
        return ERROR;
        }

    if (relMap.GetECClassIdPropertyMap() == nullptr)
        {
        Issues().Report("The class map of '%s' does not have an " ECDBSYS_PROP_ECClassId " property map.", relMap.GetClass().GetFullName());
        return ERROR;
        }


    if (relMap.GetPropertyMaps().Find(ECDBSYS_PROP_SourceECInstanceId) == nullptr)
        {
        Issues().Report("The class map of the ECRelationshipClass '%s' does not have a " ECDBSYS_PROP_SourceECInstanceId " property map.", relMap.GetClass().GetFullName());
        return ERROR;
        }

    if (relMap.GetPropertyMaps().Find(ECDBSYS_PROP_SourceECClassId) == nullptr)
        {
        Issues().Report("The class map of ECRelationshipClass '%s' does not have a " ECDBSYS_PROP_SourceECClassId " property map.", relMap.GetClass().GetFullName());
        return ERROR;
        }

    if (relMap.GetPropertyMaps().Find(ECDBSYS_PROP_TargetECInstanceId) == nullptr)
        {
        Issues().Report("The class map of ECRelationshipClass '%s' does not have a " ECDBSYS_PROP_TargetECInstanceId " property map.", relMap.GetClass().GetFullName());
        return ERROR;
        }

    if (relMap.GetPropertyMaps().Find(ECDBSYS_PROP_TargetECClassId) == nullptr)
        {
        Issues().Report("The class map of ECRelationshipClass '%s' does not have a " ECDBSYS_PROP_TargetECClassId " property map.", relMap.GetClass().GetFullName());
        return ERROR;
        }


    const std::set<DbTable const*> sourceTables = RelationshipMappingInfo::GetTablesFromRelationshipEnd(relMap.GetDbMap(), m_schemaImportContext, relMap.GetRelationshipClass().GetSource(), true);
    if (sourceTables.size() > 1)
        {
        Utf8String tableStr;
        bool isFirstTable = true;
        for (DbTable const* table : sourceTables)
            {
            if (!isFirstTable)
                tableStr.append(",");

            tableStr.append(table->GetName().c_str());
            isFirstTable = false;
            }

        Issues().Report("The class map for the link table ECRelationshipClass '%s' maps to more than one table on the source constraint: %s.",
                        relMap.GetClass().GetFullName(), tableStr.c_str());
        return ERROR;
        }

    const std::set<DbTable const*> targetTables = RelationshipMappingInfo::GetTablesFromRelationshipEnd(relMap.GetDbMap(), m_schemaImportContext, relMap.GetRelationshipClass().GetTarget(), true);
    if (targetTables.size() > 1)
        {
        Utf8String tableStr;
        bool isFirstTable = true;
        for (DbTable const* table : targetTables)
            {
            if (!isFirstTable)
                tableStr.append(",");

            tableStr.append(table->GetName().c_str());
            isFirstTable = false;
            }

        Issues().Report("The class map for the link table ECRelationshipClass '%s' maps to more than one table on the target constraint: %s.",
                        relMap.GetClass().GetFullName(), tableStr.c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                           06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::ValidatePropertyMap(PropertyMap const& propertyMap, bmap<DbColumn const*, SingleColumnDataPropertyMap const*>& duplicateColumnMappings) const
    {
    if (propertyMap.IsSystem())
        {
        if (propertyMap.GetAs<SystemPropertyMap>().GetDataPropertyMaps().empty())
            {
            Issues().Report("Invalid system property map '%s.%s'. It is not mapped to any column.",
                            propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str());
            return ERROR;
            }
        }

    switch (propertyMap.GetType())
        {
            case PropertyMap::Type::ConstraintECInstanceId:
            case PropertyMap::Type::ConstraintECClassId:
            case PropertyMap::Type::NavigationId:
            case PropertyMap::Type::NavigationRelECClassId:
            case PropertyMap::Type::Primitive:
            case PropertyMap::Type::PrimitiveArray:
            case PropertyMap::Type::StructArray:
                break;

            case PropertyMap::Type::ECInstanceId:
            {
            ECInstanceIdPropertyMap const& prop = propertyMap.GetAs<ECInstanceIdPropertyMap>();
            for (SystemPropertyMap::PerTableIdPropertyMap const* perTablePropMap : prop.GetDataPropertyMaps())
                {
                if (perTablePropMap->GetColumn().GetKind() != DbColumn::Kind::ECInstanceId)
                    {
                    Issues().Report("The ECInstanceId property map '%s.%s' must map to columns of Kind 'DbColumn::Kind::ECInstanceId'. Violating column: %s.%s ", propertyMap.GetClassMap().GetClass().GetFullName(),
                                    propertyMap.GetAccessString().c_str(), perTablePropMap->GetColumn().GetTable().GetName().c_str(), perTablePropMap->GetColumn().GetName().c_str());
                    return ERROR;
                    }

                if (SUCCESS != ValidatePropertyMap(*perTablePropMap, duplicateColumnMappings))
                    return ERROR;
                }

            break;
            }


            case PropertyMap::Type::ECClassId:
            {
            ECClassIdPropertyMap const& prop = propertyMap.GetAs<ECClassIdPropertyMap>();
            for (SystemPropertyMap::PerTableIdPropertyMap const* perTablePropMap : prop.GetDataPropertyMaps())
                {
                if (perTablePropMap->GetColumn().GetKind() != DbColumn::Kind::ECClassId)
                    {
                    Issues().Report("The ECClassId property map '%s.%s' must map to columns of Kind 'DbColumn::Kind::ECClassId'. Violating column: %s.%s ", propertyMap.GetClassMap().GetClass().GetFullName(),
                                    propertyMap.GetAccessString().c_str(), perTablePropMap->GetColumn().GetTable().GetName().c_str(), perTablePropMap->GetColumn().GetName().c_str());
                    return ERROR;
                    }

                if (SUCCESS != ValidatePropertyMap(*perTablePropMap, duplicateColumnMappings))
                    return ERROR;
                }

            break;
            }

            case PropertyMap::Type::Navigation:
            {
            if (SUCCESS != ValidateNavigationPropertyMap(propertyMap.GetAs<NavigationPropertyMap>()))
                return ERROR;

            break;
            }

            case PropertyMap::Type::Point2d:
            {
            Point2dPropertyMap const& propMap = propertyMap.GetAs<Point2dPropertyMap>();
            if (propMap.Find(ECDBSYS_PROP_PointX) == nullptr || propMap.Find(ECDBSYS_PROP_PointY) == nullptr)
                {
                Issues().Report("The Point2d property map '%s.%s' does not have 'X' and 'Y' member property maps.", propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str());
                return ERROR;
                }

            break;
            }

            case PropertyMap::Type::Point3d:
            {
            Point3dPropertyMap const& propMap = propertyMap.GetAs<Point3dPropertyMap>();
            if (propMap.Find(ECDBSYS_PROP_PointX) == nullptr || propMap.Find(ECDBSYS_PROP_PointY) == nullptr || propMap.Find(ECDBSYS_PROP_PointY) == nullptr)
                {
                Issues().Report("The Point3d property map '%s.%s' does not have'X', 'Y' and 'Z' member property maps.", propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str());
                return ERROR;
                }

            break;
            }

            case PropertyMap::Type::Struct:
            {
            StructPropertyMap const& propMap = propertyMap.GetAs<StructPropertyMap>();
            const size_t expectedPropertyCount = propMap.GetProperty().GetAsStructProperty()->GetType().GetPropertyCount();
            if (propMap.Size() != expectedPropertyCount)
                {
                Issues().Report("The struct property map '%s.%s' has %d member property maps, although the corresponding struct has %d properties.",
                                propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str(),
                                (int) propMap.Size(), (int) expectedPropertyCount);
                return ERROR;
                }

            break;
            }

            case PropertyMap::Type::SystemPerTableId:
            case PropertyMap::Type::SystemPerTableClassId:
            {
            DbColumn const& col = propertyMap.GetAs<SystemPropertyMap::PerTableIdPropertyMap>().GetColumn();
            if (col.IsShared())
                {
                Issues().Report("The system property map '%s.%s' maps to the shared column '%s.%s' which is invalid.",
                                propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str(),
                                col.GetTable().GetName().c_str(), col.GetName().c_str());
                return ERROR;
                }

            //for existing tables, the DbColumn type cannot always be determined
            if (col.GetTable().GetType() != DbTable::Type::Existing && col.GetType() != DbColumn::Type::Integer)
                {
                Issues().Report("The system property map '%s.%s' maps to a column which is not of type 'Integer'. Violating column '%s.%s'.",
                                propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str(),
                                col.GetTable().GetName().c_str(), col.GetName().c_str());
                return ERROR;
                }
            break;
            }

            default:
                Issues().Report("The PropertyMap '%s.%s' has the unknown PropertyMap::Type::%d.", propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str(), Enum::ToInt(propertyMap.GetType()));
                return ERROR;
        }


    if (Enum::Contains(PropertyMap::Type::SingleColumnData, propertyMap.GetType()))
        {
        SingleColumnDataPropertyMap const& propMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
        DbColumn const& column = propMap.GetColumn();
        auto it = duplicateColumnMappings.find(&column);
        if (it == duplicateColumnMappings.end())
            duplicateColumnMappings[&column] = &propMap;
        else
            {
            PropertyMap const* duplicatePropMap = it->second;
            Issues().Report("Invalid property mapping. Multiple properties map to the column '%s.%s'. Violating properties: %s.%s and %s.%s",
                            column.GetTable().GetName().c_str(), column.GetName().c_str(),
                            propertyMap.GetClassMap().GetClass().GetFullName(), propertyMap.GetAccessString().c_str(),
                            duplicatePropMap->GetClassMap().GetClass().GetFullName(), duplicatePropMap->GetAccessString().c_str());
            return ERROR;
            }
        }
    else if (Enum::Contains(PropertyMap::Type::CompoundData, propertyMap.GetType()))
        {
        for (PropertyMap const* memberPropMap : propertyMap.GetAs<CompoundDataPropertyMap>())
            {
            if (SUCCESS != ValidatePropertyMap(*memberPropMap, duplicateColumnMappings))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Krischan.Eberle                       07/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::ValidateNavigationPropertyMap(NavigationPropertyMap const& propMap) const
    {
    if (propMap.Find(ECDBSYS_PROP_NavPropId) == nullptr || propMap.Find(ECDBSYS_PROP_NavPropRelECClassId) == nullptr)
        {
        Issues().Report("The navigation property map '%s.%s' does not have 'Id' and 'RelECClassId' member property maps.", propMap.GetClassMap().GetClass().GetFullName(), propMap.GetAccessString().c_str());
        return ERROR;
        }

    //for existing tables we don't create constraints and such, so don't validate them in that case
    if (propMap.GetTable().GetType() == DbTable::Type::Existing)
        {
        if (propMap.HasForeignKeyConstraint())
            {
            Issues().Report("The navigation property map '%s.%s' has the ForeignKeyConstraint custom attribute which is not allowed for classes with MapStrategy 'ExistingTable'.", propMap.GetClassMap().GetClass().GetFullName(), propMap.GetAccessString().c_str());
            return ERROR;
            }

        return SUCCESS;
        }

    const bool isPhysicalFk = propMap.HasForeignKeyConstraint();
    NavigationPropertyMap::IdPropertyMap const& idPropMap = propMap.GetIdPropertyMap();
    DbColumn const& idCol = idPropMap.GetColumn();

    NavigationPropertyMap::RelECClassIdPropertyMap const& relClassIdPropMap = propMap.GetRelECClassIdPropertyMap();
    DbColumn const& relClassIdCol = relClassIdPropMap.GetColumn();

    if (propMap.GetProperty().GetAsNavigationProperty()->GetRelationshipClass()->GetClassModifier() == ECClassModifier::Sealed)
        {
        if (relClassIdCol.GetPersistenceType() == PersistenceType::Physical)
            {
            Issues().Report("The navigation property '%s.%s' has the RelECClassId column '%s.%s' which should not exist because the navigation property's relationship is sealed.",
                            propMap.GetClassMap().GetClass().GetFullName(), propMap.GetAccessString().c_str(), relClassIdCol.GetTable().GetName().c_str(), relClassIdCol.GetName().c_str());
            return ERROR;
            }
        }
    else
        {
        //if the table per se is virtual (because it is a mixin or abstract class), then we must not do the check
        if (relClassIdCol.GetTable().GetType() != DbTable::Type::Virtual && relClassIdCol.GetPersistenceType() == PersistenceType::Virtual)
            {
            Issues().Report("The navigation property '%s.%s' has the RelECClassId column '%s.%s' which is virtual, but should exist in the table because the navigation property's relationship is not sealed.",
                            propMap.GetClassMap().GetClass().GetFullName(), propMap.GetAccessString().c_str(), relClassIdCol.GetTable().GetName().c_str(), relClassIdCol.GetName().c_str());
            return ERROR;
            }
        }

    if (isPhysicalFk)
        {
        if (idCol.IsShared() || relClassIdCol.IsShared())
            {
            Issues().Report("The navigation property '%s.%s' maps to a physical foreign key. But its foreign key column '%s.%s' or its RelECClassId column '%s.%s' is a shared column which is invalid.",
                            propMap.GetClassMap().GetClass().GetFullName(), propMap.GetAccessString().c_str(),
                            idCol.GetTable().GetName().c_str(), idCol.GetName().c_str(), relClassIdCol.GetTable().GetName().c_str(), relClassIdCol.GetName().c_str());
            return ERROR;
            }
        }

    if (SUCCESS != ValidateNavigationPropertyMapNotNull(propMap, idCol, relClassIdCol, isPhysicalFk))
        return ERROR;

    return ValidateNavigationPropertyMapUniqueness(propMap, idCol, relClassIdCol, isPhysicalFk);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Krischan.Eberle                       07/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::ValidateNavigationPropertyMapNotNull(NavigationPropertyMap const& propMap, DbColumn const& idCol, DbColumn const& relClassIdCol, bool isPhysicalFk) const
    {
    if (!isPhysicalFk)
        {
        if (idCol.DoNotAllowDbNull())
            {
            Issues().Report("The navigation property '%s.%s' maps to a logical foreign key. But its foreign column '%s.%s' has a NOT NULL constraint which is invalid for logical foreign keys.",
                            propMap.GetClassMap().GetClass().GetFullName(), propMap.GetAccessString().c_str(),
                            idCol.GetTable().GetName().c_str(), idCol.GetName().c_str());
            return ERROR;
            }

        return SUCCESS;
        }

    BeAssert(isPhysicalFk);
    //The FK and RelECClassId can be made NOT NULL if the multiplicity on the referenced end is (1..X) AND
    //if the nav prop's class is the exclusive root of the table. If it wasn't base classes would face NOT NULL constraint violations
    //because they will leave the FK column empty
    const bool isNavPropClassExclusiveRootClass = idCol.GetTable().HasExclusiveRootECClass() && idCol.GetTable().GetExclusiveRootECClassId() == propMap.GetClassMap().GetClass().GetId();
    const bool fkExpectedNotNull = propMap.CardinalityImpliesNotNull() && isNavPropClassExclusiveRootClass;
    if (fkExpectedNotNull)
        {
        if (!idCol.DoNotAllowDbNull())
            {
            Issues().Report("The navigation property '%s.%s' implies a NOT NULL constraint on the foreign key column '%s.%s'. But the column doesn't have one.",
                            propMap.GetClassMap().GetClass().GetFullName(), propMap.GetAccessString().c_str(),
                            idCol.GetTable().GetName().c_str(), idCol.GetName().c_str());
            return ERROR;
            }

        if (!relClassIdCol.DoNotAllowDbNull())
            {
            Issues().Report("The navigation property '%s.%s' implies a NOT NULL constraint on the RelECClassId column '%s.%s'. But the column doesn't have one.",
                            propMap.GetClassMap().GetClass().GetFullName(), propMap.GetAccessString().c_str(),
                            relClassIdCol.GetTable().GetName().c_str(), relClassIdCol.GetName().c_str());
            return ERROR;
            }

        return SUCCESS;
        }

    //FK expected to be nullable
    if (idCol.DoNotAllowDbNull())
        {
        Issues().Report("The navigation property '%s.%s' implies that the foreign key column is nullable. But the column has a NOT NULL constraint.",
                        propMap.GetClassMap().GetClass().GetFullName(), propMap.GetAccessString().c_str(),
                        idCol.GetTable().GetName().c_str(), idCol.GetName().c_str());
        return ERROR;
        }

    if (relClassIdCol.DoNotAllowDbNull())
        {
        Issues().Report("The navigation property '%s.%s' implies that the RelECClassId column is nullable. But the column has a NOT NULL constraint.",
                        propMap.GetClassMap().GetClass().GetFullName(), propMap.GetAccessString().c_str(),
                        relClassIdCol.GetTable().GetName().c_str(), relClassIdCol.GetName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Krischan.Eberle                       07/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbMapValidator::ValidateNavigationPropertyMapUniqueness(NavigationPropertyMap const& propMap, DbColumn const& idCol, DbColumn const& relClassIdCol, bool isPhysicalFk) const
    {
    DbTable const& table = idCol.GetTable();

    if (idCol.IsUnique() || relClassIdCol.IsUnique())
        {
        Issues().Report("The navigation property '%s.%s' maps to the foreign key column '%s.%s' and the RelECClassId column '%s.%s'. At least one of them has a UNIQUE constraint. This should never be the case as uniqueness is enforced via a unique index where necessary.",
                        propMap.GetClassMap().GetClass().GetFullName(), propMap.GetAccessString().c_str(),
                        table.GetName().c_str(), idCol.GetName().c_str(), table.GetName().c_str(), relClassIdCol.GetName().c_str());
        return ERROR;
        }


    const bool uniqueIndexExpected = isPhysicalFk && propMap.CardinalityImpliesUnique();
    auto it = m_indexesByColumnCache.find(idCol.GetId());

    int systemIndexCount = 0;

    if (it != m_indexesByColumnCache.end())
        {
        for (DbIndex const* index : it->second)
            {
            //user defined indexes and multi-col indexes are ignored as they are not cardinality driven indexes
            if (!index->IsAutoGenerated() || index->GetColumns().size() > 1)
                continue;

            if (!uniqueIndexExpected && index->GetIsUnique())
                {
                Issues().Report("The navigation property '%s.%s' implies that the foreign key column '%s.%s' must not be unique. But there is the unique index '%s' on the column.",
                                propMap.GetClassMap().GetClass().GetFullName(), propMap.GetAccessString().c_str(),
                                table.GetName().c_str(), idCol.GetName().c_str(), index->GetName().c_str());
                return ERROR;
                }

            systemIndexCount++;
            }
        }


    if (uniqueIndexExpected && systemIndexCount == 0)
        {
        Issues().Report("The navigation property '%s.%s' implies that the foreign key column '%s.%s' is unique. But there is no unique index to enforce it.",
                        propMap.GetClassMap().GetClass().GetFullName(), propMap.GetAccessString().c_str(),
                        table.GetName().c_str(), idCol.GetName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

