/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/NavigationPropertyECSqlBinder.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "NavigationPropertyECSqlBinder.h"
#include "IdECSqlBinder.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
NavigationPropertyECSqlBinder::NavigationPropertyECSqlBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& ecsqlTypeInfo)
    : ECSqlBinder(ecsqlStatement, ecsqlTypeInfo, 0, false, false), IECSqlStructBinder(), m_idBinder(nullptr), m_relECClassIdBinder(nullptr)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
BentleyStatus NavigationPropertyECSqlBinder::Initialize(ECSqlPrepareContext& ctx)
    {
    ECSqlTypeInfo const& typeInfo = GetTypeInfo();
    if (typeInfo.GetPropertyMap() == nullptr || typeInfo.GetPropertyMap()->GetType() != PropertyMap::Type::Navigation)
        {
        BeAssert(false);
        return ERROR;
        }

    NavigationPropertyMap const* navPropMap = typeInfo.GetPropertyMap()->GetAs<NavigationPropertyMap>();

    int totalMappedSqliteParameterCount = 0;
    NavigationPropertyMap::IdPropertyMap const& navIdPropMap = navPropMap->GetIdPropertyMap();
    m_idBinder = ECSqlBinderFactory::CreateIdBinder(ctx, navIdPropMap, ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Navigation::Id));
    if (m_idBinder == nullptr)
        return ERROR;

    int mappedSqliteParameterCount = m_idBinder->GetMappedSqlParameterCount();
    BeAssert(mappedSqliteParameterCount == 1);
    totalMappedSqliteParameterCount += mappedSqliteParameterCount;

    NavigationPropertyMap::RelECClassIdPropertyMap const& navRelClassIdPropMap = navPropMap->GetRelECClassIdPropertyMap();
    m_relECClassIdBinder = ECSqlBinderFactory::CreateIdBinder(ctx, navRelClassIdPropMap, ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Navigation::RelECClassId));
    if (m_relECClassIdBinder == nullptr)
        return ERROR;

    mappedSqliteParameterCount = m_relECClassIdBinder->GetMappedSqlParameterCount();
    totalMappedSqliteParameterCount += mappedSqliteParameterCount;

    SetMappedSqlParameterCount(totalMappedSqliteParameterCount);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
void NavigationPropertyECSqlBinder::_SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex)
    {
    switch (ecsqlParameterComponentIndex)
        {
            case 0:
                m_idBinder->SetSqliteIndex((size_t) sqliteParameterIndex);
                return;

            case 1:
                m_relECClassIdBinder->SetSqliteIndex((size_t) sqliteParameterIndex);
                return;

            default:
                BeAssert(false);
                return;
        }
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
    if (navPropMemberPropertyId == ECDbSystemSchemaHelper::GetSystemProperty(GetECDb().Schemas(), ECSqlSystemPropertyInfo::Navigation::Id)->GetId())
        {
        BeAssert(m_idBinder != nullptr);
        return *m_idBinder;
        }

    if (navPropMemberPropertyId == ECDbSystemSchemaHelper::GetSystemProperty(GetECDb().Schemas(), ECSqlSystemPropertyInfo::Navigation::RelECClassId)->GetId())
        {
        BeAssert(m_relECClassIdBinder != nullptr);
        return *m_relECClassIdBinder;
        }

    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Cannot bind to NavigationECProperty member. Member with property index %s is not valid. Only %s and %s are valid members.",
                                                       navPropMemberPropertyId.ToString().c_str(), ECDbSystemSchemaHelper::NAVPROP_ID_PROPNAME, ECDbSystemSchemaHelper::NAVPROP_RELECCLASSID_PROPNAME);
    return NoopECSqlBinder::Get();
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
