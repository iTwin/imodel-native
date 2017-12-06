/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ChangeSummaryExtractor.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                11/2017
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangeSummaryExtractor::Extract(ECInstanceKey& summaryKey, ChangeSummaryManager& manager, IChangeSet& changeSet, ECDb::ChangeSummaryExtractOptions const& options) const
    {
    Context ctx(manager);
    if (BE_SQLITE_OK != ctx.OpenChangeSummaryECDb())
        return ERROR; //errors already logged in above call

    if (InsertSummary(summaryKey, ctx) != SUCCESS)
        return ERROR;

    // Pass 1
    if (SUCCESS != Extract(ctx, summaryKey.GetInstanceId(), changeSet, ExtractMode::InstancesOnly))
        return ERROR;

    // Pass 2
    if (options.IncludeRelationshipInstances())
        return Extract(ctx, summaryKey.GetInstanceId(), changeSet, ExtractMode::RelationshipInstancesOnly);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryExtractor::Extract(Context& ctx, ECInstanceId summaryId, IChangeSet& changeSet, ExtractMode mode) const
    {
    ChangeIterator iter(ctx.GetPrimaryECDb(), changeSet);
    for (ChangeIterator::RowEntry const& rowEntry : iter)
        {
        if (!rowEntry.IsMapped())
            {
            LOG.warningv("ChangeSet includes changes to unmapped table %s", rowEntry.GetTableName().c_str());
            continue; // There are tables which are just not mapped to EC that we simply don't care about (e.g., be_Prop table)
            }

        ECClassCP primaryClass = rowEntry.GetPrimaryClass();
        ECInstanceId primaryInstanceId = rowEntry.GetPrimaryInstanceId();
        if (primaryClass == nullptr || !primaryInstanceId.IsValid())
            {
            ctx.Issues().Report("Could not determine the primary instance corresponding to a change to table %s", rowEntry.GetTableName().c_str());
            BeAssert(false && "Could not determine the primary instance corresponding to a change.");
            return ERROR;
            }

        if (mode == ExtractMode::InstancesOnly && !primaryClass->IsRelationshipClass())
            {
            if (SUCCESS != ExtractInstance(ctx, summaryId, rowEntry))
                return ERROR;

            continue;
            }

        if (mode == ExtractMode::RelationshipInstancesOnly)
            {
            if (SUCCESS != ExtractRelInstance(ctx, summaryId, rowEntry))
                return ERROR;

            continue;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryExtractor::ExtractInstance(Context& ctx, ECInstanceId summaryId, ChangeIterator::RowEntry const& rowEntry) const
    {
    InstanceChange instance(summaryId, ECInstanceKey(rowEntry.GetPrimaryClass()->GetId(), rowEntry.GetPrimaryInstanceId()), rowEntry.GetDbOpcode(),
                                       RawIndirectToBool(rowEntry.GetIndirect()));
    bool recordOnlyIfUpdatedProperties = (rowEntry.GetDbOpcode() == DbOpcode::Update);
    return RecordInstance(ctx, instance, rowEntry, recordOnlyIfUpdatedProperties);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryExtractor::ExtractRelInstance(Context& ctx, ECInstanceId summaryId, ChangeIterator::RowEntry const& rowEntry) const
    {
    ECClassCP primaryClass = rowEntry.GetPrimaryClass();

    ClassMap const* classMap = ctx.GetSchemaManager().GetClassMap(*primaryClass);
    if (classMap == nullptr)
        {
        ctx.Issues().Report("Failed to extract change summary. The changed relationship class '%s' is not in the main ECDb, but in an attached file, which is not supported.",
                            primaryClass->GetFullName());
        return ERROR;
        }

    ClassMap::Type type = classMap->GetType();
    if (type == ClassMap::Type::RelationshipLinkTable)
        {
        LinkTableRelChangeExtractor linkTableExtractor(*this);
        return linkTableExtractor.Extract(ctx, summaryId, rowEntry, classMap->GetAs<RelationshipClassLinkTableMap>());
        }

    ChangeIterator::TableClassMap const* tableClassMap = rowEntry.GetTableMap()->GetTableClassMap(*primaryClass);
    BeAssert(tableClassMap != nullptr);

    FkRelChangeExtractor fkRelExtractor(*this);
    for (ChangeIterator::TableClassMap::EndTableRelationshipMap const* endTableRelMap : tableClassMap->GetEndTableRelationshipMaps())
        {
        if (SUCCESS != fkRelExtractor.Extract(ctx, summaryId, rowEntry, *endTableRelMap))
            return ERROR;
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryExtractor::RecordRelInstance(Context& ctx, InstanceChange const& instance, ChangeIterator::RowEntry const& rowEntry, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey) const
    {
    if (SUCCESS != RecordInstance(ctx, instance, rowEntry, false)) // Even if any of the properties of the relationship is not updated, the relationship needs to be recorded since the source/target keys would have changed (to get here)
        return ERROR;

    if (BE_SQLITE_DONE != InsertPropertyValueChange(ctx, instance.GetSummaryId(), instance.GetKeyOfChangedInstance(), ECDBSYS_PROP_SourceECClassId, oldSourceKey.IsValid() ? oldSourceKey.GetClassId() : ECClassId(), newSourceKey.IsValid() ? newSourceKey.GetClassId() : ECClassId()))
        return ERROR;

    if (BE_SQLITE_DONE != InsertPropertyValueChange(ctx, instance.GetSummaryId(), instance.GetKeyOfChangedInstance(), ECDBSYS_PROP_SourceECInstanceId, oldSourceKey.IsValid() ? oldSourceKey.GetInstanceId() : ECInstanceId(), newSourceKey.IsValid() ? newSourceKey.GetInstanceId() : ECInstanceId()))
        return ERROR;

    if (BE_SQLITE_DONE != InsertPropertyValueChange(ctx, instance.GetSummaryId(), instance.GetKeyOfChangedInstance(), ECDBSYS_PROP_TargetECClassId, oldTargetKey.IsValid() ? oldTargetKey.GetClassId() : ECClassId(), newTargetKey.IsValid() ? newTargetKey.GetClassId() : ECClassId()))
        return ERROR;

    return InsertPropertyValueChange(ctx, instance.GetSummaryId(), instance.GetKeyOfChangedInstance(), ECDBSYS_PROP_TargetECInstanceId, oldTargetKey.IsValid() ? oldTargetKey.GetInstanceId() : ECInstanceId(), newTargetKey.IsValid() ? newTargetKey.GetInstanceId() : ECInstanceId()) == BE_SQLITE_DONE ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryExtractor::RecordInstance(Context& ctx, InstanceChange const& instance, ChangeIterator::RowEntry const& rowEntry, bool recordOnlyIfUpdatedProperties) const
    {
    bool removeIfNotUpdatedProperties = false;
    if (recordOnlyIfUpdatedProperties)
        removeIfNotUpdatedProperties = !ContainsChange(ctx, instance.GetSummaryId(), instance.GetKeyOfChangedInstance());

    DbResult stat = InsertOrUpdate(ctx, instance);
    if (BE_SQLITE_DONE != stat)
        return ERROR;

    bool updatedProperties = false;
    ECN::ECClassCP ecClass = ctx.GetPrimaryECDb().Schemas().GetClass(instance.GetKeyOfChangedInstance().GetClassId());
    for (ChangeIterator::ColumnEntry const& columnEntry : rowEntry.MakeColumnIterator(*ecClass))
        {
        if (columnEntry.IsPrimaryKeyColumn())
            continue;  // Primary key columns need not be included in the values table

        bool isNoNeedToRecord = false;
        if (SUCCESS != RecordValue(isNoNeedToRecord, ctx, instance, columnEntry))
            return ERROR;

        if (isNoNeedToRecord) //nothing had to be recorded;
            continue;

        updatedProperties = true;
        }

    if (!(removeIfNotUpdatedProperties && !updatedProperties))
        return SUCCESS;

    // If recording an update for the first time, and none of the properties have really been updated, remove record of the updated instance
    return BE_SQLITE_DONE == Delete(ctx, instance.GetSummaryId(), instance.GetKeyOfChangedInstance()) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryExtractor::RecordValue(bool& isNoNeedToRecord, Context& ctx, InstanceChange const& instance, ChangeIterator::ColumnEntry const& columnEntry) const
    {
    DbOpcode dbOpcode = instance.GetDbOpcode();

    DbDupValue oldValue(nullptr);
    if (dbOpcode != DbOpcode::Insert)
        oldValue = columnEntry.GetValue(Changes::Change::Stage::Old);

    DbDupValue newValue(nullptr);
    if (dbOpcode != DbOpcode::Delete)
        newValue = columnEntry.GetValue(Changes::Change::Stage::New);

    bool hasOldValue = oldValue.IsValid() && !oldValue.IsNull();
    bool hasNewValue = newValue.IsValid() && !newValue.IsNull();

    if (!hasOldValue && !hasNewValue) // Do not persist entirely empty fields
        {
        isNoNeedToRecord = true;
        return SUCCESS;
        }

    return BE_SQLITE_DONE == InsertPropertyValueChange(ctx, instance.GetSummaryId(), instance.GetKeyOfChangedInstance(), columnEntry.GetPropertyAccessString().c_str(), oldValue, newValue) ? SUCCESS : ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
InstanceChange ChangeSummaryExtractor::QueryInstanceChange(Context& ctx, ECInstanceId summaryId, ECInstanceKey const& keyOfChangedInstance) const
    {
    InstanceChange instanceChange;

    CachedECSqlStatementPtr stmt = ctx.GetChangeSummaryStatement("SELECT OpCode,IsIndirect FROM " ECSCHEMA_ALIAS_ECDbChangeSummaries "." ECDBCHANGE_CLASS_InstanceChange " WHERE ChangedInstance.ClassId=? AND ChangedInstance.Id=? AND Summary.Id=?");
    if (stmt == nullptr)
        {
        BeAssert(false);
        return instanceChange;
        }

    stmt->BindId(1, keyOfChangedInstance.GetClassId());
    stmt->BindId(2, keyOfChangedInstance.GetInstanceId());
    stmt->BindId(3, summaryId);

    if (BE_SQLITE_ROW != stmt->Step())
        return instanceChange;

    int opCodeVal = stmt->GetValueInt(0);
    Nullable<ChangeOpCode> opCode = ChangeSummaryManager::ToChangeOpCode(opCodeVal);
    if (opCode.IsNull())
        {
        BeAssert(false);
        return instanceChange;
        }

    Nullable<DbOpcode> dbOpCode = ChangeSummaryManager::ToDbOpCode(opCode.Value());
    if (dbOpCode.IsNull()) // enum has changed -> fatal error
        return instanceChange;

    return InstanceChange(summaryId, keyOfChangedInstance, dbOpCode.Value(), stmt->GetValueBoolean(1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Affan.Khan           10/2017
//---------------------------------------------------------------------------------------
ECInstanceId ChangeSummaryExtractor::FindChangeId(Context& ctx, ECInstanceId summaryId, ECInstanceKey const& keyOfChangedInstance) const
    {
    CachedECSqlStatementPtr stmt = ctx.GetChangeSummaryStatement("SELECT ECInstanceId FROM " ECSCHEMA_ALIAS_ECDbChangeSummaries "." ECDBCHANGE_CLASS_InstanceChange " WHERE ChangedInstance.Id=? AND ChangedInstance.ClassId=? AND Summary.Id=?");
    if (stmt == nullptr)
        {
        BeAssert(false);
        return ECInstanceId();
        }

    stmt->BindId(1, keyOfChangedInstance.GetInstanceId());
    stmt->BindId(2, keyOfChangedInstance.GetClassId());
    stmt->BindId(3, summaryId);

    if (stmt->Step() == BE_SQLITE_ROW)
        return stmt->GetValueId<ECInstanceId>(0);

    return ECInstanceId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Affan.Khan           11/2017
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryExtractor::InsertSummary(ECInstanceKey& summaryKey, Context& ctx) const
    {
    CachedECSqlStatementPtr stmt = ctx.GetChangeSummaryStatement("INSERT INTO " ECSCHEMA_ALIAS_ECDbChangeSummaries "." ECDBCHANGE_CLASS_ChangeSummary "(ECInstanceId) VALUES(NULL)");
    if (stmt == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (stmt->Step(summaryKey) != BE_SQLITE_DONE)
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
DbResult ChangeSummaryExtractor::InsertOrUpdate(Context& ctx, InstanceChange const& instance) const
    {
    /*
    * Here's the logic to consolidate new changes with the ones
    * previously found:
    *
    * not-found    + new:*       = Insert new entry
    *
    * found:UPDATE + new:INSERT  = Update existing entry to INSERT
    * found:UPDATE + new:DELETE  = Update existing entry to DELETE
    *
    * <all other cases keep existing entry>
    */

    DbOpcode dbOpcode = instance.GetDbOpcode();

    Nullable<ChangeOpCode> op = ChangeSummaryManager::ToChangeOpCode(dbOpcode);
    if (op.IsNull()) // DbOpCode enum has changed -> fatal error code needs to be adjusted
        return BE_SQLITE_ERROR;

    InstanceChange foundInstanceChange = QueryInstanceChange(ctx, instance.GetSummaryId(), instance.GetKeyOfChangedInstance());
    if (!foundInstanceChange.IsValid())
        {
        CachedECSqlStatementPtr stmt = ctx.GetChangeSummaryStatement("INSERT INTO " ECSCHEMA_ALIAS_ECDbChangeSummaries "." ECDBCHANGE_CLASS_InstanceChange "(ChangedInstance.ClassId,ChangedInstance.Id, OpCode, IsIndirect, Summary.Id) VALUES(?,?,?,?,?)");
        if (stmt == nullptr)
            {
            BeAssert(false);
            return BE_SQLITE_ERROR;
            }

        stmt->BindId(1, instance.GetKeyOfChangedInstance().GetClassId());
        stmt->BindId(2, instance.GetKeyOfChangedInstance().GetInstanceId());
        stmt->BindInt(3, Enum::ToInt(op.Value()));
        stmt->BindBoolean(4, instance.IsIndirect());
        stmt->BindId(5, instance.GetSummaryId());
        return stmt->Step();
        }

    if (foundInstanceChange.GetDbOpcode() == DbOpcode::Update && dbOpcode != DbOpcode::Update)
        {
        CachedECSqlStatementPtr stmt = ctx.GetChangeSummaryStatement("UPDATE " ECSCHEMA_ALIAS_ECDbChangeSummaries "." ECDBCHANGE_CLASS_InstanceChange " SET OpCode=?, IsIndirect=? WHERE ChangedInstance.ClassId=? AND ChangedInstance.Id=? AND Summary.Id=?");
        if (stmt == nullptr)
            {
            BeAssert(false);
            return BE_SQLITE_ERROR;
            }

        stmt->BindInt(1, Enum::ToInt(op.Value()));
        stmt->BindBoolean(2, instance.IsIndirect());
        stmt->BindId(3, instance.GetKeyOfChangedInstance().GetClassId());
        stmt->BindId(4, instance.GetKeyOfChangedInstance().GetInstanceId());
        stmt->BindId(5, instance.GetSummaryId());
        return stmt->Step();
        }

    return BE_SQLITE_DONE;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
DbResult ChangeSummaryExtractor::Delete(Context& ctx, ECInstanceId summaryId, ECInstanceKey const& keyOfChangedInstance) const
    {
    CachedECSqlStatementPtr stmt = ctx.GetChangeSummaryStatement("DELETE FROM " ECSCHEMA_ALIAS_ECDbChangeSummaries "." ECDBCHANGE_CLASS_InstanceChange " WHERE ChangedInstance.ClassId=? AND ChangedInstance.Id=? AND Summary.Id=?");
    if (stmt == nullptr)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    stmt->BindId(1, keyOfChangedInstance.GetClassId());
    stmt->BindId(2, keyOfChangedInstance.GetInstanceId());
    stmt->BindId(3, summaryId);

    return stmt->Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
DbResult ChangeSummaryExtractor::InsertPropertyValueChange(Context& ctx, ECInstanceId summaryId, ECInstanceKey const& keyOfChangedInstance, Utf8CP accessString, DbValue const& oldValue, DbValue const& newValue) const
    {
    ECInstanceId changeId = FindChangeId(ctx, summaryId, keyOfChangedInstance);

    CachedECSqlStatementPtr stmt = ctx.GetChangeSummaryStatement("INSERT INTO " ECSCHEMA_ALIAS_ECDbChangeSummaries  "." ECDBCHANGE_CLASS_PropertyValueChange "(InstanceChange.Id, AccessString, RawOldValue, RawNewValue) VALUES (?,?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    stmt->BindId(1, changeId);
    stmt->BindText(2, accessString, IECSqlBinder::MakeCopy::No);
    BindDbValue(*stmt, 3, oldValue);
    BindDbValue(*stmt, 4, newValue);

    return stmt->Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
DbResult ChangeSummaryExtractor::InsertPropertyValueChange(Context& ctx, ECInstanceId summaryId, ECInstanceKey const& keyOfChangedInstance, Utf8CP accessString, ECClassId oldValue, ECClassId newValue) const
    {
    ECInstanceId changeId = FindChangeId(ctx, summaryId, keyOfChangedInstance);

    CachedECSqlStatementPtr stmt = ctx.GetChangeSummaryStatement("INSERT INTO " ECSCHEMA_ALIAS_ECDbChangeSummaries  "." ECDBCHANGE_CLASS_PropertyValueChange "(InstanceChange.Id, AccessString, RawOldValue, RawNewValue) VALUES (?,?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    stmt->BindId(1, changeId);
    stmt->BindText(2, accessString, IECSqlBinder::MakeCopy::No);

    if (oldValue.IsValid())
        stmt->BindId(3, oldValue);

    if (newValue.IsValid())
        stmt->BindId(4, newValue);

    return stmt->Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
DbResult ChangeSummaryExtractor::InsertPropertyValueChange(Context& ctx, ECInstanceId summaryId, ECInstanceKey const& keyOfChangedInstance, Utf8CP accessString, ECInstanceId oldValue, ECInstanceId newValue) const
    {
    ECInstanceId changeId = FindChangeId(ctx, summaryId, keyOfChangedInstance);

    CachedECSqlStatementPtr stmt = ctx.GetChangeSummaryStatement("INSERT INTO " ECSCHEMA_ALIAS_ECDbChangeSummaries  "." ECDBCHANGE_CLASS_PropertyValueChange "(InstanceChange.Id, AccessString, RawOldValue, RawNewValue) VALUES (?,?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    stmt->BindId(1, changeId);
    stmt->BindText(2, accessString, IECSqlBinder::MakeCopy::No);

    if (oldValue.IsValid())
        stmt->BindId(3, oldValue);

    if (newValue.IsValid())
        stmt->BindId(4, newValue);

    return stmt->Step();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                              Affan.Khan          10/2017
//---------------------------------------------------------------------------------------
//static
ECSqlStatus ChangeSummaryExtractor::BindDbValue(ECSqlStatement& stmt, int idx, DbValue const& value)
    {
    if (!value.IsValid() || value.GetValueType() == DbValueType::NullVal)
        return stmt.BindNull(idx);

    if (value.GetValueType() == DbValueType::BlobVal)
        return stmt.BindBlob(idx, value.GetValueBlob(), value.GetValueBytes(), IECSqlBinder::MakeCopy::No);

    if (value.GetValueType() == DbValueType::TextVal)
        return stmt.BindText(idx, value.GetValueText(), IECSqlBinder::MakeCopy::No);

    if (value.GetValueType() == DbValueType::FloatVal)
        return stmt.BindDouble(idx, value.GetValueDouble());

    if (value.GetValueType() == DbValueType::IntegerVal)
        return stmt.BindInt64(idx, value.GetValueInt64());

    return ECSqlStatus::Error;
    }


//****************************************************************************************
// ChangeSummaryExtractor::FkRelChangeExtractor
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryExtractor::FkRelChangeExtractor::Extract(Context& ctx, ECInstanceId summaryId, ChangeIterator::RowEntry const& rowEntry, ChangeIterator::TableClassMap::EndTableRelationshipMap const& endTableRelMap) const
    {
    // Check if the other end was/is valid to determine if there's really a relationship that was inserted/updated/deleted
    ChangeIterator::ColumnMap const& otherEndColumnMap = endTableRelMap.m_relatedInstanceIdColumnMap;
    ECInstanceId oldOtherEndInstanceId, newOtherEndInstanceId;
    rowEntry.GetSqlChange()->GetValueIds(oldOtherEndInstanceId, newOtherEndInstanceId, otherEndColumnMap.GetIndex());
    if (!oldOtherEndInstanceId.IsValid() && !newOtherEndInstanceId.IsValid())
        return SUCCESS; // no changes in relationship

    // Evaluate the relationship information
    ECN::ECClassId relClassId = endTableRelMap.m_relationshipClassId;
    if (!relClassId.IsValid())
        {
        ChangeIterator::ColumnMap const& relClassIdColumnMap = endTableRelMap.m_relationshipClassIdColumnMap;

        relClassId = rowEntry.GetClassIdFromChangeOrTable(relClassIdColumnMap.GetName().c_str(), rowEntry.GetPrimaryInstanceId());
        BeAssert(relClassId.IsValid());
        }
    ECInstanceId relInstanceId = rowEntry.GetPrimaryInstanceId();
    
    ECN::ECClassCP relClassRaw = ctx.GetPrimaryECDb().Schemas().GetClass(relClassId);
    if (relClassRaw == nullptr || !relClassRaw->IsRelationshipClass())
        {
        BeAssert(false);
        return ERROR;
        }
    ECRelationshipClassCR relClass = *relClassRaw->GetRelationshipClassCP();

    RelationshipClassEndTableMap const& relClassMap = ctx.GetSchemaManager().GetClassMap(relClass)->GetAs<RelationshipClassEndTableMap>();

    // Setup this end of the relationship (Note: EndInstanceId = RelationshipInstanceId)
    ECN::ECClassId thisEndClassId = rowEntry.GetPrimaryClass()->GetId();
    ECInstanceKey thisEndInstanceKey(thisEndClassId, relInstanceId);
    ECN::ECRelationshipEnd thisEnd = relClassMap.GetForeignEnd();

    // Setup other end of relationship
    ECN::ECRelationshipEnd otherEnd = (thisEnd == ECRelationshipEnd_Source) ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
    DbColumn const* otherEndClassIdColumn = endTableRelMap.GetForeignEndClassIdColumn(ctx.GetPrimaryECDb(), relClass);
    if (otherEndClassIdColumn == nullptr)
        {
        BeAssert(false && "Need to adjust code when constraint ecclassid column is nullptr");
        return ERROR;
        }

    ECClassId oldOtherEndClassId, newOtherEndClassId;
    if (otherEndClassIdColumn->IsVirtual())
        {
        // The table at the end contains a single class only - just use the relationship to get the end class
        ECRelationshipConstraintCR endConstraint = (otherEnd == ECRelationshipEnd_Source) ? relClass.GetSource() : relClass.GetTarget();
        if (endConstraint.GetConstraintClasses().size() != 1)
            {
            BeAssert(false && "Multiple classes at end. Cannot pick something arbitrary");
            return ERROR;
            }

        oldOtherEndClassId = endConstraint.GetConstraintClasses()[0]->GetId();
        newOtherEndClassId = oldOtherEndClassId;
        }
    else
        {
        DbTable const& otherEndTable = otherEndClassIdColumn->GetTable();
        if (otherEndTable.GetPrimaryKeyConstraint() == nullptr || otherEndTable.GetPrimaryKeyConstraint()->GetColumns().size() != 1)
            {
            BeAssert(false && "Other end table is expected to have primary key constraint with one column");
            return ERROR;
            }
         
        DbColumn const& otherEndPkCol = *otherEndTable.GetPrimaryKeyConstraint()->GetColumns()[0];

        if (newOtherEndInstanceId.IsValid())
            {
            newOtherEndClassId = DbUtilities::QueryRowClassId(ctx.GetPrimaryECDb(), otherEndTable.GetName(), otherEndClassIdColumn->GetName(), otherEndPkCol.GetName(),
                                                                         newOtherEndInstanceId);
            BeAssert(newOtherEndClassId.IsValid());
            }

        if (oldOtherEndInstanceId.IsValid())
            {
            oldOtherEndClassId = DbUtilities::QueryRowClassId(ctx.GetPrimaryECDb(), otherEndTable.GetName(), otherEndClassIdColumn->GetName(), otherEndPkCol.GetName(),
                                                                         oldOtherEndInstanceId);
            BeAssert(oldOtherEndClassId.IsValid());
            }
        }

    ECInstanceKey oldOtherEndInstanceKey, newOtherEndInstanceKey;
    if (newOtherEndInstanceId.IsValid())
        newOtherEndInstanceKey = ECInstanceKey(newOtherEndClassId, newOtherEndInstanceId);
    if (oldOtherEndInstanceId.IsValid())
        oldOtherEndInstanceKey = ECInstanceKey(oldOtherEndClassId, oldOtherEndInstanceId);

    // Setup the change instance of the relationship
    DbOpcode relDbOpcode;
    if (newOtherEndInstanceKey.IsValid() && !oldOtherEndInstanceKey.IsValid())
        relDbOpcode = DbOpcode::Insert;
    else if (!newOtherEndInstanceKey.IsValid() && oldOtherEndInstanceKey.IsValid())
        relDbOpcode = DbOpcode::Delete;
    else /* if (newOtherEndInstanceKey.IsValid() && oldOtherEndInstanceKey.IsValid()) */
        relDbOpcode = DbOpcode::Update;

    ECInstanceKeyCP oldSourceInstanceKey, newSourceInstanceKey, oldTargetInstanceKey, newTargetInstanceKey;
    ECInstanceKey invalidKey;
    oldSourceInstanceKey = newSourceInstanceKey = oldTargetInstanceKey = newTargetInstanceKey = nullptr;
    if (thisEnd == ECRelationshipEnd_Source)
        {
        oldSourceInstanceKey = (relDbOpcode != DbOpcode::Insert) ? &thisEndInstanceKey : &invalidKey;
        oldTargetInstanceKey = &oldOtherEndInstanceKey;
        newSourceInstanceKey = (relDbOpcode != DbOpcode::Delete) ? &thisEndInstanceKey : &invalidKey;
        newTargetInstanceKey = &newOtherEndInstanceKey;
        }
    else
        {
        oldSourceInstanceKey = &oldOtherEndInstanceKey;
        oldTargetInstanceKey = (relDbOpcode != DbOpcode::Insert) ? &thisEndInstanceKey : &invalidKey;
        newSourceInstanceKey = &newOtherEndInstanceKey;
        newTargetInstanceKey = (relDbOpcode != DbOpcode::Delete) ? &thisEndInstanceKey : &invalidKey;
        }

    InstanceChange instance(summaryId, ECInstanceKey(relClassId, relInstanceId), relDbOpcode, RawIndirectToBool(rowEntry.GetIndirect()));
    return m_extractor.RecordRelInstance(ctx, instance, rowEntry, *oldSourceInstanceKey, *newSourceInstanceKey, *oldTargetInstanceKey, *newTargetInstanceKey);
    }



//****************************************************************************************
// ChangeSummaryExtractor::LinkTableRelChangeExtractor
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                           Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryExtractor::LinkTableRelChangeExtractor::Extract(Context& ctx, ECInstanceId summaryId, ChangeIterator::RowEntry const& rowEntry, RelationshipClassLinkTableMap const& classMap) const
    {
    InstanceChange instance(summaryId, ECInstanceKey(rowEntry.GetPrimaryClass()->GetId(), rowEntry.GetPrimaryInstanceId()), rowEntry.GetDbOpcode(), RawIndirectToBool(rowEntry.GetIndirect()));

    ECInstanceKey oldSourceInstanceKey, newSourceInstanceKey;
    GetRelEndInstanceKeys(ctx, oldSourceInstanceKey, newSourceInstanceKey, summaryId, rowEntry, classMap, instance.GetKeyOfChangedInstance().GetInstanceId(), ECRelationshipEnd_Source);

    ECInstanceKey oldTargetInstanceKey, newTargetInstanceKey;
    GetRelEndInstanceKeys(ctx, oldTargetInstanceKey, newTargetInstanceKey, summaryId, rowEntry, classMap, instance.GetKeyOfChangedInstance().GetInstanceId(), ECRelationshipEnd_Target);

    return m_extractor.RecordRelInstance(ctx, instance, rowEntry, oldSourceInstanceKey, newSourceInstanceKey, oldTargetInstanceKey, newTargetInstanceKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryExtractor::LinkTableRelChangeExtractor::GetRelEndInstanceKeys(Context& ctx, ECInstanceKey& oldInstanceKey, ECInstanceKey& newInstanceKey, ECInstanceId summaryId, ChangeIterator::RowEntry const& rowEntry, RelationshipClassLinkTableMap const& relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd) const
    {
    oldInstanceKey = ECInstanceKey();
    newInstanceKey = ECInstanceKey();

    ConstraintECInstanceIdPropertyMap const* constraintIdPropMap = relClassMap.GetConstraintECInstanceIdPropMap(relEnd);
    if (constraintIdPropMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    GetColumnsPropertyMapVisitor columnsDisp(PropertyMap::Type::All, /* doNotSkipSystemPropertyMaps */ true);
    constraintIdPropMap->AcceptVisitor(columnsDisp);
    if (columnsDisp.GetColumns().size() != 1)
        return ERROR;

    int instanceIdColumnIndex = rowEntry.GetTableMap()->GetColumnIndexByName(columnsDisp.GetColumns()[0]->GetName());
    BeAssert(instanceIdColumnIndex >= 0);

    ECInstanceId oldEndInstanceId, newEndInstanceId;
    rowEntry.GetSqlChange()->GetValueIds(oldEndInstanceId, newEndInstanceId, instanceIdColumnIndex);

    if (newEndInstanceId.IsValid())
        {
        ECClassId newClassId = GetRelEndClassId(ctx, summaryId, rowEntry, relClassMap, relInstanceId, relEnd, newEndInstanceId);
        BeAssert(newClassId.IsValid());
        newInstanceKey = ECInstanceKey(newClassId, newEndInstanceId);
        }

    if (oldEndInstanceId.IsValid())
        {
        ECClassId oldClassId = GetRelEndClassId(ctx, summaryId, rowEntry, relClassMap, relInstanceId, relEnd, oldEndInstanceId);
        BeAssert(oldClassId.IsValid());
        oldInstanceKey = ECInstanceKey(oldClassId, oldEndInstanceId);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
ECN::ECClassId ChangeSummaryExtractor::LinkTableRelChangeExtractor::GetRelEndClassId(Context& ctx, ECInstanceId summaryId, ChangeIterator::RowEntry const& rowEntry, RelationshipClassLinkTableMap const& relationshipClassMap, ECInstanceId relationshipInstanceId, ECN::ECRelationshipEnd relEnd, ECInstanceId relEndInstanceId) const
    {
    ConstraintECClassIdPropertyMap const* classIdPropMap = relationshipClassMap.GetConstraintECClassIdPropMap(relEnd);
    if (classIdPropMap == nullptr)
        {
        BeAssert(false);
        return ECClassId();
        }

    GetColumnsPropertyMapVisitor columnsDisp(PropertyMap::Type::All, /* doNotSkipSystemPropertyMaps */ true);
    classIdPropMap->AcceptVisitor(columnsDisp);
    if (columnsDisp.GetColumns().size() != 1)
        {
        BeAssert(false);
        return ECClassId();
        }

    DbColumn const* classIdColumn = columnsDisp.GetColumns()[0];

    /*
    * There are various cases that need to be considered to resolve the constrained ECClassId at the end of a relationship:
    *
    * 1. By definition the end can point to only one class, and the ECClassId is implicit.
    *    + We use the mapping/relationship class to obtain the class id in this case.
    *    + We detect this case by checking if the ClassId constraint column is not persisted (i.e., PersistenceType = "Virtual")
    *
    * 2. By definition the end can point to only one table, and a ECClassId column in that "end" table stores the ECClassId.
    *    + We use the end instance id to search the ECClassId from the end table.
    *    + Since the end instance id is a different change record than the one currently being processed, we need to first search
    *      for ECClassId in all changes, and subsequently the DbTable itself.
    *    + Note that the end table could be the relationship table itself in case of foreign key constraints (e.g., Element's ParentId)
    *    + We detect this case by checking if the ClassId constraint column is persisted, and is setup as the primary "ECClassId" column
    *      of the table that contains it. Note that the latter check differentiates case #3.
    *
    * 3. By definition the end can point to many tables, and a Source/Target ECClassId column in the relationship table stores the ECClassId
    *    + We use the relationship instance id to search the Source/Target ECClassId column in the relationship table
    *    + Even if the Source/Target ECClassId is in the same row as the currently processed change, it may not be part of the
    *      change if it wasn't modified. Therefore we need to first search for the column in the current change, and subsequently the DbTable.
    *    + Note that like the previous case, the end table could be the same as the relationship table.
    *    + We detect this case by checking if the ClassId constraint column is persisted, and is *not* the primary "ECClassId" column of the
    *      table that contains it.
    *
    */

    // Case #1: End can point to only one class
    const bool endIsInOneClass = (classIdColumn->GetPersistenceType() == PersistenceType::Virtual);
    if (endIsInOneClass)
        {
        ECRelationshipClassCR relClass = *relationshipClassMap.GetClass().GetRelationshipClassCP();
        ECRelationshipConstraintCR endConstraint = (relEnd == ECRelationshipEnd_Source) ? relClass.GetSource() : relClass.GetTarget();
        if (endConstraint.GetConstraintClasses().size() != 1)
            {
            BeAssert(false && "Multiple classes at end. Cannot pick something arbitrary");
            return ECClassId();
            }

        return endConstraint.GetConstraintClasses()[0]->GetId();
        }

    // Case #2: End is in only one table (Note: not in the current table the row belongs to, but some OTHER end table)
    const bool endIsInOneTable = classIdPropMap->GetTables().size() == 1;
    if (endIsInOneTable)
        {
        DbTable const& endTable = classIdColumn->GetTable();
        if (endTable.GetPrimaryKeyConstraint() == nullptr || endTable.GetPrimaryKeyConstraint()->GetColumns().size() != 1)
            {
            BeAssert(false && "End table is expected to have a single col PK");
            return ECClassId();
            }
        // Search in the end table
        ECClassId classId = DbUtilities::QueryRowClassId(ctx.GetPrimaryECDb(), endTable.GetName(), classIdColumn->GetName(), endTable.GetPrimaryKeyConstraint()->GetColumns()[0]->GetName(), relEndInstanceId);
        BeAssert(classId.IsValid());
        return classId;
        }

    // Case #3: End could be in many tables
    Utf8StringCR classIdColumnName = classIdColumn->GetName();
    ECClassId classId = rowEntry.GetClassIdFromChangeOrTable(classIdColumnName.c_str(), relationshipInstanceId);
    BeAssert(classId.IsValid());
    return classId;
    }

//****************************************************************************************************
// ChangeSummaryExtractor::Context
//****************************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                            Krischan.Eberle    11/2017
//---------------------------------------------------------------------------------------
ChangeSummaryExtractor::Context::Context(ChangeSummaryManager& manager) : m_manager(manager), m_changeSummaryStmtCache(15) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                            Krischan.Eberle    11/2017
//---------------------------------------------------------------------------------------
ChangeSummaryExtractor::Context::~Context()
    {
    if (!m_changeSummaryECDb.IsDbOpen())
        return;

    if (BE_SQLITE_OK != m_changeSummaryECDb.SaveChanges())
        {
        Issues().Report("Failed to extract ChangeSummaries from changeset: Could not commit changes to ChangeSummary cache file %s", m_changeSummaryECDb.GetDbFileName());
        BeAssert(false);
        }

    m_changeSummaryStmtCache.Empty();
    m_changeSummaryECDb.CloseDb();

    if (m_wasChangeSummaryFileAttached)
        {
        if (BE_SQLITE_OK != m_manager.AttachChangeSummaryCacheFile(false))
            {
            Issues().Report("Failed to extract ChangeSummaries from changeset: Could not re-attach ChangeSummary cache file  to %s", GetPrimaryECDb().GetDbFileName());
            BeAssert(false);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                            Krischan.Eberle    11/2017
//---------------------------------------------------------------------------------------
DbResult ChangeSummaryExtractor::Context::OpenChangeSummaryECDb()
    {
    if (m_manager.ChangeSummaryTableSpaceExists())
        {
        m_wasChangeSummaryFileAttached = m_manager.IsChangeSummaryCacheAttachedAndValid(true);
        if (!m_wasChangeSummaryFileAttached)
            return BE_SQLITE_ERROR;
        }

    if (m_wasChangeSummaryFileAttached)
        {
        DbResult r = GetPrimaryECDb().DetachDb(TABLESPACE_ChangeSummaries);
        if (BE_SQLITE_OK != r)
            {
            Issues().Report("Failed to extract ChangeSummaries from changeset: Could not detach ChangeSummary cache file  from '%s': %s", GetPrimaryECDb().GetDbFileName(), GetPrimaryECDb().GetLastError().c_str());
            return r;
            }
        }

    BeAssert(!m_changeSummaryECDb.IsDbOpen());
    BeFileName path = GetPrimaryECDb().GetChangeSummaryCachePath();
    if (!path.DoesPathExist())
        {
        Issues().Report("Failed to extract ChangeSummaries from changeset: ChangeSummary cache file  %s not found.", path.GetNameUtf8().c_str());
        return BE_SQLITE_ERROR_FileNotFound;
        }

    //Must open the cache file with "DoNotAttach" to avoid that a cache to the cache is being created
    DbResult r = m_changeSummaryECDb.OpenBeSQLiteDb(path, ECDb::OpenParams(ECDb::OpenMode::ReadWrite).Set(ECDb::ChangeSummaryCacheMode::DoNotAttach));
    if (BE_SQLITE_OK != r)
        return r;

    if (!ChangeSummaryManager::IsChangeSummaryCacheValid(m_changeSummaryECDb, true))
        return BE_SQLITE_ERROR;

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                            Krischan.Eberle    11/2017
//---------------------------------------------------------------------------------------
IssueReporter const& ChangeSummaryExtractor::Context::Issues() const { return GetPrimaryECDb().GetImpl().Issues(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                            Krischan.Eberle    11/2017
//---------------------------------------------------------------------------------------
ECDbCR ChangeSummaryExtractor::Context::GetPrimaryECDb() const { return m_manager.GetECDb(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                            Krischan.Eberle    11/2017
//---------------------------------------------------------------------------------------
MainSchemaManager const& ChangeSummaryExtractor::Context::GetSchemaManager() const { return GetPrimaryECDb().Schemas().Main(); }

END_BENTLEY_SQLITE_EC_NAMESPACE