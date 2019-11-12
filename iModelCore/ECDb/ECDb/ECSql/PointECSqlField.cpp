/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool PointECSqlField::_IsNull() const
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
DPoint2d PointECSqlField::_GetPoint2d() const
    {
    if (IsPoint3d())
        {
        LOG.error("GetValuePoint2d cannot be called for Point3d column. Call GetValuePoint3d instead.");
        return NoopECSqlValue::GetSingleton().GetPoint2d();
        }

    return DPoint2d::From(GetSqliteStatement().GetValueDouble(m_xColumnIndex),
                          GetSqliteStatement().GetValueDouble(m_yColumnIndex));
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d PointECSqlField::_GetPoint3d() const
    {
    if (!IsPoint3d())
        {
        LOG.error("GetValuePoint3d cannot be called for Point2d column. Call GetPoint2d instead.");
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
void const* PointECSqlField::_GetBlob(int* blobSize) const
    {
    LOG.error("GetBlob cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetBlob(blobSize);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool PointECSqlField::_GetBoolean() const
    {
    LOG.error("GetBoolean cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetBoolean();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
uint64_t PointECSqlField::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const
    {
    LOG.error("GetDateTime cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
double PointECSqlField::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    {
    LOG.error("GetDateTime cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
double PointECSqlField::_GetDouble() const
    {
    LOG.error("GetDouble cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetDouble();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
int PointECSqlField::_GetInt() const
    {
    LOG.error("GetInt cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetInt();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
int64_t PointECSqlField::_GetInt64() const
    {
    LOG.error("GetInt64 cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetInt64();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8CP PointECSqlField::_GetText() const
    {
    LOG.error("GetText cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetText();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2014
//+---------------+---------------+---------------+---------------+---------------+--------
IGeometryPtr PointECSqlField::_GetGeometry() const
    {
    LOG.error("GetGeometry cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetGeometry();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& PointECSqlField::_GetStructMemberValue(Utf8CP memberName) const
    {
    LOG.error("GetStructMemberValue cannot be called for Point2d or Point3d columns.");
    return NoopECSqlValue::GetSingleton()[memberName];
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& PointECSqlField::_GetStructIterable() const
    {
    LOG.error("GetStructIterable cannot be called for Point2d or Point3d columns.");
    return NoopECSqlValue::GetSingleton().GetStructIterable();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/2017
//+---------------+---------------+---------------+---------------+---------------+------
int PointECSqlField::_GetArrayLength() const
    {
    LOG.error("GetArrayLength cannot be called for Point2d or Point3d columns.");
    return NoopECSqlValue::GetSingleton().GetArrayLength();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& PointECSqlField::_GetArrayIterable() const
    {
    LOG.error("GetArrayIterable cannot be called for Point2d or Point3d columns.");
    return NoopECSqlValue::GetSingleton().GetArrayIterable();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
