/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/CompatibilityTestFixture.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <functional>
#include <Bentley/Bentley.h>
#include "../TestFixture/DgnDbTestFixtures.h"
#include <Logging/bentleylogging.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTimeUtilities.h>

#define PROFILE_NAMESPACE_BEDB    "be_Db"
#define PROFILE_NAMESPACE_ECDB    "ec_Db"
#define PROFILE_NAMESPACE_DGNPROJ "dgn_Proj"
#define PROFILE_EXTENSION_BEDB    "bedb"
#define PROFILE_EXTENSION_ECDB    "ecdb"
#define PROFILE_EXTENSION_DGNPROJ "dgndb"
#define PROFILE_SCHEMAVERSION     "SchemaVersion"
//BeDb/2.0.0.1

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"Compatibility"))
struct CompareIUtf8Ascii
    {
    bool operator()(Utf8CP s1, Utf8CP s2) const { return BeStringUtilities::StricmpAscii(s1, s2) < 0; }
    bool operator()(Utf8StringCR s1, Utf8StringCR s2) const { return BeStringUtilities::StricmpAscii(s1.c_str(), s2.c_str()) < 0; }
    bool operator()(Utf8StringCP s1, Utf8StringCP s2) const { BeAssert(s1 != nullptr && s2 != nullptr); return BeStringUtilities::StricmpAscii(s1->c_str(), s2->c_str()) < 0; }
    };


//======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
struct ProfileManager final : NonCopyableClass
    {
    public:
        enum  Kind
            {
            PROFILE_BeDb = 0,
            PROFILE_ECDb = 1,
            PROFILE_DgnDb = 2
            };

        struct Profile;
        //======================================================================================
        // @bsiclass                                                 Affan.Khan          03/2018
        //======================================================================================
        struct TestFile
            {
            friend struct Profile;
            private:
                BeFileName m_resolvedFileName;
                
            public:
                virtual void Create() = 0;
                virtual bool IsPersisted() const = 0;
                virtual Utf8CP GetFileName() const = 0;
                virtual Utf8CP GetProfileName() const = 0;
                bool IsResolved() const { return !m_resolvedFileName.empty(); }
                BeFileName const& GetResolvedFilePath() const { return m_resolvedFileName; }
                bool Exists() const { return m_resolvedFileName.DoesPathExist(); }
            };

        //======================================================================================
        // @bsiclass                                                 Affan.Khan          03/2018
        //======================================================================================
        struct Profile : NonCopyableClass
            {
            private:
                virtual DbResult _GetSchemaVersion(ProfileVersion& schemaVersion, PropertySpec const& spec) const = 0;
                virtual void _Init() const = 0;

            private:
                Kind m_type;
                PropertySpec m_versionPropertySpec;
                Utf8String m_fileExtension;
                std::map<Utf8CP, TestFile*, CompareIUtf8Ascii> m_testFiles;
                mutable std::vector<BeVersion> m_verList;
                mutable BeFileName m_profileSeedFolder;
                mutable ProfileVersion m_expectedVersion;
                mutable bool m_initalized;

            private:
                BeFileName GetSeedFolder(BeVersion const& ver) const;
                BeFileName const& GetSeedFolder() const;
                bool GenerateSeedFile(TestFile* testFile, BeVersion const& ver) const;
                bool GenerateAllSeedFiles() const;
            public:
                void Init() const { if (!m_initalized) { _Init(); m_initalized = true; } }
                Profile(Kind type, Utf8CP nameSpace, Utf8CP schemaVersionProp, Utf8CP fileExtension);
                ProfileVersion const& GetExpectedVersion() const;
                bool Register(TestFile* testFileMethod);
                std::vector<BeVersion> const& ReadProfileVersionFromDisk() const;
                bool GetCopyOfTestFile(BeFileName& copyTestFile, Utf8CP name, BeVersion const& ver) const;
                std::vector<BeFileName> GetCopyOfAllVersionOfTestFile(Utf8CP name) const;
            };

    private:
        //======================================================================================
        // @bsiclass                                                 Affan.Khan          03/2018
        //======================================================================================
        struct _ECDb final : Profile
            {
            private:
                DbResult _GetSchemaVersion(ProfileVersion& schemaVersion, PropertySpec const& spec) const override;
                void _Init() const override;
            public:
                _ECDb() :Profile(Kind::PROFILE_BeDb, PROFILE_NAMESPACE_ECDB, PROFILE_SCHEMAVERSION, PROFILE_EXTENSION_ECDB) {}
            } m_ecdb;

        //======================================================================================
        // @bsiclass                                                 Affan.Khan          03/2018
        //======================================================================================
        struct _BeDb final : Profile
            {
            private:
                DbResult _GetSchemaVersion(ProfileVersion& schemaVersion, PropertySpec const& spec) const override;
                void _Init() const override;
            public:
                _BeDb() :Profile(Kind::PROFILE_ECDb, PROFILE_NAMESPACE_BEDB, PROFILE_SCHEMAVERSION, PROFILE_EXTENSION_BEDB) {}
            } m_bedb;

        //======================================================================================
        // @bsiclass                                                 Affan.Khan          03/2018
        //======================================================================================
        struct _DgnDb final : Profile
            {
            private:
                DbResult _GetSchemaVersion(ProfileVersion& schemaVersion, PropertySpec const& spec) const override;
                void _Init() const override;
            public:
                _DgnDb() :Profile(Kind::PROFILE_DgnDb, PROFILE_NAMESPACE_DGNPROJ, PROFILE_SCHEMAVERSION, PROFILE_EXTENSION_DGNPROJ) {}
            } m_dgndb;

        ProfileManager();
    protected:
        mutable BeFileName m_testSeedFolder; //this where test file are created if they do not exist
        mutable BeFileName m_outFolder; //this is where they are copied before running the test.
    public:
        BeFileName const& GetSeedFolder() const;
        BeFileName const& GetOutFolder() const;

        Profile* GetProfile(Kind kind);
        Profile* GetProfile(Utf8CP name);
        void Register(TestFile* tf);
        static int CompareFileVersion(BeFileName const& fl);
        static ProfileManager& GetInstance();
    };
