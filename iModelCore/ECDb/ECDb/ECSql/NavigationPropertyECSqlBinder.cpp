/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/NavigationPropertyECSqlBinder.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
NavigationPropertyECSqlBinder::NavigationPropertyECSqlBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& ecsqlTypeInfo)
    : ECSqlBinder(ecsqlStatement, ecsqlTypeInfo, 0, true, false), m_idBinder(nullptr), m_relECClassIdBinder(nullptr)
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

    NavigationPropertyMap const& navPropMap = typeInfo.GetPropertyMap()->GetAs<NavigationPropertyMap>();

    int totalMappedSqliteParameterCount = 0;
    NavigationPropertyMap::IdPropertyMap const& navIdPropMap = navPropMap.GetIdPropertyMap();
    m_idBinder = ECSqlBinderFactory::CreateIdBinder(ctx, navIdPropMap, ECSqlSystemPropertyInfo::NavigationId());
    if (m_idBinder == nullptr)
        return ERROR;

    int mappedSqliteParameterCount = m_idBinder->GetMappedSqlParameterCount();
    BeAssert(mappedSqliteParameterCount == 1);
    totalMappedSqliteParameterCount += mappedSqliteParameterCount;

    NavigationPropertyMap::RelECClassIdPropertyMap const& navRelClassIdPropMap = navPropMap.GetRelECClassIdPropertyMap();
    m_relECClassIdBinder = ECSqlBinderFactory::CreateIdBinder(ctx, navRelClassIdPropMap, ECSqlSystemPropertyInfo::NavigationRelECClassId());
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
IECSqlBinder& NavigationPropertyECSqlBinder::_BindStructMember(Utf8CP navPropMemberPropertyName)
    {
    if (BeStringUtilities::StricmpAscii(ECDBSYS_PROP_NavPropId, navPropMemberPropertyName) == 0)
        {
        BeAssert(m_idBinder != nullptr);
        return *m_idBinder;
        }

    if (BeStringUtilities::StricmpAscii(ECDBSYS_PROP_NavPropRelECClassId, navPropMemberPropertyName) == 0)
        {
        BeAssert(m_relECClassIdBinder != nullptr);
        return *m_relECClassIdBinder;
        }

    LOG.errorv("Cannot bind to NavigationECProperty member. Member %s is not valid. "
               "Only " ECDBSYS_PROP_NavPropId " and " ECDBSYS_PROP_NavPropRelECClassId " are valid members.",
                 navPropMemberPropertyName);
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& NavigationPropertyECSqlBinder::_BindStructMember(ECN::ECPropertyId navPropMemberPropertyId)
    {
    if (navPropMemberPropertyId == GetECDb().Schemas().GetReader().GetSystemSchemaHelper().GetSystemProperty(ECSqlSystemPropertyInfo::NavigationId())->GetId())
        {
        BeAssert(m_idBinder != nullptr);
        return *m_idBinder;
        }

    if (navPropMemberPropertyId == GetECDb().Schemas().GetReader().GetSystemSchemaHelper().GetSystemProperty(ECSqlSystemPropertyInfo::NavigationRelECClassId())->GetId())
        {
        BeAssert(m_relECClassIdBinder != nullptr);
        return *m_relECClassIdBinder;
        }

    LOG.errorv("Cannot bind to NavigationECProperty member. Member with property index %s is not valid. "
               "Only " ECDBSYS_PROP_NavPropId " and " ECDBSYS_PROP_NavPropRelECClassId " are valid members.",
                  navPropMemberPropertyId.ToString().c_str());
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlBinder::_BindBoolean(bool value)
    {
    LOG.error("Type mismatch. Cannot bind boolean value to navigation property parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlBinder::_BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    LOG.error("Type mismatch. Cannot bind Blob value to navigation property parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2016
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlBinder::_BindZeroBlob(int blobSize)
    {
    LOG.error("Type mismatch. Cannot bind Zeroblob value to navigation property parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2015
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlBinder::_BindDateTime(uint64_t julianDayMsec, DateTime::Info const&)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to navigation property parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlBinder::_BindDateTime(double julianDay, DateTime::Info const&)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to navigation property parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlBinder::_BindDouble(double value)
    {
    LOG.error("Type mismatch. Cannot bind double value to navigation property parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlBinder::_BindInt(int value)
    {
    LOG.error("Type mismatch. Cannot bind integer value to navigation property parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlBinder::_BindInt64(int64_t value)
    {
    LOG.error("Type mismatch. Cannot bind int64_t value to navigation property parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlBinder::_BindText(Utf8CP stringValue, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    LOG.error("Type mismatch. Cannot bind string value to navigation property parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlBinder::_BindPoint2d(DPoint2dCR value)
    {
    LOG.error("Type mismatch. Cannot bind Point2d value to navigation property parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus NavigationPropertyECSqlBinder::_BindPoint3d(DPoint3dCR value)
    {
    LOG.error("Type mismatch. Cannot bind Point3d value to navigation property parameter.");
    return ECSqlStatus::Error;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& NavigationPropertyECSqlBinder::_AddArrayElement()
    {
    LOG.error("Type mismatch. Cannot bind array to navigation property parameter.");
    return NoopECSqlBinder::Get();
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
