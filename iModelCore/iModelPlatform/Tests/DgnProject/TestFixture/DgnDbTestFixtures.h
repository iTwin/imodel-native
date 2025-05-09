/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "../NonPublished/DgnHandlersTests.h"
#include "GeomHelper.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <ECDb/ECDbApi.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include <UnitTests/BackDoor/DgnPlatform/PerfTestDomain.h>
#include <DgnPlatform/PlatformLib.h>
#include <DgnPlatform/GenericDomain.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_DGNDB_UNIT_TESTS_NAMESPACE

//=======================================================================================
//! A base test fixture to be used for using DgnPlatformTest schema and domain
// @bsiclass
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

    void TearDown() override { try {SaveDb();} catch(...){}; }

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
    bool JsonDeepEqual(BeJsDocument const& a, BeJsDocument const& b) const;
    bool JsonDeepEqual(Json::Value const& a, Json::Value const& b) const;
};


//=======================================================================================
//! A base test fixture to be used for using DgnPlatformTest schema and domain
// @bsiclass
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

//=======================================================================================
//! Used in combination with LogCatcher to capture log messages
// @bsiclass
//=======================================================================================
struct TestLogger : NativeLogging::Logger {
    std::vector<std::pair<NativeLogging::SEVERITY, Utf8String>> m_messages;
        void LogMessage(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg) override {
            m_messages.emplace_back(sev, msg);
    }

    bool IsSeverityEnabled(Utf8CP category, NativeLogging::SEVERITY sev) override{
        return true;
    }

    void Clear() { m_messages.clear(); }

    bool ValidateMessageAtIndex(size_t index, NativeLogging::SEVERITY expectedSeverity, const Utf8String& expectedMessage) const {
        if (index < m_messages.size()) {
            const auto& [severity, message] = m_messages[index];
            return severity == expectedSeverity && message.Equals(expectedMessage);
        }
        return false; // Return false if the index is out of bounds
    }
    
    const std::pair<NativeLogging::SEVERITY, Utf8String>* GetLastMessage() const {
        if (!m_messages.empty()) {
            return &m_messages.back();
        }
        return nullptr; // Return nullptr if there are no messages
    }

    const std::pair<NativeLogging::SEVERITY, Utf8String>* GetLastMessage(NativeLogging::SEVERITY severity) const {
        for (auto it = m_messages.rbegin(); it != m_messages.rend(); ++it) {
            if (it->first == severity) {
                return &(*it);
            }
        }
        return nullptr; // Return nullptr if no messages with the specified severity are found
    }
};

//=======================================================================================
//! Until destruction, captures log messages and redirects them to the TestLogger
// @bsiclass
//=======================================================================================
struct LogCatcher {
    NativeLogging::Logger& m_previousLogger;
    TestLogger& m_testLogger;

    LogCatcher(TestLogger& testLogger) : m_testLogger(testLogger), m_previousLogger(NativeLogging::Logging::GetLogger()) {
        NativeLogging::Logging::SetLogger(&m_testLogger);
    }

    ~LogCatcher() {
        NativeLogging::Logging::SetLogger(&m_previousLogger);
    }
};
