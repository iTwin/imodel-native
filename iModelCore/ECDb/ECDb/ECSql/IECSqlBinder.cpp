/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/IECSqlBinder.h>
#include <GeomSerialization/GeomSerializationApi.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*********************** IECSqlBinder ********************************
//---------------------------------------------------------------------------------------
//not inlined to prevent being called outside ECDb
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder::IECSqlBinder() {}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindNull() { return _BindNull(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy) { return _BindBlob(value, blobSize, makeCopy); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindZeroBlob(int blobSize) { return _BindZeroBlob(blobSize); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindBoolean(bool value) { return _BindBoolean(value); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindDateTime(DateTimeCR value)
    {
    uint64_t jd = 0;
    return SUCCESS != value.ToJulianDay(jd) ? ECSqlStatus::Error : BindDateTime(jd, value.GetInfo());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindDateTime(uint64_t julianDayMsec, DateTime::Info const& metadata) { return _BindDateTime(julianDayMsec, metadata); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindDateTime(double julianDay, DateTime::Info const& metadata) { return _BindDateTime(julianDay, metadata); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindDouble(double value) { return _BindDouble(value); }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindInt(int value) { return _BindInt(value); }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindInt64(int64_t value) { return _BindInt64(value); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindPoint2d(DPoint2dCR value) { return _BindPoint2d(value); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindPoint3d(DPoint3dCR value) { return _BindPoint3d(value); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount) { return _BindText(value, makeCopy, byteCount); }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus IECSqlBinder::BindIdSet(std::shared_ptr<VirtualSet> virtualSet) { return _BindIdSet(virtualSet); }

// --------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& IECSqlBinder::operator[](Utf8CP structMemberPropertyName) { return _BindStructMember(structMemberPropertyName); }

// --------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& IECSqlBinder::operator[](ECN::ECPropertyId structMemberPropertyId) { return _BindStructMember(structMemberPropertyId); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlBinder& IECSqlBinder::AddArrayElement() { return _AddArrayElement(); }

END_BENTLEY_SQLITE_EC_NAMESPACE