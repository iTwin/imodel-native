/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/IdECSqlBinder.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
IdECSqlBinder::IdECSqlBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo, bool isNoop, SqlParamNameGenerator& paramNameGen)
    : ECSqlBinder(ctx, typeInfo, paramNameGen, isNoop ? 0 : 1, false, false), m_isNoop(isNoop)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindNull()
    {
    if (!m_isNoop)
        {
        const DbResult sqliteStat = GetSqliteStatement().BindNull(GetSqlParamIndex());
        if (sqliteStat != BE_SQLITE_OK)
            return LogSqliteError(sqliteStat, "ECSqlStatement::BindNull against id property.");
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindPoint2d(DPoint2dCR value)
    {
    LOG.error("Type mismatch. Cannot bind Point2d value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindPoint3d(DPoint3dCR value)
    {
    LOG.error("Type mismatch. Cannot bind Point3d value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindBoolean(bool value)
    {
    LOG.error("Type mismatch. Cannot bind boolean value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy)
    {
    LOG.error("Type mismatch. Cannot bind Blob value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2016
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindZeroBlob(int blobSize)
    {
    LOG.error("Type mismatch. Cannot bind Zeroblob value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindDateTime(double julianDay, DateTime::Info const&)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindDateTime(uint64_t julianDayHns, DateTime::Info const&)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindDouble(double value)
    {
    LOG.error("Type mismatch. Cannot bind double value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindInt(int value)
    {
    LOG.error("Type mismatch. Cannot bind 32 bit integer value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindInt64(int64_t value)
    {
    if (!m_isNoop)
        {
        const DbResult sqliteStat = GetSqliteStatement().BindInt64(GetSqlParamIndex(), value);
        if (sqliteStat != BE_SQLITE_OK)
            return LogSqliteError(sqliteStat, "ECSqlStatement::BindInt64");
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    if (!m_isNoop)
        {
        if (value != nullptr && value[0] == '0' && (value[1] =='x' || value[1] == 'X') && value[2] != '\0')
            {
            BentleyStatus stat = SUCCESS;
            uint64_t id = BeStringUtilities::ParseHex(value, &stat);
            if (SUCCESS != stat)
                return LogSqliteError(BE_SQLITE_MISMATCH, Utf8PrintfString("Binding Id failed. Could not parse the bound hexadecimal string '%s'.", value).c_str());

            return _BindInt64(id);
            }

        const DbResult sqliteStat = GetSqliteStatement().BindText(GetSqlParamIndex(), value, ToBeSQliteBindMakeCopy(makeCopy), byteCount);
        if (sqliteStat != BE_SQLITE_OK)
            return LogSqliteError(sqliteStat, Utf8PrintfString("Binding Id failed. Could not bind string value '%s' to the Id parameter.", value).c_str());
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& IdECSqlBinder::_BindStructMember(Utf8CP structMemberPropertyName)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to Id parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& IdECSqlBinder::_BindStructMember(ECN::ECPropertyId structMemberPropertyId)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to Id parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& IdECSqlBinder::_AddArrayElement()
    {
    LOG.error("Type mismatch. Cannot bind array to Id parameter.");
    return NoopECSqlBinder::Get();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
