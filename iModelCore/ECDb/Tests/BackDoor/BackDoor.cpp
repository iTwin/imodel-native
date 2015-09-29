/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/BackDoor.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/BackDoor.h"
#include <GeomSerialization/GeomSerializationApi.h>

namespace ECDbBackDoor = BentleyApi::BeSQLite::EC::Tests::BackDoor;

BEGIN_ECDBUNITTESTS_NAMESPACE

void ReadJsonInputFromFile(Json::Value& jsonInput, BeFileName& jsonFilePath);
//---------------------------------------------------------------------------------------
// @bsimethod                                  Muhammad.Zaighum                  05/13
//+---------------+---------------+---------------+---------------+---------------+------
void ReadJsonInputFromFile(Json::Value& jsonInput, BeFileName& jsonFilePath)
{
    const Byte utf8BOM[] = { 0xef, 0xbb, 0xbf };

    Utf8String fileContent;

    BeFile file;
    auto fileStatus = file.Open(jsonFilePath, BeFileAccess::Read);
    ASSERT_TRUE(BeFileStatus::Success == fileStatus);

    uint64_t rawSize;
    fileStatus = file.GetSize(rawSize);
    ASSERT_TRUE(BeFileStatus::Success == fileStatus && rawSize <= UINT32_MAX);

    uint32_t sizeToRead = (uint32_t)rawSize;

    uint32_t sizeRead;
    ScopedArray<Byte> scopedBuffer(sizeToRead);
    Byte* buffer = scopedBuffer.GetData();
    fileStatus = file.Read(buffer, &sizeRead, sizeToRead);
    ASSERT_TRUE(BeFileStatus::Success == fileStatus && sizeRead == sizeToRead);
    ASSERT_TRUE(buffer[0] == utf8BOM[0] && buffer[1] == utf8BOM[1] && buffer[2] == utf8BOM[2]) << "Json file is expected to be encoded in UTF-8";

    for (uint32_t ii = 3; ii < sizeRead; ii++)
    {
        if (buffer[ii] == '\n' || buffer[ii] == '\r')
            continue;
        fileContent.append(1, buffer[ii]);
    }

    file.Close();

    ASSERT_TRUE(Json::Reader::Parse(fileContent, jsonInput)) << "Error when parsing the JSON file";
}
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
IGeometryPtr ECDbBackDoor::IGeometryFlatBuffer::BytesToGeometry(bvector <Byte> &buffer)
    {
    return BentleyGeometryFlatBuffer::BytesToGeometry(buffer);
    }
