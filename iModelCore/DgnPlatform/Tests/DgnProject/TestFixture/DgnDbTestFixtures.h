/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/TestFixture/DgnDbTestFixtures.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../NonPublished/DgnHandlersTests.h"
#include "GeomHelper.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <ECDb/ECDbApi.h>
#include <DgnPlatform/DgnHandlers/ScopedDgnHost.h>
#include <DgnPlatform/DgnPlatformLib.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_DGNDB_UNIT_TESTS_NAMESPACE

#define TMTEST_SCHEMA_NAME                               "DgnPlatformTest"
#define TMTEST_TEST_ELEMENT_CLASS_NAME                   "TestElement"
#define TMTEST_TEST_ELEMENT2d_CLASS_NAME                 "TestElement2d"
#define TMTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME    "TestElementDrivesElement"
#define TMTEST_TEST_ELEMENT_TestElementProperty          "TestElementProperty"
#define TMTEST_TEST_ITEM_CLASS_NAME                      "TestItem"
#define TMTEST_TEST_ITEM_TestItemProperty                "TestItemProperty"

struct TestElementHandler;
struct TestElement;
struct TestElement2dHandler;
struct TestElement2d;

//These typedefs have to be defined
typedef RefCountedPtr<TestElement> TestElementPtr;
typedef RefCountedCPtr<TestElement> TestElementCPtr;
typedef TestElement& TestElementR;
typedef TestElement const& TestElementCR;

//These typedefs have to be defined
typedef RefCountedPtr<TestElement2d> TestElement2dPtr;
typedef RefCountedCPtr<TestElement2d> TestElement2dCPtr;
typedef TestElement2d& TestElement2dR;
typedef TestElement2d const& TestElement2dCR;


//=======================================================================================
//! A test Element. Has methods to manipulate Item data.
// @bsiclass                                                     Sam.Wilson      04/15
//=======================================================================================
struct TestElement : PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(TMTEST_TEST_ELEMENT_CLASS_NAME, PhysicalElement) 

protected:
    friend struct TestElementHandler;

    Utf8String m_testItemProperty;

    virtual DgnDbStatus _InsertSecondary() override;
    virtual DgnDbStatus _UpdateInDb() override;
    virtual DgnDbStatus _DeleteInDb() const override;

public:
    TestElement(CreateParams const& params) : T_Super(params) {}
    static TestElementPtr Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Code elementCode);
    static TestElementPtr Create(DgnDbR db, ElemDisplayParamsCR ep, DgnModelId mid, DgnCategoryId categoryId, Code elementCode);
    static ECN::ECClassCP GetTestElementECClass(DgnDbR db) { return db.Schemas().GetECClass(TMTEST_SCHEMA_NAME, TMTEST_TEST_ELEMENT_CLASS_NAME); }
    void SetTestItemProperty(Utf8CP value) { m_testItemProperty.AssignOrClear(value); }
};

//=======================================================================================
//! A test ElementHandler for a class in DgnPlatformTest schema
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct TestElementHandler : dgn_ElementHandler::Physical
{
    ELEMENTHANDLER_DECLARE_MEMBERS(TMTEST_TEST_ELEMENT_CLASS_NAME, TestElement, TestElementHandler, dgn_ElementHandler::Physical, )
};
struct GTestElement2dHandler;

//=======================================================================================
//! A test Element
// @bsiclass                                                     Sam.Wilson      04/15
//=======================================================================================
struct TestElement2d : Dgn::DrawingElement
{
    DGNELEMENT_DECLARE_MEMBERS(TMTEST_TEST_ELEMENT2d_CLASS_NAME, Dgn::DrawingElement) 

private:
    friend struct GTestElement2dHandler;
public:
    TestElement2d(CreateParams const& params) : T_Super(params) {}
    static TestElement2dPtr Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Code elementCode);
};

//=======================================================================================
//! A test ElementHandler
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct TestElement2dHandler : dgn_ElementHandler::Drawing
{
    ELEMENTHANDLER_DECLARE_MEMBERS(TMTEST_TEST_ELEMENT2d_CLASS_NAME, TestElement2d, TestElement2dHandler, dgn_ElementHandler::Drawing, )
};

//=======================================================================================
//! Domain that knows DgnPlatformTest schema
// @bsiclass                                                     Majd.Uddin      06/15
//=======================================================================================
struct DgnPlatformTestDomain : DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(DgnPlatformTestDomain, )
public:
    DgnPlatformTestDomain();
};

//=======================================================================================
//! A base test fixture to be used for using DgnPlatformTest schema and domain
// @bsiclass                                                     Majd.Uddin      06/15
//=======================================================================================
struct DgnDbTestFixture : ::testing::Test
{
    ScopedDgnHost               m_host;
    DgnDbPtr                    m_db;
    DgnModelId                  m_defaultModelId;
    DgnCategoryId               m_defaultCategoryId;
    DgnModelPtr                 m_defaultModelP;
public:
    DgnDbTestFixture()
    {
        DgnDomains::RegisterDomain(DgnPlatformTestDomain::GetDomain());
    }

    ~DgnDbTestFixture()
    {
    }

    void SetupProject(WCharCP baseProjFile, WCharCP testProjFile, BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite);

    DgnElementCPtr InsertElement(DgnElement::Code elementCode, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnDbStatus* result = nullptr);
    DgnElementCPtr InsertElement(DgnElement::Code elementCode, ElemDisplayParamsCR ep, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId());
    DgnElementKey InsertElementUsingGeomPart(Utf8CP gpCode, DgnModelId mid, DgnCategoryId categoryId, DgnElement::Code elementCode);
    DgnElementKey InsertElementUsingGeomPart(DgnGeomPartId gpId, DgnModelId mid, DgnCategoryId categoryId, DgnElement::Code elementCode);
    DgnElementKey InsertElement2d(DgnModelId mid, DgnCategoryId categoryId, DgnElement::Code elementCode);
    DgnElementKey InsertElementUsingGeomPart2d(Utf8CP gpCode, DgnModelId mid, DgnCategoryId categoryId, DgnElement::Code elementCode);
    bool SelectElementItem(DgnElementId id);
};

