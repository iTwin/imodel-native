/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/NavigationPropertyECSqlBinder.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "NavigationPropertyECSqlBinder.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
NavigationPropertyECSqlBinder::NavigationPropertyECSqlBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& ecsqlTypeInfo)
    : ECSqlBinder(ecsqlStatement, ecsqlTypeInfo, 0, true, true), IECSqlStructBinder(), m_idSqliteIndex(-1), m_relClassIdSqliteIndex(-1)
    {
    Initialize();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void NavigationPropertyECSqlBinder::Initialize()
    {
    ECSqlTypeInfo const& typeInfo = GetTypeInfo();
    BeAssert(typeInfo.GetPropertyMap() != nullptr && typeInfo.GetPropertyMap()->GetType() == PropertyMap::Type::Navigation);
    NavigationPropertyMap const* navPropMap = static_cast<NavigationPropertyMap const*> (typeInfo.GetPropertyMap());

    int totalMappedSqliteParameterCount = 0;
    NavigationPropertyMap::IdPropertyMap const& navIdPropMap = navPropMap->GetIdPropertyMap();
    std::unique_ptr<ECSqlBinder> idBinder = ECSqlBinderFactory::CreateIdBinder(GetECSqlStatementR(), navIdPropMap, ECSqlSystemPropertyKind::NavigationId);

    int mappedSqliteParameterCount = idBinder->GetMappedSqlParameterCount();
    BeAssert(mappedSqliteParameterCount == 1);
    totalMappedSqliteParameterCount += mappedSqliteParameterCount;

    m_memberBinders[navIdPropMap.GetProperty().GetId()] = std::move(idBinder);

    NavigationPropertyMap::RelECClassIdPropertyMap const& navRelClassIdPropMap = navPropMap->GetRelECClassIdPropertyMap();
    std::unique_ptr<ECSqlBinder> relClassIdBinder = ECSqlBinderFactory::CreateIdBinder(GetECSqlStatementR(), navRelClassIdPropMap, ECSqlSystemPropertyKind::NavigationRelECClassId);

    mappedSqliteParameterCount = relClassIdBinder->GetMappedSqlParameterCount();
    totalMappedSqliteParameterCount += mappedSqliteParameterCount;

    m_memberBinders[navRelClassIdPropMap.GetProperty().GetId()] = std::move(relClassIdBinder);

    SetMappedSqlParameterCount(totalMappedSqliteParameterCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
void NavigationPropertyECSqlBinder::_SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex)
    {
    switch (ecsqlParameterComponentIndex)
        {
            case 0: m_idSqliteIndex = (int) sqliteParameterIndex; break;
            case 1: m_relClassIdSqliteIndex = (int) sqliteParameterIndex; break;

            default:
                BeAssert(ecsqlParameterComponentIndex <= 1);
                break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2015
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlBinder::_OnBeforeStep()
    {
    for (auto const& kvPair : m_memberBinders)
        {
        ECSqlStatus stat = kvPair.second->OnBeforeStep();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2015
//---------------------------------------------------------------------------------------
void NavigationPropertyECSqlBinder::_OnClearBindings()
    {
    for (auto const& kvPair : m_memberBinders)
        {
        kvPair.second->OnClearBindings();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlBinder::_BindNull()
    {
    for (auto const& kvPair : m_memberBinders)
        {
        ECSqlStatus stat = kvPair.second->BindNull();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveBinder& NavigationPropertyECSqlBinder::_BindPrimitive()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind primitive value to a NavigationECProperty parameter.");
    return NoopECSqlBinder::Get().BindPrimitive();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& NavigationPropertyECSqlBinder::_BindArray(uint32_t initialCapacity)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind array to a NavigationECProperty parameter.");
    return NoopECSqlBinder::Get().BindArray(initialCapacity);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& NavigationPropertyECSqlBinder::_GetMember(Utf8CP navPropMemberPropertyName)
    {
    ECSqlSystemPropertyKind sysPropKind;
    if (BeStringUtilities::StricmpAscii(ECDbSystemSchemaHelper::NAVPROP_ID_PROPNAME, navPropMemberPropertyName) == 0)
        sysPropKind = ECSqlSystemPropertyKind::NavigationId;
    else if (BeStringUtilities::StricmpAscii(ECDbSystemSchemaHelper::NAVPROP_RELECCLASSID_PROPNAME, navPropMemberPropertyName) == 0)
        sysPropKind = ECSqlSystemPropertyKind::NavigationRelECClassId;
    else
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Cannot bind to NavigationECProperty member. Member %s is not valid. Only %s and %s are valid members.", 
                                                           navPropMemberPropertyName, ECDbSystemSchemaHelper::NAVPROP_ID_PROPNAME, ECDbSystemSchemaHelper::NAVPROP_RELECCLASSID_PROPNAME);
        return NoopECSqlBinder::Get();
        }

    ECPropertyCP memberProp = ECDbSystemSchemaHelper::GetSystemProperty(GetECDb().Schemas(), sysPropKind);
    if (memberProp == nullptr)
        {
        BeAssert(false);
        return NoopECSqlBinder::Get();
        }

    return _GetMember(memberProp->GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& NavigationPropertyECSqlBinder::_GetMember(ECN::ECPropertyId navPropMemberPropertyId)
    {
    auto it = m_memberBinders.find(navPropMemberPropertyId);
    if (it == m_memberBinders.end())
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Cannot bind to NavigationECProperty member. Member with ECPropertyId %s is not valid. Only %s and %s are valid members.", 
                                                           navPropMemberPropertyId.ToString().c_str(), ECDbSystemSchemaHelper::NAVPROP_ID_PROPNAME, ECDbSystemSchemaHelper::NAVPROP_RELECCLASSID_PROPNAME);        
        return NoopECSqlBinder::Get();
        }

    return *it->second;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
