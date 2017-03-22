    /*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/Tests/BackDoor/DataCaptureTestDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if 0  //NOT NOW

#include "PublicAPI/BackDoor/DataCapture/BackDoor.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN

HANDLER_DEFINE_MEMBERS(TestElementGroupHandler)
DOMAIN_DEFINE_MEMBERS(DataCaptureTestDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2015
//---------------------------------------------------------------------------------------
TestElementGroupPtr TestElementGroup::Create(DgnDbR dgndb, DgnModelId modelId, DgnCategoryId categoryId, DgnCode elementCode)
    {
    TestElementGroupPtr testElementGroup = new TestElementGroup(CreateParams(dgndb, modelId, DgnClassId(QueryClassId(dgndb)), categoryId, Placement3d(), elementCode));
    return testElementGroup;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2015
//---------------------------------------------------------------------------------------
DataCaptureTestDomain::DataCaptureTestDomain() : DgnDomain("DataCaptureTest", "DataCapture Test Schema", 1)
    {
    RegisterHandler(TestElementGroupHandler::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2015
//---------------------------------------------------------------------------------------
DgnDbStatus DataCaptureTestDomain::Register()
    {
    DgnDomains::RegisterDomain(GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2015
//---------------------------------------------------------------------------------------
DgnDbStatus DataCaptureTestDomain::ImportSchema(DgnDbR dgndb)
    {
    BeFileName schemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    schemaFile.AppendToPath(L"ECSchemas/" DATACAPTURE_TEST_SCHEMA_NAMEW L".01.00.ecschema.xml");

    DgnDbStatus status = GetDomain().DgnDomain::ImportSchema(dgndb, schemaFile);
    if (DgnDbStatus::Success != status)
        return status;

    ECN::ECSchemaCP schema = dgndb.Schemas().GetSchema(DATACAPTURE_TEST_SCHEMA_NAME, true);
    if (nullptr == schema)
        return DgnDbStatus::BadSchema;

    if (!TestElementGroup::QueryClassId(dgndb).IsValid())
        return DgnDbStatus::BadSchema;

    return DgnDbStatus::Success;
    }

#endif
