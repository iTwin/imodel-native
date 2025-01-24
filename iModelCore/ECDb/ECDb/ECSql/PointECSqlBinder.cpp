/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindNull()
    {
    Statement& sqliteStmt = GetSqliteStatement();
    DbResult sqliteStat = sqliteStmt.BindNull(GetCoordSqlParamIndex(Coordinate::X));
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindNull against Point2d or Point3d property.");

    sqliteStat = sqliteStmt.BindNull(GetCoordSqlParamIndex(Coordinate::Y));
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindNull against Point2d or Point3d property.");

    if (m_isPoint3d)
        {
        sqliteStat = sqliteStmt.BindNull(GetCoordSqlParamIndex(Coordinate::Z));
        if (sqliteStat != BE_SQLITE_OK)
            return LogSqliteError(sqliteStat, "ECSqlStatement::BindNull against Point2d or Point3d property.");
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindPoint2d(DPoint2dCR value)
    {
    if (m_isPoint3d)
        {
        LOG.error("Type mismatch. Cannot bind Point2d value to Point3d parameter.");
        return ECSqlStatus::Error;
        }

    // Set all co-ordinate values to NULL if either of the co-ordinates are INF/NaN
    const auto setAllCoordsToNull = (std::isinf(value.x) || std::isnan(value.x) || std::isinf(value.y) || std::isnan(value.y));

    Statement& sqliteStmt = GetSqliteStatement();
    if (const auto sqliteStat = setAllCoordsToNull ? sqliteStmt.BindNull(GetCoordSqlParamIndex(Coordinate::X)) : sqliteStmt.BindDouble(GetCoordSqlParamIndex(Coordinate::X), value.x); sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindPoint2d.");

    if (const auto sqliteStat = setAllCoordsToNull ? sqliteStmt.BindNull(GetCoordSqlParamIndex(Coordinate::Y)) : sqliteStmt.BindDouble(GetCoordSqlParamIndex(Coordinate::Y), value.y); sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindPoint2d.");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindPoint3d(DPoint3dCR value)
    {
    if (!m_isPoint3d)
        {
        LOG.error("Type mismatch. Cannot bind Point3d value to Point2d parameter.");
        return ECSqlStatus::Error;
        }

    // Set all co-ordinate values to NULL if any of the co-ordinates are INF/NaN
    const auto setAllCoordsToNull = (std::isinf(value.x) || std::isnan(value.x) || std::isinf(value.y) || std::isnan(value.y) || std::isinf(value.z) || std::isnan(value.z));

    Statement& sqliteStmt = GetSqliteStatement();
    if (const auto sqliteStat = setAllCoordsToNull ? sqliteStmt.BindNull(GetCoordSqlParamIndex(Coordinate::X)) : sqliteStmt.BindDouble(GetCoordSqlParamIndex(Coordinate::X), value.x); sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindPoint3d.");

    if (const auto sqliteStat = setAllCoordsToNull ? sqliteStmt.BindNull(GetCoordSqlParamIndex(Coordinate::Y)) : sqliteStmt.BindDouble(GetCoordSqlParamIndex(Coordinate::Y), value.y); sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindPoint3d.");

    if (const auto sqliteStat = setAllCoordsToNull ? sqliteStmt.BindNull(GetCoordSqlParamIndex(Coordinate::Z)) : sqliteStmt.BindDouble(GetCoordSqlParamIndex(Coordinate::Z), value.z); sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindPoint3d.");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindBoolean(bool value)
    {
    LOG.error("Type mismatch. Cannot bind boolean value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    LOG.error("Type mismatch. Cannot bind Blob value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindZeroBlob(int blobSize)
    {
    LOG.error("Type mismatch. Cannot bind Zeroblob value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindDateTime(uint64_t julianDayMsec, DateTime::Info const&)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindDateTime(double julianDay, DateTime::Info const&)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindDouble(double value)
    {
    LOG.error("Type mismatch. Cannot bind double value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindInt(int value)
    {
    LOG.error("Type mismatch. Cannot bind integer value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindInt64(int64_t value)
    {
    LOG.error("Type mismatch. Cannot bind int64_t value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindText(Utf8CP stringValue, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    LOG.error("Type mismatch. Cannot bind string value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& PointECSqlBinder::_BindStructMember(Utf8CP structMemberPropertyName)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to Point2d / Point3d parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& PointECSqlBinder::_BindStructMember(ECN::ECPropertyId structMemberPropertyId)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to Point2d / Point3d parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& PointECSqlBinder::_AddArrayElement()
    {
    LOG.error("Type mismatch. Cannot bind array to Point2d / Point3d parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BinderInfo const& PointECSqlBinder::_GetBinderInfo()
    {
    return m_binderInfo;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
