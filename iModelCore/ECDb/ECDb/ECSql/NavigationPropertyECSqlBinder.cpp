/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/NavigationPropertyECSqlBinder.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "NavigationPropertyECSqlBinder.h"
#include "SystemPropertyECSqlBinder.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
NavigationPropertyECSqlBinder::NavigationPropertyECSqlBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& ecsqlTypeInfo)
    : ECSqlBinder(ecsqlStatement, ecsqlTypeInfo, 0, true, true), IECSqlStructBinder(), m_idBinder(nullptr), m_relECClassIdBinder(nullptr)
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
    m_idBinder = ECSqlBinderFactory::CreateIdBinder(GetECSqlStatementR(), navIdPropMap, ECSqlSystemPropertyKind::NavigationId);

    int mappedSqliteParameterCount = m_idBinder->GetMappedSqlParameterCount();
    BeAssert(mappedSqliteParameterCount == 1);
    totalMappedSqliteParameterCount += mappedSqliteParameterCount;

    NavigationPropertyMap::RelECClassIdPropertyMap const& navRelClassIdPropMap = navPropMap->GetRelECClassIdPropertyMap();
    m_relECClassIdBinder = ECSqlBinderFactory::CreateIdBinder(GetECSqlStatementR(), navRelClassIdPropMap, ECSqlSystemPropertyKind::NavigationRelECClassId);

    mappedSqliteParameterCount = m_relECClassIdBinder->GetMappedSqlParameterCount();
    totalMappedSqliteParameterCount += mappedSqliteParameterCount;

    SetMappedSqlParameterCount(totalMappedSqliteParameterCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
void NavigationPropertyECSqlBinder::_SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex)
    {
    if (ecsqlParameterComponentIndex == 0)
        m_idBinder->SetSqliteIndex((size_t) ecsqlParameterComponentIndex);
    else if (ecsqlParameterComponentIndex == 1)
        m_relECClassIdBinder->SetSqliteIndex((size_t) ecsqlParameterComponentIndex);

    BeAssert(false);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlBinder::_OnBeforeStep()
    {
    ECSqlStatus stat = m_idBinder->OnBeforeStep();
    if (stat != ECSqlStatus::Success)
        return stat;

    return m_relECClassIdBinder->OnBeforeStep();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
void NavigationPropertyECSqlBinder::_OnClearBindings()
    {
    m_idBinder->OnClearBindings();
    m_relECClassIdBinder->OnClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlBinder::_BindNull()
    {
    ECSqlStatus stat = m_idBinder->BindNull();
    if (!stat.IsSuccess())
        return stat;

    return m_relECClassIdBinder->BindNull();
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
    if (BeStringUtilities::StricmpAscii(ECDbSystemSchemaHelper::NAVPROP_ID_PROPNAME, navPropMemberPropertyName) == 0)
        {
        BeAssert(m_idBinder != nullptr);
        return *m_idBinder;
        }

    if (BeStringUtilities::StricmpAscii(ECDbSystemSchemaHelper::NAVPROP_RELECCLASSID_PROPNAME, navPropMemberPropertyName) == 0)
        {
        BeAssert(m_relECClassIdBinder != nullptr);
        return *m_relECClassIdBinder;
        }

    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Cannot bind to NavigationECProperty member. Member %s is not valid. Only %s and %s are valid members.",
                                                       navPropMemberPropertyName, ECDbSystemSchemaHelper::NAVPROP_ID_PROPNAME, ECDbSystemSchemaHelper::NAVPROP_RELECCLASSID_PROPNAME);
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& NavigationPropertyECSqlBinder::_GetMember(ECN::ECPropertyId navPropMemberPropertyId)
    {
    if (navPropMemberPropertyId == ECDbSystemSchemaHelper::GetSystemProperty(GetECDb().Schemas(), ECSqlSystemPropertyKind::NavigationId)->GetId())
        {
        BeAssert(m_idBinder != nullptr);
        return *m_idBinder;
        }

    if (navPropMemberPropertyId == ECDbSystemSchemaHelper::GetSystemProperty(GetECDb().Schemas(), ECSqlSystemPropertyKind::NavigationRelECClassId)->GetId())
        {
        BeAssert(m_relECClassIdBinder != nullptr);
        return *m_relECClassIdBinder;
        }

    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Cannot bind to NavigationECProperty member. Member with property index %s is not valid. Only %s and %s are valid members.",
                                                       navPropMemberPropertyId.ToString().c_str(), ECDbSystemSchemaHelper::NAVPROP_ID_PROPNAME, ECDbSystemSchemaHelper::NAVPROP_RELECCLASSID_PROPNAME);
    return NoopECSqlBinder::Get();
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
