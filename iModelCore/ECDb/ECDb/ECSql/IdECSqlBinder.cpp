/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IdECSqlBinder::IdECSqlBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo, bool isNoop, SqlParamNameGenerator& paramNameGen)
    : ECSqlBinder(ctx, typeInfo, paramNameGen, isNoop ? 0 : 1, false, false), m_isNoop(isNoop)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindPoint2d(DPoint2dCR value)
    {
    LOG.error("Type mismatch. Cannot bind Point2d value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindPoint3d(DPoint3dCR value)
    {
    LOG.error("Type mismatch. Cannot bind Point3d value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindBoolean(bool value)
    {
    LOG.error("Type mismatch. Cannot bind boolean value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy)
    {
    LOG.error("Type mismatch. Cannot bind Blob value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindZeroBlob(int blobSize)
    {
    LOG.error("Type mismatch. Cannot bind Zeroblob value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindDateTime(double julianDay, DateTime::Info const&)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindDateTime(uint64_t julianDayHns, DateTime::Info const&)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindDouble(double value)
    {
    LOG.error("Type mismatch. Cannot bind double value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindInt(int value)
    {
    if (!m_isNoop)
        {
        const DbResult sqliteStat = GetSqliteStatement().BindInt(GetSqlParamIndex(), value);
        if (sqliteStat != BE_SQLITE_OK)
            return LogSqliteError(sqliteStat, Utf8PrintfString("Failed to bind int value %d to Id parameter.", value).c_str());
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindInt64(int64_t value)
    {
    if (!m_isNoop)
        {
        const DbResult sqliteStat = GetSqliteStatement().BindInt64(GetSqlParamIndex(), value);
        if (sqliteStat != BE_SQLITE_OK)
            return LogSqliteError(sqliteStat, Utf8PrintfString("Failed to bind Int64 value %" PRIi64 " to Id parameter.", value).c_str());
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    if (!m_isNoop)
        {
        if (value != nullptr && value[0] == '0' && (value[1] =='x' || value[1] == 'X') && value[2] != '\0')
            {
            BentleyStatus stat = ERROR;
            uint64_t id = BeStringUtilities::ParseHex(value, &stat);
            if (SUCCESS != stat)
                {
                LOG.errorv("Type mismatch. Failed to bind Id value: Could not parse the bound hexadecimal string '%s'.", value);
                return ECSqlStatus::Error;
                }

            return _BindInt64(id);
            }

        const DbResult sqliteStat = GetSqliteStatement().BindText(GetSqlParamIndex(), value, ToBeSQliteBindMakeCopy(makeCopy), byteCount);
        if (sqliteStat != BE_SQLITE_OK)
            return LogSqliteError(sqliteStat, Utf8PrintfString("Could not bind string value '%s' to the Id parameter.", value).c_str());
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindIdSet(IdSet<BeInt64Id> const& idSet)
    {
    m_virtualCopy = idSet;
    return _BindInt64((int64_t) &m_virtualCopy);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& IdECSqlBinder::_BindStructMember(Utf8CP structMemberPropertyName)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to Id parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& IdECSqlBinder::_BindStructMember(ECN::ECPropertyId structMemberPropertyId)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to Id parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& IdECSqlBinder::_AddArrayElement()
    {
    LOG.error("Type mismatch. Cannot bind array to Id parameter.");
    return NoopECSqlBinder::Get();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
