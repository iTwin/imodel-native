/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/TestFixture/DgnDbTestFixtures.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../NonPublished/DgnHandlersTests.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"
#include "GeomHelper.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <ECDb/ECDbApi.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <DgnPlatform/DgnPlatformLib.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_DGNDB_UNIT_TESTS_NAMESPACE

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
        DgnDomains::RegisterDomain(DPTest::DgnPlatformTestDomain::GetDomain());
    }

    ~DgnDbTestFixture()
    {
    }

    void SetupProject(WCharCP baseProjFile, WCharCP testProjFile, BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite, bool needBriefcase = false);
    void SetupProject(WCharCP baseProjFile, CharCP testFile, BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite, bool needBriefcase = false);
    static BeFileName CopyDb(WCharCP inputFileName, WCharCP outputFileName);
    static void OpenDb(DgnDbPtr& db, BeFileNameCR name, DgnDb::OpenMode mode, bool needBriefcase = false);
    void CloseDb() { m_db->CloseDb(); }
    void SaveDb() {
        if (m_db.IsValid())
            m_db->SaveChanges();
    }

    DgnModelR GetDefaultModel() { return *m_defaultModelP; }

    DgnElementCPtr InsertElement(DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnDbStatus* result = nullptr, DgnElement::Code elementCode = DgnElement::Code());
    DgnElementCPtr InsertElement(Render::GeometryParamsCR ep, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnElement::Code elementCode = DgnElement::Code());
    DgnElementCPtr InsertElement(Utf8CP elementCode, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId());
    DgnElementId InsertElement2d(DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnElement::Code elementCode = DgnElement::Code());

    DgnElementId InsertElementUsingGeomPart(Utf8CP gpCode, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnElement::Code elementCode = DgnElement::Code());
    DgnElementId InsertElementUsingGeomPart(DgnGeomPartId gpId, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnElement::Code elementCode = DgnElement::Code());
    DgnElementId InsertElementUsingGeomPart2d(Utf8CP gpCode, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnElement::Code elementCode = DgnElement::Code());

    void setUpPhysicalView(DgnDbR dgnDb, DgnModelR model, ElementAlignedBox3d elementBox, DgnCategoryId categoryId);
};

