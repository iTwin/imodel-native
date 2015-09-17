/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/Backdoor.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/Backdoor.h"
#include <GeomSerialization/GeomSerializationApi.h>

namespace ECDbBackdoor = BentleyApi::BeSQLite::EC::Tests::Backdoor;

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  07/2013
//+---------------+---------------+---------------+---------------+---------------+------
void* ECDbBackdoor::ECDb::GetSqliteDb (BeSQLite::EC::ECDbCR ecdb)
    {
    return ecdb.GetSqlDb ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  08/2014
//+---------------+---------------+---------------+---------------+---------------+------
BeSQLite::EC::ECSqlStatus ECDbBackdoor::ECDb::ECSqlStatement::BindDateTime (BeSQLite::EC::ECSqlStatement& stmt, int parameterIndex, uint64_t julianDayTicksHns, DateTime::Info const* metadata)
    {
    return stmt.GetBinder (parameterIndex).BindDateTime (julianDayTicksHns, metadata);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  07/2014
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbBackdoor::ECObjects::ECValue::SetAllowsPointersIntoInstanceMemory (ECN::ECValueR value, bool allow)
    {
    value.SetAllowsPointersIntoInstanceMemory (allow);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  07/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECObjectsStatus ECDbBackdoor::ECObjects::ECSchemaReadContext::AddSchema(ECN::ECSchemaReadContext& context, ECN::ECSchemaR schema)
    {
    return context.AddSchema(schema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  07/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbBackdoor::ECObjects::ECValue::AllowsPointersIntoInstanceMemory (ECN::ECValueCR value)
    {
    return value.AllowsPointersIntoInstanceMemory ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  07/2014
//+---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr ECDbBackdoor::IGeometryFlatBuffer::BytesToGeometry(bvector <Byte> &buffer)
    {
    return BentleyGeometryFlatBuffer::BytesToGeometry(buffer);
    }
