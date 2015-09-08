/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/IECSqlBinder.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/IECSqlBinder.h>
#include <GeomSerialization/GeomSerializationApi.h>
#include "IECSqlPrimitiveBinder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*********************** IECSqlBinder ********************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindNull()
    {
    return _BindNull();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindBinary(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    return _BindPrimitive()._BindBinary(value, binarySize, makeCopy);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindBoolean(bool value)
    {
    return _BindPrimitive()._BindBoolean(value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindDateTime(DateTimeCR value)
    {
    uint64_t jd = 0ULL;
    if (SUCCESS != value.ToJulianDay(jd))
        {
        BeAssert(false && "ECSqlStatement::BindDateTime> Could not convert DateTime into Julian Day.");
        return ECSqlStatus::ProgrammerError;
        }

    return BindDateTime(jd, &value.GetInfo());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindDateTime(uint64_t julianDayHns, DateTime::Info const* metadata)
    {
    return _BindPrimitive()._BindDateTime(julianDayHns, metadata);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindDateTime(double julianDay, DateTime::Info const* metadata)
    {
    return _BindPrimitive()._BindDateTime(julianDay, metadata);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindDouble(double value)
    {
    return _BindPrimitive()._BindDouble(value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindGeometry(IGeometryCR value)
    {
    bvector<Byte> geometryBlob;
    BentleyGeometryFlatBuffer::GeometryToBytes(value, geometryBlob);
    if (geometryBlob.empty())
        {
        LOG.error("Failed to serialize IGeometry to BentleyGeometryFlatBuffer.");
        BeAssert(false && "Failed to serialize IGeometry to BentleyGeometryFlatBuffer.");
        return ECSqlStatus::ProgrammerError;
        }

    return BindGeometryBlob(geometryBlob.data(), (int) geometryBlob.size(), MakeCopy::Yes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindGeometryBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy)
    {
    return _BindPrimitive()._BindGeometryBlob(value, blobSize, makeCopy);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindInt(int value)
    {
    return _BindPrimitive()._BindInt(value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindInt64(int64_t value)
    {
    return _BindPrimitive()._BindInt64(value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindPoint2D (DPoint2dCR value)
    {
    return _BindPrimitive()._BindPoint2D (value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindPoint3D (DPoint3dCR value)
    {
    return _BindPrimitive()._BindPoint3D (value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    return _BindPrimitive()._BindText(value, makeCopy, byteCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlStructBinder& IECSqlBinder::BindStruct()
    {
    return _BindStruct();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& IECSqlBinder::BindArray(uint32_t initialCapacity)
    {
    return _BindArray(initialCapacity);
    }

//*********************** IECSqlStructBinder ********************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& IECSqlStructBinder::GetMember(Utf8CP structMemberPropertyName)
    {
    return _GetMember(structMemberPropertyName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& IECSqlStructBinder::GetMember(ECN::ECPropertyId structMemberPropertyId)
    {
    return _GetMember(structMemberPropertyId);
    }

//*********************** IECSqlArrayBinder ********************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& IECSqlArrayBinder::AddArrayElement()
    {
    return _AddArrayElement();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE