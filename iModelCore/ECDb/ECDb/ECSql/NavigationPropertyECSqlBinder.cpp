/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
NavigationPropertyECSqlBinder::NavigationPropertyECSqlBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& ecsqlTypeInfo, SqlParamNameGenerator& paramNameGen)
    : ECSqlBinder(ctx, ecsqlTypeInfo, paramNameGen, true, false)
    {
    Initialize(ctx, paramNameGen);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
BentleyStatus NavigationPropertyECSqlBinder::Initialize(ECSqlPrepareContext& ctx, SqlParamNameGenerator& paramNameGen)
    {
    ECSqlTypeInfo const& typeInfo = GetTypeInfo();
    if (typeInfo.GetPropertyMap() == nullptr || typeInfo.GetPropertyMap()->GetType() != PropertyMap::Type::Navigation)
        {
        BeAssert(false);
        return ERROR;
        }

    NavigationPropertyMap const& navPropMap = typeInfo.GetPropertyMap()->GetAs<NavigationPropertyMap>();

    NavigationPropertyMap::IdPropertyMap const& navIdPropMap = navPropMap.GetIdPropertyMap();
    m_idBinder = ECSqlBinderFactory::CreateIdBinder(ctx, navIdPropMap, ECSqlSystemPropertyInfo::NavigationId(), paramNameGen);
    if (m_idBinder == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    AddChildMemberMappedSqlParameterIndices(*m_idBinder);

    NavigationPropertyMap::RelECClassIdPropertyMap const& navRelClassIdPropMap = navPropMap.GetRelECClassIdPropertyMap();
    m_relECClassIdBinder = ECSqlBinderFactory::CreateIdBinder(ctx, navRelClassIdPropMap, ECSqlSystemPropertyInfo::NavigationRelECClassId(), paramNameGen);
    if (m_relECClassIdBinder == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    AddChildMemberMappedSqlParameterIndices(*m_relECClassIdBinder);
    return SUCCESS;
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
    if (navPropMemberPropertyId == GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemProperty(ECSqlSystemPropertyInfo::NavigationId())->GetId())
        {
        BeAssert(m_idBinder != nullptr);
        return *m_idBinder;
        }

    if (navPropMemberPropertyId == GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemProperty(ECSqlSystemPropertyInfo::NavigationRelECClassId())->GetId())
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
