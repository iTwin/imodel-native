/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PrimitiveECSqlBinder.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlBinder::_BindNull()
    {
    const DbResult sqliteStat = GetSqliteStatement().BindNull(GetSqlParameterIndex());
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindNull");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlBinder::_BindBoolean(bool value)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_Boolean);
    if (!stat.IsSuccess())
        return stat;

    const DbResult sqliteStat = GetSqliteStatement().BindInt(GetSqlParameterIndex(), value ? 1 : 0);
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindBoolean");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlBinder::_BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_Binary);
    if (!stat.IsSuccess())
        return stat;

    const DbResult sqliteStat = GetSqliteStatement().BindBlob(GetSqlParameterIndex(), value, binarySize, ToBeSQliteBindMakeCopy(makeCopy));
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindBlob");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlBinder::_BindZeroBlob(int blobSize)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_Binary);
    if (!stat.IsSuccess())
        return stat;

    const DbResult sqliteStat = GetSqliteStatement().BindZeroBlob(GetSqlParameterIndex(), blobSize);
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindZeroBlob");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlBinder::_BindDateTime(uint64_t julianDayMsec, DateTime::Info const& metadata)
    {
    const double jd = DateTime::MsecToRationalDay(julianDayMsec);
    return _BindDateTime(jd, metadata);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlBinder::_BindDateTime(double julianDay, DateTime::Info const& metadata)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_DateTime);
    if (!stat.IsSuccess())
        return stat;

    if (metadata.IsValid() && metadata.GetKind() == DateTime::Kind::Local)
        {
        LOG.error("ECDb does not support to bind local date times.");
        return ECSqlStatus::Error;
        }

    ECSqlTypeInfo const& parameterTypeInfo = GetTypeInfo();
    if (!parameterTypeInfo.DateTimeInfoMatches(metadata))
        {
        LOG.error("Metadata of DateTime value to bind doesn't match the metadata on the corresponding ECProperty.");
        return ECSqlStatus::Error;
        }

    const DbResult sqliteStat = GetSqliteStatement().BindDouble(GetSqlParameterIndex(), julianDay);
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindDateTime");

    return ECSqlStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlBinder::_BindDouble(double value)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_Double);
    if (!stat.IsSuccess())
        return stat;

    const DbResult sqliteStat = GetSqliteStatement().BindDouble(GetSqlParameterIndex(), value);
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindDouble");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlBinder::_BindInt(int value)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_Integer);
    if (!stat.IsSuccess())
        return stat;

    const DbResult sqliteStat = GetSqliteStatement().BindInt(GetSqlParameterIndex(), value);
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindInt");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlBinder::_BindInt64(int64_t value)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_Long);
    if (!stat.IsSuccess())
        return stat;

    const DbResult sqliteStat = GetSqliteStatement().BindInt64(GetSqlParameterIndex(), value);
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindInt64");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlBinder::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_String);
    if (!stat.IsSuccess())
        return stat;

    if (!Utf8String::IsNullOrEmpty(value) && GetTypeInfo().IsDateTime())
        {
        DateTime dt;
        if (SUCCESS != DateTime::FromString(dt, value))
            {
            LOG.errorv("Type mismatch. Failed to bind string '%s' to DateTime parameter. String must be a valid ISO 8601 date, time or timestamp.", value);
            return ECSqlStatus::Error;
            }

        return BindDateTime(dt);
        }

    const DbResult sqliteStat = GetSqliteStatement().BindText(GetSqlParameterIndex(), value, ToBeSQliteBindMakeCopy(makeCopy), byteCount);
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "Failed to bind string value to parameter.");

    return ECSqlStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlBinder::_BindPoint2d(DPoint2dCR value)
    {
    LOG.error("Type mismatch. Point2d value can only be bound to parameter of same type.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlBinder::_BindPoint3d(DPoint3dCR value)
    {
    LOG.error("Type mismatch. Point3d value can only be bound to parameter of same type.");
    return ECSqlStatus::Error;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& PrimitiveECSqlBinder::_BindStructMember(Utf8CP structMemberPropertyName)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to primitive type parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& PrimitiveECSqlBinder::_BindStructMember(ECN::ECPropertyId structMemberPropertyId)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to primitive type parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& PrimitiveECSqlBinder::_AddArrayElement()
    {
    LOG.error("Type mismatch. Cannot bind array to primitive parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlBinder::CanBind(ECN::PrimitiveType requestedType) const
    {
    //For DateTimes and Geometry column type and BindXXX type must match. All other types are implicitly
    //converted to each other by SQLite.
    const PrimitiveType fieldDataType = GetTypeInfo().GetPrimitiveType();
    switch (fieldDataType)
        {
            case PRIMITIVETYPE_DateTime:
            {
            if (requestedType != PRIMITIVETYPE_DateTime && requestedType != PRIMITIVETYPE_String)
                {
                LOG.error("Type mismatch: only BindDateTime or BindText can be called for a column of the DateTime type.");
                return ECSqlStatus::Error;
                }
            else
                break;
            }
            case PRIMITIVETYPE_IGeometry:
            {
            if (requestedType != PRIMITIVETYPE_IGeometry && requestedType != PRIMITIVETYPE_Binary)
                {
                LOG.error("Type mismatch: only BindGeometry or BindBlob can be called for a column of the IGeometry type.");
                return ECSqlStatus::Error;
                }
            else
                break;
            }

            default:
                break;
        }

    switch (requestedType)
        {
            case PRIMITIVETYPE_DateTime:
            {
            if (requestedType != fieldDataType)
                {
                LOG.error("Type mismatch: BindDateTime cannot be called for a column which is not of the DateTime type.");
                return ECSqlStatus::Error;
                }
            else
                break;
            }
            case PRIMITIVETYPE_IGeometry:
            {
            if (requestedType != fieldDataType)
                {
                LOG.error("Type mismatch: BindGeometry cannot be called for a column which is not of the IGeometry type.");
                return ECSqlStatus::Error;
                }
            else
                break;
            }
            default:
                break;
        }

    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE