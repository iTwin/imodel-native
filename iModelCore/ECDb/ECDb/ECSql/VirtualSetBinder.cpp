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
VirtualSetBinder::VirtualSetBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo, SqlParamNameGenerator& paramNameGen)
    : ECSqlBinder(ctx, typeInfo, paramNameGen, 1, false, false)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus VirtualSetBinder::_BindNull()
    {
    const DbResult sqliteStat = GetSqliteStatement().BindNull(GetSqlParameterIndex());
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindNull");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus VirtualSetBinder::_BindBoolean(bool value)
    {
    LOG.error("Type mismatch. Cannot bind Boolean value to IdSet parameter.");
    return NoopECSqlBinder::Get().BindBoolean(value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus VirtualSetBinder::_BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    LOG.error("Type mismatch. Cannot bind Blob value to IdSet parameter.");
    return NoopECSqlBinder::Get().BindBlob(value, binarySize, makeCopy);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus VirtualSetBinder::_BindZeroBlob(int blobSize)
    {
    LOG.error("Type mismatch. Cannot bind ZeroBlob value to IdSet parameter.");
    return NoopECSqlBinder::Get().BindZeroBlob(blobSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus VirtualSetBinder::_BindDateTime(uint64_t julianDayMsec, DateTime::Info const& metadata)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to IdSet parameter.");
    return NoopECSqlBinder::Get().BindDateTime(julianDayMsec, metadata);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus VirtualSetBinder::_BindDateTime(double julianDay, DateTime::Info const& metadata)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to IdSet parameter.");
    return NoopECSqlBinder::Get().BindDateTime(julianDay, metadata);
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus VirtualSetBinder::_BindDouble(double value)
    {
    LOG.error("Type mismatch. Cannot bind Double value to IdSet parameter.");
    return NoopECSqlBinder::Get().BindDouble(value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus VirtualSetBinder::_BindInt(int value)
    {
    LOG.error("Type mismatch. Cannot bind Int value to IdSet parameter.");
    return NoopECSqlBinder::Get().BindInt(value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus VirtualSetBinder::_BindInt64(int64_t value)
    {
    LOG.error("Type mismatch. Cannot bind Int64 value to IdSet parameter.");
    return NoopECSqlBinder::Get().BindInt64(value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus VirtualSetBinder::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    LOG.error("Type mismatch. Cannot bind Text value to IdSet parameter.");
    return NoopECSqlBinder::Get().BindText(value, makeCopy, byteCount);
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus VirtualSetBinder::_BindPoint2d(DPoint2dCR value)
    {
    LOG.error("Type mismatch. Point2d value can only be bound to parameter of same type.");
    return NoopECSqlBinder::Get().BindPoint2d(value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus VirtualSetBinder::_BindPoint3d(DPoint3dCR value)
    {
    LOG.error("Type mismatch. Point3d value can only be bound to parameter of same type.");
    return NoopECSqlBinder::Get().BindPoint3d(value);
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& VirtualSetBinder::_BindStructMember(Utf8CP structMemberPropertyName)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to IdSet parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& VirtualSetBinder::_BindStructMember(ECN::ECPropertyId structMemberPropertyId)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to IdSet parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& VirtualSetBinder::_AddArrayElement()
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to IdSet parameter.");
    return NoopECSqlBinder::Get();
    }

// --------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus VirtualSetBinder::_BindVirtualSet(std::shared_ptr<VirtualSet> virtualSet)
    {
    const DbResult sqliteStat = GetSqliteStatement().BindInt64(GetSqlParameterIndex(), (int64_t) virtualSet.get());
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, Utf8PrintfString("Failed to bind Int64 value %" PRIi64 " to Id parameter.", virtualSet.get()).c_str());

    m_virtualSet = virtualSet;
    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE