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

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_DGNDB_UNIT_TESTS_NAMESPACE

#define TMTEST_SCHEMA_NAME                               "DgnPlatformTest"
#define TMTEST_SCHEMA_NAMEW                             L"DgnPlatformTest"
#define TMTEST_TEST_ELEMENT_CLASS_NAME                   "TestElement"
#define TMTEST_TEST_ELEMENT_TestElementProperty         L"TestElementProperty"
#define TMTEST_TEST_ITEM_CLASS_NAME                       "TestItem"
#define TMTEST_TEST_ITEM_TestItemProperty               L"TestItemProperty"
#define TMTEST_TEST_ITEM_TestItemPropertyA               "TestItemProperty"

struct TestElementHandler;

//=======================================================================================
//! A test Element
// @bsiclass                                                     Sam.Wilson      04/15
//=======================================================================================
struct TestElement : Dgn::PhysicalElement
{
    DEFINE_T_SUPER(Dgn::PhysicalElement)

private:
    friend struct TestElementHandler;

    TestElement(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A test ElementHandler for a class in DgnPlatformTest schema
// @bsiclass                                                     Sam.Wilson      01/15
//=======================================================================================
struct TestElementHandler : Dgn::ElementHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS("TestElement", TestElement, TestElementHandler, Dgn::ElementHandler, )

public:
    ECN::ECClassCP GetTestElementECClass(DgnDbR db);
    DgnElementKey InsertElement(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode);
    DgnElementKey InsertElement(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode, ElemDisplayParamsCR ep);
    DgnDbStatus DeleteElement(DgnDbR db, DgnElementId eid);
};

//=======================================================================================
//! Domain that knows DgnPlatformTest schema
// @bsiclass                                                     Majd.Uddin      06/15
//=======================================================================================
struct DgnPlatformTestDomain : public DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(DgnPlatformTestDomain, )
public:
    DgnPlatformTestDomain();
};



//=======================================================================================
//! A base test fixture to be used for using DgnPlatformTest schema and domain
// @bsiclass                                                     Majd.Uddin      06/15
//=======================================================================================
struct DgnDbTestFixture : public ::testing::Test
{

public:
    ScopedDgnHost               m_host;
    DgnDbPtr                    m_db;
    DgnModelId                  m_defaultModelId;
    DgnCategoryId               m_defaultCategoryId;
    DgnModelP                   m_defaultModelP;

    DgnDbTestFixture()
    {
        DgnDomains::RegisterDomain(DgnPlatformTestDomain::GetDomain());
    }

    ~DgnDbTestFixture()
    {
    }

    void SetupProject(WCharCP baseProjFile, WCharCP testProjFile, BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::OPEN_ReadWrite);
    
    DgnElementKey InsertElement(Utf8CP elementCode, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId());
    DgnElementKey InsertElement(Utf8CP elementCode, ElemDisplayParamsCR ep, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId());
    bool InsertElementItem(DgnElementId id, WCharCP propValue);
    bool UpdateElementItem(DgnElementId id, WCharCP propValue);
    bool DeleteElementItem(DgnElementId id);
    bool SelectElementItem(DgnElementId id);


};

