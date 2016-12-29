/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PrimitiveToSingleColumnECSqlBinder.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "PrimitiveToSingleColumnECSqlBinder.h"
#include "ECSqlStatementBase.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
void PrimitiveToSingleColumnECSqlBinder::_SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteIndex)
    {
    m_sqliteIndex = (int) sqliteIndex;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveToSingleColumnECSqlBinder::_BindNull()
    {
    std::vector<IECSqlBinder*>* ehs = GetOnBindEventHandlers();
    if (ehs != nullptr)
        {
        for (IECSqlBinder* eh : *ehs)
            {
            const ECSqlStatus es = eh->BindNull();
            if (es != ECSqlStatus::Success)
                return es;
            }
        }

    const DbResult sqliteStat = GetSqliteStatementR().BindNull(m_sqliteIndex);
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindNull");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveToSingleColumnECSqlBinder::_BindBoolean(bool value)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_Boolean);
    if (!stat.IsSuccess())
        return stat;

    std::vector<IECSqlBinder*>* ehs = GetOnBindEventHandlers();
    if (ehs != nullptr)
        {
        for (IECSqlBinder* eh : *ehs)
            {
            ECSqlStatus es = eh->BindBoolean(value);
            if (es != ECSqlStatus::Success)
                return es;
            }
        }

    const auto sqliteStat = GetSqliteStatementR ().BindInt(m_sqliteIndex, value ? 1 : 0);
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindBoolean");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveToSingleColumnECSqlBinder::_BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_Binary);
    if (!stat.IsSuccess())
        return stat;

    std::vector<IECSqlBinder*>* ehs = GetOnBindEventHandlers();
    if (ehs != nullptr)
        {
        for (IECSqlBinder* eh : *ehs)
            {
            ECSqlStatus es = eh->BindBlob(value, binarySize, makeCopy);
            if (es != ECSqlStatus::Success)
                return es;
            }
        }

    const auto sqliteStat = GetSqliteStatementR ().BindBlob(m_sqliteIndex, value, binarySize, ToBeSQliteBindMakeCopy(makeCopy));
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindBlob");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveToSingleColumnECSqlBinder::_BindZeroBlob(int blobSize)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_Binary);
    if (!stat.IsSuccess())
        return stat;

    std::vector<IECSqlBinder*>* ehs = GetOnBindEventHandlers();
    if (ehs != nullptr)
        {
        for (IECSqlBinder* eh : *ehs)
            {
            ECSqlStatus es = eh->BindZeroBlob(blobSize);
            if (es != ECSqlStatus::Success)
                return es;
            }
        }

    const DbResult sqliteStat = GetSqliteStatementR().BindZeroBlob(m_sqliteIndex, blobSize);
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindZeroBlob");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveToSingleColumnECSqlBinder::_BindDateTime(uint64_t julianDayMsec, DateTime::Info const& metadata)
    {
    const double jd = DateTime::MsecToRationalDay(julianDayMsec);
    return _BindDateTime(jd, metadata);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveToSingleColumnECSqlBinder::_BindDateTime(double julianDay, DateTime::Info const& metadata)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_DateTime);
    if (!stat.IsSuccess())
        return stat;

    if (metadata.IsValid() && metadata.GetKind() == DateTime::Kind::Local)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECDb does not support to bind local date times.");
        return ECSqlStatus::Error;
        }

    ECSqlTypeInfo const& parameterTypeInfo = GetTypeInfo();
    if (!parameterTypeInfo.DateTimeInfoMatches(metadata))
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Metadata of DateTime value to bind doesn't match the metadata on the corresponding ECProperty.");
        return ECSqlStatus::Error;
        }
    
    std::vector<IECSqlBinder*>* ehs = GetOnBindEventHandlers();
    if (ehs != nullptr)
        {
        for (IECSqlBinder* eh : *ehs)
            {
            auto es = eh->BindDateTime(julianDay, metadata);
            if (es != ECSqlStatus::Success)
                return es;
            }
        }

    const DbResult sqliteStat = GetSqliteStatementR().BindDouble(m_sqliteIndex, julianDay);
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindDateTime");

    return ECSqlStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveToSingleColumnECSqlBinder::_BindDouble(double value)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_Double);
    if (!stat.IsSuccess())
        return stat;

    std::vector<IECSqlBinder*>* ehs = GetOnBindEventHandlers();
    if (ehs != nullptr)
        {
        for (IECSqlBinder* eh : *ehs)
            {
            ECSqlStatus es = eh->BindDouble(value);
            if (es != ECSqlStatus::Success)
                return es;
            }
        }

    const auto sqliteStat = GetSqliteStatementR ().BindDouble(m_sqliteIndex, value);
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindDouble");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveToSingleColumnECSqlBinder::_BindGeometryBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_IGeometry);
    if (!stat.IsSuccess())
        return stat;

    std::vector<IECSqlBinder*>* ehs = GetOnBindEventHandlers();
    if (ehs != nullptr)
        {
        for (IECSqlBinder* eh : *ehs)
            {
            ECSqlStatus es = eh->BindGeometryBlob(value, blobSize, makeCopy);
            if (es != ECSqlStatus::Success)
                return es;
            }
        }

    const auto sqliteStat = GetSqliteStatementR ().BindBlob(m_sqliteIndex, value, blobSize, ToBeSQliteBindMakeCopy(makeCopy));
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindGeometry");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveToSingleColumnECSqlBinder::_BindInt(int value)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_Integer);
    if (!stat.IsSuccess())
        return stat;

    std::vector<IECSqlBinder*>* ehs = GetOnBindEventHandlers();
    if (ehs != nullptr)
        {
        for (IECSqlBinder* eh : *ehs)
            {
            ECSqlStatus es = eh->BindInt(value);
            if (es != ECSqlStatus::Success)
                return es;
            }
        }

    const auto sqliteStat = GetSqliteStatementR ().BindInt(m_sqliteIndex, value);
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindInt");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveToSingleColumnECSqlBinder::_BindInt64(int64_t value)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_Long);
    if (!stat.IsSuccess())
        return stat;

    std::vector<IECSqlBinder*>* ehs = GetOnBindEventHandlers();
    if (ehs != nullptr)
        {
        for (IECSqlBinder* eh : *ehs)
            {
            const ECSqlStatus es = eh->BindInt64(value);
            if (es != ECSqlStatus::Success)
                return es;
            }
        }

    const DbResult sqliteStat = GetSqliteStatementR ().BindInt64(m_sqliteIndex, value);
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindInt64");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveToSingleColumnECSqlBinder::_BindPoint2d (DPoint2dCR value)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Point2d value can only be bound to parameter of same type.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveToSingleColumnECSqlBinder::_BindPoint3d (DPoint3dCR value)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Point3d value can only be bound to parameter of same type.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveToSingleColumnECSqlBinder::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    const ECSqlStatus stat = CanBind(PRIMITIVETYPE_String);
    if (!stat.IsSuccess())
        return stat;

    std::vector<IECSqlBinder*>* ehs = GetOnBindEventHandlers();
    if (ehs != nullptr)
        {
        for (IECSqlBinder* eh : *ehs)
            {
            ECSqlStatus es = eh->BindText(value, makeCopy, byteCount);
            if (es != ECSqlStatus::Success)
                return es;
            }
        }

    const auto sqliteStat = GetSqliteStatementR ().BindText(m_sqliteIndex, value, ToBeSQliteBindMakeCopy(makeCopy), byteCount);
    if (sqliteStat != BE_SQLITE_OK)
        return ReportError(sqliteStat, "ECSqlStatement::BindText");

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveBinder& PrimitiveToSingleColumnECSqlBinder::_BindPrimitive()
    {
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlStructBinder& PrimitiveToSingleColumnECSqlBinder::_BindStruct()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind ECStruct to primitive type parameter.");
    return NoopECSqlBinder::Get().BindStruct();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& PrimitiveToSingleColumnECSqlBinder::_BindArray(uint32_t initialCapacity)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind array to primitive parameter.");
    return NoopECSqlBinder::Get().BindArray(initialCapacity);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveToSingleColumnECSqlBinder::CanBind(ECN::PrimitiveType requestedType) const
    {
    //For DateTimes and Geometry column type and BindXXX type must match. All other types are implicitly
    //converted to each other by SQLite.
    const PrimitiveType fieldDataType = GetTypeInfo().GetPrimitiveType();
    switch (fieldDataType)
        {
            case PRIMITIVETYPE_DateTime:
                {
                if (requestedType != fieldDataType)
                    {
                    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch: only BindDateTime can be called for a column of the DateTime type.");
                    return ECSqlStatus::Error;
                    }
                else
                    break;
                }
            case PRIMITIVETYPE_IGeometry:
                {
                if (requestedType != fieldDataType)
                    {
                    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch: only BindGeometry can be called for a column of the IGeometry type.");
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
                    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch: BindDateTime cannot be called for a column which is not of the DateTime type.");
                    return ECSqlStatus::Error;
                    }
                else
                    break;
                }
            case PRIMITIVETYPE_IGeometry:
                {
                if (requestedType != fieldDataType)
                    {
                    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch: BindGeometry cannot be called for a column which is not of the IGeometry type.");
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