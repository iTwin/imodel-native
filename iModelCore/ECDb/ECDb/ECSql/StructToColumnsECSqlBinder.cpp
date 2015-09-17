/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructToColumnsECSqlBinder.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "StructToColumnsECSqlBinder.h"
#include "ECSqlTypeInfo.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
StructToColumnsECSqlBinder::StructToColumnsECSqlBinder (ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& ecsqlTypeInfo)
: ECSqlBinder (ecsqlStatement, ecsqlTypeInfo, 0, true, true), IECSqlStructBinder ()
    {
    Initialize ();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void StructToColumnsECSqlBinder::Initialize ()
    {
    auto const& typeInfo = GetTypeInfo ();
    auto propMap = typeInfo.GetPropertyMap ();
    BeAssert (propMap != nullptr && "Struct parameters are expected to always have a PropertyNameExp as target expression");
    BeAssert (dynamic_cast<PropertyMapToInLineStructCP> (propMap) != nullptr);

    int totalMappedSqliteParameterCount = 0;
    for (auto childPropMap : propMap->GetChildren ()) //GetChildren ensures the correct and always same order
        {
        if (childPropMap->IsUnmapped ())
            continue;

        auto binder = ECSqlBinderFactory::CreateBinder (GetECSqlStatementR (), *childPropMap);
        int mappedSqliteParameterCount = binder->GetMappedSqlParameterCount ();

        auto binderP = binder.get (); //cache raw pointer as it is needed after the unique_ptr has been moved into the collection
        m_memberBinders[childPropMap->GetProperty ().GetId ()] = std::move (binder);

        //for SetSqliteIndex we need a mapping from a given ECSqlComponent index to the respective member binder
        //and also the relative component index within that member binder.
        m_ecsqlComponentIndexToMemberBinderMapping.reserve (mappedSqliteParameterCount);
        m_ecsqlComponentIndexToMemberBinderMapping.insert (m_ecsqlComponentIndexToMemberBinderMapping.end (), mappedSqliteParameterCount,
                                MemberBinderInfo (*binderP, totalMappedSqliteParameterCount));

        totalMappedSqliteParameterCount += mappedSqliteParameterCount;
        }

    SetMappedSqlParameterCount (totalMappedSqliteParameterCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
void StructToColumnsECSqlBinder::_SetSqliteIndex (int ecsqlParameterComponentIndex, size_t sqliteIndex)
    {
    BeAssert (ecsqlParameterComponentIndex >= 0 && (size_t) ecsqlParameterComponentIndex < m_ecsqlComponentIndexToMemberBinderMapping.size ());
    auto const& mapping = m_ecsqlComponentIndexToMemberBinderMapping[(size_t) ecsqlParameterComponentIndex];
    int compIndexOfMember = ecsqlParameterComponentIndex - mapping.GetECSqlComponentIndexOffset ();

    mapping.GetMemberBinder ().SetSqliteIndex (compIndexOfMember, sqliteIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2015
//---------------------------------------------------------------------------------------
ECSqlStatus StructToColumnsECSqlBinder::_OnBeforeStep ()
    {
    for (auto const& kvPair : m_memberBinders)
        {
        auto stat = kvPair.second->OnBeforeStep ();
        if (stat != ECSqlStatus::Success)
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2015
//---------------------------------------------------------------------------------------
void StructToColumnsECSqlBinder::_OnClearBindings ()
    {
    for (auto const& kvPair : m_memberBinders)
        {
        kvPair.second->OnClearBindings ();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus StructToColumnsECSqlBinder::_BindNull ()
    {
    for (auto const& kvPair : m_memberBinders)
        {
        auto stat = kvPair.second->BindNull ();
        if (stat != ECSqlStatus::Success)
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveBinder& StructToColumnsECSqlBinder::_BindPrimitive ()
    {
    GetECDb().GetECDbImplR().ReportIssue(ECDb::IssueSeverity::Error, "Type mismatch. Cannot bind primitive value to ECStruct parameter.");
    return GetNoopBinder (ECSqlStatus::UserError).BindPrimitive ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlStructBinder& StructToColumnsECSqlBinder::_BindStruct ()
    {
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& StructToColumnsECSqlBinder::_BindArray (uint32_t initialCapacity)
    {
    GetECDb().GetECDbImplR().ReportIssue(ECDb::IssueSeverity::Error, "Type mismatch. Cannot bind array to ECStruct parameter.");
    return GetNoopBinder (ECSqlStatus::UserError).BindArray (initialCapacity);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& StructToColumnsECSqlBinder::_GetMember (Utf8CP structMemberPropertyName)
    {
    auto memberProp = GetTypeInfo ().GetStructType ().GetPropertyP (structMemberPropertyName, true);
    if (memberProp == nullptr)
        {
        GetECDb().GetECDbImplR().ReportIssue(ECDb::IssueSeverity::Error, "Cannot bind to struct member. Member %s does not exist.", structMemberPropertyName);
        BeAssert (false);
        return GetNoopBinder (ECSqlStatus::UserError);
        }

    return _GetMember (memberProp->GetId ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& StructToColumnsECSqlBinder::_GetMember (ECN::ECPropertyId structMemberPropertyId)
    {
    auto it = m_memberBinders.find (structMemberPropertyId);
    if (it == m_memberBinders.end ())
        {
        Utf8CP structMemberPropertyName = nullptr;
        for (auto prop : GetTypeInfo ().GetStructType ().GetProperties (true))
            {
            if (prop->GetId () == structMemberPropertyId)
                {
                structMemberPropertyName = prop->GetName ().c_str ();
                break;
                }
            }

        GetECDb().GetECDbImplR().ReportIssue(ECDb::IssueSeverity::Error, "Cannot bind to struct member. Member %s does not exist.", structMemberPropertyName);
        return GetNoopBinder (ECSqlStatus::UserError);
        }
    else
        return *it->second;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE