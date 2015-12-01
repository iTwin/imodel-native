/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbMapAnalyser.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbPolicyManager.h"
#include <set>
#include "ECSql/ECSqlPrepareContext.h"
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
#define ECDB_HOLDING_VIEW "ec_RelationshipHoldingStatistics"


//=====================================SqlViewBuilder===================================
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlViewBuilder::SqlViewBuilder ()
:m_isTmp (false), m_isNullView (false)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void SqlViewBuilder::MarkAsNullView ()
    {
    m_isNullView = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool SqlViewBuilder::IsNullView () const { return m_isNullView; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
NativeSqlBuilder& SqlViewBuilder::GetNameBuilder ()  { return m_name; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void SqlViewBuilder::SetTemprory (bool tmp) { m_isTmp = tmp; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
NativeSqlBuilder& SqlViewBuilder::AddSelect ()
    {
    m_selectList.push_back (NativeSqlBuilder ());
    return m_selectList.back ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool SqlViewBuilder::IsEmpty () const
    {
    return m_selectList.empty () && m_name.IsEmpty ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool SqlViewBuilder::IsValid () const
    {
    if (m_name.IsEmpty ())
        {
        BeAssert (false && "Must specify a view name");
        return false;
        }

    if (m_selectList.empty ())
        {
        BeAssert (false && "View must have atleast one select statement");
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8CP SqlViewBuilder::GetName () const{ return m_name.ToString (); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool SqlViewBuilder::IsTemprory () const { return m_isTmp; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool SqlViewBuilder::IsCompound () const { return m_selectList.size () > 1; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
const Utf8String SqlViewBuilder::ToString (SqlOption option, bool escape , bool useUnionAll) const
    {
    if (!IsValid ())
        {
        BeAssert (false && "view specification is not valid");
        }

    NativeSqlBuilder sql;
    if (option == SqlOption::Drop || option == SqlOption::DropIfExists)
        {
        sql.Append ("DROP VIEW ").AppendIf (option == SqlOption::DropIfExists, "IF EXISTS ").AppendEscapedIf (escape, GetName ()).Append (";");
        }
    else
        {
        sql.AppendLine ("--### WARNING: SYSTEM GENERATED VIEW. DO NOT CHANGE THIS VIEW IN ANYWAY. ####");
        sql.Append ("CREATE ").AppendIf (IsTemprory (), "TEMP ").Append ("VIEW ").AppendIf (option == SqlOption::CreateIfNotExist, "IF NOT EXISTS ").AppendEscapedIf (escape, GetName ()).AppendEOL ();
        sql.Append ("AS").AppendEOL ();
        for (auto& select : m_selectList)
            {
            if (&select != &m_selectList.front ())
                sql.AppendTAB ().Append ("UNION ").AppendIf (useUnionAll, "ALL").AppendEOL ();

            sql.AppendTAB (2).AppendLine (select.ToString ());
            }
        sql.Append (";");
        }

    return sql.ToString ();
    }

//=====================================SqlTriggerBuilder::TriggerList====================
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::TriggerList::TriggerList (SqlTriggerBuilder::TriggerList::List const&& list)
:m_list (std::move (list))
    {
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::TriggerList::TriggerList ()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder& SqlTriggerBuilder::TriggerList::Create (SqlTriggerBuilder::Type type, SqlTriggerBuilder::Condition condition, bool temprary)
    {
    m_list.push_back (SqlTriggerBuilder (type, condition, temprary));
    return m_list.back ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::TriggerList::List const& SqlTriggerBuilder::TriggerList::GetTriggers () const { return m_list; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void SqlTriggerBuilder::TriggerList::Delete (SqlTriggerBuilder const& trigger)
    {
    for (auto itor = m_list.begin (); itor != m_list.end (); ++itor)
        {
        if (&(*itor) == &trigger)
            {
            m_list.erase (itor);
            break;
            }
        }
    }

//=====================================SqlTriggerBuilder=================================
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::SqlTriggerBuilder (Type type, Condition condition, bool temprary)
:m_type (type), m_condition (condition), m_temprory (temprary)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::SqlTriggerBuilder (SqlTriggerBuilder && rhs)
    : m_type (std::move (rhs.m_type)), m_condition (std::move (rhs.m_condition)), m_temprory (std::move (rhs.m_temprory)), m_name (std::move (rhs.m_name)),
    m_when (std::move (rhs.m_when)), m_body (std::move (rhs.m_body)), m_on (rhs.m_on), m_ofColumns (std::move (rhs.m_ofColumns))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::SqlTriggerBuilder (SqlTriggerBuilder const& rhs)
    : m_type (rhs.m_type), m_condition (rhs.m_condition), m_temprory (rhs.m_temprory), m_name (rhs.m_name),
    m_when (rhs.m_when), m_body (rhs.m_body), m_on (rhs.m_on), m_ofColumns (rhs.m_ofColumns)
    {
    }

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
        m_name = std::move (rhs.m_name);
        m_type = std::move (rhs.m_type);
        m_body = std::move (rhs.m_body);
        m_condition = std::move (rhs.m_condition);
        m_on = std::move (rhs.m_on);
        m_temprory = std::move (rhs.m_temprory);
        m_when = std::move (rhs.m_when);
        m_ofColumns = std::move (rhs.m_ofColumns);
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
NativeSqlBuilder& SqlTriggerBuilder::GetNameBuilder () { return m_name; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
NativeSqlBuilder& SqlTriggerBuilder::GetWhenBuilder () { return m_when; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
NativeSqlBuilder& SqlTriggerBuilder::GetBodyBuilder () { return m_body; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
NativeSqlBuilder& SqlTriggerBuilder::GetOnBuilder () { return m_on; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::Type SqlTriggerBuilder::GetType () const { return m_type; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::Condition SqlTriggerBuilder::GetCondition () const { return m_condition; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
std::vector<Utf8String> const& SqlTriggerBuilder::GetUpdateOfColumns () const
    {
    BeAssert (m_type == Type::UpdateOf);
    return m_ofColumns;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
std::vector<Utf8String>& SqlTriggerBuilder::GetUpdateOfColumnsR ()
    {
    BeAssert (m_type == Type::UpdateOf);
    return m_ofColumns;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8CP SqlTriggerBuilder::GetName () const { return m_name.ToString (); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8CP SqlTriggerBuilder::GetWhen () const { return m_when.ToString (); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8CP SqlTriggerBuilder::GetBody () const { return m_body.ToString (); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8CP SqlTriggerBuilder::GetOn () const { return m_on.ToString (); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool SqlTriggerBuilder::IsTemprory () const { return m_temprory; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool SqlTriggerBuilder::IsValid () const
    {
    if (m_name.IsEmpty ())
        {
        BeAssert (false && "Must specify a trigger name");
        return false;
        }

    if (m_on.IsEmpty ())
        {
        BeAssert (false && "Must specify a trigger ON Table/View");
        return false;
        }

    if (m_body.IsEmpty ())
        {
        BeAssert (false && "Must specify a trigger body");
        return false;
        }

    if (m_type == Type::UpdateOf && m_ofColumns.empty ())
        {
        BeAssert (false && "For UPDATE OF trigger must specify atleast one column");
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
const Utf8String SqlTriggerBuilder::ToString (SqlOption option, bool escape) const
    {
    if (!IsValid ())
        {
        BeAssert (false && "Trigger specification is not valid");
        }

    NativeSqlBuilder sql;
    if (option == SqlOption::Drop || option == SqlOption::DropIfExists)
        {
        sql.Append ("DROP TRIGGER ").AppendIf (option == SqlOption::DropIfExists, "IF EXISTS ").AppendEscapedIf (escape, GetName ()).Append (";");
        }
    else
        {
        sql.AppendLine ("\n--### WARNING: SYSTEM GENERATED TRIGGER. DO NOT CHANGE THIS TRIGGER IN ANYWAY. ####");
        sql.Append ("CREATE TRIGGER ").AppendIf (IsTemprory (), "TEMP ").AppendIf (option == SqlOption::CreateIfNotExist, "IF NOT EXISTS ").AppendEscapedIf (escape, GetName ()).AppendEOL ();
        switch (m_condition)
            {
            case Condition::After:
                sql.Append ("AFTER "); break;
            case Condition::Before:
                sql.Append ("BEFORE "); break;
            case Condition::InsteadOf:
                sql.Append ("INSTEAD OF "); break;
            }

        switch (m_type)
            {
            case Type::Delete:
                sql.Append ("DELETE "); break;
            case Type::Insert:
                sql.Append ("INSERT "); break;
            case Type::Update:
                sql.Append ("UPDATE "); break;
            case Type::UpdateOf:
                sql.Append ("UPDATE OF ");
                for (auto& column : m_ofColumns)
                    {
                    if (&column != &m_ofColumns.front ())
                        sql.Append (", ");

                    sql.AppendEscapedIf (escape, column.c_str ());
                    }
                break;
            }

        sql.AppendEOL ();
        sql.Append ("ON ").AppendEscapedIf (escape, GetOn ()).AppendEOL ();
        if (!m_when.IsEmpty ())
            {
            sql.Append ("\tWHEN ").Append (GetWhen ()).AppendEOL ();
            }

        sql.Append ("BEGIN").AppendEOL ();
        sql.Append (GetBody ());
        sql.Append ("END;").AppendEOL ();;
        }

    return sql.ToString ();
    }

//=====================================ECDbMapAnalyser::Class============================
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Class::Class (ClassMapCR classMap, ECDbMapAnalyser::Storage& storage, ECDbMapAnalyser::Class* parent)
    :m_classMap (classMap), m_storage (storage), m_inQueue (true), m_parent (parent)
    {
    m_name = GetClassMap ().GetClass ().GetSchema ().GetNamespacePrefix () + "_" + GetClassMap ().GetClass ().GetName ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8CP ECDbMapAnalyser::Class::GetSqlName () const { return  m_name.c_str (); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Storage& ECDbMapAnalyser::Class::GetStorageR () { return m_storage; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ClassMapCR ECDbMapAnalyser::Class::GetClassMap () const { return m_classMap; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Class* ECDbMapAnalyser::Class::GetParent () { return m_parent; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
std::map <ECDbMapAnalyser::Storage const*, std::set<ECDbMapAnalyser::Class const*>>& ECDbMapAnalyser::Class::GetPartitionsR () { return m_partitions; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool ECDbMapAnalyser::Class::InQueue () const { return m_inQueue; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void ECDbMapAnalyser::Class::Done () { m_inQueue = false; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
std::vector<ECDbMapAnalyser::Storage const*> ECDbMapAnalyser::Class::GetNoneVirtualStorages () const
    {
    std::vector<Storage const*> tmp;
    for (auto& key : m_partitions)
        {
        if (key.first->IsVirtual ())
            continue;

        tmp.push_back (key.first);
        }

    return std::move (tmp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool ECDbMapAnalyser::Class::IsAbstract () const
    {
    return GetNoneVirtualStorages ().empty ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool ECDbMapAnalyser::Class::RequireView () const
    {
    return GetNoneVirtualStorages ().size () > 1 && !m_classMap.GetMapStrategy ().IsNotMapped ();
    }

//=====================================ECDbMapAnalyser::Relationship::EndInfo===========
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndInfo::EndInfo(Utf8CP accessString, ECDbSqlColumn const& column)
    :m_accessString(accessString), m_column(&column)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndInfo::EndInfo(PropertyMapCR map)
    : m_accessString(map.GetPropertyAccessString())
    {
    auto firstColumn = map.GetFirstColumn();
    BeAssert(firstColumn != nullptr);
    m_column = firstColumn;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndInfo::EndInfo(PropertyMapCR map, Storage const& storage, ColumnKind columnType)
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
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndInfo::EndInfo()
    : m_accessString(nullptr), m_column(nullptr)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndInfo& ECDbMapAnalyser::Relationship::EndInfo::operator = (EndInfo const&& rhs)
    {
    if (this != &rhs)
        {
        m_accessString = std::move(rhs.m_accessString);
        m_column = std::move(rhs.m_column);
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
Utf8CP ECDbMapAnalyser::Relationship::EndInfo::GetAccessString() const 
    { 
    BeAssert(m_accessString != nullptr); return m_accessString; 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbSqlColumn const& ECDbMapAnalyser::Relationship::EndInfo::GetColumn() const
    {
    BeAssert(m_column != nullptr); return *m_column;
    }

//=====================================ECDbMapAnalyser::Relationship::EndPoint===========
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndPoint::EndPoint (Relationship const& parent, EndType type)
    :m_ecid (nullptr), m_classId (nullptr), m_type (type),m_parent(parent)
    {
    auto const& map = parent.GetRelationshipClassMap();
    auto direction = map.GetRelationshipClass ().GetStrengthDirection ();
    if (direction == ECRelatedInstanceDirection::Forward)
        {
        if (type == EndType::From)
            {
            m_ecid = map.GetSourceECInstanceIdPropMap ();
            m_classId = map.GetSourceECClassIdPropMap ();
            }
        else{
            m_ecid = map.GetTargetECInstanceIdPropMap ();
            m_classId = map.GetTargetECClassIdPropMap ();
            }
        }
    else
        {
        if (type == EndType::From)
            {
            m_ecid = map.GetTargetECInstanceIdPropMap ();
            m_classId = map.GetTargetECClassIdPropMap ();
            }
        else{
            m_ecid = map.GetSourceECInstanceIdPropMap ();
            m_classId = map.GetSourceECClassIdPropMap ();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool ECDbMapAnalyser::Relationship::EndPoint::Contains(Class const& constraintClass) const
    {
    return m_classes.find(const_cast<Class*>(&constraintClass)) != m_classes.end();
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

    return EndInfo(*GetInstanceId(), forStorage, ColumnKind::ECInstanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndInfo ECDbMapAnalyser::Relationship::EndPoint::GetResolvedClassId(Storage const& forStorage) const
    {
    if (m_parent.IsLinkTable())
        return EndInfo(*GetClassId());

    if (this == &(const_cast<Relationship&>(m_parent).ForeignEnd()))
        {
        return EndInfo(*GetClassId());
        }

    return EndInfo(*GetInstanceId(), forStorage, ColumnKind::ECClassId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
std::set<ECDbMapAnalyser::Class*>& ECDbMapAnalyser::Relationship::EndPoint::GetClassesR (){ return m_classes; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
std::set<ECDbMapAnalyser::Storage const*> ECDbMapAnalyser::Relationship::EndPoint::GetStorages () const
    {
    std::set<Storage const*> storages;
    for (auto cl : m_classes)
        {
        for (auto& s1 : cl->GetPartitionsR ())
            if (!s1.first->IsVirtual ())
                storages.insert (s1.first);
        }
    return std::move (storages);
    }

PropertyMapCP ECDbMapAnalyser::Relationship::EndPoint::GetInstanceId () const { return m_ecid; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
PropertyMapCP ECDbMapAnalyser::Relationship::EndPoint::GetClassId () const { return m_classId; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndType ECDbMapAnalyser::Relationship::EndPoint::GetEnd () const { return m_type; }

//=====================================ECDbMapAnalyser::Relationship=====================
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::Relationship (RelationshipClassMapCR classMap, Storage& storage, Class* parent)
    :Class (classMap, storage, parent), m_from (*this, EndType::From), m_to (*this, EndType::To), m_onDeleteAction (ForeignKeyActionType::NotSpecified), m_onUpdateAction (ForeignKeyActionType::NotSpecified)
    {
    ECDbForeignKeyRelationshipMap foreignKeyRelMap;
    if (ECDbMapCustomAttributeHelper::TryGetForeignKeyRelationshipMap (foreignKeyRelMap, GetRelationshipClassMap ().GetRelationshipClass ()))
        {
        Utf8String onDeleteActionStr;
        if (ECOBJECTS_STATUS_Success != foreignKeyRelMap.TryGetOnDeleteAction (onDeleteActionStr))
            return;

        Utf8String onUpdateActionStr;
        if (ECOBJECTS_STATUS_Success != foreignKeyRelMap.TryGetOnUpdateAction (onUpdateActionStr))
            return;

        m_onDeleteAction = ECDbSqlForeignKeyConstraint::ToActionType (onDeleteActionStr.c_str ());
        m_onUpdateAction = ECDbSqlForeignKeyConstraint::ToActionType (onUpdateActionStr.c_str ());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
RelationshipClassMapCR ECDbMapAnalyser::Relationship::GetRelationshipClassMap () const
    {
    return static_cast<RelationshipClassMapCR>(GetClassMap ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::PersistanceLocation ECDbMapAnalyser::Relationship::GetPersistanceLocation () const
    {
    if (IsLinkTable ())
        return PersistanceLocation::Self;

    auto& endTable = static_cast<RelationshipClassEndTableMapCR>(GetRelationshipClassMap ());
    if (endTable.GetThisEnd () == ECN::ECRelationshipEnd::ECRelationshipEnd_Source)
        {
        if (endTable.GetRelationshipClass ().GetStrengthDirection () == ECN::ECRelatedInstanceDirection::Forward)
            return PersistanceLocation::From;
        else
            return PersistanceLocation::To;
        }
    else
        {
        if (endTable.GetRelationshipClass ().GetStrengthDirection () == ECN::ECRelatedInstanceDirection::Forward)
            return PersistanceLocation::To;
        else
            return PersistanceLocation::From;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool ECDbMapAnalyser::Relationship::RequireCascade () const
    {
    return GetRelationshipClassMap ().GetRelationshipClass ().GetStrength () != StrengthType::STRENGTHTYPE_Referencing;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool ECDbMapAnalyser::Relationship::IsLinkTable () const
    {
    return GetRelationshipClassMap ().GetClassMapType () == IClassMap::Type::RelationshipLinkTable;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndPoint& ECDbMapAnalyser::Relationship::From () { return m_from; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship::EndPoint& ECDbMapAnalyser::Relationship::To () { return m_to; }

//=====================================ECDbMapAnalyser::Storage==========================
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Storage::Storage (ECDbSqlTable const& table)
    :m_table (table)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbSqlTable const& ECDbMapAnalyser::Storage::GetTable () const { return m_table; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
bool ECDbMapAnalyser::Storage::IsVirtual () const { return m_table.GetPersistenceType () == PersistenceType::Virtual; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
std::set<ECDbMapAnalyser::Class*> & ECDbMapAnalyser::Storage::GetClassesR ()  { return m_classes; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
std::set<ECDbMapAnalyser::Relationship*> & ECDbMapAnalyser::Storage::GetRelationshipsR ()  { return m_relationships; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
std::map<ECDbMapAnalyser::Storage*, std::set<ECDbMapAnalyser::Relationship*>> & ECDbMapAnalyser::Storage::CascadesTo ()  { return m_cascades; } //OnDelete_tableA

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------

SqlTriggerBuilder::TriggerList& ECDbMapAnalyser::Storage::GetTriggerListR () { return m_triggers; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlTriggerBuilder::TriggerList const& ECDbMapAnalyser::Storage::GetTriggerList () const
    {
    return m_triggers;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void ECDbMapAnalyser::Storage::HandleStructArray ()
    {
    if (m_structCascades.empty ())
        return;

    auto& builder = GetTriggerListR ().Create (SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::After, false);
    builder.GetNameBuilder ()
        .Append (GetTable ().GetName ().c_str ())
        .Append ("_")
        .Append ("StructArray_Delete");

    builder.GetOnBuilder ().Append (GetTable ().GetName ().c_str ());
    auto& body = builder.GetBodyBuilder ();
    auto ecInstanceid = GetTable ().GetFilteredColumnFirst (ColumnKind::ECInstanceId);
    BeAssert (ecInstanceid != nullptr);

    for (auto structClass : m_structCascades)
        {
        for (auto& i : structClass->GetPartitionsR ())
            {
            auto toStorage = i.first;
            auto parentECInstanceId = toStorage->GetTable ().GetFilteredColumnFirst (ColumnKind::ParentECInstanceId);
            BeAssert (parentECInstanceId != nullptr);
            body
                .Append ("DELETE FROM ")
                .AppendEscaped (toStorage->GetTable ().GetName ().c_str ())
                .Append (" WHERE ")
                .AppendFormatted ("OLD.[%s] = [%s] ", ecInstanceid->GetName ().c_str (), parentECInstanceId->GetName ().c_str ());

            body.Append (";").AppendEOL ();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void ECDbMapAnalyser::Storage::HandleCascadeLinkTable (std::vector<ECDbMapAnalyser::Relationship*> const& relationships)
    {
    for (auto relationship : relationships)
        {
        if (!relationship->RequireCascade ())
            continue;

        auto& builder = GetTriggerListR ().Create (SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::After, false);
        builder.GetNameBuilder ()
            .Append (GetTable ().GetName ().c_str ())
            .Append ("_")
            .Append (relationship->GetSqlName ())
            .Append ("_Cascade");

        builder.GetOnBuilder ().Append (GetTable ().GetName ().c_str ());
        auto& body = builder.GetBodyBuilder ();
        body.Append ("--3 ").Append (relationship->GetRelationshipClassMap ().GetRelationshipClass ().GetFullName ()).AppendEOL ();
        for (auto storage : relationship->To ().GetStorages ())
            {
            body
                .Append ("DELETE FROM ")
                .AppendEscaped (storage->GetTable ().GetName ().c_str ())
                .Append (" WHERE ");

            auto otherEndPrimaryKey = storage->GetTable ().GetFilteredColumnFirst (ColumnKind::ECInstanceId);
            body.AppendFormatted ("(OLD.[%s] = [%s])", relationship->To ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str (), otherEndPrimaryKey->GetName ().c_str ());
            if (relationship->IsHolding ())
                {
                body.AppendFormatted (" AND (SELECT COUNT (*) FROM " ECDB_HOLDING_VIEW "  WHERE ECInstanceId = OLD.[%s]) = 0", relationship->To ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str());
                }
            body.Append (";").AppendEOL ();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void ECDbMapAnalyser::Storage::Generate ()
    {
    std::vector<Relationship*> linkTables;
    for (auto relationship : GetRelationshipsR ())
        {
        if (relationship->IsLinkTable ())
            {
            linkTables.push_back (relationship);
            }
        }

    HandleCascadeLinkTable (linkTables);
    HandleStructArray ();
    }

//=====================================ECDbMapAnalyser===================================
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Storage& ECDbMapAnalyser::GetStorage (Utf8CP tableName)
    {
    auto itor = m_storage.find (tableName);
    if (itor != m_storage.end ())
        return *(itor->second);

    auto table = m_map.GetSQLManager ().GetDbSchema ().FindTable (tableName);
    BeAssert (table != nullptr);
    auto ptr = Storage::Ptr (new Storage (*table));
    auto p = ptr.get ();
    m_storage[table->GetName ().c_str ()] = std::move (ptr);
    return *p;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Storage& ECDbMapAnalyser::GetStorage (ClassMapCR classMap)
    {
    return GetStorage (classMap.GetTable ().GetName ().c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Class& ECDbMapAnalyser::GetClass (ClassMapCR classMap)
    {
    if (classMap.GetClass ().GetRelationshipClassCP () != nullptr)
        {
        return GetRelationship (static_cast<RelationshipClassMapCR>(classMap));
        }

    auto itor = m_classes.find (classMap.GetClass ().GetId ());
    if (itor != m_classes.end ())
        return *(itor->second);

    Storage& storage = GetStorage (classMap);
    if (classMap.GetClass ().GetIsStruct ())
        m_classes[classMap.GetClass ().GetId ()] = Struct::Ptr (new Struct (classMap, storage, nullptr));
    else
        m_classes[classMap.GetClass ().GetId ()] = Class::Ptr (new Class (classMap, storage, nullptr));

    auto ptr = m_classes[classMap.GetClass ().GetId ()].get ();
    if (classMap.GetParentMapClassId () != 0LL)
        {
        ptr->SetParent (GetClass (*GetClassMap (classMap.GetParentMapClassId ())));
        }

    storage.GetClassesR ().insert (ptr);
    if (classMap.IsJoinedTable())
        {
        auto& storage = GetStorage(classMap.GetTable().GetName().c_str());
        for (auto id : classMap.GetStorageDescription().GetVerticalPartition(classMap.GetTable())->GetClassIds())
            {
            auto refClassMap = GetClassMap(id);
            BeAssert(refClassMap != nullptr);
            auto classM = &(GetClass(*refClassMap));
            BeAssert(classM != nullptr);
            ptr->GetPartitionsR()[&storage].insert(classM);
            }
        }
    else
        {
        for (auto& part : classMap.GetStorageDescription().GetHorizontalPartitions())
            {
            auto& storage = GetStorage(part.GetTable().GetName().c_str());
            for (auto id : part.GetClassIds())
                {
                auto refClassMap = GetClassMap(id);
                BeAssert(refClassMap != nullptr);
                auto classM = &(GetClass(*refClassMap));
                BeAssert(classM != nullptr);
                ptr->GetPartitionsR()[&storage].insert(classM);
                }
            }
        }
    return *ptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::Relationship&  ECDbMapAnalyser::GetRelationship (RelationshipClassMapCR classMap)
    {
    auto itor = m_relationships.find (classMap.GetClass ().GetId ());
    if (itor != m_relationships.end ())
        return *(itor->second.get ());

    Storage& storage = GetStorage (classMap);
    m_relationships[classMap.GetClass ().GetId ()] = Relationship::Ptr (new Relationship (classMap, GetStorage (classMap), nullptr));
    auto ptr = m_relationships[classMap.GetClass ().GetId ()].get ();
    if (classMap.GetParentMapClassId () != 0LL)
        {
        ptr->SetParent (GetClass (*GetClassMap (classMap.GetParentMapClassId ())));
        }


    for (auto& part : classMap.GetStorageDescription ().GetHorizontalPartitions ())
        {
        auto& storage = GetStorage (part.GetTable ().GetName ().c_str ());
        for (auto id : part.GetClassIds ())
            {
            ptr->GetPartitionsR ()[&storage].insert (&GetClass (*GetClassMap (id)));
            }
        }
    storage.GetRelationshipsR ().insert (ptr);
    auto isForward = classMap.GetRelationshipClass ().GetStrengthDirection () == ECRelatedInstanceDirection::Forward;
    bool hasRootOfJoinedTable = false;
    for (auto& key : m_map.GetLightweightCache().GetRelationships(classMap.GetClass().GetId()))
        {
        auto constraintMap = GetClassMap(key.first);
        if (constraintMap->IsParentOfJoinedTable())
            {
            hasRootOfJoinedTable = true;
            break;
            }
        }
    for (auto& key : m_map.GetLightweightCache ().GetRelationships (classMap.GetClass ().GetId ()))
        {
        auto constraintMap = GetClassMap(key.first);
        if (constraintMap->IsJoinedTable() && hasRootOfJoinedTable)
            {
            continue;
            }

        auto& constraitClass = GetClass (*constraintMap);
        if (Enum::Contains(key.second, ECDbMap::LightweightCache::RelationshipEnd::Source))
            {
            if (isForward)
                ptr->From ().GetClassesR ().insert (&constraitClass);
            else
                ptr->To ().GetClassesR ().insert (&constraitClass);
            }
        if (Enum::Contains(key.second, ECDbMap::LightweightCache::RelationshipEnd::Target))
            {
            if (!isForward)
                ptr->From ().GetClassesR ().insert (&constraitClass);
            else
                ptr->To ().GetClassesR ().insert (&constraitClass);
            }
        }

    for (auto from : ptr->From ().GetStorages ())
        {
        for (auto to : ptr->To ().GetStorages ())
            {
            switch (ptr->GetPersistanceLocation ())
                {
                case Relationship::PersistanceLocation::From:
                    const_cast<Storage *>(to)->CascadesTo ()[const_cast<Storage*>(from)].insert (ptr); break;
                case Relationship::PersistanceLocation::To:
                    const_cast<Storage *>(from)->CascadesTo ()[const_cast<Storage*>(to)].insert (ptr); break;
                case Relationship::PersistanceLocation::Self:
                    const_cast<Storage *>(from)->CascadesTo ()[const_cast<Storage*>(to)].insert (ptr); break;
                }
            }
        }
    return *ptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMapAnalyser::AnalyseClass (ClassMapCR ecClassMap)
    {
    auto& ptr = GetClass (ecClassMap);
    if (!ptr.InQueue ())
        {
        return BentleyStatus::SUCCESS;
        }

    AnalyseStruct (ptr);
    ptr.Done (); //mark it as done
    for (auto derivedClassId : GetDerivedClassIds (ecClassMap.GetClass ().GetId ()))
        {
        if (AnalyseClass (*GetClassMap (derivedClassId)) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        }
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void ECDbMapAnalyser::AnalyseStruct (Class& classInfo)
    {
    std::map<IClassMap const*, std::vector<PropertyMapToTableCP>> structPropertyMaps;
    classInfo.GetClassMap ().GetPropertyMaps ().Traverse ([&] (TraversalFeedback& feedback, PropertyMapCP propertyMap)
        {
        if (auto mapToTable = dynamic_cast<PropertyMapToTableCP>(propertyMap))
            {
            if (auto associatedClasMap = m_map.GetClassMap (mapToTable->GetElementType ()))
                {
                if (associatedClasMap->GetTable ().GetPersistenceType () == PersistenceType::Persisted)
                    {
                    structPropertyMaps[associatedClasMap].push_back (mapToTable);
                    }
                }
            }
        feedback = TraversalFeedback::Next;
        }, true);

    for (auto& key : structPropertyMaps)
        {
        auto& elementType = static_cast<Struct&>(GetClass (static_cast<ClassMap const&>(*key.first)));
        classInfo.GetStorageR ().StructCascadeTo ().insert (&elementType);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMapAnalyser::AnalyseRelationshipClass (RelationshipClassMapCR ecRelationshipClassMap)
    {
    auto& ptr = GetRelationship (ecRelationshipClassMap);
    if (!ptr.InQueue ())
        return BentleyStatus::SUCCESS;


    ptr.Done ();
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
const std::vector<ECN::ECClassId> ECDbMapAnalyser::GetRootClassIds () const
    {
    Utf8CP sql0 =
        "SELECT C.Id"
        "	FROM ec_Class C "
        "   	INNER JOIN ec_ClassMap M ON M.ClassId = C.Id "
        "       LEFT JOIN ec_BaseClass B ON B.ClassId = C.Id "
        "	WHERE B.BaseClassId IS NULL And C.IsRelationship = 0";

    std::vector<ECN::ECClassId> classIds;
    Statement stmt;
    stmt.Prepare (GetMap ().GetECDbR (), sql0);
    while (stmt.Step () == BE_SQLITE_ROW)
        classIds.push_back (stmt.GetValueInt64 (0));

    return std::move (classIds);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
const std::vector<ECN::ECClassId> ECDbMapAnalyser::GetRelationshipClassIds () const
    {
    Utf8CP sql0 =
        "SELECT C.Id"
        "	FROM ec_Class C "
        "   	INNER JOIN ec_ClassMap M ON M.ClassId = C.Id "
        "       LEFT JOIN ec_BaseClass B ON B.ClassId = C.Id "
        "	WHERE B.BaseClassId IS NULL And C.IsRelationship = 1";

    std::vector<ECN::ECClassId> classIds;
    Statement stmt;
    stmt.Prepare (GetMap ().GetECDbR (), sql0);
    while (stmt.Step () == BE_SQLITE_ROW)
        classIds.push_back (stmt.GetValueInt64 (0));

    return std::move (classIds);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
std::set<ECN::ECClassId> const& ECDbMapAnalyser::GetDerivedClassIds (ECN::ECClassId baseClassId) const
    {
    return m_derivedClassLookup[baseClassId];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ClassMapCP ECDbMapAnalyser::GetClassMap (ECN::ECClassId classId) const
    {
    auto classMap = GetMap ().GetClassMapCP (classId);
    BeAssert (classMap != nullptr && "ClassMap not found");
    return classMap;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void ECDbMapAnalyser::SetupDerivedClassLookup ()
    {
    m_derivedClassLookup.clear ();
    Utf8CP sql0 =
        "SELECT  BaseClassId, ClassId FROM ec_BaseClass ORDER BY BaseClassId";

    Statement stmt;
    m_derivedClassLookup.clear ();
    stmt.Prepare (GetMap ().GetECDbR (), sql0);
    while (stmt.Step () == BE_SQLITE_ROW)
        m_derivedClassLookup[stmt.GetValueInt64 (0)].insert (stmt.GetValueInt64 (1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::ECDbMapAnalyser (ECDbMapR ecdbMap)
    :m_map (ecdbMap)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
ECDbMapAnalyser::ViewInfo* ECDbMapAnalyser::GetViewInfoForClass (Class const& nclass)
    {
    auto itor = m_viewInfos.find (&nclass);
    if (itor == m_viewInfos.end ())
        return nullptr;

    return &(itor->second);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
const NativeSqlBuilder ECDbMapAnalyser::GetClassFilter (std::pair<ECDbMapAnalyser::Storage const*, std::set<ECDbMapAnalyser::Class const*>> const& partition)
    {
    auto storage = partition.first;
    auto& classes = partition.second;
    std::set<ECN::ECClassId> classSet;
    std::set<ECN::ECClassId> classSubset;
    std::set<ECN::ECClassId> classSubsetNotIn;
    NativeSqlBuilder sql;
    for (auto c : storage->GetClasses ())
        classSet.insert (c->GetClassMap ().GetClass ().GetId ());

    for (auto c : classes)
        {
        BeAssert (classSet.find (c->GetClassMap ().GetClass ().GetId ()) != classSet.end ());
        classSubset.insert (c->GetClassMap ().GetClass ().GetId ());
        }

    for (auto c : classSet)
        {
        if (classSubset.find (c) == classSubset.end ())
            classSubsetNotIn.insert (c);
        }


    if (classSubset.size () <= classSubsetNotIn.size () || classSubsetNotIn.empty ())
        {
        sql.Append ("IN (");
        for (auto id : classSubset)
            {
            sql.Append (id);
            if (id != *(classSubset.rbegin ()))
                sql.Append (",");
            }
        sql.Append (")");
        }
    else
        {
        sql.Append (" NOT IN (");
        for (auto id : classSubsetNotIn)
            {
            sql.Append (id);
            if (id != *(classSubsetNotIn.rbegin ()))
                sql.Append (",");
            }
        sql.Append (")");
        }

    return std::move (sql);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMapAnalyser::BuildPolymorphicDeleteTrigger (Class& nclass)
    {
    BeAssert (nclass.RequireView ());
    auto viewInfo = GetViewInfoForClass (nclass);
    BeAssert (viewInfo != nullptr && !viewInfo->GetView ().IsEmpty ());
    auto p = nclass.GetStorage ().GetTable ().GetFilteredColumnFirst (ColumnKind::ECInstanceId);
    auto c = nclass.GetStorage ().GetTable ().GetFilteredColumnFirst (ColumnKind::ECClassId);
    for (auto & i : nclass.GetPartitionsR ())
        {
        auto storage = i.first;
        if (storage->IsVirtual ())
            continue;

        SqlTriggerBuilder& builder = viewInfo->GetTriggersR ().Create (SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::InsteadOf, false);
        builder.GetOnBuilder ().Append (viewInfo->GetViewR ().GetName ());
        builder.GetNameBuilder ().Append (nclass.GetSqlName ()).Append ("_").Append (storage->GetTable ().GetName ().c_str ()).Append ("_Delete");
        NativeSqlBuilder classFilter = GetClassFilter (i);
        if (!classFilter.IsEmpty ())
            {
            if (c == nullptr)
                builder.GetWhenBuilder ().Append ("OLD.").Append ("ECClassId ").Append (classFilter);
            else
                builder.GetWhenBuilder ().Append ("OLD.").Append (c->GetName ().c_str ()).AppendSpace ().Append (classFilter);
            }

        auto& body = builder.GetBodyBuilder ();
        auto f = storage->GetTable ().GetFilteredColumnFirst (ColumnKind::ECInstanceId);
        body.Append ("DELETE FROM ").AppendEscaped (storage->GetTable ().GetName ().c_str ());
        body.AppendFormatted (" WHERE OLD.[%s] = [%s]", p->GetName ().c_str (), f->GetName ().c_str ());
        body.Append (";").AppendEOL ();
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMapAnalyser::BuildPolymorphicUpdateTrigger (Class& nclass)
    {
    BeAssert (nclass.RequireView ());
    BeAssert (nclass.RequireView ());
    auto viewInfo = GetViewInfoForClass (nclass);
    BeAssert (viewInfo != nullptr && !viewInfo->GetView ().IsEmpty ());

    auto rootPMS = PropertyMapSet::Create (nclass.GetClassMap ());
    auto rootEndPoints = rootPMS->GetEndPoints ();
    auto p = nclass.GetStorage ().GetTable ().GetFilteredColumnFirst (ColumnKind::ECInstanceId);
    auto c = nclass.GetStorage ().GetTable ().GetFilteredColumnFirst (ColumnKind::ECClassId);

    for (auto & i : nclass.GetPartitionsR ())
        {
        auto storage = i.first;
        if (storage->IsVirtual ())
            continue;

        SqlTriggerBuilder& builder = viewInfo->GetTriggersR ().Create (SqlTriggerBuilder::Type::Update, SqlTriggerBuilder::Condition::InsteadOf, false);
        builder.GetOnBuilder ().Append (viewInfo->GetViewR ().GetName ());
        builder.GetNameBuilder ().Append(nclass.GetSqlName()).Append("_").Append (storage->GetTable().GetName().c_str()).Append ("_").Append ("Update");
        NativeSqlBuilder classFilter = GetClassFilter (i);
        if (!classFilter.IsEmpty ())
            {
            if (c == nullptr)
                builder.GetWhenBuilder ().Append ("OLD.").Append ("ECClassId ").Append (classFilter);
            else
                builder.GetWhenBuilder ().Append ("OLD.").Append (c->GetName ().c_str ()).AppendSpace ().Append (classFilter);
            }

        auto& body = builder.GetBodyBuilder ();
        auto& firstClass = **(i.second.begin ());
        auto f = storage->GetTable ().GetFilteredColumnFirst (ColumnKind::ECInstanceId);
        auto childPMS = PropertyMapSet::Create (firstClass.GetClassMap ());
        body.Append ("UPDATE ").AppendEscaped (storage->GetTable ().GetName ().c_str ());
        body.Append ("SET ");
        int nColumns = 0;

        for (auto const rootE : rootEndPoints)
            {
            auto childE = childPMS->GetEndPointByAccessString (rootE->GetAccessString ().c_str ());
            if (rootE->GetColumnKind () != ColumnKind::DataColumn)
                continue;

            if (childE->GetValue ().IsNull ())
                {
                body.AppendFormatted ("[%s] = NEW.[%s]", childE->GetColumn ()->GetName ().c_str (), rootE->GetColumn ()->GetName ().c_str ());
                }

            if (rootE != rootEndPoints.back ())
                body.Append (", ");

            nColumns++;
            }

        if (nColumns == 0)
            {
            viewInfo->GetTriggersR ().Delete (builder);
            return BentleyStatus::SUCCESS;
            }

        body.AppendFormatted (" WHERE OLD.[%s] = [%s]", p->GetName ().c_str (), f->GetName ().c_str ());
        body.Append (";").AppendEOL ();
        }
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
SqlViewBuilder ECDbMapAnalyser::BuildView (Class& nclass)
    {
    //BeAssert (nclass.GetType ().)
    auto classMap = &nclass.GetClassMap ();
    auto rootPMS = PropertyMapSet::Create (*classMap);
    auto const& storageDescription = classMap->GetStorageDescription ();

    SqlViewBuilder builder;
    builder.GetNameBuilder ()
        .Append ("_")
        .Append (classMap->GetClass ().GetSchema ().GetNamespacePrefix ().c_str ())
        .Append ("_")
        .Append (classMap->GetClass ().GetName ().c_str ());

    NativeSqlBuilder::List selects;
    Partition const* root = &storageDescription.GetRootHorizontalPartition ();
    if (root->GetTable ().GetPersistenceType () == PersistenceType::Virtual)
        root = nullptr;

    bool bFirst = true;
    for (auto const& hp : storageDescription.GetHorizontalPartitions ())
        {
        if (hp.GetTable ().GetPersistenceType () == PersistenceType::Virtual)
            continue;

        NativeSqlBuilder select;
        select.Append ("SELECT ");
        auto firstChildMap = m_map.GetClassMapCP (hp.GetClassIds ().front ());
        auto childPMS = PropertyMapSet::Create (*firstChildMap);
        auto rootEndPoints = rootPMS->GetEndPoints ();
        for (auto const rootE : rootEndPoints)
            {
            auto childE = childPMS->GetEndPointByAccessString (rootE->GetAccessString ().c_str ());
            if (childE->GetValue ().IsNull ())
                {
                select.AppendEscaped (childE->GetColumn ()->GetName ().c_str ());
                if (bFirst && root == nullptr)
                    {
                    bFirst = false;
                    select.AppendSpace ().AppendEscaped (rootE->GetColumn ()->GetName ().c_str ());
                    }
                }
            else
                {
                if (rootE->GetColumn () != nullptr)
                    select.Append (Utf8PrintfString ("%lld [%s]", childE->GetValue ().GetLong (), rootE->GetColumn ()->GetName ().c_str ()));
                else
                    select.Append (Utf8PrintfString ("%lld [%s]", childE->GetValue ().GetLong (), (rootE->GetAccessString ().c_str ())));
                }

            if (rootE != rootEndPoints.back ())
                select.Append (", ");
            }

        select.Append (" FROM ").AppendEscaped (firstChildMap->GetTable ().GetName ().c_str ());
        ECDbSqlColumn const* classIdColumn = nullptr;
        if (hp.GetTable ().TryGetECClassIdColumn(classIdColumn))
            {
            if (classIdColumn->GetPersistenceType () == PersistenceType::Persisted && hp.NeedsECClassIdFilter ())
                {
                select.Append (" WHERE ");
                hp.AppendECClassIdFilterSql (classIdColumn->GetName().c_str(), select);
                }
            }

        if (&hp == root)
            selects.insert (selects.begin (), std::move (select));
        else
            selects.push_back (std::move (select));
        }

    if (!selects.empty ())
        {
        for (auto const& select : selects)
            {
            builder.AddSelect ().Append (select);
            }
        }
    else
        {
        auto& select = builder.AddSelect ();
        select.Append ("SELECT ");
        auto rootEndPoints = rootPMS->GetEndPoints ();
        for (auto const rootE : rootEndPoints)
            {
            select.Append ("NULL ");
            select.AppendEscaped (rootE->GetColumn ()->GetName ().c_str ());
            if (rootE != rootEndPoints.back ())
                select.Append (", ");
            }

        select.Append (" LIMIT 0");
        builder.MarkAsNullView ();
        }

    return std::move (builder);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
DbResult ECDbMapAnalyser::ExecuteDDL (Utf8CP sql)
    {
    auto r = m_map.GetECDbR ().ExecuteSql (sql);
    if (r != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to run DDL statement");
        LOG.errorv ("Failed to execute DLL statement during mapping - %s", sql);
        }
    return r;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
DbResult ECDbMapAnalyser::ApplyChanges ()
    {
    Utf8String sql;
    DbResult r = UpdateHoldingView ();
    if (r != BE_SQLITE_OK)
        return r;

    for (auto& i : m_viewInfos)
        {
        ViewInfo const& viewInfo = i.second;
        if (viewInfo.GetView ().IsEmpty ())
            {
            BeAssert (false && "must have a view");
            continue;
            }

        sql = viewInfo.GetView ().ToString (SqlOption::DropIfExists, true);
        r = ExecuteDDL (sql.c_str ());
        if (r != BE_SQLITE_OK)
            return r;

        sql = viewInfo.GetView ().ToString (SqlOption::Create, true);
        r = ExecuteDDL (sql.c_str ());
        if (r != BE_SQLITE_OK)
            return r;

        for (auto & trigger : viewInfo.GetTriggers ().GetTriggers ())
            {
            if (!trigger.IsEmpty ())
                {
                sql = trigger.ToString (SqlOption::DropIfExists, true);
                r = ExecuteDDL (sql.c_str ());
                if (r != BE_SQLITE_OK)
                    return r;

                sql = trigger.ToString (SqlOption::Create, true);
                r = ExecuteDDL (sql.c_str ());
                if (r != BE_SQLITE_OK)
                    return r;
                }
            }

        }

    for (auto& i : m_storage)
        {
        auto& storage = *i.second;
        if (storage.GetTable ().GetOwnerType () == OwnerType::ExistingTable)
            {
            continue;
            }

        for (auto const& trigger : storage.GetTriggerList ().GetTriggers ())
            {
            if (storage.IsVirtual ())
                continue;

            BeAssert (trigger.IsEmpty () == false);

            sql = trigger.ToString (SqlOption::DropIfExists, true);
            r = ExecuteDDL (sql.c_str ());
            if (r != BE_SQLITE_OK)
                return r;

            sql = trigger.ToString (SqlOption::Create, true);
            r = ExecuteDDL (sql.c_str ());
            if (r != BE_SQLITE_OK)
                return r;
            }
        }

    return r;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMapAnalyser::Analyse (bool applyChanges)
    {
    m_classes.clear ();
    m_derivedClassLookup.clear ();
    m_relationships.clear ();
    m_storage.clear ();
    m_viewInfos.clear ();

    SetupDerivedClassLookup ();
    for (auto rootClassId : GetRootClassIds ())
        {
        if (AnalyseClass (*GetClassMap (rootClassId)) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        }

    for (auto rootClassId : GetRelationshipClassIds ())
        {
        if (AnalyseRelationshipClass (static_cast<RelationshipClassMapCR> (*GetClassMap (rootClassId))) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        }


    for (auto& i : m_classes)
        {
        auto ecclass = i.second.get ();
        if (!ecclass->RequireView ())
            continue;

        auto& viewInfo = m_viewInfos[ecclass];
        viewInfo.GetViewR () = std::move (BuildView (*ecclass));
        BuildPolymorphicDeleteTrigger (*ecclass);
        BuildPolymorphicUpdateTrigger (*ecclass);
        }

    ProcessEndTableRelationships ();
    ProcessLinkTableRelationships ();
    for (auto& storage : m_storage)
        {
        storage.second->Generate ();
        }

    if (applyChanges)
        return ApplyChanges () == DbResult::BE_SQLITE_OK ? BentleyStatus::SUCCESS : BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
DbResult ECDbMapAnalyser::UpdateHoldingView ()
    {
    NativeSqlBuilder viewSql;
    Utf8CP sql = "SELECT Id FROM ec_Class WHERE ec_Class.RelationStrength = 1"; // Holding relationships       
    NativeSqlBuilder::List unionList;
    auto stmt = m_map.GetECDbR ().GetCachedStatement (sql);
    if (!stmt.IsValid ())
        {
        BeAssert (false && "Failed to prepared statement");
        return DbResult::BE_SQLITE_ERROR;
        }
    std::map<ECDbSqlTable const*, ECDbMap::LightweightCache::RelationshipEnd> doneSet;
    while (stmt->Step () == BE_SQLITE_ROW)
        {
        ECClassId ecClassId = stmt->GetValueInt64 (0);
        auto holdingRelationshipClass = m_map.GetECDbR ().Schemas ().GetECClass (ecClassId);
        if (holdingRelationshipClass == nullptr)
            {
            BeAssert (false && "Fail to find class for holding relationship");
            return DbResult::BE_SQLITE_ERROR;
            }

        auto holdingRelationshipClassMap = static_cast<RelationshipClassMapCP>(m_map.GetClassMap (*holdingRelationshipClass));
        if (holdingRelationshipClassMap == nullptr || holdingRelationshipClassMap->GetTable ().GetPersistenceType () == PersistenceType::Virtual)
            continue;


        Utf8CP column;
        ECDbMap::LightweightCache::RelationshipEnd filter;
        if (holdingRelationshipClassMap->GetRelationshipClass ().GetStrengthDirection () == ECRelatedInstanceDirection::Forward)
            {
            column = holdingRelationshipClassMap->GetTargetECInstanceIdPropMap ()->GetFirstColumn ()->GetName ().c_str ();
            filter = ECDbMap::LightweightCache::RelationshipEnd::Source;
            }
        else
            {
            column = holdingRelationshipClassMap->GetSourceECInstanceIdPropMap ()->GetFirstColumn ()->GetName ().c_str ();
            filter = ECDbMap::LightweightCache::RelationshipEnd::Target;
            }

        auto table = &holdingRelationshipClassMap->GetTable ();
        auto itor = doneSet.find (table);
        if (itor == doneSet.end () || (((int)(itor->second) & (int)filter) == 0))
            {
            NativeSqlBuilder relaitonshipView;
            relaitonshipView.Append ("SELECT ");
            relaitonshipView.Append (column);
            relaitonshipView.Append (" ECInstanceId FROM ").Append (table->GetName ().c_str ());
            //relaitonshipView.Append (" WHERE (").Append (column).Append(" IS NOT NULL)").AppendEOL();
            unionList.push_back (std::move (relaitonshipView));
            if (itor == doneSet.end ())
                doneSet[table] = filter;
            else
                doneSet[table] = static_cast<ECDbMap::LightweightCache::RelationshipEnd>((int)(itor->second) & (int)(filter));
            }
        }
    viewSql.Append ("DROP VIEW IF EXISTS ").Append (ECDB_HOLDING_VIEW).Append (";").AppendEOL ();
    viewSql.Append ("CREATE VIEW ").Append (ECDB_HOLDING_VIEW).Append (" AS ").AppendEOL ();
    if (!unionList.empty ())
        {

        for (auto& select : unionList)
            {
            viewSql.Append (select);
            if (&select != &(unionList.back ()))
                viewSql.Append (" \r\n UNION ALL \r\n");
            }
        viewSql.Append (";\n");
        }
    else
        {
        viewSql.Append ("SELECT NULL ECInstanceId LIMIT 0;");
        }

    return ExecuteDDL (viewSql.ToString ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void ECDbMapAnalyser::HandleLinkTable (Storage* fromStorage, std::map<ECDbMapAnalyser::Storage*, std::set<ECDbMapAnalyser::Relationship*>> const& relationshipsByStorage, bool isFrom)
    {
    // table_delete_linkTable
    for (auto& i : relationshipsByStorage)
        {
        auto relationshipStorage = i.first;
        auto& relationships = i.second;
        if (relationshipStorage->IsVirtual ())
            continue;

        //Determine which source/target or both side to delete
        std::set<ECDbSqlColumn const*> forignKeys;
        for (auto relationship : relationships)
            {
            if (isFrom)
                forignKeys.insert (relationship->From ().GetInstanceId ()->GetFirstColumn ());
            else
                forignKeys.insert (relationship->To ().GetInstanceId ()->GetFirstColumn ());
            }

        auto& builder = fromStorage->GetTriggerListR ().Create (SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::After, false);
        builder.GetNameBuilder ()
            .Append (fromStorage->GetTable ().GetName ().c_str ())
            .Append ("_")
            .Append (relationshipStorage->GetTable ().GetName ().c_str ())
            .AppendIIf (isFrom, "_From", "_To")
            .Append ("_Delete");


        builder.GetOnBuilder ().Append (fromStorage->GetTable ().GetName ().c_str ());
        auto& body = builder.GetBodyBuilder ();
        for (auto relationship : relationships)
            {
            body.Append ("--4 ").Append (relationship->GetRelationshipClassMap ().GetRelationshipClass ().GetFullName ()).AppendEOL ();
            }

        body
            .Append ("DELETE FROM ")
            .AppendEscaped (relationshipStorage->GetTable ().GetName ().c_str ())
            .Append (" WHERE ");

        auto thisECInstanceIdColumn = fromStorage->GetTable ().GetFilteredColumnFirst (ColumnKind::ECInstanceId);
        for (auto forignKey : forignKeys)
            {
            body.AppendFormatted ("(OLD.[%s] = [%s])", thisECInstanceIdColumn->GetName ().c_str (), forignKey->GetName ().c_str ());
            if (forignKey != *forignKeys.rbegin ())
                {
                body.Append (" OR ");
                }
            }
        body.Append (";").AppendEOL ();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void ECDbMapAnalyser::ProcessLinkTableRelationships ()
    {
    //       lhs/from                rel table         rel
    std::map<Storage*, std::map<Storage*, std::set<Relationship*>>> lhs2rel;
    //       rhs/to                 rel table          rel
    std::map<Storage*, std::map<Storage*, std::set<Relationship*>>> rhs2rel;

    for (auto& i : m_relationships)
        {
        auto relationship = i.second.get ();
        if (!relationship->IsLinkTable ())
            continue;

        for (auto from : relationship->From ().GetStorages ())
            for (auto to : relationship->To ().GetStorages ())
                {
                lhs2rel[const_cast<Storage*>(from)][const_cast<Storage*>(&relationship->GetStorage ())].insert (relationship);
                rhs2rel[const_cast<Storage*>(to)][const_cast<Storage*>(&relationship->GetStorage ())].insert (relationship);
                }
        }

    for (auto& i : lhs2rel)
        {
        auto lhsStorage = i.first;
        auto& relByStorage = i.second;
        HandleLinkTable (lhsStorage, relByStorage, true);
        }
    for (auto& i : rhs2rel)
        {
        auto rhsStorage = i.first;
        auto& relByStorage = i.second;
        HandleLinkTable (rhsStorage, relByStorage, false);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         09/2015
//---------------------------------------------------------------------------------------
void ECDbMapAnalyser::ProcessEndTableRelationships ()
    {
    // DELETE FROM to WHERE to.Id = from.Key
    // FROM            TO
    //  [A] -> AHasB -> B
    //  
    //  DELETE B / UPDATE A.K = NULL
    //  A -> AHasB -> [B]
    //                
    //  DELETE B 
    // relationship UPDATE ....
    for (auto& i : m_relationships)
        {
        auto relationship = i.second.get ();
        if (relationship->IsLinkTable ())
            continue;

        if (relationship->IsMarkedForCascadeDelete ())
            continue;

        bool isSelfRelationship = false;
        auto const lhsStorages = relationship->From ().GetStorages ();
        auto const rhsStorages = relationship->To ().GetStorages ();
        if (lhsStorages.size () == 1 && rhsStorages.size () == 1)
            {
            isSelfRelationship = *lhsStorages.begin () == *rhsStorages.begin ();
            }

        bool persistedInFrom = &relationship->From () == &relationship->ForeignEnd ();

        if (persistedInFrom)
            {
            for (auto toStorage : relationship->To ().GetStorages ())
                {
                auto& builder = const_cast<Storage*>(toStorage)->GetTriggerListR ().Create (SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::After, false);
                builder.GetNameBuilder ()
                    .Append (toStorage->GetTable ().GetName ().c_str ())
                    .Append ("_")
                    .Append (relationship->GetSqlName ())
                    .Append ("_Delete");

                builder.GetOnBuilder ().Append (toStorage->GetTable ().GetName ().c_str ());
                auto& body = builder.GetBodyBuilder ();
                body.Append ("--10").Append (relationship->GetRelationshipClassMap ().GetRelationshipClass ().GetFullName ()).AppendEOL ();
                for (auto fromStorage : relationship->From ().GetStorages ())
                    {
                    body.Append ("UPDATE ")
                        .AppendEscaped (fromStorage->GetTable ().GetName ().c_str ())
                        .Append (" SET ")
                        .AppendEscaped (relationship->To ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str ())
                        .Append (" = NULL");

                    if (!relationship->To().GetClassId()->IsVirtual()
                        && relationship->To().GetClassId()->IsMappedToPrimaryTable()
                        && relationship->To().GetClassId()->GetFirstColumn()->GetKind() != ColumnKind::ECClassId)
                        {
                        body.Append(", ")
                            .AppendEscaped(relationship->To().GetClassId()->GetFirstColumn()->GetName().c_str())
                            .Append(" = NULL");
                        }

                    body.Append (" WHERE OLD.")
                        .AppendEscaped (relationship->To ().GetResolvedInstanceId (*toStorage).GetColumn().GetName ().c_str ())
                        .Append (" = ")
                        .AppendEscaped (relationship->To ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str ());
                    body.Append (";").AppendEOL ();
                    }
                }
            }
        else if (isSelfRelationship)
            {
            for (auto foreignEnd : relationship->ForeignEnd ().GetStorages ())
                {
                auto& builder = const_cast<Storage*>(foreignEnd)->GetTriggerListR ().Create (SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::After, false);
                builder.GetNameBuilder ()
                    .Append (foreignEnd->GetTable ().GetName ().c_str ())
                    .Append ("_")
                    .Append (relationship->GetSqlName ())
                    .Append ("_Self_Delete");

                builder.GetOnBuilder ().Append (foreignEnd->GetTable ().GetName ().c_str ());
                auto& body = builder.GetBodyBuilder ();
                body.Append ("--11").Append (relationship->GetRelationshipClassMap ().GetRelationshipClass ().GetFullName ()).AppendEOL ();
                for (auto primaryEnd : relationship->PrimaryEnd ().GetStorages ())
                    {
                    body.Append ("UPDATE ")
                        .AppendEscaped (primaryEnd->GetTable ().GetName ().c_str ())
                        .Append (" SET ")
                        .AppendEscaped (relationship->PrimaryEnd ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str ())
                        .Append (" = NULL");

                    if (!relationship->To ().GetClassId ()->IsVirtual ()
                        && relationship->To ().GetClassId ()->IsMappedToPrimaryTable ()
                        && relationship->To().GetClassId()->GetFirstColumn()->GetKind() != ColumnKind::ECClassId)
                        {
                        body.Append (", ")
                            .AppendEscaped (relationship->PrimaryEnd ().GetClassId ()->GetFirstColumn ()->GetName ().c_str ())
                            .Append (" = NULL");
                        }

                    body.Append (" WHERE OLD.")
                        .AppendEscaped (relationship->ForeignEnd ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str ())
                        .Append (" = ")
                        .AppendEscaped (relationship->PrimaryEnd ().GetInstanceId ()->GetFirstColumn ()->GetName ().c_str ());
                    body.Append (";").AppendEOL ();
                    }
                }
            }
        if (relationship->RequireCascade ())
            {
            for (auto fromStorage : relationship->From ().GetStorages ())
                {
                auto& builder = const_cast<Storage*>(fromStorage)->GetTriggerListR ()
                    .Create (SqlTriggerBuilder::Type::Delete, SqlTriggerBuilder::Condition::After, false);

                builder.GetNameBuilder ()
                    .Append (fromStorage->GetTable ().GetName ().c_str ())
                    .Append ("_")
                    .Append (relationship->GetSqlName ())
                    .Append ("_CascadeDelete");

                builder.GetOnBuilder ().Append (fromStorage->GetTable ().GetName ().c_str ());
                auto& body = builder.GetBodyBuilder ();
                body.Append ("--2 ").Append (relationship->GetRelationshipClassMap ().GetRelationshipClass ().GetFullName ()).AppendEOL ();
                for (auto toStorage : relationship->To ().GetStorages ())
                    {
                    body
                        .Append ("DELETE FROM ")
                        .AppendEscaped (toStorage->GetTable ().GetName ().c_str ())
                        .Append (" WHERE ");

                    auto toKeyColumn = relationship->To ().GetInstanceId ()->GetFirstColumn ();
                    auto fromKeyColumn = relationship->From ().GetInstanceId ()->GetFirstColumn ();
                    if (toStorage != fromStorage)
                        {
                        //Self join should not be processed here.
                        //This is issue with EndTable our Source/Target key is always in same table. Following should fix that
                        if (persistedInFrom)
                            {
                            fromKeyColumn = toStorage->GetTable ().GetFilteredColumnFirst (ColumnKind::ECInstanceId);
                            }
                        else
                            {
                            toKeyColumn = fromStorage->GetTable ().GetFilteredColumnFirst (ColumnKind::ECInstanceId);
                            }
                        }
                    body.AppendFormatted ("([%s] = OLD.[%s])",
                        fromKeyColumn->GetName ().c_str (),
                        toKeyColumn->GetName ().c_str ());

                    if (relationship->IsHolding ())
                        {
                        body.AppendFormatted (" AND (SELECT COUNT (*) FROM " ECDB_HOLDING_VIEW "  WHERE ECInstanceId = OLD.[%s]) = 0", toKeyColumn->GetName ().c_str ());
                        }
                    body.Append (";").AppendEOL ();

                    }
                }
            }
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE