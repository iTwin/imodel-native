/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <GeomSerialization/GeomSerializationApi.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
PrimitiveECSqlField::PrimitiveECSqlField(ECSqlSelectPreparedStatement& stmt, ECSqlColumnInfo const& ecsqlColumnInfo, int columnIndex)
    : ECSqlField(stmt, ecsqlColumnInfo, false, false), m_sqliteColumnIndex(columnIndex)
    {
    if (m_ecsqlColumnInfo.GetDataType().GetPrimitiveType() == PRIMITIVETYPE_DateTime)
        {
        ECPropertyCP property = m_ecsqlColumnInfo.GetProperty();
        BeAssert(property != nullptr && "ColumnInfo::GetProperty can return null. Please double-check");
        if (CoreCustomAttributeHelper::GetDateTimeInfo(m_datetimeMetadata, *property) != ECObjectsStatus::Success)
            {
            LOG.error("Could not read DateTimeInfo custom attribute from the corresponding ECProperty.");
            BeAssert(false && "Could not read DateTimeInfo custom attribute from the corresponding ECProperty.");
            return;
            }

        if (!m_datetimeMetadata.IsValid())
            m_datetimeMetadata = DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified); //default
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
void const* PrimitiveECSqlField::_GetBlob(int* blobSize) const
    {
    if (blobSize != nullptr)
        *blobSize = GetSqliteStatement().GetColumnBytes(m_sqliteColumnIndex);

    return GetSqliteStatement().GetValueBlob(m_sqliteColumnIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
double PrimitiveECSqlField::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    {
    const double jd = GetSqliteStatement().GetValueDouble(m_sqliteColumnIndex);
    metadata = m_datetimeMetadata;
    return jd;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t PrimitiveECSqlField::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const
    {
    const double jd = _GetDateTimeJulianDays(metadata);
    return DateTime::RationalDayToMsec(jd);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
IGeometryPtr PrimitiveECSqlField::_GetGeometry() const
    {
    int blobSize = -1;
    Byte const* blob = static_cast<Byte const*> (_GetBlob(&blobSize));
    if (blob == nullptr)
        return nullptr;

    BeAssert(blobSize > 0);
    const size_t blobSizeU = (size_t) blobSize;
    bvector<Byte> byteVec;
    byteVec.reserve(blobSizeU);
    byteVec.assign(blob, blob + blobSizeU);
    if (!BentleyGeometryFlatBuffer::IsFlatBufferFormat(byteVec))
        {
        LOG.error("Cannot retrieve Geometry value. The geometry is not persisted in the Bentley Geometry FlatBuffer format.");
        return nullptr;
        }

    IGeometryPtr geom = BentleyGeometryFlatBuffer::BytesToGeometry(byteVec);
    BeAssert(geom != nullptr);
    return geom;
    }

//**** No-op implementations

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d PrimitiveECSqlField::_GetPoint2d() const
    {
    LOG.error("GetPoint2d cannot be called for columns which are not of the Point2d type.");
    return NoopECSqlValue::GetSingleton().GetPoint2d();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d PrimitiveECSqlField::_GetPoint3d() const
    {
    LOG.error("GetPoint3d cannot be called for columns which are not of the Point3d type.");
    return NoopECSqlValue::GetSingleton().GetPoint3d();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& PrimitiveECSqlField::_GetStructMemberValue(Utf8CP memberName) const
    {
    LOG.error("GetStructMemberValue cannot be called for primitive columns.");
    return NoopECSqlValue::GetSingleton()[memberName];
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& PrimitiveECSqlField::_GetStructIterable() const
    {
    LOG.error("GetStructIterable cannot be called for primitive columns.");
    return NoopECSqlValue::GetSingleton().GetStructIterable();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/2017
//+---------------+---------------+---------------+---------------+---------------+------
int PrimitiveECSqlField::_GetArrayLength() const
    {
    LOG.error("GetArrayLength cannot be called for primitive columns.");
    return NoopECSqlValue::GetSingleton().GetArrayLength();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& PrimitiveECSqlField::_GetArrayIterable() const
    {
    LOG.error("GetArrayIterable cannot be called for primitive columns.");
    return NoopECSqlValue::GetSingleton().GetArrayIterable();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
