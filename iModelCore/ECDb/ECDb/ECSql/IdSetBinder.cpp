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
IdSetBinder::IdSetBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo, SqlParamNameGenerator& paramNameGen)
    : ECSqlBinder(ctx, typeInfo, paramNameGen, 0, false, false)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdSetBinder::_BindNull()
    {
    const DbResult sqliteStat = GetSqliteStatement().BindNull(GetSqlParameterIndex());
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindNull");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdSetBinder::_BindBoolean(bool value)
    {
    LOG.error("Type mismatch. Cannot bind Boolean value to IdSet parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdSetBinder::_BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    LOG.error("Type mismatch. Cannot bind Blob value to IdSet parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdSetBinder::_BindZeroBlob(int blobSize)
    {
    LOG.error("Type mismatch. Cannot bind ZeroBlob value to IdSet parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdSetBinder::_BindDateTime(uint64_t julianDayMsec, DateTime::Info const& metadata)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to IdSet parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdSetBinder::_BindDateTime(double julianDay, DateTime::Info const& metadata)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to IdSet parameter.");
    return ECSqlStatus::Error;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdSetBinder::_BindDouble(double value)
    {
    LOG.error("Type mismatch. Cannot bind Double value to IdSet parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdSetBinder::_BindInt(int value)
    {
    LOG.error("Type mismatch. Cannot bind Int value to IdSet parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdSetBinder::_BindInt64(int64_t value)
    {
    LOG.error("Type mismatch. Cannot bind Int64 value to IdSet parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdSetBinder::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    LOG.error("Type mismatch. Cannot bind Text value to IdSet parameter.");
    return ECSqlStatus::Error;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdSetBinder::_BindPoint2d(DPoint2dCR value)
    {
    LOG.error("Type mismatch. Point2d value can only be bound to parameter of same type.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdSetBinder::_BindPoint3d(DPoint3dCR value)
    {
    LOG.error("Type mismatch. Point3d value can only be bound to parameter of same type.");
    return ECSqlStatus::Error;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& IdSetBinder::_BindStructMember(Utf8CP structMemberPropertyName)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to IdSet parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& IdSetBinder::_BindStructMember(ECN::ECPropertyId structMemberPropertyId)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to IdSet parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& IdSetBinder::_AddArrayElement()
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to IdSet parameter.");
    return NoopECSqlBinder::Get();
    }

// --------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IdSetBinder::_BindIdSet(IdSet<BeInt64Id> const& idSet)
    {
    m_virtualCopy = idSet;
    return _BindInt64((int64_t) &m_virtualCopy);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE