/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/BackDoor.h"
#include <GeomSerialization/GeomSerializationApi.h>

namespace ECDbBackDoor = BentleyApi::BeSQLite::EC::Tests::BackDoor;

BEGIN_ECDBUNITTESTS_NAMESPACE

END_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbBackDoor::ECObjects::ECValue::SetAllowsPointersIntoInstanceMemory (ECN::ECValueR value, bool allow)
    {
    value.SetAllowsPointersIntoInstanceMemory (allow);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECObjectsStatus ECDbBackDoor::ECObjects::ECSchemaReadContext::AddSchema(ECN::ECSchemaReadContext& context, ECN::ECSchemaR schema)
    {
    return context.AddSchema(schema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbBackDoor::ECObjects::ECValue::AllowsPointersIntoInstanceMemory (ECN::ECValueCR value)
    {
    return value.AllowsPointersIntoInstanceMemory ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr ECDbBackDoor::BentleyGeometryFlatBuffer::BytesToGeometry(Byte const* buffer)
    {
    return BentleyApi::BentleyGeometryFlatBuffer::BytesToGeometry(buffer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbBackDoor::BentleyGeometryFlatBuffer::GeometryToBytes(bvector<Byte>& buffer, IGeometryCR geom)
    {
    BentleyApi::BentleyGeometryFlatBuffer::GeometryToBytes(geom, buffer);
    }
