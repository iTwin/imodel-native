/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PrimitiveMappedToSingleColumnECSqlField.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <GeomSerialization/GeomSerializationApi.h>

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//********************** PrimitiveMappedToSingleColumnECSqlField ********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
PrimitiveMappedToSingleColumnECSqlField::PrimitiveMappedToSingleColumnECSqlField (ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo&& ecsqlColumnInfo, int columnIndex)
    : ECSqlField (ecsqlStatement, move (ecsqlColumnInfo), false, false), m_sqliteColumnIndex (columnIndex)
    {
    if (m_ecsqlColumnInfo.GetDataType().GetPrimitiveType() == PRIMITIVETYPE_DateTime)
        {
        auto property = m_ecsqlColumnInfo.GetProperty();
        BeAssert(property != nullptr && "ColumnInfo::GetProperty can return null. Please double-check");
        DateTimeInfo dateTimeInfo;
        if (StandardCustomAttributeHelper::GetDateTimeInfo(dateTimeInfo, *property) != ECObjectsStatus::Success)
            {
            ReportError(ECSqlStatus::Error, "Could not read DateTimeInfo custom attribute from the corresponding ECProperty.");
            BeAssert(false && "Could not read DateTimeInfo custom attribute from the corresponding ECProperty.");
            return;
            }

        m_datetimeMetadata = dateTimeInfo.GetInfo(true);
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool PrimitiveMappedToSingleColumnECSqlField::_IsNull () const
    {
    return GetSqliteStatement ().IsColumnNull (GetSqliteColumnIndex ());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
void const* PrimitiveMappedToSingleColumnECSqlField::_GetBinary (int* binarySize) const
    {
    if (!CanGetValue (PRIMITIVETYPE_Binary))
        return NoopECSqlValue::GetSingleton ().GetBinary (binarySize);
        
    const auto columnIndex = GetSqliteColumnIndex ();

    if (binarySize != nullptr)
        {
        *binarySize = GetSqliteStatement ().GetColumnBytes (columnIndex);
        }

    return GetSqliteStatement ().GetValueBlob (columnIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool PrimitiveMappedToSingleColumnECSqlField::_GetBoolean () const
    {
    if (!CanGetValue (PRIMITIVETYPE_Boolean))
        return NoopECSqlValue::GetSingleton ().GetBoolean ();

    return GetSqliteStatement ().GetValueInt (GetSqliteColumnIndex ()) != 0;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
double PrimitiveMappedToSingleColumnECSqlField::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    {
    if (!CanGetValue(PRIMITIVETYPE_DateTime))
        return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);

    const double jd = GetSqliteStatement().GetValueDouble(GetSqliteColumnIndex());
    metadata = m_datetimeMetadata;
    return jd;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t PrimitiveMappedToSingleColumnECSqlField::_GetDateTimeJulianDaysHns (DateTime::Info& metadata) const
    {
    const double jd = _GetDateTimeJulianDays(metadata);
    return DateTime::RationalDayToHns(jd);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
double PrimitiveMappedToSingleColumnECSqlField::_GetDouble () const
    {
    if (!CanGetValue (PRIMITIVETYPE_Double))
        return NoopECSqlValue::GetSingleton ().GetDouble ();

    return GetSqliteStatement ().GetValueDouble (GetSqliteColumnIndex ());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
int PrimitiveMappedToSingleColumnECSqlField::_GetInt () const
    {
    if (!CanGetValue (PRIMITIVETYPE_Integer))
        return NoopECSqlValue::GetSingleton ().GetInt ();

    return GetSqliteStatement ().GetValueInt (GetSqliteColumnIndex ());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
int64_t PrimitiveMappedToSingleColumnECSqlField::_GetInt64 () const
    {
    if (!CanGetValue (PRIMITIVETYPE_Long))
        return NoopECSqlValue::GetSingleton ().GetInt64 ();

    return GetSqliteStatement ().GetValueInt64 (GetSqliteColumnIndex ());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP PrimitiveMappedToSingleColumnECSqlField::_GetText () const 
    {
    if (!CanGetValue (PRIMITIVETYPE_String))
        return NoopECSqlValue::GetSingleton ().GetText ();

    return GetSqliteStatement ().GetValueText (GetSqliteColumnIndex ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
IGeometryPtr PrimitiveMappedToSingleColumnECSqlField::_GetGeometry() const
    {
    if (!CanGetValue(PRIMITIVETYPE_IGeometry))
        return NoopECSqlValue::GetSingleton().GetGeometry();

    int blobSize = -1;
    auto blob = static_cast<Byte const*> (_GetGeometryBlob(&blobSize));
    if (blob == nullptr)
        return nullptr;

    BeAssert(blobSize > 0);
    const size_t blobSizeU = (size_t) blobSize;
    bvector<Byte> byteVec;
    byteVec.reserve(blobSizeU);
    byteVec.assign(blob, blob + blobSizeU);
    if (!BentleyGeometryFlatBuffer::IsFlatBufferFormat(byteVec))
        {
        ReportError(ECSqlStatus::Error, "Cannot retrieve Geometry value. The geometry is not persisted in the Bentley Geometry FlatBuffer format.");
        return nullptr;
        }

    IGeometryPtr geom = BentleyGeometryFlatBuffer::BytesToGeometry(byteVec);
    BeAssert(geom != nullptr);
    return geom;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2014
//+---------------+---------------+---------------+---------------+---------------+--------
void const* PrimitiveMappedToSingleColumnECSqlField::_GetGeometryBlob (int* blobSize) const
    {
    if (!CanGetValue (PRIMITIVETYPE_IGeometry))
        return NoopECSqlValue::GetSingleton ().GetGeometryBlob(blobSize);

    const int columnIndex = GetSqliteColumnIndex ();
    if (blobSize != nullptr)
        *blobSize = GetSqliteStatement ().GetColumnBytes (columnIndex);

    return GetSqliteStatement ().GetValueBlob (columnIndex);
    }

//**** No-op implementations

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d PrimitiveMappedToSingleColumnECSqlField::_GetPoint2D () const
    {
    ReportError (ECSqlStatus::Error, "GetPoint2D cannot be called for columns which are not of the Point2D type.");
    BeAssert (false && "GetPoint2D cannot be called for columns which are not of the Point2D type.");
    return NoopECSqlValue::GetSingleton ().GetPoint2D ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d PrimitiveMappedToSingleColumnECSqlField::_GetPoint3D () const
    {
    ReportError (ECSqlStatus::Error, "GetPoint3D cannot be called for columns which are not of the Point3D type.");
    BeAssert (false && "GetPoint3D cannot be called for columns which are not of the Point3D type."); 
    return NoopECSqlValue::GetSingleton ().GetPoint3D ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                       03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlPrimitiveValue const& PrimitiveMappedToSingleColumnECSqlField::_GetPrimitive () const
    {
    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue const& PrimitiveMappedToSingleColumnECSqlField::_GetArray () const
    {
    ReportError (ECSqlStatus::Error, "GetArray cannot be called for primitive columns.");
    BeAssert (false && "GetArray cannot be called for primitive columns.");
    return NoopECSqlValue::GetSingleton ().GetArray ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlStructValue const& PrimitiveMappedToSingleColumnECSqlField::_GetStruct () const
    {
    ReportError (ECSqlStatus::Error, "GetStruct cannot be called for primitive columns.");
    BeAssert (false && "GetStruct cannot be called for primitive columns.");
    return NoopECSqlValue::GetSingleton ().GetStruct ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool PrimitiveMappedToSingleColumnECSqlField::CanGetValue (PrimitiveType requestedType) const
    {
    //For DateTimes and Geometry column type and GetValueXXX type must match. All other types are implicitly
    //converted to each other by SQLite.
    return CanGetValue (PRIMITIVETYPE_DateTime, requestedType, "DateTime", "GetValueDateTime") &&
        CanGetValue (PRIMITIVETYPE_IGeometry, requestedType, "IGeometry", "GetValueGeometry");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool PrimitiveMappedToSingleColumnECSqlField::CanGetValue (PrimitiveType testType, PrimitiveType requestedType, Utf8CP typeName, Utf8CP getValueMethodName) const
    {
    //For DateTimes and Geometry column type and GetValueXXX type must match. All other types are implicitly
    //converted to each other by SQLite.
    const auto fieldDataType = m_ecsqlColumnInfo.GetDataType().GetPrimitiveType();
    if (fieldDataType == testType && requestedType != testType)
        {
        Utf8String error;
        error.Sprintf ("Only %s can be called for a column of the %s type.", getValueMethodName, typeName);
        ReportError (ECSqlStatus::Error, error.c_str ());
        return false;
        }

    if (fieldDataType != testType && requestedType == testType)
        {
        Utf8String error;
        error.Sprintf ("%s cannot be called for a column which is not of the % type.", getValueMethodName, typeName);
        ReportError (ECSqlStatus::Error, error.c_str ());
        return false;
        }

    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
int PrimitiveMappedToSingleColumnECSqlField::GetSqliteColumnIndex() const 
    {
    return m_sqliteColumnIndex;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
