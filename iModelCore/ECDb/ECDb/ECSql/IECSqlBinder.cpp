/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/IECSqlBinder.h>
#include <GeomSerialization/GeomSerializationApi.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*********************** IECSqlBinder ********************************
//---------------------------------------------------------------------------------------
//not inlined to prevent being called outside ECDb
// @bsimethod                                                Krischan.Eberle      02/2017
//---------------------------------------------------------------------------------------
IECSqlBinder::IECSqlBinder() {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindNull() { return _BindNull(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy) { return _BindBlob(value, blobSize, makeCopy); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2016
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindZeroBlob(int blobSize) { return _BindZeroBlob(blobSize); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindBoolean(bool value) { return _BindBoolean(value); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindDateTime(DateTimeCR value)
    {
    uint64_t jd = INT64_C(0);
    if (SUCCESS != value.ToJulianDay(jd))
        {
        BeAssert(false && "ECSqlStatement::BindDateTime> Could not convert DateTime into Julian Day.");
        return ECSqlStatus::Error;
        }

    return BindDateTime(jd, value.GetInfo());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindDateTime(uint64_t julianDayMsec, DateTime::Info const& metadata) { return _BindDateTime(julianDayMsec, metadata); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindDateTime(double julianDay, DateTime::Info const& metadata) { return _BindDateTime(julianDay, metadata); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindDouble(double value) { return _BindDouble(value); }

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
        return ECSqlStatus::Error;
        }

    return BindBlob(geometryBlob.data(), (int) geometryBlob.size(), MakeCopy::Yes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindInt(int value) { return _BindInt(value); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2018
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindEnum(ECN::ECEnumeratorCR value) 
    {
    if (value.IsInteger())
        return BindInt(value.GetInteger());

    if (value.IsString())
        return BindText(value.GetString().c_str(), IECSqlBinder::MakeCopy::No);

    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindInt64(int64_t value) { return _BindInt64(value); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindPoint2d(DPoint2dCR value) { return _BindPoint2d(value); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindPoint3d(DPoint3dCR value) { return _BindPoint3d(value); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount) { return _BindText(value, makeCopy, byteCount); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindNavigation(BeInt64Id relatedInstanceId, ECN::ECClassId relationshipECClassId)
    {
    IECSqlBinder& idBinder = _BindStructMember(ECDBSYS_PROP_NavPropId);
    ECSqlStatus stat = idBinder.BindId(relatedInstanceId);
    if (stat != ECSqlStatus::Success)
        return stat;

    if (!relationshipECClassId.IsValid())
        return ECSqlStatus::Success;

    IECSqlBinder& relClassIdBinder = _BindStructMember(ECDBSYS_PROP_NavPropRelECClassId);
    return relClassIdBinder.BindId(relationshipECClassId);
    }

// --------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
IECSqlBinder& IECSqlBinder::operator[](Utf8CP structMemberPropertyName) { return _BindStructMember(structMemberPropertyName); }

// --------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
IECSqlBinder& IECSqlBinder::operator[](ECN::ECPropertyId structMemberPropertyId) { return _BindStructMember(structMemberPropertyId); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& IECSqlBinder::AddArrayElement() { return _AddArrayElement(); }

END_BENTLEY_SQLITE_EC_NAMESPACE