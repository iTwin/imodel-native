/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/BackDoor.h"
#include <GeomSerialization/GeomSerializationApi.h>

namespace ECDbBackDoor = BentleyApi::BeSQLite::EC::Tests::BackDoor;

BEGIN_ECDBUNITTESTS_NAMESPACE

END_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  07/2014
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbBackDoor::ECObjects::ECValue::SetAllowsPointersIntoInstanceMemory (ECN::ECValueR value, bool allow)
    {
    value.SetAllowsPointersIntoInstanceMemory (allow);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  07/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECObjectsStatus ECDbBackDoor::ECObjects::ECSchemaReadContext::AddSchema(ECN::ECSchemaReadContext& context, ECN::ECSchemaR schema)
    {
    return context.AddSchema(schema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  07/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbBackDoor::ECObjects::ECValue::AllowsPointersIntoInstanceMemory (ECN::ECValueCR value)
    {
    return value.AllowsPointersIntoInstanceMemory ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  07/2014
//+---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr ECDbBackDoor::BentleyGeometryFlatBuffer::BytesToGeometry(Byte const* buffer)
    {
    return BentleyApi::BentleyGeometryFlatBuffer::BytesToGeometry(buffer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  07/2014
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbBackDoor::BentleyGeometryFlatBuffer::GeometryToBytes(bvector<Byte>& buffer, IGeometryCR geom)
    {
    BentleyApi::BentleyGeometryFlatBuffer::GeometryToBytes(geom, buffer);
    }
