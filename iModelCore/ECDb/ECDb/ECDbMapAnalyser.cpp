/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbMapAnalyser.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbPolicyManager.h"
#include <set>
#include "ECSql/ECSqlPrepareContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=====================================SqlViewBuilder===================================

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool SqlViewBuilder::IsValid() const
    {
    if (m_name.empty())
        {
        BeAssert(false && "Must specify a view name");
        return false;
        }

    if (m_selectStatementList.empty())
        {
        BeAssert(false && "View must have atleast one select statement");
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8String SqlViewBuilder::ToString(SqlOption option, bool escape, bool useUnionAll) const
    {
    if (!IsValid())
        {
        BeAssert(false && "view specification is not valid");
        }

    NativeSqlBuilder sql;
    if (option == SqlOption::Drop || option == SqlOption::DropIfExists)
        {
        sql.Append("DROP VIEW ").AppendIf(option == SqlOption::DropIfExists, "IF EXISTS ").Append(GetName(), escape).Append(";");
        }
    else
        {
        sql.Append("CREATE VIEW ").AppendIf(option == SqlOption::CreateIfNotExist, "IF NOT EXISTS ").Append(GetName(), escape).AppendLine(" AS");

        if (!m_sqlComment.empty())
            sql.Append("--").AppendLine(m_sqlComment.c_str());

        for (auto& select : m_selectStatementList)
            {
            if (&select != &m_selectStatementList.front())
                sql.Append("UNION ").AppendIf(useUnionAll, "ALL").AppendEol();

            sql.AppendLine(select.ToString());
            }
        sql.Append(";");
        }

    return sql.ToString();
    }

//=====================================SqlTriggerBuilder::TriggerList====================

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::TriggerList::TriggerList()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder& SqlTriggerBuilder::TriggerList::Create(SqlTriggerBuilder::Type type, SqlTriggerBuilder::Condition condition, bool temprary)
    {
    m_list.push_back(SqlTriggerBuilder(type, condition, temprary));
    return m_list.back();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::TriggerList::List const& SqlTriggerBuilder::TriggerList::GetTriggers() const { return m_list; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void SqlTriggerBuilder::TriggerList::Delete(SqlTriggerBuilder const& trigger)
    {
    for (auto itor = m_list.begin(); itor != m_list.end(); ++itor)
        {
        if (&(*itor) == &trigger)
            {
            m_list.erase(itor);
            break;
            }
        }
    }

//=====================================SqlTriggerBuilder=================================
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::SqlTriggerBuilder(Type type, Condition condition, bool temprary)
    :m_type(type), m_condition(condition), m_temprory(temprary)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::SqlTriggerBuilder(SqlTriggerBuilder && rhs)
    : m_type(std::move(rhs.m_type)), m_condition(std::move(rhs.m_condition)), m_temprory(std::move(rhs.m_temprory)), m_name(std::move(rhs.m_name)),
    m_when(std::move(rhs.m_when)), m_body(std::move(rhs.m_body)), m_on(rhs.m_on), m_ofColumns(std::move(rhs.m_ofColumns))
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::SqlTriggerBuilder(SqlTriggerBuilder const& rhs)
    : m_type(rhs.m_type), m_condition(rhs.m_condition), m_temprory(rhs.m_temprory), m_name(rhs.m_name),
    m_when(rhs.m_when), m_body(rhs.m_body), m_on(rhs.m_on), m_ofColumns(rhs.m_ofColumns)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder& SqlTriggerBuilder::operator = (SqlTriggerBuilder&& rhs)
    {
    if (this != &rhs)
        {
        m_name = rhs.m_name;
        m_type = rhs.m_type;
        m_body = rhs.m_body;
        m_condition = rhs.m_condition;
        m_on = rhs.m_on;
        m_temprory = rhs.m_temprory;
        m_when = rhs.m_when;
        rhs.m_ofColumns = rhs.m_ofColumns;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder& SqlTriggerBuilder::operator = (SqlTriggerBuilder const& rhs)
    {
    if (this != &rhs)
        {
        m_name = std::move(rhs.m_name);
        m_type = std::move(rhs.m_type);
        m_body = std::move(rhs.m_body);
        m_condition = std::move(rhs.m_condition);
        m_on = std::move(rhs.m_on);
        m_temprory = std::move(rhs.m_temprory);
        m_when = std::move(rhs.m_when);
        m_ofColumns = std::move(rhs.m_ofColumns);
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
NativeSqlBuilder& SqlTriggerBuilder::GetNameBuilder() { return m_name; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
NativeSqlBuilder& SqlTriggerBuilder::GetWhenBuilder() { return m_when; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
NativeSqlBuilder& SqlTriggerBuilder::GetBodyBuilder() { return m_body; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
NativeSqlBuilder& SqlTriggerBuilder::GetOnBuilder() { return m_on; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8CP SqlTriggerBuilder::GetName() const { return m_name.ToString(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8CP SqlTriggerBuilder::GetWhen() const { return m_when.ToString(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8CP SqlTriggerBuilder::GetBody() const { return m_body.ToString(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8CP SqlTriggerBuilder::GetOn() const { return m_on.ToString(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool SqlTriggerBuilder::IsTemporary() const { return m_temprory; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool SqlTriggerBuilder::IsValid() const
    {
    if (m_name.IsEmpty())
        {
        BeAssert(false && "Must specify a trigger name");
        return false;
        }

    if (m_on.IsEmpty())
        {
        BeAssert(false && "Must specify a trigger ON Table/View");
        return false;
        }

    if (m_body.IsEmpty())
        {
        BeAssert(false && "Must specify a trigger body");
        return false;
        }

    if (m_type == Type::UpdateOf && m_ofColumns.empty())
        {
        BeAssert(false && "For UPDATE OF trigger must specify atleast one column");
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8String SqlTriggerBuilder::ToString(SqlOption option, bool escape) const
    {
    NativeSqlBuilder sql;
    if (option == SqlOption::Drop || option == SqlOption::DropIfExists)
        {
        BeAssert(!m_name.IsEmpty() && "At least trigger name must be specified when trying to delete it");
        sql.Append("DROP TRIGGER ").AppendIf(option == SqlOption::DropIfExists, "IF EXISTS ").Append(GetName(), escape).Append(";");
        }
    else
        {
        if (!IsValid())
            {
            BeAssert(false && "Trigger specification is not valid");
            return Utf8String();
            }

        sql.Append("CREATE TRIGGER ").AppendIf(IsTemporary(), "TEMP ").AppendIf(option == SqlOption::CreateIfNotExist, "IF NOT EXISTS ").Append(GetName(), escape).AppendEol();
        switch (m_condition)
            {
                case Condition::After:
                    sql.Append("AFTER "); break;
                case Condition::Before:
                    sql.Append("BEFORE "); break;
                case Condition::InsteadOf:
                    sql.Append("INSTEAD OF "); break;
            }

        switch (m_type)
            {
                case Type::Delete:
                    sql.Append("DELETE "); break;
                case Type::Insert:
                    sql.Append("INSERT "); break;
                case Type::Update:
                    sql.Append("UPDATE "); break;
                case Type::UpdateOf:
                    sql.Append("UPDATE OF ");
                    for (auto& column : m_ofColumns)
                        {
                        if (&column != &m_ofColumns.front())
                            sql.Append(", ");

                        sql.Append(column.c_str(), escape);
                        }
                    break;
            }

        sql.AppendEol();
        sql.Append("ON ").Append(GetOn(), escape).AppendEol();
        if (!m_when.IsEmpty())
            {
            sql.Append("  WHEN ").Append(GetWhen()).AppendEol();
            }

        sql.Append("BEGIN").AppendEol();
        sql.Append(GetBody());
        sql.Append("END;").AppendEol();;
        }

    return sql.ToString();
    }

//=====================================ECDbMapAnalyser::Class============================
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Class::Class(ClassMapCR classMap, ECDbMapAnalyser::Storage& storage, ECDbMapAnalyser::Class* parent)
    :m_classMap(classMap), m_storage(storage), m_inQueue(true), m_parent(parent)
    {
    m_name = GetClassMap().GetClass().GetSchema().GetNamespacePrefix() + "_" + GetClassMap().GetClass().GetName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8CP ECDbMapAnalyser::Class::GetSqlName() const { return  m_name.c_str(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ClassMapCR ECDbMapAnalyser::Class::GetClassMap() const { return m_classMap; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
std::map <ECDbMapAnalyser::Storage const*, std::set<ECDbMapAnalyser::Class const*>>& ECDbMapAnalyser::Class::GetPartitionsR() { return m_partitions; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool ECDbMapAnalyser::Class::InQueue() const { return m_inQueue; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void ECDbMapAnalyser::Class::Done() { m_inQueue = false; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
std::vector<ECDbMapAnalyser::Storage const*> ECDbMapAnalyser::Class::GetNoneVirtualStorages() const
    {
    std::vector<Storage const*> tmp;
    for (auto& key : m_partitions)
        {
        if (key.first->IsVirtual())
            continue;

        tmp.push_back(key.first);
        }

    return tmp;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool ECDbMapAnalyser::Class::RequireView() const
    {
    return GetNoneVirtualStorages().size() > 1 && !m_classMap.GetMapStrategy().IsNotMapped();
    }

//=====================================ECDbMapAnalyser::Relationship::EndInfo===========
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndInfo::EndInfo(Utf8CP accessString, DbColumn const& column)
    :m_accessString(accessString), m_column(&column)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndInfo::EndInfo(PropertyMapCR map)
    : m_accessString(map.GetPropertyAccessString())
    {
    auto firstColumn = map.GetSingleColumn();
    BeAssert(firstColumn != nullptr);
    m_column = firstColumn;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndInfo::EndInfo(PropertyMapCR map, Storage const& storage, DbColumn::Kind columnType)
    :m_accessString(map.GetPropertyAccessString())
    {
    auto firstColumn = storage.GetTable().GetFilteredColumnFirst(columnType);
    BeAssert(firstColumn != nullptr);
    m_column = firstColumn;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndInfo::EndInfo(EndInfo const&& rhs)
    :m_accessString(rhs.m_accessString), m_column(rhs.m_column)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndInfo::EndInfo()
    : m_accessString(nullptr), m_column(nullptr)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
DbColumn const& ECDbMapAnalyser::Relationship::EndInfo::GetColumn() const
    {
    BeAssert(m_column != nullptr); return *m_column;
    }

//=====================================ECDbMapAnalyser::Relationship::EndPoint===========
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndPoint::EndPoint(Relationship const& parent, EndType type)
    :m_ecid(nullptr), m_classId(nullptr), m_parent(parent)
    {
    auto const& map = parent.GetRelationshipClassMap();
    auto direction = map.GetRelationshipClass().GetStrengthDirection();
    if (direction == ECRelatedInstanceDirection::Forward)
        {
        if (type == EndType::From)
            {
            m_ecid = map.GetSourceECInstanceIdPropMap();
            m_classId = map.GetSourceECClassIdPropMap();
            }
        else
            {
            m_ecid = map.GetTargetECInstanceIdPropMap();
            m_classId = map.GetTargetECClassIdPropMap();
            }
        }
    else
        {
        if (type == EndType::From)
            {
            m_ecid = map.GetTargetECInstanceIdPropMap();
            m_classId = map.GetTargetECClassIdPropMap();
            }
        else
            {
            m_ecid = map.GetSourceECInstanceIdPropMap();
            m_classId = map.GetSourceECClassIdPropMap();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndInfo ECDbMapAnalyser::Relationship::EndPoint::GetResolvedInstanceId(Storage const& forStorage) const
    {
    if (m_parent.IsLinkTable())
        return EndInfo(*GetInstanceId());

    if (this == &(const_cast<Relationship&>(m_parent).ForeignEnd()))
        {
        return EndInfo(*GetInstanceId());
        }

    return EndInfo(*GetInstanceId(), forStorage, DbColumn::Kind::ECInstanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
std::set<ECDbMapAnalyser::Class*>& ECDbMapAnalyser::Relationship::EndPoint::GetClassesR() { return m_classes; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
std::set<ECDbMapAnalyser::Storage const*> ECDbMapAnalyser::Relationship::EndPoint::GetStorages() const
    {
    std::set<Storage const*> storages;
    for (auto cl : m_classes)
        {
        for (auto& s1 : cl->GetPartitionsR())
            if (!s1.first->IsVirtual())
                storages.insert(s1.first);
        }
    return storages;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
PropertyMapCP ECDbMapAnalyser::Relationship::EndPoint::GetInstanceId() const { return m_ecid; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECClassIdRelationshipConstraintPropertyMap const* ECDbMapAnalyser::Relationship::EndPoint::GetClassId() const { return m_classId; }

//=====================================ECDbMapAnalyser::Relationship=====================
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::Relationship(RelationshipClassMapCR classMap, Storage& storage, Class* parent)
    :Class(classMap, storage, parent), m_from(*this, EndType::From), m_to(*this, EndType::To), m_onDeleteAction(ForeignKeyDbConstraint::ActionType::NotSpecified), m_onUpdateAction(ForeignKeyDbConstraint::ActionType::NotSpecified)
    {
    ECDbForeignKeyRelationshipMap foreignKeyRelMap;
    if (ECDbMapCustomAttributeHelper::TryGetForeignKeyRelationshipMap(foreignKeyRelMap, GetRelationshipClassMap().GetRelationshipClass()))
        {
        Utf8String onDeleteActionStr;
        if (ECObjectsStatus::Success != foreignKeyRelMap.TryGetOnDeleteAction(onDeleteActionStr))
            return;

        Utf8String onUpdateActionStr;
        if (ECObjectsStatus::Success != foreignKeyRelMap.TryGetOnUpdateAction(onUpdateActionStr))
            return;

        m_onDeleteAction = ForeignKeyDbConstraint::ToActionType(onDeleteActionStr.c_str());
        m_onUpdateAction = ForeignKeyDbConstraint::ToActionType(onUpdateActionStr.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
RelationshipClassMapCR ECDbMapAnalyser::Relationship::GetRelationshipClassMap() const
    {
    return static_cast<RelationshipClassMapCR>(GetClassMap());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::PersistanceLocation ECDbMapAnalyser::Relationship::GetPersistanceLocation() const
    {
    if (IsLinkTable())
        return PersistanceLocation::Self;

    auto& endTable = static_cast<RelationshipClassEndTableMap const&>(GetRelationshipClassMap());
    if (endTable.GetForeignEnd() == ECN::ECRelationshipEnd::ECRelationshipEnd_Source)
        {
        if (endTable.GetRelationshipClass().GetStrengthDirection() == ECN::ECRelatedInstanceDirection::Forward)
            return PersistanceLocation::From;
        else
            return PersistanceLocation::To;
        }
    else
        {
        if (endTable.GetRelationshipClass().GetStrengthDirection() == ECN::ECRelatedInstanceDirection::Forward)
            return PersistanceLocation::To;
        else
            return PersistanceLocation::From;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool ECDbMapAnalyser::Relationship::RequireCascade() const
    {
    return GetRelationshipClassMap().GetRelationshipClass().GetStrength() != StrengthType::Referencing;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool ECDbMapAnalyser::Relationship::IsLinkTable() const
    {
    return GetRelationshipClassMap().GetType() == ClassMap::Type::RelationshipLinkTable;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndPoint& ECDbMapAnalyser::Relationship::From() { return m_from; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndPoint& ECDbMapAnalyser::Relationship::To() { return m_to; }

//=====================================ECDbMapAnalyser===================================
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Storage& ECDbMapAnalyser::GetStorage(Utf8CP tableName)
    {
    auto it = m_storage.find(tableName);
    if (it != m_storage.end())
        return *(it->second);

    DbTable const* table = m_map.GetDbSchema().FindTable(tableName);
    BeAssert(table != nullptr);
    std::unique_ptr<Storage> storagePtr = std::unique_ptr<Storage>(new Storage(*table));
    Storage* storage = storagePtr.get();
    m_storage[table->GetName().c_str()] = std::move(storagePtr);
    return *storage;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Class& ECDbMapAnalyser::GetClass(ClassMapCR classMap)
    {
    if (classMap.GetClass().GetRelationshipClassCP() != nullptr)
        return GetRelationship(static_cast<RelationshipClassMapCR>(classMap));

    const ECClassId classId = classMap.GetClass().GetId();
    auto it = m_classes.find(classId);
    if (it != m_classes.end())
        return *(it->second);

    Storage& storage = GetStorage(classMap);

    std::unique_ptr<Class> clsPtr = std::unique_ptr<Class>(new Class(classMap, storage, nullptr));
    Class* cls = clsPtr.get();
    m_classes[classId] = std::move(clsPtr);

    if (classMap.GetParentMapClassId().IsValid())
        cls->SetParent(GetClass(*GetClassMap(classMap.GetParentMapClassId())));

    storage.GetClassesR().insert(cls);
    if (classMap.HasJoinedTable())
        {
        Storage const& storage = GetStorage(classMap.GetJoinedTable().GetName().c_str());
        for (ECClassId id : classMap.GetStorageDescription().GetVerticalPartition(classMap.GetJoinedTable())->GetClassIds())
            {
            ClassMapCP refClassMap = GetClassMap(id);
            BeAssert(refClassMap != nullptr);
            Class& refclass = GetClass(*refClassMap);
            cls->GetPartitionsR()[&storage].insert(&refclass);
            }
        }
    else
        {
        for (Partition const& part : classMap.GetStorageDescription().GetHorizontalPartitions())
            {
            Storage const& storage = GetStorage(part.GetTable().GetName().c_str());
            for (ECClassId id : part.GetClassIds())
                {
                ClassMapCP refClassMap = GetClassMap(id);
                BeAssert(refClassMap != nullptr);
                Class& refclass = GetClass(*refClassMap);
                cls->GetPartitionsR()[&storage].insert(&refclass);
                }
            }
        }

    return *cls;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship& ECDbMapAnalyser::GetRelationship(RelationshipClassMapCR relClassMap)
    {
    const ECClassId relClassId = relClassMap.GetClass().GetId();
    auto it = m_relationships.find(relClassId);
    if (it != m_relationships.end())
        return *(it->second.get());

    Storage& storage = GetStorage(relClassMap);

    std::unique_ptr<Relationship> relPtr = std::unique_ptr<Relationship>(new Relationship(relClassMap, GetStorage(relClassMap), nullptr));
    Relationship* rel = relPtr.get();
    m_relationships[relClassId] = std::move(relPtr);

    if (relClassMap.GetParentMapClassId().IsValid())
        rel->SetParent(GetClass(*GetClassMap(relClassMap.GetParentMapClassId())));

    for (Partition const& part : relClassMap.GetStorageDescription().GetHorizontalPartitions())
        {
        Storage const& storage = GetStorage(part.GetTable().GetName().c_str());
        for (ECClassId id : part.GetClassIds())
            {
            rel->GetPartitionsR()[&storage].insert(&GetClass(*GetClassMap(id)));
            }
        }
    storage.GetRelationshipsR().insert(rel);

    const bool isForward = relClassMap.GetRelationshipClass().GetStrengthDirection() == ECRelatedInstanceDirection::Forward;
    bool hasRootOfJoinedTableSource = false;
    bool hasRootOfJoinedTableTarget = false;

    for (bpair<ECClassId, ECDbMap::LightweightCache::RelationshipEnd> const& key : m_map.GetLightweightCache().GetConstraintClassesForRelationshipClass(relClassId))
        {
        ClassMapCP constraintMap = GetClassMap(key.first);
        if (constraintMap->IsParentOfJoinedTable())
            {
            if (!hasRootOfJoinedTableSource && Enum::Contains(key.second, ECDbMap::LightweightCache::RelationshipEnd::Source))
                hasRootOfJoinedTableSource = true;

            if (!hasRootOfJoinedTableTarget && Enum::Contains(key.second, ECDbMap::LightweightCache::RelationshipEnd::Target))
                hasRootOfJoinedTableTarget = true;
            }
        }
    for (bpair<ECClassId, ECDbMap::LightweightCache::RelationshipEnd> const& key : m_map.GetLightweightCache().GetConstraintClassesForRelationshipClass(relClassId))
        {
        ClassMapCP constraintMap = GetClassMap(key.first);

        Class& constraitClass = GetClass(*constraintMap);
        if (Enum::Contains(key.second, ECDbMap::LightweightCache::RelationshipEnd::Source))
            {
            if (!(constraintMap->HasJoinedTable() && hasRootOfJoinedTableSource))
                {
                if (isForward)
                    rel->From().GetClassesR().insert(&constraitClass);
                else
                    rel->To().GetClassesR().insert(&constraitClass);
                }
            }
        if (Enum::Contains(key.second, ECDbMap::LightweightCache::RelationshipEnd::Target))
            {
            if (!(constraintMap->HasJoinedTable() && hasRootOfJoinedTableTarget))
                {
                if (!isForward)
                    rel->From().GetClassesR().insert(&constraitClass);
                else
                    rel->To().GetClassesR().insert(&constraitClass);
                }
            }
        }

    for (Storage const* from : rel->From().GetStorages())
        {
        for (Storage const* to : rel->To().GetStorages())
            {
            switch (rel->GetPersistanceLocation())
                {
                    case Relationship::PersistanceLocation::From:
                        const_cast<Storage *>(to)->CascadesTo()[const_cast<Storage*>(from)].insert(rel); break;
                    case Relationship::PersistanceLocation::To:
                        const_cast<Storage *>(from)->CascadesTo()[const_cast<Storage*>(to)].insert(rel); break;
                    case Relationship::PersistanceLocation::Self:
                        const_cast<Storage *>(from)->CascadesTo()[const_cast<Storage*>(to)].insert(rel); break;
                }
            }
        }

    return *rel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMapAnalyser::AnalyseClass(ClassMapCR ecClassMap)
    {
    Class& cls = GetClass(ecClassMap);
    if (!cls.InQueue())
        return SUCCESS;

    cls.Done(); //mark it as done
    for (ECClassId derivedClassId : GetDerivedClassIds(ecClassMap.GetClass().GetId()))
        {
        if (AnalyseClass(*GetClassMap(derivedClassId)) != SUCCESS)
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMapAnalyser::AnalyseRelationshipClass(RelationshipClassMapCR ecRelationshipClassMap)
    {
    Relationship& rel = GetRelationship(ecRelationshipClassMap);
    if (rel.InQueue())
        rel.Done();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void ECDbMapAnalyser::GetClassIds(std::vector<ECN::ECClassId>& rootClassIds, std::vector<ECN::ECClassId>& rootRelationshipClassIds) const
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_map.GetECDb(), "SELECT C.Id, C.Type FROM ec_Class C "
                                     "INNER JOIN ec_ClassMap M ON M.ClassId=C.Id "
                                     "LEFT JOIN ec_ClassHasBaseClasses B ON B.ClassId=C.Id "
                                     "WHERE B.BaseClassId IS NULL"))
        {
        BeAssert(false);
        return;
        }


    while (stmt.Step() == BE_SQLITE_ROW)
        {
        const ECClassId classId = stmt.GetValueId<ECClassId>(0);
        const ECClassType classType = Enum::FromInt<ECClassType>(stmt.GetValueInt(1));

        if (classType == ECClassType::Relationship)
            rootRelationshipClassIds.push_back(classId);
        else
            rootClassIds.push_back(classId);
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
std::set<ECN::ECClassId> const& ECDbMapAnalyser::GetDerivedClassIds(ECN::ECClassId baseClassId) const
    {
    return m_derivedClassLookup[baseClassId];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ClassMapCP ECDbMapAnalyser::GetClassMap(ECN::ECClassId classId) const
    {
    return m_map.GetClassMap(classId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void ECDbMapAnalyser::SetupDerivedClassLookup()
    {
    m_derivedClassLookup.clear();
    Statement stmt;
    stmt.Prepare(m_map.GetECDb(), "SELECT BaseClassId, ClassId FROM ec_ClassHasBaseClasses ORDER BY BaseClassId");
    while (stmt.Step() == BE_SQLITE_ROW)
        m_derivedClassLookup[stmt.GetValueId<ECClassId>(0)].insert(stmt.GetValueId<ECClassId>(1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::ViewInfo* ECDbMapAnalyser::GetViewInfoForClass(Class const& nclass)
    {
    auto itor = m_viewInfos.find(&nclass);
    if (itor == m_viewInfos.end())
        return nullptr;

    return itor->second.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
NativeSqlBuilder ECDbMapAnalyser::GetClassFilter(std::pair<ECDbMapAnalyser::Storage const*, std::set<ECDbMapAnalyser::Class const*>> const& partition)
    {
    auto storage = partition.first;
    auto& classes = partition.second;
    std::set<ECN::ECClassId> classIdSet;
    std::set<ECN::ECClassId> classIdSubset;
    std::set<ECN::ECClassId> classIdSubsetNotIn;
    NativeSqlBuilder sql;
    for (Class const* c : storage->GetClasses())
        classIdSet.insert(c->GetClassMap().GetClass().GetId());

    for (Class const* c : classes)
        {
        //WIP: Affan, is this assert needed. It fires when running the ATP JoinedTableECDbMapStrategyTests.MultiInheritence1
        //BeAssert(classIdSet.find(c->GetClassMap().GetClass().GetId()) != classIdSet.end());
        classIdSubset.insert(c->GetClassMap().GetClass().GetId());
        }

    for (ECClassId classId : classIdSet)
        {
        if (classIdSubset.find(classId) == classIdSubset.end())
            classIdSubsetNotIn.insert(classId);
        }


    if (classIdSubset.size() <= classIdSubsetNotIn.size() || classIdSubsetNotIn.empty())
        {
        sql.Append("IN (");
        for (ECClassId id : classIdSubset)
            {
            Utf8Char classIdStr[BeInt64Id::ID_STRINGBUFFER_LENGTH];
            id.ToString(classIdStr);
            sql.Append(classIdStr);
            if (id != *(classIdSubset.rbegin()))
                sql.Append(",");
            }
        sql.Append(")");
        }
    else
        {
        sql.Append(" NOT IN (");
        for (ECClassId id : classIdSubsetNotIn)
            {
            sql.Append(id.ToString().c_str());
            if (id != *(classIdSubsetNotIn.rbegin()))
                sql.Append(",");
            }
        sql.Append(")");
        }

    return sql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMapAnalyser::BuildPolymorphicDeleteTrigger(Class& nclass)
    {
    BeAssert(nclass.RequireView());
    auto viewInfo = GetViewInfoForClass(nclass);
    BeAssert(viewInfo != nullptr && !viewInfo->GetViewDdl().IsEmpty());
    auto p = nclass.GetStorage().GetTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
    auto c = nclass.GetStorage().GetTable().GetFilteredColumnFirst(DbColumn::Kind::ECClassId);
    for (auto & i : nclass.GetPartitionsR())
        {
        auto storage = i.first;
        if (storage->IsVirtual())
            continue;

        SqlTriggerBuilder& builder = viewInfo->GetTriggersR().Create(SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::InsteadOf, false);
        builder.GetOnBuilder().Append(viewInfo->GetViewDdl().GetName());
        builder.GetNameBuilder().Append(nclass.GetSqlName()).Append("_").Append(storage->GetTable().GetName().c_str()).Append("_Delete");
        NativeSqlBuilder classFilter = GetClassFilter(i);
        if (!classFilter.IsEmpty())
            {
            if (c == nullptr)
                builder.GetWhenBuilder().Append("OLD.").Append("ECClassId ").Append(classFilter);
            else
                builder.GetWhenBuilder().Append("OLD.").Append(c->GetName().c_str()).AppendSpace().Append(classFilter);
            }

        auto& body = builder.GetBodyBuilder();
        auto f = storage->GetTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
        body.Append("DELETE FROM ").AppendEscaped(storage->GetTable().GetName().c_str());
        body.AppendFormatted(" WHERE OLD.[%s] = [%s]", p->GetName().c_str(), f->GetName().c_str());
        body.Append(";").AppendEol();
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMapAnalyser::BuildPolymorphicUpdateTrigger(Class& nclass)
    {
    BeAssert(nclass.RequireView());
    BeAssert(nclass.RequireView());
    auto viewInfo = GetViewInfoForClass(nclass);
    BeAssert(viewInfo != nullptr && !viewInfo->GetViewDdl().IsEmpty());

    auto rootPMS = PropertyMapSet::Create(nclass.GetClassMap());
    auto rootEndPoints = rootPMS->GetEndPoints();
    auto p = nclass.GetStorage().GetTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
    auto c = nclass.GetStorage().GetTable().GetFilteredColumnFirst(DbColumn::Kind::ECClassId);

    for (auto & i : nclass.GetPartitionsR())
        {
        auto storage = i.first;
        if (storage->IsVirtual())
            continue;

        SqlTriggerBuilder& builder = viewInfo->GetTriggersR().Create(SqlTriggerBuilder::Type::Update, SqlTriggerBuilder::Condition::InsteadOf, false);
        builder.GetOnBuilder().Append(viewInfo->GetViewDdl().GetName());
        builder.GetNameBuilder().Append(nclass.GetSqlName()).Append("_").Append(storage->GetTable().GetName().c_str()).Append("_").Append("Update");
        NativeSqlBuilder classFilter = GetClassFilter(i);
        if (!classFilter.IsEmpty())
            {
            if (c == nullptr)
                builder.GetWhenBuilder().Append("OLD.").Append("ECClassId ").Append(classFilter);
            else
                builder.GetWhenBuilder().Append("OLD.").Append(c->GetName().c_str()).AppendSpace().Append(classFilter);
            }

        auto& body = builder.GetBodyBuilder();
        auto& firstClass = **(i.second.begin());
        auto f = storage->GetTable().GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
        auto childPMS = PropertyMapSet::Create(firstClass.GetClassMap());
        body.Append("UPDATE ").AppendEscaped(storage->GetTable().GetName().c_str());
        body.Append(" SET ");
        int nColumns = 0;

        bool isFirstSetExpr = true;
        for (PropertyMapSet::EndPoint const* rootE : rootEndPoints)
            {
            PropertyMapSet::EndPoint const* childE = childPMS->GetEndPointByAccessString(rootE->GetAccessString().c_str());
            DbColumn::Kind rootEColKind = rootE->GetColumnKind();
            if (rootEColKind != DbColumn::Kind::DataColumn && rootEColKind != DbColumn::Kind::SharedDataColumn)
                continue;

            if (!isFirstSetExpr)
                body.AppendComma();

            if (childE->GetValue().IsNull())
                {
                body.AppendFormatted("[%s] = NEW.[%s]", childE->GetColumn()->GetName().c_str(), rootE->GetColumn()->GetName().c_str());
                isFirstSetExpr = false;
                }

            nColumns++;
            }

        if (nColumns == 0)
            {
            viewInfo->GetTriggersR().Delete(builder);
            return BentleyStatus::SUCCESS;
            }

        body.AppendFormatted(" WHERE OLD.[%s] = [%s];", p->GetName().c_str(), f->GetName().c_str());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlViewBuilder ECDbMapAnalyser::BuildView(Class& nclass)
    {
    ClassMapCR classMap = nclass.GetClassMap();
    std::unique_ptr<PropertyMapSet> rootPMS = PropertyMapSet::Create(classMap);
    StorageDescription const& storageDescription = classMap.GetStorageDescription();

    Partition const* root = &storageDescription.GetRootHorizontalPartition();
    if (root->GetTable().GetPersistenceType() == PersistenceType::Virtual)
        root = nullptr;

    NativeSqlBuilder::List selectStatementList;

    bool bFirst = true;
    for (Partition const& hp : storageDescription.GetHorizontalPartitions())
        {
        if (hp.GetTable().GetPersistenceType() == PersistenceType::Virtual)
            continue;

        NativeSqlBuilder select;
        select.Append("SELECT ");
        ClassMapCP firstChildMap = m_map.GetClassMap(hp.GetClassIds().front());
        std::unique_ptr<PropertyMapSet> childPMS = PropertyMapSet::Create(*firstChildMap);
        auto rootEndPoints = rootPMS->GetEndPoints();
        for (PropertyMapSet::EndPoint const* rootE : rootEndPoints)
            {
            PropertyMapSet::EndPoint const* childE = childPMS->GetEndPointByAccessString(rootE->GetAccessString().c_str());
            if (childE->GetValue().IsNull())
                {
                select.AppendEscaped(childE->GetColumn()->GetName().c_str());
                if (bFirst && root == nullptr)
                    {
                    bFirst = false;
                    select.AppendSpace().AppendEscaped(rootE->GetColumn()->GetName().c_str());
                    }
                }
            else
                {
                if (rootE->GetColumn() != nullptr)
                    select.AppendFormatted("%" PRIu64 " [%s]", (uint64_t) childE->GetValue().GetLong(), rootE->GetColumn()->GetName().c_str());
                else
                    select.AppendFormatted("%" PRIu64 " [%s]", (uint64_t) childE->GetValue().GetLong(), (rootE->GetAccessString().c_str()));
                }

            if (rootE != rootEndPoints.back())
                select.Append(", ");
            }

        select.Append(" FROM ").AppendEscaped(firstChildMap->GetJoinedTable().GetName().c_str());
        DbColumn const* classIdColumn = nullptr;
        if (hp.GetTable().TryGetECClassIdColumn(classIdColumn))
            {
            if (classIdColumn->GetPersistenceType() == PersistenceType::Persisted && hp.NeedsECClassIdFilter())
                {
                Utf8String classIdFilter;
                hp.AppendECClassIdFilterSql(classIdFilter, classIdColumn->GetName().c_str());
                select.Append(" WHERE ").Append(classIdFilter.c_str());
                }
            }

        if (&hp == root)
            selectStatementList.insert(selectStatementList.begin(), std::move(select));
        else
            selectStatementList.push_back(std::move(select));
        }

    const bool isEmptyView = selectStatementList.empty();
    if (isEmptyView)
        {
        NativeSqlBuilder select("SELECT ");
        auto rootEndPoints = rootPMS->GetEndPoints();
        for (auto const rootE : rootEndPoints)
            {
            select.Append("NULL ");
            select.AppendEscaped(rootE->GetColumn()->GetName().c_str());
            if (rootE != rootEndPoints.back())
                select.Append(", ");
            }

        select.Append(" LIMIT 0");
        selectStatementList.push_back(std::move(select));
        }

    SqlViewBuilder builder(classMap.GetUpdatableViewName().c_str(), isEmptyView, selectStatementList);
    return builder;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
DbResult ECDbMapAnalyser::ExecuteDDL(Utf8CP sql) const
    {
    auto r = m_map.GetECDb().ExecuteSql(sql);
    if (r != BE_SQLITE_OK)
        {
        BeAssert(false && "Failed to run DDL statement");
        LOG.errorv("Failed to execute DDL statement during mapping - %s", sql);
        }
    return r;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
DbResult ECDbMapAnalyser::ApplyChanges() const
    {
    for (auto const& kvPair : m_viewInfos)
        {
        ViewInfo const& viewInfo = *kvPair.second;
        if (viewInfo.GetViewDdl().IsEmpty())
            {
            BeAssert(false && "must have a view");
            continue;
            }

        Utf8String sql = viewInfo.GetViewDdl().ToString(SqlOption::DropIfExists, true);
        DbResult stat = ExecuteDDL(sql.c_str());
        if (BE_SQLITE_OK != stat)
            return stat;

        sql = viewInfo.GetViewDdl().ToString(SqlOption::Create, true);
        stat = ExecuteDDL(sql.c_str());
        if (BE_SQLITE_OK != stat)
            return stat;

        for (SqlTriggerBuilder const& trigger : viewInfo.GetTriggers().GetTriggers())
            {
            if (!trigger.IsEmpty())
                {
                sql = trigger.ToString(SqlOption::DropIfExists, true);
                stat = ExecuteDDL(sql.c_str());
                if (BE_SQLITE_OK != stat)
                    return stat;

                sql = trigger.ToString(SqlOption::Create, true);
                stat = ExecuteDDL(sql.c_str());
                if (BE_SQLITE_OK != stat)
                    return stat;
                }
            }
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMapAnalyser::Analyse(bool applyChanges)
    {
    m_classes.clear();
    m_derivedClassLookup.clear();
    m_relationships.clear();
    m_storage.clear();
    m_viewInfos.clear();

    SetupDerivedClassLookup();

    std::vector<ECClassId> rootClassIds;
    std::vector<ECClassId> rootRelClassIds;
    GetClassIds(rootClassIds, rootRelClassIds);
    for (ECClassId rootClassId : rootClassIds)
        {
        ClassMapCP rootClassMap = GetClassMap(rootClassId);
        if (rootClassMap == nullptr)
            return ERROR;

        if (!rootClassMap->GetMapStrategy().IsNotMapped())
            if (AnalyseClass(*rootClassMap) != SUCCESS)
                return ERROR;
        }

    for (ECClassId rootRelClassId : rootRelClassIds)
        {
        ClassMapCP rootRelClassMap = GetClassMap(rootRelClassId);
        if (rootRelClassMap == nullptr)
            return ERROR;

        if (!rootRelClassMap->GetMapStrategy().IsNotMapped())
            if (AnalyseRelationshipClass(static_cast<RelationshipClassMapCR> (*rootRelClassMap)) != SUCCESS)
                return ERROR;
        }

    for (auto const& kvPair : m_classes)
        {
        Class& ecclass = *kvPair.second;
        if (!ecclass.RequireView())
            continue;

        SqlViewBuilder viewBuilder = BuildView(ecclass);
        m_viewInfos[&ecclass] = std::unique_ptr<ViewInfo>(new ViewInfo(viewBuilder));
        BuildPolymorphicDeleteTrigger(ecclass);
        BuildPolymorphicUpdateTrigger(ecclass);
        }

    if (applyChanges)
        return ApplyChanges() == BE_SQLITE_OK ? SUCCESS : ERROR;

    return SUCCESS;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE