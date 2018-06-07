/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/ProfileManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <BeSQLite/BeSQLite.h>
#include <Bentley/BeVersion.h>
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
        Utf8String m_name;
        BeFileName m_path;
        ProfileState m_profileState;
        BeSQLite::ProfileVersion m_profileVersion;

    public:
        TestFile(Utf8StringCR name, BeFileName const& path, ProfileState state, BeSQLite::ProfileVersion const& version) : m_name(name), m_path(path), m_profileState(state), m_profileVersion(version) {}

        Utf8StringCR GetName() const { return m_name; }
        BeFileNameCR GetPath() const { return m_path; }
        ProfileState GetProfileState() const { return m_profileState; }
        BeSQLite::ProfileVersion const& GetVersion() const { return m_profileVersion; }
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
struct Profile : NonCopyableClass
    {
    protected:
        mutable BeSQLite::ProfileVersion m_expectedVersion = BeSQLite::ProfileVersion(0, 0, 0, 0);
        BeSQLite::PropertySpec m_versionPropertySpec;

    private:
        ProfileType m_type;
        Utf8CP m_name = nullptr;
        BeFileName m_profileSeedFolder;

        virtual BentleyStatus _Init() const = 0;

    protected:
        Profile(ProfileType type, Utf8CP nameSpace, Utf8CP name);

    public:
        virtual ~Profile() {}

        BentleyStatus Init() const;

        BeSQLite::ProfileVersion const& GetExpectedVersion() const { return m_expectedVersion; }
        BeSQLite::ProfileVersion ReadProfileVersion(BeSQLite::Db const&) const;

        std::vector<TestFile> GetAllVersionsOfTestFile(Utf8CP testFileName, bool logFoundFiles = true) const;

        ProfileState GetFileProfileState(BeSQLite::ProfileVersion const& fileProfileVersion) const
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
