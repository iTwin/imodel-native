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
        BeSQLite::ProfileVersion m_bedbVersion = BeSQLite::ProfileVersion(0, 0, 0, 0);
        BeSQLite::ProfileVersion m_ecdbVersion = BeSQLite::ProfileVersion(0, 0, 0, 0);
        BeSQLite::ProfileVersion m_dgndbVersion = BeSQLite::ProfileVersion(0, 0, 0, 0);

    public:
        TestFile(Utf8StringCR name, BeFileName const& path, BeSQLite::ProfileVersion const& bedbVersion, BeSQLite::ProfileVersion const& ecdbVersion, BeSQLite::ProfileVersion const& dgndbVersion);

        Utf8StringCR GetName() const { return m_name; }
        BeFileNameCR GetPath() const { return m_path; }
        BeSQLite::ProfileVersion const& GetBeDbVersion() const { return m_bedbVersion; }
        BeSQLite::ProfileVersion const& GetECDbVersion() const { return m_ecdbVersion; }
        BeSQLite::ProfileVersion const& GeDgnDbVersion() const { return m_dgndbVersion; }

        Utf8String ToString() const;
    };

//======================================================================================
// @bsiclass                                                 Affan.Khan          03/2018
//======================================================================================
struct Profile : NonCopyableClass
    {
    protected:
        mutable BeSQLite::ProfileVersion m_expectedVersion = BeSQLite::ProfileVersion(0, 0, 0, 0);
        BeSQLite::PropertySpec m_versionPropertySpec;
        mutable std::vector<BeSQLite::ProfileVersion> m_expectedIncludedProfileVersions;

    private:
        ProfileType m_type;
        Utf8CP m_name = nullptr;
        BeFileName m_profileSeedFolder;

        virtual BentleyStatus _Init() const = 0;

    protected:
        Profile(ProfileType type, Utf8CP nameSpace, Utf8CP name, BeFileNameCR seedFolder);

        BeSQLite::ProfileVersion ReadProfileVersion(BeSQLite::Db const&) const;

    public:
        virtual ~Profile() {}

        BentleyStatus Init() const;

        BeSQLite::ProfileVersion const& GetExpectedVersion() const { return m_expectedVersion; }
        std::vector<BeSQLite::ProfileVersion> const& GetExpectedIncludedProfileVersions() const { return m_expectedIncludedProfileVersions; }

        std::vector<TestFile> GetAllVersionsOfTestFile(Utf8CP testFileName, bool logFoundFiles = true) const;
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
        explicit BeDbProfile(BeFileNameCR seedFolder) : Profile(ProfileType::BeDb, "be_Db", PROFILE_NAME_BEDB, seedFolder) {}
        BentleyStatus _Init() const override;

    public:
        ~BeDbProfile() {}

        static std::unique_ptr<BeDbProfile> Create(BeFileNameCR seedFolder)
            {
            std::unique_ptr<BeDbProfile> profile(new BeDbProfile(seedFolder));
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
        explicit ECDbProfile(BeFileNameCR seedFolder) : Profile(ProfileType::ECDb, "ec_Db", PROFILE_NAME_ECDB, seedFolder) {}
        BentleyStatus _Init() const override;
    public:
        ~ECDbProfile() {}

        static std::unique_ptr<ECDbProfile> Create(BeFileNameCR seedFolder)
            {
            std::unique_ptr<ECDbProfile> profile(new ECDbProfile(seedFolder));
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
        explicit DgnDbProfile(BeFileNameCR seedFolder) : Profile(ProfileType::DgnDb, "dgn_Db", PROFILE_NAME_DGNDB, seedFolder) {}
        BentleyStatus _Init() const override;
    public:
        ~DgnDbProfile() {}

        static std::unique_ptr<DgnDbProfile> Create(BeFileNameCR seedFolder)
            {
            std::unique_ptr<DgnDbProfile> profile(new DgnDbProfile(seedFolder));
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
        std::map<ProfileType, std::unique_ptr<Profile>> m_profiles;
        BeFileName m_testSeedFolder;
        static ProfileManager* s_singleton;

        ProfileManager();

    public:
        static ProfileManager& Get() { return *s_singleton; }
        Profile& GetProfile(ProfileType) const;
        BeFileNameCR GetSeedFolder() const { return m_testSeedFolder; }
    };
