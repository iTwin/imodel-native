/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ChangeSummaryImplV2.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SqlNames.h"
#include "ChangeSummaryImpl.h"
#include "ChangeSummaryImplV2.h"
#define CHANGED_TABLES_TEMP_PREFIX "temp."
#define CHANGED_INSTANCES_TABLE_BASE_NAME "ec_ChangedInstances"
#define CHANGED_VALUES_TABLE_BASE_NAME "ec_ChangedValues"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                              Affan.Khan           11/2017
//---------------------------------------------------------------------------------------
static ChangeSummaryV2::Operation FomOpCode(DbOpcode code)
    {
    static std::map<DbOpcode, ChangeSummaryV2::Operation> map = 
        {
            {DbOpcode::Insert, ChangeSummaryV2::Operation::Inserted},
            {DbOpcode::Update, ChangeSummaryV2::Operation::Updated},
            {DbOpcode::Delete, ChangeSummaryV2::Operation::Deleted}
        };

    return map[code];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Affan.Khan          11/2017
//---------------------------------------------------------------------------------------
static DbOpcode ToOpCode(ChangeSummaryV2::Operation code)
    {
    static std::map<ChangeSummaryV2::Operation, DbOpcode> map =
        {
                {ChangeSummaryV2::Operation::Inserted, DbOpcode::Insert},
                {ChangeSummaryV2::Operation::Updated, DbOpcode::Update},
                {ChangeSummaryV2::Operation::Deleted, DbOpcode::Delete}
        };

    return map[code];
    }
//***********************************************************************************
// ChangeSummary
//***********************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryV2::Initialize()
    {
    if (m_isValid)
        return SUCCESS;

    m_instanceChangeManager = new InstanceChangeManager(*this);

    m_propertyValueChangeManager = new PropertyValueChangeManager(*m_instanceChangeManager);
    if (SUCCESS != m_propertyValueChangeManager->Initialize())
        return ERROR;

    m_changeExtractor = new ChangeExtractorV2(*this, *m_instanceChangeManager, *m_propertyValueChangeManager);
    if (CreateSummaryEntry() != SUCCESS)
        return ERROR;

    m_isValid = true;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryV2::Free()
    {
    if (!IsValid())
        return;

    delete m_instanceChangeManager;
    m_instanceChangeManager = nullptr;

    delete m_propertyValueChangeManager;
    m_propertyValueChangeManager = nullptr;

    delete m_changeExtractor;
    m_changeExtractor = nullptr;

    m_isValid = false;
    //if (DeleteSummaryEntry() != SUCCESS)
    //    {
    //    BeAssert(false);
    //    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryV2::FromChangeSet(IChangeSet& changeSet, ChangeSummaryV2::Options const& options)
    {
    Initialize();
    return m_changeExtractor->FromChangeSet(changeSet, options.IncludeRelationshipInstances());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeSummaryV2::ContainsInstance(ECInstanceKey const& key) const { return m_instanceChangeManager->ContainsChange(key); }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummaryV2::Instance ChangeSummaryV2::GetInstance(ECInstanceKey const& key) const { return m_instanceChangeManager->QueryChangedInstance(key); }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Affan.Khan           11/2017
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryV2::CreateSummaryEntry()
    {
    ECSqlStatement stmt;
    if (stmt.Prepare(m_ecdb, "INSERT INTO change.Summary (ECInstanceId) VALUES (NULL)") != ECSqlStatus::Success)
        return ERROR;

    ECInstanceKey instanceKey;
    if (stmt.Step(instanceKey) != BE_SQLITE_DONE)
        return ERROR;

    m_changeSummaryId = instanceKey.GetInstanceId();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Affan.Khan           11/2017
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryV2::DeleteSummaryEntry()
    {
    static auto deleteSummary = [] (ECDbCR ecdb, ECInstanceId summaryId)
        {
        ECSqlStatement stmt;
        if (stmt.Prepare(ecdb, "DELETE FROM change.Summary WHERE ECInstanceId = ?") != ECSqlStatus::Success)
            return ERROR;

        stmt.BindId(1, summaryId);
        if (stmt.Step() != BE_SQLITE_DONE)
            return ERROR;

        return SUCCESS;
        };

    static auto deleteInstance = [] (ECDbCR ecdb, ECInstanceId summaryId)
        {
        ECSqlStatement stmt;
        if (stmt.Prepare(ecdb, "DELETE FROM change.Instance WHERE Summary.Id = ?") != ECSqlStatus::Success)
            return ERROR;

        stmt.BindId(1, summaryId);
        if (stmt.Step() != BE_SQLITE_DONE)
            return ERROR;

        return SUCCESS;
        };

    static auto deletePropertyValue = [] (ECDbCR ecdb, ECInstanceId summaryId)
        {
        ECSqlStatement stmt;
        if (stmt.Prepare(ecdb, "DELETE FROM change.PropertyValue WHERE Instance.Id IN (SELECT ECInstanceId FROM change.Instance WHERE Summary.Id = ?)") != ECSqlStatus::Success)
            return ERROR;

        stmt.BindId(1, summaryId);
        if (stmt.Step() != BE_SQLITE_DONE)
            return ERROR;

        return SUCCESS;
        };

    if (!m_changeSummaryId.IsValid())
        return SUCCESS;

    if (deletePropertyValue(m_ecdb, m_changeSummaryId) != SUCCESS)
        return ERROR;

    if (deleteInstance(m_ecdb, m_changeSummaryId) != SUCCESS)
        return ERROR;

    if (deleteSummary(m_ecdb, m_changeSummaryId) != SUCCESS)
        return ERROR;

    m_changeSummaryId = ECInstanceId();
    return SUCCESS;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2015
//---------------------------------------------------------------------------------------
Utf8String ChangeSummaryV2::ConstructWhereInClause(OpCode queryDbOpcodes) const
    {
    Utf8String whereInStr;
    if (OpCode::None != (queryDbOpcodes & OpCode::Insert))
        {
        Utf8PrintfString addStr("%d", Enum::ToInt(DbOpcode::Insert));
        whereInStr.append(addStr);
        }
    if (OpCode::None != (queryDbOpcodes & OpCode::Update))
        {
        if (!whereInStr.empty())
            whereInStr.append(",");

        Utf8PrintfString addStr("%d", Enum::ToInt(DbOpcode::Update));
        whereInStr.append(addStr);
        }
    if (OpCode::None != (queryDbOpcodes & OpCode::Delete))
        {
        if (!whereInStr.empty())
            whereInStr.append(",");
        Utf8PrintfString addStr("%d", Enum::ToInt(DbOpcode::Delete));
        whereInStr.append(addStr);
        }

    BeAssert(!whereInStr.empty());

    return Utf8PrintfString("Operation IN (%s)", whereInStr.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
// static
BentleyStatus ChangeSummaryV2::GetMappedPrimaryTable(Utf8StringR tableName, bool& isTablePerHierarcy, ECN::ECClassCR ecClass, ECDbCR ecdb)
    {
    // TODO: This functionality needs to be moved to some publicly available ECDb mapping utility. 
    ClassMapCP classMap = ecdb.Schemas().GetDbMap().GetClassMap(ecClass);
    if (!classMap)
        return ERROR;

    DbTable& table = classMap->GetPrimaryTable();
    if (!table.IsValid())
        return ERROR;

    tableName = classMap->GetPrimaryTable().GetName();
    isTablePerHierarcy = classMap->GetMapStrategy().IsTablePerHierarchy();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
Utf8String ChangeSummaryV2::FormatInstanceIdStr(ECInstanceId id) const
    {
    if (!id.IsValid())
        return "NULL";

    const uint64_t idVal = id.GetValue();

    uint32_t briefcaseId = (uint32_t) ((idVal << 40) & 0xffffff);
    uint64_t localId = (uint64_t) (0xffffffffffLL & idVal);

    Utf8PrintfString idStr("%" PRIu32 ":%" PRIu64, briefcaseId, localId);
    return idStr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
Utf8String ChangeSummaryV2::FormatClassIdStr(ECClassId id) const
    {
    if (!id.IsValid())
        return "NULL";

    ECN::ECClassCP ecClass = m_ecdb.Schemas().GetClass(id);
    BeAssert(ecClass != nullptr);

    Utf8PrintfString idStr("%s:%" PRIu64, ecClass->GetFullName(), id.GetValue());
    return idStr;
    }

//***********************************************************************************
// ChangeSummary::Instance
//***********************************************************************************


//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummaryV2::Instance& ChangeSummaryV2::Instance::operator=(Instance const& other)
    {
    m_keyOfChangedInstance = other.m_keyOfChangedInstance;
    m_dbOpcode = other.m_dbOpcode;
    m_isIndirect = other.m_isIndirect;
    m_changeSummary = other.m_changeSummary;
    m_tableName = other.m_tableName;
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryV2::Instance::SetupValuesTableSelectStatement(Utf8CP accessString) const
    {//TODO ToECSql
    if (!m_valuesTableSelect.IsPrepared())
        {
        m_valuesTableSelect.Prepare(m_changeSummary->GetECDb(), "SELECT RawOldValue, RawNewValue FROM change.PropertyValue WHERE ClassId=? AND InstanceId=? AND AccessString=?");
        m_valuesTableSelect.BindId(1, m_keyOfChangedInstance.GetClassId());
        m_valuesTableSelect.BindId(2, m_keyOfChangedInstance.GetInstanceId());
        }

    m_valuesTableSelect.Reset();
    m_valuesTableSelect.BindText(3, accessString, IECSqlBinder::MakeCopy::No);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2015
//---------------------------------------------------------------------------------------
DbDupValue ChangeSummaryV2::Instance::GetOldValue(Utf8CP accessString) const
    {
    BeAssert(IsValid());
    //TODO ToECSql
    //if (IsValid())
    //   {
    //   SetupValuesTableSelectStatement(accessString);
    //   DbResult result = m_valuesTableSelect.Step();
    //   if (result == BE_SQLITE_ROW)
    //       return m_valuesTableSelect.GetDbValue(0);
    //   BeAssert(result == BE_SQLITE_DONE);
    //   }

    DbDupValue invalidValue(nullptr);
    return invalidValue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2015
//---------------------------------------------------------------------------------------
DbDupValue ChangeSummaryV2::Instance::GetNewValue(Utf8CP accessString) const
    {
    BeAssert(IsValid());
    //TODO ToECSql
    //if (IsValid())
    //    {
    //    SetupValuesTableSelectStatement(accessString);
    //    DbResult result = m_valuesTableSelect.Step();
    //    if (result == BE_SQLITE_ROW)
    //        return m_valuesTableSelect.GetDbValue(1);
    //    BeAssert(result == BE_SQLITE_DONE);
    //    }

    DbDupValue invalidValue(nullptr);
    return invalidValue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2015
//---------------------------------------------------------------------------------------
bool ChangeSummaryV2::Instance::ContainsValue(Utf8CP accessString) const
    {
    if (!IsValid())
        {
        BeAssert(false);
        return false;
        }

    SetupValuesTableSelectStatement(accessString);

    DbResult result = m_valuesTableSelect.Step();
    BeAssert(result == BE_SQLITE_DONE || result == BE_SQLITE_ROW);

    return (result == BE_SQLITE_ROW);
    }



//***********************************************************************************
// ChangeExtractorV2
//***********************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
BentleyStatus ChangeExtractorV2::FromChangeSet(IChangeSet& changeSet, bool includeRelationshipInstances)
    {
    // Pass 1
    BentleyStatus status = FromChangeSet(changeSet, ExtractOption::InstancesOnly);
    if (SUCCESS != status || !includeRelationshipInstances)
        return status;

    // Pass 2
    return FromChangeSet(changeSet, ExtractOption::RelationshipInstancesOnly);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
BentleyStatus ChangeExtractorV2::FromChangeSet(IChangeSet& changeSet, ExtractOption extractOption)
    {
    ChangeIterator iter(m_changeSummary.GetECDb(), changeSet);
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
            LOG.errorv("Could not determine the primary instance corresponding to a change to table %s", rowEntry.GetTableName().c_str());
            BeAssert(false && "Could not determine the primary instance corresponding to a change.");
            return ERROR;
            }

        if (extractOption == ExtractOption::InstancesOnly && !primaryClass->IsRelationshipClass())
            {
            ExtractInstance(rowEntry);
            continue;
            }

        if (extractOption == ExtractOption::RelationshipInstancesOnly)
            {
            ExtractRelInstances(rowEntry);
            continue;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
int ChangeExtractorV2::GetFirstColumnIndex(PropertyMap const* propertyMap, ChangeIterator::RowEntry const& rowEntry) const
    {
    if (propertyMap == nullptr)
        return -1;

    GetColumnsPropertyMapVisitor columnsDisp(PropertyMap::Type::All, /* doNotSkipSystemPropertyMaps */ true);
    propertyMap->AcceptVisitor(columnsDisp);
    if (columnsDisp.GetColumns().size() != 1)
        return -1;

    return rowEntry.GetTableMap()->GetColumnIndexByName(columnsDisp.GetColumns()[0]->GetName());
    }




//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
BentleyStatus ChangeExtractorV2::ExtractInstance(ChangeIterator::RowEntry const& rowEntry)
    {
    ChangeSummaryV2::Instance instance(m_changeSummary, ECInstanceKey(rowEntry.GetPrimaryClass()->GetId(), rowEntry.GetPrimaryInstanceId()), rowEntry.GetDbOpcode(), 
                                       RawIndirectToBool(rowEntry.GetIndirect()), rowEntry.GetTableName());
    bool recordOnlyIfUpdatedProperties = (rowEntry.GetDbOpcode() == DbOpcode::Update);
    return RecordInstance(instance, rowEntry, recordOnlyIfUpdatedProperties);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
BentleyStatus ChangeExtractorV2::ExtractRelInstances(ChangeIterator::RowEntry const& rowEntry)
    {
    ECClassCP primaryClass = rowEntry.GetPrimaryClass();

    ClassMap const* classMap = m_changeSummary.GetECDb().Schemas().GetDbMap().GetClassMap(*primaryClass);
    BeAssert(classMap != nullptr);

    ClassMap::Type type = classMap->GetType();
    if (type == ClassMap::Type::RelationshipLinkTable)
        {
        RelationshipClassLinkTableMap const& relClassMap = classMap->GetAs<RelationshipClassLinkTableMap>();
        return ExtractRelInstanceInLinkTable(rowEntry, relClassMap);
        }

    TableClassMap const* tableClassMap = rowEntry.GetTableMap()->GetTableClassMap(*primaryClass);
    BeAssert(tableClassMap != nullptr);

    for (TableClassMap::EndTableRelationshipMap const* endTableRelMap : tableClassMap->GetEndTableRelationshipMaps())
        {
        if (SUCCESS != ExtractRelInstanceInEndTable(rowEntry, *endTableRelMap))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
BentleyStatus ChangeExtractorV2::ExtractRelInstanceInLinkTable(ChangeIterator::RowEntry const& rowEntry, RelationshipClassLinkTableMap const& relClassMap)
    {
    ChangeSummaryV2::Instance instance(m_changeSummary, ECInstanceKey(rowEntry.GetPrimaryClass()->GetId(), rowEntry.GetPrimaryInstanceId()), rowEntry.GetDbOpcode(),
                                      RawIndirectToBool(rowEntry.GetIndirect()), rowEntry.GetTableName());

    ECInstanceKey oldSourceInstanceKey, newSourceInstanceKey;
    GetRelEndInstanceKeys(oldSourceInstanceKey, newSourceInstanceKey, rowEntry, relClassMap, instance.GetKeyOfChangedInstance().GetInstanceId(), ECRelationshipEnd_Source);

    ECInstanceKey oldTargetInstanceKey, newTargetInstanceKey;
    GetRelEndInstanceKeys(oldTargetInstanceKey, newTargetInstanceKey, rowEntry, relClassMap, instance.GetKeyOfChangedInstance().GetInstanceId(), ECRelationshipEnd_Target);

    return RecordRelInstance(instance, rowEntry, oldSourceInstanceKey, newSourceInstanceKey, oldTargetInstanceKey, newTargetInstanceKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
ECN::ECClassId ChangeExtractorV2::GetClassIdFromColumn(TableMap const& tableMap, DbColumn const& classIdColumn, ECInstanceId instanceId) const
    {
    // Search in all changes
    ECClassId classId = m_instanceChangeManager.QueryClassIdOfChangedInstance(tableMap.GetTableName(), instanceId);
    if (classId.IsValid())
        return classId;

    // Search in table itself
    classId = tableMap.QueryValueId<ECClassId>(classIdColumn.GetName(), instanceId);
    BeAssert(classId.IsValid());

    return classId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeExtractorV2::ExtractRelInstanceInEndTable(ChangeIterator::RowEntry const& rowEntry, TableClassMap::EndTableRelationshipMap const& endTableRelMap)
    {
    // Check if the other end was/is valid to determine if there's really a relationship that was inserted/updated/deleted
    ColumnMap const& otherEndColumnMap = endTableRelMap.m_relatedInstanceIdColumnMap;
    ECInstanceId oldOtherEndInstanceId, newOtherEndInstanceId;
    rowEntry.GetSqlChange()->GetValueIds(oldOtherEndInstanceId, newOtherEndInstanceId, otherEndColumnMap.GetIndex());
    if (!oldOtherEndInstanceId.IsValid() && !newOtherEndInstanceId.IsValid())
        return ERROR;

    // Evaluate the relationship information
    ECN::ECClassId relClassId = endTableRelMap.m_relationshipClassId;
    if (!relClassId.IsValid())
        {
        ColumnMap const& relClassIdColumnMap = endTableRelMap.m_relationshipClassIdColumnMap;

        relClassId = rowEntry.GetClassIdFromChangeOrTable(relClassIdColumnMap.GetName().c_str(), rowEntry.GetPrimaryInstanceId());
        BeAssert(relClassId.IsValid());
        }
    ECInstanceId relInstanceId = rowEntry.GetPrimaryInstanceId();

    ECN::ECClassCP relClass = m_changeSummary.GetECDb().Schemas().GetClass(relClassId);
    BeAssert(relClass != nullptr);
    RelationshipClassEndTableMap const* relClassMap = dynamic_cast<RelationshipClassEndTableMap const*> (m_changeSummary.GetECDb().Schemas().GetDbMap().GetClassMap(*relClass));
    BeAssert(relClassMap != nullptr);

    // Setup this end of the relationship (Note: EndInstanceId = RelationshipInstanceId)
    ECN::ECClassId thisEndClassId = rowEntry.GetPrimaryClass()->GetId();
    ECInstanceKey thisEndInstanceKey(thisEndClassId, relInstanceId);
    ECN::ECRelationshipEnd thisEnd = relClassMap->GetForeignEnd();

    // Setup other end of relationship
    ECN::ECRelationshipEnd otherEnd = (thisEnd == ECRelationshipEnd_Source) ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
    DbColumn const* otherEndClassIdColumn = endTableRelMap.GetForeignEndClassIdColumn(m_changeSummary.GetECDb(), *relClass->GetRelationshipClassCP());
    if (otherEndClassIdColumn == nullptr)
        {
        BeAssert(false && "Need to adjust code when constraint ecclassid column is nullptr");
        return ERROR;
        }

    ECClassId oldOtherEndClassId, newOtherEndClassId;
    if (otherEndClassIdColumn->IsVirtual())
        {
        // The table at the end contains a single class only - just use the relationship to get the end class
        oldOtherEndClassId = newOtherEndClassId = GetRelEndClassIdFromRelClass(relClass->GetRelationshipClassCP(), otherEnd);
        }
    else
        {
        TableMap const* otherEndTableMap = rowEntry.GetChangeIterator().GetTableMap(otherEndClassIdColumn->GetTable().GetName());
        BeAssert(otherEndTableMap != nullptr);

        if (newOtherEndInstanceId.IsValid())
            newOtherEndClassId = GetClassIdFromColumn(*otherEndTableMap, *otherEndClassIdColumn, newOtherEndInstanceId);
        if (oldOtherEndInstanceId.IsValid())
            oldOtherEndClassId = GetClassIdFromColumn(*otherEndTableMap, *otherEndClassIdColumn, oldOtherEndInstanceId);
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

    ChangeSummaryV2::Instance instance(m_changeSummary, ECInstanceKey(relClassId, relInstanceId), relDbOpcode, RawIndirectToBool(rowEntry.GetIndirect()), rowEntry.GetTableName());
    return RecordRelInstance(instance, rowEntry, *oldSourceInstanceKey, *newSourceInstanceKey, *oldTargetInstanceKey, *newTargetInstanceKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeExtractorV2::RecordRelInstance(ChangeSummaryV2::Instance const& instance, ChangeIterator::RowEntry const& rowEntry, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey)
    {
    if (SUCCESS != RecordInstance(instance, rowEntry, false)) // Even if any of the properties of the relationship is not updated, the relationship needs to be recorded since the source/target keys would have changed (to get here)
        return ERROR;

    if (BE_SQLITE_DONE != m_propertyValueChangeManager.Insert(instance.GetKeyOfChangedInstance(), ECDBSYS_PROP_SourceECClassId, oldSourceKey.IsValid() ? oldSourceKey.GetClassId() : ECClassId(), newSourceKey.IsValid() ? newSourceKey.GetClassId() : ECClassId()))
        return ERROR;

    if (BE_SQLITE_DONE != m_propertyValueChangeManager.Insert(instance.GetKeyOfChangedInstance(), ECDBSYS_PROP_SourceECInstanceId, oldSourceKey.IsValid() ? oldSourceKey.GetInstanceId() : ECInstanceId(), newSourceKey.IsValid() ? newSourceKey.GetInstanceId() : ECInstanceId()))
        return ERROR;

    if (BE_SQLITE_DONE != m_propertyValueChangeManager.Insert(instance.GetKeyOfChangedInstance(), ECDBSYS_PROP_TargetECClassId, oldTargetKey.IsValid() ? oldTargetKey.GetClassId() : ECClassId(), newTargetKey.IsValid() ? newTargetKey.GetClassId() : ECClassId()))
        return ERROR;
   
    return m_propertyValueChangeManager.Insert(instance.GetKeyOfChangedInstance(), ECDBSYS_PROP_TargetECInstanceId, oldTargetKey.IsValid() ? oldTargetKey.GetInstanceId() : ECInstanceId(), newTargetKey.IsValid() ? newTargetKey.GetInstanceId() : ECInstanceId()) == BE_SQLITE_DONE ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeExtractorV2::RecordInstance(ChangeSummaryV2::InstanceCR instance, ChangeIterator::RowEntry const& rowEntry, bool recordOnlyIfUpdatedProperties)
    {
    bool removeIfNotUpdatedProperties = false;
    if (recordOnlyIfUpdatedProperties)
        removeIfNotUpdatedProperties = !m_instanceChangeManager.ContainsChange(instance.GetKeyOfChangedInstance());

    DbResult stat = m_instanceChangeManager.InsertOrUpdate(instance);
    if (BE_SQLITE_DONE != stat)
        return ERROR;

    bool updatedProperties = false;
    ECN::ECClassCP ecClass = m_changeSummary.GetECDb().Schemas().GetClass(instance.GetKeyOfChangedInstance().GetClassId());
    for (ChangeIterator::ColumnEntry const& columnEntry : rowEntry.MakeColumnIterator(*ecClass))
        {
        if (columnEntry.IsPrimaryKeyColumn())
            continue;  // Primary key columns need not be included in the values table

        bool isNoNeedToRecord = false;
        if (SUCCESS != RecordValue(isNoNeedToRecord, instance, columnEntry))
            return ERROR;

        if (isNoNeedToRecord) //nothing had to be recorded;
            continue;

        updatedProperties = true;
        }

    if (removeIfNotUpdatedProperties && !updatedProperties)
        return SUCCESS;

    // If recording an update for the first time, and none of the properties have really been updated, remove record of the updated instance
    return BE_SQLITE_DONE == m_instanceChangeManager.Delete(instance.GetKeyOfChangedInstance()) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
BentleyStatus ChangeExtractorV2::RecordValue(bool& isNoNeedToRecord, ChangeSummaryV2::Instance const& instance, ChangeIterator::ColumnEntry const& columnEntry)
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

    return BE_SQLITE_DONE == m_propertyValueChangeManager.Insert(instance.GetKeyOfChangedInstance(), columnEntry.GetPropertyAccessString().c_str(), oldValue, newValue) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractorV2::GetRelEndInstanceKeys(ECInstanceKey& oldInstanceKey, ECInstanceKey& newInstanceKey, ChangeIterator::RowEntry const& rowEntry, RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd) const
    {
    oldInstanceKey = ECInstanceKey(); 
    newInstanceKey = ECInstanceKey();

    int instanceIdColumnIndex = GetFirstColumnIndex(relClassMap.GetConstraintECInstanceIdPropMap(relEnd), rowEntry);
    BeAssert(instanceIdColumnIndex >= 0);

    ECInstanceId oldEndInstanceId, newEndInstanceId;
    rowEntry.GetSqlChange()->GetValueIds(oldEndInstanceId, newEndInstanceId, instanceIdColumnIndex);

    if (newEndInstanceId.IsValid())
        {
        ECClassId newClassId = GetRelEndClassId(rowEntry, relClassMap, relInstanceId, relEnd, newEndInstanceId);
        BeAssert(newClassId.IsValid());
        newInstanceKey = ECInstanceKey(newClassId, newEndInstanceId);
        }

    if (oldEndInstanceId.IsValid())
        {
        ECClassId oldClassId = GetRelEndClassId(rowEntry, relClassMap, relInstanceId, relEnd, oldEndInstanceId);
        BeAssert(oldClassId.IsValid());
        oldInstanceKey = ECInstanceKey(oldClassId, oldEndInstanceId);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
ECN::ECClassId ChangeExtractorV2::GetRelEndClassId(ChangeIterator::RowEntry const& rowEntry, RelationshipClassMapCR relationshipClassMap, ECInstanceId relationshipInstanceId, ECN::ECRelationshipEnd relEnd, ECInstanceId relEndInstanceId) const
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
        // TODO: dynamic_cast<PropertyMapRelationshipConstraintClassId const*> (propMap)->GetDefaultConstraintECClassId()
        // should work, but doesn't for link tables - need to check with Krischan/Affan. 
        return GetRelEndClassIdFromRelClass(relationshipClassMap.GetClass().GetRelationshipClassCP(), relEnd);
        }

    // Case #2: End is in only one table (Note: not in the current table the row belongs to, but some OTHER end table)
    const bool endIsInOneTable = classIdPropMap->GetTables().size() == 1;
    if (endIsInOneTable)
        {
        Utf8StringCR endTableName = classIdColumn->GetTable().GetName();

        // Search in all changes
        ECClassId classId = m_instanceChangeManager.QueryClassIdOfChangedInstance(endTableName, relEndInstanceId);
        if (classId.IsValid())
            return classId;

        // Search in the end table
        classId = rowEntry.GetChangeIterator().GetTableMap(endTableName)->QueryValueId<ECClassId>(classIdColumn->GetName(), relEndInstanceId);
        BeAssert(classId.IsValid());
        return classId;
        }

    // Case #3: End could be in many tables
    Utf8StringCR classIdColumnName = classIdColumn->GetName();
    ECClassId classId = rowEntry.GetClassIdFromChangeOrTable(classIdColumnName.c_str(), relationshipInstanceId);
    BeAssert(classId.IsValid());
    return classId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
bool ChangeExtractorV2::ClassIdMatchesConstraint(ECN::ECClassId relClassId, ECN::ECRelationshipEnd end, ECN::ECClassId candidateClassId) const
    {
    bmap<ECN::ECClassId, LightweightCache::RelationshipEnd> const& constraintClassIds = m_changeSummary.GetECDb().Schemas().GetDbMap().GetLightweightCache().GetConstraintClassesForRelationshipClass(relClassId);
    auto it = constraintClassIds.find(candidateClassId);
    if (it == constraintClassIds.end())
        return false;

    const LightweightCache::RelationshipEnd requiredEnd = end == ECRelationshipEnd::ECRelationshipEnd_Source ? LightweightCache::RelationshipEnd::Source : LightweightCache::RelationshipEnd::Target;
    const LightweightCache::RelationshipEnd actualEnd = it->second;
    return actualEnd == LightweightCache::RelationshipEnd::Both || actualEnd == requiredEnd;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
// static
ECN::ECClassId ChangeExtractorV2::GetRelEndClassIdFromRelClass(ECN::ECRelationshipClassCP relClass, ECN::ECRelationshipEnd relEnd)
    {
    if (relClass == nullptr)
        {
        BeAssert(false);
        return ECClassId();
        }

    ECRelationshipConstraintR endConstraint = (relEnd == ECRelationshipEnd_Source) ? relClass->GetSource() : relClass->GetTarget();
    ECRelationshipConstraintClassList const& endClasses = endConstraint.GetConstraintClasses();
    if (endClasses.size() != 1)
        {
        BeAssert(false && "Multiple classes at end. Cannot pick something arbitrary");
        return ECClassId();
        }

    ECClassId classId = endClasses[0]->GetId();
    BeAssert(classId.IsValid());
    return classId;
    }

//***********************************************************************************
// InstanceChangeManager
//***********************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
DbResult InstanceChangeManager::InsertOrUpdate(ChangeSummaryV2::Instance const& instance)
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

    ChangeSummaryV2::Instance foundInstance = QueryChangedInstance(instance.GetKeyOfChangedInstance());
    if (!foundInstance.IsValid())
        {
        CachedECSqlStatementPtr stmt = m_stmtCache.GetPreparedStatement(m_changeSummary.GetECDb(), "INSERT INTO change.Instance(ClassIdOfChangedInstance, IdOfChangedInstance, Operation, IsIndirect, TableName, Summary) VALUES (?,?,?,?,?,?)");
        if (stmt == nullptr)
            {
            BeAssert(false);
            return BE_SQLITE_ERROR;
            }

        stmt->BindId(1, instance.GetKeyOfChangedInstance().GetClassId());
        stmt->BindId(2, instance.GetKeyOfChangedInstance().GetInstanceId());
        stmt->BindInt(3, (int) dbOpcode);
        stmt->BindBoolean(4, instance.IsIndirect());
        stmt->BindText(5, instance.GetTableName().c_str(), IECSqlBinder::MakeCopy::No);
        stmt->BindNavigationValue(6, m_changeSummary.GetId());

        return stmt->Step();
        }

    if (foundInstance.GetDbOpcode() == DbOpcode::Update && dbOpcode != DbOpcode::Update)
        {
        CachedECSqlStatementPtr stmt = m_stmtCache.GetPreparedStatement(m_changeSummary.GetECDb(), "UPDATE change.Instance SET Operation=?, IsIndirect=? WHERE ClassIdOfChangedInstance=? AND IdOfChangedInstance=? AND Summary.Id=?");
        if (stmt == nullptr)
            {
            BeAssert(false);
            return BE_SQLITE_ERROR;
            }

        stmt->BindInt(1, (int) dbOpcode);
        stmt->BindBoolean(2, instance.IsIndirect());
        stmt->BindId(3, instance.GetKeyOfChangedInstance().GetClassId());
        stmt->BindId(4, instance.GetKeyOfChangedInstance().GetInstanceId());
        stmt->BindId(5, m_changeSummary.GetId());

        return stmt->Step();
        }

    return BE_SQLITE_DONE;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
DbResult InstanceChangeManager::Delete(ECInstanceKey const& keyOfChangedInstance)
    {
    CachedECSqlStatementPtr stmt = m_stmtCache.GetPreparedStatement(m_changeSummary.GetECDb(), "DELETE FROM change.Instance WHERE ClassIdOfChangedInstance=? AND IdOfChangedInstance=? AND Summary.Id=?");
    if (stmt == nullptr)
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    stmt->BindId(1, keyOfChangedInstance.GetClassId());
    stmt->BindId(2, keyOfChangedInstance.GetInstanceId());
    stmt->BindId(3, m_changeSummary.GetId());

    return stmt->Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummaryV2::Instance InstanceChangeManager::QueryChangedInstance(ECInstanceKey const& keyofChangedInstance) const
    {
    ChangeSummaryV2::Instance instance;

    CachedECSqlStatementPtr stmt = m_stmtCache.GetPreparedStatement(m_changeSummary.GetECDb(), "SELECT OpCode,IsIndirect,TableName FROM change.Instance WHERE ClassIdOfChangedInstance=? AND IdOfChangedInstance=? AND Summary.Id=?");
    if (stmt == nullptr)
        {
        BeAssert(false);
        return instance;
        }

    stmt->BindId(1, keyofChangedInstance.GetClassId());
    stmt->BindId(2, keyofChangedInstance.GetInstanceId());
    stmt->BindId(3, m_changeSummary.GetId());

    if (BE_SQLITE_ROW == stmt->Step())
        return ChangeSummaryV2::Instance(m_changeSummary, keyofChangedInstance, (DbOpcode) stmt->GetValueInt(0), stmt->GetValueBoolean(1), Utf8String(stmt->GetValueText(2)));
    
    return instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Affan.Khan           10/2017
//---------------------------------------------------------------------------------------
ECInstanceId InstanceChangeManager::FindChangeId(ECInstanceKey const& keyofChangedInstance) const
    {
    CachedECSqlStatementPtr stmt = m_stmtCache.GetPreparedStatement(m_changeSummary.GetECDb(), "SELECT ECInstanceId FROM change.Instance WHERE ClassIdOfChangedInstance=? AND IdOfChangedInstance=? AND Summary.Id=?");
    if (stmt == nullptr)
        {
        BeAssert(false);
        return ECInstanceId();
        }

    stmt->BindId(1, keyofChangedInstance.GetClassId());
    stmt->BindId(2, keyofChangedInstance.GetInstanceId());
    stmt->BindId(3, m_changeSummary.GetId());

    if (stmt->Step() == BE_SQLITE_ROW)
        return stmt->GetValueId<ECInstanceId>(0);

    return ECInstanceId();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
ECClassId InstanceChangeManager::QueryClassIdOfChangedInstance(Utf8StringCR tableName, ECInstanceId instanceId) const
    {
    CachedECSqlStatementPtr stmt = m_stmtCache.GetPreparedStatement(m_changeSummary.GetECDb(), "SELECT ClassIdOfChangedInstance FROM change.Instance WHERE IdOfChangedInstance=? AND Summary.Id=? AND TableName=?");
    if (stmt == nullptr)
        {
        BeAssert(false);
        return ECClassId();
        }

    stmt->BindId(1, instanceId);
    stmt->BindId(2, m_changeSummary.GetId());
    stmt->BindText(3, tableName.c_str(), IECSqlBinder::MakeCopy::No);

    if (stmt->Step() == BE_SQLITE_ROW)
        return stmt->GetValueId<ECClassId>(0);

    return ECClassId();
    }

//***********************************************************************************
// PropertyValueChangeManager
//***********************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ECSqlStatus PropertyValueChangeManager::PrepareStatements()
    {
    BeAssert(!m_valuesTableInsert.IsPrepared());
    return m_valuesTableInsert.Prepare(m_changeSummary.GetECDb(), "INSERT INTO change.PropertyValue(Instance, AccessString, RawOldValue, RawNewValue) VALUES (?1, ?2, ?3, ?4)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Affan.Khan          10/2017
//---------------------------------------------------------------------------------------
//static
ECSqlStatus PropertyValueChangeManager::BindDbValue(ECSqlStatement& stmt, int idx, DbValue const& value)
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

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
DbResult PropertyValueChangeManager::Insert(ECInstanceKey const& keyofChangedInstance, Utf8CP accessString, DbValue const& oldValue, DbValue const& newValue)
    {
    ECSqlStatement& statement = m_valuesTableInsert;
    ECInstanceId instanceId = m_instancesTable.FindChangeId(keyofChangedInstance);
    statement.BindNavigationValue(1, instanceId);
    statement.BindText(2, accessString, IECSqlBinder::MakeCopy::No);
    BindDbValue(statement, 3, oldValue);
    BindDbValue(statement, 4, newValue);

    const DbResult result = statement.Step();
    statement.Reset();
    statement.ClearBindings();
    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
DbResult PropertyValueChangeManager::Insert(ECInstanceKey const& keyofChangedInstance, Utf8CP accessString, ECClassId oldValue, ECClassId newValue)
    {
    ECSqlStatement& statement = m_valuesTableInsert;
    ECInstanceId instanceId = m_instancesTable.FindChangeId(keyofChangedInstance);
    statement.BindNavigationValue(1, instanceId);
    statement.BindText(2, accessString, IECSqlBinder::MakeCopy::No);

    if (oldValue.IsValid())
        statement.BindId(3, oldValue);
    else
        statement.BindNull(3);

    if (newValue.IsValid())
        statement.BindId(4, newValue);
    else
        statement.BindNull(4);

    const DbResult result = statement.Step();
    statement.Reset();
    statement.ClearBindings();
    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
DbResult PropertyValueChangeManager::Insert(ECInstanceKey const& keyofChangedInstance, Utf8CP accessString, ECInstanceId oldValue, ECInstanceId newValue)
    {
    ECSqlStatement& statement = m_valuesTableInsert;
    ECInstanceId instanceId = m_instancesTable.FindChangeId(keyofChangedInstance);
    statement.BindNavigationValue(1, instanceId);
    statement.BindText(2, accessString, IECSqlBinder::MakeCopy::No);

    if (oldValue.IsValid())
        statement.BindId(3, oldValue);
    else
        statement.BindNull(3);

    if (newValue.IsValid())
        statement.BindId(4, newValue);
    else
        statement.BindNull(4);

    const DbResult result = statement.Step();
    statement.Reset();
    statement.ClearBindings();
    return result;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
