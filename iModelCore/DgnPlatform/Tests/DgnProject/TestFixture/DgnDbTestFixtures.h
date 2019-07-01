/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../NonPublished/DgnHandlersTests.h"
#include "GeomHelper.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <ECDb/ECDbApi.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include <UnitTests/BackDoor/DgnPlatform/PerfTestDomain.h>
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
    static DgnPlatformSeedManager::SeedDbInfo s_seedFileInfo;

    ScopedDgnHost               m_host;
    DgnDbPtr                    m_db;
    DgnModelId                  m_defaultModelId;
    DgnCategoryId               m_defaultCategoryId;

public:
    explicit DgnDbTestFixture()
    {
    DgnDomains::RegisterDomain(DPTest::DgnPlatformTestDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
    }

    ~DgnDbTestFixture()
    {
    }

    void TearDown() override { SaveDb(); }
    
    //! Initialize a seed file with the name provided
    void SetupSeedProject(WCharCP inFile, BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite, bool needBriefcase = false);
    
    //! Initialize a seed file with the name same as Test_Name
    void SetupSeedProject( BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite, bool needBriefcase = false);
    
    static BeFileName CopyDb(WCharCP inputFileName, WCharCP outputFileName);
    static void OpenDb(DgnDbPtr& db, BeFileNameCR name, DgnDb::OpenMode mode, bool needBriefcase = false);
    void CloseDb() { m_db->CloseDb(); }
    DgnDbR GetDgnDb() { return *m_db; }

    static DgnDbStatus GetSeedDbCopy(BeFileNameR actualName, WCharCP newName);

    void SaveDb() 
        {
        if (m_db.IsValid() && m_db->IsDbOpen() && !m_db->IsReadonly())
            m_db->SaveChanges();
        }

    //! Get the default PhysicalModel created by SetupSeedProject
    //! @note will fail if a 2d model is the default
    PhysicalModelPtr GetDefaultPhysicalModel();

    //! Get the DgnCategoryId of the default category created by SetupSeedProject
    DgnCategoryId GetDefaultCategoryId() {return m_defaultCategoryId;}

    DgnElementCPtr InsertElement(DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnDbStatus* result = nullptr, DgnCode elementCode = DgnCode());
    DgnElementCPtr InsertElement(Render::GeometryParamsCR ep, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnCode elementCode = DgnCode());
    DgnElementCPtr InsertElement(Utf8CP elementCode, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId());
    DgnElementId InsertElement2d(DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnCode elementCode = DgnCode());

    DgnElementId InsertElementUsingGeometryPart(DgnGeometryPartId gpId, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnCode elementCode = DgnCode());
    DgnElementId InsertElementUsingGeometryPart2d(DgnCodeCR gpCode, DgnModelId mid = DgnModelId(), DgnCategoryId categoryId = DgnCategoryId(), DgnCode elementCode = DgnCode());
};


//=======================================================================================
//! A base test fixture to be used for using DgnPlatformTest schema and domain
// @bsiclass                                                     Majd.Uddin      06/15
//=======================================================================================
struct PerfTestFixture : ::testing::Test
    {
    public:
        static void SetUpTestCase();
        static void TearDownTestCase();
        static DgnPlatformSeedManager::SeedDbInfo s_seedFileInfo;

        ScopedDgnHost               m_host;
        DgnDbPtr                    m_db;
        DgnModelId                  m_defaultModelId;
        DgnCategoryId               m_defaultCategoryId;

    public:
        explicit PerfTestFixture()
            {
            DgnDomains::RegisterDomain(DPTest::PerfTestDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::No);
            }

        ~PerfTestFixture()
            {}

        void TearDown() override { SaveDb(); }

        //! Initialize a seed file with the name provided
        void SetupSeedProject(WCharCP inFile, BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite, bool needBriefcase = false);

        //! Initialize a seed file with the name same as Test_Name
        void SetupSeedProject(BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite, bool needBriefcase = false);

        static BeFileName CopyDb(WCharCP inputFileName, WCharCP outputFileName);
        static void OpenDb(DgnDbPtr& db, BeFileNameCR name, DgnDb::OpenMode mode, bool needBriefcase = false);
        void CloseDb() { m_db->CloseDb(); }
        DgnDbR GetDgnDb() { return *m_db; }

        static DgnDbStatus GetSeedDbCopy(BeFileNameR actualName, WCharCP newName);

        void SaveDb()
            {
            if (m_db.IsValid() && m_db->IsDbOpen() && !m_db->IsReadonly())
                m_db->SaveChanges();
            }

        //! Get the default PhysicalModel created by SetupSeedProject
        //! @note will fail if a 2d model is the default
        PhysicalModelPtr GetDefaultPhysicalModel();

        //! Get the DgnCategoryId of the default category created by SetupSeedProject
        DgnCategoryId GetDefaultCategoryId() { return m_defaultCategoryId; }

    };
