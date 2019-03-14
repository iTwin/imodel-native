/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/Profiles.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <BeSQLite/BeSQLite.h>
#include <Bentley/BeVersion.h>
#include <Bentley/Nullable.h>
#include <vector>
#include <map>

#define PROFILE_NAME_BEDB "bedb"
#define PROFILE_NAME_ECDB "ecdb"
#define PROFILE_NAME_DGNDB "dgndb"

//======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
enum class ProfileType
    {
    BeDb = 0,
    ECDb = 1,
    DgnDb = 2
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct TestFile final
    {
    private:
        Utf8String m_name;
        BeFileName m_path;
        BeFileName m_seedPath;
        BeSQLite::ProfileVersion m_bedbVersion = BeSQLite::ProfileVersion(0, 0, 0, 0);
        BeSQLite::ProfileVersion m_ecdbVersion = BeSQLite::ProfileVersion(0, 0, 0, 0);
        BeSQLite::ProfileVersion m_dgndbVersion = BeSQLite::ProfileVersion(0, 0, 0, 0);

        BeSQLite::ProfileVersion m_initialBeDbVersion = BeSQLite::ProfileVersion(0, 0, 0, 0);
        BeSQLite::ProfileVersion m_initialECDbVersion = BeSQLite::ProfileVersion(0, 0, 0, 0);
        BeSQLite::ProfileVersion m_initialDgnDbVersion = BeSQLite::ProfileVersion(0, 0, 0, 0);

    public:
        TestFile(Utf8StringCR name, BeFileNameCR path, BeFileNameCR seedPath, 
                 BeSQLite::ProfileVersion const& bedbVersion, BeSQLite::ProfileVersion const& ecdbVersion, BeSQLite::ProfileVersion const& dgndbVersion, 
                 BeSQLite::ProfileVersion const& initialBeDbVersion, BeSQLite::ProfileVersion const& initialECDbVersion, BeSQLite::ProfileVersion const& initialDgnDbVersion);

        TestFile(TestFile const&) = default;
        TestFile& operator=(TestFile const&) = default;
        TestFile(TestFile&&) = default;
        TestFile& operator=(TestFile&&) = default;

        BeFileNameStatus CloneSeedToOutput() const { return CloneSeed(m_path); }
        BeFileNameStatus CloneSeed(BeFileNameCR targetFolder) const;

        Utf8StringCR GetName() const { return m_name; }
        BeFileNameCR GetPath() const { return m_path; }
        BeFileNameCR GetSeedPath() const { return m_seedPath; }
        BeSQLite::ProfileVersion const& GetBeDbVersion() const { return m_bedbVersion; }
        BeSQLite::ProfileVersion const& GetECDbVersion() const { return m_ecdbVersion; }
        BeSQLite::ProfileVersion const& GetDgnDbVersion() const { return m_dgndbVersion; }

        //The test file is an upgraded file, i.e. not created from scratch
        bool IsUpgraded() const { return m_dgndbVersion > m_initialDgnDbVersion || m_ecdbVersion > m_initialECDbVersion || m_bedbVersion > m_initialBeDbVersion; }
        BeSQLite::ProfileVersion const& GetInitialBeDbVersion() const { return m_initialBeDbVersion; }
        BeSQLite::ProfileVersion const& GetInitialECDbVersion() const { return m_initialECDbVersion; }
        BeSQLite::ProfileVersion const& GetInitialDgnDbVersion() const { return m_initialDgnDbVersion; }

        //Indicates whether the test file in its un-upgraded state is older/newer/up-to-date compared to the version expected by the software.
        BeSQLite::ProfileState::Age GetAge() const;
        Utf8String ToString() const;
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
struct Profile : NonCopyableClass
    {
    protected:
        ProfileType m_type;
        mutable BeSQLite::ProfileVersion m_expectedVersion = BeSQLite::ProfileVersion(0, 0, 0, 0);
        BeSQLite::PropertySpec m_versionPropertySpec;

    private:
        static constexpr Utf8CP PROFILEVERSIONFOLDER_VERSIONSEPARATOR = "_";
        static constexpr WCharCP PROFILEVERSIONFOLDER_VERSIONSEPARATOR_W = L"_";

        Utf8CP m_name = nullptr;
        BeFileName m_profileCreatedDataFolder; // files created by this test run (will be saved as nugets)
        BeFileName m_profilePulledTestDataFolder; // pre-existing test files (pulled as nugets)
        BeFileName m_profileOutFolder;

        virtual BentleyStatus _Init() const = 0;
        BeFileName GetFolderForNewTestFile() const;

        std::vector<TestFile> GetAllVersionsOfTestFile(BeFileNameCR rootFolder, WStringCR fileNamePattern, bool logFoundFiles) const;

    protected:
        Profile(ProfileType type, Utf8CP nameSpace, Utf8CP name);

        BeSQLite::ProfileVersion ReadProfileVersion(BeSQLite::Db const&) const;

    public:
        virtual ~Profile() {}

        BentleyStatus Init() const;

        BeSQLite::ProfileVersion const& GetExpectedVersion() const { return m_expectedVersion; }

        BeFileNameCR GetCreatedDataFolder() const { return m_profileCreatedDataFolder; }
        BeFileNameCR GetTestDataFolder() const { return m_profilePulledTestDataFolder; }
        BeFileNameCR GetOutFolder() const { return m_profileOutFolder; }

        std::vector<TestFile> GetAllVersionsOfTestFile(Utf8CP testFileName, bool logFoundFiles = true) const;
        std::vector<TestFile> GetAllVersionsOfAllPulledTestFiles(bool logFoundFiles = true) const { return GetAllVersionsOfTestFile(m_profilePulledTestDataFolder, L"*.*", logFoundFiles); }
        std::vector<TestFile> GetAllVersionsOfTestFile(BeFileNameCR rootFolder, Utf8CP testFileName, bool logFoundFiles) const;
        BeFileName GetPathForNewTestFile(Utf8StringCR testFileName) const { return GetFolderForNewTestFile().AppendToPath(BeFileName(testFileName)); }
        BeFileName GetPathForNewUpgradedTestFile(TestFile const& oldSeedFile) const;
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
struct BeDbProfile final : Profile
    {
    private:
        static BeDbProfile const* s_singleton;

        BeDbProfile() : Profile(ProfileType::BeDb, "be_Db", PROFILE_NAME_BEDB) {}
        BentleyStatus _Init() const override;

    public:
        ~BeDbProfile() {}

        static BeDbProfile const& Get()
            {
            if (s_singleton == nullptr)
                {
                s_singleton = new BeDbProfile();
                EXPECT_EQ(SUCCESS, s_singleton->Init()) << "Failed to initialize BeDbProfile";
                }

            return *s_singleton;
            }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
struct ECDbProfile final : Profile
    {
    private:
        static ECDbProfile const* s_singleton;

        ECDbProfile() : Profile(ProfileType::ECDb, "ec_Db", PROFILE_NAME_ECDB) {}
        BentleyStatus _Init() const override;
    public:
        ~ECDbProfile() {}

        static ECDbProfile const& Get()
            {
            if (s_singleton == nullptr)
                {
                s_singleton = new ECDbProfile();
                EXPECT_EQ(SUCCESS, s_singleton->Init()) << "Failed to initialize ECDbProfile";
                }

            return *s_singleton;
            }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
struct DgnDbProfile final : Profile
    {
    private:
        static DgnDbProfile const* s_singleton;

        DgnDbProfile() : Profile(ProfileType::DgnDb, "dgn_Db", PROFILE_NAME_DGNDB) {}
        BentleyStatus _Init() const override;
    public:
        ~DgnDbProfile() {}

        static DgnDbProfile const& Get()
            {
            if (s_singleton == nullptr)
                {
                s_singleton = new DgnDbProfile();
                EXPECT_EQ(SUCCESS, s_singleton->Init()) << "Failed to initialize DgnDbProfile";
                }

            return *s_singleton;
            }
    };
