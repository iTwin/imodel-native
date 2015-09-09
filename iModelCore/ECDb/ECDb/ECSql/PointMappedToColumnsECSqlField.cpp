/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PointMappedToColumnsECSqlField.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
PointMappedToColumnsECSqlField::PointMappedToColumnsECSqlField (ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo&& ecsqlColumnInfo, int xColumnIndex, int yColumnIndex, int zColumnIndex) 
    : ECSqlField (ecsqlStatement, move (ecsqlColumnInfo)), m_xColumnIndex (xColumnIndex), m_yColumnIndex (yColumnIndex), m_zColumnIndex (zColumnIndex)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool PointMappedToColumnsECSqlField::_IsNull () const 
    {
    ResetStatus ();

    /*
    Note: The method returns true if *any* of the columns that represent a point is NULL. 
    It's normally assumed that *if* one of the point columns are NULL, then *all* are NULL. If that's 
    not the case, the cause is likely a logical flaw in the code to insert/update. 
    The (arguable) design choice made here is to treat these cases as NULL values. It seems the more 
    conservative choice *for use cases* where we don't want to propagate the corrupted data. 
    It's obviously the less conservative choice for use cases where the user would like to know about 
    corrupted data sooner than later. 
    */
    if (GetSqliteStatement ().IsColumnNull (m_xColumnIndex))
        return true;

    if (GetSqliteStatement ().IsColumnNull (m_yColumnIndex))
        return true;

    if (IsPoint3D ())
        return GetSqliteStatement ().IsColumnNull (m_zColumnIndex);

    return false;

    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d PointMappedToColumnsECSqlField::_GetPoint2D () const 
    {
    if (IsPoint3D ())
        {
        SetError (ECSqlStatus::UserError, "GetValuePoint2D cannot be called for Point3D column. Call GetPoint3D instead.");
        BeAssert (false && "GetValuePoint2D cannot be called for Point3D column. Call GetPoint3D instead.");
        return NoopECSqlValue::GetSingleton ().GetPoint2D ();
        }
    else
        ResetStatus ();


    return DPoint2d::From (GetSqliteStatement ().GetValueDouble (m_xColumnIndex), 
                           GetSqliteStatement ().GetValueDouble (m_yColumnIndex));
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d PointMappedToColumnsECSqlField::_GetPoint3D () const 
    {
    if (!IsPoint3D ())
        {
        SetError (ECSqlStatus::UserError, "GetValuePoint3D cannot be called for Point2D column. Call GetPoint2D instead.");
        BeAssert (false && "GetValuePoint3D cannot be called for Point2D column. Call GetPoint2D instead.");
        return NoopECSqlValue::GetSingleton ().GetPoint3D ();
        }
    else
        ResetStatus ();

    return DPoint3d::From (GetSqliteStatement ().GetValueDouble (m_xColumnIndex), 
                           GetSqliteStatement ().GetValueDouble (m_yColumnIndex),
                           GetSqliteStatement ().GetValueDouble (m_zColumnIndex));
    }


//****  no-op overrides ***

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void const* PointMappedToColumnsECSqlField::_GetBinary (int* binarySize) const
    {
    SetError (ECSqlStatus::UserError, "GetBinary cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    BeAssert (false && "GetBinary cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    return NoopECSqlValue::GetSingleton ().GetBinary (binarySize);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool PointMappedToColumnsECSqlField::_GetBoolean () const
    {
    SetError (ECSqlStatus::UserError, "GetBoolean cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    BeAssert (false && "GetBoolean cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    return NoopECSqlValue::GetSingleton ().GetBoolean ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
uint64_t PointMappedToColumnsECSqlField::_GetDateTimeJulianDaysHns (DateTime::Info& metadata) const
    {
    SetError (ECSqlStatus::UserError, "GetDateTime cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    BeAssert (false && "GetDateTime cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    return NoopECSqlValue::GetSingleton ().GetDateTimeJulianDaysHns (metadata);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
double PointMappedToColumnsECSqlField::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    {
    SetError(ECSqlStatus::UserError, "GetDateTime cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    BeAssert(false && "GetDateTime cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
double PointMappedToColumnsECSqlField::_GetDouble () const
    {
    SetError (ECSqlStatus::UserError, "GetDouble cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    BeAssert (false && "GetDouble cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    return NoopECSqlValue::GetSingleton ().GetDouble ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
int PointMappedToColumnsECSqlField::_GetInt () const
    {
    SetError (ECSqlStatus::UserError, "GetInt cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    BeAssert (false && "GetInt cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    return NoopECSqlValue::GetSingleton ().GetInt ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
int64_t PointMappedToColumnsECSqlField::_GetInt64 () const
    {
    SetError (ECSqlStatus::UserError, "GetInt64 cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    BeAssert (false && "GetInt64 cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    return NoopECSqlValue::GetSingleton ().GetInt64 ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8CP PointMappedToColumnsECSqlField::_GetText () const 
    {
    SetError (ECSqlStatus::UserError, "GetText cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    BeAssert (false && "GetText cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    return NoopECSqlValue::GetSingleton ().GetText ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2014
//+---------------+---------------+---------------+---------------+---------------+--------
IGeometryPtr PointMappedToColumnsECSqlField::_GetGeometry() const
    {
    SetError(ECSqlStatus::UserError, "GetGeometry cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    BeAssert(false && "GetGeometry cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    return NoopECSqlValue::GetSingleton().GetGeometry();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2014
//+---------------+---------------+---------------+---------------+---------------+--------
void const* PointMappedToColumnsECSqlField::_GetGeometryBlob (int* blobSize) const
    {
    SetError (ECSqlStatus::UserError, "GetGeometryBlob cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    BeAssert (false && "GetGeometryBlob cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    return NoopECSqlValue::GetSingleton ().GetGeometryBlob (blobSize);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                       03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlPrimitiveValue const& PointMappedToColumnsECSqlField::_GetPrimitive () const
    {
    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
IECSqlArrayValue const& PointMappedToColumnsECSqlField::_GetArray () const
    {
    SetError (ECSqlStatus::UserError, "GetArray cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    BeAssert (false && "GetArray cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    return NoopECSqlValue::GetSingleton ().GetArray ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
IECSqlStructValue const& PointMappedToColumnsECSqlField::_GetStruct () const
    {
    SetError (ECSqlStatus::UserError, "GetStruct cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    BeAssert (false && "GetStruct cannot be called for Point2D or Point3D column. Call GetPoint2D / GetPoint3D instead.");
    return NoopECSqlValue::GetSingleton ().GetStruct ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool PointMappedToColumnsECSqlField::IsPoint3D () const
    {
    return m_zColumnIndex >= 0;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
