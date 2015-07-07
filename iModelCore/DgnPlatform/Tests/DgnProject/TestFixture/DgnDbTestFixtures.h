/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/TestFixture/DgnDbTestFixtures.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../NonPublished/DgnHandlersTests.h"
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
#define TMTEST_SCHEMA_NAMEW                             L"DgnPlatformTest"
#define TMTEST_TEST_ELEMENT_CLASS_NAME                   "TestElement"
#define TMTEST_TEST_ELEMENT_TestElementProperty          "TestElementProperty"
#define TMTEST_TEST_ITEM_CLASS_NAME                      "TestItem"
#define TMTEST_TEST_ITEM_TestItemProperty                "TestItemProperty"

struct TestElementHandler;

//=======================================================================================
//! A test Element. Has methods to manipulate Item data.
// @bsiclass                                                     Sam.Wilson      04/15
//=======================================================================================
struct TestElement : PhysicalElement
{
    DEFINE_T_SUPER(PhysicalElement)

protected:
    friend struct TestElementHandler;

    Utf8String m_testItemProperty;

    virtual DgnDbStatus _InsertInDb() override;
    virtual DgnDbStatus _UpdateInDb() override;
    virtual DgnDbStatus _DeleteInDb() const override;

public:
    TestElement(CreateParams const& params) : T_Super(params) {}
    static RefCountedPtr<TestElement> Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode);
    static RefCountedPtr<TestElement> Create(DgnDbR db, ElemDisplayParamsCR ep, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode);
    static ECN::ECClassCP GetTestElementECClass(DgnDbR db) { return db.Schemas().GetECClass(TMTEST_SCHEMA_NAME, TMTEST_TEST_ELEMENT_CLASS_NAME); }
    void TestElement::SetTestItemProperty(Utf8CP value) { m_testItemProperty.AssignOrClear(value); }
};

//These typedefs have to be defined
typedef RefCountedPtr<TestElement> TestElementPtr;
typedef RefCountedCPtr<TestElement> TestElementCPtr;
typedef TestElement& TestElementR;
typedef TestElement const& TestElementCR;

//=======================================================================================
//! A test ElementHandler for a class in DgnPlatformTest schema
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct TestElementHandler : dgn_ElementHandler::Element
{
    ELEMENTHANDLER_DECLARE_MEMBERS("TestElement", TestElement, TestElementHandler, dgn_ElementHandler::Element, )
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

    DgnDbTestFixture()
    {
        DgnDomains::RegisterDomain(DgnPlatformTestDomain::GetDomain());
    }

    ~DgnDbTestFixture()
    {
    }

    void SetupProject(WCharCP baseProjFile, WCharCP testProjFile, BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite);

    DgnElementCPtr InsertElement(Utf8CP elementCode, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId());
    DgnElementCPtr InsertElement(Utf8CP elementCode, ElemDisplayParamsCR ep, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId());
    bool SelectElementItem(DgnElementId id);
};

