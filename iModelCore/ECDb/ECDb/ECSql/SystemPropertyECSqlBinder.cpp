/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/SystemPropertyECSqlBinder.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SystemPropertyECSqlBinder.h"
#include "ECSqlStatementBase.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
IdECSqlBinder::IdECSqlBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& typeInfo, bool isNoop) 
    : ECSqlBinder(ecsqlStatement, typeInfo, isNoop ? 0 : 1, true, true), m_sqliteIndex(-1), m_isNoop(isNoop), m_isNull(true)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_OnBeforeStep()
    {
    if (!m_isNoop && m_isNull)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Constraint violation. No value bound to Id parameter.");
        return ECSqlStatus::Error;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
void IdECSqlBinder::_SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteIndex)
    {
    BeAssert(!m_isNoop);
    m_sqliteIndex = (int) sqliteIndex;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindNull()
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

    if (!m_isNoop)
        {
        const auto sqliteStat = GetSqliteStatementR().BindNull(m_sqliteIndex);
        if (sqliteStat != BE_SQLITE_OK)
            return ReportError(sqliteStat, "ECSqlStatement::BindNull against id property.");
        }

    m_isNull = true;
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindPoint2d(DPoint2dCR value)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind Point2d value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindPoint3d(DPoint3dCR value)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind Point3d value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindBoolean(bool value)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind boolean value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindBinary(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind Blob value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindDateTime(double julianDay, DateTime::Info const* metadata)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind DateTime value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindDateTime(uint64_t julianDayHns, DateTime::Info const* metadata)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind DateTime value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindDouble(double value)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind double value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindGeometryBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind IGeometry value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindInt(int value)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind 32 bit integer value to Id parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindInt64(int64_t value)
    {
    std::vector<IECSqlBinder*>* ehs = GetOnBindEventHandlers();
    if (ehs != nullptr)
        {
        for (IECSqlBinder* eh : *ehs)
            {
            ECSqlStatus es = eh->BindInt64(value);
            if (es != ECSqlStatus::Success)
                return es;
            }
        }

    if (!m_isNoop)
        {
        BeAssert(m_sqliteIndex > 0);
        const DbResult sqliteStat = GetSqliteStatementR().BindInt64(m_sqliteIndex, value);
        if (sqliteStat != BE_SQLITE_OK)
            return ReportError(sqliteStat, "ECSqlStatement::BindInt64");
        }

    auto onBindEventHandler = GetOnBindECInstanceIdEventHandler();
    if (onBindEventHandler != nullptr)
        onBindEventHandler(ECInstanceId((uint64_t) value));

    m_isNull = false;
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IdECSqlBinder::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    std::vector<IECSqlBinder*>* ehs = GetOnBindEventHandlers();
    if (ehs != nullptr)
        {
        for (IECSqlBinder* eh : *ehs)
            {
            auto es = eh->BindText(value, makeCopy, byteCount);
            if (es != ECSqlStatus::Success)
                return es;
            }
        }

    if (!m_isNoop)
        {
        const auto sqliteStat = GetSqliteStatementR().BindText(m_sqliteIndex, value, ToBeSQliteBindMakeCopy(makeCopy), byteCount);
        if (sqliteStat != BE_SQLITE_OK)
            return ReportError(sqliteStat, "ECSqlStatement::BindText");
        }

    auto onBindEventHandler = GetOnBindECInstanceIdEventHandler();
    if (onBindEventHandler != nullptr)
        {
        ECInstanceId id;
        if (SUCCESS != ECInstanceId::FromString(id, value))
            {
            GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Binding string value to Id parameter failed. Value cannot be converted to an ECInstanceId.");
            return ECSqlStatus::Error;
            }

        onBindEventHandler(id);
        }

    m_isNull = false;
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlStructBinder& IdECSqlBinder::_BindStruct()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind ECStruct to Id parameter.");
    return NoopECSqlBinder::Get().BindStruct();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& IdECSqlBinder::_BindArray(uint32_t initialCapacity)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind array to Id parameter.");
    return NoopECSqlBinder::Get().BindArray(initialCapacity);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
