/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <GeomSerialization/GeomSerializationApi.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr IECSqlValueHelper::GeometryFromBlob(void const* blob, int blobSize)
    {
    if (blob == nullptr)
        return nullptr;

    BeAssert(blobSize > 0);
    const size_t blobSizeU = (size_t) blobSize;
    bvector<Byte> byteVec;
    byteVec.reserve(blobSizeU);
    byteVec.assign(static_cast<Byte const*>(blob), static_cast<Byte const*>(blob) + blobSizeU);
    if (!BentleyGeometryFlatBuffer::IsFlatBufferFormat(byteVec))
        {
        LOG.error("Cannot retrieve Geometry value. The geometry is not persisted in the Bentley Geometry FlatBuffer format.");
        return nullptr;
        }

    IGeometryPtr geom = BentleyGeometryFlatBuffer::BytesToGeometry(byteVec);
    BeAssert(geom != nullptr);
    return geom;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& IECSqlValueHelper::GetNavMemberValue(Utf8CP memberName,
                                                        IECSqlValue const* id,
                                                        IECSqlValue const* relClassId)
    {
    if (BeStringUtilities::StricmpAscii(memberName, ECDBSYS_PROP_NavPropId) == 0)
        return id != nullptr ? *id : (IECSqlValue const&) NoopECSqlValue::GetSingleton();

    if (BeStringUtilities::StricmpAscii(memberName, ECDBSYS_PROP_NavPropRelECClassId) == 0)
        return relClassId != nullptr ? *relClassId : (IECSqlValue const&) NoopECSqlValue::GetSingleton();

    LOG.errorv("Member name '%s' passed to navigation property IECSqlValue[] does not exist.", memberName);
    return NoopECSqlValue::GetSingleton()[memberName];
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& IECSqlValueHelper::GetNavIterCurrentByStateIndex(uint8_t stateIndex,
                                                                     IECSqlValue const* id,
                                                                     IECSqlValue const* relClassId)
    {
    if (stateIndex == 1) // Id
        return id != nullptr ? *id : (IECSqlValue const&) NoopECSqlValue::GetSingleton();

    if (stateIndex == 2) // RelECClassId
        return relClassId != nullptr ? *relClassId : (IECSqlValue const&) NoopECSqlValue::GetSingleton();

    BeAssert(false && "GetNavIterCurrentByStateIndex called with an out-of-range state");
    return NoopECSqlValue::GetSingleton();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
