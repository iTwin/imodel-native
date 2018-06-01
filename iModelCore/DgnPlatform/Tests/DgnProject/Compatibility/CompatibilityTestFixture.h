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
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>

#define PROFILE_NAME_BEDB "bedb"
#define PROFILE_NAME_ECDB "ecdb"
#define PROFILE_NAME_DGNDB "dgndb"

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
enum class ProfileType
    {
    BeDb = 0,
    ECDb = 1,
    DgnDb = 2
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
enum class ProfileState
    {
    Current = 0,
    Older = -1,
    Newer = 1
    };

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct TestFile final
    {
    private:
        Utf8StringCR m_name;
        BeFileName m_path;
        ProfileState m_profileState;
        ProfileVersion m_profileVersion;

    public:
        TestFile(Utf8StringCR name, BeFileName const& path, ProfileState state, ProfileVersion const& version) : m_name(name), m_path(path), m_profileState(state), m_profileVersion(version) {}

        Utf8StringCR GetName() const { return m_name; }
        BeFileNameCR GetPath() const { return m_path; }
        ProfileState GetProfileState() const { return m_profileState; }
        ProfileVersion const& GetVersion() const { return m_profileVersion; }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
struct Profile : NonCopyableClass
    {
    protected:
        mutable ProfileVersion m_expectedVersion = ProfileVersion(0, 0, 0, 0);
        PropertySpec m_versionPropertySpec;

    private:
        ProfileType m_type;
        Utf8CP m_name = nullptr;
        BeFileName m_profileSeedFolder;

        virtual BentleyStatus _Init() const = 0;
        BentleyStatus RetrieveTestFiles() const;

    protected:
        Profile(ProfileType type, Utf8CP nameSpace, Utf8CP name);

        static BentleyStatus ReadProfileVersion(ProfileVersion&, Db const&, PropertySpec const&);

    public:
        virtual ~Profile() {}

        BentleyStatus Init() const;

        ProfileVersion const& GetExpectedVersion() const { return m_expectedVersion; }

        std::vector<TestFile> GetAllVersionsOfTestFile(Utf8CP testFileName) const;

        ProfileState GetFileProfileState(ProfileVersion const& fileProfileVersion) const
            {
            const int compareRes = fileProfileVersion.CompareTo(m_expectedVersion);
            if (compareRes == 0)
                return ProfileState::Current;

            if (compareRes < 0)
                return ProfileState::Older;

            return ProfileState::Newer;
            }

        BeFileNameCR GetSeedFolder() const { return m_profileSeedFolder; }

        BeFileName GetPathForNewTestFile(Utf8CP testFileName) const;
        static ProfileType ParseProfileType(Utf8CP);
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
struct BeDbProfile final : Profile
    {
    private:
        BeDbProfile() : Profile(ProfileType::BeDb, "be_Db", PROFILE_NAME_BEDB) {}
        BentleyStatus _Init() const override;

    public:
        ~BeDbProfile() {}

        static std::unique_ptr<BeDbProfile> Create() 
            {
            std::unique_ptr<BeDbProfile> profile(new BeDbProfile());
            if (SUCCESS != profile->Init())
                return nullptr;

            return profile;
            }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
struct ECDbProfile final : Profile
    {
    private:
        ECDbProfile() : Profile(ProfileType::ECDb, "ec_Db", PROFILE_NAME_ECDB) {}
        BentleyStatus _Init() const override;
    public:
        ~ECDbProfile() {}

        static std::unique_ptr<ECDbProfile> Create()
            {
            std::unique_ptr<ECDbProfile> profile(new ECDbProfile());
            if (SUCCESS != profile->Init())
                return nullptr;

            return profile;
            }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
struct DgnDbProfile final : Profile
    {
    private:
        DgnDbProfile() : Profile(ProfileType::DgnDb, "dgn_Db", PROFILE_NAME_DGNDB) {}
        BentleyStatus _Init() const override;
    public:
        ~DgnDbProfile() {}

        static std::unique_ptr<DgnDbProfile> Create()
            {
            std::unique_ptr<DgnDbProfile> profile(new DgnDbProfile());
            if (SUCCESS != profile->Init())
                return nullptr;

            return profile;
            }
    };



//======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
struct ProfileManager final : NonCopyableClass
    {
    private:
        mutable std::map<ProfileType, std::unique_ptr<Profile>> m_profiles;
        mutable BeFileName m_testSeedFolder; //this where test file are located
        mutable BeFileName m_outFolder; //this is where they are copied before running the test.

        static ProfileManager* s_singleton;

        ProfileManager() {}

        BeFileName const& GetOutFolder() const;
   
    public:
        static ProfileManager& Get() { return *s_singleton; }

        BeFileNameCR GetSeedFolder() const;

        Profile& GetProfile(ProfileType) const;
    };
//
//======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
struct CompatibilityTestFixture : ::testing::Test 
    {
protected:
    ProfileManager& ProfileManager() const { return ProfileManager::Get(); }
    };