//
//======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
struct CompatibilityTestFixture : ::testing::Test
    {
    private:
        virtual void SetUp() override { ; }
        virtual void TearDown() override { ; }
        virtual void TestBody() override { ; }
    public:
        static bool HasNewProfile(BeFileName const& fileName);
        static bool HasOldProfile(BeFileName const& fileName);
        static bool HasCurrentProfile(BeFileName const& fileName);
    };



//===========================================================================================================
#define TESTFILE_CLASS_NAME(profile, fileName) TEST_##profile##_##fileName
#define TESTFILE(profile, fileName, persisted)                                                              \
        struct TESTFILE_CLASS_NAME(profile, fileName) final : ProfileManager::TestFile                      \
            {                                                                                               \
            Utf8CP GetFileName() const override { return #fileName; }                                       \
            Utf8CP GetProfileName() const override { return #profile; }                                     \
            bool IsPersisted() const override { return persisted; }                                         \
            virtual void Create() override;                                                                 \
            static ProfileManager::TestFile* s_this;                                                        \
            TESTFILE_CLASS_NAME(profile, fileName) () { ProfileManager::GetInstance().Register(this); }     \
            };                                                                                              \
        ProfileManager::TestFile* TESTFILE_CLASS_NAME(profile, fileName) ::s_this = new TESTFILE_CLASS_NAME(profile, fileName) ();     \
        void TESTFILE_CLASS_NAME(profile, fileName)::Create()

#define ECDB_TESTFILE(FILENAME)  TESTFILE(ECDB, FILENAME, true)
#define BEDB_TESTFILE(FILENAME)  TESTFILE(BEDB, FILENAME, true)
#define DGNDB_TESTFILE(FILENAME) TESTFILE(DGNDB, FILENAME, true)
//
#define USE_TESTFILE(profile, fileName, bfn) ASSERT_TRUE(ProfileManager::GetInstance().GetProfile(#profile)->GetCopyOfTestFile(bfn, #fileName,ProfileManager::GetInstance().GetProfile(#profile)->GetExpectedVersion()));
#define USE_ECDB_TESTFILE(fileName, bfn) USE_TESTFILE(ECDB, fileName, bfn)

#define FOR_EACH_TESTFILE(profile, X,fileName) for(BeFileName const& X : ProfileManager::GetInstance().GetProfile(#profile)->GetCopyOfAllVersionOfTestFile(#fileName)) 

#define ECDB_FOR_EACH(X,fileName) FOR_EACH_TESTFILE(ECDB,X, fileName)
#define BEDB_FOR_EACH(X,fileName) FOR_EACH_TESTFILE(BEDB,X, fileName)
#define DGNDB_FOR_EACH(X,fileName) FOR_EACH_TESTFILE(DGNDB,X, fileName)


#define ECDB_OPEN_READONLY(T, testFile)   ECDb T; ASSERT_EQ(BE_SQLITE_OK, T.OpenBeSQLiteDb(testFile, Db::OpenParams(Db::OpenMode::Readonly)));
#define ECDB_OPEN_READOWRITE(T, testFile) ECDb T; ASSERT_EQ(BE_SQLITE_OK, T.OpenBeSQLiteDb(testFile, Db::OpenParams(Db::OpenMode::ReadWrite)));

#define BEDB_OPEN_READONLY(T, testFile)   Db T; ASSERT_EQ(BE_SQLITE_OK, T.OpenBeSQLiteDb(testFile, Db::OpenParams(Db::OpenMode::Readonly)));
#define BEDB_OPEN_READOWRITE(T, testFile) Db T; ASSERT_EQ(BE_SQLITE_OK, T.OpenBeSQLiteDb(testFile, Db::OpenParams(Db::OpenMode::ReadWrite)));

#define DGNDB_OPEN_READONLY(T, testFile)
#define DGNDB_OPEN_READOWRITE(T, testFile)

