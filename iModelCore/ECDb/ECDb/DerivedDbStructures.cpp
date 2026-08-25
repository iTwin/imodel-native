/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DerivedDbStructures::Derive(MainSchemaManager const& manager)
    {
    if (SUCCESS != AddChildTableForeignKeys(manager))
        return ERROR;

    if (SUCCESS != AddRelationshipForeignKeys(manager))
        return ERROR;

    return AddCurrentTimeStampTriggers(manager);
    }

//---------------------------------------------------------------------------------------
// A joined or overflow table is meaningless without its parent's row, so it carries a
// cascading foreign key to it. Purely structural - ec_Table.ParentTableId is persisted and
// restored by DbSchema::LoadTable.
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DerivedDbStructures::AddChildTableForeignKeys(MainSchemaManager const& manager)
    {
    for (DbTable const* table : manager.GetDbSchema().Tables().GetTablesInDependencyOrder())
        {
        if (table->GetType() != DbTable::Type::Joined && table->GetType() != DbTable::Type::Overflow)
            continue;

        DbTable::LinkNode const* parentNode = table->GetLinkNode().GetParent();
        if (parentNode == nullptr)
            continue;

        DbTable const& parentTable = parentNode->GetTable();
        if (parentTable.GetType() == DbTable::Type::Virtual || parentTable.GetType() == DbTable::Type::Existing)
            continue;

        DbColumn const* fkColumn = table->FindFirst(DbColumn::Kind::ECInstanceId);
        DbColumn const* referencedColumn = parentTable.FindFirst(DbColumn::Kind::ECInstanceId);
        if (fkColumn == nullptr || referencedColumn == nullptr)
            {
            BeAssert(false && "Child table and its parent must both have an ECInstanceId column");
            return ERROR;
            }

        // NotSpecified and NoAction are not interchangeable: DoAppendForeignKeyDdl omits the clause
        // for NotSpecified, so an overflow table reads ON UPDATE NO ACTION and a joined table gets no
        // ON UPDATE at all. Keep the difference - the DDL is compared across briefcases.
        const ForeignKeyDbConstraint::ActionType onUpdateAction = table->GetType() == DbTable::Type::Overflow ?
            ForeignKeyDbConstraint::ActionType::NoAction : ForeignKeyDbConstraint::ActionType::NotSpecified;

        if (SUCCESS != EnsureForeignKey(const_cast<DbTable&>(*table), *fkColumn, *referencedColumn, ForeignKeyDbConstraint::ActionType::Cascade, onUpdateAction))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DerivedDbStructures::AddRelationshipForeignKeys(MainSchemaManager const& manager)
    {
    Utf8CP tableSpace = manager.GetTableSpace().GetName().c_str();
    Utf8String sql;
    sql.Sprintf("SELECT CM.ClassId,CM.MapStrategy FROM [%s]." TABLE_ClassMap " CM "
                "INNER JOIN [%s]." TABLE_Class " C ON C.Id=CM.ClassId "
                "WHERE C.Type=" SQLVAL_ECClassType_Relationship " AND CM.MapStrategy IN ("
                SQLVAL_MapStrategy_OwnTable "," SQLVAL_MapStrategy_TablePerHierarchy ","
                SQLVAL_MapStrategy_ForeignKeyRelationshipInSourceTable "," SQLVAL_MapStrategy_ForeignKeyRelationshipInTargetTable ") "
                "ORDER BY C.Name,C.Id", // two foreign keys on one column come out in the order the mapper wrote them, and the DDL text is compared across briefcases
                tableSpace, tableSpace);

    // Collect first, then act - the loop below loads class maps, which writes to the statement cache.
    std::vector<std::pair<ECClassId, MapStrategy>> relationships;
        {
        CachedStatementPtr stmt = manager.GetECDb().GetImpl().GetCachedSqliteStatement(sql.c_str());
        if (stmt == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        while (stmt->Step() == BE_SQLITE_ROW)
            relationships.push_back(std::make_pair(stmt->GetValueId<ECClassId>(0), Enum::FromInt<MapStrategy>(stmt->GetValueInt(1))));
        }

    for (auto const& entry : relationships)
        {
        ECClassCP ecClass = manager.GetClass(entry.first);
        if (ecClass == nullptr || !ecClass->IsRelationshipClass())
            {
            BeAssert(false);
            return ERROR;
            }

        // Only root relationships own foreign keys. Subclasses share the root's constraint - the
        // same rule DbMappingManager::FkRelationships::FinishMapping applies.
        if (ecClass->HasBaseClasses())
            continue;

        if (entry.second == MapStrategy::ForeignKeyRelationshipInSourceTable || entry.second == MapStrategy::ForeignKeyRelationshipInTargetTable)
            {
            if (SUCCESS != AddNavigationPropertyForeignKeys(manager, *ecClass->GetRelationshipClassCP(), entry.second))
                return ERROR;

            continue;
            }

        ClassMap const* classMap = manager.GetClassMap(*ecClass);
        if (classMap == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        if (classMap->GetType() != ClassMap::Type::RelationshipLinkTable)
            continue;

        if (SUCCESS != AddLinkTableForeignKeys(manager, classMap->GetAs<RelationshipClassMap>()))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// A link table's SourceECInstanceId / TargetECInstanceId reference the constraint tables and
// cascade, unless the LinkTableRelationshipMap custom attribute turns them off.
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DerivedDbStructures::AddLinkTableForeignKeys(MainSchemaManager const& manager, RelationshipClassMap const& classMap)
    {
    DbTable& linkTable = classMap.GetPrimaryTable();
    if (linkTable.GetType() == DbTable::Type::Existing || linkTable.GetType() == DbTable::Type::Virtual)
        return SUCCESS;

    ECRelationshipClassCR relClass = classMap.GetRelationshipClass();

    bool createFkConstraints = true;
    LinkTableRelationshipMapCustomAttribute ca;
    if (ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap(ca, relClass) && ca.IsValid())
        {
        Nullable<bool> createFkConstraintsVal;
        if (SUCCESS != ca.TryGetCreateForeignKeyConstraints(createFkConstraintsVal))
            return ERROR;

        if (!createFkConstraintsVal.IsNull())
            createFkConstraints = createFkConstraintsVal.Value();
        }

    if (!createFkConstraints)
        return SUCCESS;

    // Only the TPH root owns the constraints, since the whole hierarchy shares one table.
    if (classMap.GetMapStrategy().IsTablePerHierarchy() && classMap.GetTphHelper()->DetermineTphRootClassId() != relClass.GetId())
        return SUCCESS;

    struct { ECRelationshipConstraintCP constraint; ConstraintECInstanceIdPropertyMap const* propMap; } ends[] = {
        { &relClass.GetSource(), classMap.GetSourceECInstanceIdPropMap() },
        { &relClass.GetTarget(), classMap.GetTargetECInstanceIdPropMap() } };

    for (auto const& end : ends)
        {
        if (end.propMap == nullptr)
            {
            BeAssert(false && "Link table map must have source and target ECInstanceId property maps");
            return ERROR;
            }

        std::set<DbTable const*> constraintTables = GetConstraintPrimaryTables(manager, *end.constraint);
        if (constraintTables.size() != 1)
            continue; // the mapper refuses more than one; nothing to reference if there is none

        DbTable const* constraintTable = *constraintTables.begin();
        if (constraintTable->GetType() == DbTable::Type::Virtual || constraintTable->GetType() == DbTable::Type::Existing)
            continue;

        DataPropertyMap const* dataPropMap = end.propMap->FindDataPropertyMap(linkTable);
        DbColumn const* referencedColumn = constraintTable->FindFirst(DbColumn::Kind::ECInstanceId);
        if (dataPropMap == nullptr || referencedColumn == nullptr)
            continue;

        DbColumn const& fkColumn = dataPropMap->GetAs<SingleColumnDataPropertyMap>().GetColumn();
        if (fkColumn.IsShared() || fkColumn.GetPersistenceType() == PersistenceType::Virtual)
            continue;

        if (SUCCESS != EnsureForeignKey(linkTable, fkColumn, *referencedColumn, ForeignKeyDbConstraint::ActionType::Cascade, ForeignKeyDbConstraint::ActionType::NotSpecified))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// A navigation property's column references the other end's table when the property carries
// the ForeignKeyConstraint custom attribute. Without it the relationship is a logical foreign
// key and gets no constraint at all.
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DerivedDbStructures::AddNavigationPropertyForeignKeys(MainSchemaManager const& manager, ECRelationshipClassCR relClass, MapStrategy mapStrategy)
    {
    const ECRelationshipEnd foreignEnd = mapStrategy == MapStrategy::ForeignKeyRelationshipInSourceTable ? ECRelationshipEnd::ECRelationshipEnd_Source : ECRelationshipEnd::ECRelationshipEnd_Target;
    ECRelationshipConstraintCR foreignEndConstraint = foreignEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();
    ECRelationshipConstraintCR referencedEndConstraint = foreignEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? relClass.GetTarget() : relClass.GetSource();

    NavigationECPropertyCP navProp = FindNavigationProperty(manager, foreignEndConstraint, relClass);
    if (navProp == nullptr)
        return SUCCESS;

    ForeignKeyConstraintCustomAttribute fkConstraintCA;
    if (!ECDbMapCustomAttributeHelper::TryGetForeignKeyConstraint(fkConstraintCA, *navProp) || !fkConstraintCA.IsValid())
        return SUCCESS; // logical foreign key

    Nullable<Utf8String> onDeleteActionStr;
    if (SUCCESS != fkConstraintCA.TryGetOnDeleteAction(onDeleteActionStr))
        return ERROR;

    ForeignKeyDbConstraint::ActionType onDeleteAction = ForeignKeyDbConstraint::ActionType::NotSpecified;
    if (SUCCESS != ForeignKeyDbConstraint::TryParseActionType(onDeleteAction, onDeleteActionStr))
        return ERROR;

    if (onDeleteAction == ForeignKeyDbConstraint::ActionType::NotSpecified)
        onDeleteAction = relClass.GetStrength() == StrengthType::Embedding ? ForeignKeyDbConstraint::ActionType::Cascade : ForeignKeyDbConstraint::ActionType::SetNull;

    Nullable<Utf8String> onUpdateActionStr;
    if (SUCCESS != fkConstraintCA.TryGetOnUpdateAction(onUpdateActionStr))
        return ERROR;

    ForeignKeyDbConstraint::ActionType onUpdateAction = ForeignKeyDbConstraint::ActionType::NotSpecified;
    if (SUCCESS != ForeignKeyDbConstraint::TryParseActionType(onUpdateAction, onUpdateActionStr))
        return ERROR;

    std::set<DbTable const*> referencedTables = GetConstraintPrimaryTables(manager, referencedEndConstraint);
    if (referencedTables.size() != 1)
        return SUCCESS; // the mapper already refused this during the import that created the mapping

    DbTable const& referencedTable = **referencedTables.begin();
    if (referencedTable.GetType() == DbTable::Type::Virtual)
        return SUCCESS;

    DbColumn const* referencedColumn = referencedTable.FindFirst(DbColumn::Kind::ECInstanceId);
    if (referencedColumn == nullptr)
        return SUCCESS;

    std::unique_ptr<ForeignKeyPartitionView> partitionView = ForeignKeyPartitionView::CreateReadonly(manager, relClass);
    if (partitionView == nullptr)
        return SUCCESS; // relationship is not mapped as an end table relationship (yet)

    for (ForeignKeyPartitionView::Partition const* partition : partitionView->GetPartitions())
        {
        DbColumn const& fkColumn = partition->GetFromECInstanceIdColumn();
        DbTable& fkTable = const_cast<DbTable&>(fkColumn.GetTable());

        // A child table does carry the constraint; only a cascading one is refused, and the mapper
        // does that refusing, so a mapping that reached this file never has one.
        if (fkTable.GetType() == DbTable::Type::Existing || fkTable.GetType() == DbTable::Type::Virtual || fkColumn.IsShared())
            continue;

        if (SUCCESS != EnsureForeignKey(fkTable, fkColumn, *referencedColumn, onDeleteAction, onUpdateAction))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// The mapper marks a current-timestamp column by giving it DEFAULT(julianday('now')), and that
// does reach ec_Column - so the trigger is derivable from the column alone, without reading the
// ClassHasCurrentTimeStampProperty custom attribute back.
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DerivedDbStructures::AddCurrentTimeStampTriggers(MainSchemaManager const& manager)
    {
    for (DbTable const* table : manager.GetDbSchema().Tables().GetTablesInDependencyOrder())
        {
        if (table->GetType() == DbTable::Type::Existing || table->GetType() == DbTable::Type::Virtual)
            continue;

        DbColumn const* instanceIdColumn = table->FindFirst(DbColumn::Kind::ECInstanceId);
        if (instanceIdColumn == nullptr)
            continue;

        for (DbColumn const* column : table->GetColumns())
            {
            if (column->GetPersistenceType() == PersistenceType::Virtual || column->IsShared())
                continue;

            if (!column->GetConstraints().GetDefaultValueConstraint().Equals(CURRENTIMESTAMP_SQLEXP))
                continue;

            Utf8String triggerName;
            triggerName.Sprintf("%s_CurrentTimeStamp", table->GetName().c_str());

            Utf8String body;
            body.Sprintf("BEGIN UPDATE [%s] SET [%s]=" CURRENTIMESTAMP_SQLEXP " WHERE [%s]=new.[%s]; END",
                         table->GetName().c_str(), column->GetName().c_str(), instanceIdColumn->GetName().c_str(), instanceIdColumn->GetName().c_str());

            Utf8String whenCondition;
            whenCondition.Sprintf("old.[%s]=new.[%s] AND old.[%s]!=" CURRENTIMESTAMP_SQLEXP,
                                  column->GetName().c_str(), column->GetName().c_str(), column->GetName().c_str());

            // AddTrigger is a no-op if a trigger of that name is already on the table.
            if (SUCCESS != const_cast<DbTable*>(table)->AddTrigger(triggerName, DbTrigger::Type::After, whenCondition, body))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DerivedDbStructures::EnsureForeignKey(DbTable& table, DbColumn const& fkColumn, DbColumn const& referencedColumn, ForeignKeyDbConstraint::ActionType onDelete, ForeignKeyDbConstraint::ActionType onUpdate)
    {
    // Two relationships can put differently-acting foreign keys on the same column, so the actions
    // are part of the identity - matching ForeignKeyDbConstraint::Equals.
    for (DbConstraint const* constraint : table.GetConstraints())
        {
        if (constraint->GetType() != DbConstraint::Type::ForeignKey)
            continue;

        ForeignKeyDbConstraint const* fk = static_cast<ForeignKeyDbConstraint const*>(constraint);
        if (fk->GetFkColumns().size() == 1 && fk->GetFkColumns().front() == &fkColumn &&
            fk->GetReferencedTableColumns().size() == 1 && fk->GetReferencedTableColumns().front() == &referencedColumn &&
            fk->GetOnDeleteAction() == onDelete && fk->GetOnUpdateAction() == onUpdate)
            return SUCCESS;
        }

    return table.AddForeignKeyConstraint(fkColumn, referencedColumn, onDelete, onUpdate) != nullptr ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// Read-only counterpart of MainSchemaManager::GetRelationshipConstraintPrimaryTables, which
// needs a SchemaImportContext and so is not available here.
// @bsimethod
//---------------------------------------------------------------------------------------
//static
std::set<DbTable const*> DerivedDbStructures::GetConstraintPrimaryTables(MainSchemaManager const& manager, ECRelationshipConstraintCR constraint)
    {
    std::set<DbTable const*> tables;
    for (ECClassCP constraintClass : constraint.GetConstraintClasses())
        {
        std::vector<ECClassCP> classes { constraintClass };
        if (constraint.GetIsPolymorphic())
            {
            Nullable<ECDerivedClassesList> derivedClasses = manager.GetAllDerivedClasses(*constraintClass);
            if (!derivedClasses.IsNull())
                {
                for (ECClassCP derivedClass : derivedClasses.Value())
                    classes.push_back(derivedClass);
                }
            }

        for (ECClassCP ecClass : classes)
            {
            ClassMap const* classMap = manager.GetClassMap(*ecClass);
            if (classMap == nullptr || classMap->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
                continue;

            DbTable const& primaryTable = classMap->GetPrimaryTable();
            if (primaryTable.GetType() == DbTable::Type::Virtual)
                continue;

            tables.insert(&primaryTable);
            }
        }

    return tables;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
NavigationECPropertyCP DerivedDbStructures::FindNavigationProperty(MainSchemaManager const& manager, ECRelationshipConstraintCR foreignEndConstraint, ECRelationshipClassCR relClass)
    {
    for (ECClassCP constraintClass : foreignEndConstraint.GetConstraintClasses())
        {
        std::vector<ECClassCP> classes { constraintClass };
        Nullable<ECDerivedClassesList> derivedClasses = manager.GetAllDerivedClasses(*constraintClass);
        if (!derivedClasses.IsNull())
            {
            for (ECClassCP derivedClass : derivedClasses.Value())
                classes.push_back(derivedClass);
            }

        for (ECClassCP ecClass : classes)
            {
            for (ECPropertyCP prop : ecClass->GetProperties(true))
                {
                NavigationECPropertyCP navProp = prop->GetAsNavigationProperty();
                if (navProp != nullptr && navProp->GetRelationshipClass() == &relClass)
                    return navProp;
                }
            }
        }

    return nullptr;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
