/*--------------------------------------------------------------------------------------+
|
|  $Source: DataCaptureSchema/Tests/BackDoor/PublicAPI/BackDoor/DataCapture/DataCaptureTestDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#if 0 //NOT NOW
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnDomain.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/DgnModel.h>
#include <DgnPlatform/ElementHandler.h>

#define DATACAPTURE_TEST_SCHEMA_NAME   "DataCaptureTest"
#define DATACAPTURE_TEST_SCHEMA_NAMEW   L"DataCaptureTest"
#define DATACAPTURE_TEST_ELEMENTGROUP_CLASS_NAME "TestElementGroup"

DATACAPTURE_TEST_TYPEDEFS(TestElementGroup)
DATACAPTURE_TEST_REFCOUNTED_TYPEDEFS(TestElementGroup)

BEGIN_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE

//=======================================================================================
// Group of physical elements for testing. 
//=======================================================================================
struct TestElementGroup : PhysicalElement, IElementGroupOf < PhysicalElement >
    {
    DGNELEMENT_DECLARE_MEMBERS(DATACAPTURE_TEST_ELEMENTGROUP_CLASS_NAME, PhysicalElement)
        friend struct TestElementGroupHandler;
    private:
        virtual DgnElementCP _ToGroupElement() const override { return this; }

    public:
        TestElementGroup(CreateParams const& params) : T_Super(params) {}
        static TestElementGroupPtr Create(DgnDbR dgndb, DgnModelId modelId, DgnCategoryId categoryId, DgnCode elementCode);
        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(DATACAPTURE_TEST_SCHEMA_NAME, DATACAPTURE_TEST_ELEMENTGROUP_CLASS_NAME)); }
    };

//=======================================================================================
// Test element group handler
//=======================================================================================
struct TestElementGroupHandler : dgn_ElementHandler::Geometric3d
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(DATACAPTURE_TEST_ELEMENTGROUP_CLASS_NAME, TestElementGroup, TestElementGroupHandler, dgn_ElementHandler::Geometric3d, )
    };

//=======================================================================================
// Domain for planning tests
//=======================================================================================
struct DataCaptureTestDomain : Dgn::DgnDomain
    {
private:
    DOMAIN_DECLARE_MEMBERS(DataCaptureTestDomain, )
        DataCaptureTestDomain();

public:
    static Dgn::DgnDbStatus Register();
    static Dgn::DgnDbStatus ImportSchema(Dgn::DgnDbR);
    };


END_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE
#endif
