/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PrimitiveMappedToSingleColumnECSqlField.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <GeomSerialization/GeomSerializationApi.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//********************** PrimitiveMappedToSingleColumnECSqlField ********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
PrimitiveMappedToSingleColumnECSqlField::PrimitiveMappedToSingleColumnECSqlField(ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo const& ecsqlColumnInfo, int columnIndex)
    : ECSqlField(ecsqlStatement, ecsqlColumnInfo, false, false), m_sqliteColumnIndex(columnIndex)
    {
    if (m_ecsqlColumnInfo.GetDataType().GetPrimitiveType() == PRIMITIVETYPE_DateTime)
        {
        ECPropertyCP property = m_ecsqlColumnInfo.GetProperty();
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
bool PrimitiveMappedToSingleColumnECSqlField::_IsNull() const
    {
    return GetSqliteStatement().IsColumnNull(m_sqliteColumnIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
void const* PrimitiveMappedToSingleColumnECSqlField::_GetBinary(int* binarySize) const
    {
    if (binarySize != nullptr)
        *binarySize = GetSqliteStatement().GetColumnBytes(m_sqliteColumnIndex);

    return GetSqliteStatement().GetValueBlob(m_sqliteColumnIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool PrimitiveMappedToSingleColumnECSqlField::_GetBoolean() const
    {
    return GetSqliteStatement().GetValueInt(m_sqliteColumnIndex) != 0;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
double PrimitiveMappedToSingleColumnECSqlField::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    {
    const double jd = GetSqliteStatement().GetValueDouble(m_sqliteColumnIndex);
    metadata = m_datetimeMetadata;
    return jd;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t PrimitiveMappedToSingleColumnECSqlField::_GetDateTimeJulianDaysHns(DateTime::Info& metadata) const
    {
    const double jd = _GetDateTimeJulianDays(metadata);
    return DateTime::RationalDayToHns(jd);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
double PrimitiveMappedToSingleColumnECSqlField::_GetDouble() const
    {
    return GetSqliteStatement().GetValueDouble(m_sqliteColumnIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
int PrimitiveMappedToSingleColumnECSqlField::_GetInt() const
    {
    return GetSqliteStatement().GetValueInt(m_sqliteColumnIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
int64_t PrimitiveMappedToSingleColumnECSqlField::_GetInt64() const
    {
    return GetSqliteStatement().GetValueInt64(m_sqliteColumnIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP PrimitiveMappedToSingleColumnECSqlField::_GetText() const
    {
    return GetSqliteStatement().GetValueText(m_sqliteColumnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
IGeometryPtr PrimitiveMappedToSingleColumnECSqlField::_GetGeometry() const
    {
    int blobSize = -1;
    Byte const* blob = static_cast<Byte const*> (_GetGeometryBlob(&blobSize));
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
void const* PrimitiveMappedToSingleColumnECSqlField::_GetGeometryBlob(int* blobSize) const
    {
    if (blobSize != nullptr)
        *blobSize = GetSqliteStatement().GetColumnBytes(m_sqliteColumnIndex);

    return GetSqliteStatement().GetValueBlob(m_sqliteColumnIndex);
    }

//**** No-op implementations

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d PrimitiveMappedToSingleColumnECSqlField::_GetPoint2d() const
    {
    ReportError(ECSqlStatus::Error, "GetPoint2d cannot be called for columns which are not of the Point2d type.");
    BeAssert(false && "GetPoint2d cannot be called for columns which are not of the Point2d type.");
    return NoopECSqlValue::GetSingleton().GetPoint2d();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d PrimitiveMappedToSingleColumnECSqlField::_GetPoint3d() const
    {
    ReportError(ECSqlStatus::Error, "GetPoint3d cannot be called for columns which are not of the Point3d type.");
    BeAssert(false && "GetPoint3d cannot be called for columns which are not of the Point3d type.");
    return NoopECSqlValue::GetSingleton().GetPoint3d();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue const& PrimitiveMappedToSingleColumnECSqlField::_GetArray() const
    {
    ReportError(ECSqlStatus::Error, "GetArray cannot be called for primitive columns.");
    BeAssert(false && "GetArray cannot be called for primitive columns.");
    return NoopECSqlValue::GetSingleton().GetArray();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlStructValue const& PrimitiveMappedToSingleColumnECSqlField::_GetStruct() const
    {
    ReportError(ECSqlStatus::Error, "GetStruct cannot be called for primitive columns.");
    BeAssert(false && "GetStruct cannot be called for primitive columns.");
    return NoopECSqlValue::GetSingleton().GetStruct();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
