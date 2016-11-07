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
    : ECSqlBinder(ecsqlStatement, ecsqlTypeInfo, 0, false, false), IECSqlStructBinder()
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

    m_memberBinders.push_back(std::move(idBinder));

    NavigationPropertyMap::RelECClassIdPropertyMap const& navRelClassIdPropMap = navPropMap->GetRelECClassIdPropertyMap();
    std::unique_ptr<ECSqlBinder> relClassIdBinder = ECSqlBinderFactory::CreateIdBinder(GetECSqlStatementR(), navRelClassIdPropMap, ECSqlSystemPropertyKind::NavigationRelECClassId);

    mappedSqliteParameterCount = relClassIdBinder->GetMappedSqlParameterCount();
    totalMappedSqliteParameterCount += mappedSqliteParameterCount;

    m_memberBinders.push_back(std::move(relClassIdBinder));

    SetMappedSqlParameterCount(totalMappedSqliteParameterCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
void NavigationPropertyECSqlBinder::_SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex)
    {
    if (ecsqlParameterComponentIndex > 1)
        {
        BeAssert(false);
        return;
        }

    std::unique_ptr<ECSqlBinder>& memberBinder = m_memberBinders[(size_t) ecsqlParameterComponentIndex];
    BeAssert(memberBinder != nullptr);
    memberBinder->SetSqliteIndex(sqliteParameterIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlBinder::_BindNull()
    {
    for (std::unique_ptr<ECSqlBinder>& memberBinder : m_memberBinders)
        {
        ECSqlStatus stat = memberBinder->BindNull();
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
    size_t memberIndex = 0;
    if (BeStringUtilities::StricmpAscii(ECDbSystemSchemaHelper::NAVPROP_ID_PROPNAME, navPropMemberPropertyName) == 0)
        memberIndex = ID_MEMBER_INDEX;
    else if (BeStringUtilities::StricmpAscii(ECDbSystemSchemaHelper::NAVPROP_RELECCLASSID_PROPNAME, navPropMemberPropertyName) == 0)
        memberIndex = RELECCLASSID_MEMBER_INDEX;
    else
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Cannot bind to NavigationECProperty member. Member %s is not valid. Only %s and %s are valid members.", 
                                                           navPropMemberPropertyName, ECDbSystemSchemaHelper::NAVPROP_ID_PROPNAME, ECDbSystemSchemaHelper::NAVPROP_RELECCLASSID_PROPNAME);
        return NoopECSqlBinder::Get();
        }

    BeAssert(m_memberBinders[memberIndex] != nullptr);
    return *m_memberBinders[memberIndex];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& NavigationPropertyECSqlBinder::_GetMember(ECN::ECPropertyId navPropMemberPropertyId)
    {
    size_t memberIndex = 0;
    if (navPropMemberPropertyId == ECDbSystemSchemaHelper::GetSystemProperty(GetECDb().Schemas(), ECSqlSystemPropertyKind::NavigationId)->GetId())
        memberIndex = ID_MEMBER_INDEX;
    else if (navPropMemberPropertyId == ECDbSystemSchemaHelper::GetSystemProperty(GetECDb().Schemas(), ECSqlSystemPropertyKind::NavigationRelECClassId)->GetId())
        memberIndex = RELECCLASSID_MEMBER_INDEX;
    else
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Cannot bind to NavigationECProperty member. Member with property index %s is not valid. Only %s and %s are valid members.",
                                                          navPropMemberPropertyId.ToString().c_str(), ECDbSystemSchemaHelper::NAVPROP_ID_PROPNAME, ECDbSystemSchemaHelper::NAVPROP_RELECCLASSID_PROPNAME);
        return NoopECSqlBinder::Get();
        }

    BeAssert(m_memberBinders[memberIndex] != nullptr);
    return *m_memberBinders[memberIndex];
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
