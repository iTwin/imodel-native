/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PointMappedToColumnsECSqlField.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
PointMappedToColumnsECSqlField::PointMappedToColumnsECSqlField(ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo const& ecsqlColumnInfo, int xColumnIndex, int yColumnIndex, int zColumnIndex)
    : ECSqlField(ecsqlStatement, ecsqlColumnInfo, false, false), m_xColumnIndex(xColumnIndex), m_yColumnIndex(yColumnIndex), m_zColumnIndex(zColumnIndex)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool PointMappedToColumnsECSqlField::_IsNull() const
    {
    //To be consistent with the ECSQL parser which translates "MyPoint IS NULL" to "MyPoint.X IS NULL *AND* MyPoint.Y IS NULL"
    //this method uses the same semantics.
    return GetSqliteStatement().IsColumnNull(m_xColumnIndex) &&
        GetSqliteStatement().IsColumnNull(m_yColumnIndex) &&
        (!IsPoint3d() || GetSqliteStatement().IsColumnNull(m_zColumnIndex));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d PointMappedToColumnsECSqlField::_GetPoint2d() const
    {
    if (IsPoint3d())
        {
        ReportError(ECSqlStatus::Error, "GetValuePoint2d cannot be called for Point3d column. Call GetValuePoint3d instead.");
        BeAssert(false && "GetValuePoint2d cannot be called for Point3d column. Call GetValuePoint3d instead.");
        return NoopECSqlValue::GetSingleton().GetPoint2d();
        }

    return DPoint2d::From(GetSqliteStatement().GetValueDouble(m_xColumnIndex),
                          GetSqliteStatement().GetValueDouble(m_yColumnIndex));
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d PointMappedToColumnsECSqlField::_GetPoint3d() const
    {
    if (!IsPoint3d())
        {
        ReportError(ECSqlStatus::Error, "GetValuePoint3d cannot be called for Point2d column. Call GetPoint2d instead.");
        BeAssert(false && "GetValuePoint3d cannot be called for Point2d column. Call GetPoint2d instead.");
        return NoopECSqlValue::GetSingleton().GetPoint3d();
        }

    return DPoint3d::From(GetSqliteStatement().GetValueDouble(m_xColumnIndex),
                          GetSqliteStatement().GetValueDouble(m_yColumnIndex),
                          GetSqliteStatement().GetValueDouble(m_zColumnIndex));
    }


//****  no-op overrides ***

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void const* PointMappedToColumnsECSqlField::_GetBlob(int* blobSize) const
    {
    ReportError(ECSqlStatus::Error, "GetBlob cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    BeAssert(false && "GetBlob cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetBlob(blobSize);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool PointMappedToColumnsECSqlField::_GetBoolean() const
    {
    ReportError(ECSqlStatus::Error, "GetBoolean cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    BeAssert(false && "GetBoolean cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetBoolean();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
uint64_t PointMappedToColumnsECSqlField::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const
    {
    ReportError(ECSqlStatus::Error, "GetDateTime cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    BeAssert(false && "GetDateTime cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
double PointMappedToColumnsECSqlField::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    {
    ReportError(ECSqlStatus::Error, "GetDateTime cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    BeAssert(false && "GetDateTime cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
double PointMappedToColumnsECSqlField::_GetDouble() const
    {
    ReportError(ECSqlStatus::Error, "GetDouble cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    BeAssert(false && "GetDouble cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetDouble();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
int PointMappedToColumnsECSqlField::_GetInt() const
    {
    ReportError(ECSqlStatus::Error, "GetInt cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    BeAssert(false && "GetInt cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetInt();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
int64_t PointMappedToColumnsECSqlField::_GetInt64() const
    {
    ReportError(ECSqlStatus::Error, "GetInt64 cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    BeAssert(false && "GetInt64 cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetInt64();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8CP PointMappedToColumnsECSqlField::_GetText() const
    {
    ReportError(ECSqlStatus::Error, "GetText cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    BeAssert(false && "GetText cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetText();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2014
//+---------------+---------------+---------------+---------------+---------------+--------
IGeometryPtr PointMappedToColumnsECSqlField::_GetGeometry() const
    {
    ReportError(ECSqlStatus::Error, "GetGeometry cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    BeAssert(false && "GetGeometry cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetGeometry();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                       03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlPrimitiveValue const& PointMappedToColumnsECSqlField::_GetPrimitive() const
    {
    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
IECSqlArrayValue const& PointMappedToColumnsECSqlField::_GetArray() const
    {
    ReportError(ECSqlStatus::Error, "GetArray cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    BeAssert(false && "GetArray cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetArray();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
IECSqlStructValue const& PointMappedToColumnsECSqlField::_GetStruct() const
    {
    ReportError(ECSqlStatus::Error, "GetStruct cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    BeAssert(false && "GetStruct cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetStruct();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
