/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PointECSqlBinder.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "PointECSqlBinder.h"
#include "ECSqlStatementBase.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
void PointToColumnsECSqlBinder::_SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex)
    {
    switch (ecsqlParameterComponentIndex)
        {
            case 0: m_xSqliteIndex = (int) sqliteParameterIndex; break;
            case 1: m_ySqliteIndex = (int) sqliteParameterIndex; break;
            case 2: m_zSqliteIndex = (int) sqliteParameterIndex; break;

            default:
                BeAssert(ecsqlParameterComponentIndex <= 2);
                break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointToColumnsECSqlBinder::_BindNull()
    {
    if (auto ehs = GetOnBindEventHandlers())
        {
        for (auto eh : *ehs)
            {
            auto es = eh->BindNull();
            if (es != ECSqlStatus::Success)
                return es;
            }
        }

    auto sqliteStat = GetSqliteStatementR ().BindNull(m_xSqliteIndex);
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindNull against Point2d or Point3d property.");

    sqliteStat = GetSqliteStatementR ().BindNull(m_ySqliteIndex);
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindNull against Point2d or Point3d property.");

    if (IsPoint3d ())
        {
        sqliteStat = GetSqliteStatementR ().BindNull(m_ySqliteIndex);
        if (sqliteStat != BE_SQLITE_OK)
            return ReportError(sqliteStat, "ECSqlStatement::BindNull against Point2d or Point3d property.");
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointToColumnsECSqlBinder::_BindPoint2d (DPoint2dCR value)
    {
    if (IsPoint3d())
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind Point2d value to Point3d parameter.");
        return ECSqlStatus::Error;
        }

    if (auto ehs = GetOnBindEventHandlers())
        {
        for (auto eh : *ehs)
            {
            auto es = eh->BindPoint2d(value);
            if (es != ECSqlStatus::Success)
                return es;
            }
        }

    auto sqliteStat = GetSqliteStatementR ().BindDouble(m_xSqliteIndex, value.x);
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindPoint2d.");

    sqliteStat = GetSqliteStatementR ().BindDouble(m_ySqliteIndex, value.y);
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindPoint2d.");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointToColumnsECSqlBinder::_BindPoint3d (DPoint3dCR value)
    {
    if (!IsPoint3d())
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind Point3d value to Point2d parameter.");
        return ECSqlStatus::Error;
        }

    if (auto ehs = GetOnBindEventHandlers())
        {
        for (auto eh : *ehs)
            {
            auto es = eh->BindPoint3d(value);
            if (es != ECSqlStatus::Success)
                return es;
            }
        }

    auto sqliteStat = GetSqliteStatementR ().BindDouble(m_xSqliteIndex, value.x);
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindPoint3d.");

    sqliteStat = GetSqliteStatementR ().BindDouble(m_ySqliteIndex, value.y);
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindPoint3d.");

    sqliteStat = GetSqliteStatementR ().BindDouble(m_zSqliteIndex, value.z);
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindPoint3d.");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointToColumnsECSqlBinder::_BindBoolean(bool value)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind boolean value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointToColumnsECSqlBinder::_BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind Blob value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PointToColumnsECSqlBinder::_BindZeroBlob(int blobSize)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind Zeroblob value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2015
//---------------------------------------------------------------------------------------
ECSqlStatus PointToColumnsECSqlBinder::_BindDateTime(uint64_t julianDayMsec, DateTime::Info const&)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind DateTime value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointToColumnsECSqlBinder::_BindDateTime(double julianDay, DateTime::Info const&)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind DateTime value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointToColumnsECSqlBinder::_BindDouble(double value)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind double value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PointToColumnsECSqlBinder::_BindGeometryBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind IGeometry value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointToColumnsECSqlBinder::_BindInt(int value)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind integer value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointToColumnsECSqlBinder::_BindInt64(int64_t value)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind int64_t value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PointToColumnsECSqlBinder::_BindText(Utf8CP stringValue, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind string value to Point2d / Point3d parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveBinder& PointToColumnsECSqlBinder::_BindPrimitive()
    {
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlStructBinder& PointToColumnsECSqlBinder::_BindStruct()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind ECStruct to Point2d / Point3d parameter.");
    return NoopECSqlBinder::Get().BindStruct();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& PointToColumnsECSqlBinder::_BindArray(uint32_t initialCapacity)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind array to Point2d / Point3d parameter.");
    return NoopECSqlBinder::Get().BindArray(initialCapacity);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
