/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
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
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindPoint2d(DPoint2dCR value)
    {
    if (m_isPoint3d)
        {
        LOG.error("Type mismatch. Cannot bind Point2d value to Point3d parameter.");
        return ECSqlStatus::Error;
        }

    Statement& sqliteStmt = GetSqliteStatement();
    DbResult sqliteStat = sqliteStmt.BindDouble(GetCoordSqlParamIndex(Coordinate::X), value.x);
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindPoint2d.");

    sqliteStat = sqliteStmt.BindDouble(GetCoordSqlParamIndex(Coordinate::Y), value.y);
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindPoint2d.");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindPoint3d(DPoint3dCR value)
    {
    if (!m_isPoint3d)
        {
        LOG.error("Type mismatch. Cannot bind Point3d value to Point2d parameter.");
        return ECSqlStatus::Error;
        }

    Statement& sqliteStmt = GetSqliteStatement();
    DbResult sqliteStat = sqliteStmt.BindDouble(GetCoordSqlParamIndex(Coordinate::X), value.x);
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindPoint3d.");

    sqliteStat = sqliteStmt.BindDouble(GetCoordSqlParamIndex(Coordinate::Y), value.y);
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindPoint3d.");

    sqliteStat = sqliteStmt.BindDouble(GetCoordSqlParamIndex(Coordinate::Z), value.z);
    if (sqliteStat != BE_SQLITE_OK)
        return LogSqliteError(sqliteStat, "ECSqlStatement::BindPoint3d.");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindBoolean(bool value)
    {
    LOG.error("Type mismatch. Cannot bind boolean value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    LOG.error("Type mismatch. Cannot bind Blob value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindZeroBlob(int blobSize)
    {
    LOG.error("Type mismatch. Cannot bind Zeroblob value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2015
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindDateTime(uint64_t julianDayMsec, DateTime::Info const&)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindDateTime(double julianDay, DateTime::Info const&)
    {
    LOG.error("Type mismatch. Cannot bind DateTime value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindDouble(double value)
    {
    LOG.error("Type mismatch. Cannot bind double value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindInt(int value)
    {
    LOG.error("Type mismatch. Cannot bind integer value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindInt64(int64_t value)
    {
    LOG.error("Type mismatch. Cannot bind int64_t value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointECSqlBinder::_BindText(Utf8CP stringValue, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    LOG.error("Type mismatch. Cannot bind string value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& PointECSqlBinder::_BindStructMember(Utf8CP structMemberPropertyName)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to Point2d / Point3d parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& PointECSqlBinder::_BindStructMember(ECN::ECPropertyId structMemberPropertyId)
    {
    LOG.error("Type mismatch. Cannot bind ECStruct to Point2d / Point3d parameter.");
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& PointECSqlBinder::_AddArrayElement()
    {
    LOG.error("Type mismatch. Cannot bind array to Point2d / Point3d parameter.");
    return NoopECSqlBinder::Get();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
