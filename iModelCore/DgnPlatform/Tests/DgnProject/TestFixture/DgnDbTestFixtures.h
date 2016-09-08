/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/TestFixture/DgnDbTestFixtures.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
#include <DgnPlatform/GenericDomain.h>

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
public: 
    static void SetUpTestCase();
    static void TearDownTestCase();
    static DgnDbTestUtils::SeedDbInfo s_seedFileInfo;

    ScopedDgnHost               m_host;
    DgnDbPtr                    m_db;
    DgnModelId                  m_defaultModelId;
    DgnCategoryId               m_defaultCategoryId;
    DgnModelPtr                 m_defaultModelP;
public:
    explicit DgnDbTestFixture()
    {
        DgnDomains::RegisterDomain(DPTest::DgnPlatformTestDomain::GetDomain());
    }

    ~DgnDbTestFixture()
    {
    }

    virtual void TearDown(){ SaveDb(); }
    
    //! Initialize already converted/published dgndb/idgndb file
    void SetupWithPrePublishedFile(WCharCP baseProjFile, WCharCP testProjFile, BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite, bool needBriefcase = false, bool needTestDomain = false);
    
    //! Initialize a seed file with the name provided
    void SetupSeedProject(WCharCP inFile, BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite, bool needBriefcase = false);
    
    //! Initialize a seed file with the name same as Test_Name
    void SetupSeedProject( BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite, bool needBriefcase = false);
    
    static BeFileName CopyDb(WCharCP inputFileName, WCharCP outputFileName);
    static void OpenDb(DgnDbPtr& db, BeFileNameCR name, DgnDb::OpenMode mode, bool needBriefcase = false);
    void CloseDb() { m_db->CloseDb(); }
    DgnDbR GetDgnDb() { return *m_db; }

    static DgnDbStatus GetSeedDbCopy(BeFileNameR actualName, WCharCP newName);

    void SaveDb() {
        if (m_db.IsValid() && m_db->IsDbOpen() && !m_db->IsReadonly())
            m_db->SaveChanges();
    }

    DgnModelR GetDefaultModel() { return *m_defaultModelP; }

    DgnElementCPtr InsertElement(DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnDbStatus* result = nullptr, DgnCode elementCode = DgnCode());
    DgnElementCPtr InsertElement(Render::GeometryParamsCR ep, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnCode elementCode = DgnCode());
    DgnElementCPtr InsertElement(Utf8CP elementCode, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId());
    DgnElementId InsertElement2d(DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnCode elementCode = DgnCode());

    DgnElementId InsertElementUsingGeometryPart(DgnGeometryPartId gpId, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnCode elementCode = DgnCode());
    DgnElementId InsertElementUsingGeometryPart2d(DgnCodeCR gpCode, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnCode elementCode = DgnCode());

    static void SetUpSpatialView(DgnDbR dgnDb, DgnModelR model, ElementAlignedBox3d elementBox, DgnCategoryId categoryId);
};

